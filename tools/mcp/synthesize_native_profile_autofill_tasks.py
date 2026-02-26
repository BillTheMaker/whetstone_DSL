#!/usr/bin/env python3
import argparse
import json
from pathlib import Path
from typing import Dict, List, Tuple


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


def collect_task_unions(tasks: List[Dict]) -> Tuple[List[str], List[str], List[Dict]]:
    reasons: List[str] = []
    prereq_ops: List[str] = []
    contracts: List[Dict] = []
    for t in tasks:
        for r in (t.get("reasons") or []):
            reasons.append(str(r).lower())
        for op in (t.get("prerequisiteOps") or []):
            prereq_ops.append(str(op))
        ec = t.get("executionContract") or {}
        if isinstance(ec, dict):
            contracts.append(ec)
    return reasons, prereq_ops, contracts


def check_profile(profile: Dict, tasks: List[Dict]) -> Dict:
    reasons, prereq_ops, contracts = collect_task_unions(tasks)
    missing: List[str] = []

    min_tasks = int(profile.get("min_native_task_count", 0) or 0)
    if len(tasks) < min_tasks:
        missing.append(f"native_task_count<{min_tasks}")

    for key in [str(x).lower() for x in (profile.get("required_reason_keywords") or [])]:
        if not any(key in r for r in reasons):
            missing.append(f"missing_reason_keyword:{key}")

    ops_set = set(prereq_ops)
    for op in [str(x) for x in (profile.get("required_prerequisite_ops") or [])]:
        if op not in ops_set:
            missing.append(f"missing_prerequisite_op:{op}")

    for field in [str(x) for x in (profile.get("required_execution_contract") or [])]:
        ok = False
        for ec in contracts:
            if isinstance(ec.get(field), bool):
                if ec.get(field):
                    ok = True
                    break
            elif field in ec:
                ok = True
                break
        if not ok:
            missing.append(f"missing_execution_contract:{field}")

    return {
        "id": profile.get("id", "unknown"),
        "passed": len(missing) == 0,
        "missing": missing,
    }


def mk_task(profile: Dict, idx: int) -> Dict:
    pid = str(profile.get("id", f"profile_{idx}"))
    req_ops = [str(x) for x in (profile.get("required_prerequisite_ops") or [])]
    reason_keys = [str(x) for x in (profile.get("required_reason_keywords") or [])]
    req_contract = [str(x) for x in (profile.get("required_execution_contract") or [])]

    ec = {
        "deterministic": True,
        "rollbackRequired": True,
        "replayValidationRequired": True,
        "maxFileTouches": 4,
        "maxContextTokens": 6144,
        "maxExecutionSteps": 8,
        "executionSpecificityScore": 90,
    }
    for field in req_contract:
        ec[field] = True

    return {
        "taskId": f"profile-autofill-task-{idx+1}",
        "title": f"Profile Autofill: {pid}",
        "prerequisiteOps": req_ops,
        "reasons": [f"impact_profile:{pid}", "native_profile_autofill"] + reason_keys,
        "confidence": 0.93,
        "dependencyTaskIds": ["task-1"],
        "resourceLocks": [],
        "executionContract": ec,
    }


def main() -> int:
    p = argparse.ArgumentParser(description="Synthesize first-pass profile autofill tasks for active failing impact profiles.")
    p.add_argument("--spec", required=True)
    p.add_argument("--tasks", required=True)
    p.add_argument("--profiles", required=True)
    p.add_argument("--out-tasks", required=True)
    p.add_argument("--out-report", required=True)
    p.add_argument("--max-tasks", type=int, default=12)
    args = p.parse_args()

    spec_text = Path(args.spec).read_text(encoding="utf-8", errors="ignore")
    tasks = load_json(Path(args.tasks))
    if not isinstance(tasks, list):
        raise SystemExit("--tasks must be a JSON array")

    profiles_doc = load_json(Path(args.profiles))
    profiles = list(profiles_doc.get("profiles") or [])
    active = activate_profiles(profiles, spec_text)

    checks = [check_profile(pf, tasks) for pf in active]
    failing_ids = [c["id"] for c in checks if not c.get("passed", False)]
    by_id = {str(p.get("id", "")): p for p in active}

    out_tasks: List[Dict] = []
    for i, pid in enumerate(failing_ids[: max(0, args.max_tasks)]):
        out_tasks.append(mk_task(by_id.get(pid, {"id": pid}), i))

    report = {
        "status": "ok",
        "active_profile_count": len(active),
        "failing_profile_count": len(failing_ids),
        "autofill_task_count": len(out_tasks),
        "checks": checks,
    }

    write_json(Path(args.out_tasks), out_tasks)
    write_json(Path(args.out_report), report)
    print(json.dumps({
        "status": "ok",
        "active_profile_count": len(active),
        "failing_profile_count": len(failing_ids),
        "autofill_task_count": len(out_tasks),
    }, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
