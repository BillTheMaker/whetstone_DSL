# Sprint 168 Plan: Autonomous Production Completion Loop

## Context

With stronger generation and hard gates, the final gap is orchestration:
tools must iterate automatically until production gates are green, instead of
stopping at first draft output.

This sprint delivers a closed-loop production completion workflow and validates
it on the PriorityQueue benchmark and one additional non-trivial workload.

---

## Goals

1. Add iterative fix loop until compile/test/placeholder gates pass
2. Integrate gate-driven retries into taskitem execution flow
3. Record deterministic completion traces for training and audit
4. Demonstrate full “spec -> production-ready code” completion on benchmark tasks

---

## Steps

### Step 1884: Gate-driven remediation planner (10 tests)

Implement remediation planner that maps gate failures to concrete next actions:
- syntax/type failures -> targeted regeneration/patch ops
- missing behavior -> test-guided method/body completion
- placeholders -> forced concrete-type/body refinement pass

Tests (10): failure classification, action selection correctness, deterministic
plan order, no-op on green gates, multi-failure prioritization, retry budget
handling, loop termination criteria, unsupported failure fallback, structured
reason emission, reproducibility.

### Step 1885: Closed-loop execution engine for production completion (12 tests)

Add loop:
1. generate
2. run gates
3. remediate
4. repeat until green or budget exhausted

Tests (12): converges on solvable sample, exits on budget exhaustion, preserves
best-known artifact, deterministic iteration logs, handles intermittent failures,
reports per-iteration deltas, no silent success on red gates, timeout safety,
parallel-safe temp dirs, cancellation support, final status schema, performance bounds.

### Step 1886: Taskitem pipeline integration (`overall_ready` semantics) (8 tests)

Update planning/execution integration so completion requires green gates:
- tasks remain incomplete if `overall_ready=false`
- queue readiness reflects gate status

Tests (8): task blocked on gate failure, task completes on all-green, queue-ready
counts reflect gating, status propagation into validation report, backward-compatible
fields for existing dashboards, deterministic ordering, no false-ready state,
explicit blocker reasons.

### Step 1887: Benchmark validation (PriorityQueue + secondary workload) (10 tests)

Run benchmark set:
- PriorityQueue (A/B task)
- one additional structured workload (e.g., cache indexer module)

Collect metrics:
- iterations to green
- compile/test pass rates
- placeholder count final
- wall time and token usage

Tests (10): benchmark harness runnable, both workloads reported, green-gate proof
for completed run, metric schema completeness, reproducible run IDs, output artifact
preservation, comparison vs baseline draft-only mode, failure-mode trace capture,
summary generation, non-empty diagnostics on failure.

### Step 1888: Sprint 168 Integration Summary (8 tests)

Add `editor/src/Sprint168IntegrationSummary.h`.
Record: steps_completed=5 (1884-1888), closed_loop_active=true,
production_gate_enforced=true, benchmark_completion_verified=true, success=true.

Tests (8): constructable summary struct, steps_completed==5, loop active,
gate enforcement active, benchmark evidence path non-empty, no false-green
completion, regression suite pass, success==true.

---

## Architecture Gate

- Completion state must be gate-derived, never text-heuristic-only
- Every failed run must preserve actionable trace/evidence
- Benchmark outputs must be reproducible and auditable from saved artifacts

