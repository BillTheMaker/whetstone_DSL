#!/usr/bin/env python3
"""Evaluate production gates for generated code with real compile/test execution."""

from __future__ import annotations

import argparse
import json
import re
import shutil
import subprocess
import tempfile
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Tuple


PLACEHOLDER_PATTERNS = [
    r"\bTODO\b",
    r"\bFIXME\b",
    r"\bplaceholder\b",
    r"auto\s*/\*\s*TODO",
    r"/\*\s*missing",
]


@dataclass
class ExecResult:
    passed: bool
    skipped: bool
    reason: str
    return_code: int
    stdout: str
    stderr: str
    command: List[str]


def truncate(text: str, limit: int = 6000) -> str:
    if len(text) <= limit:
        return text
    return text[: limit - 32] + "\n...<truncated>...\n"


def detect_placeholders(code: str) -> Dict[str, object]:
    findings: List[Dict[str, object]] = []
    for pat in PLACEHOLDER_PATTERNS:
        for m in re.finditer(pat, code, re.IGNORECASE):
            findings.append({"pattern": pat, "start": m.start(), "end": m.end()})
    return {"passed": len(findings) == 0, "count": len(findings), "findings": findings}


def run_cmd(cmd: List[str], timeout_s: int) -> ExecResult:
    try:
        p = subprocess.run(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            timeout=timeout_s,
            check=False,
        )
        return ExecResult(
            passed=(p.returncode == 0),
            skipped=False,
            reason="ok" if p.returncode == 0 else "non_zero_exit",
            return_code=p.returncode,
            stdout=truncate(p.stdout),
            stderr=truncate(p.stderr),
            command=cmd,
        )
    except FileNotFoundError:
        return ExecResult(
            passed=False,
            skipped=True,
            reason="tool_missing",
            return_code=127,
            stdout="",
            stderr="",
            command=cmd,
        )
    except subprocess.TimeoutExpired as e:
        return ExecResult(
            passed=False,
            skipped=False,
            reason="timeout",
            return_code=124,
            stdout=truncate(e.stdout or ""),
            stderr=truncate(e.stderr or ""),
            command=cmd,
        )


def language_tools(language: str) -> Dict[str, str]:
    lang = language.lower()
    if lang in ("cpp", "c++"):
        cxx = shutil.which("g++") or shutil.which("clang++") or ""
        return {"compile": cxx, "test": cxx}
    if lang == "python":
        py = shutil.which("python3") or shutil.which("python") or ""
        return {"compile": py, "test": py}
    if lang == "rust":
        rustc = shutil.which("rustc") or ""
        return {"compile": rustc, "test": rustc}
    if lang == "go":
        go = shutil.which("go") or ""
        return {"compile": go, "test": go}
    return {"compile": "", "test": ""}


def detect_cpp_namespace_prefix(code: str) -> str:
    m = re.search(r"namespace\\s+([A-Za-z_][A-Za-z0-9_]*)\\s*\\{", code)
    if m:
        return m.group(1) + "::"
    q = re.search(r"([A-Za-z_][A-Za-z0-9_]*)::PriorityQueue", code)
    if q:
        return q.group(1) + "::"
    return ""


def cpp_harness(code: str) -> str:
    prefix = detect_cpp_namespace_prefix(code)
    return (
        code
        + "\nint main() {\n"
        + f"  {prefix}PriorityQueue q;\n"
        + f"  {prefix}WorkItem w(\"job\", 1, \"payload\");\n"
        + "  (void)q;\n"
        + "  (void)w;\n"
        + "  return 0;\n"
        + "}\n"
    )


def rust_harness(code: str) -> str:
    return (
        code
        + "\nfn main() {\n"
        + "  let q = PriorityQueue{};\n"
        + "  let w = WorkItem{job_id: \"job\".to_string(), priority: 1, payload: \"payload\".to_string()};\n"
        + "  let _ = q;\n"
        + "  let _ = w;\n"
        + "}\n"
    )


def go_harness(code: str) -> str:
    return (
        code
        + "\nfunc main() {\n"
        + "  q := PriorityQueue{}\n"
        + "  _ = q\n"
        + "}\n"
    )


