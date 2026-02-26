#!/usr/bin/env python3
import argparse
import json
from pathlib import Path
from typing import Dict, List, Tuple


def load_json(path: Path) -> Dict:
    with path.open("r", encoding="utf-8") as f:
        return json.load(f)


def write_json(path: Path, obj: Dict) -> None:
    with path.open("w", encoding="utf-8") as f:
        json.dump(obj, f, indent=2, sort_keys=True)
        f.write("\n")


def load_native_tasks(run_dir: Path, summary: Dict) -> List[Dict]:
    retry = summary.get("native_decomposition_retry") or {}
    retry_applied = bool(retry.get("applied", False))
    retry_path = run_dir / "02aa_generate_taskitems_retry.json"
    base_path = run_dir / "02_generate_taskitems.json"
    if retry_applied and retry_path.exists():
        data = load_json(retry_path)
        if isinstance(data, dict):
            return list(data.get("tasks") or [])
    if base_path.exists():
        data = load_json(base_path)
        if isinstance(data, dict):
            return list(data.get("tasks") or [])
    return []


def load_spec_text(summary: Dict, repo_root: Path) -> str:
    input_file = str(summary.get("input_file", "")).strip()
    if not input_file:
        return ""
    p = Path(input_file)
    if not p.is_absolute():
        p = repo_root / input_file
    try:
        return p.read_text(encoding="utf-8", errors="ignore")
    except Exception:
        return ""


def activate_profiles(profiles: List[Dict], spec_text: str) -> List[Dict]:
    s = spec_text.lower()
    active = []
    for profile in profiles:
        keys = [str(k).lower() for k in (profile.get("trigger_keywords") or [])]
        if not keys:
            continue
        if any(k in s for k in keys):
            active.append(profile)
    return active


def collect_task_unions(tasks: List[Dict]) -> Tuple[List[str], List[str], List[Dict]]:
    reasons: List[str] = []
    prereq_ops: List[str] = []
    contracts: List[Dict] = []
    for t in tasks:
        rs = t.get("reasons") or []
        po = t.get("prerequisiteOps") or []
        ec = t.get("executionContract") or {}
        for r in rs:
            reasons.append(str(r).lower())
        for op in po:
            prereq_ops.append(str(op))
        if isinstance(ec, dict):
            contracts.append(ec)
    return reasons, prereq_ops, contracts


def check_profile(profile: Dict, tasks: List[Dict]) -> Dict:
    reasons, prereq_ops, contracts = collect_task_unions(tasks)
    missing: List[str] = []

    min_tasks = int(profile.get("min_native_task_count", 0) or 0)
    if len(tasks) < min_tasks:
        missing.append(f"native_task_count<{min_tasks}")

    req_reason_keys = [str(x).lower() for x in (profile.get("required_reason_keywords") or [])]
    for key in req_reason_keys:
        if not any(key in r for r in reasons):
            missing.append(f"missing_reason_keyword:{key}")

    req_ops = [str(x) for x in (profile.get("required_prerequisite_ops") or [])]
    ops_set = set(prereq_ops)
    for op in req_ops:
        if op not in ops_set:
            missing.append(f"missing_prerequisite_op:{op}")

    req_contract = [str(x) for x in (profile.get("required_execution_contract") or [])]
    for field in req_contract:
        has_field_true = False
        for ec in contracts:
            if isinstance(ec.get(field), bool):
                if ec.get(field) is True:
                    has_field_true = True
                    break
            elif field in ec:
                has_field_true = True
                break
        if not has_field_true:
            missing.append(f"missing_execution_contract:{field}")

    return {
        "id": profile.get("id", "unknown"),
        "description": profile.get("description", ""),
        "passed": len(missing) == 0,
        "native_task_count": len(tasks),
        "missing": missing,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description="Check native decomposition coverage against impact profiles.")
    parser.add_argument("--run-dir", required=True, help="Pipeline run directory containing 00_summary.json")
    parser.add_argument("--profiles", default="tools/mcp/profiles/native_decomposition_impact_profiles.json", help="Profile JSON path")
    parser.add_argument("--out", required=True, help="Output report path")
    parser.add_argument("--enforce", action="store_true", help="Exit nonzero on failures")
    args = parser.parse_args()

    run_dir = Path(args.run_dir)
    summary_path = run_dir / "00_summary.json"
    if not summary_path.exists():
        raise SystemExit(f"missing summary: {summary_path}")

    summary = load_json(summary_path)
    profiles_doc = load_json(Path(args.profiles))
    profiles = list(profiles_doc.get("profiles") or [])
    repo_root = Path(__file__).resolve().parents[2]
    spec_text = load_spec_text(summary, repo_root)

    active = activate_profiles(profiles, spec_text)
    tasks = load_native_tasks(run_dir, summary)

    checks = [check_profile(p, tasks) for p in active]
    failing = [c for c in checks if not c.get("passed", False)]

    result = {
        "status": "ok" if not failing else "fail",
        "run_dir": str(run_dir),
        "active_profile_count": len(active),
        "failing_profile_count": len(failing),
        "active_profiles": [p.get("id", "unknown") for p in active],
        "checks": checks,
    }
    write_json(Path(args.out), result)
    print(json.dumps({
        "status": result["status"],
        "active_profile_count": result["active_profile_count"],
        "failing_profile_count": result["failing_profile_count"],
        "out": args.out,
    }, sort_keys=True))

    if args.enforce and failing:
        return 12
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
