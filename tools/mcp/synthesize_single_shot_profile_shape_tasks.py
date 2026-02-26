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


def apply_profile(task: Dict, profile: Dict, idx: int) -> Dict:
    pid = str(profile.get("id", f"profile_{idx}"))
    ops = [str(x) for x in (profile.get("required_prerequisite_ops") or [])]
    reasons = [str(x) for x in (profile.get("required_reason_keywords") or [])]
    contracts = [str(x) for x in (profile.get("required_execution_contract") or [])]

    ec = dict(task.get("executionContract") or {})
    ec["deterministic"] = True
    ec["rollbackRequired"] = bool(ec.get("rollbackRequired", True))
    ec["replayValidationRequired"] = bool(ec.get("replayValidationRequired", True))
    for f in contracts:
        ec[f] = True

    return {
        **task,
        "taskId": f"{task.get('taskId', f'task-{idx+1}')}-ss-{pid.replace('-', '_')}-{idx+1}",
        "prerequisiteOps": sorted(set((task.get("prerequisiteOps") or []) + ops)),
        "reasons": sorted(set((task.get("reasons") or []) + reasons + ["single_shot_profile_shape", f"impact_profile:{pid}"])),
        "executionContract": ec,
    }


def main() -> int:
    p = argparse.ArgumentParser(description="Shape single-shot native tasks to satisfy active profile contracts.")
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

    base_tasks = load_json(Path(args.tasks))
    if not isinstance(base_tasks, list):
        raise SystemExit("--tasks must be JSON array")

    shaped: List[Dict] = list(base_tasks)
    if base_tasks and active:
        for i, pf in enumerate(active):
            base = base_tasks[i % len(base_tasks)]
            shaped.append(apply_profile(base, pf, i))

    write_json(Path(args.out), shaped)
    write_json(Path(args.out_report), {
        "status": "ok",
        "base_task_count": len(base_tasks),
        "active_profile_count": len(active),
        "shaped_task_count": len(shaped),
    })
    print(json.dumps({"status": "ok", "base_task_count": len(base_tasks), "active_profile_count": len(active), "shaped_task_count": len(shaped)}, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
