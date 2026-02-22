# Next Agent Notes

Before executing sprint work, follow:

- `docs/SPRINT_TASKITEM_EXECUTION_POLICY.md`

## Required Workflow (Do Not Skip)

1. Generate taskitems via MCP pipeline:
   - `tools/mcp/run_sprint_taskitem_pipeline.sh <sprint_plan.md>`
2. Export run to LoRA capture JSONL:
   - `tools/mcp/export_taskitem_run_for_lora.sh <run_output_dir>`
3. For ranges:
   - `tools/mcp/run_sprint_range_with_capture.sh <start> <end>`

## Stable MCP Binary

Use the pinned stable binary path (not dev build path):

- `editor/build-native/whetstone_mcp_stable`

Global MCP config should point to this stable path.
