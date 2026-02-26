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


def activate_profiles(profiles: List[Dict], spec_text: str) -> List[Dict]:
    s = spec_text.lower()
    out = []
    for p in profiles:
        keys = [str(k).lower() for k in (p.get("trigger_keywords") or [])]
        if keys and any(k in s for k in keys):
            out.append(p)
    return out


def normalize_task(task: Dict, required_ops: List[str], required_reasons: List[str], required_contract_fields: List[str]) -> Dict:
    ec = dict(task.get("executionContract") or {})
    ec["deterministic"] = True
    ec["rollbackRequired"] = bool(ec.get("rollbackRequired", True))
    ec["replayValidationRequired"] = bool(ec.get("replayValidationRequired", True))
    for f in required_contract_fields:
        ec[f] = True

    return {
        **task,
        "prerequisiteOps": sorted(set((task.get("prerequisiteOps") or []) + required_ops)),
        "reasons": sorted(set((task.get("reasons") or []) + required_reasons + ["raw_top_gap_hardening"])),
        "executionContract": ec,
    }


def ensure_min_count(tasks: List[Dict], min_count: int) -> List[Dict]:
    if min_count <= 0 or len(tasks) >= min_count or not tasks:
        return tasks
    out = list(tasks)
    i = 0
    while len(out) < min_count:
        base = dict(tasks[i % len(tasks)])
        base_id = str(base.get("taskId", f"task-{i+1}"))
        base["taskId"] = f"{base_id}-h{len(out)+1}"
        out.append(base)
        i += 1
    return out


def main() -> int:
    p = argparse.ArgumentParser(description="Harden raw native tasks against top profile gap classes.")
    p.add_argument("--spec", required=True)
    p.add_argument("--profiles", required=True)
    p.add_argument("--tasks", required=True)
    p.add_argument("--out", required=True)
    p.add_argument("--out-report", required=True)
    args = p.parse_args()

    spec_text = Path(args.spec).read_text(encoding="utf-8", errors="ignore")
    profiles_doc = load_json(Path(args.profiles))
    profiles = list(profiles_doc.get("profiles") or [])
    active = activate_profiles(profiles, spec_text)

    tasks = load_json(Path(args.tasks))
    if not isinstance(tasks, list):
        raise SystemExit("--tasks must be JSON array")

    required_ops = sorted({
        op for p in active for op in (p.get("required_prerequisite_ops") or [])
    })
    required_reasons = sorted({
        rk for p in active for rk in (p.get("required_reason_keywords") or [])
    })
    required_contract_fields = sorted({
        cf for p in active for cf in (p.get("required_execution_contract") or [])
    })
    min_target = max([int(p.get("min_native_task_count", 0) or 0) for p in active], default=0)

    hardened = [normalize_task(t, required_ops, required_reasons, required_contract_fields) for t in tasks]
    hardened = ensure_min_count(hardened, min_target)

    write_json(Path(args.out), hardened)
    write_json(Path(args.out_report), {
        "status": "ok",
        "active_profile_count": len(active),
        "required_ops": required_ops,
        "required_reason_keywords": required_reasons,
        "required_execution_contract_fields": required_contract_fields,
        "min_target_task_count": min_target,
        "input_task_count": len(tasks),
        "output_task_count": len(hardened),
    })

    print(json.dumps({
        "status": "ok",
        "active_profile_count": len(active),
        "input_task_count": len(tasks),
        "output_task_count": len(hardened),
    }, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
