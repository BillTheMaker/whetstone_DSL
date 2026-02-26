# Spec Hardening Batch Report - 2026-02-26

## Scope

Batches executed with:
- `tools/mcp/run_spec_hardening_gate.sh`

Corpora:
1. Drive project docs (`/home/bill/Documents/drive_projects/...`) - 7 specs
2. Example product meta specs (`example_projects/*/PROJECT_META.md`) - 10 specs

## Batch Artifacts

Drive batch:
- `logs/taskitem_runs/drive_spec_hardening_batch_20260226/results.jsonl`
- `logs/taskitem_runs/drive_spec_hardening_batch_20260226/summary.json`
- `logs/taskitem_runs/drive_spec_hardening_batch_20260226/quality_summary.json`

Example project meta batch:
- `logs/taskitem_runs/example_projectmeta_hardening_batch_20260226/results.jsonl`
- `logs/taskitem_runs/example_projectmeta_hardening_batch_20260226/summary.json`

## High-Density Outcomes

Readiness/hard-gate behavior:
- Original specs: all blocked by hard gate (`original_rc=7`)
- Hardened specs: all passed (`hardened_rc=0`)
- Readiness scores improved to `100` for all hardened outputs.

Drive batch deltas:
- before score range: `15..22`
- after score range: `100..100`
- delta range: `78..85`

Example product-meta deltas:
- before score range: `0..8`
- after score range: `100..100`
- delta range: `92..100`

## Important Limitation Discovered

Downstream taskitem metrics are currently low-discrimination across both batches:
- `task_count` consistently `2`
- `validation.average_score` consistently `87.5`
- `validation.average_execution_specificity_score` consistently `75.0`
- `validation.failing_count` consistently `0`

Interpretation:
- The new planning gate/hardening loop is working as intended for pre-execution quality control.
- But current sprint pipeline translation quality signal is too flat for heterogeneous product specs.
- Next required work: increase spec-to-taskitem semantic sensitivity (more variable decomposition and specificity based on domain constraints).

## Recommendation

Use real specs first (drive + product metas), then generate synthetic specs only to cover missing domains after translation-sensitivity improvements are in place.
