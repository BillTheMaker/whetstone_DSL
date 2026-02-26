# Fullstack + Multi-file Gap Hunt - 2026-02-26

Runs:
- Baseline AB-only: `logs/taskitem_runs/challenging_fullstack_multifile_20260226`
- Production-enabled recheck: `logs/taskitem_runs/challenging_fullstack_multifile_20260226_r5`
- Parity-gated production recheck: `logs/taskitem_runs/challenging_fullstack_multifile_20260226_r6`
- Repair-augmented parity recheck: `logs/taskitem_runs/challenging_fullstack_multifile_20260226_r7`

## High-Density Findings

- `total_runs=12`
- Path A ready: `50%`
- Path B ready: `50%`
- C++ readiness in this set: `0%` (Path A and Path B)
- Python readiness in this set: `100%`
- Token ratio `PathB/PathA`: `14.15x`

## New Gap Signals

- `GR-016` Multi-file transactional closure gap
  - deterministic generation struggles when changes must span tightly-coupled file sets with rollback semantics.

- `GR-017` Full-stack contract propagation gap
  - backend/frontend/schema/docs co-evolution lacks robust cross-layer consistency guarantees in deterministic path.

- `GR-018` Performance/security/rollout constrained refactor gap
  - changes with explicit SLO/security/rollout constraints are under-enforced as first-class execution contracts.

## Suggested Next Sprint Tranche

- `217`: multi-file edit graph + required-artifact closure gate
- `218`: full-stack contract propagation gate (OpenAPI/sdk/frontend sync)
- `219`: migration/backfill/rollback hard gate with data-safety evidence
- `220`: security propagation + deny-by-default cross-layer gate
- `221`: SLO budget and rollout-choreography strict enforcement

## Post-217-221 Recheck (r5)

- `fullstack_contract.invalid_runs=0` with strict-mode checks enabled.
- Production loop now runs across this set (`RUN_PROD=1`) and reports:
  - `ready_runs=12/12`
  - `false_green_candidates=0`
  - `ab_prod_divergence_count=6`
- Remaining blocker is not contract presence; it is execution-path divergence:
  - C++ AB readiness remains `0%` while production reports ready for same C++ rows.

## Post-222-224 Parity Recheck (r6)

- parity mode enabled (`REQUIRE_AB_PARITY=1`)
- unresolved divergence now closed:
  - `ab_prod_divergence_count=0`
- safety gate now blocks inconsistent C++ rows:
  - `ab_consistency_blocked_count=6`
- practical effect:
  - production ready drops from `12/12` (r5) to `6/12` (r6), matching only non-divergent rows.

## Post-225-227 Repair + Parity Recheck (r7)

- deterministic path-B repair layer enabled for C++/Go/Rust transpile artifacts.
- parity closure metrics:
  - `ab_prod_divergence_count=0`
  - `ab_consistency_blocked_count=0`
- hard suite headline:
  - Path B ready: `12/12` (`100%`)
  - production ready: `12/12` (`100%`)
