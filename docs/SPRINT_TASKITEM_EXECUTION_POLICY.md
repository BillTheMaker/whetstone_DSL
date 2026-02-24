# Sprint Taskitem Execution Policy

This policy defines how agents must execute sprint plans using MCP taskitem tools
and how run data must be captured for later LoRA curation.

## Scope

- Applies to sprint plan execution (for example: `sprint46_plan.md` through `sprint100_plan.md`)
- Applies to all future agents working in this repository

## Required MCP Pipeline

For each sprint plan, agents must run:

1. `whetstone_architect_intake`
2. `whetstone_generate_taskitems`
3. `whetstone_queue_ready`
4. `whetstone_validate_taskitem`

Use:

`tools/mcp/run_sprint_taskitem_pipeline.sh <sprint_plan.md>`

## Required Data Capture

After each successful run, agents must export run artifacts to LoRA capture JSONL:

`tools/mcp/export_taskitem_run_for_lora.sh <run_output_dir>`

Use:

`training_data/lora/taskitem_pipeline_runs.jsonl`

Note: Raw logs are capture material, not training-ready data. Curation is required
before any fine-tuning.

## Batch Execution

For multiple sprints, agents should use:

`tools/mcp/run_sprint_range_with_capture.sh <start_sprint> <end_sprint>`

Example:

`tools/mcp/run_sprint_range_with_capture.sh 46 100`

## Quality Requirements

- Keep `whetstone_mcp_stable` as execution binary during active development
- Do not use unstable binaries for bulk taskitem generation
- Preserve all run artifacts under `logs/taskitem_runs/`
- Preserve JSONL append-only history under `training_data/lora/`

## Hybrid Contract Gate

To reduce C++-first bias while keeping language-first and AST-first workflows available,
run the hybrid contract validator after sprint execution:

`tools/mcp/validate_hybrid_pipeline_contract.sh --start <N> --end <M>`

Use strict blocking mode for enforcement:

`tools/mcp/validate_hybrid_pipeline_contract.sh --start <N> --end <M> --strict --enforce-non-cpp`

Reference: `docs/HYBRID_PIPELINE_CONTRACT.md`

## Handoff Requirement

When finishing a batch, agents must report:

1. sprint range attempted
2. successful runs count
3. failed runs count
4. batch summary file path
5. LoRA JSONL append confirmation
