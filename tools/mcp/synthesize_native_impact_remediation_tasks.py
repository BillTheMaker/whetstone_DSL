#!/usr/bin/env python3
import argparse
import json
from pathlib import Path
from typing import Dict, List


def load_json(path: Path) -> Dict:
    with path.open("r", encoding="utf-8") as f:
        return json.load(f)


def write_json(path: Path, obj) -> None:
    with path.open("w", encoding="utf-8") as f:
        json.dump(obj, f, indent=2, sort_keys=True)
        f.write("\n")


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

    reasons = [f"impact_profile:{pid}", "native_impact_remediation"] + reason_keys

    return {
        "taskId": f"impact-remediation-task-{idx+1}",
        "title": f"Impact Remediation: {pid}",
        "prerequisiteOps": req_ops,
        "reasons": reasons,
        "confidence": 0.93,
        "dependencyTaskIds": ["task-1"],
        "resourceLocks": [],
        "executionContract": ec,
    }


def main() -> int:
    p = argparse.ArgumentParser(description="Synthesize remediation tasks from failing native impact checks.")
    p.add_argument("--coverage-report", required=True)
    p.add_argument("--profiles", required=True)
    p.add_argument("--out", required=True)
    args = p.parse_args()

    coverage = load_json(Path(args.coverage_report))
    profiles_doc = load_json(Path(args.profiles))
    by_id = {str(x.get("id", "")): x for x in (profiles_doc.get("profiles") or [])}

    out: List[Dict] = []
    i = 0
    for check in coverage.get("checks", []):
      if check.get("passed", False):
        continue
      pid = str(check.get("id", ""))
      profile = by_id.get(pid, {"id": pid})
      out.append(mk_task(profile, i))
      i += 1

    write_json(Path(args.out), out)
    print(json.dumps({"status": "ok", "count": len(out), "out": args.out}, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
