# MCP Stability Workflow

Use this when active development builds may break `whetstone_mcp`.

## Files

- Stable symlink: `editor/build-native/whetstone_mcp_stable`
- Versioned binaries: `editor/build-native/releases/whetstone_mcp_YYYYMMDD_HHMMSS`

## Commands

Pin the current known-good build:

```bash
./tools/mcp/pin_working_mcp.sh
```

Promote a newly built binary only if initialize health check passes:

```bash
./tools/mcp/promote_mcp_if_healthy.sh
```

Validate hybrid AST/language pipeline quality for sprint summaries:

```bash
./tools/mcp/validate_hybrid_pipeline_contract.sh --start 50 --end 90
```

## Current global MCP config

Global MCP config is expected at:

`/home/bill/Documents/.mcp.json`

and should point to:

`/home/bill/Documents/CLionProjects/whetstone_DSL/editor/build-native/whetstone_mcp_stable`

## Version and Compatibility Checks

When an agent sees unexpected MCP behavior, check runtime/tool compatibility first:

1. Read `whetstoneVersionHeader` from MCP `initialize`, `ping`, `tools/list`, or `tools/call` response.
2. Compare against `docs/mcp_compatibility_ledger.json`.
3. If issue is marked fixed in a newer runtime, upgrade server runtime before deeper debugging.

Reference docs:
- `docs/mcp_versioning_and_compatibility.md`
- `docs/mcp_compatibility_ledger.json`