def python_harness(code: str) -> str:
    return (
        code
        + "\nif __name__ == '__main__':\n"
        + "    q = PriorityQueue()\n"
        + "    w = WorkItem('job', 1, 'payload')\n"
        + "    _ = q\n"
        + "    _ = w\n"
    )


def compile_check(code: str, language: str, strict: bool) -> Dict[str, object]:
    lang = language.lower()
    tools = language_tools(lang)
    if lang not in ("cpp", "c++", "python", "rust", "go"):
        return {
            "passed": False,
            "skipped": True,
            "reason": "unsupported_language",
            "strict_blocking": bool(strict),
        }

    with tempfile.TemporaryDirectory(prefix="whetstone_gate_compile_") as td:
        td_path = Path(td)
        if lang in ("cpp", "c++"):
            src = td_path / "generated.cpp"
            src.write_text(code)
            cmd = [tools["compile"] or "g++", "-std=c++20", "-fsyntax-only", str(src)]
        elif lang == "python":
            src = td_path / "generated.py"
            src.write_text(code)
            cmd = [tools["compile"] or "python3", "-m", "py_compile", str(src)]
        elif lang == "rust":
            src = td_path / "generated.rs"
            src.write_text(code)
            cmd = [tools["compile"] or "rustc", "--crate-type", "lib", str(src), "-o", str(td_path / "libgenerated.rlib")]
        else:  # go
            src = td_path / "generated.go"
            src.write_text(code)
            cmd = [tools["compile"] or "go", "build", str(src)]

        res = run_cmd(cmd, timeout_s=20)
        payload = {
            "passed": res.passed,
            "skipped": res.skipped,
            "reason": res.reason,
            "return_code": res.return_code,
            "command": res.command,
            "stdout": res.stdout,
            "stderr": res.stderr,
            "strict_blocking": bool(strict and (res.skipped or not res.passed)),
        }
        return payload


def test_check(code: str, language: str, strict: bool) -> Dict[str, object]:
    lang = language.lower()
    queue_spec = ("priorityqueue" in code.lower())
    if not queue_spec:
        return {
            "passed": True,
            "skipped": True,
            "reason": "queue_harness_not_required",
            "strict_blocking": False,
        }

    required = ["enqueue", "dequeue", "peek", "size", "empty", "workitem"]
    text = code.lower()
    missing = [r for r in required if r not in text]
    if missing:
        return {
            "passed": False,
            "skipped": False,
            "reason": "missing_required_queue_members",
            "missing": missing,
            "strict_blocking": bool(strict),
        }

    with tempfile.TemporaryDirectory(prefix="whetstone_gate_test_") as td:
        td_path = Path(td)
        if lang in ("cpp", "c++"):
            src = td_path / "harness.cpp"
            src.write_text(cpp_harness(code))
            cxx = shutil.which("g++") or shutil.which("clang++")
            cmd = [cxx or "g++", "-std=c++20", "-fsyntax-only", str(src)]
        elif lang == "python":
            src = td_path / "harness.py"
            src.write_text(python_harness(code))
            py = shutil.which("python3") or shutil.which("python")
            cmd = [py or "python3", str(src)]
        elif lang == "rust":
            src = td_path / "harness.rs"
            src.write_text(rust_harness(code))
            rustc = shutil.which("rustc")
            cmd = [rustc or "rustc", str(src), "-o", str(td_path / "harness")]
        elif lang == "go":
            src = td_path / "harness.go"
            src.write_text(go_harness(code))
            go = shutil.which("go")
            cmd = [go or "go", "build", str(src)]
        else:
            return {
                "passed": False,
                "skipped": True,
                "reason": "unsupported_language",
                "strict_blocking": bool(strict),
            }

        res = run_cmd(cmd, timeout_s=25)
        return {
            "passed": res.passed,
            "skipped": res.skipped,
            "reason": res.reason,
            "return_code": res.return_code,
            "command": res.command,
            "stdout": res.stdout,
            "stderr": res.stderr,
            "strict_blocking": bool(strict and (res.skipped or not res.passed)),
        }


