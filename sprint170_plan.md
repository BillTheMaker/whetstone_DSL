# Sprint 170 Plan: Real Compile/Test Gates Across Target Languages

## Context

Current gates are significantly improved but still partially heuristic, especially
outside the benchmark happy path. To claim production capability, gate decisions
must be backed by real compile/test execution for key target languages.

---

## Goals

1. Replace heuristic compile/test estimates with real command execution gates
2. Support at least C++/Rust/Go/Python gate execution in local environment
3. Produce normalized diagnostics from real compiler/test output
4. Enforce “no green without real gate pass” in production loop

---

## Steps

### Step 1894: Multi-language compile gate executor (12 tests)

Extend gate evaluator with real compile/syntax checks:
- C++: `g++`/`clang++` syntax/build check path
- Rust: `rustc`/`cargo check` path
- Go: `go build` path
- Python: `py_compile`

Tests (12): per-language pass/fail cases, missing-tool behavior, timeout handling,
temp workspace cleanup, deterministic command selection, normalized error schema,
stderr capture truncation policy, path safety, language override behavior, fail on
unsupported language in strict mode, optional non-strict skip mode, reproducibility.

### Step 1895: Multi-language minimal test harness runner (10 tests)

Add generated test harness execution for canonical patterns:
- queue behavior tests and data-model access tests
- language-specific harness templates

Tests (10): harness generation per language, test pass path, behavior fail path,
framework/tool missing handling, deterministic harness naming, test output parser,
timeout and cleanup, strict/non-strict behavior, normalized result schema,
artifact preservation for failed runs.

### Step 1896: Diagnostic normalizer for compile/test gates (8 tests)

Normalize raw compiler/test outputs into stable machine-readable diagnostics:
- severity, category, file, line, column, message, raw excerpt

Tests (8): parser coverage for gcc/clang/rustc/go/python formats, unknown format
fallback, ordering stability, dedupe behavior, truncated raw payload handling,
category mapping, non-empty message guarantee, schema validity.

### Step 1897: Strict gate mode in production loop (8 tests)

Update production loop script/tooling:
- strict mode requires real compile + real test pass
- no heuristic-only success in strict mode

Tests (8): strict pass requires real gate pass, strict fail on missing toolchain,
non-strict fallback behavior, explicit reason codes, gate summary fields, iteration
termination on non-recoverable gate failures, benchmark strict run pass, no false-green.

### Step 1898: Sprint 170 Integration Summary (8 tests)

Add `editor/src/Sprint170IntegrationSummary.h`.
Record: steps_completed=5 (1894-1898), real_compile_gates_active=true,
real_test_gates_active=true, strict_loop_enforced=true, success=true.

Tests (8): constructable summary struct, steps_completed==5, compile gates active,
test gates active, strict mode active, normalized diagnostics active, benchmark
strict pass evidence present, success==true.

---

## Architecture Gate

- In strict mode, `overall_ready=true` requires real compile + real test pass
- Missing toolchain must be an explicit gate failure, not silent pass
- Diagnostics must be actionable and normalized

