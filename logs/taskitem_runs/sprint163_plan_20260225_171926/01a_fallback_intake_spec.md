## Goals
- Step 1859: Strictness audit utility and baseline report (10 tests)
- Step 1860: Canonical schema normalizer (12 tests)
- Step 1861: Refactor grammar generator to use normalized schema IR (10 tests)
- Step 1862: Top-level strictness gate (8 tests)
- Step 1863: Sprint 163 Integration Summary (8 tests)

## Constraints
- No MCP runtime behavior changes; this sprint is tooling + generation pipeline only
- Generated grammar artifacts remain in `tools/mcp/grammars/`
- Deterministic generation is mandatory (stable ordering)
- All strictness checks must be machine-readable JSON

## Dependencies
- Existing editor/src modules and MCP toolchain
- whetstone_mcp stable binary and workspace config

## Acceptance Criteria
- All sprint step tests pass
- No regression in existing MCP tool behavior
- Generated task queue is ready or blockers are explicit
