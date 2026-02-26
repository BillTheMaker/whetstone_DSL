#!/usr/bin/env python3
import argparse
import json
from pathlib import Path
from typing import Dict, List


def load_json(path: Path):
    with path.open("r", encoding="utf-8") as f:
        return json.load(f)


def write_json(path: Path, obj) -> None:
    with path.open("w", encoding="utf-8") as f:
        json.dump(obj, f, indent=2, sort_keys=True)
        f.write("\n")


def main() -> int:
    p = argparse.ArgumentParser(description="Build intrinsic prompt-pack requirements from top-gap missing-signal classes.")
    p.add_argument("--top-gaps", required=True)
    p.add_argument("--max-signals", type=int, default=8)
    p.add_argument("--out", required=True)
    p.add_argument("--out-report", required=True)
    args = p.parse_args()

    backlog = load_json(Path(args.top_gaps))
    prioritized = list(backlog.get("prioritized_missing") or [])[: max(0, args.max_signals)]

    required_ops: List[str] = []
    required_contracts: List[str] = []
    required_reasons: List[str] = []
    for row in prioritized:
        signal = str(row.get("missing", ""))
        if signal.startswith("missing_prerequisite_op:"):
            required_ops.append(signal.split(":", 1)[1])
        elif signal.startswith("missing_execution_contract:"):
            required_contracts.append(signal.split(":", 1)[1])
        elif signal.startswith("missing_reason_keyword:"):
            required_reasons.append(signal.split(":", 1)[1])

    required_ops = sorted(set(required_ops))
    required_contracts = sorted(set(required_contracts))
    required_reasons = sorted(set(required_reasons))

    prompt_text = (
        "intrinsic raw generation contract: emit concrete taskitems only; "
        "for each task include taskId/title/prerequisiteOps/reasons/executionContract/dependencyTaskIds; "
        f"ensure prerequisiteOps include: {', '.join(required_ops) if required_ops else 'none'}; "
        f"ensure executionContract true fields include: {', '.join(required_contracts) if required_contracts else 'deterministic,rollbackRequired,replayValidationRequired'}; "
        f"ensure reasons contain keywords: {', '.join(required_reasons) if required_reasons else 'risk,contract,replay'}; "
        "forbid umbrella tasks like 'implement everything' and require at least one verifiable operation per task."
    )

    reqs = [
        {
            "requirementId": "raw-intrinsic-prompt-pack",
            "kind": "constraint",
            "normalizedText": prompt_text,
            "anchor": "raw_intrinsic_prompt_pack",
            "sourceLine": 0,
            "ambiguous": False,
        }
    ]
    report = {
        "status": "ok",
        "selected_signal_count": len(prioritized),
        "required_ops": required_ops,
        "required_contract_fields": required_contracts,
        "required_reason_keywords": required_reasons,
    }

    write_json(Path(args.out), reqs)
    write_json(Path(args.out_report), report)
    print(json.dumps({"status": "ok", "selected_signal_count": len(prioritized), "out": args.out}, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
