# Sprint 252 Execution Tracker - 2026-02-26

## Scope
- `sprint252_plan.md`

## Implemented

New analyzer:
- `tools/mcp/analyze_raw_candidate_uplift_history.py`
  - Computes recent raw-search uplift rate from run summaries.

Closure ladder policy routing in `tools/mcp/run_native_profile_closure_ladder.sh`:
- Added controls:
  - `WSTONE_CLOSURE_LADDER_SKIP_RAW_WHEN_NO_UPLIFT` (default `1`)
  - `WSTONE_CLOSURE_LADDER_RAW_HISTORY_GLOB`
  - `WSTONE_CLOSURE_LADDER_RAW_HISTORY_MAX_RUNS`
  - `WSTONE_CLOSURE_LADDER_RAW_MIN_UPLIFT_RATE`
  - `WSTONE_CLOSURE_LADDER_RAW_MIN_RECORDS` (default `3`)
- Emits policy artifact when enabled:
  - `raw_uplift_history.json`
- Dynamically removes `raw_only` from ladder modes when uplift evidence is below threshold.

## Validation Artifacts

- policy-enabled run:
  - `logs/taskitem_runs/TEST_ONLY_sprint252_closure_ladder_policy_20260226/closure_ladder_summary.json`
- forced-skip run (demonstrates raw mode removed):
  - `logs/taskitem_runs/TEST_ONLY_sprint252_closure_ladder_policy_skip2_20260226/closure_ladder_summary.json`

Observed forced-skip signal:
- `attempted_modes=[single_shot_shape,multishot,autofill]`
- selected mode remained `single_shot_shape`.

## Explicit Completion Signal

- Sprint 252: `DONE` (implemented + policy skip behavior verified)
