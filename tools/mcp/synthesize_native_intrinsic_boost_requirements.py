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


def mk_req(profile: Dict, idx: int) -> Dict:
    pid = str(profile.get("id", f"profile_{idx}"))
    min_tasks = int(profile.get("min_native_task_count", 0) or 0)
    reason_keys = [str(x) for x in (profile.get("required_reason_keywords") or [])]
    ops = [str(x) for x in (profile.get("required_prerequisite_ops") or [])]
    contract = [str(x) for x in (profile.get("required_execution_contract") or [])]
    text = (
        f"intrinsic decomposition policy for {pid}: produce at least {min_tasks} concrete taskitems; "
        f"ensure reasons include {', '.join(reason_keys) if reason_keys else 'none'}; "
        f"ensure prerequisiteOps include {', '.join(ops) if ops else 'none'}; "
        f"ensure executionContract includes {', '.join(contract) if contract else 'none'}."
    )
    return {
        "requirementId": f"intrinsic-boost-{pid}",
        "kind": "constraint",
        "normalizedText": text,
        "anchor": "native_intrinsic_boost",
        "sourceLine": 0,
        "ambiguous": False,
    }


def main() -> int:
    p = argparse.ArgumentParser(description="Synthesize intrinsic decomposition boost requirements from active impact profiles.")
    p.add_argument("--spec", required=True)
    p.add_argument("--profiles", required=True)
    p.add_argument("--out", required=True)
    args = p.parse_args()

    spec_text = Path(args.spec).read_text(encoding="utf-8", errors="ignore")
    profiles_doc = load_json(Path(args.profiles))
    profiles = list(profiles_doc.get("profiles") or [])
    active = activate_profiles(profiles, spec_text)

    reqs = [mk_req(pf, i) for i, pf in enumerate(active)]
    write_json(Path(args.out), reqs)
    print(json.dumps({"status": "ok", "active_profile_count": len(active), "requirement_count": len(reqs), "out": args.out}, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
