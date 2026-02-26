# Sprint 228-231 Execution Tracker - 2026-02-26

## Scope
Planned and started:
- `sprint228_plan.md`
- `sprint229_plan.md`
- `sprint230_plan.md`
- `sprint231_plan.md`

## Implemented So Far

- Added planning-readiness tool:
  - `tools/mcp/spec_planning_readiness.py`
- Integrated planning precheck into taskitem pipeline:
  - `tools/mcp/run_sprint_taskitem_pipeline.sh`
  - new env controls:
    - `WSTONE_SPEC_READINESS_PRECHECK` (default `1`)
    - `WSTONE_SPEC_READINESS_HARD_GATE` (default `0`)
    - `WSTONE_SPEC_READINESS_MIN_SCORE` (default `65`)
- Tool outputs:
  - readiness score (`section_score`, `keyword_score`, `total`)
  - verdict (`execution_ready` or `needs_spec_hardening`)
  - missing-action list for planning gaps
  - recommended markdown template blocks for missing sections

## Baseline Evidence

- `logs/taskitem_runs/TEST_ONLY_spec_planning_baseline_20260226/fullstack_spec_readiness.json`
- `logs/taskitem_runs/TEST_ONLY_spec_planning_baseline_20260226/deterministic_spec_readiness.json`
- `logs/taskitem_runs/TEST_ONLY_spec_planning_hardening_20260226/sprint228_before.json`
- `logs/taskitem_runs/TEST_ONLY_spec_planning_hardening_20260226/sprint228_after.json`
- `logs/taskitem_runs/TEST_ONLY_spec_planning_hardening_20260226/sprint228_hardened.md`
- `logs/taskitem_runs/TEST_ONLY_spec_hardening_gate_20260226_s228_r2/report.json`

Observed baseline:
- both sample docs currently return `needs_spec_hardening`
- low readiness scores indicate missing first-class sections for constraints/acceptance/environment/security/performance.

Hardening delta (sprint228 plan sample):
- before: `score=17`, verdict `needs_spec_hardening`, missing actions `10`
- after auto-hardening scaffold: `score=100`, verdict `execution_ready`, missing actions `0`

Pipeline hard-gate proof:
- original spec run (hard gate on): `rc=7` with explicit readiness failure
  - `logs/taskitem_runs/sprint228_plan_20260226_141045/00a_spec_readiness.json`
- hardened spec run (hard gate on): `rc=0` and full pipeline completion
  - `logs/taskitem_runs/sprint228_hardened_20260226_141045/00_summary.json`

Reusable flow wrapper:
- `tools/mcp/run_spec_hardening_gate.sh`
- example invocation:
  - `tools/mcp/run_spec_hardening_gate.sh sprint228_plan.md logs/taskitem_runs/TEST_ONLY_spec_hardening_gate_20260226_s228_r2`

## Explicit Completion Signal

- Sprint 228: `PARTIAL` (tool implemented, baseline executed, pipeline precheck integrated)
- Sprint 229: `PARTIAL` (constraint synthesis mapping implemented in tool)
- Sprint 230: `PARTIAL` (acceptance readiness checks implemented in tool)
- Sprint 231: `PARTIAL` (environment/projection checks implemented in tool)

## Next Closure Work

- Enable hard-gate mode for selected sprint ranges and capture pass/fail deltas.
- Add reusable wrapper script for readiness->hardening->hard-gate flow across catalogs.
- Re-run benchmarks using hardened specs and measure downstream readiness deltas.
