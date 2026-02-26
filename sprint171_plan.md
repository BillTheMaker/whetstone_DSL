# Sprint 171 Plan: Tool-Chained Autonomous Remediation to Green

## Context

With improved generation and real gates, the final requirement is robust autonomous
remediation: when output fails, the system should chain generation + debug/fix tools
until gates are green (or a clear, auditable stop reason is reached).

This sprint connects coding tools to remediation/debug tooling so failed outputs
trigger deterministic repair workflows rather than manual intervention.

---

## Goals

1. Chain generation failures into debug/fix tool workflows automatically
2. Use taskitem + remediation planning to drive next-best tool calls
3. Preserve full audit trace of each remediation step
4. Achieve repeated benchmark green completion without manual prompt edits

---

## Steps

### Step 1899: Remediation router from gate diagnostics to tool actions (10 tests)

Map normalized diagnostics to repair actions/tool choices:
- signature/type errors -> AST mutation/codegen refinement
- missing methods/fields -> class/data-model completion actions
- test behavior failures -> targeted body patch plan

Tests (10): diagnostic->action mapping coverage, deterministic action ordering,
priority handling, unsupported-category fallback, no-op on green state, confidence
scoring for routes, stable serialization, conflict resolution, retry budget policy,
tool argument schema validation.

### Step 1900: Debug tool integration path for failed generations (10 tests)

Integrate existing debug-oriented tools into remediation loop where relevant:
- diagnostic fetch, quick-fix application, build/test output parsing
- replayable fix attempts with rollback metadata

Tests (10): debug tool invocation path, quick-fix roundtrip, build-parse integration,
rollback metadata capture, failure classification, deterministic trace format,
tool permission safety, no infinite retry loops, non-recoverable failure stop,
structured stop reason output.

### Step 1901: Autonomous loop controller with “green or explicit blocked” contract (12 tests)

Controller semantics:
- iterate generate -> gate -> remediate
- finish only on all-green OR explicit blocked status with complete evidence

Tests (12): green completion path, blocked path with reason, mixed-failure handling,
budget exhaustion semantics, state checkpointing, resumable runs, deterministic
iteration IDs, artifact retention, status schema validity, strict-mode compliance,
benchmark repetition stability, no silent partial success.

### Step 1902: Benchmark suite for production capability claim (8 tests)

Run repeated benchmark batch (minimum N runs) for:
- PriorityQueue
- one additional structured workload

Collect pass rate, average iterations, mean/95p latency, failure taxonomy.

Tests (8): batch runner works, repeated-run summary generated, pass-rate field
accurate, latency stats present, failure taxonomy non-empty when failures exist,
trace links preserved, deterministic IDs, report schema validity.

### Step 1903: Sprint 171 Integration Summary (8 tests)

Add `editor/src/Sprint171IntegrationSummary.h`.
Record: steps_completed=5 (1899-1903), remediation_router_active=true,
debug_tool_chaining_active=true, autonomous_green_or_blocked_contract=true,
production_capability_benchmarked=true, success=true.

Tests (8): constructable summary struct, steps_completed==5, router active,
debug chaining active, contract enforced, benchmark report present, regression
suite pass, success==true.

---

## Architecture Gate

- No silent “success” without green gates
- Non-green exits must include explicit blocked reason + full evidence trail
- Remediation routing must stay deterministic and auditable

