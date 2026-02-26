# Sprint 167 Plan: Production Gates for Generated Code (Compile/Test/No-Placeholder)

## Context

Generation quality is currently inferred from text shape. To be production-ready,
generated artifacts must pass hard gates:
- compile successfully
- satisfy required tests/checks
- contain no placeholder markers in shipped output

This sprint introduces enforceable production gates and integrates them into MCP
tool outputs and planning flow.

---

## Goals

1. Add compile/test validation for generated artifacts
2. Block completion when placeholders/TODOs remain
3. Expose gate results in MCP responses
4. Enable deterministic fail-fast behavior for non-production output

---

## Steps

### Step 1879: Placeholder/TODO detector and policy (10 tests)

Create detector for non-production markers in generated code:
- `TODO`, `FIXME`, `placeholder`, `auto /* TODO`, synthetic stubs
- language-specific placeholder patterns

Tests (10): marker detection across C++/Rust/Go/Java/Python, false-positive guard,
comment-only configurable handling, deterministic counts, severity mapping,
policy override handling, report serialization, integration with quality metadata,
non-empty marker context capture, no-regression on clean code.

### Step 1880: Compile gate runner for generated snippets/projects (12 tests)

Add compile gate utility:
- compile generated unit (header/source or minimal target scaffold)
- capture structured diagnostics
- return pass/fail + error categories

Tests (12): compile pass path, syntax failure path, type failure path, missing
include path, deterministic command construction, timeout handling, temp workspace
cleanup, diagnostics normalization, per-language command routing, empty-input guard,
partial compile mode, machine-readable report schema.

### Step 1881: Test gate runner and minimal harness injection (10 tests)

Add test gate path:
- generate or attach minimal behavioral tests for known patterns (PriorityQueue etc.)
- run and collect results

Tests (10): harness generation, test pass path, behavioral fail path, timeout path,
deterministic harness names, output parsing, flaky-test guard mode, no-harness
scenario reporting, per-language skip policy, report schema validity.

### Step 1882: MCP output contract update for production gates (8 tests)

Update relevant tools (`whetstone_generate_code`, `whetstone_run_pipeline`) to
include gate block:
- `gates.compile.passed`
- `gates.tests.passed`
- `gates.placeholder.passed`
- `gates.overall_ready`

Tests (8): all fields present, pass/fail correctness, backward compatibility for
existing consumers, schema sync with `tools/claude/tools.json`, cross-tool
consistency, deterministic overall aggregation, failure reasons included, docs updated.

### Step 1883: Sprint 167 Integration Summary (8 tests)

Add `editor/src/Sprint167IntegrationSummary.h`.
Record: steps_completed=5 (1879-1883), compile_gate_active=true,
test_gate_active=true, placeholder_gate_active=true, success=true.

Tests (8): constructable summary struct, steps_completed==5, three gates active,
overall_ready false when any gate fails, overall_ready true on green sample,
schema consistency verified, regression suite pass, success==true.

---

## Architecture Gate

- Gate checks must be deterministic and machine-readable
- Failures must include actionable diagnostics and not silently downgrade quality
- “Ready” status is forbidden when placeholders or failed compile/tests remain

