# MCP Versioning and Compatibility (Agent Guide)

## Why this exists
Agents can fail for reasons that are already fixed in newer MCP runtime builds. This guide defines where to check runtime/tool versions and where fix records are tracked.

## Runtime version header
Whetstone MCP now exposes a `whetstoneVersionHeader` in:
- `initialize.result`
- `ping.result`
- `tools/list.result`
- `tools/call.result`

Header fields:
- `runtimeVersion`: MCP runtime version (for example `0.8.4`)
- `protocolVersion`: MCP protocol date string
- `toolContractDefaultVersion`: default contract version for tools
- `toolSurfaceSchemaVersion`: schema snapshot version
- `toolCount`: tool count in the running server
- `toolSurfaceFingerprint`: deterministic fingerprint of tool names
- `compatibilityLedger.path`: path to the issue/fix ledger JSON
- `compatibilityLedger.version`: ledger schema/snapshot version

## Tool-level contract header
Each `tools/list` entry includes `x-whetstone`:
- `contractVersion`
- `compatibilityLedger`

## Compatibility ledger
Machine-readable ledger:
- `docs/mcp_compatibility_ledger.json`

Use this for checks like:
- "Issue reproduced on `runtimeVersion=0.8.1`"
- "Ledger says fixed in `0.8.4`"
- "Action: update server runtime before more debugging"

## Recommended triage flow for agents
1. Call `initialize` (or `ping`) and capture `whetstoneVersionHeader`.
2. If behavior seems wrong, read `docs/mcp_compatibility_ledger.json` and match symptom tags.
3. If `runtimeVersion < fixed_in`, report upgrade recommendation first.
4. Only escalate to deep debugging after version mismatch is ruled out.

## Notes
- Keep ledger entries small and concrete (symptom, affected tools, fixed version).
- Add entries when a recurring failure pattern is fixed deterministically.