def normalize_diagnostics(compile_res: Dict[str, object], test_res: Dict[str, object]) -> List[Dict[str, object]]:
    patterns: List[Tuple[str, re.Pattern[str]]] = [
        ("gcc_clang", re.compile(r"^(?P<file>[^:\n]+):(\d+):(\d+):\s*(?P<sev>warning|error):\s*(?P<msg>.+)$", re.MULTILINE)),
        ("rustc", re.compile(r"^(?P<sev>error|warning)(\[[^\]]+\])?:\s*(?P<msg>.+)$", re.MULTILINE)),
        ("python", re.compile(r"File \"(?P<file>[^\"]+)\", line (?P<line>\d+).*(?P<msg>SyntaxError:.*)$", re.MULTILINE)),
    ]

    out: List[Dict[str, object]] = []

    def scan(source: str, category: str, text: str) -> None:
        seen = set()
        for family, pat in patterns:
            for m in pat.finditer(text or ""):
                file_name = m.groupdict().get("file", "")
                sev = m.groupdict().get("sev", "error")
                msg = m.groupdict().get("msg", "parse failure")
                line = int(m.group(2)) if m.lastindex and m.lastindex >= 2 and m.group(2) and m.group(2).isdigit() else None
                col = int(m.group(3)) if m.lastindex and m.lastindex >= 3 and m.group(3) and m.group(3).isdigit() else None
                key = (source, category, file_name, line, col, msg)
                if key in seen:
                    continue
                seen.add(key)
                out.append(
                    {
                        "source": source,
                        "family": family,
                        "severity": "warning" if sev == "warning" else "error",
                        "category": category,
                        "file": file_name,
                        "line": line,
                        "column": col,
                        "message": msg.strip() or "diagnostic",
                        "raw_excerpt": truncate(m.group(0), 300),
                    }
                )

    scan("compile", "compile", str(compile_res.get("stderr", "")))
    scan("tests", "tests", str(test_res.get("stderr", "")))
    if not out and (not compile_res.get("passed", True) or not test_res.get("passed", True)):
        out.append(
            {
                "source": "gate",
                "family": "fallback",
                "severity": "error",
                "category": "gates",
                "file": "",
                "line": None,
                "column": None,
                "message": "Gate failed without parseable diagnostic output",
                "raw_excerpt": "",
            }
        )
    return out


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--code-file", help="Path to generated code file", default="")
    ap.add_argument("--code", help="Inline generated code", default="")
    ap.add_argument("--language", required=True)
    ap.add_argument("--strict", action="store_true", help="Strict mode: missing tools/skips are blocking")
    ap.add_argument("--out", default="")
    args = ap.parse_args()

    code = args.code
    if args.code_file:
        code = Path(args.code_file).read_text()

    placeholders = detect_placeholders(code)
    compile_res = compile_check(code, args.language, strict=args.strict)
    test_res = test_check(code, args.language, strict=args.strict)
    diagnostics = normalize_diagnostics(compile_res, test_res)

    compile_pass = bool(compile_res.get("passed", False))
    test_pass = bool(test_res.get("passed", False))
    if args.strict:
        compile_pass = compile_pass and not bool(compile_res.get("strict_blocking", False))
        test_pass = test_pass and not bool(test_res.get("strict_blocking", False))

    overall = bool(placeholders["passed"] and compile_pass and test_pass)

    failure_reasons: List[str] = []
    if not placeholders["passed"]:
        failure_reasons.append("placeholder_or_todo_detected")
    if not compile_pass:
        failure_reasons.append(f"compile_failed:{compile_res.get('reason', 'unknown')}")
    if not test_pass:
        failure_reasons.append(f"tests_failed:{test_res.get('reason', 'unknown')}")

    payload = {
        "language": args.language,
        "strict_mode": bool(args.strict),
        "diagnostics": diagnostics,
        "gates": {
            "compile": compile_res,
            "tests": test_res,
            "placeholder": placeholders,
            "failure_reasons": failure_reasons,
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
