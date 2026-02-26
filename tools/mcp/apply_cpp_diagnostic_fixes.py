#!/usr/bin/env python3
"""Apply deterministic C++ source fixes from gate diagnostics."""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path
from typing import List, Set

KNOWN_STL_HEADERS = {"vector", "map", "set", "optional", "tuple", "memory", "string", "algorithm"}
SYMBOL_TO_HEADER = {
    "std::vector": "vector",
    "std::map": "map",
    "std::set": "set",
    "std::optional": "optional",
    "std::tuple": "tuple",
    "std::shared_ptr": "memory",
    "std::unique_ptr": "memory",
}


def parse_headers_from_stderr(stderr: str) -> Set[str]:
    out: Set[str] = set()
    for m in re.finditer(r"header [‘']<([^>]+)>[’']", stderr):
        h = m.group(1).strip()
        if h in KNOWN_STL_HEADERS:
            out.add(h)
    return out


def parse_headers_from_code_symbols(code: str) -> Set[str]:
    out: Set[str] = set()
    for symbol, header in SYMBOL_TO_HEADER.items():
        if symbol in code:
            out.add(header)
    return out


def existing_headers(lines: List[str]) -> Set[str]:
    out: Set[str] = set()
    for line in lines:
        m = re.match(r"\s*#include\s*<([^>]+)>", line)
        if m:
            out.add(m.group(1).strip())
    return out


def insert_headers(code: str, headers_to_add: Set[str]) -> str:
    if not headers_to_add:
        return code
    lines = code.splitlines()
    have = existing_headers(lines)
    needed = sorted(h for h in headers_to_add if h not in have)
    if not needed:
        return code

    insert_at = 0
    for i, line in enumerate(lines):
        if line.strip().startswith("#include"):
            insert_at = i + 1
    include_lines = [f"#include <{h}>" for h in needed]
    new_lines = lines[:insert_at] + include_lines + lines[insert_at:]
    return "\n".join(new_lines) + ("\n" if code.endswith("\n") else "")


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--code-file", required=True)
    ap.add_argument("--gates-json", required=True)
    ap.add_argument("--out-code-file", required=True)
    ap.add_argument("--out-report", default="")
    args = ap.parse_args()

    code_path = Path(args.code_file)
    gates_path = Path(args.gates_json)
    code = code_path.read_text()
    gates = json.loads(gates_path.read_text()) if gates_path.exists() else {}

    compile_stderr = str(((gates.get("gates") or {}).get("compile") or {}).get("stderr", ""))
    test_stderr = str(((gates.get("gates") or {}).get("tests") or {}).get("stderr", ""))

    diag_headers = parse_headers_from_stderr(compile_stderr + "\n" + test_stderr)
    symbol_headers = parse_headers_from_code_symbols(code)
    headers = diag_headers | symbol_headers

    fixed = insert_headers(code, headers)
    Path(args.out_code_file).write_text(fixed)

    report = {
        "changed": fixed != code,
        "headers_requested": sorted(headers),
        "headers_from_diagnostics": sorted(diag_headers),
        "headers_from_symbols": sorted(symbol_headers),
    }
    if args.out_report:
        Path(args.out_report).write_text(json.dumps(report, indent=2, sort_keys=True) + "\n")
    print(json.dumps(report, indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
