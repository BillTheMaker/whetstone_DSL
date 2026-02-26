#!/usr/bin/env python3
"""
Audit strictness coverage for generated MCP grammars.
"""

from __future__ import annotations

import argparse
import glob
import json
from pathlib import Path
from typing import Any, Dict


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--schemas", default="tools/mcp/whetstone_tool_schemas.json")
    ap.add_argument("--grammars-dir", default="tools/mcp/grammars")
    ap.add_argument("--out", default="tools/mcp/grammars/strictness_report.json")
    args = ap.parse_args()

    schemas = json.loads(Path(args.schemas).read_text())
    grammars_dir = Path(args.grammars_dir)
    per_tool = json.loads((grammars_dir / "per_tool_schemas.json").read_text())
    normalized_path = grammars_dir / "normalized_tool_schemas.json"
    normalized = json.loads(normalized_path.read_text()) if normalized_path.exists() else {"tools": {}}

    gbnf_files = [Path(p) for p in glob.glob(str(grammars_dir / "*.gbnf"))]
    tool_gbnf = [p for p in gbnf_files if p.name != "dispatch.gbnf"]

    by_tool: Dict[str, Any] = {}
    fallback_count = 0
    waived_fallback_count = 0
    unsupported_count = 0
    fallback_by_tool: Dict[str, Dict[str, int]] = {}

    generated_report_path = grammars_dir / "strictness_report.json"
    generated_report = (
        json.loads(generated_report_path.read_text())
        if generated_report_path.exists()
        else {"fallbacks": []}
    )
    for fb in generated_report.get("fallbacks", []):
        tool = fb.get("tool", "")
        if not tool:
            continue
        ent = fallback_by_tool.setdefault(tool, {"total": 0, "waived": 0})
        ent["total"] += 1
        if fb.get("allowed", False):
            ent["waived"] += 1

    for tool in sorted(schemas.keys()):
        fpath = grammars_dir / f"{tool}.gbnf"
        has_gbnf = fpath.exists()
        fb = fallback_by_tool.get(tool, {}).get("total", 0)
        fallback_count += fb
        tool_norm = normalized.get("tools", {}).get(tool, {})
        unsupported = len(tool_norm.get("unsupported", []))
        unsupported_count += unsupported

        schema = schemas[tool]
        allow_broad = bool(schema.get("x-whetstone-allow-broad", False))
        tool_waived = fallback_by_tool.get(tool, {}).get("waived", 0)
        if allow_broad and fb > 0 and tool_waived == 0:
            tool_waived = fb
        waived_fallback_count += tool_waived

        by_tool[tool] = {
            "has_gbnf": has_gbnf,
            "has_per_tool_schema": tool in per_tool,
            "fallback_token_count": fb,
            "unsupported_schema_entries": unsupported,
            "allow_broad": allow_broad,
            "waived_fallback_count": tool_waived,
        }

    payload = {
        "summary": {
            "tool_count": len(schemas),
            "gbnf_count": len(tool_gbnf),
            "per_tool_schema_count": len(per_tool),
            "missing_gbnf_count": sum(1 for v in by_tool.values() if not v["has_gbnf"]),
            "missing_per_tool_schema_count": sum(1 for v in by_tool.values() if not v["has_per_tool_schema"]),
            "fallback_token_count": fallback_count,
            "waived_fallback_count": waived_fallback_count,
            "unwaived_fallback_count": fallback_count - waived_fallback_count,
            "unsupported_schema_entry_count": unsupported_count,
        },
        "tools": by_tool,
    }

    out = Path(args.out)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n")
    print(f"Wrote strictness report: {out}")
    print(json.dumps(payload["summary"], indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
