#!/usr/bin/env python3
"""
Normalize Whetstone MCP tool schemas into a canonical, recursion-safe structure.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any, Dict, List, Set, Tuple


SUPPORTED_CONSTRAINT_KEYS = {
    "enum",
    "const",
    "pattern",
    "minLength",
    "maxLength",
    "minimum",
    "maximum",
    "exclusiveMinimum",
    "exclusiveMaximum",
    "minItems",
    "maxItems",
}


def _json_pointer_get(doc: Any, ref: str) -> Any:
    if not ref.startswith("#/"):
        raise ValueError(f"Only local refs are supported: {ref}")
    cur = doc
    for tok in ref[2:].split("/"):
        tok = tok.replace("~1", "/").replace("~0", "~")
        if not isinstance(cur, dict) or tok not in cur:
            raise KeyError(f"Ref path not found: {ref}")
        cur = cur[tok]
    return cur


def _infer_type(node: Dict[str, Any]) -> str:
    t = node.get("type")
    if isinstance(t, str):
        return t
    if isinstance(t, list):
        if "null" in t and len(t) == 2:
            return next(x for x in t if x != "null")
    if "properties" in node:
        return "object"
    if "items" in node or "prefixItems" in node:
        return "array"
    if "enum" in node:
        vals = node["enum"]
        if vals and all(isinstance(v, str) for v in vals):
            return "string"
    return "any"


def _extract_constraints(node: Dict[str, Any]) -> Dict[str, Any]:
    out: Dict[str, Any] = {}
    for k in SUPPORTED_CONSTRAINT_KEYS:
        if k in node:
            out[k] = node[k]
    return out


def normalize_node(
    node: Any,
    *,
    root_doc: Dict[str, Any],
    path: str,
    unsupported: List[Dict[str, Any]],
    ref_stack: Set[str],
) -> Dict[str, Any]:
    if not isinstance(node, dict):
        unsupported.append({"path": path, "reason": "non_object_schema_node"})
        return {"kind": "any", "path": path}

    if "$ref" in node:
        ref = node["$ref"]
        if not isinstance(ref, str):
            unsupported.append({"path": path, "reason": "invalid_ref"})
            return {"kind": "any", "path": path}
        if ref in ref_stack:
            unsupported.append({"path": path, "reason": "ref_cycle", "ref": ref})
            return {"kind": "any", "path": path}
        try:
            target = _json_pointer_get(root_doc, ref)
        except Exception as exc:
            unsupported.append(
                {"path": path, "reason": "ref_resolution_failed", "ref": ref, "error": str(exc)}
            )
            return {"kind": "any", "path": path}
        return normalize_node(
            target,
            root_doc=root_doc,
            path=f"{path}->$ref({ref})",
            unsupported=unsupported,
            ref_stack=ref_stack | {ref},
        )

    for key in ("not", "if", "then", "else", "dependentSchemas", "patternProperties"):
        if key in node:
            unsupported.append({"path": path, "reason": "unsupported_keyword", "keyword": key})

    if "oneOf" in node:
        branches = node.get("oneOf") or []
        return {
            "kind": "oneOf",
            "path": path,
            "branches": [
                normalize_node(
                    b,
                    root_doc=root_doc,
                    path=f"{path}.oneOf[{i}]",
                    unsupported=unsupported,
                    ref_stack=ref_stack,
                )
                for i, b in enumerate(branches)
            ],
        }
    if "anyOf" in node:
        branches = node.get("anyOf") or []
        return {
            "kind": "anyOf",
            "path": path,
            "branches": [
                normalize_node(
                    b,
                    root_doc=root_doc,
                    path=f"{path}.anyOf[{i}]",
                    unsupported=unsupported,
                    ref_stack=ref_stack,
                )
                for i, b in enumerate(branches)
            ],
        }
    if "allOf" in node:
        branches = node.get("allOf") or []
        return {
            "kind": "allOf",
            "path": path,
            "branches": [
                normalize_node(
                    b,
                    root_doc=root_doc,
                    path=f"{path}.allOf[{i}]",
                    unsupported=unsupported,
                    ref_stack=ref_stack,
                )
                for i, b in enumerate(branches)
            ],
        }

    t = _infer_type(node)
    constraints = _extract_constraints(node)

    if t == "object":
        props = node.get("properties", {}) or {}
        required = sorted([x for x in node.get("required", []) if isinstance(x, str)])
        normalized_props: Dict[str, Any] = {}
        for key in sorted(props.keys()):
            normalized_props[key] = normalize_node(
                props[key],
                root_doc=root_doc,
                path=f"{path}.properties.{key}",
                unsupported=unsupported,
                ref_stack=ref_stack,
            )
        ap = node.get("additionalProperties", True)
        additional_properties = (
            normalize_node(
                ap,
                root_doc=root_doc,
                path=f"{path}.additionalProperties",
                unsupported=unsupported,
                ref_stack=ref_stack,
            )
            if isinstance(ap, dict)
            else bool(ap)
        )
        return {
            "kind": "object",
            "path": path,
            "properties": normalized_props,
            "required": required,
            "additionalProperties": additional_properties,
            "constraints": constraints,
            "allowBroad": bool(node.get("x-whetstone-allow-broad", False)),
        }

    if t == "array":
        if "prefixItems" in node and isinstance(node["prefixItems"], list):
            items = [
                normalize_node(
                    it,
                    root_doc=root_doc,
                    path=f"{path}.prefixItems[{i}]",
                    unsupported=unsupported,
                    ref_stack=ref_stack,
                )
                for i, it in enumerate(node["prefixItems"])
            ]
            additional = node.get("items", False)
            add_items = (
                normalize_node(
                    additional,
                    root_doc=root_doc,
                    path=f"{path}.items",
                    unsupported=unsupported,
                    ref_stack=ref_stack,
                )
                if isinstance(additional, dict)
                else bool(additional)
            )
            return {
                "kind": "arrayTuple",
                "path": path,
                "prefixItems": items,
                "additionalItems": add_items,
                "constraints": constraints,
                "allowBroad": bool(node.get("x-whetstone-allow-broad", False)),
            }
        return {
            "kind": "array",
            "path": path,
            "items": normalize_node(
                node.get("items", {}),
                root_doc=root_doc,
                path=f"{path}.items",
                unsupported=unsupported,
                ref_stack=ref_stack,
            ),
            "constraints": constraints,
            "allowBroad": bool(node.get("x-whetstone-allow-broad", False)),
        }

    if t in ("string", "integer", "number", "boolean", "null"):
        out = {"kind": t, "path": path, "constraints": constraints}
        if "enum" in node:
            out["enum"] = node["enum"]
        if "const" in node:
            out["const"] = node["const"]
        return out

    return {
        "kind": "any",
        "path": path,
        "constraints": constraints,
        "allowBroad": bool(node.get("x-whetstone-allow-broad", False)),
    }


def normalize_tool_schemas(raw: Dict[str, Any]) -> Dict[str, Any]:
    out: Dict[str, Any] = {}
    for tool_name in sorted(raw.keys()):
        schema = raw[tool_name]
        unsupported: List[Dict[str, Any]] = []
        normalized = normalize_node(
            schema,
            root_doc=raw,
            path=f"tools.{tool_name}",
            unsupported=unsupported,
            ref_stack=set(),
        )
        out[tool_name] = {
            "schema": normalized,
            "unsupported": unsupported,
        }
    return out


def _summarize(normalized: Dict[str, Any]) -> Dict[str, Any]:
    unsupported_count = sum(len(v.get("unsupported", [])) for v in normalized.values())
    return {
        "tool_count": len(normalized),
        "unsupported_entry_count": unsupported_count,
    }


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument(
        "--schemas",
        default="tools/mcp/whetstone_tool_schemas.json",
        help="Input tool schema map",
    )
    ap.add_argument(
        "--out",
        default="tools/mcp/grammars/normalized_tool_schemas.json",
        help="Output normalized schema file",
    )
    args = ap.parse_args()

    schema_path = Path(args.schemas)
    raw = json.loads(schema_path.read_text())
    normalized = normalize_tool_schemas(raw)
    payload = {
        "summary": _summarize(normalized),
        "tools": normalized,
    }
    out_path = Path(args.out)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n")
    print(f"Wrote normalized schemas: {out_path}")
    print(json.dumps(payload["summary"], indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
