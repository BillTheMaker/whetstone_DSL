# Sprint 9 Progress — Agent-First Tooling

## Phase 9a: Standalone MCP Server

### Step 245: HeadlessEditorState (no ImGui dependency)
**Status:** PASS (20/20 tests)

Created the headless agent API surface — same method interface as EditorState
but with zero GUI dependencies (no ImGui, no SDL, no WebSocket).

**Files created:**
- `editor/src/ASTUtils.h` — Pure AST utility functions extracted from EditorUtils.h
  (cloneModule, isAnnotationNode, countAnnotationNodes, findNodeById, findNodeAtPosition)
- `editor/src/HeadlessEditorState.h` — Lightweight state: HeadlessAgentState,
  HeadlessLibraryState, HeadlessBufferState, buffer management, orchestrator sync
- `editor/src/HeadlessAgentRPCHandler.h` — Full RPC dispatch mirroring
  AgentRPCHandler.h (20+ methods: getAST, parseSource, runPipeline, applyMutation,
  applyBatch, projectLanguage, generateCode, workflow recording, etc.)
- `editor/tests/step245_test.cpp` — 20 test cases covering construction, buffer
  management, RPC dispatch, permission enforcement, pipeline execution

**Key design decisions:**
- JSON roundtrip cloning (matching EditorUtils.h) instead of manual deep copy
- Separate HeadlessAgentState (no AgentMarketplace/WebSocket dependency chain)
- All files under 600-line architecture limit

### Step 246: mcp_main.cpp — Standalone MCP Server Entry Point
**Status:** PASS (12/12 tests)

Wires HeadlessEditorState + MCPBridge into a standalone `whetstone_mcp` binary
that any MCP client (Claude Code, Cursor, etc.) can launch over stdio.

**Files created:**
- `editor/src/mcp_main.cpp` — Standalone MCP server entry point with CLI arg
  parsing (--workspace, --language, --verbose), signal handling, resource reader
  lambda routing whetstone:// URIs, and MCPBridge stdio loop
- `editor/tests/step246_test.cpp` — 12 test cases: initialize, notifications,
  tools/list (10+), resources/list (5), prompts/list (4), tools/call pipeline,
  tools/call get_ast, resources/read ast, resources/read diagnostics, ping,
  resources/read settings, CLI arg parsing

**Key design decisions:**
- No SDL, no ImGui, no OpenGL — headless only binary
- Fixed "mcp-session" session ID (MCP stdio is single-client)
- Default Refactor role for full MCP access
- Empty scratch buffer on startup so getAST works immediately
- All logging to stderr (stdout is the MCP transport)

### Step 247: File Operation Tools for MCP
**Status:** PASS (12/12 tests)

Adds 5 file I/O tools to the MCP server so agents can create, read, write,
diff, and list files without a GUI. All operations respect the `--workspace`
root with path-escape security checks.

**Files created:**
- `editor/src/FileOperations.h` — Standalone file ops: resolve path, read,
  write, create (with templates), unified diff, workspace listing via FileTree
- `editor/tests/step247_test.cpp` — 12 test cases: path resolution, path
  escape rejection, file create/read/write via RPC, line range, diff, workspace
  list, glob filter, language templates, parent dir creation, permission denial

**Files modified:**
- `editor/src/AgentPermissionPolicy.h` — fileRead/workspaceList/fileDiff
  read-only; fileWrite/fileCreate require Refactor/Generator
- `editor/src/HeadlessAgentRPCHandler.h` — 5 new RPC handlers
- `editor/src/HeadlessEditorState.h` — include FileOperations.h
- `editor/src/MCPServer.h` — registerFileTools() with 5 MCP tool definitions
- `editor/CMakeLists.txt` — step247_test target

**Key design decisions:**
- Path security: all paths resolved against workspaceRoot, escape rejected
- Simple line-by-line unified diff (no external dependency)
- Language templates for Python, C++, Rust, Go, Java, JS/TS
- FileTree.h reuse for .gitignore-aware workspace listing
- tools/list now returns 15 tools (was 10)

