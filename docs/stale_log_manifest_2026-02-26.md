# Stale Log Manifest - 2026-02-26

Purpose: prevent confusion from historical docs that are still present but no longer authoritative for active sprint execution.

## Authoritative Current Docs

- `docs/progress_log_2026-02-26.md`
- `docs/generator_readiness_gap_registry_2026-02-26.md`
- `docs/sprint206_211_execution_tracker_2026-02-26.md`
- `docs/sprint217_221_execution_tracker_2026-02-26.md`
- `docs/sprint222_224_execution_tracker_2026-02-26.md`
- `docs/sprint225_227_execution_tracker_2026-02-26.md`

## Historical / Potentially Stale Docs

These are useful for history but should not be treated as current execution truth without re-validation:

- `docs/SPRINT_1_PROGRESS.md` (historical)
- `docs/SPRINT_2_PLAN.md` (historical)
- `docs/SPRINT_2_VISION.md` (historical)
- `docs/SPRINT_3_PLAN.md` (historical)
- `docs/sprint161_162_taskitem_execution_log_2026-02-25.md` (historical checkpoint)
- `docs/sprint163_165_taskitem_execution_log_2026-02-26.md` (historical checkpoint)
- `docs/sprint166_168_taskitem_execution_log_2026-02-26.md` (historical checkpoint)
- `AGENT_NOTES_2026-02-22.md` (dated handoff; stale for active state)
- `AGENT_NOTES_2026-02-23.md` (dated handoff; stale for active state)
- `docs/AGENT_HANDOFF_2026-02-24.md` (dated handoff; stale for active state)

## Rule For Next Agents

When docs disagree, trust newest dated execution tracker + run artifacts under `logs/taskitem_runs/`.

## Test-Only Artifact Rule

- Treat directories under `logs/taskitem_runs/TEST_ONLY_*` as validation artifacts only.
- Never treat markdown specs generated within `TEST_ONLY_*` directories as authoritative product specs.
