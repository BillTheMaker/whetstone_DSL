#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Dict, List


def gate(packet: Dict[str, object], min_coverage: float, require_caps_for_complex: bool, min_complexity_score: int) -> Dict[str, object]:
    coverage_ratio = float((((packet.get("coverage") or {}).get("coverage_ratio")) or 0.0))
    anns = packet.get("annotations") or []

    cap_count = 0
    complexity_score = 0
    for ann in anns:
        if not isinstance(ann, dict):
            continue
        t = ann.get("type")
        if t == "capability_requirement":
            cap_count += 1
        if t == "complexity":
            fields = ann.get("fields") or {}
            complexity_score = int(fields.get("score") or 0)

    reasons: List[str] = []
    if coverage_ratio < min_coverage:
        reasons.append(f"coverage_below_threshold:{coverage_ratio}<{min_coverage}")

    if require_caps_for_complex and complexity_score >= min_complexity_score and cap_count == 0:
        reasons.append("complex_spec_missing_capability_requirements")

    passed = len(reasons) == 0
    return {
        "passed": passed,
        "reasons": reasons,
        "metrics": {
            "coverage_ratio": coverage_ratio,
            "capability_requirement_count": cap_count,
            "complexity_score": complexity_score,
            "min_coverage": min_coverage,
            "require_caps_for_complex": require_caps_for_complex,
            "min_complexity_score": min_complexity_score,
        },
    }


def main() -> None:
    ap = argparse.ArgumentParser(description="Check semantic planning packet quality gate.")
    ap.add_argument("--packet", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--min-coverage", type=float, default=0.9)
    ap.add_argument("--require-caps-for-complex", action="store_true")
    ap.add_argument("--min-complexity-score", type=int, default=2)
    args = ap.parse_args()

    packet = json.loads(Path(args.packet).read_text(encoding="utf-8"))
    res = gate(packet, args.min_coverage, args.require_caps_for_complex, args.min_complexity_score)
    out = Path(args.out)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(res, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(res, indent=2))
    if not res["passed"]:
        raise SystemExit(3)


if __name__ == "__main__":
    main()
