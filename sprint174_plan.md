# Sprint 174 Plan: Debug-Chain Wiring + Deterministic A/B Validation

## Context
We already have debug/remediation tools; they must be first-class in the automatic
repair chain with measurable impact on A/B outcomes.

## Goals
1. Ensure loop uses debug-chain actions deterministically
2. Produce explicit `green|blocked` outcomes with routed evidence
3. Validate improvements via A/B and benchmark reruns

## Steps

### Step 1910: Remediation router action expansion (10 tests)
Expand routing categories:
- include/symbol/import failures -> deterministic repair action
- compile structural failures -> pipeline diagnostics + generation refinement
- behavior/test failures -> targeted behavior hints

### Step 1911: Loop action execution contract (10 tests)
Enforce deterministic action order:
- action selection from router payload
- bounded retries and explicit blocked reason
- trace records for each executed action

### Step 1912: A/B + benchmark validation artifact update (8 tests)
Run:
- strict A/B test with token accounting
- strict production loop check
- benchmark suite pass-rate summary

### Step 1913: Sprint 174 Integration Summary (8 tests)
Add `editor/src/Sprint174IntegrationSummary.h` with:
- steps_completed=4
- debug_chain_execution_active=true
- deterministic_action_ordering_active=true
- validation_artifacts_generated=true
- success=true

## Architecture Gate
- Debug chain must be executed, not only reported
- Validation artifacts must quantify effect (quality + tokens + pass/blocked)
