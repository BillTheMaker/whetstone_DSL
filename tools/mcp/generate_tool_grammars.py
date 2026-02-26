#!/usr/bin/env python3
"""
Generate strict-ish GBNF grammars and JSON Schemas for all Whetstone MCP tools.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
from typing import Any, Dict, List, Tuple

from schema_normalizer import normalize_tool_schemas


GBNF_PRIMITIVES = """\
# ---------------------------------------------------------------------------
# JSON primitives
# ---------------------------------------------------------------------------
ws        ::= [ \\t\\n\\r]*
string    ::= "\\\"" ([^"\\\\\\x7F\\x00-\\x1F] | "\\\\" (["\\\\/bfnrt] | "u" [0-9a-fA-F] [0-9a-fA-F] [0-9a-fA-F] [0-9a-fA-F]))* "\\\""
number    ::= "-"? ([0-9] | [1-9] [0-9]*) ("." [0-9]+)? ([eE] [-+]? [0-9]+)?
integer   ::= "-"? ([0-9] | [1-9] [0-9]*)
boolean   ::= "true" | "false"
null      ::= "null"
any-value ::= string | number | boolean | null | any-object | any-array
any-object ::= "{" ws (string ws ":" ws any-value (ws "," ws string ws ":" ws any-value)*)? ws "}"
any-array  ::= "[" ws (any-value (ws "," ws any-value)*)? ws "]"
"""


class GrammarBuilder:
    def __init__(self) -> None:
        self.rules: Dict[str, str] = {}
        self.counter = 0
        self.fallbacks: List[Dict[str, Any]] = []

    def unique(self, base: str) -> str:
        self.counter += 1
        return f"{base}-{self.counter}"

    def add(self, rule_name: str, expr: str) -> None:
        if rule_name not in self.rules:
            self.rules[rule_name] = expr

    def emit_value(self, node: Dict[str, Any], base: str, *, tool: str, path: str) -> str:
        kind = node.get("kind", "any")

        if kind == "string":
            if "const" in node and isinstance(node["const"], str):
                return f'"\\\"{node["const"]}\\\""'
            enum_vals = node.get("enum")
            if isinstance(enum_vals, list) and enum_vals and all(isinstance(v, str) for v in enum_vals):
                return " | ".join(f'"\\\"{v}\\\""' for v in enum_vals)
            return "string"
        if kind == "integer":
            return "integer"
        if kind == "number":
            return "number"
        if kind == "boolean":
            return "boolean"
        if kind == "null":
            return "null"

        if kind == "array":
            item_rule = self.emit_value(node.get("items", {"kind": "any"}), self.unique(f"{base}-item"), tool=tool, path=f"{path}.items")
            arr_rule = self.unique(f"{base}-arr")
            self.add(arr_rule, f'"[" ws ({item_rule} (ws "," ws {item_rule})*)? ws "]"')
            return arr_rule

        if kind == "arrayTuple":
            prefix = node.get("prefixItems", [])
            if not prefix:
                return '"[" ws "]"'
            parts: List[str] = []
            for i, p in enumerate(prefix):
                parts.append(self.emit_value(p, self.unique(f"{base}-tuple{i}"), tool=tool, path=f"{path}.prefixItems[{i}]"))
            tup_rule = self.unique(f"{base}-tuple")
            expr = parts[0]
            for part in parts[1:]:
                expr += f' ws "," ws {part}'
            self.add(tup_rule, f'"[" ws {expr} ws "]"')
            return tup_rule

        if kind in ("oneOf", "anyOf"):
            branches = node.get("branches", [])
            alts: List[str] = []
            for i, b in enumerate(branches):
                alts.append(self.emit_value(b, self.unique(f"{base}-b{i}"), tool=tool, path=f"{path}.{kind}[{i}]"))
            if not alts:
                self.fallbacks.append({"tool": tool, "path": path, "reason": f"empty_{kind}", "allowed": False})
                return "any-value"
            union_rule = self.unique(f"{base}-{kind}")
            self.add(union_rule, " | ".join(alts))
            return union_rule

        if kind == "allOf":
            branches = node.get("branches", [])
            object_branches = [b for b in branches if b.get("kind") == "object"]
            if len(object_branches) == len(branches) and branches:
                merged_props: Dict[str, Any] = {}
                merged_req = set()
                additional = False
                for b in object_branches:
                    merged_props.update(b.get("properties", {}))
                    merged_req.update(b.get("required", []))
                    ap = b.get("additionalProperties", True)
                    additional = additional or bool(ap)
                merged = {
                    "kind": "object",
                    "properties": merged_props,
                    "required": sorted(merged_req),
                    "additionalProperties": additional,
                    "allowBroad": False,
                }
                return self.emit_value(merged, self.unique(f"{base}-allof-merged"), tool=tool, path=f"{path}.allOfMerged")
            self.fallbacks.append({"tool": tool, "path": path, "reason": "allof_non_object", "allowed": False})
            return "any-value"

        if kind == "object":
            props = node.get("properties", {})
            required = list(node.get("required", []))
            optional = [k for k in sorted(props.keys()) if k not in required]
            required = [k for k in required if k in props]

            if not props:
                ap = node.get("additionalProperties", True)
                if ap is False:
                    return '"{" ws "}"'
                allowed = bool(node.get("allowBroad", False))
                self.fallbacks.append({"tool": tool, "path": path, "reason": "broad_object_no_props", "allowed": allowed})
                return "any-object"

            pair_rules: Dict[str, str] = {}
            for field in sorted(props.keys()):
                field_rule = self.emit_value(props[field], self.unique(f"{base}-{field}"), tool=tool, path=f"{path}.properties.{field}")
                pair_rule = self.unique(f"{base}-{field}-pair")
                self.add(pair_rule, f'"\\\"{field}\\\"" ws ":" ws {field_rule}')
                pair_rules[field] = pair_rule

            if not required:
                # all optional: any subset in any order (coarse but strict field-key set)
                opt_rule = self.unique(f"{base}-opt")
                self.add(opt_rule, " | ".join(pair_rules[k] for k in sorted(pair_rules.keys())))
                obj_rule = self.unique(f"{base}-obj")
                self.add(obj_rule, f'"{{" ws ({opt_rule} (ws "," ws {opt_rule})*)? ws "}}"')
                return obj_rule

            req_seq = ' ws "," ws '.join(pair_rules[k] for k in required)
            obj_rule = self.unique(f"{base}-obj")
            if optional:
                opt_rule = self.unique(f"{base}-opt")
                self.add(opt_rule, " | ".join(pair_rules[k] for k in optional))
                self.add(obj_rule, f'"{{" ws {req_seq} (ws "," ws {opt_rule})* ws "}}"')
            else:
                self.add(obj_rule, f'"{{" ws {req_seq} ws "}}"')
            return obj_rule

        allowed = bool(node.get("allowBroad", False))
        self.fallbacks.append({"tool": tool, "path": path, "reason": "kind_any", "allowed": allowed})
        return "any-value"


def _rule_name(tool_name: str) -> str:
    return tool_name.replace("_", "-")


def generate_tool_gbnf(tool_name: str, normalized_tool: Dict[str, Any]) -> Tuple[str, List[Dict[str, Any]]]:
    builder = GrammarBuilder()
    schema = normalized_tool["schema"]
    base = _rule_name(tool_name)

    args_rule = builder.emit_value(schema, f"{base}-args", tool=tool_name, path=f"tools.{tool_name}")

    lines: List[str] = [GBNF_PRIMITIVES, f"# --- {tool_name} ---"]
    lines.append(
        f'root ::= "{{" ws "\\\"tool\\\"" ws ":" ws "\\\"{tool_name}\\\"" ws "," ws "\\\"arguments\\\"" ws ":" ws {args_rule} ws "}}"'
    )
    lines.append("")

    for rn in sorted(builder.rules.keys()):
        lines.append(f"{rn} ::= {builder.rules[rn]}")

    return "\n".join(lines) + "\n", builder.fallbacks


def generate_dispatch_gbnf(normalized: Dict[str, Any]) -> Tuple[str, List[Dict[str, Any]]]:
    tool_rules: Dict[str, str] = {}
    all_fallbacks: List[Dict[str, Any]] = []

    for tool_name in sorted(normalized.keys()):
        text, fallbacks = generate_tool_gbnf(tool_name, normalized[tool_name])
        # extract rule body from per-tool grammar (all rules after first root definition)
        lines = text.splitlines()
        tool_rules[tool_name] = "\n".join(lines[3:])
        all_fallbacks.extend(fallbacks)

    lines: List[str] = [GBNF_PRIMITIVES, "# --- Full Whetstone dispatch grammar ---", ""]
    alts = " |\n    ".join(f"{_rule_name(t)}-call" for t in sorted(normalized.keys()))
    lines.append(f"root ::= {alts}")
    lines.append("")

    for tool_name in sorted(normalized.keys()):
        base = _rule_name(tool_name)
        # Re-emit one call rule pointing to the generated args root rule name.
        # For per-tool files, root points directly to call JSON. For dispatch we use dedicated call rule.
        args_root = f"{base}-args-1"
        # Emit tool call rule.
        lines.append(f"# {tool_name}")
        lines.append(
            f'{base}-call ::= "{{" ws "\\\"tool\\\"" ws ":" ws "\\\"{tool_name}\\\"" ws "," ws "\\\"arguments\\\"" ws ":" ws {args_root} ws "}}"'
        )
        # Append remaining rules from tool grammar verbatim.
        lines.append(tool_rules[tool_name])
        lines.append("")

    return "\n".join(lines) + "\n", all_fallbacks


def generate_tool_json_schema(tool_name: str, schema: Dict[str, Any]) -> Dict[str, Any]:
    return {
        "type": "object",
        "properties": {
            "tool": {"type": "string", "const": tool_name},
            "arguments": schema if schema.get("properties") else {"type": "object"},
        },
        "required": ["tool", "arguments"],
        "additionalProperties": False,
    }


def generate_dispatch_json_schema(schemas: Dict[str, Any]) -> Dict[str, Any]:
    return {"oneOf": [generate_tool_json_schema(name, schema) for name, schema in sorted(schemas.items())]}


def write_manifest(out_dir: Path, schemas: Dict[str, Any], dispatch_gbnf: str, dispatch_schema: Dict[str, Any]) -> None:
    schema_json = json.dumps(schemas, sort_keys=True, separators=(",", ":"))
    dispatch_schema_json = json.dumps(dispatch_schema, sort_keys=True, separators=(",", ":"))
    payload = {
        "tool_count": len(schemas),
        "tool_schema_sha256": hashlib.sha256(schema_json.encode()).hexdigest(),
        "dispatch_gbnf_sha256": hashlib.sha256(dispatch_gbnf.encode()).hexdigest(),
        "dispatch_schema_sha256": hashlib.sha256(dispatch_schema_json.encode()).hexdigest(),
    }
    manifest_path = out_dir / "manifest.json"
    manifest_path.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n")
    lock = hashlib.sha256(json.dumps(payload, sort_keys=True, separators=(",", ":")).encode()).hexdigest()
    (out_dir / "manifest.lock").write_text(lock + "\n")


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--schemas", default="tools/mcp/whetstone_tool_schemas.json")
    ap.add_argument("--out-dir", default="tools/mcp/grammars")
    ap.add_argument("--strict-report", default="tools/mcp/grammars/strictness_report.json")
    args = ap.parse_args()

    schema_path = Path(args.schemas)
    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    raw_schemas = json.loads(schema_path.read_text())
    normalized_payload = normalize_tool_schemas(raw_schemas)

    normalized_path = out_dir / "normalized_tool_schemas.json"
    normalized_path.write_text(
        json.dumps(
            {
                "summary": {
                    "tool_count": len(normalized_payload),
                    "unsupported_entry_count": sum(len(x.get("unsupported", [])) for x in normalized_payload.values()),
                },
                "tools": normalized_payload,
            },
            indent=2,
            sort_keys=True,
        )
        + "\n"
    )

    all_fallbacks: List[Dict[str, Any]] = []
    for tool_name in sorted(normalized_payload.keys()):
        gbnf, fallbacks = generate_tool_gbnf(tool_name, normalized_payload[tool_name])
        (out_dir / f"{tool_name}.gbnf").write_text(gbnf)
        all_fallbacks.extend(fallbacks)

    dispatch_gbnf, dispatch_fallbacks = generate_dispatch_gbnf(normalized_payload)
    all_fallbacks.extend(dispatch_fallbacks)
    (out_dir / "dispatch.gbnf").write_text(dispatch_gbnf)

    per_tool = {name: generate_tool_json_schema(name, schema) for name, schema in raw_schemas.items()}
    (out_dir / "per_tool_schemas.json").write_text(json.dumps(per_tool, indent=2, sort_keys=True) + "\n")

    dispatch_schema = generate_dispatch_json_schema(raw_schemas)
    (out_dir / "dispatch_schema.json").write_text(json.dumps(dispatch_schema, indent=2, sort_keys=True) + "\n")

    write_manifest(out_dir, raw_schemas, dispatch_gbnf, dispatch_schema)

    unsupported_entries = sum(len(v.get("unsupported", [])) for v in normalized_payload.values())
    strict_report = {
        "summary": {
            "tool_count": len(raw_schemas),
            "fallback_count": len(all_fallbacks),
            "waived_fallback_count": sum(1 for x in all_fallbacks if x.get("allowed")),
            "unwaived_fallback_count": sum(1 for x in all_fallbacks if not x.get("allowed")),
            "unsupported_schema_entries": unsupported_entries,
        },
        "fallbacks": all_fallbacks,
    }
    Path(args.strict_report).write_text(json.dumps(strict_report, indent=2, sort_keys=True) + "\n")

    print(f"Wrote {len(raw_schemas)} per-tool .gbnf files")
    print(f"Wrote dispatch.gbnf ({len(dispatch_gbnf)} bytes)")
    print("Wrote per_tool_schemas.json")
    print("Wrote dispatch_schema.json")
    print("Wrote normalized_tool_schemas.json")
    print("Wrote strictness_report.json")
    print("Wrote manifest.json and manifest.lock")


if __name__ == "__main__":
    main()
