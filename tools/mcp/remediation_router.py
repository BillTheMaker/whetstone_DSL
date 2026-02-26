#!/usr/bin/env python3
"""Deterministic remediation routing from gate diagnostics to action plan."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Dict, List


def load_json(path: str) -> Dict[str, object]:
    p = Path(path)
    if not p.exists():
        return {}
    return json.loads(p.read_text())


def classify(gates: Dict[str, object]) -> Dict[str, object]:
    g = gates.get("gates", {}) if isinstance(gates, dict) else {}
    reasons = list(g.get("failure_reasons", [])) if isinstance(g, dict) else []
    compile_reason = str((g.get("compile") or {}).get("reason", ""))
    tests_reason = str((g.get("tests") or {}).get("reason", ""))
    diagnostics = gates.get("diagnostics", []) if isinstance(gates, dict) else []
    diag_text = "\\n".join(str(d.get("message", "")) + "\\n" + str(d.get("raw_excerpt", "")) for d in diagnostics if isinstance(d, dict)).lower()

    actions: List[Dict[str, object]] = []

    nonrecoverable = "tool_missing" in compile_reason or "tool_missing" in tests_reason
    if nonrecoverable:
        actions.append(
            {
                "action": "stop_blocked",
                "tool": "none",
                "reason": "missing_toolchain",
                "confidence": 1.0,
                "priority": 100,
            }
        )
        return {
            "blocked": True,
            "blocked_reason": "missing_toolchain",
            "actions": actions,
        }

    if any(r.startswith("placeholder_or_todo") for r in reasons):
        actions.append(
            {
                "action": "strengthen_spec",
                "tool": "whetstone_generate_code",
                "hint": "Eliminate placeholders/TODO markers and provide concrete field/method types and bodies.",
                "confidence": 0.95,
                "priority": 90,
            }
        )

    if any(r.startswith("compile_failed") for r in reasons):
        if "defined in header" in diag_text or "did you forget to '#include" in diag_text:
            actions.append(
                {
                    "action": "fix_cpp_includes",
                    "tool": "apply_cpp_diagnostic_fixes",
                    "hint": "Apply deterministic include insertion from compiler diagnostics before re-running gates.",
                    "confidence": 0.98,
                    "priority": 95,
                }
            )
        actions.append(
            {
                "action": "diagnose_pipeline",
                "tool": "whetstone_run_pipeline",
                "hint": "Run same-language pipeline pass to collect parse/validation diagnostics and refine signatures/types.",
                "confidence": 0.9,
                "priority": 80,
            }
        )
        actions.append(
            {
                "action": "strengthen_spec",
                "tool": "whetstone_generate_code",
                "hint": "Emit compile-clean code for target language with explicit imports/includes and fully typed declarations.",
                "confidence": 0.85,
                "priority": 70,
            }
        )

    if any(r.startswith("tests_failed") for r in reasons):
        actions.append(
            {
                "action": "strengthen_spec",
                "tool": "whetstone_generate_code",
                "hint": "Implement queue behaviors: enqueue/dequeue/peek/size/empty with deterministic semantics.",
                "confidence": 0.9,
                "priority": 75,
            }
        )

    actions.sort(key=lambda a: (-int(a["priority"]), a["action"]))
    return {
        "blocked": False,
        "blocked_reason": "",
        "actions": actions,
    }


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--gates-json", required=True)
    ap.add_argument("--out", default="")
    args = ap.parse_args()

    gates = load_json(args.gates_json)
    plan = classify(gates)
    payload = {
        "router_version": "1",
        "input_failure_reasons": (gates.get("gates", {}) or {}).get("failure_reasons", []),
        "blocked": plan["blocked"],
        "blocked_reason": plan["blocked_reason"],
        "actions": plan["actions"],
    }
    text = json.dumps(payload, indent=2, sort_keys=True)
    if args.out:
        out = Path(args.out)
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_text(text + "\n")
    print(text)


if __name__ == "__main__":
    main()
