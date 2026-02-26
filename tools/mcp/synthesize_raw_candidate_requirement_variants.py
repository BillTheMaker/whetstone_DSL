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


def req_from_profile(p: Dict, idx: int) -> Dict:
    pid = str(p.get("id", f"profile_{idx}"))
    min_tasks = int(p.get("min_native_task_count", 0) or 0)
    reasons = [str(x) for x in (p.get("required_reason_keywords") or [])]
    ops = [str(x) for x in (p.get("required_prerequisite_ops") or [])]
    contract = [str(x) for x in (p.get("required_execution_contract") or [])]
    text = (
        f"raw candidate policy {pid}: produce >= {min_tasks} concrete taskitems; "
        f"reasons include {', '.join(reasons) if reasons else 'none'}; "
        f"ops include {', '.join(ops) if ops else 'none'}; "
        f"execution contract include {', '.join(contract) if contract else 'none'}; "
        "avoid generic or umbrella tasks."
    )
    return {
        "requirementId": f"raw-variant-{pid}",
        "kind": "constraint",
        "normalizedText": text,
        "anchor": "raw_candidate_variant",
        "sourceLine": 0,
        "ambiguous": False,
    }


def global_decomposition_req(active_count: int, min_target: int) -> Dict:
    return {
        "requirementId": "raw-variant-global-decomposition",
        "kind": "constraint",
        "normalizedText": (
            f"raw decomposition hard policy: output at least {min_target} concrete taskitems "
            f"for {active_count} active impact profiles; each task must have deterministic executionContract and explicit prerequisiteOps."
        ),
        "anchor": "raw_candidate_variant",
        "sourceLine": 0,
        "ambiguous": False,
    }


def build_variants(active: List[Dict], max_variants: int) -> List[List[Dict]]:
    if max_variants <= 0:
        return []

    profile_reqs = [req_from_profile(p, i) for i, p in enumerate(active)]
    min_target = max(5, len(active) + 2)
    g = global_decomposition_req(len(active), min_target)

    variants: List[List[Dict]] = []

    # Variant 1: global only
    variants.append([g])

    # Variant 2: global + all profiles
    variants.append([g] + profile_reqs)

    # Variant 3+: one profile at a time + global
    for r in profile_reqs:
        variants.append([g, r])

    # Variant: paired profile bundles
    for i in range(0, len(profile_reqs), 2):
        bundle = [g] + profile_reqs[i:i+2]
        variants.append(bundle)

    # Deduplicate by requirementId tuple ordering
    seen = set()
    out: List[List[Dict]] = []
    for v in variants:
        key = tuple(x.get("requirementId", "") for x in v)
        if key in seen:
            continue
        seen.add(key)
        out.append(v)
        if len(out) >= max_variants:
            break
    return out


def main() -> int:
    p = argparse.ArgumentParser(description="Synthesize raw candidate requirement variants from active profiles.")
    p.add_argument("--spec", required=True)
    p.add_argument("--profiles", required=True)
    p.add_argument("--max-variants", type=int, default=8)
    p.add_argument("--out", required=True)
    args = p.parse_args()

    spec_text = Path(args.spec).read_text(encoding="utf-8", errors="ignore")
    profiles_doc = load_json(Path(args.profiles))
    profiles = list(profiles_doc.get("profiles") or [])
    active = activate_profiles(profiles, spec_text)

    variants = build_variants(active, args.max_variants)
    payload = {
        "status": "ok",
        "active_profile_count": len(active),
        "variant_count": len(variants),
        "variants": variants,
    }
    write_json(Path(args.out), payload)
    print(json.dumps({"status": "ok", "active_profile_count": len(active), "variant_count": len(variants), "out": args.out}, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
