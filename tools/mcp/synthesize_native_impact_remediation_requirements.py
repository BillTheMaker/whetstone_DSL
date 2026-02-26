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


def make_requirement(profile: Dict, missing: List[str], idx: int) -> Dict:
    pid = str(profile.get("id", f"profile_{idx}"))
    req_ops = profile.get("required_prerequisite_ops") or []
    req_contract = profile.get("required_execution_contract") or []
    reason_keys = profile.get("required_reason_keywords") or []
    min_tasks = int(profile.get("min_native_task_count", 0) or 0)

    text = (
        f"native decomposition remediation for {pid}: "
        f"produce at least {min_tasks} concrete taskitems; "
        f"ensure prerequisiteOps include {', '.join(req_ops) if req_ops else 'none'}; "
        f"ensure executionContract includes {', '.join(req_contract) if req_contract else 'none'}; "
        f"ensure reasons include keywords {', '.join(reason_keys) if reason_keys else 'none'}; "
        f"close missing checks: {', '.join(missing) if missing else 'none'}."
    )

    return {
        "requirementId": f"native-impact-remediation-{pid}",
        "kind": "constraint",
        "normalizedText": text,
        "anchor": "native_impact_coverage_remediation",
        "sourceLine": 0,
        "ambiguous": False,
    }


def main() -> int:
    p = argparse.ArgumentParser(description="Synthesize extra normalized requirements from failing native impact coverage checks.")
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
        missing = [str(m) for m in (check.get("missing") or [])]
        out.append(make_requirement(profile, missing, i))
        i += 1

    write_json(Path(args.out), out)
    print(json.dumps({"status": "ok", "count": len(out), "out": args.out}, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
