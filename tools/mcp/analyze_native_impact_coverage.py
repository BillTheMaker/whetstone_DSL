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
    p = argparse.ArgumentParser(description="Aggregate native impact coverage reports across runs.")
    p.add_argument("--runs-root", default="logs/taskitem_runs")
    p.add_argument("--include-glob", default="*")
    p.add_argument("--out", required=True)
    args = p.parse_args()

    root = Path(args.runs_root)
    profile_counts: Dict[str, Dict[str, int]] = {}
    total_runs = 0
    failed_runs = 0

    for report_path in sorted(root.glob(f"{args.include_glob}/06_native_impact_coverage.json")):
        try:
            report = load_json(report_path)
        except Exception:
            continue
        total_runs += 1
        if report.get("status") == "fail":
            failed_runs += 1
        for c in report.get("checks", []):
            pid = str(c.get("id", "unknown"))
            row = profile_counts.setdefault(pid, {"active": 0, "passed": 0, "failed": 0})
            row["active"] += 1
            if c.get("passed", False):
                row["passed"] += 1
            else:
                row["failed"] += 1

    out = {
        "status": "ok",
        "total_runs": total_runs,
        "failed_runs": failed_runs,
        "fail_rate": round((failed_runs / total_runs), 4) if total_runs else 0.0,
        "profile_counts": profile_counts,
    }
    write_json(Path(args.out), out)
    print(json.dumps({"status": "ok", "total_runs": total_runs, "failed_runs": failed_runs, "out": args.out}, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
