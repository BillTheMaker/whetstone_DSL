## Goals
- Step 1904: C++ include fixer from gate diagnostics (10 tests)
- Step 1905: Loop integration for auto-fix attempt (8 tests)
- Step 1906: Sprint 172 Integration Summary (8 tests)

## Constraints
- Missing STL include should not require manual prompt rewrite
- Auto-fix must be auditable and idempotent

## Dependencies
- Existing editor/src modules and MCP toolchain
- whetstone_mcp stable binary and workspace config

## Acceptance Criteria
- All sprint step tests pass
- No regression in existing MCP tool behavior
- Generated task queue is ready or blockers are explicit
