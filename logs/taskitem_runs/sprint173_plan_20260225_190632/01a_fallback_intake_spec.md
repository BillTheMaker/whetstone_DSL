## Goals
- Step 1907: Lint executor integration (10 tests)
- Step 1908: Lint diagnostics normalization (8 tests)
- Step 1909: Sprint 173 Integration Summary (8 tests)

## Constraints
- Lint status must never be silently dropped
- Missing lint tool must be explicit (`skipped` + reason)

## Dependencies
- Existing editor/src modules and MCP toolchain
- whetstone_mcp stable binary and workspace config

## Acceptance Criteria
- All sprint step tests pass
- No regression in existing MCP tool behavior
- Generated task queue is ready or blockers are explicit
