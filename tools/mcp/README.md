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

## Current global MCP config

Global MCP config is expected at:

`/home/bill/Documents/.mcp.json`

and should point to:

`/home/bill/Documents/CLionProjects/whetstone_DSL/editor/build-native/whetstone_mcp_stable`
