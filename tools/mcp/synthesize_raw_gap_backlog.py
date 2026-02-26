#!/usr/bin/env python3
import argparse
import json
from collections import Counter, defaultdict
from pathlib import Path
from typing import Dict, List


def load_json(path: Path):
    with path.open("r", encoding="utf-8") as f:
        return json.load(f)


def main() -> int:
    p = argparse.ArgumentParser(description="Synthesize prioritized raw-generator gap backlog from closure ladder outputs.")
    p.add_argument("--batch-dir", required=True)
    p.add_argument("--out-json", required=True)
    p.add_argument("--out-md", required=True)
    args = p.parse_args()

    batch_dir = Path(args.batch_dir)
    ladder_files = sorted(batch_dir.glob("*/closure_ladder_summary.json"))

    mode_counts = Counter()
    missing_counter = Counter()
    by_profile_missing = defaultdict(Counter)
    spec_rows: List[Dict] = []

    for lf in ladder_files:
        try:
            summary = load_json(lf)
        except Exception:
            continue
        mode = str(summary.get("selected_mode", ""))
        mode_counts[mode] += 1

        selected_run = str(summary.get("selected_run", ""))
        cov_checks = []
        raw_run = ""
        raw_rc = ""
        # Prefer raw-only attempt artifacts for gap mining.
        raw_run_file = lf.parent / "raw_only.run_dir"
        raw_rc_file = lf.parent / "raw_only.rc"
        if raw_run_file.exists():
            raw_run = raw_run_file.read_text(encoding="utf-8", errors="ignore").strip()
        if raw_rc_file.exists():
            raw_rc = raw_rc_file.read_text(encoding="utf-8", errors="ignore").strip()
        target_run = raw_run if raw_run else selected_run
        if target_run:
            sp = Path(target_run) / "00_summary.json"
            if sp.exists():
                try:
                    s = load_json(sp)
                    cov = (s.get("native_impact_coverage") or {})
                    cov_checks = list(cov.get("checks") or [])
                except Exception:
                    cov_checks = []

        missing_for_spec = []
        for c in cov_checks:
            pid = str(c.get("id", "unknown"))
            for m in (c.get("missing") or []):
                m = str(m)
                missing_counter[m] += 1
                by_profile_missing[pid][m] += 1
                missing_for_spec.append({"profile": pid, "missing": m})

        spec_rows.append(
            {
                "run_id": lf.parent.name,
                "selected_mode": mode,
                "selected_run": selected_run,
                "raw_only_run": raw_run,
                "raw_only_rc": raw_rc,
                "missing_items": missing_for_spec,
            }
        )

    prioritized_missing = [
        {"missing": k, "count": int(v)}
        for k, v in missing_counter.most_common()
    ]

    profile_priorities = {}
    for pid, ctr in by_profile_missing.items():
        profile_priorities[pid] = [
            {"missing": k, "count": int(v)} for k, v in ctr.most_common()
        ]

    out = {
        "status": "ok",
        "record_count": len(spec_rows),
        "selected_mode_counts": dict(mode_counts),
        "prioritized_missing": prioritized_missing,
        "profile_priorities": profile_priorities,
        "records": spec_rows,
    }

    with Path(args.out_json).open("w", encoding="utf-8") as f:
        json.dump(out, f, indent=2, sort_keys=True)
        f.write("\n")

    with Path(args.out_md).open("w", encoding="utf-8") as f:
        f.write("# Raw Generator Gap Backlog\n\n")
        f.write(f"- Record count: {len(spec_rows)}\n")
        f.write("- Selected mode counts:\n")
        for mode, count in mode_counts.items():
            f.write(f"  - {mode}: {count}\n")
        f.write("\n## Prioritized Missing Signals\n\n")
        if not prioritized_missing:
            f.write("- None\n")
        else:
            for row in prioritized_missing:
                f.write(f"- {row['missing']}: {row['count']}\n")

    print(json.dumps({"status": "ok", "record_count": len(spec_rows), "out_json": args.out_json, "out_md": args.out_md}, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
