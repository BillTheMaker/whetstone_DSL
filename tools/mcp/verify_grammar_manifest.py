#!/usr/bin/env python3
"""
Verify grammar manifest and lock digest against current schema/artifacts.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import sys
from pathlib import Path


def _sha(text: str) -> str:
    return hashlib.sha256(text.encode()).hexdigest()


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--schemas", default="tools/mcp/whetstone_tool_schemas.json")
    ap.add_argument("--grammars-dir", default="tools/mcp/grammars")
    args = ap.parse_args()

    gdir = Path(args.grammars_dir)
    manifest_path = gdir / "manifest.json"
    lock_path = gdir / "manifest.lock"
    dispatch_gbnf_path = gdir / "dispatch.gbnf"
    dispatch_schema_path = gdir / "dispatch_schema.json"

    if not manifest_path.exists() or not lock_path.exists():
        print("Manifest files missing")
        sys.exit(2)

    manifest = json.loads(manifest_path.read_text())
    schemas = json.loads(Path(args.schemas).read_text())
    dispatch_gbnf = dispatch_gbnf_path.read_text()
    dispatch_schema = json.loads(dispatch_schema_path.read_text())

    expected = {
        "tool_count": len(schemas),
        "tool_schema_sha256": _sha(json.dumps(schemas, sort_keys=True, separators=(",", ":"))),
        "dispatch_gbnf_sha256": _sha(dispatch_gbnf),
        "dispatch_schema_sha256": _sha(json.dumps(dispatch_schema, sort_keys=True, separators=(",", ":"))),
    }

    failures = []
    for k, v in expected.items():
        if manifest.get(k) != v:
            failures.append(f"manifest mismatch: {k}")

    expected_lock = _sha(json.dumps(manifest, sort_keys=True, separators=(",", ":")))
    actual_lock = lock_path.read_text().strip()
    if expected_lock != actual_lock:
        failures.append("manifest lock mismatch")

    if failures:
        print("Manifest verification FAILED")
        for f in failures:
            print(f"- {f}")
        sys.exit(2)

    print("Manifest verification PASSED")


if __name__ == "__main__":
    main()
