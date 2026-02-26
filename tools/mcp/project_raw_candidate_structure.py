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


def parse_gap_signals(backlog: Dict):
    missing = [str(x.get("missing", "")) for x in (backlog.get("prioritized_missing") or [])]
    ops = sorted({m.split(":", 1)[1] for m in missing if m.startswith("missing_prerequisite_op:")})
    reasons = sorted({m.split(":", 1)[1] for m in missing if m.startswith("missing_reason_keyword:")})
    contracts = sorted({m.split(":", 1)[1] for m in missing if m.startswith("missing_execution_contract:")})
    min_count = 0
    for m in missing:
        if m.startswith("native_task_count<"):
            try:
                min_count = max(min_count, int(m.split("<", 1)[1]))
            except ValueError:
                pass
    return ops, reasons, contracts, min_count


def normalize_task(task: Dict, ops: List[str], reasons: List[str], contracts: List[str]) -> Dict:
    ec = dict(task.get("executionContract") or {})
    for f in contracts:
        ec[f] = True
    ec.setdefault("deterministic", True)
    ec.setdefault("rollbackRequired", True)
    ec.setdefault("replayValidationRequired", True)
    title = str(task.get("title", "Task")).strip() or "Task"
    return {
        **task,
        "title": title,
        "prerequisiteOps": sorted(set((task.get("prerequisiteOps") or []) + ops)),
        "reasons": sorted(set((task.get("reasons") or []) + reasons + ["raw_structural_projection"])),
        "executionContract": ec,
    }


def expand_to_min_count(tasks: List[Dict], min_count: int, ops: List[str]) -> List[Dict]:
    if min_count <= 0 or len(tasks) >= min_count or not tasks:
        return tasks
    out = list(tasks)
    idx = 0
    while len(out) < min_count:
        base = dict(tasks[idx % len(tasks)])
        tid = str(base.get("taskId", f"task-{idx+1}"))
        op = ops[idx % len(ops)] if ops else "whetstone_validate_taskitem"
        base["taskId"] = f"{tid}-p{len(out)+1}"
        base["title"] = f"{base.get('title','Task')} [{op}]"
        base["prerequisiteOps"] = sorted(set((base.get("prerequisiteOps") or []) + [op]))
        out.append(base)
        idx += 1
    return out


def main() -> int:
    p = argparse.ArgumentParser(description="Project raw candidate tasks into stronger structural shape before scoring.")
    p.add_argument("--tasks", required=True)
    p.add_argument("--top-gaps", required=True)
    p.add_argument("--out", required=True)
    p.add_argument("--out-report", required=True)
    args = p.parse_args()

    tasks = load_json(Path(args.tasks))
    if not isinstance(tasks, list):
        raise SystemExit("--tasks must be JSON array")
    backlog = load_json(Path(args.top_gaps))
    ops, reasons, contracts, min_count = parse_gap_signals(backlog)

    projected = [normalize_task(t, ops, reasons, contracts) for t in tasks]
    projected = expand_to_min_count(projected, min_count, ops)

    report = {
        "status": "ok",
        "input_task_count": len(tasks),
        "output_task_count": len(projected),
        "required_ops_count": len(ops),
        "required_reason_count": len(reasons),
        "required_contract_count": len(contracts),
        "target_min_task_count": min_count,
    }
    write_json(Path(args.out), projected)
    write_json(Path(args.out_report), report)
    print(json.dumps({"status": "ok", "input_task_count": len(tasks), "output_task_count": len(projected), "out": args.out}, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
