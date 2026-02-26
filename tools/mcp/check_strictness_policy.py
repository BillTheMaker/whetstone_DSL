#!/usr/bin/env python3
"""
Fail if strictness_report.json violates strictness_policy.json thresholds.
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--policy", default="tools/mcp/grammars/strictness_policy.json")
    ap.add_argument("--report", default="tools/mcp/grammars/strictness_report.json")
    args = ap.parse_args()

    policy = json.loads(Path(args.policy).read_text())
    report = json.loads(Path(args.report).read_text())
    s = report.get("summary", {})

    failures = []
    if s.get("tool_count", 0) < int(policy.get("min_tool_coverage", 0)):
        failures.append("tool coverage below minimum")
    if s.get("missing_gbnf_count", 0) > int(policy.get("max_missing_gbnf", 0)):
        failures.append("missing gbnf files exceeds policy")
    if s.get("missing_per_tool_schema_count", 0) > int(policy.get("max_missing_per_tool_schema", 0)):
        failures.append("missing per-tool schema entries exceeds policy")
    if s.get("unwaived_fallback_count", 0) > int(policy.get("max_unwaived_fallback_count", 0)):
        failures.append("unwaived fallback count exceeds policy")
    if s.get("unsupported_schema_entry_count", 0) > int(policy.get("max_unsupported_schema_entries", 0)):
        failures.append("unsupported schema entries exceeds policy")

    if failures:
        print("Strictness policy FAILED")
        for f in failures:
            print(f"- {f}")
        print(json.dumps(s, indent=2, sort_keys=True))
        sys.exit(2)

    print("Strictness policy PASSED")
    print(json.dumps(s, indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
