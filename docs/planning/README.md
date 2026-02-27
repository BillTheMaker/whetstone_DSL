# Planning Workspace

This directory is for real project planning and backlog management.

## Files
- `professional_project_spec_v1.md`: canonical project spec template.
- `project_backlog_schema_v1.json`: JSON schema for backlog records.
- `project_backlog_starter_2026-02-27.json`: starter backlog with high-significance fields.
- `specs/`: one spec per project (`<project_id>.md`).

## Workflow
1. Add or update a project in `project_backlog_starter_2026-02-27.json`.
2. Create its spec in `specs/<project_id>.md` using the v1 template.
3. Raise `execution_readiness` to `execution_ready` only when checklist is complete.
4. During burst month, execute by priority + dependency order while preserving `personal_significance_tier`.

## Priority Policy
- `priority_tier` decides urgency.
- `personal_significance_tier` prevents mission-critical personal projects from being deprioritized by short-term throughput goals.
- If conflict exists, treat `core_life_work` as protected unless explicitly overridden.
