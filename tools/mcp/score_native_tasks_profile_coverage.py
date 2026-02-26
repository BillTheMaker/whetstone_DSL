#!/usr/bin/env python3
import argparse
import json
from pathlib import Path
from typing import Dict, List, Optional, Tuple


def load_json(path: Path):
    with path.open("r", encoding="utf-8") as f:
        return json.load(f)


def activate_profiles(profiles: List[Dict], spec_text: str) -> List[Dict]:
    s = spec_text.lower()
    out = []
    for p in profiles:
        keys = [str(k).lower() for k in (p.get("trigger_keywords") or [])]
        if keys and any(k in s for k in keys):
            out.append(p)
    return out


def collect_task_unions(tasks: List[Dict]):
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


def parse_missing_signal(signal: str) -> Tuple[str, str]:
    if ":" not in signal:
        return signal, ""
    k, v = signal.split(":", 1)
    return k, v


def signal_present(signal: str, tasks: List[Dict]) -> bool:
    if signal.startswith("native_task_count<"):
        try:
            min_count = int(signal.split("<", 1)[1])
        except (ValueError, IndexError):
            return False
        return len(tasks) >= min_count

    kind, value = parse_missing_signal(signal)
    reasons, prereq_ops, contracts = collect_task_unions(tasks)

    if kind == "missing_prerequisite_op":
        return value in set(prereq_ops)
    if kind == "missing_execution_contract":
        for ec in contracts:
            if isinstance(ec.get(value), bool):
                if ec.get(value):
                    return True
            elif value in ec:
                return True
        return False
    if kind == "missing_reason_keyword":
        v = value.lower()
        return any(v in r for r in reasons)
    return False


def load_top_gap_items(path: Optional[Path]) -> List[Dict]:
    if path is None or not path.exists():
        return []
    data = load_json(path)
    rows = list(data.get("prioritized_missing") or [])
    out: List[Dict] = []
    for row in rows:
        signal = str(row.get("missing", ""))
        if not signal:
            continue
        weight = int(row.get("count", 1) or 1)
        out.append({"missing": signal, "weight": weight})
    return out


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

    return {"id": profile.get("id", "unknown"), "passed": len(missing) == 0, "missing": missing}


def main() -> int:
    p = argparse.ArgumentParser(description="Score raw task set against active impact profiles.")
    p.add_argument("--spec", required=True)
    p.add_argument("--profiles", required=True)
    p.add_argument("--tasks", required=True)
    p.add_argument("--out", required=True)
    p.add_argument("--top-gaps", default="", help="Optional raw gap backlog JSON with prioritized_missing.")
    args = p.parse_args()

    spec_text = Path(args.spec).read_text(encoding="utf-8", errors="ignore")
    profiles_doc = load_json(Path(args.profiles))
    profiles = list(profiles_doc.get("profiles") or [])
    active = activate_profiles(profiles, spec_text)

    tasks = load_json(Path(args.tasks))
    if not isinstance(tasks, list):
        raise SystemExit("--tasks must be JSON array")

    checks = [check_profile(pf, tasks) for pf in active]
    failing = [c for c in checks if not c.get("passed", False)]
    top_gap_items = load_top_gap_items(Path(args.top_gaps) if args.top_gaps else None)
    top_gap_signal_hits = 0
    top_gap_score = 0
    top_gap_signal_weight_total = 0
    for item in top_gap_items:
        sig = str(item.get("missing", ""))
        w = int(item.get("weight", 1) or 1)
        top_gap_signal_weight_total += w
        if signal_present(sig, tasks):
            top_gap_signal_hits += 1
            top_gap_score += w

    result = {
        "status": "ok" if not failing else "fail",
        "task_count": len(tasks),
        "active_profile_count": len(active),
        "failing_profile_count": len(failing),
        "top_gap_signal_count": len(top_gap_items),
        "top_gap_signal_hits": top_gap_signal_hits,
        "top_gap_signal_weight_total": top_gap_signal_weight_total,
        "top_gap_score": top_gap_score,
        "checks": checks,
    }
    with Path(args.out).open("w", encoding="utf-8") as f:
        json.dump(result, f, indent=2, sort_keys=True)
        f.write("\n")

    print(
        json.dumps(
            {
                "status": result["status"],
                "task_count": result["task_count"],
                "failing_profile_count": result["failing_profile_count"],
                "top_gap_score": result["top_gap_score"],
                "out": args.out,
            },
            sort_keys=True,
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
