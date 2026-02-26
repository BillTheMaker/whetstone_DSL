## Goals
- Step 1879: Placeholder/TODO detector and policy (10 tests)
- Step 1880: Compile gate runner for generated snippets/projects (12 tests)
- Step 1881: Test gate runner and minimal harness injection (10 tests)
- Step 1882: MCP output contract update for production gates (8 tests)
- Step 1883: Sprint 167 Integration Summary (8 tests)

## Constraints
- Gate checks must be deterministic and machine-readable
- Failures must include actionable diagnostics and not silently downgrade quality
- “Ready” status is forbidden when placeholders or failed compile/tests remain

## Dependencies
- Existing editor/src modules and MCP toolchain
- whetstone_mcp stable binary and workspace config

## Acceptance Criteria
- All sprint step tests pass
- No regression in existing MCP tool behavior
- Generated task queue is ready or blockers are explicit
