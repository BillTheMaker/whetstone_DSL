# Constructive Editing Runtime Plan (Low-Res)

## Objective

Move from MCP contract-level/runtime packet behavior to real constructive
editing and code generation execution for representative languages:

- `cpp`
- `rust`
- `python`
- `typescript`
- `javascript`
- `go`
- `java`
- `elisp`

## Current Baseline

- Sprint 146-155 MCP handlers are now runtimeized and stateful.
- Representative language runtime exists and is wired to tool handlers.
- Handler-level step tests (`1703`-`1795` tool surface set) are passing.
- Taskitem pipeline logging is active and appending LoRA capture records.

## Low-Res Plan

1. Runtime to Engine Binding
- Replace synthetic stage outcomes with calls into existing adapter/sync/
  regeneration/merge modules.
- Keep current response envelope stable while adding real engine evidence.

2. File Mutation and Replay Integrity
- Add real file-level mutation application paths for constructive step/loop.
- Persist before/after snapshots and replay metadata per operation.

3. Toolchain Execution Integration
- Wire provider routing to real build/test command execution paths.
- Normalize diagnostics from execution output into canonical payloads.

4. Multi-Language Qualification
- Run representative-language deterministic replay and diff checks.
- Gate language tier transitions using real replay/rollback evidence.

5. GA Hardening
- Add end-to-end constructive smoke scenarios per language family.
- Finalize rollout controls and promote only evidence-backed tiers.

6. First-Class Planning Readiness
- Add pre-execution spec readiness scoring and gap detection.
- Synthesize missing constraints/acceptance/environment scaffolds before taskitem generation.
- Gate taskitem execution on minimum planning-readiness thresholds.

## Implementation Map

### Phase A: Bind Constructive Step to Real Adapter Operations

Primary files:
- `editor/src/mcp/RegisterSprint152Tools.h`
- `editor/src/graduation/RepresentativeLanguageRuntime.h`
- `editor/src/CppConstructiveEditAdapter.h`
- `editor/src/RustGoConstructiveEditAdapter.h`
- `editor/src/PythonTypeScriptConstructiveEditAdapter.h`
- `editor/src/AdapterOperationUtil.h`

Deliverables:
- `whetstone_run_constructive_step` uses language-specific adapter execution.
- `whetstone_get_constructive_status` returns real adapter diagnostics.
- `whetstone_run_constructive_loop` records per-stage real outcomes.

Validation:
- Existing step tests: `1763`, `1764`, `1765`.
- New integration test family: constructive step with real adapter outputs.

### Phase B: Real Sync/Regenerate/Merge Path

Primary files:
- `editor/src/mcp/RegisterSprint142Tools.h`
- `editor/src/mcp/RegisterSprint143Tools.h`
- `editor/src/mcp/RegisterSprint144Tools.h`
- `editor/src/TextASTSync.h`
- `editor/src/graduation/CppTextDeltaParserBridgeModel.h`
- `editor/src/graduation/ConflictRegionDetectorModel.h`
- `editor/src/graduation/MergePolicyEngineModel.h`

Deliverables:
- Sync tools call real text->AST machinery for supported languages.
- Regeneration tools call real AST->text generator path.
- Merge tools emit real conflict regions and policy outcomes.

Validation:
- Existing step tests: `1663`-`1665`, `1673`-`1675`, `1683`-`1685`.
- New cross-language sync/regenerate/merge integration tests.

### Phase C: Real Toolchain Provider and Diagnostic Integration

Primary files:
- `editor/src/mcp/RegisterSprint151Tools.h`
- `editor/src/BuildSystem.h`
- `editor/src/Diagnostics.h`
- `editor/src/StructuredDiagnostics.h`
- `editor/src/LspOps.h`
- `editor/src/graduation/DiagnosticNormalizationCanonicalModel.h`

Deliverables:
- `whetstone_list_toolchain_providers` reflects runtime-detected providers.
- `whetstone_probe_toolchain_provider` executes probe checks.
- `whetstone_normalize_diagnostics` ingests raw provider/LSP output.

Validation:
- Existing step tests: `1753`, `1754`, `1755`.
- New tests with representative toolchain output fixtures.

### Phase D: Replay/Transaction/GA Evidence from Real Runs

Primary files:
- `editor/src/mcp/RegisterSprint153Tools.h`
- `editor/src/mcp/RegisterSprint154Tools.h`
- `editor/src/mcp/RegisterSprint155Tools.h`
- `editor/src/graduation/ConstructiveTransactionSchema.h`
- `editor/src/graduation/ConstructiveReleaseCertificationPacketModel.h`
- `editor/src/graduation/MCPReplayDiagnosticsArtifact.h`

Deliverables:
- Replay suite compares real run traces.
- Transaction resume/rollback replays actual state transitions.
- GA gate evaluates real determinism and recovery evidence.

Validation:
- Existing step tests: `1773`-`1775`, `1783`-`1785`, `1793`-`1795`.
- New end-to-end replay and rollback scenario tests.

## Taskitem Execution Policy for This Plan

- For each implementation phase, generate taskitems using:
  - `tools/mcp/run_sprint_taskitem_pipeline.sh <phase_plan_or_sprint_plan.md>`
- Export each run:
  - `tools/mcp/export_taskitem_run_for_lora.sh <run_output_dir>`
- Keep `training_data/lora/taskitem_pipeline_runs.jsonl` append-only.

## Immediate Next Slice

Start Phase A:
- Bind `whetstone_run_constructive_step` to concrete language adapters.
- Keep fallback path for unsupported operations.
- Add targeted integration tests for `cpp`, `rust`, `python`, `typescript`.

Parallel planning tranche (active):
- `sprint228_plan.md` to `sprint231_plan.md`
- baseline artifacts:
  - `logs/taskitem_runs/TEST_ONLY_spec_planning_baseline_20260226/fullstack_spec_readiness.json`
  - `logs/taskitem_runs/TEST_ONLY_spec_planning_baseline_20260226/deterministic_spec_readiness.json`

Planning runtime controls (active):
- `sprint232_plan.md` to `sprint237_plan.md`
- semantic bridge + intake augmentation + requirement injection + expansion gating are wired into:
  - `tools/mcp/run_sprint_taskitem_pipeline.sh`
- current policy default is native-first semantic fallback:
  - `WSTONE_SEMANTIC_TASK_EXPANSION_MODE=fallback_only`
- fallback diagnostics + remediation metadata are emitted by:
  - `tools/mcp/analyze_semantic_fallback_gaps.py`
- fallback budget gating is available via:
  - `tools/mcp/check_semantic_fallback_budget.py`
  - `tools/mcp/run_semantic_fallback_budget_gate.sh`
