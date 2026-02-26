#!/usr/bin/env python3
"""
Evaluate production gates for generated code.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import tempfile
from pathlib import Path
from typing import Dict, List


PLACEHOLDER_PATTERNS = [
    r"\bTODO\b",
    r"\bFIXME\b",
    r"\bplaceholder\b",
    r"auto\s*/\*\s*TODO",
    r"/\*\s*missing",
]


def detect_placeholders(code: str) -> Dict[str, object]:
    findings: List[Dict[str, object]] = []
    for pat in PLACEHOLDER_PATTERNS:
        for m in re.finditer(pat, code, re.IGNORECASE):
            findings.append({"pattern": pat, "start": m.start(), "end": m.end()})
    return {"passed": len(findings) == 0, "count": len(findings), "findings": findings}


def compile_check(code: str, language: str) -> Dict[str, object]:
    lang = language.lower()
    with tempfile.TemporaryDirectory(prefix="whetstone_gate_") as td:
        td_path = Path(td)
        if lang in ("cpp", "c++"):
            src = td_path / "generated.cpp"
            src.write_text(code)
            cmd = ["g++", "-std=c++20", "-fsyntax-only", str(src)]
        elif lang in ("python", "py"):
            src = td_path / "generated.py"
            src.write_text(code)
            cmd = ["python3", "-m", "py_compile", str(src)]
        else:
            return {"passed": False, "skipped": True, "reason": f"compile gate not implemented for {language}"}

        try:
            p = subprocess.run(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                timeout=20,
                check=False,
            )
        except FileNotFoundError:
            return {"passed": False, "skipped": True, "reason": f"compiler missing for {language}"}
        except subprocess.TimeoutExpired:
            return {"passed": False, "skipped": False, "reason": "compile timeout"}

        return {
            "passed": p.returncode == 0,
            "skipped": False,
            "return_code": p.returncode,
            "stdout": p.stdout[-4000:],
            "stderr": p.stderr[-4000:],
        }


def test_check(code: str, language: str) -> Dict[str, object]:
    text = code.lower()
    if "priorityqueue" in text:
        required = ["enqueue", "dequeue", "peek", "size", "empty"]
        missing = [x for x in required if x not in text]
        return {"passed": not missing, "missing": missing}
    return {"passed": True, "missing": []}


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--code-file", help="Path to generated code file", default="")
    ap.add_argument("--code", help="Inline generated code", default="")
    ap.add_argument("--language", required=True)
    ap.add_argument("--out", default="")
    args = ap.parse_args()

    code = args.code
    if args.code_file:
        code = Path(args.code_file).read_text()

    placeholders = detect_placeholders(code)
    compile_res = compile_check(code, args.language)
    test_res = test_check(code, args.language)

    overall = bool(placeholders["passed"] and compile_res.get("passed", False) and test_res.get("passed", False))
    payload = {
        "language": args.language,
        "gates": {
            "compile": compile_res,
            "tests": test_res,
            "placeholder": placeholders,
            "overall_ready": overall,
        },
    }

    text = json.dumps(payload, indent=2, sort_keys=True)
    if args.out:
        out = Path(args.out)
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_text(text + "\n")
    print(text)


if __name__ == "__main__":
    main()

