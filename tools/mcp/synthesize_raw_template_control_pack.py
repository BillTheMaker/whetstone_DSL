#!/usr/bin/env python3
import argparse
import json
from pathlib import Path
from typing import List


def load_json(path: Path):
    with path.open("r", encoding="utf-8") as f:
        return json.load(f)


def write_json(path: Path, obj) -> None:
    with path.open("w", encoding="utf-8") as f:
        json.dump(obj, f, indent=2, sort_keys=True)
        f.write("\n")


def main() -> int:
    p = argparse.ArgumentParser(description="Synthesize raw task template control requirements from top-gap signals.")
    p.add_argument("--top-gaps", required=True)
    p.add_argument("--max-signals", type=int, default=8)
    p.add_argument("--out", required=True)
    p.add_argument("--out-report", required=True)
    args = p.parse_args()

    backlog = load_json(Path(args.top_gaps))
    prioritized = list(backlog.get("prioritized_missing") or [])[: max(0, args.max_signals)]

    signals: List[str] = [str(r.get("missing", "")) for r in prioritized if str(r.get("missing", ""))]
    template_example = {
        "taskId": "task-example-1",
        "title": "Concrete scoped operation",
        "prerequisiteOps": ["whetstone_generate_taskitems", "whetstone_queue_ready", "whetstone_validate_taskitem"],
        "reasons": ["risk", "contract", "replay"],
        "dependencyTaskIds": [],
        "executionContract": {
            "deterministic": True,
            "rollbackRequired": True,
            "replayValidationRequired": True,
        },
    }
    schema_text = (
        "output-shape control: every task must include non-empty taskId/title/prerequisiteOps/reasons/executionContract; "
        "executionContract must explicitly set deterministic, rollbackRequired, replayValidationRequired to true; "
        "task must include at least one concrete prerequisite op and no umbrella placeholder wording."
    )
    example_text = f"required template example: {json.dumps(template_example, sort_keys=True)}"
    signal_text = (
        "missing-signal alignment: " + (", ".join(signals) if signals else "no signals") +
        "; generated tasks must explicitly close these classes where applicable."
    )

    reqs = [
        {
            "requirementId": "raw-template-control-schema",
            "kind": "constraint",
            "normalizedText": schema_text,
            "anchor": "raw_template_control",
            "sourceLine": 0,
            "ambiguous": False,
        },
        {
            "requirementId": "raw-template-control-example",
            "kind": "constraint",
            "normalizedText": example_text,
            "anchor": "raw_template_control",
            "sourceLine": 0,
            "ambiguous": False,
        },
        {
            "requirementId": "raw-template-control-signals",
            "kind": "constraint",
            "normalizedText": signal_text,
            "anchor": "raw_template_control",
            "sourceLine": 0,
            "ambiguous": False,
        },
    ]
    report = {
        "status": "ok",
        "selected_signal_count": len(signals),
        "signals": signals,
        "requirement_count": len(reqs),
    }
    write_json(Path(args.out), reqs)
    write_json(Path(args.out_report), report)
    print(json.dumps({"status": "ok", "selected_signal_count": len(signals), "requirement_count": len(reqs), "out": args.out}, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
