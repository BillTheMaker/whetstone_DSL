#!/usr/bin/env python3
import argparse
import json
from pathlib import Path
from typing import Dict


def load_json(path: Path):
    with path.open("r", encoding="utf-8") as f:
        return json.load(f)


def main() -> int:
    p = argparse.ArgumentParser(description="Aggregate closure ladder outcomes.")
    p.add_argument("--runs-root", default="logs/taskitem_runs")
    p.add_argument("--include-glob", default="TEST_ONLY_*closure_ladder*")
    p.add_argument("--out", required=True)
    args = p.parse_args()

    root = Path(args.runs_root)
    summaries = sorted(root.glob(f"{args.include_glob}/closure_ladder_summary.json"))

    mode_counts: Dict[str, int] = {}
    status_counts: Dict[str, int] = {}
    rows = []
    for s in summaries:
        try:
            j = load_json(s)
        except Exception:
            continue
        mode = str(j.get("selected_mode", ""))
        status = str(j.get("status", "unknown"))
        mode_counts[mode] = mode_counts.get(mode, 0) + 1
        status_counts[status] = status_counts.get(status, 0) + 1
        rows.append({
            "run_id": s.parent.name,
            "status": status,
            "selected_mode": mode,
            "selected_run": str(j.get("selected_run", "")),
        })

    out = {
        "status": "ok",
        "record_count": len(rows),
        "status_counts": status_counts,
        "selected_mode_counts": mode_counts,
        "records": rows,
    }

    with Path(args.out).open("w", encoding="utf-8") as f:
        json.dump(out, f, indent=2, sort_keys=True)
        f.write("\n")

    print(json.dumps({"status": "ok", "record_count": len(rows), "out": args.out}, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