### Step 248: Compact AST Response Format
**Status:** PASS (12/12 tests)

Adds token-efficient AST responses so agents waste fewer tokens on large ASTs.
Compact mode returns a flat list of `{id, type, name, line, children}` nodes
at <30% the size of full AST. Subtree extraction and AST diff (version-based)
let agents query only what changed.

**Files created:**
- `editor/src/CompactAST.h` — toJsonCompact, toJsonCompactTree, toJsonSubtree,
  getNodeName, tokenEstimate, ASTVersionTracker (recordMutation, changedSince,
  buildDiff, pruneOlderThan)
- `editor/tests/step248_test.cpp` — 12 test cases: full backward compat, compact
  flat list, <30% size ratio, compact field validation, tokenEstimate, version
  counter, subtree extraction, invalid nodeId error, empty diff, version increment
  on mutation, diff after mutation, subtree vs full tokenEstimate

**Files modified:**
- `editor/src/HeadlessEditorState.h` — include CompactAST.h, add
  ASTVersionTracker to HeadlessBufferState
- `editor/src/HeadlessAgentRPCHandler.h` — getAST compact param, version and
  tokenEstimate in responses, getASTSubtree and getASTDiff methods, version
  recording in applyMutation/applyBatch
- `editor/src/AgentPermissionPolicy.h` — getASTSubtree/getASTDiff read-only
- `editor/src/MCPServer.h` — whetstone_get_ast compact param, new
  whetstone_get_ast_subtree and whetstone_get_ast_diff tools
- `editor/CMakeLists.txt` — step248_test target

**Key design decisions:**
- Compact mode: flat array of abbreviated nodes (not nested tree)
- Version counter per buffer, incremented on each mutation
- ASTVersionTracker stores affected nodeIds per version for diff
- tokenEstimate = json.dump().size() / 4 (rough LLM token approx)
- tools/list returns 17 tools (was 15): +whetstone_get_ast_subtree, +whetstone_get_ast_diff

### Step 249: MCP Server Integration Tests
**Status:** PASS (8/8 tests)

End-to-end integration tests exercising the full MCP server stack through
MCPBridge. Simulates a realistic agent session from handshake through tool
discovery, AST queries, pipeline execution, resource reads, and file I/O.

**Files created:**
- `editor/tests/step249_test.cpp` — 8 integration test cases:
  1. MCP handshake (initialize + notifications/initialized, verify protocol
     version, server info, capabilities)
  2. tools/list returns all 17 tools with valid schemas (name, description,
     inputSchema with type field)
  3. tools/call whetstone_get_ast on active buffer returns valid AST
  4. tools/call whetstone_run_pipeline: Python → C++ code generation (5393 chars)
  5. resources/read whetstone://diagnostics returns valid JSON array
  6. prompts/list returns all 4 prompts (annotate_module, cross_language_projection,
     security_audit, refactor_memory)
  7. File operations cycle: create → write → read → verify on disk (full CRUD
     through MCP tools/call layer)
  8. Compact AST vs full AST size comparison via MCP (3% ratio — 1,469 vs 39,446 chars)

**Files modified:**
- `editor/CMakeLists.txt` — step249_test target

**Key results:**
- Phase 9a complete: all 5 steps pass (64/64 tests across steps 245–249)
- Full MCP stack validated end-to-end: handshake → tools → resources → prompts → file ops
- Compact AST achieves 97% token savings through MCP layer (3% of full size)
- 17 tools, 5 resources, 4 prompts all verified with correct schemas

---

## Phase 9b: Agent-Optimized Diagnostics

### Step 250: Structured Diagnostic Format
**Status:** PASS (12/12 tests)

Unified diagnostic format for agent consumption. Merges tree-sitter parse
errors, annotation validation diagnostics, and strategy validation violations
into a single stream with error codes, severity levels, AST node references,
and machine-applicable fix mutations.

