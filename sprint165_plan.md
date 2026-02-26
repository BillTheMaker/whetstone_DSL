# Sprint 165 Plan: Enforcement, CI Gates, and Runtime Adoption of Max-Strict Grammars

## Context

After strict generation is implemented (Sprints 163-164), the remaining risk is
drift: schemas may evolve while grammar strictness regresses silently. This sprint
adds hard enforcement in CI and runtime preflight so strictness stays durable.

---

## Goals

1. Add CI gates that fail on strictness regressions
2. Version and fingerprint generated grammar artifacts
3. Validate runtime compatibility between MCP tool surface and strict artifacts
4. Provide an operational path for constrained decoding consumers

---

## Steps

### Step 1869: Strictness policy file and threshold gate (10 tests)

Add `tools/mcp/grammars/strictness_policy.json` with hard thresholds:
- tool coverage must remain 100%
- waived broad fallbacks <= configured cap (target near-zero)
- unsupported keyword count non-increasing unless explicitly approved

Add `tools/mcp/check_strictness_policy.py` to fail on policy violations.

Tests (10): threshold pass/fail behavior, coverage regression detection, waiver cap,
unsupported keyword regression detection, policy parse errors, missing report errors,
stable diagnostics, machine-readable failure output, exit code correctness,
backward-compatible policy extension handling.

### Step 1870: Artifact fingerprinting and drift detection (8 tests)

Add generator outputs:
- `tools/mcp/grammars/manifest.json` (tool count, schema hash, grammar hash, timestamp)
- `tools/mcp/grammars/manifest.lock` (deterministic lock digest)

Add `tools/mcp/verify_grammar_manifest.py` to compare generated artifacts against
current `whetstone_tool_schemas.json`.

Tests (8): manifest creation, deterministic lock stability, mismatch detection on
schema change, mismatch detection on grammar edit, tool-count drift detection,
timestamp isolation from hash, lock verify pass path, lock verify fail path.

### Step 1871: CI integration in build/test flow (8 tests)

Wire strictness checks into project validation scripts:
- run grammar regeneration
- run strictness audit
- run policy check
- run manifest verify

Fail CI on any violation.

Tests (8): happy-path CI pass, strictness regression CI fail, manifest drift CI fail,
missing grammar file CI fail, unsupported keyword growth CI fail, waived fallback cap
CI fail, clear failure summaries, reproducible local invocation.

### Step 1872: Runtime preflight strict compatibility check (10 tests)

Update `tools/mcp/validate_slm_runtime.sh` to optionally enforce strict artifacts:
- verify local MCP tool surface matches manifest tool list
- verify dispatch schema and per-tool schemas loadable
- verify representative constrained tool call samples validate

Add `--strict-grammars` mode (default on for CI, optional for local).

Tests (10): strict mode success, strict mode fail on missing grammar, fail on tool
count mismatch, fail on schema mismatch hash, pass on aligned artifacts, readable
error output, non-strict mode backward compatibility, sample validation execution,
timeout-safe behavior, exit code correctness.

### Step 1873: Sprint 165 Integration Summary (8 tests)

Add `editor/src/Sprint165IntegrationSummary.h`.
Record: steps_completed=5 (1869-1873), ci_gate_active=true,
manifest_drift_detection_active=true, runtime_strict_preflight_active=true,
success=true.

Tests (8): constructable summary struct, steps_completed==5, policy gate active,
manifest verification active, CI hook active, runtime strict mode active,
no coverage regression vs 347-tool baseline, success==true.

---

## Architecture Gate

- CI checks must be runnable locally and in automation without network dependency
- Generated artifacts remain committed and reproducible
- Strictness policy changes require explicit commit-level acknowledgment
- No runtime tool contract changes introduced in this sprint

