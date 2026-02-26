#!/usr/bin/env python3
import argparse
import json
from pathlib import Path
from typing import Dict


def load_json(path: Path) -> Dict:
    with path.open("r", encoding="utf-8") as f:
        return json.load(f)


def write_json(path: Path, obj: Dict) -> None:
    with path.open("w", encoding="utf-8") as f:
        json.dump(obj, f, indent=2, sort_keys=True)
        f.write("\n")


def main() -> int:
    parser = argparse.ArgumentParser(description="Check semantic fallback usage against a configured budget.")
    parser.add_argument("--summary", required=True, help="Path to semantic_fallback_summary.json")
    parser.add_argument("--out", required=True, help="Output gate evaluation JSON")
    parser.add_argument("--max-fallback-rate", type=float, default=0.35, help="Max allowed fallback rate in [0,1]")
    parser.add_argument("--min-record-count", type=int, default=5, help="Minimum records required before strict gating")
    args = parser.parse_args()

    summary = load_json(Path(args.summary))
    fallback_rate = float(summary.get("fallback_rate", 0.0) or 0.0)
    record_count = int(summary.get("record_count", 0) or 0)

    enforceable = record_count >= args.min_record_count
    passed = True
    reason = "insufficient_data"
    if enforceable:
        passed = fallback_rate <= args.max_fallback_rate
        reason = "within_budget" if passed else "fallback_rate_exceeds_budget"

    result = {
        "passed": bool(passed),
        "reason": reason,
        "enforceable": bool(enforceable),
        "record_count": record_count,
        "min_record_count": int(args.min_record_count),
        "fallback_rate": fallback_rate,
        "max_fallback_rate": float(args.max_fallback_rate),
    }
    write_json(Path(args.out), result)

    print(json.dumps(result, sort_keys=True))
    return 0 if passed else 9


if __name__ == "__main__":
    raise SystemExit(main())
