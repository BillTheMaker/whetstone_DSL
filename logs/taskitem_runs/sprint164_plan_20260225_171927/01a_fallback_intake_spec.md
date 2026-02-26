## Goals
- Step 1864: Recursive object/array rule generator (12 tests)
- Step 1865: Union support (`oneOf`/`anyOf`/`allOf`) (10 tests)
- Step 1866: Constraint encoding and strict scalar handling (10 tests)
- Step 1867: Strictness regression suite (8 tests)
- Step 1868: Sprint 164 Integration Summary (8 tests)

## Constraints
- Keep generator output deterministic
- No removal of existing tools or contract fields
- Unsupported JSON Schema constructs must be explicitly reported, never silently
- Grammar generation runtime remains practical for 347 tools

## Dependencies
- Existing editor/src modules and MCP toolchain
- whetstone_mcp stable binary and workspace config

## Acceptance Criteria
- All sprint step tests pass
- No regression in existing MCP tool behavior
- Generated task queue is ready or blockers are explicit
