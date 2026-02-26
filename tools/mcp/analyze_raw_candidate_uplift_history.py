#!/usr/bin/env python3
import argparse
import json
from pathlib import Path
from typing import Dict, List


def load_json(path: Path):
    with path.open("r", encoding="utf-8") as f:
        return json.load(f)


def main() -> int:
    p = argparse.ArgumentParser(description="Analyze recent raw candidate search uplift history.")
    p.add_argument("--runs-root", default="logs/taskitem_runs")
    p.add_argument("--include-glob", default="*")
    p.add_argument("--max-runs", type=int, default=20)
    p.add_argument("--out", required=True)
    args = p.parse_args()

    root = Path(args.runs_root)
    summaries: List[Path] = sorted(root.glob(f"{args.include_glob}/00_summary.json"), key=lambda x: x.stat().st_mtime, reverse=True)

    rows = []
    for path in summaries[: max(0, args.max_runs)]:
        try:
            s = load_json(path)
        except Exception:
            continue
        raw = s.get("native_raw_candidate_search") or {}
        if not isinstance(raw, dict) or not raw.get("enabled", False):
            continue
        baseline = int(raw.get("baseline_failing_profile_count", 999) or 999)
        best = int(raw.get("best_failing_profile_count", 999) or 999)
        rows.append({
            "run_id": path.parent.name,
            "baseline_failing_profile_count": baseline,
            "best_failing_profile_count": best,
            "uplift": int(best < baseline),
        })

    total = len(rows)
    uplift_count = sum(int(r.get("uplift", 0)) for r in rows)
    no_uplift_count = total - uplift_count

    out = {
        "status": "ok",
        "record_count": total,
        "uplift_count": uplift_count,
        "no_uplift_count": no_uplift_count,
        "uplift_rate": round((uplift_count / total), 4) if total else 0.0,
        "records": rows,
    }

    with Path(args.out).open("w", encoding="utf-8") as f:
        json.dump(out, f, indent=2, sort_keys=True)
        f.write("\n")

    print(json.dumps({"status": "ok", "record_count": total, "uplift_rate": out["uplift_rate"], "out": args.out}, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
