#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
from pathlib import Path


def load_readiness(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def harden_markdown(original: str, readiness: dict) -> str:
    template = (readiness.get("recommended_template") or "").strip()
    if not template:
        return original

    missing = readiness.get("missing_actions") or []
    missing_lines = "\n".join([f"- [{m.get('type','?')}] `{m.get('key','?')}`: {m.get('action','')}" for m in missing])

    addendum = (
        "\n\n---\n\n"
        "## Planning Hardening Addendum\n\n"
        "This section is auto-generated to make the spec execution-ready for Whetstone.\n\n"
        "### Detected Gaps\n"
        f"{missing_lines if missing_lines else '- none'}\n\n"
        "### Recommended Scaffold\n\n"
        f"{template}\n"
    )

    if "## Planning Hardening Addendum" in original:
        return original
    return original.rstrip() + addendum


def main() -> None:
    ap = argparse.ArgumentParser(description="Generate hardened markdown spec scaffold from planning readiness output.")
    ap.add_argument("--spec", required=True, help="Input markdown spec path")
    ap.add_argument("--readiness", required=True, help="Readiness JSON path from spec_planning_readiness.py")
    ap.add_argument("--out", required=True, help="Output hardened markdown path")
    args = ap.parse_args()

    spec_path = Path(args.spec)
    readiness_path = Path(args.readiness)
    out_path = Path(args.out)

    original = spec_path.read_text(encoding="utf-8")
    readiness = load_readiness(readiness_path)
    hardened = harden_markdown(original, readiness)

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(hardened, encoding="utf-8")

    report = {
        "input_spec": str(spec_path),
        "readiness_json": str(readiness_path),
        "output_spec": str(out_path),
        "missing_count": len(readiness.get("missing_actions") or []),
        "added_addendum": hardened != original,
    }
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
