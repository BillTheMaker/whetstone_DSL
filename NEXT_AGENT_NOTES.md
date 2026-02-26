# Next Agent Notes

## Current Canonical State (2026-02-26)

- `TEST_ONLY` artifact rule:
  - Any path under `logs/taskitem_runs/TEST_ONLY_*` is validation-only data.
  - Do not use hardened specs in `TEST_ONLY_*` as source-of-truth project specs.
  - See `logs/taskitem_runs/TEST_ONLY_README_2026-02-26.md`.

- Read first:
  - `docs/progress_log_2026-02-26.md`
  - `docs/generator_readiness_gap_registry_2026-02-26.md`
  - `docs/stale_log_manifest_2026-02-26.md`
- Most recent execution trackers:
  - `docs/sprint217_221_execution_tracker_2026-02-26.md`
  - `docs/sprint222_224_execution_tracker_2026-02-26.md`
  - `docs/sprint225_227_execution_tracker_2026-02-26.md`
- Latest parity closure evidence:
  - `logs/taskitem_runs/challenging_subset_prod_20260226_r7/summary.json`
  - `logs/taskitem_runs/challenging_fullstack_multifile_20260226_r7/summary.json`
- First-class planning tranche (new):
  - `docs/sprint228_231_execution_tracker_2026-02-26.md`
  - `tools/mcp/spec_planning_readiness.py`
  - `tools/mcp/spec_planning_hardener.py`
  - `tools/mcp/run_spec_hardening_gate.sh`
  - `docs/spec_hardening_batch_report_2026-02-26.md`

## Dated Handoff

Primary file for this handoff:

- `AGENT_NOTES_2026-02-23.md`
- `docs/generator_readiness_gap_registry_2026-02-26.md` (canonical dated generator readiness gap registry; update `Last reviewed` when touched)
- `sprint186_plan.md` through `sprint205_plan.md` (active backlog covering all open/partial generator readiness gaps)
- `docs/sprint186_205_execution_tracker_2026-02-26.md` (execution/evidence log for the full sprint range)
- `docs/sprint186_205_intent_audit_2026-02-26.md` (strict intent-vs-closeout audit)
- `docs/deterministic_gap_hunt_2026-02-26.md` (new hard benchmark findings and gap IDs GR-011..GR-015)
- `sprint206_plan.md` through `sprint211_plan.md` (new backlog for false-green/projection/parity/intent drift gaps)
- `docs/sprint206_211_execution_tracker_2026-02-26.md` (execution/evidence for sprint206-211 + hard rerun deltas)

Before executing sprint work, follow:

- `docs/SPRINT_TASKITEM_EXECUTION_POLICY.md`

## Required Workflow (Do Not Skip)

1. Generate taskitems via MCP pipeline:
   - `tools/mcp/run_sprint_taskitem_pipeline.sh <sprint_plan.md>`
2. Export run to LoRA capture JSONL:
   - `tools/mcp/export_taskitem_run_for_lora.sh <run_output_dir>`
3. For ranges:
   - `tools/mcp/run_sprint_range_with_capture.sh <start> <end>`

## Preferred Data-Collection Workflow (Decoupled)

Use dedicated run specs for SLM data collection instead of editor sprint plans:

1. Run spec batch + capture:
   - `tools/mcp/run_spec_batch_with_capture.sh datasets/run_specs`
2. Spec corpus:
   - `datasets/run_specs/` (`runspec_*.md` + `index.jsonl`)
3. Keep sprint plans for editor/product work, not synthetic data generation.

## Multi-Project Corpus (New)

Use cross-project fixtures and specs to avoid local minima:

1. Fixtures:
   - `example_projects/`
2. Specs:
   - `datasets/example_run_specs/` (`runspec_*.md` + `index.jsonl`)
3. Generator:
   - `tools/mcp/generate_example_projects_and_specs.sh`
4. Novelty loop:
   - `tools/mcp/run_novelty_churn_loop.sh`
   - includes project specs when `INCLUDE_EXAMPLE_PROJECT_SPECS=1` (default)
5. Concurrency fix:
   - `tools/mcp/run_spec_batch_with_capture.sh` now uses unique batch summary names (`timestamp + pid + random`) to prevent file overwrite in concurrent runs.

## Stable MCP Binary

Use the pinned stable binary path (not dev build path):

- `editor/build-native/whetstone_mcp_stable`

Global MCP config should point to this stable path.

## Sprint 141-145 Completion Ledger (2026-02-24)

Status: Completed in code and test scaffolding for steps `1654-1698`.

| Sprint | Step Range | Status | Notes |
|---|---:|---|---|
| 141 | 1649-1658 | Complete | Authoring mode foundations, mode tools, integration summary added |
| 142 | 1659-1668 | Complete | Text-to-AST incremental sync models/tools + integration summary added |
| 143 | 1669-1678 | Complete | AST-to-text regeneration models/tools + integration summary added |
| 144 | 1679-1688 | Complete | Text/AST conflict+merge models/tools + integration summary added |
| 145 | 1689-1698 | Complete | C++ constructive loop models/tools + integration summary added |

Key wiring completed:

- `editor/src/mcp/RegisterSprint141Tools.h`
- `editor/src/mcp/RegisterSprint142Tools.h`
- `editor/src/mcp/RegisterSprint143Tools.h`
- `editor/src/mcp/RegisterSprint144Tools.h`
- `editor/src/mcp/RegisterSprint145Tools.h`
- `editor/src/MCPServer.h` (includes for Sprint 142-145 registration headers)
- `editor/src/mcp/RegisterOnboardingAndAllTools.h` (register calls through Sprint 145)

Integration summary headers added:

- `editor/src/Sprint141IntegrationSummary.h`
- `editor/src/Sprint142IntegrationSummary.h`
- `editor/src/Sprint143IntegrationSummary.h`
- `editor/src/Sprint144IntegrationSummary.h`
- `editor/src/Sprint145IntegrationSummary.h`

Verification result:

- Built/ran all step tests `1654..1698`
- Final check command:
  - `for n in $(seq 1654 1698); do editor/build-native/step${n}_test; done`
- Final result:
  - `all_1654_1698_passed`
