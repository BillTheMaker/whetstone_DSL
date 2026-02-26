#!/usr/bin/env python3
import argparse
import json
from pathlib import Path
from typing import Dict, List


def load_json(path: Path):
    with path.open("r", encoding="utf-8") as f:
        return json.load(f)


def make_requirement(signal: str) -> str:
    if signal.startswith("missing_prerequisite_op:"):
        op = signal.split(":", 1)[1]
        return f"Include prerequisite operation `{op}` in each relevant task."
    if signal.startswith("missing_execution_contract:"):
        field = signal.split(":", 1)[1]
        return f"Set executionContract.{field}=true for deterministic-safe tasks."
    if signal.startswith("missing_reason_keyword:"):
        key = signal.split(":", 1)[1]
        return f"Include reason text covering `{key}` risk/contract intent."
    if signal.startswith("native_task_count<"):
        target = signal.split("<", 1)[1]
        return f"Produce at least {target} native taskitems for required impact coverage."
    return f"Address missing signal: {signal}"


def main() -> int:
    p = argparse.ArgumentParser(description="Synthesize normalized requirements from prioritized raw top-gap signals.")
    p.add_argument("--top-gaps", required=True)
    p.add_argument("--max-signals", type=int, default=5)
    p.add_argument("--out", required=True)
    p.add_argument("--out-report", required=True)
    args = p.parse_args()

    backlog = load_json(Path(args.top_gaps))
    prioritized = list(backlog.get("prioritized_missing") or [])
    selected = prioritized[: max(0, args.max_signals)]

    reqs: List[Dict] = []
    selected_signals: List[Dict] = []
    for row in selected:
        signal = str(row.get("missing", ""))
        if not signal:
            continue
        reqs.append(
            {
                "requirementId": f"raw-top-gap-{len(reqs)+1}",
                "kind": "constraint",
                "normalizedText": make_requirement(signal),
                "anchor": "raw_top_gap_backlog",
                "sourceLine": 0,
                "ambiguous": False,
            }
        )
        selected_signals.append(
            {
                "missing": signal,
                "count": int(row.get("count", 0) or 0),
            }
        )

    with Path(args.out).open("w", encoding="utf-8") as f:
        json.dump(reqs, f, indent=2, sort_keys=True)
        f.write("\n")
    report = {
        "status": "ok",
        "input_signal_count": len(prioritized),
        "selected_signal_count": len(selected_signals),
        "selected_signals": selected_signals,
    }
    with Path(args.out_report).open("w", encoding="utf-8") as f:
        json.dump(report, f, indent=2, sort_keys=True)
        f.write("\n")

    print(
        json.dumps(
            {
                "status": "ok",
                "selected_signal_count": len(selected_signals),
                "out": args.out,
            },
            sort_keys=True,
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
