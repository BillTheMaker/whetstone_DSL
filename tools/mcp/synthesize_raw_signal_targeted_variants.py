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


def req_from_signal(signal: str, idx: int) -> Dict:
    if signal.startswith("missing_prerequisite_op:"):
        op = signal.split(":", 1)[1]
        txt = f"raw signal-targeted policy: include prerequisite operation `{op}` in concrete tasks where applicable."
    elif signal.startswith("missing_execution_contract:"):
        field = signal.split(":", 1)[1]
        txt = f"raw signal-targeted policy: set executionContract.{field}=true for generated tasks requiring safe deterministic execution."
    elif signal.startswith("missing_reason_keyword:"):
        key = signal.split(":", 1)[1]
        txt = f"raw signal-targeted policy: reasons must explicitly include `{key}` risk/contract intent."
    elif signal.startswith("native_task_count<"):
        min_count = signal.split("<", 1)[1]
        txt = f"raw signal-targeted policy: produce at least {min_count} concrete non-generic taskitems."
    else:
        txt = f"raw signal-targeted policy: address missing signal `{signal}` with concrete task structure."
    return {
        "requirementId": f"raw-signal-{idx+1}",
        "kind": "constraint",
        "normalizedText": txt,
        "anchor": "raw_signal_targeted_variant",
        "sourceLine": 0,
        "ambiguous": False,
    }


def build_variants(prioritized_missing: List[Dict], max_variants: int) -> List[List[Dict]]:
    if max_variants <= 0:
        return []
    reqs = [req_from_signal(str(row.get("missing", "")), i) for i, row in enumerate(prioritized_missing) if str(row.get("missing", ""))]
    if not reqs:
        return []

    variants: List[List[Dict]] = []
    # Single-signal focused variants.
    for r in reqs:
        variants.append([r])
    # Pair bundles for cross-signal coupling.
    for i in range(0, len(reqs), 2):
        variants.append(reqs[i:i + 2])
    # One all-signals compact bundle.
    variants.append(reqs[: min(6, len(reqs))])

    out: List[List[Dict]] = []
    seen = set()
    for v in variants:
        key = tuple(x.get("requirementId", "") for x in v)
        if not key or key in seen:
            continue
        seen.add(key)
        out.append(v)
        if len(out) >= max_variants:
            break
    return out


def main() -> int:
    p = argparse.ArgumentParser(description="Synthesize raw candidate variants targeted to top missing backlog signals.")
    p.add_argument("--top-gaps", required=True)
    p.add_argument("--max-variants", type=int, default=6)
    p.add_argument("--out", required=True)
    args = p.parse_args()

    backlog = load_json(Path(args.top_gaps))
    prioritized = list(backlog.get("prioritized_missing") or [])
    variants = build_variants(prioritized, args.max_variants)

    payload = {
        "status": "ok",
        "input_signal_count": len(prioritized),
        "variant_count": len(variants),
        "variants": variants,
    }
    write_json(Path(args.out), payload)
    print(json.dumps({"status": "ok", "input_signal_count": len(prioritized), "variant_count": len(variants), "out": args.out}, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