**Files created:**
- `editor/src/StructuredDiagnostics.h` — Unified diagnostic format:
  - `StructuredDiagnostic` struct: code, severity, nodeId, line, col, message,
    source, fix (optional mutation object)
  - Error code scheme: E01xx (parser), E02xx (annotation), E03xx (strategy)
  - Severity enum: Error(1), Warning(2), Info(3), Hint(4)
  - Collectors: `collectParseDiagnostics`, `collectAnnotationDiagnostics`,
    `collectStrategyDiagnostics`, `collectAllDiagnostics`,
    `collectPipelineDiagnostics`
  - Fix builders: suggest concrete mutations (deleteNode, setProperty, insertNode)
  - Filters: `filterBySeverity`, `filterBySource`
  - JSON serialization: `diagnosticToJson`, `diagnosticsToJson`
- `editor/tests/step250_test.cpp` — 12 test cases: valid code (zero diags),
  parse error codes (E01xx), parse location/severity, annotation validation
  (E02xx), nodeId presence, fix suggestions, getDiagnostics RPC, severity
  filter, JSON field validation, MCP tool registration, pipeline diagnostics,
  source filter

**Files modified:**
- `editor/src/HeadlessEditorState.h` — include StructuredDiagnostics.h
- `editor/src/HeadlessAgentRPCHandler.h` — getDiagnostics RPC method with
  optional severity and source filters
- `editor/src/AgentPermissionPolicy.h` — getDiagnostics as read-only
- `editor/src/MCPServer.h` — whetstone_get_diagnostics tool registration
- `editor/CMakeLists.txt` — step250_test target

**Key design decisions:**
- Error codes are hierarchical: E01xx=parser, E02xx=annotation, E03xx=strategy
- Fix suggestions are concrete mutation objects (same format as applyMutation)
- Severity filtering uses numeric comparison (lower = more severe)
- Source filtering allows isolating parser vs annotation vs strategy diagnostics
- tools/list now returns 18 tools (was 17): +whetstone_get_diagnostics

### Step 251: Quick-Fix Actions as RPC Mutations
**Status:** PASS (12/12 tests)

Turns diagnostic fix suggestions into one-shot RPC calls. `getQuickFixes`
returns concrete mutation objects the agent can review; `applyQuickFix` takes
a diagnostic code + nodeId, applies the fix, and reports whether the
diagnostic cleared.

**Files created:**
- `editor/tests/step251_test.cpp` — 12 test cases: fix discovery, required
  fields, mutation structure, nodeId filtering, apply fix, diagnostic clearing
  report, invalid diagCode error, missing param error, permission enforcement,
  MCP tool registration, missing-return heuristic, category validation

**Files modified:**
- `editor/src/StructuredDiagnostics.h` — QuickFix struct, getQuickFixesForNode,
  getQuickFixesAll, findQuickFix, heuristic fixes (unused variable removal,
  missing return statement)
- `editor/src/HeadlessAgentRPCHandler.h` — getQuickFixes and applyQuickFix
  RPC methods
- `editor/src/AgentPermissionPolicy.h` — getQuickFixes read-only,
  applyQuickFix requires Refactor/Generator
- `editor/src/MCPServer.h` — whetstone_get_quick_fixes and
  whetstone_apply_quick_fix tool registration
- `editor/CMakeLists.txt` — step251_test target

**Key design decisions:**
- QuickFix has unique id, diagCode, nodeId, description, category, mutation
- Fix categories: remove-annotation, change-strategy, resolve-conflict,
  remove-use-after-free, add-deallocation, remove-unused, missing-return
- applyQuickFix delegates to existing applyMutation path for consistency
- After applying, re-runs diagnostics to report if the error cleared
- tools/list now returns 20 tools (was 18): +whetstone_get_quick_fixes,
  +whetstone_apply_quick_fix
