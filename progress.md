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

### Step 252: Diagnostic Delta Streaming
**Status:** PASS (12/12 tests)

Efficient delta computation so agents only see what changed since their
last diagnostic query. `getDiagnosticsDelta` returns added/removed diagnostics
based on a version number, enabling fast mutate → check loops.

**Files created:**
- `editor/tests/step252_test.cpp` — 12 test cases: version tracking,
  empty delta, addition detection, removal detection, response structure,
  version increment, DiagnosticKey comparison, tracker reset, MCP tool,
  structured format in delta, Linter role access

**Files modified:**
- `editor/src/StructuredDiagnostics.h` — DiagnosticKey (code, nodeId, line),
  DiagnosticDelta (added, removed, version), DiagnosticVersionTracker
  (recordSnapshot, getDelta, reset), deltaToJson
- `editor/src/HeadlessEditorState.h` — DiagnosticVersionTracker in
  HeadlessBufferState
- `editor/src/HeadlessAgentRPCHandler.h` — getDiagnosticsDelta RPC method,
  getDiagnostics now records snapshots for delta tracking
- `editor/src/AgentPermissionPolicy.h` — getDiagnosticsDelta as read-only
- `editor/src/MCPServer.h` — whetstone_get_diagnostics_delta tool
- `editor/CMakeLists.txt` — step252_test target

**Key design decisions:**
- DiagnosticKey tuple (code, nodeId, line) uniquely identifies a diagnostic
- Version bumps on each getDiagnostics/getDiagnosticsDelta call
- Delta computed by set difference between previous and current snapshots
- Response includes addedCount/removedCount for quick checks without parsing
- tools/list now returns 21 tools (was 20): +whetstone_get_diagnostics_delta

### Step 253: Diagnostic and Delta Integration Tests
**Status:** PASS (8/8 tests)

Comprehensive end-to-end tests for the full diagnostic pipeline: structured
diagnostics, quick fixes, delta streaming, and filtering. Exercises the
complete flow: introduce error → diagnose → fix → verify clearing.

**Files created:**
- `editor/tests/step253_test.cpp` — 8 integration test cases:
  1. Valid Python → zero diagnostics
  2. Annotation error → structured diagnostics with nodeIds and codes
  3. Quick fixes include concrete mutation objects
  4. applyQuickFix resolves diagnostic (cleared=true, remaining=0)
  5. Delta after fix shows removal (removed=1, added=0)
  6. Delta after new error shows addition (added=1, removed=0)
  7. Combined parser + annotation diagnostics in streams
  8. Severity filtering across the pipeline

**Files modified:**
- `editor/CMakeLists.txt` — step253_test target

**Key results:**
- Phase 9b complete: all 4 steps pass (44/44 tests across steps 250–253)
- Full diagnostic pipeline validated: diagnose → fix → verify → delta
- 21 MCP tools, structured error codes (E01xx/E02xx/E03xx), delta streaming

---

## Phase 9c: Token-Efficient Agent Protocol

### Step 254: Response Budget System
**Status:** PASS (12/12 tests)

Optional `budget` parameter on query methods. If the response exceeds the
budget, it's truncated with `truncated: true` and a `continuation` token
for paginated follow-up. Arrays are shrunk via binary search; responses
sorted by priority (errors first).

**Files created:**
- `editor/src/ResponseBudget.h` — applyBudget (binary search truncation of
  arrays), applyContinuation (resume from offset), sortDiagnosticsByPriority
- `editor/tests/step254_test.cpp` — 12 test cases: no-budget backward compat,
  compact AST truncation, totalCount/returnedCount, continuation pagination,
  full-mode truncated flag, diagnostics with budget, non-array truncation,
  large budget passthrough, invalid continuation error, priority sorting,
  budget=0 unlimited, multi-page coverage

**Files modified:**
- `editor/src/HeadlessEditorState.h` — include ResponseBudget.h
- `editor/src/HeadlessAgentRPCHandler.h` — getAST and getDiagnostics gain
  optional budget parameter with truncation
- `editor/CMakeLists.txt` — step254_test target

**Key design decisions:**
- Budget in chars (not tokens) — simple, predictable, cheap to compute
- Binary search finds max array elements that fit within budget
- Continuation token format: "field:offset:total" (opaque to agent)
- Diagnostics sorted by severity before truncation (errors first)
- budget=0 or omitted → unlimited (backward compatible)

### Step 255: Symbol-Only Mode for Scope Queries
**Status:** PASS (12/12 tests)

Lean vs detailed mode for getInScopeSymbols, getCallHierarchy, and
getDependencyGraph. Default (lean) mode returns minimal symbol data
(name, kind, nodeId); `detailed: true` includes full node JSON.

**Files created:**
- `editor/tests/step255_test.cpp` — 12 test cases: lean scope (no node data),
  detailed scope (with node JSON), count field, size comparison, lean call
  hierarchy (names + IDs), detailed call hierarchy (node arrays), lean deps
  (ID list + count), detailed deps (full nodes), both modes valid output,
  field validation (name/kind/nodeId), node concept field, functionName

**Files modified:**
- `editor/src/HeadlessAgentRPCHandler.h` — getInScopeSymbols, getCallHierarchy,
  getDependencyGraph gain `detailed` parameter; lean mode returns symbols-only,
  detailed mode includes full node JSON via toJson()
- `editor/CMakeLists.txt` — step255_test target

**Key design decisions:**
- Default mode is "symbols" (lean) — agents get names/IDs without node JSON
- `detailed: true` adds full node serialization for each symbol/caller/callee/dep
- Lean scope responses ~7% the size of detailed (302 vs 4329 chars)
- getDependencyGraph lean mode converts vector<string> IDs to JSON array
- Uses getNodeName() from CompactAST.h for cross-type name extraction

### Step 256: Batch Query Endpoint
**Status:** PASS (12/12 tests)

`batchQuery` method accepts an array of sub-queries, executes each
independently, and returns all results in a single round-trip. Errors
in one sub-query don't affect others.

**Files created:**
- `editor/tests/step256_test.cpp` — 12 test cases: 3-query batch, method fields,
  result fields, error isolation, AST+diagnostics+scope batch, empty batch,
  missing params error, budget passthrough, single-envelope multi-result,
  permission-denied isolation, MCP registration, lean+detailed mixed batch

**Files modified:**
- `editor/src/HeadlessAgentRPCHandler.h` — batchQuery RPC method: iterates
  sub-queries, calls handleHeadlessAgentRequest recursively, collects
  results/errors independently
- `editor/src/AgentPermissionPolicy.h` — batchQuery allowed for all roles
  (sub-queries enforce their own permissions)
- `editor/src/MCPServer.h` — registerBatchTools() with whetstone_batch_query
  tool definition and callWhetstone handler
- `editor/CMakeLists.txt` — step256_test target

**Key design decisions:**
- Sub-queries reuse the same dispatch function (recursive call)
- Each sub-query result tagged with its method name for correlation
- Error isolation: failed sub-query returns error object, others unaffected
- batchQuery itself requires no special permission; sub-queries enforce their own
- tools/list now returns 22 tools (was 21): +whetstone_batch_query

### Step 257: Token Efficiency Tests + Benchmarks
**Status:** PASS (12/12 tests)

Phase 9c closer. Systematic measurement of token savings across all
compact/lean/delta/budget/batch features, plus throughput benchmarking.

**Files created:**
- `editor/tests/step257_test.cpp` — 12 test cases:
  1. Compact vs full AST ratio for 10-function module (7%)
  2. Compact vs full AST ratio for 50-function module (6%)
  3. Compact vs full AST ratio for 200-function module (6%)
  4. Lean scope vs detailed scope ratio (4%, under 20% target)
  5. Diagnostic delta vs full diagnostics validation
  6. Budget pagination across 3 pages covers all 51 nodes
  7. Batch query vs sequential bytes (single-envelope efficiency)
  8. Token estimates: compact < full (1001 vs 15039)
  9. Lean call hierarchy + lean deps both produce valid output
  10. Combined efficiency: batch + compact + lean + budget = 95% savings
  11. Benchmark: 100 parse→mutate→diagnose cycles in 8ms (0ms/cycle)
  12. Compact AST ratio stable across module sizes (spread=1%)

**Files modified:**
- `editor/CMakeLists.txt` — step257_test target

**Key results:**
- Phase 9c complete: all 4 steps pass (48/48 tests across steps 254–257)
- Compact AST: 6–7% of full AST size across all module sizes (93–94% savings)
- Lean scope: 4% of detailed size (96% savings)
- Combined optimized query path: 95% total token savings vs naive approach
- Compact ratio stable across 10/50/200 function modules (1% spread)
- Throughput: 0ms/cycle for parse→mutate→diagnose (sub-millisecond)

---

## Phase 9d: Multi-File Project Support

### Step 258: Project Model and Workspace Indexing
**Status:** PASS (12/12 tests)

HeadlessEditorState gains project awareness: `openFile`, `closeFile`,
`listBuffers`, `setActiveBuffer` RPC methods for multi-buffer management,
plus `indexWorkspace` for workspace-level file scanning. Language
auto-detected from file extension.

**Files created:**
- `editor/src/ProjectState.h` — WorkspaceIndex (scan, findByExtension,
  findByLanguage, findByRelativePath, fileCount/dirCount), detectLanguage,
  IndexedFile struct, ProjectState wrapper
- `editor/tests/step258_test.cpp` — 12 test cases: openFile buffer creation,
  listBuffers with 3 files, setActiveBuffer switches AST context, closeFile
  removes buffer, language auto-detection, active flag in listBuffers,
  workspace indexing (5 files, 2 dirs), openFile reads from disk, closeFile
  error on nonexistent, Linter permission denied, MCP tool registration
  (5 new tools, 27 total), full open→switch→query→close workflow

**Files modified:**
- `editor/src/HeadlessEditorState.h` — include ProjectState.h, add
  ProjectState member
- `editor/src/HeadlessAgentRPCHandler.h` — openFile, closeFile, listBuffers,
  setActiveBuffer, indexWorkspace RPC methods
- `editor/src/AgentPermissionPolicy.h` — listBuffers/setActiveBuffer/
  indexWorkspace read-only; openFile/closeFile require Refactor/Generator
- `editor/src/MCPServer.h` — registerProjectTools() with 5 MCP tool
  definitions (whetstone_open_file, whetstone_close_file,
  whetstone_list_buffers, whetstone_set_active_buffer,
  whetstone_index_workspace)
- `editor/CMakeLists.txt` — step258_test target

**Key design decisions:**
- openFile auto-detects language from extension (detectLanguage)
- openFile reads from disk if no content provided (uses fileOpsResolvePath)
- listBuffers includes path, language, modified, and active flag per buffer
- indexWorkspace scans using FileTree (respects .gitignore) without opening files
- WorkspaceIndex provides findByExtension/findByLanguage for agent queries
- tools/list now returns 27 tools (was 22): +5 project management tools

### Step 259: Cross-File Symbol Resolution
**Status:** PASS (12/12 tests)

`getInScopeSymbols` gains `crossFile: true` parameter to include exported
symbols from all other open buffers. Import graph tracks which files import
which, updated automatically on openFile/closeFile. `getImportGraph` RPC
method exposes the graph.

**Files created:**
- `editor/tests/step259_test.cpp` — 12 test cases: baseline local scope,
  crossFile includes other buffers' exports, cross-file symbols have file
  path, import graph populated on openFile, full import graph across 3 files,
  importedBy reverse query, cross-file detailed mode with node JSON,
  cross-file count > local count, closeFile removes from import graph,
  single buffer crossFile=local, collectExportedSymbols extraction,
  Linter role can read cross-file data

**Files modified:**
- `editor/src/ProjectState.h` — ImportGraph (addEdge, clearFile, importsOf,
  importedBy, updateFromAST, updateFromSource), ExportedSymbol struct,
  collectExportedSymbols, ProjectState gains ImportGraph member
- `editor/src/HeadlessAgentRPCHandler.h` — getInScopeSymbols gains
  crossFile parameter (scans all bufferStates for exports), getImportGraph
  RPC method (per-file or full graph), openFile updates import graph,
  closeFile clears import graph entry
- `editor/src/AgentPermissionPolicy.h` — getImportGraph as read-only
- `editor/CMakeLists.txt` — step259_test target

**Key design decisions:**
- Cross-file symbols include `file` field identifying source buffer path
- Import graph built from AST Import nodes + source text fallback
  (Python parser doesn't generate Import AST nodes, so text scan catches
  `import foo` and `from foo import bar` patterns)
- Import graph auto-maintained: updated on openFile, cleared on closeFile
- getImportGraph supports per-file query (imports + importedBy) or full graph
- Cross-file detailed mode resolves node JSON from the source buffer's AST

### Step 260: Project-Wide Diagnostics
**Status:** PASS (12/12 tests)

`getProjectDiagnostics` returns diagnostics across all open files in one call,
grouped by file path. Includes cross-file errors (E0400 for undefined imports).
Supports optional severity filter and file glob filter.

**Files created:**
- `editor/tests/step260_test.cpp` — 12 test cases: basic result structure,
  cross-file E0400 for undefined import, known import not flagged, severity
  filter (error-only excludes warnings), fileGlob *.py, fileGlob specific file,
  empty project returns 0, multiple files with cross-file errors, diagnostic
  field validation, Linter role access, MCP tool registration, priority sorting

**Files modified:**
- `editor/src/StructuredDiagnostics.h` — added `<set>`, `<sstream>`,
  `ast/Import.h` includes; E04xx error code scheme documented;
  `collectCrossFileDiagnostics()` (AST-based) and
  `collectCrossFileDiagnosticsFromSource()` (text-based fallback) for
  detecting imports of modules not open in the project
- `editor/src/HeadlessAgentRPCHandler.h` — `getProjectDiagnostics` RPC method:
  iterates all bufferStates, collects per-file + cross-file diagnostics,
  deduplicates AST vs source-based cross-file diags, applies severity and
  fileGlob filters, groups by file path
- `editor/src/AgentPermissionPolicy.h` — `getProjectDiagnostics` as read-only
- `editor/src/MCPServer.h` — `whetstone_get_project_diagnostics` tool
  registered in registerDiagnosticTools()
- `editor/CMakeLists.txt` — step260_test target

**Key design decisions:**
- Compact grouped format: `{files: {"/path/foo.py": [{code,line,message}]}}`
- Cross-file deduplication: source-text fallback only adds diags for lines
  not already covered by AST-based cross-file checks
- File glob filter: `*` prefix for suffix match (*.py), otherwise substring
- Severity filter reuses existing `filterBySeverity()` infrastructure
- Diagnostics sorted by priority within each file group
- All roles can call (read-only); 28 MCP tools total

### Step 261: Project-Wide Search and Refactor
**Status:** PASS (12/12 tests)

`searchProject` finds symbol references across all open files by name or nodeId.
`renameSymbol` renames a symbol across all files with preview and apply modes.
Both use recursive AST traversal to find Function definitions, FunctionCalls,
Variable declarations, VariableReferences, and Parameters.

**Files created:**
- `editor/tests/step261_test.cpp` — 12 test cases: cross-file search by name,
  correct result fields, search by nodeId, definition vs call kind distinction,
  preview mode (shows changes without applying), apply mode (renames across 2
  files with verification), parameter/variable reference rename, no-match error,
  Linter can search but not rename, unknown name returns 0, MCP tool
  registration, rename marks buffers as modified

**Files modified:**
- `editor/src/ProjectState.h` — SymbolReference struct, RenameChange struct,
  `collectSymbolRefsRecursive()` and `collectSymbolReferences()` (recursive
  AST search for matching names), `buildRenameChangesRecursive()` and
  `buildRenameChanges()` (builds property-change list for rename operations)
- `editor/src/HeadlessAgentRPCHandler.h` — `searchProject` RPC method (by
  name or nodeId, returns file/line/col/nodeId/kind/context per reference),
  `renameSymbol` RPC method (preview mode returns change list, apply mode
  modifies AST nodes directly, marks buffers modified)
- `editor/src/AgentPermissionPolicy.h` — `searchProject` as read-only,
  `renameSymbol` as mutation (Refactor/Generator only)
- `editor/src/MCPServer.h` — `whetstone_search_project` and
  `whetstone_rename_symbol` tools registered in registerProjectTools()
- `editor/CMakeLists.txt` — step261_test target

**Key design decisions:**
- Recursive tree walk for deep references (FunctionCalls inside function bodies)
- Preview mode returns full change list without applying (agent can review)
- Apply mode directly modifies AST node properties (name, functionName, variableName)
- Rename changes include property name so agent sees what exactly changes
- Buffers marked as modified after rename for save-tracking
- Search returns kind field (definition/call/reference/parameter) to categorize
- 30 MCP tools total

### Step 262: Phase 9d Multi-File Project Integration Tests
**Status:** PASS (8/8 tests)

End-to-end integration tests exercising the full multi-file project workflow
across 3-file Python projects. Simulates a realistic agent session from
file opening through cross-file analysis, search, rename, and batch queries.

**Files created:**
- `editor/tests/step262_test.cpp` — 8 integration test cases:
  1. Open 3 files, listBuffers shows all with correct state
  2. setActiveBuffer switches context, getAST returns correct module
  3. Cross-file symbol resolution (crossFile=true includes other buffers' exports)
  4. Import graph tracks which files import which (main.py→utils+math_ops)
  5. Project-wide diagnostics: undefined import raises cross-file E0400
  6. searchProject finds symbol across 3 files (definition + call kinds)
  7. renameSymbol: preview then apply across 3 files, verified both directions
  8. Full workflow: open→batch(diagnostics+search+buffers)→rename→batch verify

**Files modified:**
- `editor/CMakeLists.txt` — step262_test target

**Key results:**
- Phase 9d complete: all 5 steps pass (56/56 tests across steps 258–262)
- Full multi-file project workflow validated end-to-end
- 30 MCP tools, cross-file symbols, import graph, project diagnostics, search, rename
- Sprint 9 phases 9a–9d complete: 212/212 tests across steps 245–262

---

## Phase 9e: Persistence and Undo/Redo

### Step 263: Save Buffer to Disk
**Status:** PASS (12/12 tests)

`saveBuffer` writes a buffer's editBuf to disk, resolving paths against the
workspace root and creating parent directories as needed. `saveAllBuffers`
saves all modified buffers in one call. Both clear the `modified` flag.

**Files created:**
- `editor/tests/step263_test.cpp` — 12 test cases: write to disk, clear modified
  flag, response fields (path/bytesWritten/success), saveAllBuffers batch save,
  mutation→save→verify content, Linter permission denied, default to active buffer,
  error on unknown buffer, saveAllBuffers clears all flags, returns saved paths,
  MCP tool registration, workspace path resolution with parent dir creation

**Files modified:**
- `editor/src/HeadlessAgentRPCHandler.h` — saveBuffer (resolve path, create
  parent dirs, write editBuf, clear modified, return bytesWritten), saveAllBuffers
  (iterate bufferStates, save modified, return savedCount/skippedCount/paths);
  renameSymbol editBuf regeneration via Pipeline::generate() after AST rename
- `editor/src/AgentPermissionPolicy.h` — saveBuffer/saveAllBuffers/undo/redo
  as mutation methods (Refactor/Generator only)
- `editor/src/MCPServer.h` — registerSaveUndoTools() with 4 MCP tools:
  whetstone_save_buffer, whetstone_save_all_buffers, whetstone_undo, whetstone_redo
- `editor/CMakeLists.txt` — step263_test target

**Key design decisions:**
- Paths resolved against workspaceRoot with escape protection
- Parent directories created automatically (std::filesystem::create_directories)
- saveAllBuffers skips unmodified buffers, reports savedCount/skippedCount
- renameSymbol now regenerates editBuf so saved content reflects renamed code
- 34 MCP tools total (was 30): +saveBuffer, +saveAllBuffers, +undo, +redo

### Step 264: Undo/Redo for Headless Mode
**Status:** PASS (12/12 tests)

State-based undo/redo using `HeadlessUndoStack`. Every mutation (applyMutation,
applyBatch, renameSymbol) automatically records a snapshot (editBuf + AST JSON).
`undo` restores the previous state; `redo` re-applies after undo. New mutations
after undo clear the redo stack.

**Files created:**
- `editor/tests/step264_test.cpp` — 12 test cases: no history error, rename→undo
  restores original, editBuf restoration, undo→redo restores mutation, redo error,
  depth tracking (undoDepth/redoDepth), Linter permission denied, 3 mutations→3
  undos, new mutation clears redo stack, undo sets modified flag, MCP tool
  registration, applyMutation→undo

**Files modified:**
- `editor/src/HeadlessEditorState.h` — HeadlessUndoState (text + astJson),
  HeadlessUndoStack class (record, canUndo/canRedo, undo/redo, undoDepth/redoDepth),
  HeadlessBufferState gains `undoStack` field, openBuffer records initial state
- `editor/src/HeadlessAgentRPCHandler.h` — undo RPC method (restore AST via
  fromJson + sync.setAST, restore editBuf, set modified, return depths),
  redo RPC method (same restoration), post-mutation undoStack.record() calls
  in applyMutation, applyBatch, renameSymbol handlers

**Key design decisions:**
- State-based undo (snapshot text + AST JSON) vs operation-based
- Initial state recorded at openFile (position=0), post-mutation at position=N
- Undo decrements position, redo increments — classic position-based stack
- New mutation truncates redo states (standard undo/redo behavior)
- AST restored via JSON roundtrip (toJson/fromJson) for reliable deep clone
- Response includes undoDepth/redoDepth for agent awareness of stack state

### Step 265: Phase 9e Persistence & Undo/Redo Integration Tests
**Status:** PASS (8/8 tests)

End-to-end integration tests exercising save + undo/redo workflows together.
Validates that save writes the correct version to disk after mutations, undos,
and redos, and that saveAllBuffers respects mixed undo states across buffers.

**Files created:**
- `editor/tests/step265_test.cpp` — 8 integration test cases:
  1. mutate→save writes mutated code to disk
  2. mutate→undo→save writes original code to disk
  3. undo→redo→save writes mutated code back to disk
  4. saveAllBuffers with mixed undo states (a.py mutated, b.py undone)
  5. undo/redo/save cycle: greet→a→b all consistent on disk
  6. MCP has 34 tools with save+undo+redo tools present
  7. undo in memory doesn't affect already-saved file on disk
  8. Full workflow: open→mutate→diagnose→undo→redo→save

**Files modified:**
- `editor/CMakeLists.txt` — step265_test target

**Key results:**
- Phase 9e complete: all 3 steps pass (32/32 tests across steps 263–265)
- Save + undo/redo integration validated end-to-end
- 34 MCP tools total
- Sprint 9 complete: all 5 phases pass (244/244 tests across steps 245–265)

---

# Sprint 10 Progress — Semantic Annotation Taxonomy & Environment Layer

## Phase 10a: Semantic Annotation Core + Sidecar Persistence (Steps 266-268)

**Status:** PASS (32/32 tests) — committed as `976161d`

5 semantic annotation types (IntentAnnotation, ComplexityAnnotation, RiskAnnotation, ContractAnnotation, SemanticTagAnnotation) with JSON roundtrip, compact AST semantic summary, and sidecar persistence (.whetstone/<file>.ast.json).

**Key files:**
- `editor/src/ast/Annotation.h` — 5 annotation class definitions
- `editor/src/ast/Serialization.h` — propertiesToJson/createNode/setPropertiesFromJson
- `editor/src/CompactAST.h` — extractSemanticSummary()
- `editor/src/SidecarPersistence.h` — sidecarPath, save/load, isSemanticAnnotation

---

## Phase 10b: Annotation Agent Interface (Steps 269-271)

**Status:** PASS (32/32 tests) — committed as `a049d60`

High-level RPC methods for agent annotation workflow: setSemanticAnnotation, getSemanticAnnotations, removeSemanticAnnotation, getUnannotatedNodes. MCP prompt templates for annotation guidance.

**Key additions:**
- `HeadlessAgentRPCHandler.h` — 4 new RPC methods
- `MCPServer.h` — 4 MCP tools + 5 annotation prompt templates
- `AgentPermissionPolicy.h` — read/write permission mapping

---

## Phase 10c: Type System, Execution & Scope Annotations (Steps 272-277)

**Status:** PASS (171/171 tests) — committed as `e2d1872`

24 new annotation classes across Subjects 2-4:
- **Subject 2 (Type System):** BitWidth, Endian, Layout, Nullability, Variance, Identity, Mut, TypeState
- **Subject 3 (Concurrency):** Atomic, Sync, ThreadModel, MemoryBarrier, Exec, Blocking, Parallel, Trap, Exception, Panic
- **Subject 4 (Scope):** Binding, Lookup, Capture, Visibility, Namespace, Scope

All wired through Serialization.h (3 dispatch points), CompactAST.h, SidecarPersistence.h, and HeadlessAgentRPCHandler.h setSemanticAnnotation type mapping.

---

## Phase 10d: Shims, Optimization, Meta-Programming & Policy Annotations (Steps 278-283)

**Status:** PASS (117/117 tests) — committed as `090320f`

29 new annotation classes across Subjects 5-8:
- **Subject 5 (Shims):** Intrinsic, Raw, CallingConv, Link, Shim, PointerArithmetic, Opaque, Target, Feature, Original, Mapping
- **Subject 6 (Optimization):** TailCall, Loop, Data, Align, Pack, BoundsCheck, Overflow
- **Subject 7 (Meta-Programming):** Meta, Symbol, Evaluate, Template, Synthetic
- **Subject 8 (Policy):** Policy, Ambiguity, Candidate, Tradeoff, Choice, Decision

Generic fallback added to getSemanticAnnotations RPC for extensible annotation type support.

---

## Phase 10e: Environment Layer (Steps 284-289)

**Status:** PASS (200/200 checks across steps 284–289)

**New files created:**
- `editor/src/EnvironmentSpec.h` — EnvironmentSpec AST node (envId, envVersion, capabilities, constraints, scheduler, memory, bindingTimes, exceptions, ffi), CapabilityRequirement annotation, capability vocabulary (17 known capabilities), validateCapabilities() (E0501 diagnostics), validateEnvAnnotations() (E0502-E0505 diagnostics), getLoweringHints() (env-aware lowering patterns), envSpecToJson/envSpecFromJson helpers
- `editor/src/ast/HostBoundary.h` — HostCall (Expression), ScheduleTask (Statement), ModuleLoad (Statement)

**Core files updated:**
- `Serialization.h` — propertiesToJson/createNode/setPropertiesFromJson for HostCall, ScheduleTask, ModuleLoad, EnvironmentSpec, CapabilityRequirement
- `CompactAST.h` — CapabilityRequirement in extractSemanticSummary, host boundary nodes in getNodeName
- `SidecarPersistence.h` — CapabilityRequirement in isSemanticAnnotation
- `HeadlessAgentRPCHandler.h` — capabilityRequirement type in setSemanticAnnotation, 4 new RPCs: setEnvironment, getEnvironment, validateEnvironment, getLoweringHints
- `AgentPermissionPolicy.h` — getEnvironment/validateEnvironment/getLoweringHints (read-only), setEnvironment (mutation)
- `MCPServer.h` — registerEnvironmentTools() with 4 MCP tool definitions (whetstone_set_environment, whetstone_get_environment, whetstone_validate_environment, whetstone_get_lowering_hints)

**Test files:**
- `step284_test.cpp` — 12 tests (EnvironmentSpec schema, JSON roundtrip, module attachment)
- `step285_test.cpp` — 12 tests (capability vocabulary, validation, CapabilityRequirement)
- `step286_test.cpp` — 12 tests (validateEnvAnnotations E0502-E0505, RPC validate/set/get, Linter permissions, MCP tool registration, env replacement)
- `step287_test.cpp` — 12 tests (getLoweringHints: callback/std_async/coroutine, suppress_dealloc/explicit_delete/raii_cleanup, try_catch/expected, combined hints, RPC method, null safety)
- `step288_test.cpp` — 12 tests (HostCall/ScheduleTask/ModuleLoad construction, JSON roundtrip, createNode, compact AST names, capability validation, nested body children)
- `step289_test.cpp` — 8 integration tests (full env workflow, env switch clears issues, host boundary roundtrip, capability validation e2e, lowering hints change with env, batch env queries, MCP tool count 38+, compact AST with host boundary)

**Key results:**
- Phase 10e complete: all 6 steps pass (200/200 checks across steps 284–289)
- 4 environment RPCs: setEnvironment, getEnvironment, validateEnvironment, getLoweringHints
- 5 environment diagnostics: E0501 (unmet capability), E0502–E0505 (annotation/scheduler incompatibility)
- 3 host boundary AST nodes: HostCall, ScheduleTask, ModuleLoad
- Lowering hint patterns: callback, std_async, coroutine, suppress_dealloc, explicit_delete, raii_cleanup, try_catch, expected, checked_throw
- 38 MCP tools total (was 34): +whetstone_set_environment, +whetstone_get_environment, +whetstone_validate_environment, +whetstone_get_lowering_hints
- Sprint 10 complete: all 5 phases pass (phases 10a–10e)

---

# Sprint 11 Progress — Dial It In

## Phase 11a: Semanno Format + Annotation Codegen (Steps 290-296)

### Step 290: SemannoFormat.h — Comment Format Standard (12 tests)
**Status:** PASS (12/12 tests)

`@semanno:type(key=value)` comment format standard with emitter and parser.
Roundtrip-correct for all 8 annotation subjects + semantic core + environment.

**Key files:**
- `editor/src/SemannoFormat.h` — SemannoEmitter::emit() (67+ annotation types),
  SemannoParser::parse() (comment prefix detection for `//`, `#`, `;;`),
  SemannoParser::isSemannoComment(), value escaping, vector properties
- `editor/tests/step290_test.cpp` — 12 tests: roundtrip for all 8 subjects,
  marker annotations, vector properties, comment prefix parsing, value escaping

### Step 291: ProjectionGenerator — Subject 2-4 Visitors (12 tests)
**Status:** PASS (12/12 tests)

Verifies dispatchGenerate() correctly routes Subject 2-4 annotation types
through SemannoAnnotationImpl visitor methods (Python and C++ generators).

**Key files:**
- `editor/src/SemannoAnnotationImpl.h` — CRTP mixin with virtual inheritance
  from AnnotationVisitorExtended, provides default visitor implementations
  for all 56 annotation types
- `editor/src/ast/ProjectionGenerator.h` — virtual inheritance from
  AnnotationVisitorExtended, dispatchGenerate() template
- `editor/tests/step291_test.cpp` — 12 tests: BitWidth, Endian, Layout,
  Nullability, Variance, Atomic, Sync, Exec, Capture, Visibility dispatch

### Step 292: ProjectionGenerator — Subject 5-8 + Semantic Visitors (12 tests)
**Status:** PASS (12/12 tests)

Verifies Subject 5-8, Semantic Core, and Environment annotation dispatch
through all generators. Exhaustive no-Unknown checks.

**Key files:**
- `editor/tests/step292_test.cpp` — 12 tests: Intrinsic, CallingConv, Target,
  TailCall, Loop, Meta, Synthetic, Policy, Intent, Capability dispatch

### Step 293: Generator Implementations — Python/C++/Rust/Go (12 tests)
**Status:** PASS (12/12 tests)

Validates SemannoAnnotationImpl integration across Python, C++, Rust, and Go
generators. Mixed annotation types, roundtrip verification, commentPrefix().

**Key files:**
- `editor/tests/step293_test.cpp` — 12 tests: type/concurrency/scope/shim/
  optimization/meta/policy/semantic annotations across 4 generators

### Step 294: Generator Implementations — Java/JS/Elisp (12 tests)
**Status:** PASS (12/12 tests)

Validates SemannoAnnotationImpl integration for Java, JavaScript, and Elisp
generators. Prefix verification, annotation output, exhaustive no-Unknown check.

**Key files:**
- `editor/tests/step294_test.cpp` — 12 tests: prefix verification, annotation
  output, roundtrip, exhaustive no-Unknown for all 56+ types

**Infrastructure fixes applied:**
- Virtual inheritance: `ProjectionGenerator : public virtual AnnotationVisitorExtended`
  and `SemannoAnnotationImpl : public virtual AnnotationVisitorExtended` to resolve
  diamond inheritance ambiguity in generator classes
- Removed circular include: `EnvironmentSpec.h` no longer includes `ast/Serialization.h`

### Step 295: Semanno Sidecar Integration (12 tests)
**Status:** PASS (12/12 tests)

Semanno sidecar persistence now supports full save/load roundtrip behavior,
including empty sidecar file handling.

**Key files:**
- `editor/src/SemannoSidecar.h` — implemented sidecar API with structured
  save/load results, path helper, line-keyed `L<line>: @semanno:...` emission,
  and empty-module sidecar creation.
- `editor/src/SidecarPersistence.h` — include fixes for AST helper symbols used
  by sidecar merge logic.
- `editor/tests/step295_test.cpp` — includes sidecar persistence integration
  checks alongside Semanno sidecar tests.

### Step 296: Phase 11a Integration Tests (8 tests)
**Status:** PASS (8/8 tests)

Full Phase 11a integration validation now passes end-to-end.

**Key files:**
- `editor/src/SemannoSidecar.h` — added result-based free-function API used by
  Sprint 11 tests (`semannoSidecarPath`, `saveSemannoSidecar`,
  `loadSemannoSidecar`), path/error/count reporting, and corrected line mapping
  to use parent `spanStartLine`.
- `editor/tests/step296_test.cpp` — integration suite executed and passing.

**Verified checks:**
- Parse/annotate/generate path preserves Subject 2-8 Semanno output.
- Cross-language generators preserve Semanno annotations.
- Exhaustive no-Unknown emission for 56+ annotation types.
- Semanno parse roundtrip for structured properties.
- Mixed legacy + new annotation handling in one module.
- Semanno sidecar save/load integration with expected counts.
- Language comment prefix conventions.
- Marker annotation emission without property parentheses.

---

## Phase 11b: Validation & Conflict Completion (Steps 297-301)

### Step 297: AnnotationValidatorExtended — Subject 2-4 Rules (12 tests)
**Status:** PASS (12/12 tests)

Validates type system (E0600-E0608), concurrency (E0700-E0708), and scope
(E0800-E0805) annotations with structured diagnostic codes.

**Key files:**
- `editor/src/AnnotationValidatorExtended.h` — `AnnotationValidatorExtended::validate()`
  with `isPowerOf2()`, `isOneOf()` helpers, recursive AST walk, `hasThreadModelOnAncestor()`
  for E0704
- `editor/tests/step297_test.cpp` — 12 tests: BitWidth E0600 (invalid + valid),
  Endian E0601, Layout E0602+E0603, Nullability E0604, Variance E0605,
  Atomic E0700, Exec async E0704 (with/without ThreadModel), Capture E0802,
  Visibility E0803, all-valid-no-diags

**Rules implemented:**
- E0600: BitWidth must be power of 2
- E0601: Endian order must be "big" or "little"
- E0602: Layout mode must be "packed" or "aligned"
- E0603: Layout alignment must be power of 2
- E0604: Nullability strategy must be "strict" or "nullable"
- E0605: Variance must be "covariant", "contravariant", or "invariant"
- E0606: Identity mode must be "nominal" or "structural"
- E0607: Mut depth must be "shallow", "deep", or "interior"
- E0608: TypeState must be "erased" or "reified"
- E0700: Atomic consistency must be valid ordering
- E0701: Sync primitive must be "monitor", "spin", or "semaphore"
- E0702: ThreadModel must be "green", "os", or "fiber"
- E0703: Exec mode must be "async" or "event"
- E0704: Exec(async) requires ThreadModel on ancestor
- E0705: Blocking kind must be "io" or "compute"
- E0706: Parallel kind must be "data" or "task"
- E0707: Exception style must be "checked" or "unchecked"
- E0708: Panic behavior must be "abort" or "unwind"
- E0800: Binding time must be "static" or "dynamic"
- E0801: Lookup mode must be "lexical" or "hoisted"
- E0802: Capture strategy must be "value", "ref", or "move"
- E0803: Visibility must be "private", "internal", "friend", or "public"
- E0804: Namespace style must be "qualified" or "flat"
- E0805: Scope kind must be "local", "global_leaked", or "singleton"

### Step 298: AnnotationValidatorExtended — Subject 5-8 Rules (12 tests)
**Status:** PASS (12/12 tests)

Validates shim/FFI (E0900-E0901), optimization (E1000-E1004), meta-programming
(E1100-E1104), and policy (E1200-E1202) annotations.

**Key files:**
- `editor/src/AnnotationValidatorExtended.h` — extended with Subject 5-8 branches
- `editor/tests/step298_test.cpp` — 12 tests: CallingConv E0900, Shim E0901,
  Inline+TailCall E1000 (Always vs Hint), Loop hint E1001, Loop factor E1002,
  Align E1003, Overflow E1004, Meta state E1100, Template E1104,
  Policy strictness E1200, all-valid-no-diags

**Rules implemented:**
- E0900: CallingConv must be "stdcall", "cdecl", or "fastcall"
- E0901: Shim strategy must be "vtable", "trampoline", "union_tag", or "cast"
- E1000: Inline(Always) conflicts with TailCall
- E1001: Loop hint must be "unroll", "vectorize", or "fuse"
- E1002: Loop unroll factor must be positive
- E1003: Align bytes must be positive power of 2
- E1004: Overflow behavior must be "wrap", "saturation", or "panic"
- E1100: Meta state must be "quoted" or "unquoted"
- E1101: Meta phase must be "compile" or "runtime"
- E1102: Symbol mode must be "gensym" or "interned"
- E1103: Evaluate phase must be "compile_time" or "runtime"
- E1104: Template specialization must be "trait", "monomorphize", or "erasure"
- E1200: Policy strictness must be "high" or "low"
- E1201: Policy perf must be "critical" or "normal"
- E1202: Policy style must be "idiomatic" or "literal"

### Step 299: Cross-Type Annotation Conflict Detection (12 tests)
**Status:** PASS (12/12 tests)

Detects semantic conflicts between different annotation families on the same node.
Extends AnnotationConflict.h (which handles same-family parent/child conflicts)
with cross-type detection.

**Key files:**
- `editor/src/AnnotationConflictExtended.h` — `CrossTypeConflict` struct,
  `collectCrossTypeConflicts()` with conditional lambda predicates, recursive
  child traversal
- `editor/tests/step299_test.cpp` — 12 tests: 6 conflict pairs detected,
  4 no-false-positive tests (Capture value, ConstExpr+event, Inline Hint,
  Pack+packed), recursive child detection, multiple conflicts on same node

**Conflict pairs:**
1. @Pure + @Blocking — purity violated by blocking behavior
2. @Atomic + @Sync — conflicting synchronization models
3. @Capture(move) + @Owner(Shared_ARC) — move semantics incompatible with shared ref counting
4. @ConstExpr + @Exec(async) — compile-time eval can't be async
5. @Inline(Always) + @TailCall — inlining defeats tail call optimization
6. @Pack + @Layout(aligned) — packing and alignment contradict

### Step 300: TransformEngineExtended (12 tests)
**Status:** PASS (12/12 tests)

Extended transform engine with float constant folding, dead variable elimination,
and annotation-aware optimization that respects @OptimizationLock and @BoundsCheck.

**Key files:**
- `editor/src/TransformEngineExtended.h` — extends TransformEngine with
  `floatConstantFolding()` (bottom-up BinaryOperation folding on FloatLiterals),
  `deadVariableElimination()` (removes unreferenced Variable declarations),
  `annotationAwareOptimize()` (skips OptimizationLock/BoundsCheck-protected nodes),
  `applyAllExtended()` (runs all 3 transforms)
- `editor/tests/step300_test.cpp` — 12 tests: float add/mul folding, div-by-zero
  safety, dead var removal, used var preservation, OptimizationLock blocking,
  BoundsCheck skipping, nested float fold, multiple dead vars, applyAllExtended,
  nodesModified count accuracy

### Step 301: Phase 11b Integration Tests (8 tests)
**Status:** PASS (8/8 tests)

End-to-end integration tests for the full validation + conflict + transform pipeline.

**Key files:**
- `editor/tests/step301_test.cpp` — 8 integration tests:
  1. Extended E06xx-E12xx codes appear in collectAllDiagnostics
  2. Valid AST produces 0 diagnostics (no regressions)
  3. Cross-type conflicts (E0210) appear in diagnostic output
  4. Structured JSON fields correct (code/severity/nodeId/source)
  5. Combined extended validation + cross-type conflict on same AST
  6. annotationErrorCode extracts embedded [Exxxx] codes correctly
  7. Pipeline wiring: validator + conflict API work together
  8. Exhaustive coverage: all 7 subjects produce diagnostics for invalid input

**Key results:**
- Phase 11b complete: all 5 steps pass (56/56 tests across steps 297–301)
- 33+ annotation types with explicit validation rules (E0600-E1202)
- 6 cross-type conflict pairs detected with conditional predicates
- Float constant folding, dead variable elimination, annotation-aware optimization
- No regressions on existing E01xx-E05xx diagnostics

---

## Phase 11c: Parser Deepening (Steps 302-308)

### Step 302: New AST Nodes — Class/Interface/Generic (12 tests)
**Status:** PASS (12/12 tests)

5 new AST node types for structured OOP and generic programming support.
Full JSON roundtrip via Serialization.h (propertiesToJson, createNode,
setPropertiesFromJson).

**Key files:**
- `editor/src/ast/ClassDeclaration.h` — ClassDeclaration (name, superClass,
  isAbstract; children: interfaces, fields, methods, annotations),
  InterfaceDeclaration (name; children: methods, annotations),
  MethodDeclaration (extends Function with className, isStatic, visibility,
  isOverride, isVirtual)
- `editor/src/ast/GenericType.h` — GenericType (baseName; children: typeParameters),
  TypeParameter (name, constraint)
- `editor/src/ast/Serialization.h` — propertiesToJson/createNode/setPropertiesFromJson
  for all 5 new types
- `editor/tests/step302_test.cpp` — 12 tests: construction + properties for all 5
  types, child roles (methods/fields/interfaces), MethodDeclaration inherits Function
  children (parameters/body), JSON roundtrip for all 5 types, nested class structure
  with generic + method + field roundtrip

### Step 303: New AST Nodes — Async/Lambda/Decorator (12 tests)
**Status:** PASS (12/12 tests)

4 new AST node types for modern language constructs: async/await, lambdas/closures,
and decorator annotations. Full JSON roundtrip via Serialization.h.

**Key files:**
- `editor/src/ast/AsyncNodes.h` — AsyncFunction (extends Function, isAsync flag),
  AwaitExpression (children: expression), LambdaExpression (captureList vector;
  children: parameters, body), DecoratorAnnotation (name; children: arguments)
- `editor/src/ast/Serialization.h` — propertiesToJson/createNode/setPropertiesFromJson
  for all 4 new types (captureList as JSON array)
- `editor/tests/step303_test.cpp` — 12 tests: construction for all 4 types,
  child roles (parameters/body/expression/arguments), JSON roundtrip for all 4,
  captureList array preservation, async+await nested in module roundtrip,
  lambda with captures inside function body roundtrip

### Step 304: Serialization + Dispatch for New Nodes (12 tests)
**Status:** PASS (12/12 tests)

All 9 new AST node types (ClassDeclaration, InterfaceDeclaration, MethodDeclaration,
GenericType, TypeParameter, AsyncFunction, AwaitExpression, LambdaExpression,
DecoratorAnnotation) dispatch through PythonGenerator and CppGenerator with
language-appropriate output. JSON roundtrip preserves dispatch output identity.

**Key files:**
- `editor/src/ast/PythonGenerator.h` — 9 visitor implementations: class→`class Name(Super):`,
  interface→`class Name(ABC):`, method→`def name(self):`, generic→`Name[T]`,
  async→`async def name():`, await→`await expr`, lambda→`lambda x: body`,
  decorator→`@name(args)`
- `editor/src/ast/CppGeneratorTypes.h` — 9 visitor implementations: class→`class Name : public Super {}`,
  interface→`class Name {}`, method→`virtual void name()`, generic→`Name<T>`,
  async→`std::future<void> name()`, await→`co_await expr`, lambda→`[captures](auto x) { body }`,
  decorator→`// @name`
- `editor/src/ast/ProjectionGenerator.h` — dispatchGenerate branches for all 9 types (lines 322-339)
- `editor/tests/step304_test.cpp` — 12 tests: per-type Python+C++ dispatch (9 types),
  nested class-with-methods dispatch, async-with-await nested dispatch,
  all-9-types JSON roundtrip→dispatch identity check

### Step 305: Python + Java Parser Deepening (12 tests)
**Status:** PASS (12/12 tests)

Python: class, async def, await, lambda, @decorator.
Java: class, interface, generics.

### Step 306: JS/TS + Rust Parser Deepening (12 tests)
**Status:** PASS (12/12 tests)

JS/TS: class, async/await, arrow functions.
Rust: struct, impl, trait, async fn, closures.

### Step 307: Go + C++ + Elisp Parser Deepening (12 tests)
**Status:** PASS (12/12 tests)

Go: struct/interface, method receivers.
C++: class/struct, templates, virtual methods, lambdas.
Elisp: lambda forms.

### Step 308: Generator Updates + Phase 11c Integration (8 tests)
**Status:** PASS (8/8 tests)

All 8 generators now produce language-appropriate output for all 9 new AST
node types (ClassDeclaration, InterfaceDeclaration, MethodDeclaration,
GenericType, TypeParameter, AsyncFunction, AwaitExpression, LambdaExpression,
DecoratorAnnotation).

**Files modified:**
- `editor/src/ast/RustGenerator.h` — 9 new visitor methods: struct/impl, trait,
  fn(&self), async fn, .await, |closures|, // @decorator
- `editor/src/ast/GoGenerator.h` — 9 new visitor methods: type struct, type interface,
  func receiver, func (goroutine async), <-channel await, func() lambda
- `editor/src/ast/JavaGenerator.h` — 9 new visitor methods: class extends, interface,
  public void method, List<T>, CompletableFuture async, .get() await, (x) -> lambda, @annotation
- `editor/src/ast/JavaScriptGenerator.h` — 9 new visitor methods: class extends,
  class (interface fallback), method(), Name<T>, async function, await, (x) => arrow, // @decorator
- `editor/src/ast/ElispGenerator.h` — 9 new visitor methods: cl-defstruct, cl-defgeneric,
  cl-defmethod, baseName (dynamic types), defun ;; async, (async-get), (lambda), ; @decorator
- `editor/tests/step308_test.cpp` — 8 integration tests
- `editor/CMakeLists.txt` — step308_test target

**Key results:**
- Phase 11c complete: all 7 steps pass (80/80 tests across steps 302-308)
- All 8 generators produce non-empty, language-appropriate output for all 9 new AST types
- Language idioms verified: Rust struct+impl, Go type struct, Java extends/implements,
  JS extends/arrow, Python ABC/async def/lambda, C++ virtual/co_await/captures, Elisp defstruct/lambda
- JSON roundtrip → generate identity verified for ClassDeclaration
- TypeScript inherits JavaScript generator implementations

## Phase 11d: New Languages — Kotlin + C# (Steps 309-313)

### Step 309: Kotlin Parser (12 tests)
**Status:** PASS (12/12 tests)

KotlinParser (regex-based, standalone) parses: fun → Function, suspend fun →
AsyncFunction, class/data class → ClassDeclaration, val/var → Variable.
Module target language set to "kotlin".

### Step 310: Kotlin Generator (12 tests)
**Status:** PASS (12/12 tests)

KotlinGenerator produces idiomatic Kotlin: fun, val, suspend fun, class : Super(),
interface, { params -> body } lambdas, @decorator, generic types.
Type mappings: int→Int, string→String, bool→Boolean, void→Unit, double→Double.
Collection types: List<>, Set<>, Map<>, Array<>, Pair<>.

### Step 311: C# Parser (12 tests)
**Status:** PASS (12/12 tests)

CSharpParser (regex-based, standalone) parses: methods with visibility/return-type
detection, async methods → AsyncFunction, class → ClassDeclaration, interface →
InterfaceDeclaration. Skips using directives and namespace wrappers.

### Step 312: C# Generator (12 tests)
**Status:** PASS (12/12 tests)

CSharpGenerator produces idiomatic C#: public void, Allman braces, foreach..in,
class : Base, interface, public async Task, await, (x) => lambda, [Attribute].
Type mappings: int, float, double, string, bool, void.
Collection types: List<>, HashSet<>, Dictionary<>.

### Step 313: New Languages Integration + MCP (8 tests)
**Status:** PASS (8/8 tests)

Full pipeline integration for Kotlin + C#:
- Pipeline.parse() and Pipeline.generate() route correctly for both languages
- Cross-language projection verified (Python→Kotlin, Java→C#)
- MemoryStrategyInference runs without error on Kotlin/C# modules
- All 10 generators produce non-empty output
- Final count: 10 parsers, 10 generators

**Files created:**
- `editor/tests/step309_test.cpp` — 12 Kotlin parser tests
- `editor/tests/step310_test.cpp` — 12 Kotlin generator tests
- `editor/tests/step311_test.cpp` — 12 C# parser tests
- `editor/tests/step312_test.cpp` — 12 C# generator tests
- `editor/tests/step313_test.cpp` — 8 integration tests

**Files modified:**
- `editor/src/ast/KotlinGenerator.h` — 9 new AST node visitors (class, interface,
  method, generic, type param, suspend fun, await, lambda, @decorator)
- `editor/src/ast/CSharpGenerator.h` — 9 new AST node visitors (class, interface,
  method, generic, type param, async Task, await, lambda, [Attribute])
- `editor/src/Pipeline.h` — fixed Kotlin/C# parser routing (KotlinParser/CSharpParser
  instead of TreeSitterParser)
- `editor/CMakeLists.txt` — 5 new test targets (steps 309-313)

**Key results:**
- Phase 11d complete: all 5 steps pass (56/56 tests across steps 309-313)
- 10 language parsers, 10 language generators fully operational
- No regressions: Phase 11c (80/80) still passes
- Total Phase 11c+11d: 136/136 tests

---

# Sprint 12 — Workflow Model + C++ Depth

## Phase 12a: Core Workflow Model

### Step 320: WorkItem — Core Execution Model
**Status:** PASS (12/12 tests)

WorkItem extends SkeletonTask with execution lifecycle tracking — status
transitions, worker assignment, timestamps, and results. This is the unit of
work that flows through the routing engine.

**Files created:**
- `editor/src/WorkItem.h` — WorkItemResult struct (generatedCode, astJson,
  diagnostics, confidence, tokens, reasoning + JSON roundtrip), WorkItem struct
  (identity, routing, lifecycle, assignment, result), state machine helpers
  (isTerminal, canTransition, transitionWorkItem), createWorkItem from
  SkeletonTask, priorityToInt ordering, full JSON serialization
- `editor/tests/step320_test.cpp` — 12 tests: construction from SkeletonTask,
  unique ID generation, valid/invalid state transitions, timestamp population,
  WorkItemResult roundtrip, WorkItem roundtrip, isTerminal, createdAt,
  dependencies preserved, priority ordering, result attachment roundtrip

**Files modified:**
- `editor/CMakeLists.txt` — step320_test target

**State machine:** pending→ready→assigned→in-progress→(review→complete|rejected OR complete); rejected→ready

### Step 321: TaskQueue — Priority Queue with Dependencies
**Status:** PASS (12/12 tests)

Ordered queue that respects both priority levels and dependency chains.
Items whose dependencies are unsatisfied stay blocked; ready items are ordered
by priority (critical first) then creation time.

**Files created:**
- `editor/src/TaskQueue.h` — TaskQueue class with enqueue, dequeue, peek,
  complete, reject, getReady, getBlocked, getByStatus, getItem, updateItem;
  dependency resolution on completion, priority-based sorting
- `editor/tests/step321_test.cpp` — 12 tests: enqueue/dequeue ordering,
  priority ordering, dependency blocking, reject/re-enqueue, getReady
  filtering, empty queue, getByStatus, size tracking, completion cascading
  (A→B→C chain), mixed priorities with deps, peek no-remove, updateItem

**Files modified:**
- `editor/CMakeLists.txt` — step321_test target

### Step 322: WorkflowState — Project-Level Workflow Tracking
**Status:** PASS (12/12 tests)

Top-level state managing the full workflow lifecycle for a project: from skeleton
creation through routing, execution, review, and completion. Auto-detects phase
from item statuses, tracks audit trail, computes stats.

**Files created:**
- `editor/src/WorkflowState.h` — WorkflowPhase enum (Modeling/Routing/Executing/
  Reviewing/Complete), StatusChange audit trail struct, WorkflowStats struct,
  WorkflowState class with populateFromSkeleton, getStats, getPhase (auto-computed),
  getHistory, recordChange, toJson/fromJson
- `editor/tests/step322_test.cpp` — 12 tests: populate from skeleton, phase
  auto-detection (empty, all pending, executing, reviewing, complete), stats
  accuracy, history tracking, JSON roundtrip, empty workflow, phase transitions
  through lifecycle, populate records history

**Files modified:**
- `editor/CMakeLists.txt` — step322_test target

### Step 323: Workflow Sidecar Persistence
**Status:** PASS (12/12 tests)

Save and load workflow state to `.whetstone/<project>.workflow.json` so
workflows survive across sessions. Handles directory creation, overwrite,
delete, and multiple workflows in the same workspace.

**Files created:**
- `editor/src/WorkflowPersistence.h` — SaveResult struct, workflowSidecarPath,
  ensureDirectoryExists, saveWorkflow (serialize + write), loadWorkflow
  (read + deserialize, nullopt if missing), deleteWorkflow
- `editor/tests/step323_test.cpp` — 12 tests: save/load roundtrip, all fields
  preserved, history preserved, missing file nullopt, directory creation,
  delete removes file, multiple workflows, results preserved, empty workflow,
  sidecar path format, overwrite, delete nonexistent

**Files modified:**
- `editor/CMakeLists.txt` — step323_test target

### Step 324: Workflow RPC + MCP Tools
**Status:** PASS (12/12 tests)

Exposes workflow management through 8 RPC methods and 8 MCP tools so agents
can create, inspect, and advance workflows. Linter role restricted to read-only
access; Refactor/Generator can mutate.

**Files created:**
- `editor/tests/step324_test.cpp` — 12 tests: createWorkflow, getReadyTasks
  ordering, assignTask transitions, completeTask with result + cascade,
  rejectTask re-enqueue, getWorkItem details, Linter restrictions, MCP
  registration (8 new tools, 50+ total), saveWorkflow RPC, getWorkflowState,
  no-workflow error, nonexistent item error

**Files modified:**
- `editor/src/HeadlessEditorState.h` — added WorkflowState/WorkflowPersistence
  includes, optional<WorkflowState> workflow member
- `editor/src/HeadlessAgentRPCHandler.h` — 8 new RPC methods: createWorkflow,
  getWorkflowState, getReadyTasks, getWorkItem, assignTask, completeTask,
  rejectTask, saveWorkflow
- `editor/src/AgentPermissionPolicy.h` — read-only: getWorkflowState,
  getReadyTasks, getWorkItem; mutation: createWorkflow, assignTask,
  completeTask, rejectTask, saveWorkflow
- `editor/src/MCPServer.h` — registerWorkflowExecutionTools() with 8 tools
- `editor/CMakeLists.txt` — step324_test target

**Tool count:** 50+ (42 existing + 8 workflow execution tools)

### Step 325: Phase 12a Integration Tests
**Status:** PASS (8/8 tests)

End-to-end workflow lifecycle from skeleton to completion. Covers priority
ordering, dependency chains (A→B→C cascade), full lifecycle with results,
rejection flow, persistence roundtrip, stats accuracy, and a combined
5-function workflow with mixed priorities and dependencies.

**Files created:**
- `editor/tests/step325_test.cpp` — 8 integration tests: skeleton→workflow,
  priority ordering, dependency chain cascade, full lifecycle with result,
  rejection flow, persistence save/load, stats accuracy, combined 5-item
  workflow completing to phase=Complete

**Files modified:**
- `editor/src/TaskQueue.h` — updateItem triggers resolveDependencies when
  item status becomes complete (fixes cascade through RPC path)
- `editor/CMakeLists.txt` — step325_test target

**Phase 12a complete:** 6/6 steps (320-325), 68/68 tests passing

## Phase 12b: Routing Engine + Workers

### Step 326: RoutingEngine — Annotation-to-Dispatch Logic
**Status:** PASS (12/12 tests)

Given a WorkItem with routing annotations, produces a RoutingDecision with
worker type, context width, token budget, review requirements, confidence,
and reasoning. Priority cascade: explicit override → ambiguity gate →
context width escalation → getter/setter patterns → defaults.

**Files created:**
- `editor/src/RoutingEngine.h` — RoutingDecision struct (with JSON helpers),
  estimateContextBudget, agentRoleForWorker, isGetterSetterPattern, RoutingEngine
  class with route, routeBatch, routeAndApply
- `editor/tests/step326_test.cpp` — 12 tests: explicit override, ambiguity→human,
  getter/setter→deterministic, cross-project→llm, project→llm, review annotation,
  batch routing, default local/file, confidence levels, context budgets

**Files modified:**
- `editor/CMakeLists.txt` — step326_test target

### Step 327: WorkerRegistry — Worker Abstractions
**Status:** PASS (12/12 tests)

Worker interface with 5 concrete implementations: DeterministicWorker
(intent-based code gen), TemplateWorker (getter/setter/accessor patterns),
SLMAgentWorker/LLMAgentWorker (context bundle preparation for external
invocation), HumanWorker (marks for human review). Registry with lookup.

**Files created:**
- `editor/src/WorkerRegistry.h` — WorkerContext struct, WorkerInterface
  abstract base, DeterministicWorker, TemplateWorker, SLMAgentWorker,
  LLMAgentWorker, HumanWorker, WorkerRegistry with getDefaultRegistry
- `editor/tests/step327_test.cpp` — 12 tests: deterministic with/without
  intent, template getter/setter/boolean accessor, LLM context bundle,
  human review marking, registry lookup, canHandle filtering, worker type
  strings, SLM context assembly, rejection feedback in context

**Files modified:**
- `editor/CMakeLists.txt` — step327_test target

### Step 328: ContextAssembler — Context Window Assembly + Budget
**Status:** PASS (12/12 tests)

Builds context windows per @ContextWidth (local/file/project/cross-project)
with token budget enforcement. Prioritizes: target node > annotations >
siblings > buffer content > project summaries under truncation.

**Files created:**
- `editor/src/ContextAssembler.h` — BufferInfo struct, estimateTokens helpers,
  ContextAssembler class with assembleContext (local/file/project/cross-project
  strategies), budget truncation, sibling/import graph inclusion
- `editor/tests/step328_test.cpp` — 12 tests: local/file/project/cross-project
  context, budget truncation, priority under truncation, import graph,
  empty project fallback, token estimation, unlimited budget, siblings,
  rejection feedback preservation

**Files modified:**
- `editor/CMakeLists.txt` — step328_test target

### Step 329: Routing RPC + MCP Tools
**Status:** PASS (12/12 tests)

Exposes routing and worker dispatch through 4 RPC methods and 4 MCP tools.
routeTask/routeAllReady apply routing decisions, executeTask runs workers
(deterministic/template produce code, agent/human prepare context bundles),
getRoutingExplanation shows reasoning.

**Files created:**
- `editor/tests/step329_test.cpp` — 12 tests: routeTask, routeAllReady batch,
  execute deterministic, agent prepared, human marked, routing explanation,
  Linter restrictions, MCP registration (4 new, 54+ total), route updates item,
  no-worker error, itemIds in batch, auto-approved for template

**Files modified:**
- `editor/src/HeadlessEditorState.h` — added RoutingEngine, WorkerRegistry,
  ContextAssembler members
- `editor/src/HeadlessAgentRPCHandler.h` — 4 new RPC methods: routeTask,
  routeAllReady, executeTask, getRoutingExplanation
- `editor/src/AgentPermissionPolicy.h` — read-only: getRoutingExplanation;
  mutation: routeTask, routeAllReady, executeTask
- `editor/src/MCPServer.h` — registerRoutingTools() with 4 tools
- `editor/src/RoutingEngine.h` — getter/setter routes to "template" (not
  "deterministic") for correct auto-approve with TemplateWorker
- `editor/CMakeLists.txt` — step329_test target

**Tool count:** 54+ (50 existing + 4 routing tools)

### Step 330: Review Gates
**Status:** PASS (12/12 tests)

Configurable auto-approve rules and review queue. Default policy auto-approves
deterministic/template workers with >=0.9 confidence; everything else requires
review. Explicit @Review(required) always overrides auto-approve.

**Files created:**
- `editor/src/ReviewGate.h` — AutoApproveRule, ReviewPolicy (with default),
  ReviewDecision, ReviewGate with shouldAutoApprove, risk level helpers
- `editor/tests/step330_test.cpp` — 12 tests: deterministic/template auto-approve,
  LLM requires review, low confidence rejected, custom policy, default policy,
  @Review override, wildcard rule, policy roundtrip, empty policy, default
  require-review, human requires review

**Files modified:**
- `editor/src/HeadlessEditorState.h` — ReviewGate + ReviewPolicy members
- `editor/src/HeadlessAgentRPCHandler.h` — setReviewPolicy, getReviewPolicy RPCs
- `editor/src/AgentPermissionPolicy.h` — getReviewPolicy read-only,
  setReviewPolicy mutation
- `editor/src/MCPServer.h` — registerReviewTools() with 2 tools
- `editor/CMakeLists.txt` — step330_test target

**Tool count:** 56+ (54 existing + 2 review tools)

### Step 331: Phase 12b Integration Tests
**Status:** PASS (8/8 tests)

Full routing pipeline integration tests covering skeleton→workflow→route→execute→review
lifecycle. Tests verify mixed worker type routing, getter template auto-approve,
LLM context preparation, human routing for high-ambiguity, dependency chains with
mixed routing, rejection with feedback flow, review policy switching, and 5-function
mixed skeleton lifecycle.

**Files created:**
- `editor/tests/step331_test.cpp` — 8 integration tests: skeleton_route_all,
  getter_full_lifecycle, complex_llm_context, high_ambiguity_human,
  dependency_mixed_routing, rejection_with_feedback, review_policy_switch,
  full_lifecycle_mixed

**Files modified:**
- `editor/CMakeLists.txt` — step331_test target with tree-sitter libraries

**Phase 12b complete:** Steps 326-331 (RoutingEngine, WorkerRegistry,
ContextAssembler, Routing RPC+MCP, ReviewGate, Integration Tests)

## Phase 12c: C++ AST Depth — Inheritance + CRTP

### Step 332: Multiple Inheritance in ClassDeclaration
**Status:** PASS (12/12 tests)

Upgraded ClassDeclaration from single superClass string to vector<BaseClass> with
access specifiers (public/protected/private) and virtual inheritance flags. Backward
compatible: legacy superClass field still works via getBases() migration. Added
diamond inheritance detection via static hasDiamondInheritance() with BFS traversal.

**Files modified:**
- `editor/src/ast/ClassDeclaration.h` — BaseClass struct, addBase(), getBases(),
  hasDiamondInheritance() static method
- `editor/src/ast/Serialization.h` — baseClasses array serialization/deserialization
  with legacy superClass backward compat
- `editor/src/CompactAST.h` — getNodeName for ClassDeclaration, InterfaceDeclaration,
  MethodDeclaration
- `editor/CMakeLists.txt` — step332_test target

**Files created:**
- `editor/tests/step332_test.cpp` — 12 tests: backward compat, multiple bases,
  access specifiers, virtual inheritance, JSON roundtrip, diamond detection, legacy
  migration, addBase helper, full serialization roundtrip

### Step 333: Template Class Declarations
**Status:** PASS (12/12 tests)

Extended GenericType with isClassTemplate flag and TypeParameter with isVariadic for
variadic template packs. Added isCRTPClass() helper for CRTP pattern detection. Full
serialization round-trip for both flags.

**Files modified:**
- `editor/src/ast/GenericType.h` — isCRTPClass() helper, isClassTemplate/isVariadic
  already had fields, replaced stub isCRTP()
- `editor/src/ast/Serialization.h` — isClassTemplate for GenericType, isVariadic for
  TypeParameter in propertiesToJson/createNode/setPropertiesFromJson
- `editor/src/CompactAST.h` — getNodeName for GenericType ("template:name"), TypeParameter
- `editor/CMakeLists.txt` — step333_test target

**Files created:**
- `editor/tests/step333_test.cpp` — 12 tests: isClassTemplate flag, isVariadic,
  CRTP detection, JSON roundtrip, CompactAST output

### Step 334: C++ Parser — Inheritance + Templates
**Status:** PASS (12/12 tests)

Extended CppParser to extract multiple base classes from tree-sitter CST, including
access specifiers and virtual keyword. Iterates ALL children (named + unnamed) of
base_class_clause to detect virtual (unnamed node), access_specifier, and type identifiers.

**Files modified:**
- `editor/src/ast/CppParser.h` — Rewrote base_class_clause parsing to iterate all
  children with pending access/virtual state machine
- `editor/CMakeLists.txt` — step334_test target with tree-sitter libraries

**Files created:**
- `editor/tests/step334_test.cpp` — 12 tests: single/multiple inheritance, access
  specifiers, virtual, template classes, CRTP detection, struct default public

### Step 335: C++ Generator — Inheritance + Templates
**Status:** PASS (12/12 tests)

Updated CppGenerator, PythonGenerator, and JavaGenerator to emit correct inheritance
syntax using getBases(). C++ outputs access specifiers + virtual keyword + template
prefix. Python outputs direct multiple inheritance. Java uses first base as extends,
rest as implements.

**Files modified:**
- `editor/src/ast/CppGeneratorTypes.h` — visitClassDeclaration: template prefix from
  GenericType.isClassTemplate, multiple inheritance with access specifiers + virtual
- `editor/src/ast/PythonGenerator.h` — visitClassDeclaration: getBases() for multi-base
- `editor/src/ast/JavaGenerator.h` — visitClassDeclaration: extends + implements adaptation
- `editor/CMakeLists.txt` — step335_test target

**Files created:**
- `editor/tests/step335_test.cpp` — 12 tests: C++ multi-inheritance, access specifiers,
  virtual, template class, CRTP, cross-language (Python/Java/Rust), variadic templates,
  combined template + inheritance, method visibility, full roundtrip

### Step 336: Phase 12c Integration Tests
**Status:** PASS (8/8 tests)

Full integration pipeline for C++ inheritance + templates. Parse real C++ classes with
multiple inheritance, CRTP, and templates via tree-sitter, generate cross-language output,
verify diamond detection, and JSON serialization roundtrip.

**Files modified:**
- `editor/CMakeLists.txt` — step336_test target with tree-sitter libraries

**Files created:**
- `editor/tests/step336_test.cpp` — 8 tests: parse Whetstone-style class, CRTP detection,
  parse→generate roundtrip, C++→Java extends+implements, C++→Python multi-base, template
  3 params, diamond inheritance detection, mixed template+inheritance+virtual pipeline

## Phase 12d: C++ AST Depth — Preprocessor, Enums, Namespaces

### Step 337: Preprocessor AST Nodes
**Status:** PASS (12/12 tests)

Added first-class AST nodes for C++ preprocessor directives: IncludeDirective (path,
isSystem), PragmaDirective (directive), MacroDefinition (name, parameters, body,
isFunctionLike). Statement-level nodes that attach to Module children. Full JSON
serialization roundtrip and CompactAST node name support.

**Files created:**
- `editor/src/ast/PreprocessorNodes.h` — IncludeDirective, PragmaDirective, MacroDefinition
- `editor/tests/step337_test.cpp` — 12 tests: construction, system vs local include,
  JSON roundtrip, pragma, function-like vs object-like macros, CompactAST names, module
  with mixed nodes, empty body, createNode dispatch

**Files modified:**
- `editor/src/ast/Serialization.h` — propertiesToJson/createNode/setPropertiesFromJson
  for all 3 preprocessor types
- `editor/src/CompactAST.h` — getNodeName for IncludeDirective, PragmaDirective,
  MacroDefinition
- `editor/CMakeLists.txt` — step337_test target

### Step 338: EnumDeclaration + NamespaceDeclaration
**Status:** PASS (12/12 tests)

Added EnumDeclaration (scoped/unscoped, underlying type, EnumMember children),
NamespaceDeclaration (with body children, supports nesting), and TypeAlias (using
vs typedef). Full JSON serialization roundtrip and CompactAST names for all 4 types.

**Files created:**
- `editor/src/ast/EnumNamespaceNodes.h` — EnumDeclaration, EnumMember,
  NamespaceDeclaration, TypeAlias
- `editor/tests/step338_test.cpp` — 12 tests: construction, scoped vs unscoped enum,
  members with values, namespace with body, typedef vs using, JSON roundtrip for all
  4 types, CompactAST names, nested namespace, enum inside namespace

**Files modified:**
- `editor/src/ast/Serialization.h` — propertiesToJson/createNode/setPropertiesFromJson
  for all 4 types
- `editor/src/CompactAST.h` — getNodeName for EnumDeclaration, EnumMember,
  NamespaceDeclaration, TypeAlias
- `editor/CMakeLists.txt` — step338_test target

### Step 339: C++ Parser — Preprocessor, Enum, Namespace
**Status:** PASS (12/12 tests)

Extended CppParser to parse preprocessor directives (#include, #pragma, #define),
enums (scoped/unscoped with values), namespaces (with recursive body parsing), and
type aliases (using/typedef) from tree-sitter CST. Handles preproc_include,
preproc_def, preproc_function_def, preproc_call, enum_specifier, namespace_definition,
alias_declaration, and type_definition node types.

**Files modified:**
- `editor/src/ast/Parser.h` — Added PreprocessorNodes.h, EnumNamespaceNodes.h includes
- `editor/src/ast/CppParser.h` — Extended convertCppTranslationUnit dispatch, added
  convertCppInclude, convertCppMacroDef, convertCppPragma, convertCppEnum,
  convertCppNamespace, convertCppTypeAlias converter functions
- `editor/CMakeLists.txt` — step339_test target with tree-sitter libraries

**Files created:**
- `editor/tests/step339_test.cpp` — 12 tests: system/local include, pragma once,
  object-like/function-like macros, scoped/plain enums, namespace with body, using alias,
  typedef, mixed file, backward compat

### Step 340: C++ Generator — Preprocessor, Enum, Namespace
**Status:** PASS (12/12 tests)

All 9 generators (C++, Python, Java, Rust, Go, JavaScript, Elisp, Kotlin, C#)
now produce language-appropriate output for all 6 new AST node types:
IncludeDirective, PragmaDirective, MacroDefinition, EnumDeclaration,
NamespaceDeclaration, TypeAlias. ProjectionGenerator.h dispatch extended
with 6 new branches + default visitor methods.

**Files modified:**
- `editor/src/ast/ProjectionGenerator.h` — 6 new virtual visitors + dispatch branches,
  added PreprocessorNodes.h + EnumNamespaceNodes.h includes
- `editor/src/ast/CppGeneratorTypes.h` — 6 visitor implementations (native C++ output)
- `editor/src/ast/PythonGenerator.h` — import, class(Enum), def macro, type alias
- `editor/src/ast/JavaGenerator.h` — import, enum, package, static final
- `editor/src/ast/RustGenerator.h` — use, enum, mod, macro_rules!, type alias
- `editor/src/ast/GoGenerator.h` — import, type+iota, package, const
- `editor/src/ast/JavaScriptGenerator.h` — import, Object.freeze enum, const, JSDoc typedef
- `editor/src/ast/ElispGenerator.h` — require, defconst enum, defmacro, defalias
- `editor/src/ast/KotlinGenerator.h` — import, enum class, package, typealias
- `editor/src/ast/CSharpGenerator.h` — using, enum, namespace, #pragma, type alias

**Files created:**
- `editor/tests/step340_test.cpp` — 12 tests: C++ include/enum/namespace output,
  Python/Java/Rust/Go enum adaptation, cross-language includes (8 generators),
  using/typedef, Kotlin+C# enum, pragma+define, all 9 generators non-empty for 6 types

### Step 341: Phase 12d Integration + Sprint 12 Summary
**Status:** PASS (8/8 tests)

End-to-end integration tests for full C++ depth: realistic header parsing (includes
+ pragma + namespace + enum + class + function), JSON roundtrip, enum-inside-namespace
scoping, cross-language enum output (6 languages), combined namespace+class+enum pipeline,
batch serialization verification, CompactAST names, self-hosting progress (simplified
Whetstone header fragment parsed successfully).

**Files created:**
- `editor/tests/step341_test.cpp` — 8 integration tests

**Key results:**
- Phase 12d complete: all 5 steps pass (56/56 tests across steps 337–341)
- All 10 generators produce language-appropriate output for 6 new C++ node types
- C++ parser handles: preprocessor, enum class, namespace, type alias
- Self-hosting milestone: simplified Whetstone header fragment parses correctly
- Sprint 12 complete: all 4 phases pass (248/248 tests across steps 320–341)
- 56+ MCP tools, workflow engine, routing engine, review gates, C++ depth

---

# Sprint 13 Progress — GUI Overhaul Phase 1

---

### Step 351: Keybinding Registry
**Status:** PASS (12/12 tests)

Expanded the centralized keybinding registry so actions have stable bindings
that can be displayed with symbols, matched from input state, and persisted
to/from disk. This extends the earlier keybinding foundation with stronger
runtime and persistence primitives for the upcoming command palette/menu work.

**Files modified:**
- `editor/src/KeybindingRegistry.h` — added:
  - `KeyCombo::InputState` and `KeyCombo::matches(...)` for key state matching
  - `KeyCombo::keyToSymbol(...)`, `normalizeKey(...)`, `toSymbolsAuto(...)`
  - `KeybindingRegistry::getAll()` alias for settings/UI consumption
  - `KeybindingRegistry::processInput(...) -> optional<actionId>`
  - `KeybindingRegistry::saveToJson(path)` and `loadFromJsonFile(path)`
- `editor/CMakeLists.txt` — `step351_test` target

**Files created:**
- `editor/tests/step351_test.cpp` — 12 tests covering:
  1. bind + retrieve
  2. default bindings
  3. `KeyCombo::toString`
  4. mac-style symbol rendering
  5. input processing match
  6. input processing no-match
  7. file save/load roundtrip
  8. rebinding
  9. conflict detection
  10. display symbol retrieval
  11. modifier bitmask helpers
  12. exact match behavior + `getAll`

**Verification run:**
- `step343_test` — PASS (12/12) regression coverage
- `step350_test` — PASS (8/8) latest completed sprint checkpoint
- `step351_test` — PASS (12/12) new step coverage

### Step 352: Key Symbol Rendering
**Status:** PASS (12/12 tests)

Implemented a headless key symbol rendering layer for menus/tooltips/command
palette UI data. The renderer now builds styled key badge data, action rows
with shortcut columns, and multi-key chord strings in both text and symbol forms.

**Files modified:**
- `editor/src/KeySymbolRenderer.h` — added/expanded:
  - `renderKeySymbol(...)` (text badge) + backward-compatible `renderKeyBadge(...)`
  - `renderKeyBadgeSymbols(..., macOS)` for symbol mode
  - `renderActionWithKey(..., theme, category)` with layout metadata
  - overload preserving previous call shape
  - `renderKeyChord(...)` + `renderKeyChordSymbols(..., macOS)`
  - badge styling fields (padding/radius) and menu overlap heuristic
- `editor/CMakeLists.txt` — `step352_test` target

**Files created:**
- `editor/tests/step352_test.cpp` — 12 tests covering:
  1. single key badge rendering
  2. modifier+key rendering
  3. platform symbol output
  4. menu item label/shortcut layout data
  5. key chord text rendering
  6. badge background/border assignment
  7. theme color/font propagation
  8. empty binding behavior
  9. special key symbol mapping (Enter/Escape/Tab)
  10. long-label overlap check
  11. chord symbols rendering
  12. `shouldRender` behavior

**Verification run:**
- `step351_test` — PASS (12/12) regression coverage
- `step352_test` — PASS (12/12) new step coverage

### Step 353: Command Palette
**Status:** PASS (12/12 tests)

Extended the command palette into a richer step-353 foundation: stateful open/close,
selection movement, execute-selected behavior, and broadened fuzzy search over labels,
ids, categories, and aliases. Added population helpers that pull actions from
`KeybindingRegistry` and panel toggles from `PanelManager`.

**Files modified:**
- `editor/src/CommandPalette.h` — added:
  - palette state APIs: `open()`, `close()`, `isOpen()`, `setQuery()`, `selectedIndex()`
  - command metadata: `icon`, `aliases`
  - registration helpers: `registerFromKeybindings(...)`, `registerPanelToggles(...)`
  - expanded fuzzy search scoring across label/id/category/aliases
  - selection navigation: `moveSelection(...)`
  - execution helper: `executeSelected(...)` with recent-use tracking
- `editor/CMakeLists.txt` — `step353_test` target

**Files created:**
- `editor/tests/step353_test.cpp` — 12 tests covering:
  1. open/close state
  2. fuzzy search by label
  3. fuzzy search by alias
  4. keybinding registry population
  5. panel toggle population
  6. execute selected action
  7. selection wrap behavior
  8. query reset behavior
  9. recent action ranking
  10. strict context filtering
  11. category-based matching
  12. no-match behavior

**Verification run:**
- `step344_test` — PASS (12/12) command palette integration regression
- `step346_test` — PASS (8/8) layout + palette integration regression
- `step353_test` — PASS (12/12) new step coverage

### Step 354: Menu Bar with Key Symbols
**Status:** PASS (12/12 tests)

Added a structured menu bar model with File/Edit/View/Tools/Workflow/Help menus,
key-symbol metadata on actions, separator grouping, action selection handling,
and submenu support for help/documentation. Menu action labels/shortcuts are
derived from `KeybindingRegistry` and rendered via `KeySymbolRenderer` so the
symbol display path is consistent with step 352.

**Files created:**
- `editor/src/MenuBar.h` — headless menu model:
  - `MenuItem` / `Menu` / `MenuBar`
  - `buildDefaults(...)` for the full menu set
  - selection API (`openMenu`, `selectItem`) with close-on-select behavior
  - submenu metadata support and separator counting helpers
- `editor/tests/step354_test.cpp` — 12 tests covering:
  1. core menus present
  2. File Save shortcut
  3. Edit Undo shortcut
  4. View panel toggle actions
  5. Tools pipeline action
  6. correct action execution id
  7. key symbol visibility
  8. menu close after selection
  9. separator grouping
  10. workflow action presence
  11. submenu metadata
  12. invalid selection handling

**Files modified:**
- `editor/CMakeLists.txt` — `step354_test` target

**Verification run:**
- `step352_test` — PASS (12/12) regression coverage
- `step353_test` — PASS (12/12) regression coverage
- `step354_test` — PASS (12/12) new step coverage

### Step 355: Phase 13c Integration — Keyboard Shortcuts Help Panel
**Status:** PASS (8/8 tests)

Integrated a dedicated keyboard-shortcuts panel model and connected the end-to-end
keyboard flow across the keybinding registry, command palette, and menu bar.
This provides grouped/filterable shortcut rows plus refresh-on-rebind behavior and
a full keyboard-driven command execution path.

**Files created:**
- `editor/src/KeyboardShortcutsPanel.h` — panel data model:
  - visibility state, filter text, grouped rows by category
  - row projection from `KeybindingRegistry` (label/category/symbols)
  - customize-request signal (`requestCustomize` / `consumeCustomizeRequest`)
- `editor/tests/step355_test.cpp` — 8 integration tests:
  1. defaults appear in shortcuts panel
  2. Ctrl+S resolves to save action
  3. Ctrl+P opens command palette
  4. command palette entries include key symbols
  5. menu entries include key symbols
  6. rebind propagates to shortcuts panel
  7. symbol rendering works for current platform mode
  8. keyboard workflow: Ctrl+P → query pipeline → execute run-pipeline

**Files modified:**
- `editor/src/KeybindingRegistry.h` — added `keyboard-shortcuts` action and default
  binding (`Ctrl+Shift+?`) to support help-panel keyboard access
- `editor/src/CommandPalette.h` — added `symbols` field on command entries and
  populated symbol strings from keybindings
- `editor/CMakeLists.txt` — `step355_test` target

**Verification run:**
- `step351_test` — PASS (12/12) regression coverage
- `step353_test` — PASS (12/12) regression coverage
- `step354_test` — PASS (12/12) regression coverage
- `step355_test` — PASS (8/8) new integration coverage

### Step 356: Breadcrumb Navigation
**Status:** PASS (12/12 tests)

Implemented a breadcrumb bar model that tracks AST path from module root to
cursor node, supports click-based navigation targets, and exposes sibling lists
for dropdown navigation. The model uses compact node names (`getNodeName`) and
theme-derived colors for consistent visual integration.

**Files created:**
- `editor/src/panels/BreadcrumbBar.h` — headless breadcrumb model:
  - `buildBreadcrumbBar(...)` path generation from module/cursor
  - clickable navigation helper (`navigateBreadcrumbClick`)
  - sibling dropdown helpers (`breadcrumbSiblings`, `breadcrumbSiblingNames`)
  - theme color wiring + overflow signal for deep paths
- `editor/tests/step356_test.cpp` — 12 tests covering:
  1. module name segment
  2. cursor-driven nested updates
  3. click navigation
  4. sibling dropdown population
  5. empty-file fallback
  6. deep path overflow handling
  7. theme color usage
  8. annotation nodes in path
  9. compact naming for class/method
  10. out-of-range click safety
  11. root sibling behavior
  12. foreign-node fallback to module-only

**Files modified:**
- `editor/CMakeLists.txt` — `step356_test` target

**Verification run:**
- `step354_test` — PASS (12/12) regression coverage
- `step355_test` — PASS (8/8) regression coverage
- `step356_test` — PASS (12/12) new step coverage

### Step 357: Status Bar
**Status:** PASS (12/12 tests)

Added a status-bar data model that provides left/center/right sections for
language/file, cursor+encoding+line-ending, and diagnostics/workflow/MCP state.
The model includes theme-driven colors, overflow signaling, and a diagnostics
focus target for click handling.

**Files created:**
- `editor/src/panels/StatusBar.h` — headless status model:
  - `StatusBarInput` + `StatusBarModel`
  - `buildStatusBar(...)` section construction and overflow check
  - language badge color mapping
  - diagnostics click target helper
- `editor/tests/step357_test.cpp` — 12 tests covering:
  1. language badge
  2. cursor position updates
  3. diagnostic summary counts
  4. workflow badge visibility/content
  5. diagnostics click target
  6. theme color usage
  7. language switch behavior
  8. encoding display
  9. normal-fit no-overflow case
  10. overflow flag on extreme content
  11. line-ending display
  12. MCP connected/offline indicator

**Files modified:**
- `editor/CMakeLists.txt` — `step357_test` target

**Verification run:**
- `step355_test` — PASS (8/8) regression coverage
- `step356_test` — PASS (12/12) regression coverage
- `step357_test` — PASS (12/12) new step coverage

### Step 358: Go-to-Definition + Go-to-Symbol
**Status:** PASS (12/12 tests)

Implemented headless navigation tooling for symbol discovery and jump resolution:
file-level and project-level symbol lists, symbol filtering, icon/type metadata,
and go-to-definition lookup with current-file preference and cross-file fallback.

**Files created:**
- `editor/src/panels/NavigationTools.h` — navigation model:
  - symbol collection (`collectFileSymbols`, `collectProjectSymbols`)
  - symbol filtering (`filterSymbols`)
  - F12-style definition lookup (`goToDefinition`)
  - no-match messaging (`definitionStatusMessage`)
  - line-order sorting and icon assignment
- `editor/tests/step358_test.cpp` — 12 tests covering:
  1. definition jump from function call
  2. file symbol listing
  3. project symbol listing
  4. icon presence by symbol type
  5. symbol filtering
  6. cross-file definition jump
  7. no-match message
  8. line-number sort order
  9. empty-query behavior
  10. case-insensitive filtering
  11. current-file definition preference
  12. null-call safety

**Files modified:**
- `editor/CMakeLists.txt` — `step358_test` target

**Verification run:**
- `step356_test` — PASS (12/12) regression coverage
- `step357_test` — PASS (12/12) regression coverage
- `step358_test` — PASS (12/12) new step coverage

### Step 359: Find and Replace Improvements
**Status:** PASS (12/12 tests)

Implemented a find/replace bar model with in-file match tracking, current-match
navigation, replace-current/replace-all flows, and project-wide search results.
Model behavior includes open/close modes, highlight state, and match counter text.

**Files created:**
- `editor/src/panels/FindReplaceBar.h` — find/replace model:
  - find/replace visibility + mode toggles
  - query + match indexing + counter text
  - next/previous match navigation
  - replace-current and replace-all operations
  - project-wide search result generation
- `editor/tests/step359_test.cpp` — 12 tests covering:
  1. open find
  2. query-to-match filtering
  3. highlight visibility
  4. match counter formatting
  5. next-match cycling
  6. replace mode open
  7. replace-current operation
  8. project search results
  9. close behavior
  10. empty-query clears highlights
  11. previous-match cycling
  12. replace-all behavior

**Files modified:**
- `editor/CMakeLists.txt` — `step359_test` target

**Verification run:**
- `step357_test` — PASS (12/12) regression coverage
- `step358_test` — PASS (12/12) regression coverage
- `step359_test` — PASS (12/12) new step coverage

### Step 360: Phase 13d Integration + Sprint 13 Summary
**Status:** PASS (8/8 tests)

Completed Phase 13d integration with end-to-end checks across keyboard navigation,
go-to-definition, breadcrumb navigation, command palette search flows, status bar
state transitions, key symbol visibility, and panel layout persistence.

**Files created:**
- `editor/tests/step360_test.cpp` — 8 integration tests:
  1. keyboard workflow + definition jump + breadcrumb back-navigation
  2. command palette find flow + project result navigation
  3. status bar response to buffer/language/cursor/diagnostic changes
  4. docked-panel invariant (no floating model state)
  5. theme consistency across navigation components
  6. key symbols in menu + palette + shortcuts panel
  7. layout snapshot/restore roundtrip
  8. sprint-level operational gates (docking/theme/keys/navigation)

**Files modified:**
- `editor/CMakeLists.txt` — `step360_test` target

**Verification run:**
- `step358_test` — PASS (12/12) regression coverage
- `step359_test` — PASS (12/12) regression coverage
- `step360_test` — PASS (8/8) phase integration coverage

**Key results:**
- Phase 13d complete: all 5 steps pass (56/56 tests across steps 356–360)
- Sprint 13 complete: all phases pass (212/212 tests across steps 342–360)
- Navigation foundation in place: breadcrumbs, status model, symbol navigation,
  find/replace model, and full keyboard-centric integration path
- Key symbol consistency validated across all primary interaction surfaces
- Post-sprint architecture gate passed: refactored `MenuBar::buildDefaults` into
  helper functions to satisfy the max function length constraint (<=80 lines),
  with `step354_test`, `step355_test`, and `step360_test` revalidated

---

# Sprint 14 Progress — Language Batch 1

### Step 361: C Parser — Functions, Structs, Enums
**Status:** PASS (12/12 tests)

Added a standalone C parser (regex/text-based, no new tree-sitter dependency)
that handles core C declarations and preprocessor constructs needed for the
pipeline entry point in Sprint 14.

**Files created:**
- `editor/src/ast/CParser.h` — C parser support:
  - `parseC(...)` and `parseCWithDiagnostics(...)`
  - preprocessor parsing: `#include`, `#define` (object/function-like), `#pragma`
  - header-guard shape recognition (`#ifndef/#ifdef/#endif`) into pragma-style nodes
  - C declarations: functions, structs, typedef-struct, enums (named + anonymous),
    top-level variables, function-pointer parameters
  - comment stripping for C line/block comments before declaration parsing
- `editor/tests/step361_test.cpp` — 12 tests covering:
  1. function parsing
  2. struct parsing
  3. typedef struct parsing
  4. enum parsing (named + anonymous)
  5. preprocessor directives
  6. static/extern qualifiers
  7. function-pointer parameter parsing
  8. pointer variable parsing
  9. multiple functions per file
  10. C++ parser backward compatibility
  11. C comment handling
  12. pipeline routing for `"c"` language + header-guard pattern

**Files modified:**
- `editor/src/ast/Parser.h` — include `ast/CParser.h`
- `editor/src/Pipeline.h` — add parse routing for language `"c"`
- `editor/CMakeLists.txt` — `step361_test` target

**Verification run:**
- `step361_test` — PASS (12/12) new step coverage
- `step360_test` — PASS (8/8) regression coverage

### Step 362: C Generator
**Status:** PASS (12/12 tests)

Added a dedicated C generator and routed pipeline generation for `"c"`.
The generator emits C-oriented forms for structs, enums, typedefs, macros,
includes, and method-name lowering for class-style methods.

**Files created:**
- `editor/src/ast/CGenerator.h` — C generation support:
  - module emission including statements/classes/functions
  - `ClassDeclaration` -> `typedef struct ...`
  - `MethodDeclaration` -> `ClassName_method(...)` naming
  - C null literal output (`NULL`)
  - C preprocessor and enum/type-alias output
- `editor/tests/step362_test.cpp` — 12 tests covering:
  1. function generation
  2. struct output
  3. typedef output
  4. enum output
  5. preprocessor output
  6. pointer type output
  7. owner/manual annotation comment presence
  8. class-to-struct generation
  9. method-to-C function naming
  10. comment prefix
  11. string/null literal behavior
  12. pipeline `"c"` generation route

**Files modified:**
- `editor/src/ast/Generator.h` — include `CGenerator.h`
- `editor/src/Pipeline.h` — add generate routing for language `"c"`
- `editor/CMakeLists.txt` — `step362_test` target

**Verification run:**
- `step362_test` — PASS (12/12) new step coverage
- `step361_test` — PASS (12/12) regression coverage
- `step360_test` — PASS (8/8) regression coverage

### Step 363: C Memory Annotation Mapping
**Status:** PASS (12/12 tests)

Implemented C-specific memory and semantic annotation inference so C code now
gets meaningful defaults and risk signals aligned with explicit-memory patterns.

**Files created:**
- `editor/src/ProjectionAdaptation.h` — extracted projection adaptation helpers:
  - C owner strategy adaptation for Rust targets (`Manual -> Box`,
    `Transferred -> Moved`)
  - pointer-type adaptation for Rust references (`T* -> &T`)

**Files modified:**
- `editor/src/MemoryStrategyInference.h` — C-specific memory inference:
  - module defaults: `Owner(Manual)`, `Lifetime(Scope)`, `Reclaim(Explicit)`
  - malloc/calloc detection: function-level owner/reclaim suggestions
  - pointer parameter inference: `Owner(Borrowed)`
  - pointer return inference by naming convention:
    - `get*/borrow*/peek*/view*` -> `Owner(Borrowed)`
    - otherwise -> `Owner(Transferred)`
  - local/static variable lifetime inference:
    - locals -> `Lifetime(Scope)`
    - static storage -> `Lifetime(Static)`
- `editor/src/AnnotationInference.h` — C pattern inference:
  - `goto` pattern -> `Complexity(high)` + `Risk(medium)`
  - unchecked array indexing -> `BoundsCheck(unchecked)`
  - `void*` usage -> `Risk(high)` + `Ambiguity(medium)`
  - header guard detection -> `Synthetic(guard)`
- `editor/src/CrossLanguageProjector.h` — uses `ProjectionAdaptation` helpers
  for ownership and pointer-type adaptation without exceeding header size limits
- `editor/tests/step363_test.cpp` — 12 tests covering defaults, C-specific rules,
  confidence bounds, header-guard synthesis, and C->Rust ownership adaptation
- `editor/CMakeLists.txt` — `step363_test` target

**Verification run:**
- `step363_test` — PASS (12/12) new step coverage
- `step362_test` — PASS (12/12) regression coverage
- `step361_test` — PASS (12/12) regression coverage
- `step360_test` — PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/CrossLanguageProjector.h` remained under the 600-line cap after
  refactor by extracting target adaptation logic into
  `editor/src/ProjectionAdaptation.h`

### Step 364: C Integration + Self-Hosting Prep
**Status:** PASS (8/8 tests)

Added phase-level integration coverage for the C pipeline and projection paths,
including pipeline execution, C parser/generator roundtrip checks, ownership
projection behavior, and MCP tool contract stability.

**Files created:**
- `editor/tests/step364_test.cpp` — 8 integration tests covering:
  1. C parse -> generate -> reparse declaration continuity
  2. C struct -> Python class-form generation
  3. Python class shape -> C struct + constructor-style method generation
  4. C manual ownership -> Rust `Owner(Box)` adaptation
  5. `Pipeline.run(..., \"c\", \"cpp\")` success path with generated output
  6. Combined C inference: malloc + pointer parameter + unchecked index access
  7. simplified dependency-style C header parsing
  8. MCP tools/list stability + run_pipeline language schema acceptance

**Files modified:**
- `editor/src/CrossLanguageProjector.h` — integration support:
  - copy `classes` and `statements` roles during projection
  - generic serialization fallback clone for unhandled declaration nodes
- `editor/CMakeLists.txt` — `step364_test` target with parser-linked test deps

**Verification run:**
- `step364_test` — PASS (8/8) new step coverage
- `step363_test` — PASS (12/12) regression coverage
- `step362_test` — PASS (12/12) regression coverage
- `step361_test` — PASS (12/12) regression coverage
- `step360_test` — PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/CrossLanguageProjector.h` remains within header size limit
  (`596` lines <= `600`)

### Step 365: WAT Parser
**Status:** PASS (12/12 tests)

Added a standalone WebAssembly text-format parser (`wat`/`wasm`) and wired it
into the shared parsing pipeline.

**Files created:**
- `editor/src/ast/WatParser.h` — WAT parser support:
  - `parseWat(...)` and `parseWatWithDiagnostics(...)`
  - balanced-expression scanning inside `(module ...)`
  - function parsing for params/results/locals
  - import/export/global/memory/table top-level mapping
  - instruction-shape mapping for `i32.add`, `call`, `if`, `block`, `loop`
  - line-comment stripping for `;; ...`
- `editor/tests/step365_test.cpp` — 12 tests covering:
  1. module parsing
  2. function params/results
  3. locals
  4. exports
  5. imports
  6. globals
  7. `i32.add` mapping
  8. `call` mapping
  9. `if` mapping
  10. block/loop mapping
  11. empty module
  12. pipeline `wat` and `wasm` routing + type preservation

**Files modified:**
- `editor/src/ast/Parser.h` — include `ast/WatParser.h`
- `editor/src/Pipeline.h` — add parse routing for `\"wat\"` and `\"wasm\"`
- `editor/CMakeLists.txt` — `step365_test` target

**Verification run:**
- `step365_test` — PASS (12/12) new step coverage
- `step364_test` — PASS (8/8) regression coverage
- `step363_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ast/WatParser.h` is within header size limit (`231` lines)

### Step 366: WAT Generator
**Status:** PASS (12/12 tests)

Added a dedicated WAT generator and integrated `wat/wasm` generation routing in
the shared pipeline.

**Files created:**
- `editor/src/ast/WatGenerator.h` — WAT generation support:
  - module emission in s-expression format
  - function emission with `(param ...)` / `(result ...)`
  - primitive type mapping to WAT types (`i32`, `i64`, `f32`, `f64`)
  - instruction emission for arithmetic, calls, if/else, and block constructs
  - import emission and pragma-driven memory/table/export output
  - annotation comment output using WAT prefix (`;; `)
- `editor/tests/step366_test.cpp` — 12 tests covering:
  1. function generation
  2. module s-expression formatting
  3. type mapping
  4. arithmetic lowering to stack ops
  5. if/else output
  6. function call output
  7. export output
  8. import output
  9. annotation comment prefix
  10. Python -> WAT pipeline route
  11. parse->generate->parse roundtrip
  12. balanced parentheses invariant

**Files modified:**
- `editor/src/ast/Generator.h` — include `WatGenerator.h`
- `editor/src/Pipeline.h` — add generate routing for `\"wat\"` and `\"wasm\"`
- `editor/CMakeLists.txt` — `step366_test` target

**Verification run:**
- `step366_test` — PASS (12/12) new step coverage
- `step365_test` — PASS (12/12) regression coverage
- `step364_test` — PASS (8/8) regression coverage
- `step363_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ast/WatGenerator.h` is within header size limit (`219` lines)

### Step 367: WAT Annotation Mapping
**Status:** PASS (12/12 tests)

Implemented WAT-specific annotation inference for execution model, linkage, and
memory-layout signals, plus WAT memory-strategy defaults.

**Files created:**
- `editor/tests/step367_test.cpp` — 12 tests covering:
  1. function -> `Exec(stack)`
  2. memory operations -> `Owner(Manual)`
  3. export -> `Visibility(public)`
  4. import -> `Link(import)`
  5. WAT defaults (`Owner(Manual)`, `Reclaim(Explicit)`)
  6. table dispatch -> `Shim(vtable)`
  7. memory declaration -> `Align` + `Layout(linear)`
  8. confidence bounds
  9. annotation preservation through projection to C
  10. Semanno emit/parse roundtrip
  11. pipeline parse + inference integration
  12. WAT -> C -> WAT annotation roundtrip preservation

**Files modified:**
- `editor/src/MemoryStrategyInference.h` — WAT defaults and memory-op rule:
  - module defaults for `wat/wasm`
  - function memory-op detection (`memory.*`, `*.load`, `*.store`)
- `editor/src/AnnotationInference.h` — WAT-specific pattern inference:
  - function-level `Exec(stack)`
  - import/export mapping to `Link`/`Visibility`
  - table mapping to `Shim(vtable)`
  - memory mapping to `Align` + `Layout(linear)`
- `editor/CMakeLists.txt` — `step367_test` target

**Verification run:**
- `step367_test` — PASS (12/12) new step coverage
- `step366_test` — PASS (12/12) regression coverage
- `step365_test` — PASS (12/12) regression coverage
- `step364_test` — PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/AnnotationInference.h` remains within header size limit
  (`589` lines <= `600`)

### Step 368: WAT Cross-Language Projection
**Status:** PASS (12/12 tests)

Added cross-language WAT projection coverage across pipeline and direct
projection paths (Python/C/Rust/Java), including annotation and metadata
preservation checks.

**Files created:**
- `editor/tests/step368_test.cpp` — 12 tests covering:
  1. Python -> WAT
  2. C -> WAT
  3. WAT -> Python
  4. WAT -> C
  5. WAT -> Rust
  6. Java -> WAT
  7. Python -> WAT -> Python identity check
  8. WAT <-> C annotation preservation
  9. import metadata preservation through projection
  10. multi-function WAT -> C output generation
  11. Semanno roundtrip on projected annotation data
  12. WAT-source pipeline run success

**Files modified:**
- `editor/CMakeLists.txt` — `step368_test` target

**Verification run:**
- `step368_test` — PASS (12/12) new step coverage
- `step367_test` — PASS (12/12) regression coverage
- `step366_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/tests/step368_test.cpp` remains within project file-size guidance
  for test artifacts (`162` lines)

### Step 369: Phase 14b Integration
**Status:** PASS (8/8 tests)

Added phase-level integration validation for WAT as both source and target,
including compact AST coverage, malformed-source diagnostics, sidecar
persistence, and MCP contract checks.

**Files created:**
- `editor/tests/step369_test.cpp` — 8 integration tests covering:
  1. parse WAT -> generate WAT validity
  2. C -> WAT -> C roundtrip path
  3. broad multi-language -> WAT pipeline targeting coverage
  4. WAT annotation completeness checks
  5. compact AST node coverage on WAT module
  6. malformed WAT diagnostic detection
  7. Semanno sidecar save/load on `.wat` path
  8. MCP `whetstone_run_pipeline` language-parameter compatibility

**Files modified:**
- `editor/src/ast/WatParser.h` — `parseWatWithDiagnostics(...)` now reports:
  - unmatched/unbalanced parenthesis errors
  - missing module-form warning
- `editor/CMakeLists.txt` — `step369_test` target

**Verification run:**
- `step369_test` — PASS (8/8) new step coverage
- `step368_test` — PASS (12/12) regression coverage
- `step367_test` — PASS (12/12) regression coverage
- `step366_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ast/WatParser.h` remains within header-size limit after diagnostic
  additions (`248` lines <= `600`)

### Step 370: Common Lisp Parser
**Status:** PASS (12/12 tests)

Added a dedicated Common Lisp parser with S-expression parsing and direct
mapping for core CL forms into the Whetstone AST, including CLOS and macro
surface forms.

**Files created:**
- `editor/src/ast/CommonLispParser.h` — Common Lisp parse support:
  - recursive S-expression reader with comment/string handling
  - top-level form conversion for `defun`, `defmethod`, `defclass`,
    `defvar`/`defparameter`, and `defmacro`
  - expression conversion for `if`, `cond`, `let`, `lambda`, loop forms,
    `funcall`/`apply`, `quote`, and `values`
  - convention handling for package-qualified symbols and `*earmuff*`
    dynamic-variable detection
  - `(declare (type ...))` mapping to parameter type nodes
- `editor/tests/step370_test.cpp` — 12 tests covering:
  1. `defun` parsing
  2. `defclass` parsing
  3. `lambda` parsing
  4. `let` binding/scope mapping
  5. `if`/`cond` mapping
  6. `loop`/`do`/`dotimes` mapping
  7. `defmacro` mapping
  8. quote shorthand and `@Meta(quoted)` annotation mapping
  9. dynamic `*earmuff*` variable annotation
  10. deep nested S-expression parsing
  11. mixed top-level definitions in one source
  12. CLOS method parsing + pipeline language alias routing

**Files modified:**
- `editor/src/ast/Parser.h` — include `ast/CommonLispParser.h`
- `editor/src/Pipeline.h` — add parse routing for `\"common-lisp\"`,
  `\"commonlisp\"`, `\"lisp\"`, and `\"cl\"`
- `editor/CMakeLists.txt` — `step370_test` target

**Verification run:**
- `step370_test` — PASS (12/12) new step coverage
- `step369_test` — PASS (8/8) regression coverage
- `step365_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ast/CommonLispParser.h` is within header size limit
  (`598` lines <= `600`)

### Step 371: Common Lisp Generator
**Status:** PASS (12/12 tests)

Added a dedicated Common Lisp generator and integrated target-language routing
for Common Lisp aliases in the pipeline.

**Files created:**
- `editor/src/ast/CommonLispGenerator.h` — Common Lisp generation support:
  - module/function emission as idiomatic S-expressions
  - `defclass` + `defmethod` output for CLOS class/method projection
  - typed parameter declarations via `(declare (type ...))`
  - `let` emission from block-scoped variable statements
  - macro emission via `defmacro`
  - function-level exception annotation mapping to `handler-case` wrappers
  - comment prefix alignment (`;; `) with explicit owner annotation formatting
- `editor/tests/step371_test.cpp` — 12 tests covering:
  1. `defun` generation
  2. `defclass` output
  3. `lambda` output
  4. `let` binding emission
  5. S-expression shape / balanced output
  6. typed `declare` emission
  7. Python -> Common Lisp pipeline route
  8. Java -> Common Lisp pipeline route
  9. Semanno comment prefix (`;;`)
  10. parse -> generate -> parse roundtrip
  11. CLOS method dispatch output
  12. condition-system wrapper from exception annotation

**Files modified:**
- `editor/src/ast/Generator.h` — include `CommonLispGenerator.h`
- `editor/src/Pipeline.h` — add generate routing for `\"common-lisp\"`,
  `\"commonlisp\"`, `\"lisp\"`, and `\"cl\"`
- `editor/CMakeLists.txt` — `step371_test` target

**Verification run:**
- `step371_test` — PASS (12/12) new step coverage
- `step370_test` — PASS (12/12) regression coverage
- `step369_test` — PASS (8/8) regression coverage
- `step366_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ast/CommonLispGenerator.h` remains within header-size limit
  (`267` lines <= `600`)

### Step 372: Lisp Annotation Mapping
**Status:** PASS (12/12 tests)

Added Lisp-focused annotation inference coverage for macros, dynamic bindings,
multi-value contracts, method dispatch signals, and optimize/declaration forms,
plus Common Lisp memory-strategy defaults.

**Files created:**
- `editor/src/AnnotationInferenceLisp.h` — extracted Lisp-specific inference helpers:
  - macro inference (`Meta(quoted)` + `Synthetic(macro)`)
  - dynamic vs lexical binding inference (`Binding(dynamic|static)`)
  - CL form pattern checks for `values`, `declare(type ...)`, and `optimize`
  - CLOS method synthetic hint inference
- `editor/tests/step372_test.cpp` — 12 tests covering:
  1. macro -> `Meta(quoted)`
  2. dynamic variable -> `Binding(dynamic)`
  3. `values` -> multi-value `Contract`
  4. tail-recursive function -> `TailCall`
  5. CLOS class -> `Visibility(public)`
  6. optimize declaration -> `Policy(perf=critical, style=literal)`
  7. Common Lisp GC defaults (`Reclaim(Tracing)`, `Owner(Shared_GC)`)
  8. confidence bounds and strong signals
  9. `Binding(dynamic)` preservation through CL -> Python projection
  10. `Binding(dynamic)` preservation through CL -> C++ projection
  11. `declare(type ...)` -> `Complexity(declared-type)`
  12. method synthetic hint for CLOS dispatch

**Files modified:**
- `editor/src/AnnotationInference.h` — integrate Lisp inference hooks while
  preserving architecture line limits
- `editor/src/MemoryStrategyInference.h` — add Common Lisp module defaults for GC
- `editor/CMakeLists.txt` — `step372_test` target

**Verification run:**
- `step372_test` — PASS (12/12) new step coverage
- `step371_test` — PASS (12/12) regression coverage
- `step370_test` — PASS (12/12) regression coverage
- `step369_test` — PASS (8/8) regression coverage
- `step367_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/AnnotationInference.h` remains within header-size limit
  (`599` lines <= `600`)
- `editor/src/MemoryStrategyInference.h` remains within header-size limit
  (`384` lines <= `600`)

### Step 373: Common Lisp Integration
**Status:** PASS (8/8 tests)

Added phase-level integration validation for Common Lisp parse/generate
roundtrips, cross-language projection paths, inference integration, and CL
artifact compatibility with Semanno and compact AST tooling.

**Files created:**
- `editor/tests/step373_test.cpp` — 8 integration tests covering:
  1. Common Lisp parse -> generate -> parse roundtrip shape
  2. Python class -> Common Lisp CLOS generation path
  3. Common Lisp defclass retained through Python projection
  4. Common Lisp class hierarchy retained through Java projection
  5. Common Lisp annotation inference integration (macro/dynamic/tail-call)
  6. Semanno sidecar save/load roundtrip on `.lisp` path
  7. `Pipeline.run()` success with Common Lisp source
  8. compact AST node naming on Common Lisp module

**Files modified:**
- `editor/CMakeLists.txt` — `step373_test` target

**Verification run:**
- `step373_test` — PASS (8/8) new step coverage
- `step372_test` — PASS (12/12) regression coverage
- `step371_test` — PASS (12/12) regression coverage
- `step370_test` — PASS (12/12) regression coverage
- `step369_test` — PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/tests/step373_test.cpp` remains within project file-size guidance
  for test artifacts (`147` lines)

### Step 374: Scheme Parser
**Status:** PASS (12/12 tests)

Added a dedicated Scheme parser with S-expression parsing and Scheme-specific
form mapping into the Whetstone AST, including continuation and hygienic macro
signals.

**Files created:**
- `editor/src/ast/SchemeParser.h` — Scheme parse support:
  - recursive S-expression reader with Scheme comment and quote handling
  - top-level mapping for `define` (function/value) and `define-syntax`
  - expression mapping for `lambda`, `let`/`let*`/`letrec`, `if`, `cond`,
    `do`, `begin`, `call-with-current-continuation`/`call/cc`, `quote`,
    and `values`
  - continuation call mapping with `Exec(mode=continuation)` annotation
  - `define-syntax` mapping to `MacroDefinition` with hygienic `Meta` signal
- `editor/tests/step374_test.cpp` — 12 tests covering:
  1. `define` function parsing
  2. `define` variable parsing
  3. `lambda` parsing
  4. `let` variants mapping
  5. `if`/`cond` mapping
  6. `do` loop mapping
  7. `call/cc` continuation annotation mapping
  8. `define-syntax` hygienic macro mapping
  9. `values` contract mapping
  10. `begin` sequence mapping
  11. deep nesting parsing
  12. mixed forms + pipeline scheme/scm routing + malformed diagnostics

**Files modified:**
- `editor/src/ast/Parser.h` — include `ast/SchemeParser.h`
- `editor/src/Pipeline.h` — add parse routing for `\"scheme\"` and `\"scm\"`
- `editor/CMakeLists.txt` — `step374_test` target

**Verification run:**
- `step374_test` — PASS (12/12) new step coverage
- `step373_test` — PASS (8/8) regression coverage
- `step372_test` — PASS (12/12) regression coverage
- `step371_test` — PASS (12/12) regression coverage
- `step370_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ast/SchemeParser.h` is within header size limit
  (`461` lines <= `600`)

### Step 375: Scheme Generator
**Status:** PASS (12/12 tests)

Added a dedicated Scheme generator and pipeline generation routing for Scheme
targets, with Scheme-specific output conventions and continuation/tail-call
annotation handling.

**Files created:**
- `editor/src/ast/SchemeGenerator.h` — Scheme generation support:
  - module/function/variable emission using `define` forms
  - Scheme-flavored block lowering (`let` / `begin`)
  - boolean/null mapping to `#t`, `#f`, and `'()`
  - continuation-aware call lowering to `call/cc`
  - record-style class projection output (`define-record-type`)
  - Scheme semanno comment output for owner/tail-call/exec annotations
- `editor/tests/step375_test.cpp` — 12 tests covering:
  1. define-function generation
  2. lambda output
  3. let binding output
  4. boolean mapping
  5. S-expression balance/formatting
  6. Python -> Scheme pipeline route
  7. Common Lisp -> Scheme pipeline route
  8. Scheme -> Python pipeline route
  9. semanno comment prefix behavior
  10. parse -> generate -> parse roundtrip
  11. tail-call annotation preservation in output
  12. continuation annotation mapping to `call/cc`

**Files modified:**
- `editor/src/ast/Generator.h` — include `SchemeGenerator.h`
- `editor/src/Pipeline.h` — add generate routing for `\"scheme\"` and `\"scm\"`
- `editor/CMakeLists.txt` — `step375_test` target

**Verification run:**
- `step375_test` — PASS (12/12) new step coverage
- `step374_test` — PASS (12/12) regression coverage
- `step373_test` — PASS (8/8) regression coverage
- `step372_test` — PASS (12/12) regression coverage
- `step371_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ast/SchemeGenerator.h` is within header size limit
  (`159` lines <= `600`)

### Step 376: Scheme-Specific Annotations + CL Differentiation
**Status:** PASS (12/12 tests)

Added Scheme-specific inference signals and explicit Common Lisp vs Scheme
differentiation rules in annotation inference, plus Scheme memory defaults in
memory-strategy inference.

**Files created:**
- `editor/tests/step376_test.cpp` — 12 tests covering:
  1. Scheme tail recursion -> `TailCall`
  2. `call/cc` -> `Exec(continuation)`
  3. `define-syntax` -> `Meta(hygienic)`
  4. CL -> Scheme projection path
  5. Scheme -> CL projection path
  6. GC defaults for both Scheme and CL
  7. differentiated macro meta states (quoted vs hygienic)
  8. differentiated binding states (dynamic vs parameter)
  9. differentiated exception styles (condition vs guard)
  10. Scheme/CL annotation fidelity through projection
  11. tail-recursive loop idiom signal (`Loop(tail-recursive)`)
  12. confidence bound checks

**Files modified:**
- `editor/src/AnnotationInferenceLisp.h` — add Lisp/Scheme-specific inference:
  - continuation detection (`call/cc`, `call-with-current-continuation`)
  - parameter-binding detection (`make-parameter`)
  - condition-vs-guard exception style differentiation
  - tail-recursive loop signal (`LoopAnnotation: tail-recursive`)
  - macro meta-state differentiation via existing hygienic/quoted metadata
- `editor/src/MemoryStrategyInference.h` — add Scheme defaults:
  - include `scheme/scm` in tracing-GC defaults
  - shared GC ownership default for Lisp-family modules
- `editor/CMakeLists.txt` — `step376_test` target

**Verification run:**
- `step376_test` — PASS (12/12) new step coverage
- `step375_test` — PASS (12/12) regression coverage
- `step374_test` — PASS (12/12) regression coverage
- `step373_test` — PASS (8/8) regression coverage
- `step372_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/AnnotationInference.h` remains within header-size limit
  (`599` lines <= `600`)
- `editor/src/MemoryStrategyInference.h` remains within header-size limit
  (`384` lines <= `600`)
- `editor/src/AnnotationInferenceLisp.h` remains within header-size guidance
  (`129` lines)

### Step 377: Phase 14d Integration + Sprint 14 Summary
**Status:** PASS (8/8 tests)

Completed phase-level integration validation for Scheme and finalized Sprint 14
with cross-language interoperability checks across C, WAT, Common Lisp, and
Scheme.

**Files created:**
- `editor/tests/step377_test.cpp` — 8 integration tests covering:
  1. all 14 language parse/generate route smoke checks
  2. C <-> WAT <-> Common Lisp <-> Scheme projection matrix
  3. Lisp-source -> WAT generation path
  4. C -> Common Lisp memory-default translation validation
  5. Semanno comment-prefix correctness across the 4 new languages
  6. language-appropriate annotation defaults across the 4 new languages
  7. `Pipeline.run()` success with each of the 4 new languages as source
  8. Sprint 14 route/matrix totals sanity checks

**Files modified:**
- `editor/CMakeLists.txt` — `step377_test` target

**Verification run:**
- `step377_test` — PASS (8/8) new step coverage
- `step376_test` — PASS (12/12) regression coverage
- `step375_test` — PASS (12/12) regression coverage
- `step374_test` — PASS (12/12) regression coverage
- `step373_test` — PASS (8/8) regression coverage

**Sprint 14 completion snapshot (Steps 361-377):**
- Phase 14a (C): complete
- Phase 14b (WAT): complete
- Phase 14c (Common Lisp): complete
- Phase 14d (Scheme): complete
- Sprint 14 integration: complete

**Architecture gate check:**
- `editor/tests/step377_test.cpp` remains within project file-size guidance
  for test artifacts (`188` lines)

### Step 378: Orchestrator Core Main Loop
**Status:** PASS (12/12 tests)

Implemented Sprint 15 orchestration-loop core as a dedicated workflow engine
that advances ready work items through routing, context assembly, execution,
review, completion, and blocker reporting.

**Files created:**
- `editor/src/WorkflowOrchestrator.h` — orchestration core:
  - `step()` (advance one ready item one stage-set)
  - `advance()` (advance all currently ready items)
  - `runToCompletion()` (progress until no further non-blocked progress)
  - `runUntil(predicate)` (predicate-controlled progression)
  - `getBlockers()` with typed blockers:
    `needs-human`, `needs-review`, `needs-external-model`
  - event stream via `OrchestratorEvent` (`routed`, `context-assembled`,
    `executed`, `auto-approved`, `sent-to-review`, `completed`, `blocked`)
- `editor/tests/step378_test.cpp` — 12 tests covering:
  1. single deterministic/template completion in one step
  2. dependency cascade promotion
  3. human-task blocker classification
  4. agent-task external-model blocker classification
  5. `runToCompletion()` deterministic completion behavior
  6. stage event emission coverage
  7. review blocker reporting
  8. empty-workflow no-op behavior
  9. mixed workflow partial progress behavior
  10. `runUntil(...)` predicate stop behavior
  11. blocked-event detail payload includes blockers
  12. agent context payload preservation

**Files modified:**
- `editor/CMakeLists.txt` — `step378_test` target

**Verification run:**
- `step378_test` — PASS (12/12) new step coverage
- `step377_test` — PASS (8/8) regression coverage
- `step376_test` — PASS (12/12) regression coverage
- `step375_test` — PASS (12/12) regression coverage
- `step331_test` — PASS (8/8) orchestration-foundation regression

**Architecture gate check:**
- `editor/src/WorkflowOrchestrator.h` is within header size limit
  (`231` lines <= `600`)

### Step 379: Parallel Dispatch + Batching
**Status:** PASS (12/12 tests)

Extended the orchestration engine with explicit batch advancement, shared
project-context reuse, and batch-level progress accounting.

**Files created:**
- `editor/tests/step379_test.cpp` — 12 tests covering:
  1. independent tasks advancing in one batch
  2. shared project-context token savings
  3. blocked item accounting in batch results
  4. dependency waiting behavior in batch mode
  5. priority ordering in batch event stream
  6. empty-batch behavior
  7. single-item batch behavior
  8. mixed independent/dependent behavior
  9. `advance()` compatibility over batched events
  10. shared-context event marker emission
  11. review-path handling in batch mode
  12. `runToCompletion()` stop on blockers

**Files modified:**
- `editor/src/WorkflowOrchestrator.h` — add batch orchestration support:
  - `BatchResult` struct (`events`, `itemsAdvanced`, `itemsBlocked`,
    `contextTokensSaved`)
  - `advanceBatch()` for multi-item ready-queue advancement
  - shared project-context reuse path with token-savings accounting
  - `advance()` now returns `advanceBatch().events` for API continuity
  - event detail now indicates shared project-context reuse
- `editor/CMakeLists.txt` — `step379_test` target

**Verification run:**
- `step379_test` — PASS (12/12) new step coverage
- `step378_test` — PASS (12/12) regression coverage
- `step377_test` — PASS (8/8) regression coverage
- `step331_test` — PASS (8/8) workflow-foundation regression
- `step325_test` — PASS (8/8) workflow-state regression

**Architecture gate check:**
- `editor/src/WorkflowOrchestrator.h` remains within header-size limit
  (`284` lines <= `600`)

### Step 380: Feedback Loop — Rejection Re-Routing
**Status:** PASS (12/12 tests)

Implemented rejection-aware rerouting in the orchestration loop with
attempt-history preservation, stepwise escalation, and feedback-aware context
assembly for re-attempts.

**Files created:**
- `editor/tests/step380_test.cpp` — 12 tests covering:
  1. review rejection re-enters ready queue
  2. rejection feedback appears in next worker context bundle
  3. first escalation template/deterministic -> slm
  4. second escalation slm -> llm
  5. third escalation llm -> human
  6. rejection history metadata preservation
  7. multi-rejection accumulation
  8. reviewer feedback-hints (`context=...`, `worker=...`) applied
  9. context widening after rejection
  10. no escalation-level skipping across repeated rejections
  11. human rejection remains human-routed
  12. rejection count tracking + reject-state guard

**Files modified:**
- `editor/src/WorkItem.h` — add `RejectionAttempt` history on `WorkItem`,
  add first-class `rejectionFeedback` field, and JSON serialization/deserialization
  support for both
- `editor/src/RoutingEngine.h` — add `routeWithHistory(...)` and
  stepwise `escalateWorker(...)` policy that advances one level per rejection
  (template/deterministic -> slm -> llm -> human) without skipping
- `editor/src/WorkflowOrchestrator.h` — add `rejectAndRequeue(...)`,
  preserve attempt metadata, apply reviewer feedback hints, and route
  with rejection history
- `editor/src/ContextAssembler.h` — include first-class rejection feedback
  in `WorkerContext.feedbackFromRejection` (with backward-compatible fallback)
- `editor/CMakeLists.txt` — `step380_test` target

**Verification run:**
- `step380_test` — PASS (12/12) new step coverage
- `step379_test` — PASS (12/12) regression coverage
- `step378_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/WorkflowOrchestrator.h` within header-size limit (`324` <= `600`)
- `editor/src/RoutingEngine.h` within header-size limit (`242` <= `600`)
- `editor/src/WorkItem.h` within header-size limit (`260` <= `600`)
- `editor/src/ContextAssembler.h` within header-size limit (`192` <= `600`)
- `editor/tests/step380_test.cpp` within test-file size guidance (`281` lines)

### Step 381: Progress Tracking + ETA
**Status:** PASS (12/12 tests)

Implemented a standalone workflow-progress tracker that consumes orchestrator
events and produces progress snapshots with completion %, throughput, ETA,
worker metrics, blockers, and timeline entries.

**Files created:**
- `editor/src/WorkflowProgress.h` — progress aggregation API:
  - `WorkflowProgress::recordEvent(...)`
  - `WorkflowProgress::getSnapshot()`
  - `WorkflowProgress::getTimeline()`
  - `WorkflowProgress::getWorkerStats()`
  - `ProgressSnapshot`, `WorkerStats`, `TimelineEntry` with JSON serialization
- `editor/tests/step381_test.cpp` — 12 tests covering:
  1. completion-percent accuracy
  2. throughput (`itemsPerMinute`) calculation
  3. ETA bound sanity for uniform completion cadence
  4. per-worker metric aggregation
  5. rejection-rate tracking
  6. timeline event recording
  7. empty-workflow snapshot behavior
  8. blocker propagation into snapshots
  9. snapshot JSON field coverage
  10. completion monotonicity across rejection events
  11. `startedAt`/`lastActivityAt` tracking
  12. total-item fallback via observed item IDs

**Files modified:**
- `editor/CMakeLists.txt` — `step381_test` target

**Verification run:**
- `step381_test` — PASS (12/12) new step coverage
- `step380_test` — PASS (12/12) regression coverage
- `step379_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/WorkflowProgress.h` within header-size limit (`303` <= `600`)
- `editor/tests/step381_test.cpp` within test-file size guidance (`196` lines)

### Step 382: Orchestrator RPC + MCP
**Status:** PASS (12/12 tests)

Added orchestrator control RPC methods and MCP tool bindings so a client can
step/advance workflows, run deterministic items, inspect blockers/progress, and
submit external model output back into the workflow loop.

**Files created:**
- `editor/src/HeadlessOrchestratorRPC.h` — extracted orchestrator RPC handling:
  - `orchestrateStep`
  - `orchestrateAdvance`
  - `orchestrateRunDeterministic`
  - `getBlockers`
  - `getProgress`
  - `submitExternalResult`
  - event/blocker JSON serialization helpers and buffer-context collection
- `editor/tests/step382_test.cpp` — 12 tests covering:
  1. `orchestrateStep` advances one task
  2. `orchestrateAdvance` processes batch
  3. `orchestrateRunDeterministic` stops at blockers
  4. `getBlockers` reports human blockers
  5. `getProgress` snapshot shape
  6. `submitExternalResult` accepts LLM output and advances lifecycle
  7. Linter role can read progress but cannot orchestrate
  8. MCP tool registration for all 6 orchestrator tools
  9. MCP `whetstone_orchestrate_run_deterministic` callback wiring
  10. combined deterministic + external-submit + progress flow
  11. submit-result worker-type guard
  12. no-workflow guard behavior

**Files modified:**
- `editor/src/HeadlessAgentRPCHandler.h` — delegated orchestrator-method
  handling to `tryHandleHeadlessOrchestratorRPC(...)`
- `editor/src/HeadlessEditorState.h` — add `workflowProgress` state
- `editor/src/AgentPermissionPolicy.h` — permissions for new orchestrator methods
  (`getProgress/getBlockers` read-only, orchestration/submit methods write)
- `editor/src/MCPServer.h` — register orchestrator MCP tools:
  - `whetstone_orchestrate_step`
  - `whetstone_orchestrate_advance`
  - `whetstone_orchestrate_run_deterministic`
  - `whetstone_get_blockers`
  - `whetstone_get_progress`
  - `whetstone_submit_result`
- `editor/CMakeLists.txt` — `step382_test` target

**Verification run:**
- `step382_test` — PASS (12/12) new step coverage
- `step381_test` — PASS (12/12) regression coverage
- `step380_test` — PASS (12/12) regression coverage
- `step379_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/HeadlessOrchestratorRPC.h` within header-size limit (`287` <= `600`)
- `editor/src/WorkflowProgress.h` within header-size limit (`303` <= `600`)
- `editor/tests/step382_test.cpp` within test-file size guidance (`250` lines)
- Legacy oversized headers remain and should be split in follow-up:
  - `editor/src/HeadlessAgentRPCHandler.h` (`2623` > `600`)
  - `editor/src/MCPServer.h` (`1652` > `600`)

### Step 383: Phase 15a Integration Tests
**Status:** PASS (8/8 tests)

Added end-to-end integration validation for Sprint 15a orchestration behavior:
mixed deterministic/agent/human flows, dependency gating, rejection escalation,
shared-context optimization, progress tracking, external-result submission, and
MCP orchestration tool loop closure.

**Files created:**
- `editor/tests/step383_test.cpp` — 8 integration tests covering:
  1. mixed 5-item workflow partial deterministic completion profile
  2. dependency-chain progression constraints
  3. rejection escalation through deterministic -> slm -> llm
  4. shared project-context token-savings path
  5. progress snapshot updates during orchestration
  6. `submitExternalResult` completion path for agent-prepared task
  7. blocker-category surfacing (`needs-human`, `needs-external-model`)
  8. MCP end-to-end loop (`run_deterministic` -> `submit_result` -> `get_progress`)

**Files modified:**
- `editor/CMakeLists.txt` — `step383_test` target

**Verification run:**
- `step383_test` — PASS (8/8) new integration coverage
- `step382_test` — PASS (12/12) regression coverage
- `step381_test` — PASS (12/12) regression coverage
- `step380_test` — PASS (12/12) regression coverage

**Sprint 15a completion snapshot (Steps 378-383):**
- Step 378: orchestrator core loop
- Step 379: batching + shared-context optimization
- Step 380: rejection rerouting feedback loop
- Step 381: progress/ETA tracking
- Step 382: orchestrator RPC + MCP tools
- Step 383: phase integration coverage

**Architecture gate check (end of Sprint 15a):**
- `editor/tests/step383_test.cpp` within test-file size guidance (`222` lines)
- `editor/src/HeadlessOrchestratorRPC.h` within header-size limit (`287` <= `600`)
- `editor/src/WorkflowProgress.h` within header-size limit (`303` <= `600`)
- Legacy oversized headers persist and need planned split before Sprint 15b+:
  - `editor/src/HeadlessAgentRPCHandler.h` (`2623` > `600`)
  - `editor/src/MCPServer.h` (`1652` > `600`)

### Step 384: Workflow Session Protocol
**Status:** PASS (12/12 tests)

Implemented a formal MCP workflow-session protocol model with explicit phases,
transition validation, command-history tracking, workflow-state phase inference,
and next-action guidance for clients.

**Files created:**
- `editor/src/WorkflowProtocol.h` — protocol model:
  - `WorkflowSession` (session metadata + command history)
  - `ProtocolAction` (phase + suggested command + reason)
  - phase helpers: `workflowProtocolPhases`, `validateTransition`,
    `protocolPhaseForCommand`, `recordWorkflowCommand`
  - workflow-aware helpers: `detectProtocolPhaseFromWorkflow`,
    `getNextAction`, `getSessionSummary`
- `editor/tests/step384_test.cpp` — 12 tests covering:
  1. phase-order transition validity
  2. invalid transition rejection
  3. per-phase next-action suggestions
  4. command-history tracking
  5. session-summary accuracy
  6. complete-phase auto-detection from workflow
  7. assist-phase auto-detection from workflow
  8. session JSON roundtrip persistence
  9. empty-project plan behavior
  10. unknown-phase guards
  11. monotonic phase advancement via command recording
  12. workflow-aware next-action override

**Files modified:**
- `editor/CMakeLists.txt` — `step384_test` target

**Verification run:**
- `step384_test` — PASS (12/12) new step coverage
- `step383_test` — PASS (8/8) regression coverage
- `step382_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/WorkflowProtocol.h` within header-size limit (`185` <= `600`)
- `editor/tests/step384_test.cpp` within test-file size guidance (`177` lines)

### Step 385: Context Bundle Format
**Status:** PASS (12/12 tests)

Standardized the external-model context contract with a structured bundle,
prompt rendering, JSON transport format, and token estimation utilities.

**Files created:**
- `editor/src/ContextBundle.h` — context bundle model:
  - `ContextBundle` + `AttemptSummary`
  - `buildBundle(workItem, workerContext, routingDecision)`
  - `bundleToPrompt(bundle)` (model-agnostic prompt rendering)
  - `bundleToJson(bundle)` / `ContextBundle::fromJson(...)`
  - `estimateBundleTokens(bundle)`
- `editor/tests/step385_test.cpp` — 12 tests covering:
  1. required bundle field population
  2. prompt structure readability
  3. rejection-history inclusion
  4. token-estimate reasonableness
  5. routing budget propagation
  6. annotation-constraint extraction
  7. intent propagation
  8. empty-field handling
  9. JSON roundtrip
  10. model-agnostic prompt format
  11. output-format policy (`code-only` vs `code-with-explanation`)
  12. previous-attempt ordering

**Files modified:**
- `editor/CMakeLists.txt` — `step385_test` target

**Verification run:**
- `step385_test` — PASS (12/12) new step coverage
- `step384_test` — PASS (12/12) regression coverage
- `step383_test` — PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/ContextBundle.h` within header-size limit (`168` <= `600`)
- `editor/tests/step385_test.cpp` within test-file size guidance (`192` lines)

### Step 386: Result Acceptance Protocol
**Status:** PASS (12/12 tests)

Added a standardized acceptance pipeline for external model submissions:
syntax/annotation validation, diagnostic accounting, review-gate decisioning,
structured acceptance output, dependency cascade on acceptance, and requeue on
validation failure.

**Files created:**
- `editor/src/ResultAcceptance.h` — acceptance primitives:
  - `ResultSubmission`
  - `ResultAcceptance`
  - `validateSuggestedAnnotations(...)`
  - `evaluateResultSubmission(...)`
- `editor/tests/step386_test.cpp` — 12 tests covering:
  1. valid code acceptance
  2. syntax rejection with feedback
  3. malformed suggested-annotation rejection
  4. diagnostics preventing auto-approval
  5. low-confidence review path
  6. suggested-annotation payload carry-through
  7. aggregated diagnostic counts across validation stages
  8. partial acceptance (valid + review required)
  9. multiple-submission latest-wins behavior
  10. dependency cascade on acceptance
  11. acceptance payload in RPC response
  12. non-agent worker submission guard

**Files modified:**
- `editor/src/HeadlessOrchestratorRPC.h` — `submitExternalResult` now uses
  `evaluateResultSubmission(...)`, emits acceptance metadata, supports review
  resubmission, and requeues invalid submissions with validator feedback
- `editor/CMakeLists.txt` — `step386_test` target

**Verification run:**
- `step386_test` — PASS (12/12) new step coverage
- `step385_test` — PASS (12/12) regression coverage
- `step384_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ResultAcceptance.h` within header-size limit (`125` <= `600`)
- `editor/src/HeadlessOrchestratorRPC.h` within header-size limit (`309` <= `600`)
- `editor/tests/step386_test.cpp` within test-file size guidance (`269` lines)

### Step 387: Orchestrator Event Stream
**Status:** PASS (12/12 tests)

Added a versioned event stream for orchestration telemetry and exposed polling
APIs through RPC + MCP for client-side visualization and progress UIs.

**Files created:**
- `editor/src/EventStream.h` — versioned stream:
  - `emit(OrchestratorEvent)`
  - `poll(sinceVersion)`
  - `subscribe(callback)`
  - `getVersion()`
  - `getRecent(count)`
  - stream-event JSON serialization
- `editor/tests/step387_test.cpp` — 12 tests covering:
  1. emit/poll baseline
  2. since-version filtering
  3. version tracking
  4. subscriber callback firing
  5. normalized orchestration event-type emission
  6. recent-event ordering/count
  7. empty stream behavior
  8. no-duplicate polling under frequent reads
  9. event JSON structure
  10. MCP event-stream tool registration
  11. `getEventStream` since-version behavior
  12. `getRecentEvents` count behavior

**Files modified:**
- `editor/src/HeadlessEditorState.h` — add `EventStream eventStream`
- `editor/src/HeadlessAgentRPCHandler.h` — emit `workflow.created` event on workflow init
- `editor/src/HeadlessOrchestratorRPC.h` — event emission mapping and new RPC methods:
  - `getEventStream`
  - `getRecentEvents`
  - mapped event names (`task.*`, `workflow.*`) and `workflow.progress/complete` emission
- `editor/src/AgentPermissionPolicy.h` — read permissions for
  `getEventStream` / `getRecentEvents`
- `editor/src/MCPServer.h` — MCP tools:
  - `whetstone_get_event_stream`
  - `whetstone_get_recent_events`
- `editor/CMakeLists.txt` — `step387_test` target

**Verification run:**
- `step387_test` — PASS (12/12) new step coverage
- `step386_test` — PASS (12/12) regression coverage
- `step382_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/EventStream.h` within header-size limit (`68` <= `600`)
- `editor/src/HeadlessOrchestratorRPC.h` within header-size limit (`374` <= `600`)
- `editor/tests/step387_test.cpp` within test-file size guidance (`204` lines)
- Legacy oversized header persists:
  - `editor/src/MCPServer.h` (`1679` > `600`)

### Step 388: Phase 15b Integration — Full Protocol Test
**Status:** PASS (8/8 tests)

Added end-to-end protocol integration coverage connecting session protocol,
orchestrator/event-stream RPC, context bundles, acceptance pipeline, and
progress tracking into one executable workflow path.

**Files created:**
- `editor/tests/step388_test.cpp` — 8 integration tests covering:
  1. full protocol walkthrough to completion
  2. event stream transition capture
  3. context bundle core-field integrity
  4. acceptance behavior by worker type (deterministic vs llm)
  5. rejection -> reroute -> improved submission -> acceptance
  6. progress snapshot monotonic advancement
  7. `getNextAction` protocol guidance
  8. workflow-session persistence/resume

**Files modified:**
- `editor/CMakeLists.txt` — `step388_test` target

**Verification run:**
- `step388_test` — PASS (8/8) new integration coverage
- `step387_test` — PASS (12/12) regression coverage
- `step386_test` — PASS (12/12) regression coverage
- `step385_test` — PASS (12/12) regression coverage

**Phase 15b completion snapshot (Steps 384-388):**
- Step 384: workflow session protocol
- Step 385: context bundle format
- Step 386: result acceptance protocol
- Step 387: orchestrator event stream + RPC/MCP exposure
- Step 388: full protocol integration

**Architecture gate check (end of Phase 15b):**
- `editor/tests/step388_test.cpp` within test-file size guidance (`224` lines)
- `editor/src/WorkflowProtocol.h` within header-size limit (`185` <= `600`)
- `editor/src/ContextBundle.h` within header-size limit (`168` <= `600`)
- `editor/src/ResultAcceptance.h` within header-size limit (`125` <= `600`)
- `editor/src/EventStream.h` within header-size limit (`68` <= `600`)
- `editor/src/HeadlessOrchestratorRPC.h` within header-size limit (`374` <= `600`)
- Legacy oversized header persists:
  - `editor/src/MCPServer.h` (`1679` > `600`)

## Phase 15c: Advanced Routing + Optimization

### Step 389: Routing Rules Engine
**Status:** PASS (12/12 tests)

Configurable, composable routing rules engine. Rules have conditions (AND-combined)
and actions. First matching rule wins. Default rules encode same logic as the
hardcoded RoutingEngine cascade.

**Files created:**
- `editor/src/RoutingRules.h` — RuleCondition, RoutingAction, RoutingRule structs;
  RulesEngine class with addRule, evaluate, getDefaultRules, loadRules/saveRules;
  7 condition types: annotation, complexity, contextWidth, nodeType, rejectionCount,
  language, pattern
- `editor/tests/step389_test.cpp` — 12 tests covering first-match wins, AND conditions,
  annotation match, complexity threshold, pattern matching, defaults, custom override,
  fallthrough, priority ordering, JSON persistence, budget multiplier, rejection escalation

### Step 390: Cost Estimation + Optimization
**Status:** PASS (12/12 tests)

Token cost estimation and optimization suggestions before executing a workflow.
Estimates context/output tokens per worker type with rough USD cost projection.

**Files created:**
- `editor/src/CostEstimator.h` — OptimizationSuggestion, CostEstimate structs;
  CostEstimator class with estimate() and suggest(); collectAllItems() helper;
  3 suggestion types: use-template, narrow-context, batch
- `editor/tests/step390_test.cpp` — 12 tests covering token counts, deterministic
  zero cost, suggestions, getter detection, context narrowing, batching, empty
  workflow, large workflow, JSON serialization, worker breakdown, completed exclusion,
  cost proportionality

### Step 391: Workflow Templates
**Status:** PASS (12/12 tests)

5 pre-built workflow templates for common project patterns. Each creates a populated
WorkflowState with work items, routing hints, and dependencies.

**Files created:**
- `editor/src/WorkflowTemplates.h` — TemplateInfo, TemplateParams structs;
  WorkflowTemplates class with getTemplates(), applyTemplate(), isValidTemplate();
  Templates: crud-api, module-refactor, cross-language-port, test-suite,
  legacy-modernization
- `editor/tests/step391_test.cpp` — 12 tests covering template count, CRUD skeleton,
  cross-language routing, param validation, test generation, valid WorkflowState,
  refactor template, descriptions, default params, modernization, dependencies,
  function count defaults

### Step 392: Dependency Graph Data
**Status:** PASS (12/12 tests)

Export workflow dependency graph as structured data for Sprint 19's GUI visualization.
Computes critical path (longest dependency chain).

**Files created:**
- `editor/src/DependencyGraph.h` — GraphNode, GraphEdge, GraphData structs;
  buildDependencyGraph(), graphToJson(), getCriticalPath() functions
- `editor/tests/step392_test.cpp` — 12 tests covering node count, edge matching,
  critical path, status match, worker type, JSON serialization, empty graph,
  single node, diamond dependencies, update after completion, labels, 5-chain path

### Step 393: Phase 15c Integration + Sprint 15 Summary
**Status:** PASS (8/8 tests)

End-to-end Sprint 15c validation connecting routing rules, cost estimation,
workflow templates, dependency graph, event stream, and protocol phases.

**Files created:**
- `editor/tests/step393_test.cpp` — 8 integration tests covering:
  1. custom routing rules override
  2. cost estimation with optimization suggestions for 10-function workflow
  3. CRUD template → route → deterministic execution
  4. dependency graph 5-function chain critical path
  5. event stream captures workflow transitions
  6. full protocol phase validation
  7. optimization reduces cost (narrow context)
  8. Sprint 15 totals verification

**Sprint 15 completion snapshot (Steps 378-393):**
- Phase 15a (378-383): Orchestrator core, parallel dispatch, feedback, progress, RPC
- Phase 15b (384-388): MCP protocol, context bundles, result acceptance, event stream
- Phase 15c (389-393): Rules engine, cost estimation, templates, dependency graph

**Sprint 15 totals:**
- Steps: 16 (378-393)
- Tests: ~180 (68 + 56 + 56)
- New headers: 9 (Orchestrator, WorkflowProgress, WorkflowProtocol, ContextBundle,
  EventStream, RoutingRules, CostEstimator, WorkflowTemplates, DependencyGraph)

**Architecture gate check (end of Sprint 15):**
- `editor/src/RoutingRules.h` within header-size limit (`276` <= `600`)
- `editor/src/CostEstimator.h` within header-size limit (`227` <= `600`)
- `editor/src/WorkflowTemplates.h` within header-size limit (`220` <= `600`)
- `editor/src/DependencyGraph.h` within header-size limit (`175` <= `600`)
- Legacy oversized header persists:
  - `editor/src/MCPServer.h` (`1679` > `600`)

# Sprint 16 Progress — C++ Depth + Self-Hosting Phase 1

## Phase 16a: C++ Type System Depth

### Step 394: TypeAlias + Nested Type Access
**Status:** PASS (12/12 tests)

Verified TypeAlias parsing (using/typedef), nested qualified names (Foo::Bar::Baz),
and CppQualifiers detection helpers.

**Files created:**
- `editor/src/ast/CppAdvancedNodes.h` — CppQualifiers struct (const/constexpr/static/mutable/
  smartPointerKind), AutoType node, CastExpression node, detection helpers
  (detectSmartPointerKind, extractSmartPointerInner, detectQualifiers, isAutoType,
  isConstMethod, detectCastKind, smartPointerOwnership, castRiskLevel)
- `editor/tests/step394_test.cpp` — 12 tests covering using/typedef parsing, nested types,
  construction, multiple aliases, generator format, qualified names, namespaces,
  span tracking, template targets, pointer typedefs, qualifier detection

### Step 395: Static Members + Const/Constexpr
**Status:** PASS (12/12 tests)

Tests for const, constexpr, static, and mutable qualifier detection on methods,
variables, and class members. MethodDeclaration already supports isStatic/isVirtual/
isOverride flags.

**Files created:**
- `editor/tests/step395_test.cpp` — 12 tests covering const/constexpr/static detection,
  trailing const methods, static method parsing, virtual/override, CppQualifiers
  JSON roundtrip, field declarations, combined qualifiers, mutable detection

### Step 396: Smart Pointer Patterns
**Status:** PASS (12/12 tests)

Recognition of unique_ptr/shared_ptr/weak_ptr, inner type extraction, ownership
annotation mapping (Unique/Shared_ARC/Weak), and cross-language projection concept.

**Files created:**
- `editor/tests/step396_test.cpp` — 12 tests covering detection of each smart pointer
  type, inner type extraction, ownership mapping, parser integration with smart
  pointer params/returns, no false positives, make_unique, cross-language mapping,
  new/delete expressions

### Step 397: Auto Type Deduction
**Status:** PASS (12/12 tests)

AutoType AST node for deferred type resolution. Parser handles auto, auto*, auto&,
const auto&, trailing return types, and C++20 abbreviated templates.

**Files created:**
- `editor/tests/step397_test.cpp` — 12 tests covering AutoType construction, modifiers,
  isAutoType detection, auto return types, auto variables, auto pointers, const auto
  refs, trailing return types, auto parameters, span tracking

### Step 398: Cast Expressions
**Status:** PASS (12/12 tests)

CastExpression AST node with castKind (static/dynamic/reinterpret/const), targetType,
and operand child. Risk level annotations: reinterpret_cast=high, dynamic/const=medium,
static=low.

**Files created:**
- `editor/tests/step398_test.cpp` — 12 tests covering construction, detection of all
  4 cast kinds, risk levels, parser integration with cast expressions, operand
  children, multiple casts, span tracking, risk annotation mapping

### Step 399: Phase 16a Integration
**Status:** PASS (8/8 tests)

End-to-end Phase 16a validation combining all new C++ constructs.

**Files created:**
- `editor/tests/step399_test.cpp` — 8 integration tests covering:
  1. Parse Whetstone-style header with type alias, class, smart pointers
  2. Roundtrip parse → generate → parse
  3. Qualifier detection on mixed code
  4. Smart pointer ownership mapping
  5. Cast risk assessment
  6. Auto type detection
  7. Complex class with static/virtual/const methods
  8. Pipeline integration (C++ parse + infer + validate + generate)

**Phase 16a completion snapshot (Steps 394-399):**
- Step 394: TypeAlias + nested type access (12 tests)
- Step 395: Static/const/constexpr qualifiers (12 tests)
- Step 396: Smart pointer patterns (12 tests)
- Step 397: Auto type deduction (12 tests)
- Step 398: Cast expressions (12 tests)
- Step 399: Phase 16a integration (8 tests)

**Architecture gate check (end of Phase 16a):**
- `editor/src/ast/CppAdvancedNodes.h` within header-size limit (`156` <= `600`)
- Legacy oversized header persists:
  - `editor/src/MCPServer.h` (`1679` > `600`)

## Phase 16b: Self-Hosting Phase 1

### Step 400: Self-Hosting Test Harness
**Status:** PASS (12/12 tests)

Added a reusable self-hosting harness for parsing real project headers,
checking expected structure, and reporting construct coverage with opaque
tracking for graceful degradation.

**Files created:**
- `editor/src/SelfHostHarness.h` — parseFile/parseSource helpers, expected
  structure checks, coverage tracking, coverage report output
- `editor/tests/step400_test.cpp` — 12 tests for parsing, coverage accounting,
  graceful degradation, and report formatting

**Files modified:**
- `editor/CMakeLists.txt` — `step400_test` target

**Architecture gate check:**
- `editor/src/SelfHostHarness.h` within header-size limit (`308` <= `600`)
- `editor/tests/step400_test.cpp` within test-file size guidance (`256` lines)

### Step 401: Parse AnnotationConflictExtended.h
**Status:** PASS (12/12 tests)

Validated self-hosting against a real project header (`AnnotationConflictExtended.h`):
struct and free-function capture, parameter/body extraction, field-name fidelity,
coverage reporting, and graceful degradation on missing expected constructs.

**Files created:**
- `editor/tests/step401_test.cpp` — 12 tests covering:
  1. parse real header file from disk
  2. `CrossTypeConflict` struct capture
  3. `collectCrossTypeConflicts` signature capture
  4. expected-structure 100% coverage
  5. struct field count
  6. struct field-name preservation
  7. function parameter extraction
  8. function body presence
  9. coverage report content checks
  10. lambda-heavy header pattern parse stability
  11. STL-heavy type pattern parse stability
  12. graceful degradation via opaque coverage accounting

**Files modified:**
- `editor/CMakeLists.txt` — `step401_test` target

**Verification run:**
- `step401_test` — PASS (12/12) new step coverage
- `step400_test` — PASS (12/12) regression coverage
- `step399_test` — PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/tests/step401_test.cpp` within test-file size guidance (`231` lines)
- `editor/src/SelfHostHarness.h` remains within header-size limit (`308` <= `600`)
- Legacy oversized headers persist:
  - `editor/src/MCPServer.h` (`1679` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2623` > `600`)

### Step 402: Parse AnnotationValidatorExtended.h
**Status:** PASS (12/12 tests)

Validated self-hosting parse coverage for a class-heavy production header
(`AnnotationValidatorExtended.h`): class/method extraction, static helper
method recognition, validator-style cast-pattern helper checks, complexity
inference on parsed AST, and graceful degradation accounting.

**Files created:**
- `editor/tests/step402_test.cpp` — 12 tests covering:
  1. parse real header file from disk
  2. `AnnotationValidatorExtended` class capture
  3. class method extraction
  4. public `validate` signature capture
  5. private helper method capture (`validateNode`, `validateAnnotation`)
  6. static helper method parsing (`isPowerOf2`, `isOneOf`)
  7. expected-structure 100% coverage
  8. validator-style `static_cast` detection helper checks
  9. complexity inference execution on parsed AST
  10. static-helper class snippet parse stability
  11. coverage report content checks
  12. graceful degradation via opaque coverage accounting

**Files modified:**
- `editor/CMakeLists.txt` — `step402_test` target

**Verification run:**
- `step402_test` — PASS (12/12) new step coverage
- `step401_test` — PASS (12/12) regression coverage
- `step400_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/tests/step402_test.cpp` within test-file size guidance (`239` lines)
- `editor/src/AnnotationValidatorExtended.h` within header-size limit (`349` <= `600`)
- `editor/src/SelfHostHarness.h` remains within header-size limit (`308` <= `600`)
- Legacy oversized headers persist:
  - `editor/src/MCPServer.h` (`1679` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2623` > `600`)

### Step 403: Annotate + Generate from Self-Hosted AST
**Status:** PASS (12/12 tests)

Added self-hosting coverage for inference + generation workflows over real
project headers (`AnnotationConflictExtended.h` and `AnnotationValidatorExtended.h`),
including pipeline execution and Semanno roundtrip checks from inferred
complexity metadata.

**Files created:**
- `editor/tests/step403_test.cpp` — 12 tests covering:
  1. parse both self-host headers
  2. inference output on conflict AST
  3. complexity inference on validator AST
  4. C++ generation from conflict AST
  5. C++ generation from validator AST
  6. conflict parse/generate/parse roundtrip
  7. validator parse/generate/parse roundtrip
  8. `Pipeline.run()` success for conflict header
  9. `Pipeline.run()` success for validator header
  10. Semanno emit/parse roundtrip for inferred `ComplexityAnnotation`
  11. regenerated output parseability check
  12. combined generated-source parseability check

**Files modified:**
- `editor/CMakeLists.txt` — `step403_test` target

**Verification run:**
- `step403_test` — PASS (12/12) new step coverage
- `step402_test` — PASS (12/12) regression coverage
- `step401_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/tests/step403_test.cpp` within test-file size guidance (`236` lines)
- Legacy oversized headers persist:
  - `editor/src/MCPServer.h` (`1679` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2623` > `600`)

### Step 404: Self-Hosting Progress Report + Phase Integration
**Status:** PASS (8/8 tests)

Completed Phase 16b integration reporting with aggregate self-host coverage,
pipeline execution checks over both target headers, Semanno roundtrip checks
from inferred complexity metadata, and explicit gap-reporting behavior.

**Files created:**
- `editor/tests/step404_test.cpp` — 8 integration tests covering:
  1. per-file self-host coverage metrics
  2. baseline per-file coverage ratios
  3. aggregate phase coverage accounting
  4. `Pipeline.run()` success for both target headers
  5. gap-reporting path for opaque construct deficits
  6. Semanno emit/parse check for inferred complexity
  7. report formatting (file identity + parse status)
  8. phase artifact sanity (step400-404 test files present)

**Files modified:**
- `editor/CMakeLists.txt` — `step404_test` target

**Verification run:**
- `step404_test` — PASS (8/8) new integration coverage
- `step403_test` — PASS (12/12) regression coverage
- `step402_test` — PASS (12/12) regression coverage

**Phase 16b completion snapshot (Steps 400-404):**
- Step 400: Self-hosting harness + coverage tracking (12 tests)
- Step 401: Parse `AnnotationConflictExtended.h` (12 tests)
- Step 402: Parse `AnnotationValidatorExtended.h` (12 tests)
- Step 403: Inference + generation on self-hosted ASTs (12 tests)
- Step 404: Coverage report + phase integration (8 tests)

**Sprint 16 completion snapshot (Steps 394-404):**
- Phase 16a complete (Steps 394-399, 68 tests)
- Phase 16b complete (Steps 400-404, 56 tests)
- Sprint 16 total: 11 steps, 124 tests

**Architecture gate check (end of Sprint 16):**
- `editor/tests/step404_test.cpp` within test-file size guidance (`219` lines)
- `editor/tests/step403_test.cpp` within test-file size guidance (`236` lines)
- `editor/tests/step402_test.cpp` within test-file size guidance (`239` lines)
- `editor/src/SelfHostHarness.h` within header-size limit (`308` <= `600`)
- `editor/src/AnnotationValidatorExtended.h` within header-size limit (`349` <= `600`)
- Legacy oversized headers persist and remain queued for split work:
  - `editor/src/MCPServer.h` (`1679` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2623` > `600`)

# Sprint 17 Progress — Language Batch 2

## Phase 17a: F# Parser + Generator

### Step 405: F# Parser
**Status:** PASS (12/12 tests)

Added a standalone F# parser covering core functional declarations and routing.
Parses top-level `let` function forms, mutable variables, discriminated unions,
record types, module declarations, async computation expressions, match markers,
and pipe-operator chains.

**Files created:**
- `editor/src/ast/FSharpParser.h` — F# parse support:
  - `parseFSharp(...)` and `parseFSharpWithDiagnostics(...)`
  - `let name params = ...` → `Function`
  - `let mutable x = ...` → `Variable` + `MutAnnotation`
  - `type Name = | Case...` (single-line/multi-line) → `EnumDeclaration`
  - `type Name = { field: Type; ... }` → `ClassDeclaration` with field children
  - `module Name` → `NamespaceDeclaration`
  - `match ... with` marker → `IfStatement`
  - `|>` pipeline chains → `FunctionCall` markers
  - indentation-aware top-level handling for significant-whitespace behavior
- `editor/tests/step405_test.cpp` — 12 tests covering:
  1. `let` function parsing
  2. mutable variable parsing
  3. discriminated union parsing
  4. record type parsing
  5. match mapping marker
  6. async computation parsing
  7. module/namespace parsing
  8. pipeline operator chain parsing
  9. significant-whitespace top-level behavior
  10. diagnostics entry point
  11. mixed top-level forms
  12. pipeline parse routing for `fsharp` / `f#` / `fs`

**Files modified:**
- `editor/src/ast/Parser.h` — include `ast/FSharpParser.h`
- `editor/src/Pipeline.h` — parse routing for `fsharp`, `f#`, `fs`
- `editor/CMakeLists.txt` — `step405_test` target

**Verification run:**
- `step405_test` — PASS (12/12) new step coverage
- `step404_test` — PASS (8/8) regression coverage
- `step403_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ast/FSharpParser.h` within header-size limit (`265` <= `600`)
- `editor/tests/step405_test.cpp` within test-file size guidance (`220` lines)
- Legacy oversized headers persist:
  - `editor/src/MCPServer.h` (`1679` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2623` > `600`)

### Step 406: F# Generator
**Status:** PASS (12/12 tests)

Added a dedicated F# generator with Semanno annotation emission support and
pipeline generate-route aliases (`fsharp`, `f#`, `fs`). Outputs functional
`let` forms, async computation expressions, discriminated-union style enum
output, record-style class output, and F#-style type mappings.

**Files created:**
- `editor/src/ast/FSharpGenerator.h` — F# generation support:
  - module/function/variable generation using `let`
  - mutable variable output from `MutAnnotation`
  - async output: `let name = async { ... }`
  - namespace/module output (`module Name`)
  - union output (`type Name = | Case...`)
  - record output (`type Name = { field: obj; ... }`)
  - pipeline marker generation (`|> pipeline`)
  - type mapping for `int`, `float`, `string`, `bool`, `unit`, `option`, `result`
  - Semanno output for subject 1 and extended annotation families
- `editor/tests/step406_test.cpp` — 12 tests covering:
  1. `let` function generation
  2. mutable variable generation
  3. discriminated union generation
  4. record/class generation
  5. async function generation
  6. module namespace generation
  7. if-statement generation
  8. pipeline marker generation
  9. type mapping behavior
  10. comment prefix (`// `)
  11. Semanno annotation output
  12. pipeline generate routing for `fsharp` / `f#` / `fs`

**Files modified:**
- `editor/src/ast/Generator.h` — include `FSharpGenerator.h`
- `editor/src/Pipeline.h` — generate routing for `fsharp`, `f#`, `fs`
- `editor/CMakeLists.txt` — `step406_test` target

**Verification run:**
- `step406_test` — PASS (12/12) new step coverage
- `step405_test` — PASS (12/12) regression coverage
- `step404_test` — PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/ast/FSharpGenerator.h` within header-size limit (`348` <= `600`)
- `editor/tests/step406_test.cpp` within test-file size guidance (`205` lines)
- `editor/src/Pipeline.h` within header-size limit (`240` <= `600`)
- Legacy oversized headers persist:
  - `editor/src/MCPServer.h` (`1679` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2623` > `600`)

### Step 407: VB.NET Parser
**Status:** PASS (12/12 tests)

Added a standalone VB.NET parser with case-insensitive keyword handling for
core block forms and declarations. Covers `Sub`/`Function`, `Class`,
`Interface`, `Module`, `Dim`, `If...Then`, and `For Each...In` recognition.

**Files created:**
- `editor/src/ast/VBNetParser.h` — VB.NET parse support:
  - `parseVBNet(...)` and `parseVBNetWithDiagnostics(...)`
  - `Sub` and `Function` signature extraction
  - `Class` and `Interface` declaration extraction
  - `Module` mapping to `NamespaceDeclaration`
  - `Dim ... As ...` variable extraction
  - `If ... Then` marker to `IfStatement`
  - `For Each ... In ...` marker to `ForLoop` with iterator extraction
  - case-insensitive token matching helpers
- `editor/tests/step407_test.cpp` — 12 tests covering:
  1. `Sub` parsing
  2. `Function` parsing
  3. class parsing
  4. interface parsing
  5. module/namespace parsing
  6. `Dim` variable parsing
  7. `If...Then` mapping
  8. `For Each...In` mapping
  9. case-insensitive keyword handling
  10. diagnostics entry point
  11. mixed-file parsing
  12. pipeline parse routing for `vbnet` / `vb` / `vb.net`

**Files modified:**
- `editor/src/ast/Parser.h` — include `ast/VBNetParser.h`
- `editor/src/Pipeline.h` — parse routing for `vbnet`, `vb`, `vb.net`
- `editor/CMakeLists.txt` — `step407_test` target

**Verification run:**
- `step407_test` — PASS (12/12) new step coverage
- `step406_test` — PASS (12/12) regression coverage
- `step405_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ast/VBNetParser.h` within header-size limit (`184` <= `600`)
- `editor/tests/step407_test.cpp` within test-file size guidance (`205` lines)
- Legacy oversized headers persist:
  - `editor/src/MCPServer.h` (`1679` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2623` > `600`)

### Step 408: VB.NET Generator
**Status:** PASS (12/12 tests)

Added a dedicated VB.NET generator and pipeline generation routing aliases.
Outputs VB-style `Sub`/`Function` blocks, `Dim` declarations, `If...Then...End If`,
`For Each...Next`, and .NET type-name mappings.

**Files created:**
- `editor/src/ast/VBNetGenerator.h` — VB.NET generation support:
  - module/function/class/interface/module-namespace generation
  - `Sub` vs `Function ... As Type` emission
  - `Dim name As Type = value` variable emission
  - `If ... Then ... End If` and `For Each ... In ... Next`
  - .NET type mapping (`Integer`, `Double`, `String`, `Boolean`, `Object`)
  - Semanno output with VB comment prefix (`' `)
- `editor/tests/step408_test.cpp` — 12 tests covering:
  1. Sub generation
  2. Function generation with return type
  3. class generation
  4. interface generation
  5. module/namespace generation
  6. Dim variable generation
  7. If generation
  8. For Each generation
  9. type mapping
  10. comment prefix
  11. Semanno annotation output
  12. pipeline generate routing for `vbnet` / `vb` / `vb.net`

**Files modified:**
- `editor/src/ast/Generator.h` — include `VBNetGenerator.h`
- `editor/src/Pipeline.h` — generate routing for `vbnet`, `vb`, `vb.net`
- `editor/CMakeLists.txt` — `step408_test` target

**Verification run:**
- `step408_test` — PASS (12/12) new step coverage
- `step407_test` — PASS (12/12) regression coverage
- `step406_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ast/VBNetGenerator.h` within header-size limit (`261` <= `600`)
- `editor/tests/step408_test.cpp` within test-file size guidance (`186` lines)
- `editor/src/Pipeline.h` within header-size limit (`247` <= `600`)
- Legacy oversized headers persist:
  - `editor/src/MCPServer.h` (`1679` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2623` > `600`)

### Step 409: .NET Integration
**Status:** PASS (8/8 tests)

Added phase-level .NET integration validation across C#, F#, and VB.NET parse
and generation routes, including cross-language projection checks and shared
Semanno annotation behavior across all 3 .NET-targeted outputs.

**Files created:**
- `editor/tests/step409_test.cpp` — 8 integration tests covering:
  1. C# -> F# generation path
  2. F# -> C# generation path
  3. C# -> VB.NET generation path
  4. VB.NET -> F# generation path
  5. Python -> F# generation path
  6. parse alias routing (`f#`, `fs`, `vb`, `vb.net`)
  7. shared type-annotation Semanno emission across C#/F#/VB targets
  8. `Pipeline.run()` success for both new languages as source/target

**Files modified:**
- `editor/CMakeLists.txt` — `step409_test` target

**Verification run:**
- `step409_test` — PASS (8/8) new integration coverage
- `step408_test` — PASS (12/12) regression coverage
- `step407_test` — PASS (12/12) regression coverage

**Phase 17a completion snapshot (Steps 405-409):**
- Step 405: F# parser (12 tests)
- Step 406: F# generator (12 tests)
- Step 407: VB.NET parser (12 tests)
- Step 408: VB.NET generator (12 tests)
- Step 409: .NET integration (8 tests)

**Architecture gate check (end of Phase 17a):**
- `editor/tests/step409_test.cpp` within test-file size guidance (`153` lines)
- `editor/src/ast/VBNetGenerator.h` within header-size limit (`261` <= `600`)
- `editor/src/ast/VBNetParser.h` within header-size limit (`184` <= `600`)
- Legacy oversized headers persist:
  - `editor/src/MCPServer.h` (`1679` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2623` > `600`)

## Phase 17b: SQL Dialect Support

### Step 410: SQL AST Nodes
**Status:** PASS (12/12 tests)

Added first-class SQL AST node types and wired JSON serialization/deserialization
and compact-node naming support so SQL structures can participate in existing
pipeline tooling.

**Files created:**
- `editor/src/ast/SqlNodes.h` — SQL node definitions:
  - `TableDeclaration`
  - `ColumnDefinition`
  - `SelectQuery`
  - `InsertStatement`
  - `UpdateStatement`
  - `DeleteStatement`
  - `JoinClause`
  - `WhereClause`
  - `IndexDefinition`
- `editor/tests/step410_test.cpp` — 12 tests covering:
  1. table construction
  2. column construction
  3. select query + clause children
  4. insert statement construction
  5. update statement construction
  6. delete statement construction
  7. index construction
  8. table+columns+index JSON roundtrip
  9. select query JSON roundtrip
  10. compact AST naming for SQL nodes
  11. `createNode` SQL concept dispatch
  12. full query graph roundtrip

**Files modified:**
- `editor/src/ast/Serialization.h` — SQL node property serialization,
  `createNode` mapping, and deserialization
- `editor/src/CompactAST.h` — SQL node name mapping in `getNodeName`
- `editor/CMakeLists.txt` — `step410_test` target

**Verification run:**
- `step410_test` — PASS (12/12) new step coverage
- `step409_test` — PASS (8/8) regression coverage
- `step408_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ast/SqlNodes.h` within header-size limit (`112` <= `600`)
- `editor/tests/step410_test.cpp` within test-file size guidance (`202` lines)
- `editor/src/CompactAST.h` at header-size limit (`600` <= `600`)
- Legacy oversized headers persist:
  - `editor/src/ast/Serialization.h` (`1427` > `600`)
  - `editor/src/MCPServer.h` (`1679` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2623` > `600`)

### Step 411: PostgreSQL Parser
**Status:** PASS (12/12 tests)

Added a standalone PostgreSQL parser with SQL-statement extraction and mapping
to the new SQL AST nodes. Covers DDL/DML basics plus PostgreSQL-specific type
and procedural patterns.

**Files created:**
- `editor/src/ast/PostgreSQLParser.h` — PostgreSQL parse support:
  - `parsePostgreSQL(...)` and `parsePostgreSQLWithDiagnostics(...)`
  - statement splitting with `$$ ... $$` block awareness
  - `CREATE TABLE` -> `TableDeclaration` + `ColumnDefinition`
  - `SELECT` -> `SelectQuery` with `from`/`joins`/`where` and `distinct`
  - `INSERT`/`UPDATE`/`DELETE` mappings
  - `CREATE INDEX`/`CREATE UNIQUE INDEX` -> `IndexDefinition`
  - `DO $$` and `CREATE FUNCTION` markers -> `Function` nodes
- `editor/tests/step411_test.cpp` — 12 tests covering:
  1. CREATE TABLE parse
  2. SELECT parse
  3. INSERT parse
  4. UPDATE parse
  5. DELETE parse
  6. JOIN + WHERE parse
  7. PostgreSQL types (`SERIAL`, `JSONB`, `TEXT[]`)
  8. schema-qualified table names
  9. CREATE UNIQUE INDEX parse
  10. DO block + PL/pgSQL function marker parse
  11. diagnostics entry point
  12. pipeline parse routing aliases (`postgresql`, `postgres`)

**Files modified:**
- `editor/src/ast/Parser.h` — include `ast/PostgreSQLParser.h`
- `editor/src/Pipeline.h` — parse routing for `postgresql`, `postgres`
- `editor/CMakeLists.txt` — `step411_test` target

**Verification run:**
- `step411_test` — PASS (12/12) new step coverage
- `step410_test` — PASS (12/12) regression coverage
- `step409_test` — PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/ast/PostgreSQLParser.h` within header-size limit (`288` <= `600`)
- `editor/tests/step411_test.cpp` within test-file size guidance (`185` lines)
- `editor/src/Pipeline.h` within header-size limit (`251` <= `600`)
- Legacy oversized headers persist:
  - `editor/src/ast/Serialization.h` (`1427` > `600`)
  - `editor/src/MCPServer.h` (`1679` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2623` > `600`)

### Step 412: PostgreSQL Generator
**Status:** PASS (12/12 tests)

Added a dedicated PostgreSQL generator and end-to-end generation coverage over
the SQL AST nodes introduced in Step 410 and parsed in Step 411.

**Files created:**
- `editor/src/ast/PostgreSQLGenerator.h` — PostgreSQL code generation support:
  - SQL DDL/DML emitters for:
    - `TableDeclaration`
    - `ColumnDefinition`
    - `SelectQuery`
    - `InsertStatement`
    - `UpdateStatement`
    - `DeleteStatement`
    - `JoinClause`
    - `WhereClause`
    - `IndexDefinition`
  - module/function support for SQL-oriented output
  - annotation output via Semanno mixin with PostgreSQL comment prefix (`-- `)
- `editor/tests/step412_test.cpp` — 12 tests covering:
  1. CREATE TABLE generation with columns
  2. SELECT DISTINCT + JOIN + WHERE generation
  3. INSERT generation with columns and values
  4. UPDATE generation with SET and WHERE
  5. DELETE generation
  6. UNIQUE INDEX generation
  7. module-level SQL statement emission
  8. WHERE clause passthrough behavior
  9. PostgreSQL comment prefix + semanno annotation output
  10. pipeline generate routing aliases (`postgresql`, `postgres`)
  11. parser-to-generator flow for CREATE TABLE
  12. edge case: empty SELECT defaults to `SELECT *;`

**Files modified:**
- `editor/src/ast/ProjectionGenerator.h` — SQL visitor hooks and dispatch branches
- `editor/src/ast/Generator.h` — include `PostgreSQLGenerator.h`
- `editor/src/Pipeline.h` — generate routing for `postgresql`, `postgres`
- `editor/CMakeLists.txt` — `step412_test` target

**Verification run:**
- `step412_test` — PASS (12/12) new step coverage
- `step411_test` — PASS (12/12) regression coverage
- `step410_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ast/PostgreSQLGenerator.h` within header-size limit (`412` <= `600`)
- `editor/tests/step412_test.cpp` within test-file size guidance (`203` lines)
- `editor/src/Pipeline.h` within header-size limit (`254` <= `600`)
- `editor/src/ast/ProjectionGenerator.h` within header-size limit (`408` <= `600`)
- Legacy oversized headers persist:
  - `editor/src/ast/Serialization.h` (`1427` > `600`)
  - `editor/src/MCPServer.h` (`1679` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2629` > `600`)

### Step 413: T-SQL Parser + Generator
**Status:** PASS (12/12 tests)

Added Microsoft SQL Server (T-SQL) dialect support with parser and generator
coverage for dialect-specific constructs (`TOP`, `IDENTITY`, `NVARCHAR`,
`DECLARE`/`SET`/`PRINT`, stored procedures).

**Files created:**
- `editor/src/ast/TSQLParser.h` — T-SQL parse support:
  - `parseTSQL(...)` and `parseTSQLWithDiagnostics(...)`
  - `CREATE TABLE`, `CREATE INDEX`, `SELECT`, `INSERT`, `UPDATE`, `DELETE`
  - `SELECT TOP n` captured via `SelectQuery` child `"top"`
  - `CREATE PROCEDURE ... AS BEGIN ... END` mapped to `Function` markers
  - `DECLARE`, `SET`, `PRINT` mapped to `Variable`, `Assignment`,
    and `ExpressionStatement(FunctionCall)`
- `editor/src/ast/TSQLGenerator.h` — T-SQL generation support:
  - `SELECT TOP n` emission
  - `CREATE PROCEDURE ... AS BEGIN ... END`
  - `DECLARE` / `SET` / `PRINT` statements
  - T-SQL-oriented primitive type mapping (`INT`, `FLOAT`, `NVARCHAR(MAX)`, `BIT`)
- `editor/tests/step413_test.cpp` — 12 tests covering:
  1. CREATE TABLE with IDENTITY and NVARCHAR parse
  2. SELECT TOP + WHERE parse
  3. CREATE PROCEDURE parse marker
  4. DECLARE parse
  5. SET parse
  6. PRINT parse
  7. SELECT TOP generation
  8. CREATE TABLE with IDENTITY generation
  9. PROCEDURE BEGIN/END generation
  10. DECLARE/SET/PRINT generation
  11. pipeline parse/generate alias routing (`tsql`, `sqlserver`, `mssql`)
  12. parser-to-generator T-SQL flow

**Files modified:**
- `editor/src/ast/Parser.h` — include `ast/TSQLParser.h`
- `editor/src/ast/Generator.h` — include `TSQLGenerator.h`
- `editor/src/Pipeline.h` — parse + generate routing for `tsql`, `sqlserver`, `mssql`
- `editor/CMakeLists.txt` — `step413_test` target

**Verification run:**
- `step413_test` — PASS (12/12) new step coverage
- `step412_test` — PASS (12/12) regression coverage
- `step411_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ast/TSQLParser.h` within header-size limit (`377` <= `600`)
- `editor/src/ast/TSQLGenerator.h` within header-size limit (`161` <= `600`)
- `editor/tests/step413_test.cpp` within test-file size guidance (`213` lines)
- `editor/src/Pipeline.h` within header-size limit (`261` <= `600`)
- Legacy oversized headers persist:
  - `editor/src/ast/Serialization.h` (`1427` > `600`)
  - `editor/src/MCPServer.h` (`1679` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2629` > `600`)

### Step 414: MySQL Parser + Generator
**Status:** PASS (12/12 tests)

Added MySQL/MariaDB dialect support with parser + generator behavior for
dialect-specific features (`AUTO_INCREMENT`, backtick identifiers,
`ENGINE=InnoDB`, `LIMIT`, `GROUP_CONCAT`).

**Files created:**
- `editor/src/ast/MySQLParser.h` — MySQL parse support:
  - `parseMySQL(...)` and `parseMySQLWithDiagnostics(...)`
  - `CREATE TABLE`, `CREATE INDEX`, `SELECT`, `INSERT`, `UPDATE`, `DELETE`
  - backtick identifier normalization
  - table engine extraction via `TableDeclaration` child `"engine"`
  - `LIMIT` extraction via `SelectQuery` child `"limit"`
  - `GROUP_CONCAT(...)` projection marker via `FunctionCall`
- `editor/src/ast/MySQLGenerator.h` — MySQL generation support:
  - backtick quoting for table/column identifiers
  - `CREATE TABLE ... ENGINE=...` emission
  - MySQL select generation with `LIMIT`
  - `GROUP_CONCAT(*)` generation path
  - MySQL alias support in DML emitters
- `editor/tests/step414_test.cpp` — 12 tests covering:
  1. CREATE TABLE parse with `AUTO_INCREMENT` and engine
  2. SELECT `LIMIT` parse
  3. `GROUP_CONCAT` projection parse marker
  4. INSERT/UPDATE/DELETE parse coverage
  5. diagnostics entrypoint
  6. CREATE TABLE generation with backticks + engine
  7. SELECT generation with LIMIT
  8. `GROUP_CONCAT` generation
  9. INSERT generation with backticks
  10. UPDATE/DELETE generation with backticks
  11. pipeline parse + generate alias routing (`mysql`, `mariadb`)
  12. parser-to-generator MySQL flow

**Files modified:**
- `editor/src/ast/Parser.h` — include `ast/MySQLParser.h`
- `editor/src/ast/Generator.h` — include `MySQLGenerator.h`
- `editor/src/Pipeline.h` — parse + generate routing for `mysql`, `mariadb`
- `editor/CMakeLists.txt` — `step414_test` target

**Verification run:**
- `step414_test` — PASS (12/12) new step coverage
- `step413_test` — PASS (12/12) regression coverage
- `step412_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ast/MySQLParser.h` within header-size limit (`331` <= `600`)
- `editor/src/ast/MySQLGenerator.h` within header-size limit (`178` <= `600`)
- `editor/tests/step414_test.cpp` within test-file size guidance (`209` lines)
- `editor/src/Pipeline.h` within header-size limit (`268` <= `600`)
- Legacy oversized headers persist:
  - `editor/src/ast/Serialization.h` (`1427` > `600`)
  - `editor/src/MCPServer.h` (`1679` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2629` > `600`)

### Step 415: SQL Annotation Mapping
**Status:** PASS (12/12 tests)

Added SQL-specific semantic annotation mapping focused on risk, complexity,
bounds checks, index contracts, and cross-stack ORM<->schema consistency.

**Files created:**
- `editor/src/SqlAnnotationMapper.h` — SQL annotation inference/mapping support:
  - module-level AST inference for:
    - `RiskAnnotation` on destructive SQL patterns (DELETE without WHERE, UPDATE without WHERE)
    - `ComplexityAnnotation` from JOIN depth + subquery hints
    - `BoundsCheckAnnotation` from `LIMIT`/`TOP` clauses
    - `ContractAnnotation` precondition for index-dependent queries
  - text-level SQL inference for `DROP TABLE` and bounds clause patterns
  - cross-stack mapping for Python ORM class fields vs SQL table columns
  - inferred-annotation application helper to attach generated annotations to AST nodes
- `editor/tests/step415_test.cpp` — 12 tests covering:
  1. high risk on DELETE without WHERE
  2. no high-risk false positive on DELETE with WHERE
  3. high risk on DROP TABLE text
  4. complexity from multi-join depth
  5. complexity from subquery nesting hint
  6. bounds check inference from LIMIT
  7. bounds check inference from TOP
  8. no bounds-check false positive without LIMIT/TOP
  9. index precondition contract inference
  10. cross-stack ORM<->SQL contract mapping
  11. cross-stack schema mismatch risk mapping
  12. inferred-annotation application to AST nodes

**Files modified:**
- `editor/CMakeLists.txt` — `step415_test` target

**Verification run:**
- `step415_test` — PASS (12/12) new step coverage
- `step414_test` — PASS (12/12) regression coverage
- `step413_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/SqlAnnotationMapper.h` within header-size limit (`246` <= `600`)
- `editor/tests/step415_test.cpp` within test-file size guidance (`230` lines)
- Legacy oversized headers persist:
  - `editor/src/ast/Serialization.h` (`1427` > `600`)
  - `editor/src/MCPServer.h` (`1679` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2629` > `600`)

### Step 416: Phase 17b Integration + Sprint Summary
**Status:** PASS (8/8 tests)

Added Phase 17b integration validation across the 3 SQL dialect paths, the SQL
annotation mapper, and cross-stack class-to-SQL flow checkpoints.

**Files created:**
- `editor/tests/step416_test.cpp` — 8 integration tests covering:
  1. SQL dialect parse routes (`postgresql`, `tsql`/`sqlserver`, `mysql`/`mariadb`)
  2. SQL dialect generate route behavior differences
  3. high-risk SQL inference (`DELETE` without `WHERE`)
  4. SQL complexity inference from JOIN depth
  5. bounds-check inference from `LIMIT`/`TOP`
  6. cross-stack flow checkpoint (`Python class` parse + `C# class` generation + SQL table DDL)
  7. ORM-schema contract mapping inference
  8. Sprint 17 totals verification checkpoint (F#, VB.NET, and 3 SQL dialects available)

**Files modified:**
- `editor/CMakeLists.txt` — `step416_test` target

**Verification run:**
- `step416_test` — PASS (8/8) new step coverage
- `step415_test` — PASS (12/12) regression coverage
- `step414_test` — PASS (12/12) regression coverage

**Phase 17b completion snapshot (Steps 410-416):**
- Step 410: SQL AST nodes + serialization
- Step 411: PostgreSQL parser
- Step 412: PostgreSQL generator
- Step 413: T-SQL parser + generator
- Step 414: MySQL parser + generator
- Step 415: SQL annotation mapping
- Step 416: phase integration + summary

**Sprint 17 completion snapshot (Steps 405-416):**
- Phase 17a complete (Steps 405-409, 56 tests)
- Phase 17b complete (Steps 410-416, 80 tests)
- Sprint 17 total: 12 steps, 136 tests

**Architecture gate check (end of Sprint 17):**
- `editor/tests/step416_test.cpp` within test-file size guidance (`181` lines)
- `editor/src/SqlAnnotationMapper.h` within header-size limit (`246` <= `600`)
- `editor/src/ast/PostgreSQLGenerator.h` within header-size limit (`412` <= `600`)
- `editor/src/ast/TSQLParser.h` within header-size limit (`377` <= `600`)
- `editor/src/ast/MySQLParser.h` within header-size limit (`331` <= `600`)
- `editor/src/Pipeline.h` within header-size limit (`268` <= `600`)
- Legacy oversized headers persist:
  - `editor/src/ast/Serialization.h` (`1427` > `600`)
  - `editor/src/MCPServer.h` (`1679` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2629` > `600`)

# Sprint 18 Progress — Claude Code Plugin + MCP Workflow Tools

## Phase 18a: Claude Code Integration

### Step 417: MCP Server Configuration + Discovery
**Status:** PASS (12/12 tests)

Added MCP server discovery/configuration utilities for plugin-style client setup.
This provides a concrete path toward `.mcp.json` generation and manifest
inspection without adding a custom plugin binary.

**Files created:**
- `editor/src/MCPServerConfig.h` — MCP config/discovery support:
  - binary path resolution (`whetstone_mcp`) from explicit candidates, common
    build locations, and PATH fallback
  - workspace-root detection (walk-up for `.git`, `CMakeLists.txt`, `progress.md`)
  - primary-language detection from workspace extension frequency
  - `.mcp.json` config JSON construction and file writing
  - server manifest generation from `MCPServer::getTools()` with category grouping
  - unified discovery snapshot with binary path, workspace root, language,
    tool count, and per-category counts
- `editor/tests/step417_test.cpp` — 12 tests covering:
  1. explicit candidate binary path resolution
  2. default binary path resolution
  3. workspace root detection
  4. primary-language detection (C++ majority)
  5. primary-language detection (Python majority)
  6. `.mcp.json` structure generation
  7. workspace/language arg generation
  8. `.mcp.json` write-out
  9. manifest tool-count threshold (68+)
  10. required category presence
  11. tool-name categorization mapping
  12. end-to-end discovery snapshot

**Files modified:**
- `editor/CMakeLists.txt` — `step417_test` target

**Verification run:**
- `step417_test` — PASS (12/12) new step coverage
- `step416_test` — PASS (8/8) regression coverage
- `step415_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/MCPServerConfig.h` within header-size limit (`246` <= `600`)
- `editor/tests/step417_test.cpp` within test-file size guidance (`159` lines)
- Legacy oversized headers persist:
  - `editor/src/ast/Serialization.h` (`1427` > `600`)
  - `editor/src/MCPServer.h` (`1679` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2629` > `600`)

### Step 418: Workflow Prompt Templates
**Status:** PASS (12/12 tests)

Added workflow-specific MCP prompt templates aligned to Sprint 18a's named
flows (`architect_project`, `modernize_module`, `port_to_language`,
`add_feature`, `review_pending`) with explicit tool references and rendering.

**Files created:**
- `editor/src/MCPWorkflowPrompts.h` — workflow prompt-template support:
  - canonical template catalog (5 templates)
  - parameterized prompt rendering (`{{...}}` substitution)
  - template validation (protocol section + required tool mentions)
  - JSON manifest export for client/tooling discovery
  - prompt-file writer for emitting portable `.prompt` files
- `editor/tests/step418_test.cpp` — 12 tests covering:
  1. required template-name presence
  2. `architect_project` tool references
  3. `modernize_module` tool references
  4. `port_to_language` source/target substitution
  5. `add_feature` progress workflow references
  6. `review_pending` review queue/context/approve/reject references
  7. parameter substitution behavior
  8. unresolved placeholder behavior when param omitted
  9. validation failure on empty tool list
  10. manifest structure and template count
  11. template-file write-out
  12. validation success for all shipped templates

**Files modified:**
- `editor/CMakeLists.txt` — `step418_test` target

**Verification run:**
- `step418_test` — PASS (12/12) new step coverage
- `step417_test` — PASS (12/12) regression coverage
- `step416_test` — PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/MCPWorkflowPrompts.h` within header-size limit (`159` <= `600`)
- `editor/tests/step418_test.cpp` within test-file size guidance (`146` lines)
- Legacy oversized headers persist:
  - `editor/src/ast/Serialization.h` (`1427` > `600`)
  - `editor/src/MCPServer.h` (`1679` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2629` > `600`)

### Step 419: Agent Workflow Loop
**Status:** PASS (12/12 tests)

Added a concrete MCP-driven agent loop wrapper that executes the Phase 18a
sequence across real MCP tool calls and handles external-result submission for
blocker items.

**Files created:**
- `editor/src/MCPAgentWorkflowLoop.h` — workflow-loop support:
  - required-tool gate for:
    - `whetstone_infer_annotations`
    - `whetstone_create_skeleton`
    - `whetstone_create_workflow`
    - `whetstone_orchestrate_run_deterministic`
    - `whetstone_get_blockers`
    - `whetstone_submit_result`
    - `whetstone_get_progress`
  - MCP `tools/call` execution wrapper with JSON result parsing
  - blocker parsing support (`itemId` and `id` aliases)
  - orchestrated run sequence with configurable project/skeleton metadata
  - loop report with call trace, blocker count, and final progress snapshot
- `editor/tests/step419_test.cpp` — 12 tests covering:
  1. baseline loop order without blockers
  2. single-blocker submit flow
  3. multi-blocker submit flow
  4. config propagation to skeleton/workflow creation calls
  5. blocker parsing (`itemId`)
  6. blocker parsing (`id` alias)
  7. invalid blocker filtering
  8. report call-order tracking
  9. agent handler payload delivery
  10. submit payload shape (code + confidence)
  11. final progress propagation into report
  12. required MCP tool presence check

**Files modified:**
- `editor/CMakeLists.txt` — `step419_test` target

**Verification run:**
- `step419_test` — PASS (12/12) new step coverage
- `step418_test` — PASS (12/12) regression coverage
- `step417_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/MCPAgentWorkflowLoop.h` within header-size limit (`137` <= `600`)
- `editor/tests/step419_test.cpp` within test-file size guidance (`299` lines)
- Legacy oversized headers persist:
  - `editor/src/ast/Serialization.h` (`1427` > `600`)
  - `editor/src/MCPServer.h` (`1679` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2629` > `600`)

### Step 420: Human Review Interface via MCP
**Status:** PASS (12/12 tests)

Added human-review MCP capabilities so review-required items can be listed,
inspected with readable context, and explicitly approved/rejected through the
headless JSON-RPC path.

**Files created:**
- `editor/tests/step420_test.cpp` — 12 tests covering:
  1. MCP review tools are registered
  2. tool-to-RPC mapping for `whetstone_get_review_queue`
  3. review queue returns review items
  4. review queue empty behavior
  5. review context includes human-readable summary
  6. review context missing-item error
  7. approve path marks item complete
  8. approve wrong-status error
  9. reject requires feedback
  10. reject moves item back to ready
  11. reject wrong-status error
  12. MCP end-to-end queue+approve flow

**Files modified:**
- `editor/src/AgentPermissionPolicy.h` — added review RPC permissions:
  - read: `getReviewQueue`, `getReviewContext`
  - mutate: `approveReviewItem`, `rejectReviewItem`
- `editor/src/MCPServer.h` — added review MCP tools and handlers:
  - `whetstone_get_review_queue` -> `getReviewQueue`
  - `whetstone_get_review_context` -> `getReviewContext`
  - `whetstone_approve_item` -> `approveReviewItem`
  - `whetstone_reject_item` -> `rejectReviewItem`
- `editor/src/HeadlessAgentRPCHandler.h` — implemented review RPC methods:
  - `getReviewQueue`
  - `getReviewContext` (includes `humanSummary`)
  - `approveReviewItem`
  - `rejectReviewItem`
- `editor/CMakeLists.txt` — `step420_test` target

**Verification run:**
- `step420_test` — PASS (12/12) new step coverage
- `step419_test` — PASS (12/12) regression coverage
- `step418_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/AgentPermissionPolicy.h` within header-size limit (`128` <= `600`)
- `editor/tests/step420_test.cpp` within test-file size guidance (`204` lines)
- Legacy oversized headers persist:
  - `editor/src/ast/Serialization.h` (`1427` > `600`)
  - `editor/src/MCPServer.h` (`1735` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2768` > `600`)

### Step 421: Workspace Onboarding Flow
**Status:** PASS (12/12 tests)

Added one-call MCP workspace onboarding so a client can bootstrap workflow
state on a new repo by indexing files, selecting key source files, running
annotation inference, saving sidecars, and creating `.whetstone` metadata.

**Files created:**
- `editor/tests/step421_test.cpp` — 12 tests covering:
  1. `whetstone_onboard_workspace` tool registration
  2. index/list call sequencing
  3. `root` forwarding to `indexWorkspace`
  4. `maxFiles` processing cap
  5. ignored/non-source path filtering
  6. language-count aggregation
  7. inference + sidecar aggregate counters
  8. open-file failure warning/continue behavior
  9. infer failure warning/continue behavior
  10. index failure hard-stop behavior
  11. `.whetstone/README.md` bootstrap + workflow suggestion payload
  12. headless end-to-end onboarding against a temp workspace

**Files modified:**
- `editor/src/MCPServer.h` — added Step 421 onboarding support:
  - `whetstone_onboard_workspace` tool registration
  - composite onboarding execution (`runWorkspaceOnboarding`) with:
    - `indexWorkspace`
    - `workspaceList`
    - candidate source-file selection + language detection
    - `openFile` + `inferAnnotations` + `saveAnnotatedAST` loop
    - `.whetstone/README.md` bootstrap via `fileCreate`
    - warning collection and next-tool workflow suggestion
- `editor/CMakeLists.txt` — `step421_test` target

**Verification run:**
- `step421_test` — PASS (12/12) new step coverage
- `step420_test` — PASS (12/12) regression coverage
- `step419_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/tests/step421_test.cpp` within test-file size guidance (`355` lines)
- Legacy oversized headers persist:
  - `editor/src/ast/Serialization.h` (`1427` > `600`)
  - `editor/src/MCPServer.h` (`1940` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2768` > `600`)

### Step 422: Phase 18a Integration
**Status:** PASS (8/8 tests)

Added Sprint 18a integration coverage validating the end-to-end MCP protocol
path: config readiness, onboarding, skeleton/workflow setup, deterministic
orchestration handoff, external-result submission, human review, and completion.

**Files created:**
- `editor/tests/step422_test.cpp` — 8 integration tests covering:
  1. MCP config structure for Phase 18a
  2. 75+ tool inventory + required Phase 18a tool presence
  3. workflow prompt-template availability (`MCPWorkflowPrompts`)
  4. onboard -> skeleton -> workflow tool sequence (mocked backend)
  5. orchestrate -> blocker -> submit -> review -> progress flow (mocked)
  6. full Phase 18a protocol end-to-end (mocked state machine)
  7. real headless review completion via MCP review tools
  8. real headless onboarding smoke with `.whetstone` bootstrap

**Files modified:**
- `editor/CMakeLists.txt` — `step422_test` target

**Verification run:**
- `step422_test` — PASS (8/8) new phase-integration coverage
- `step421_test` — PASS (12/12) regression coverage
- `step420_test` — PASS (12/12) regression coverage

**Phase 18a completion snapshot (Steps 417-422):**
- Step 417: MCP configuration/discovery
- Step 418: workflow prompt templates
- Step 419: agent workflow loop
- Step 420: human review MCP interface
- Step 421: workspace onboarding tool
- Step 422: end-to-end integration coverage

**Architecture gate check (end of Phase 18a):**
- `editor/tests/step422_test.cpp` within integration-test size guidance (`316` lines)
- `editor/tests/step421_test.cpp` within test-file size guidance (`355` lines)
- Legacy oversized headers persist:
  - `editor/src/ast/Serialization.h` (`1427` > `600`)
  - `editor/src/MCPServer.h` (`1940` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2768` > `600`)

## Phase 18b: Multi-Model Dispatch

### Step 423: Model Profile Registry
**Status:** PASS (12/12 tests)

Added a model-profile registry for mapping workflow worker classes (`slm`, `llm`,
deterministic/template) to concrete model profiles with context-window and cost
metadata, including `.whetstone/config.json`-style override support.

**Files created:**
- `editor/src/ModelProfileRegistry.h` — model profile support:
  - `ModelProfile` schema (`name`, `contextWindow`, cost rates, speed class, capabilities)
  - default profile set:
    - `claude-opus`
    - `claude-sonnet`
    - `claude-haiku`
    - `local-slm`
  - default worker mappings:
    - `llm` -> `claude-sonnet`
    - `slm` -> `claude-haiku`
    - `deterministic`/`template` -> `local-slm`
  - profile registration/listing/lookup
  - worker mapping override API
  - JSON/file override loading for profiles and worker mappings
  - manifest export (`toJson`)
- `editor/tests/step423_test.cpp` — 12 tests covering:
  1. default profile presence
  2. default `llm` mapping
  3. default `slm` mapping
  4. worker-to-profile resolution
  5. custom profile registration
  6. mapping validation for missing profiles
  7. mapping override behavior
  8. profile override via JSON payload
  9. worker-mapping override via JSON payload
  10. file-based override loading
  11. invalid JSON error path
  12. registry JSON export shape

**Files modified:**
- `editor/CMakeLists.txt` — `step423_test` target

**Verification run:**
- `step423_test` — PASS (12/12) new step coverage
- `step422_test` — PASS (8/8) regression coverage
- `step421_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ModelProfileRegistry.h` within header-size limit (`172` <= `600`)
- `editor/tests/step423_test.cpp` within test-file size guidance (`169` lines)
- Legacy oversized headers persist:
  - `editor/src/ast/Serialization.h` (`1427` > `600`)
  - `editor/src/MCPServer.h` (`1940` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2768` > `600`)

### Step 424: Context Window Optimization
**Status:** PASS (12/12 tests)

Added context-window-aware bundle optimization so routing profiles can adapt
prompt/context scope to the selected model budget (`local`, `file`, `project`)
with deterministic truncation behavior.

**Files created:**
- `editor/src/ContextWindowOptimizer.h` — context optimization support:
  - token estimation heuristic
  - usable-input budget derivation from model context window
  - scope selection:
    - small window -> `local`
    - medium window -> `file`
    - large window -> `project`
  - worker-aware optimization via `ModelProfileRegistry` mapping
  - truncation marker + budget clamp behavior
- `editor/tests/step424_test.cpp` — 12 tests covering:
  1. token estimation baseline
  2. empty-input token estimate edge case
  3. budget floor behavior
  4. small-model local scope
  5. medium-model file scope
  6. large-model project scope
  7. instruction-prefix preservation
  8. truncation path under hard budget pressure
  9. token estimate budget adherence
  10. worker-mapping optimization path
  11. unknown-worker fallback behavior
  12. project-scope composition includes file/local sections

**Files modified:**
- `editor/CMakeLists.txt` — `step424_test` target

**Verification run:**
- `step424_test` — PASS (12/12) new step coverage
- `step423_test` — PASS (12/12) regression coverage
- `step422_test` — PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/ContextWindowOptimizer.h` within header-size limit (`115` <= `600`)
- `editor/tests/step424_test.cpp` within test-file size guidance (`156` lines)
- Legacy oversized headers persist:
  - `editor/src/ast/Serialization.h` (`1427` > `600`)
  - `editor/src/MCPServer.h` (`1940` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2768` > `600`)

### Step 425: Batch Submission for Agent Tasks
**Status:** PASS (12/12 tests)

Added batching utilities for external agent tasks to reduce duplicated context
tokens by grouping requests with shared worker/language/file context.

**Files created:**
- `editor/src/BatchTaskSubmitter.h` — task batching support:
  - `AgentTaskRequest` / `AgentTaskBatch` / `BatchPlan` types
  - deterministic grouping key (`workerType|language|bufferId`)
  - stable item ordering within batches
  - max batch size splitting
  - token-cost estimation for individual vs batched plans
  - estimated savings percentage reporting
- `editor/tests/step425_test.cpp` — 12 tests covering:
  1. grouping by worker/language/buffer
  2. worker-type separation
  3. language separation
  4. max batch size enforcement
  5. shared-context carryover in batch payload
  6. batch JSON task payload shape
  7. deterministic in-batch ordering
  8. empty-input edge case
  9. max-size floor behavior
  10. batched token estimate <= individual
  11. positive savings for shared-context groups
  12. multi-group savings computation stability

**Files modified:**
- `editor/CMakeLists.txt` — `step425_test` target

**Verification run:**
- `step425_test` — PASS (12/12) new step coverage
- `step424_test` — PASS (12/12) regression coverage
- `step423_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/BatchTaskSubmitter.h` within header-size limit (`123` <= `600`)
- `editor/tests/step425_test.cpp` within test-file size guidance (`194` lines)
- Legacy oversized headers persist:
  - `editor/src/ast/Serialization.h` (`1427` > `600`)
  - `editor/src/MCPServer.h` (`1940` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2768` > `600`)

### Step 426: Cost Tracking + Reporting
**Status:** PASS (12/12 tests)

Added workflow cost-accounting primitives to track estimated vs actual token
usage and compute cost rollups by worker/profile for model-dispatch reporting.

**Files created:**
- `editor/src/WorkflowCostTracker.h` — cost tracking support:
  - per-item `CostRecord` (estimated + actual token usage)
  - model-profile resolution through `ModelProfileRegistry`
  - estimate/actual recording API
  - aggregated `CostSummary` (token totals, cost totals, variance)
  - cost bucket rollups by worker and profile
  - JSON report export
- `editor/tests/step426_test.cpp` — 12 tests covering:
  1. estimate record creation
  2. actual record update path
  3. actual-only record creation
  4. worker-to-profile resolution
  5. explicit profile override
  6. summary token totals
  7. non-negative cost totals
  8. cost-by-worker buckets
  9. cost-by-profile buckets
  10. positive variance when actual > estimate
  11. report JSON core fields
  12. no-registry zero-cost edge case

**Files modified:**
- `editor/CMakeLists.txt` — `step426_test` target

**Verification run:**
- `step426_test` — PASS (12/12) new step coverage
- `step425_test` — PASS (12/12) regression coverage
- `step424_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/WorkflowCostTracker.h` within header-size limit (`150` <= `600`)
- `editor/tests/step426_test.cpp` within test-file size guidance (`174` lines)
- Legacy oversized headers persist:
  - `editor/src/ast/Serialization.h` (`1427` > `600`)
  - `editor/src/MCPServer.h` (`1940` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2768` > `600`)

### Step 427: Phase 18b Integration + Sprint Summary
**Status:** PASS (8/8 tests)

Added Phase 18b integration coverage validating that model-profile selection,
context optimization, batch planning, and cost accounting compose into a
coherent dispatch pipeline.

**Files created:**
- `editor/tests/step427_test.cpp` — 8 integration tests covering:
  1. profile -> context -> batch -> cost pipeline composition
  2. profile mapping impact on context scope selection
  3. batching token-savings behavior
  4. cost report worker/profile breakdowns
  5. budget alignment across profile sizes
  6. cost variance tracking for estimate drift
  7. deterministic multi-group batching
  8. expected default worker/profile mappings

**Files modified:**
- `editor/CMakeLists.txt` — `step427_test` target

**Verification run:**
- `step427_test` — PASS (8/8) new phase-integration coverage
- `step426_test` — PASS (12/12) regression coverage
- `step425_test` — PASS (12/12) regression coverage

**Phase 18b completion snapshot (Steps 423-427):**
- Step 423: model profile registry + configurable worker mapping
- Step 424: context-window optimization by model budget
- Step 425: batch task submission planning + savings estimates
- Step 426: token/cost tracking and reporting
- Step 427: end-to-end integration across 18b components

**Sprint 18 completion snapshot (Steps 417-427):**
- Phase 18a complete (Steps 417-422, 68 tests)
- Phase 18b complete (Steps 423-427, 56 tests)
- Sprint 18 total: 11 steps, 124 tests

**Architecture gate check (end of Sprint 18):**
- New Sprint 18b headers within header-size limit:
  - `editor/src/ModelProfileRegistry.h` (`172` <= `600`)
  - `editor/src/ContextWindowOptimizer.h` (`115` <= `600`)
  - `editor/src/BatchTaskSubmitter.h` (`123` <= `600`)
  - `editor/src/WorkflowCostTracker.h` (`150` <= `600`)
- New Sprint 18b tests within file-size guidance:
  - `editor/tests/step423_test.cpp` (`169` lines)
  - `editor/tests/step424_test.cpp` (`156` lines)
  - `editor/tests/step425_test.cpp` (`194` lines)
  - `editor/tests/step426_test.cpp` (`174` lines)
  - `editor/tests/step427_test.cpp` (`164` lines)
- Legacy oversized headers persist:
  - `editor/src/ast/Serialization.h` (`1427` > `600`)
  - `editor/src/MCPServer.h` (`1940` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2768` > `600`)

# Sprint 19 Progress — GUI Phase 2: Workflow Visualization

## Phase 19a: Task Board Panel

### Step 428: Kanban-Style Task Board
**Status:** PASS (12/12 tests)

Added a task-board model layer for workflow visualization with canonical Kanban
columns, card metadata, filtering, and manual column reassignment semantics.

**Files created:**
- `editor/src/WorkflowTaskBoard.h` — task-board support:
  - 5 canonical columns: `pending`, `ready`, `in-progress`, `review`, `complete`
  - status normalization (`assigned`/`in-progress` collapse, rejected->pending view)
  - card projection with worker icon, priority, language/file metadata, tags
  - board filters: worker type, priority, language, file substring
  - deterministic card sorting (priority then item ID)
  - manual move API (`moveItem`) for board-driven reassignment
- `editor/tests/step428_test.cpp` — 12 tests covering:
  1. board column presence
  2. status-to-column mapping
  3. assigned->in-progress normalization
  4. worker filter
  5. priority filter
  6. language filter
  7. file filter
  8. worker icon + tag payloads
  9. move item across columns
  10. invalid-column move rejection
  11. missing-item move rejection
  12. deterministic ready-column sort

**Files modified:**
- `editor/CMakeLists.txt` — `step428_test` target

**Verification run:**
- `step428_test` — PASS (12/12) new step coverage
- `step427_test` — PASS (8/8) regression coverage
- `step426_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/WorkflowTaskBoard.h` within header-size limit (`154` <= `600`)
- `editor/tests/step428_test.cpp` within test-file size guidance (`186` lines)
- Legacy oversized headers persist:
  - `editor/src/ast/Serialization.h` (`1427` > `600`)
  - `editor/src/MCPServer.h` (`1940` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2768` > `600`)

### Step 429: Task Detail View
**Status:** PASS (12/12 tests)

Added a task-detail model layer with review actions so board-selected items can
surface skeleton/result context, routing rationale, diff summary, and approval/
rejection operations in one consistent view model.

**Files created:**
- `editor/src/WorkflowTaskDetail.h` — task-detail support:
  - detail projection (`TaskDetailView`) for one work item
  - skeleton stub generation based on file language
  - generated-code and line-delta diff summary
  - routing explanation synthesis
  - annotation-tag extraction from work-item semantics
  - review actions:
    - `approve` (`review` -> `complete`)
    - `reject` (`review` -> `ready`, feedback required)
- `editor/tests/step429_test.cpp` — 12 tests covering:
  1. detail build for existing item
  2. missing-item handling
  3. skeleton/generated payload inclusion
  4. diff summary inclusion
  5. routing explanation inclusion
  6. annotation tag inclusion
  7. rejection-history inclusion
  8. approve transition behavior
  9. approve invalid-status rejection
  10. reject transition + feedback persistence
  11. reject feedback-required validation
  12. reject invalid-status rejection

**Files modified:**
- `editor/CMakeLists.txt` — `step429_test` target

**Verification run:**
- `step429_test` — PASS (12/12) new step coverage
- `step428_test` — PASS (12/12) regression coverage
- `step427_test` — PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/WorkflowTaskDetail.h` within header-size limit (`121` <= `600`)
- `editor/tests/step429_test.cpp` within test-file size guidance (`169` lines)
- Legacy oversized headers persist:
  - `editor/src/ast/Serialization.h` (`1427` > `600`)
  - `editor/src/MCPServer.h` (`1940` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2768` > `600`)

### Step 430: Dependency Graph Visualization
**Status:** PASS (12/12 tests)

Added a dependency-graph visualization adapter that projects workflow
dependencies into render-ready node/edge view data, including status-based color
coding, critical-path highlighting, hover details, and click navigation IDs.

**Files created:**
- `editor/src/WorkflowDependencyView.h` — dependency-view support:
  - node/edge view structs for DAG rendering
  - status->color mapping:
    - complete: green
    - in-progress/assigned: blue
    - pending/ready: gray
    - review/rejected: red
  - critical path integration from `DependencyGraph.h`
  - critical-edge tagging
  - hover payload synthesis and task-navigation mapping
- `editor/tests/step430_test.cpp` — 12 tests covering:
  1. graph node/edge projection counts
  2. complete color mapping
  3. in-progress color mapping
  4. pending/ready color mapping
  5. review color mapping
  6. critical-path detection on chain
  7. critical-path node tagging
  8. critical-path edge tagging
  9. hover payload content
  10. node->task navigation ID mapping
  11. missing-node navigation edge case
  12. re-render update after status change

**Files modified:**
- `editor/CMakeLists.txt` — `step430_test` target

**Verification run:**
- `step430_test` — PASS (12/12) new step coverage
- `step429_test` — PASS (12/12) regression coverage
- `step428_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/WorkflowDependencyView.h` within header-size limit (`94` <= `600`)
- `editor/tests/step430_test.cpp` within test-file size guidance (`155` lines)
- Legacy oversized headers persist:
  - `editor/src/ast/Serialization.h` (`1427` > `600`)
  - `editor/src/MCPServer.h` (`1940` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2768` > `600`)

### Step 431: Progress Dashboard
**Status:** PASS (12/12 tests)

Added a progress-dashboard model adapter that composes workflow stats,
progress/timeline metrics, blocker summaries, worker distribution, and cost
tracking into a panel-ready snapshot.

**Files created:**
- `editor/src/WorkflowProgressDashboard.h` — dashboard support:
  - completion metrics passthrough (percent, throughput, ETA)
  - throughput series derivation from timeline events
  - worker-breakdown slices with percentage normalization
  - blocker-summary projection for action surfaces
  - optional cost integration (`WorkflowCostTracker`) with remaining-cost estimate
- `editor/tests/step431_test.cpp` — 12 tests covering:
  1. completion metric passthrough
  2. throughput-series length from timeline
  3. completed-count progression in throughput series
  4. worker-slice presence
  5. worker-percent normalization
  6. blocker-summary projection
  7. populated cost fields with tracker
  8. remaining-cost estimate path
  9. zero-cost behavior without tracker
  10. empty-timeline edge case
  11. ETA passthrough
  12. zero-total workflow edge case

**Files modified:**
- `editor/CMakeLists.txt` — `step431_test` target

**Verification run:**
- `step431_test` — PASS (12/12) new step coverage
- `step430_test` — PASS (12/12) regression coverage
- `step429_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/WorkflowProgressDashboard.h` within header-size limit (`112` <= `600`)
- `editor/tests/step431_test.cpp` within test-file size guidance (`202` lines)
- Legacy oversized headers persist:
  - `editor/src/ast/Serialization.h` (`1427` > `600`)
  - `editor/src/MCPServer.h` (`1940` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2768` > `600`)

### Step 432: Phase 19a Integration
**Status:** PASS (8/8 tests)

Added Phase 19a integration coverage ensuring task board, task detail actions,
dependency view, and progress dashboard stay synchronized across workflow state
changes and review lifecycle transitions.

**Files created:**
- `editor/tests/step432_test.cpp` — 8 integration tests covering:
  1. task board state sync
  2. dependency graph update after completion
  3. progress dashboard timeline/blocker sync
  4. review approve flow updates board+detail
  5. review reject flow requeues on board
  6. dependency-node click to detail navigation
  7. board filter/detail consistency
  8. end-to-end visible flow (route -> execute -> review -> complete)

**Files modified:**
- `editor/CMakeLists.txt` — `step432_test` target

**Verification run:**
- `step432_test` — PASS (8/8) new phase-integration coverage
- `step431_test` — PASS (12/12) regression coverage
- `step430_test` — PASS (12/12) regression coverage

**Phase 19a completion snapshot (Steps 428-432):**
- Step 428: task board model (kanban columns, filters, moves)
- Step 429: task detail model (skeleton/result/diff/routing/review actions)
- Step 430: dependency graph visualization adapter
- Step 431: progress dashboard model
- Step 432: integration across board/detail/graph/dashboard

**Architecture gate check (end of Phase 19a):**
- New Phase 19a headers within header-size limit:
  - `editor/src/WorkflowTaskBoard.h` (`154` <= `600`)
  - `editor/src/WorkflowTaskDetail.h` (`121` <= `600`)
  - `editor/src/WorkflowDependencyView.h` (`94` <= `600`)
  - `editor/src/WorkflowProgressDashboard.h` (`112` <= `600`)
- New Phase 19a tests within file-size guidance:
  - `editor/tests/step428_test.cpp` (`186` lines)
  - `editor/tests/step429_test.cpp` (`169` lines)
  - `editor/tests/step430_test.cpp` (`155` lines)
  - `editor/tests/step431_test.cpp` (`202` lines)
  - `editor/tests/step432_test.cpp` (`182` lines)
- Legacy oversized headers persist:
  - `editor/src/ast/Serialization.h` (`1427` > `600`)
  - `editor/src/MCPServer.h` (`1940` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2768` > `600`)

## Phase 19b: Code Annotation Overlay

### Step 433: Inline Annotation Badges
**Status:** PASS (12/12 tests)

Added an inline-annotation badge model for editor gutter overlays, including
type-specific icon/color mapping, stacked line badges, and expandable detail text.

**Files created:**
- `editor/src/CodeAnnotationBadges.h` — inline badge support:
  - annotation input -> badge projection
  - type->icon mapping (intent/complexity/risk/contract/review/priority)
  - type->color mapping for visual encoding
  - deterministic line/type sorting
  - line-based stacking for multiple annotations on one line
  - expandable detail formatter
- `editor/tests/step433_test.cpp` — 12 tests covering:
  1. input->badge count preservation
  2. sort stability by line/type
  3. known icon mapping
  4. known color mapping
  5. unknown-type fallback icon/color
  6. multi-badge same-line stacking
  7. stack ordering by line
  8. expanded detail payload content
  9. line/node identity preservation
  10. empty input edge case
  11. empty stack edge case
  12. review-annotation icon mapping

**Files modified:**
- `editor/CMakeLists.txt` — `step433_test` target

**Verification run:**
- `step433_test` — PASS (12/12) new step coverage
- `step432_test` — PASS (8/8) regression coverage
- `step431_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/CodeAnnotationBadges.h` within header-size limit (`88` <= `600`)
- `editor/tests/step433_test.cpp` within test-file size guidance (`141` lines)
- Legacy oversized headers persist:
  - `editor/src/ast/Serialization.h` (`1427` > `600`)
  - `editor/src/MCPServer.h` (`1940` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2768` > `600`)

### Step 434: Annotation Heatmap
**Status:** PASS (12/12 tests)

Added a heatmap projection for code annotations to support line-level intensity
visualization in workflow overlays, including category weighting and color mapping.

**Files created:**
- `editor/src/CodeAnnotationHeatmap.h` — heatmap support:
  - annotation input -> per-line heat cells
  - category weighting (complexity/risk/review/priority/intent/contract)
  - deterministic score accumulation per line
  - score-to-color mapping (cool blue -> green -> orange -> red)
  - range filtering for out-of-bounds annotations
- `editor/tests/step434_test.cpp` — 12 tests covering:
  1. one cell per line generation behavior
  2. disabled heatmap returns empty output
  3. invalid max-line boundary handling
  4. complexity weight mapping
  5. risk weight mapping
  6. review weight mapping
  7. low-score cool-blue color mapping
  8. low-mid cool-green color mapping
  9. mid-high orange color mapping
  10. high-score red color mapping
  11. multi-annotation score accumulation
  12. out-of-range annotation filtering

**Files modified:**
- `editor/CMakeLists.txt` — `step434_test` target

**Verification run:**
- `step434_test` — PASS (12/12) new step coverage
- `step433_test` — PASS (12/12) regression coverage
- `step432_test` — PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/CodeAnnotationHeatmap.h` within header-size limit (`50` <= `600`)
- `editor/tests/step434_test.cpp` within test-file size guidance (`126` lines)
- Legacy oversized headers persist:
  - `editor/src/ast/Serialization.h` (`1427` > `600`)
  - `editor/src/MCPServer.h` (`1940` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2768` > `600`)

### Step 435: Workflow Status Overlay
**Status:** PASS (12/12 tests)

Added a workflow status overlay model to project per-function/class lifecycle
state into editor views, with status icon/color mapping and task-detail routes.

**Files created:**
- `editor/src/WorkflowStatusOverlay.h` — overlay marker support:
  - work-item -> status marker projection
  - optional buffer filter for multi-file views
  - node-id -> line mapping with safe fallback
  - status -> icon mapping (outline/spinner/eye/check)
  - status -> color mapping aligned with board states
  - status -> task-board column normalization
  - click route helper to task-detail URI
  - stable sorting by file/line/item for deterministic rendering
- `editor/tests/step435_test.cpp` — 12 tests covering:
  1. marker generation count parity with work items
  2. file-scope buffer filtering behavior
  3. missing node-line fallback behavior
  4. pending icon mapping
  5. in-progress icon mapping
  6. review icon mapping
  7. complete icon mapping
  8. color mapping for review/complete
  9. board-column mapping for assigned/in-progress
  10. task-detail click route format
  11. node-name display preference
  12. deterministic sorting by file then line

**Files modified:**
- `editor/CMakeLists.txt` — `step435_test` target

**Verification run:**
- `step435_test` — PASS (12/12) new step coverage
- `step434_test` — PASS (12/12) regression coverage
- `step433_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/WorkflowStatusOverlay.h` within header-size limit (`94` <= `600`)
- `editor/tests/step435_test.cpp` within test-file size guidance (`155` lines)
- Legacy oversized headers persist:
  - `editor/src/ast/Serialization.h` (`1427` > `600`)
  - `editor/src/MCPServer.h` (`1940` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2768` > `600`)

### Step 436: Review Comparison View
**Status:** PASS (12/12 tests)

Added a review-comparison model that produces side-by-side diff rows and
integrates approve/reject actions with keyboard shortcut metadata.

**Files created:**
- `editor/src/ReviewComparisonView.h` — review diff model support:
  - workflow item -> comparison model projection
  - skeleton/generated side labels
  - line-oriented diff rows (added/removed/modified/unchanged)
  - contextual change notes (routing, feedback, annotations)
  - action availability flags keyed to review status
  - approve/reject wrappers delegated to workflow detail logic
  - keyboard shortcut constants (`Ctrl+Shift+A`, `Ctrl+Shift+R`)
- `editor/tests/step436_test.cpp` — 12 tests covering:
  1. model generation for existing item
  2. missing-item null handling
  3. modified-line diff detection
  4. review-state action enablement
  5. shortcut constant mapping
  6. shortcut propagation into model
  7. routing note inclusion
  8. rejection-feedback note inclusion
  9. approve transition to complete
  10. reject transition to ready
  11. reject-feedback requirement
  12. non-review action disablement

**Files modified:**
- `editor/CMakeLists.txt` — `step436_test` target

**Verification run:**
- `step436_test` — PASS (12/12) new step coverage
- `step435_test` — PASS (12/12) regression coverage
- `step434_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ReviewComparisonView.h` within header-size limit (`118` <= `600`)
- `editor/tests/step436_test.cpp` within test-file size guidance (`173` lines)
- Legacy oversized headers persist:
  - `editor/src/ast/Serialization.h` (`1427` > `600`)
  - `editor/src/MCPServer.h` (`1940` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2768` > `600`)

### Step 437: Phase 19b Integration + Sprint Summary
**Status:** PASS (8/8 tests)

Added Phase 19b integration coverage validating that annotation overlays,
workflow status overlays, and review comparison views work together in a single
visual workflow path.

**Files created:**
- `editor/tests/step437_test.cpp` — 8 integration tests covering:
  1. inline annotation badge visibility on annotated code
  2. heatmap emphasis for complex regions
  3. workflow status overlay projection from live state
  4. code-overlay status click -> review comparison access
  5. task-board review selection -> comparison access
  6. approve flow updates board + overlay state
  7. reject flow requeues and reflects overlay status
  8. end-to-end visualization stack operability (badges/heatmap/overlay/review/dashboard)

**Files modified:**
- `editor/CMakeLists.txt` — `step437_test` target

**Verification run:**
- `step437_test` — PASS (8/8) new integration coverage
- `step436_test` — PASS (12/12) regression coverage
- `step435_test` — PASS (12/12) regression coverage
- `step434_test` — PASS (12/12) regression coverage

**Sprint 19 completion snapshot (Steps 428-437):**
- Phase 19a (428-432): task board, task detail, dependency graph, progress dashboard, integration
- Phase 19b (433-437): annotation badges, heatmap, workflow status overlay, review comparison, integration
- Result: workflow visualization stack operational end-to-end across board, code overlays, and review flow

**Architecture gate check (end of Sprint 19):**
- New Sprint 19 headers within header-size limit:
  - `editor/src/WorkflowTaskBoard.h` (`154` <= `600`)
  - `editor/src/WorkflowTaskDetail.h` (`121` <= `600`)
  - `editor/src/WorkflowDependencyView.h` (`94` <= `600`)
  - `editor/src/WorkflowProgressDashboard.h` (`112` <= `600`)
  - `editor/src/CodeAnnotationBadges.h` (`88` <= `600`)
  - `editor/src/CodeAnnotationHeatmap.h` (`50` <= `600`)
  - `editor/src/WorkflowStatusOverlay.h` (`94` <= `600`)
  - `editor/src/ReviewComparisonView.h` (`118` <= `600`)
- New Sprint 19 tests within file-size guidance:
  - `editor/tests/step428_test.cpp` (`186` lines)
  - `editor/tests/step429_test.cpp` (`169` lines)
  - `editor/tests/step430_test.cpp` (`155` lines)
  - `editor/tests/step431_test.cpp` (`202` lines)
  - `editor/tests/step432_test.cpp` (`182` lines)
  - `editor/tests/step433_test.cpp` (`141` lines)
  - `editor/tests/step434_test.cpp` (`126` lines)
  - `editor/tests/step435_test.cpp` (`155` lines)
  - `editor/tests/step436_test.cpp` (`173` lines)
  - `editor/tests/step437_test.cpp` (`208` lines)
- Legacy oversized headers persist:
  - `editor/src/ast/Serialization.h` (`1427` > `600`)
  - `editor/src/MCPServer.h` (`1940` > `600`)
  - `editor/src/HeadlessAgentRPCHandler.h` (`2768` > `600`)

## Post-Sprint Stabilization: Build + Startup Usability

### Hotfix A: Keybinding type collision blocking editor build
**Status:** PASS (build restored)

Resolved a compile-breaking global type collision between legacy keybinding code
and the newer registry by namespacing the legacy combo type.

**Files modified:**
- `editor/src/KeybindingManager.h` — renamed legacy `KeyCombo` to `LegacyKeyCombo`
- `editor/src/main.cpp` — updated shortcut dispatch type usage
- `editor/src/ShortcutReference.h` — updated shortcut panel type usage
- `editor/tests/step54_test.cpp` — updated test references

**Verification run:**
- `cmake --build editor/build-native --target whetstone_editor` — PASS
- `step54_test` — PASS (10/10) regression coverage

### Hotfix B: startup usability (editable buffer + dismissible side panel)
**Status:** PASS (build restored, UX defaults improved)

Adjusted startup defaults so users can type immediately and hide optional side
panels cleanly.

**Files modified:**
- `editor/src/state/UIFlags.h` — added `showMemoryStrategies` flag (default `false`)
- `editor/src/panels/SidePanels.h` — made Memory Strategies panel conditional and closable
- `editor/src/panels/MenuBarPanel.h` — added View menu toggle for Memory Strategies panel
- `editor/src/main.cpp` — auto-create untitled text buffer when startup/session has no buffers; set editor focus

**Verification run:**
- `cmake --build editor/build-native --target whetstone_editor` — PASS
- `step54_test` — PASS (10/10) regression coverage
- `step437_test` — PASS (8/8) regression coverage

**Build health check cadence (to run after each sprint step cluster):**
- `cmake --build editor/build-native --target whetstone_editor -j4`
- `./editor/build-native/step54_test`
- `./editor/build-native/step437_test`

**Architecture gate check:**
- `editor/src/main.cpp` within main-file limit (`587` <= `1500`)
- `editor/src/KeybindingManager.h` within header-size limit (`276` <= `600`)
- `editor/src/panels/SidePanels.h` within header-size limit (`308` <= `600`)
- `editor/src/panels/MenuBarPanel.h` within header-size limit (`268` <= `600`)
- `editor/src/state/UIFlags.h` within header-size limit (`33` <= `600`)

### Hotfix C: completion helper dismiss behavior in text mode
**Status:** PASS (helper popup now dismisses reliably)

Fixed sticky completion helper behavior where popup reappeared immediately after
dismiss, especially noticeable in text-mode editing.

**Files modified:**
- `editor/src/EditorState.h` — added `completionDismissed` state
- `editor/src/panels/EditorPanel.h` — completion popup behavior updates:
  - preserve dismissal until next text-change trigger
  - close on `Esc` and mark dismissed
  - close on click outside popup and mark dismissed
  - reset dismissal on new edits
  - avoid forcing popup visible every frame when results exist

**Verification run:**
- `cmake --build editor/build-native --target whetstone_editor` — PASS
- `step54_test` — PASS (10/10) regression coverage
- `step437_test` — PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/EditorState.h` remains within header-size limit (`634` > `600`) — existing oversize file prior to this hotfix
- `editor/src/panels/EditorPanel.h` remains within header-size limit (`850` > `600`) — existing oversize file prior to this hotfix

### Hotfix D: completion selection bounds hardening (stability)
**Status:** PASS (build stable)

Hardened completion popup selection handling to prevent stale-selection indexing
when the candidate list shrinks between frames (e.g., while typing quickly and
confirming with Enter/Tab).

**Files modified:**
- `editor/src/panels/EditorPanel.h` — clamped `completionSelected` before navigation and before selection accept

**Verification run:**
- `cmake --build editor/build-native --target whetstone_editor` — PASS
- `step54_test` — PASS (10/10) regression coverage
- `step437_test` — PASS (8/8) regression coverage

### Hotfix E: inline completion helper default-off + toggle
**Status:** PASS (helper now opt-in)

Changed inline completion helper to default OFF and only appear when explicitly
enabled by the user.

**Files modified:**
- `editor/src/state/UIFlags.h` — added `showCompletionHelper` UI flag (default `false`)
- `editor/src/SettingsManager.h` — added persisted `showCompletionHelper` setting
- `editor/src/BufferOps.h` — synced `showCompletionHelper` with settings load/save
- `editor/src/panels/MenuBarPanel.h` — View toggle: `Inline Completion Helper`
- `editor/src/panels/SettingsPanel.h` — Settings checkbox and cleanup on disable
- `editor/src/panels/EditorPanel.h` — gated completion requests/rendering behind toggle

**Verification run:**
- `cmake --build editor/build-native --target whetstone_editor` — PASS
- `step54_test` — PASS (10/10) regression coverage
- `step437_test` — PASS (8/8) regression coverage

### Hotfix F: context-aware include completion filtering
**Status:** PASS (relevance improved for `#include` workflows)

Improved completion relevance in C/C++ preprocessor include contexts to avoid
noisy unrelated suggestions (for example `std::*` symbols when typing `#inc`).

**Files modified:**
- `editor/src/CompletionUtils.h`:
  - added completion context detection (`General`, `CppInclude`)
  - added include-context filtering for LSP items
  - added include-focused snippet suggestions (`include`, `<string>`, `<vector>`, `<iostream>`, `<memory>`)
- `editor/src/panels/EditorPanel.h`:
  - passed runtime completion context into completion builder

**Verification run:**
- `cmake --build editor/build-native --target whetstone_editor` — PASS
- `step54_test` — PASS (10/10) regression coverage
- `step437_test` — PASS (8/8) regression coverage

## Sprint 20 Kickoff (MCP Trial)

### Step 438 kickoff via `whetstone_mcp` tool path
**Status:** PASS (MCP tool chain works for basic edit operations)

Ran a real stdio MCP session against `whetstone_mcp` using framed
`Content-Length` JSON-RPC requests and verified tool execution.

**MCP calls executed:**
1. `initialize`
2. `tools/list`
3. `tools/call` -> `whetstone_workspace_list` (glob=`sprint20*`)
4. `tools/call` -> `whetstone_file_write`

**File created through MCP tool call:**
- `editor/src/LegacyIdiomDetector.h` (Step 438 scaffold)

**Verification run:**
- `cmake --build editor/build-native --target whetstone_editor` — PASS

**Architecture gate check:**
- `editor/src/LegacyIdiomDetector.h` within header-size limit (`8` <= `600`)

### Step 438: Code Age + Idiom Detection
**Status:** PASS (12/12 tests)

Implemented legacy idiom/code-age detection with per-file and per-function
reporting, plus language-version heuristics and bounded 0-10 legacy scoring.

**Files created:**
- `editor/tests/step438_test.cpp` — 12 tests covering:
  1. K&R declaration detection
  2. goto-heavy flow detection
  3. deprecated `gets` detection
  4. deprecated `sprintf` detection
  5. deprecated `strcpy` detection
  6. pointer arithmetic detection
  7. manual memory management detection
  8. C89 detection heuristic
  9. C99 detection heuristic
  10. C++11 detection heuristic
  11. score range clamp (0-10)
  12. per-function findings include function name

**Files modified:**
- `editor/src/LegacyIdiomDetector.h` — full detector model implementation:
  - idiom finding/report structs
  - file analysis entrypoint
  - per-function findings summary
  - language version inference (C/C++/Python heuristics)
  - legacy score normalization/clamping
- `editor/CMakeLists.txt` — `step438_test` target

**MCP queue trial for this step (human-in-the-loop simulation):**
- Created workflow tasks via MCP:
  - `write_detector_tests` (deterministic, critical)
  - `implement_legacy_detector` (template, high)
- Pulled queue items via `whetstone_get_ready_tasks` and confirmed task metadata/IDs.
- Routed queue with `whetstone_route_all_ready`.
- Note: workflow state in `whetstone_mcp` is session-scoped; execute/get_work_item
  must run in the same long-lived MCP session.
- Experiment outcome: MCP task flow is functional, but current execution overhead
  (session management + transport framing + state locality) is still higher than
  direct text-editing workflow for implementation speed. Keep MCP usage for trace
  capture and behavior tuning, while using direct edits as primary path for sprint velocity.

**Verification run:**
- `step438_test` — PASS (12/12) new step coverage
- `step437_test` — PASS (8/8) regression coverage
- `step54_test` — PASS (10/10) regression coverage
- `cmake --build editor/build-native --target whetstone_editor` — PASS

**Architecture gate check:**
- `editor/src/LegacyIdiomDetector.h` within header-size limit (`193` <= `600`)
- `editor/tests/step438_test.cpp` within test-file size guidance (`129` lines)

**Step 438 experiment note (follow-up):**
- Human-in-the-loop comparison confirmed that normal direct text editing is
  currently faster and easier for sprint delivery than the MCP worker path.
- Decision for now: keep direct editing as the primary implementation path,
  and use MCP/task-queue flows selectively for behavior tracing and later
  training-data/worker-tuning experiments.

### Step 439: Safety Audit via Annotations
**Status:** PASS (12/12 tests)

Implemented structured safety analysis with annotation-based findings,
CWE mapping, and per-function risk classification.

**Files created:**
- `editor/src/SafetyAuditor.h` — full safety audit model:
  - buffer overflow detection (gets, strcpy, sprintf, strcat, unchecked array access)
  - use-after-free detection (manual malloc/free without RAII, dangling dereference)
  - null dereference detection (nullable pointers without null check, unchecked malloc)
  - race condition detection (shared mutable state + threading without synchronization)
  - integer overflow detection (unchecked arithmetic on integer types)
  - CWE code mapping (CWE-120, CWE-416, CWE-476, CWE-362, CWE-190, CWE-787)
  - risk levels: 0=info, 1=low, 2=medium, 3=high, 4=critical
  - per-function safety findings with max risk aggregation
- `editor/tests/step439_test.cpp` — 12 tests covering:
  1. buffer overflow via gets() → CWE-120
  2. buffer overflow via strcpy() → CWE-120
  3. use-after-free with manual malloc/free → CWE-416
  4. dangling pointer dereference after free → @Lifetime(dangling)
  5. null dereference with unchecked nullable pointer → CWE-476
  6. unchecked malloc return value → @Nullability(unchecked-alloc)
  7. race condition: thread + global mutable state without sync → CWE-362
  8. no false positive when mutex synchronization present
  9. integer overflow without bounds check → CWE-190
  10. no false positive when INT_MAX check present
  11. per-function findings include function name and max risk
  12. comprehensive CWE mapping across all categories
- `editor/CMakeLists.txt` — `step439_test` target

**Verification run:**
- `step439_test` — PASS (12/12) new step coverage
- `step438_test` — PASS (12/12) regression coverage
- `step54_test` — PASS (10/10) regression coverage

**Architecture gate check:**
- `editor/src/SafetyAuditor.h` within header-size limit (`225` <= `600`)
- `editor/tests/step439_test.cpp` within test-file size guidance (`148` lines)

### Step 440: Modernization Suggestions
**Status:** PASS (12/12 tests)

Maps legacy patterns to modern replacements with @Modernize annotations,
effort classification (quick win / moderate / deep refactor), and cross-language
modernization support (C→Rust ownership, C→Java/Python GC).

**Files created:**
- `editor/src/ModernizationSuggester.h` — suggestion model:
  - maps 7 legacy patterns to modern replacements (malloc→smart_ptr, sprintf→format,
    strcpy→string, gets→fgets, goto→structured, pointer_arith→span, K&R→ANSI)
  - safety-derived suggestions (unprotected shared state→mutex, unchecked arith→SafeInt)
  - cross-language suggestions (C→Rust ownership, C→Java/Python GC)
  - effort enum: QuickWin, Moderate, DeepRefactor
  - @Modernize(from=..., to=..., risk=...) annotation format
- `editor/tests/step440_test.cpp` — 12 tests
- `editor/CMakeLists.txt` — `step440_test` target

### Step 441: Modernization Workflow Generation
**Status:** PASS (12/12 tests)

Converts modernization suggestions into an ordered workflow with routing
(deterministic/LLM/human), dependency tracking, and skeleton code generation.

**Files created:**
- `editor/src/ModernizationWorkflow.h` — workflow model:
  - work items with routing (QuickWin→Deterministic, Moderate→LLM, DeepRefactor→Human)
  - priority ordering (safe changes first, risky last)
  - dependency graph (deep refactors depend on quick wins)
  - skeleton generation with @Modernize inline comments
- `editor/tests/step441_test.cpp` — 12 tests
- `editor/CMakeLists.txt` — `step441_test` target

### Step 442: Modernization RPC + MCP
**Status:** PASS (12/12 tests)

JSON-RPC dispatch and MCP tool definitions for the full modernization pipeline.

**Files created:**
- `editor/src/ModernizationRPC.h` — RPC handler + MCP tool defs:
  - `analyzeLegacy` / `whetstone_analyze_legacy` — legacy idiom detection
  - `getSafetyReport` / `whetstone_get_safety_report` — structured safety audit
  - `suggestModernization` / `whetstone_suggest_modernization` — modernization suggestions
  - `createModernizationWorkflow` / `whetstone_create_modernization_workflow` — full pipeline
  - JSON serialization for all report types
  - canHandle() + dispatch() for integration with existing RPC chain
  - 4 MCP tool definitions with input schemas
- `editor/tests/step442_test.cpp` — 12 tests
- `editor/CMakeLists.txt` — `step442_test` target

### Step 443: Phase 20a Integration
**Status:** PASS (8/8 tests)

End-to-end integration: legacy C file → analyze → safety audit → suggest →
workflow → validate routing and risk ordering via both native API and JSON-RPC.

**Files created:**
- `editor/tests/step443_test.cpp` — 8 integration tests covering:
  - full pipeline legacy→workflow, safety report CWE validation,
  - risk ordering verification, C→Rust cross-language, RPC pipeline,
  - deterministic vs LLM/human routing, language version detection

### Step 444: Migration Plan Generator
**Status:** PASS (12/12 tests)

Multi-file project analysis with dependency resolution, phased migration ordering
(leaves first), and effort/risk/routing classification per migration unit.

**Files created:**
- `editor/src/MigrationPlanGenerator.h` — plan generator:
  - FileInfo input with exports/imports for dependency resolution
  - topological phase assignment (leaf modules = phase 0)
  - effort (file size), risk (API surface + deps), routing classification
  - ordered execution plan respecting dependencies
- `editor/tests/step444_test.cpp` — 12 tests
- `editor/CMakeLists.txt` — `step444_test` target

### Step 445: API Boundary Preservation
**Status:** PASS (12/12 tests)

Preserves public API surfaces during migration with function signature extraction,
type mappings, @Contract annotations, and FFI boundary generation.

**Files created:**
- `editor/src/APIBoundaryPreserver.h` — boundary preserver:
  - public function signature extraction from source
  - type mappings C→Rust/Java/Python (int→i32, char*→&str, etc.)
  - @Contract annotations (null preconditions, return postconditions)
  - FFI boundaries (#[no_mangle] for Rust, JNI for Java)
  - verifyPreservation() for pre/post migration comparison
- `editor/tests/step445_test.cpp` — 12 tests
- `editor/CMakeLists.txt` — `step445_test` target

### Step 446: Test Generation for Migration Validation
**Status:** PASS (12/12 tests)

Auto-generates equivalence, edge case, and performance regression tests for
migrated modules in the target language.

**Files created:**
- `editor/src/MigrationTestGenerator.h` — test generator:
  - equivalence tests (same inputs → same outputs) per public function
  - edge case tests from @Contract pre/post conditions (null handling, etc.)
  - performance regression tests
  - target-language-specific test body generation (Rust #[test], Java @Test, Python def test_)
  - SLM/LLM routing for generated test skeletons
- `editor/tests/step446_test.cpp` — 12 tests
- `editor/CMakeLists.txt` — `step446_test` target

### Step 447: Migration Execution Integration
**Status:** PASS (12/12 tests)

Hooks migration plans into orchestration with dependency-respecting execution,
rollback annotations, progressive (partial) migration, and FFI boundary management.

**Files created:**
- `editor/src/MigrationExecutor.h` — execution engine:
  - work items with cross-unit dependency tracking
  - ready-items computation (unblocked pending items)
  - completeUnit/failUnit state transitions
  - rollback annotations (@Rollback pointing to original files)
  - partial migration detection (mixed-language state)
  - FFI boundary auto-detection during progressive migration
- `editor/tests/step447_test.cpp` — 12 tests
- `editor/CMakeLists.txt` — `step447_test` target

### Step 448: Phase 20b Integration + Sprint 20 Summary
**Status:** PASS (8/8 tests)

Full integration: 3-file C project → Rust migration with API preservation,
test generation, partial migration with FFI boundaries, and combined
modernization + migration pipeline.

**Files created:**
- `editor/tests/step448_test.cpp` — 8 integration tests covering:
  - full 3-file C→Rust migration, API boundary preservation,
  - generated test validation, partial migration (2/3 files),
  - FFI boundary management, legacy+migration combined pipeline,
  - sprint 20 complete pipeline verification

**Sprint 20 totals:**
- **Steps:** 438-448 (11 steps)
- **Tests:** 124/124 passing
- **Headers created:** 8 (LegacyIdiomDetector, SafetyAuditor, ModernizationSuggester,
  ModernizationWorkflow, ModernizationRPC, MigrationPlanGenerator, APIBoundaryPreserver,
  MigrationTestGenerator, MigrationExecutor)
- **MCP tools added:** 4 (whetstone_analyze_legacy, whetstone_get_safety_report,
  whetstone_suggest_modernization, whetstone_create_modernization_workflow)
- **Capabilities delivered:**
  - Legacy code analysis (K&R, goto, deprecated APIs, manual memory, pointer arithmetic)
  - Safety audit with CWE mapping (CWE-120, 190, 362, 416, 476, 787)
  - Modernization suggestions with effort classification and @Modernize annotations
  - Workflow generation with deterministic/LLM/human routing
  - Multi-file migration planning with dependency-ordered phases
  - API boundary preservation with type mappings, contracts, FFI annotations
  - Test generation in target language (Rust, Java, Python)
  - Progressive migration with FFI boundary management

# Roadmap Planning — Sprints 12-25+

## Status: Planning Complete (Sprints 12-19 detailed, 20-25 in roadmap.md)

Sprint 11 revised: Phase 11e changed from "Training Data Pipeline" to "Workflow
Annotation Foundation" — routing annotation types (Subject 9), skeleton AST, and
inference-to-routing bridge replace training data export (deferred to post-25).

### Planning documents created:
- `ARCHITECT.md` — Core thesis, skeleton AST concept, architecture invariants, key metrics
- `roadmap.md` — Sprint 12-25+ high-level roadmap with post-25 training data notes
- `sprint11_plan.md` — Revised: 11e is now workflow annotation foundation
- `sprint12_plan.md` — Workflow Model + C++ Depth (22 steps, ~248 tests, 56+ tools)
- `sprint13_plan.md` — GUI Overhaul Phase 1 (19 steps, ~212 tests)
- `sprint14_plan.md` — Languages: C, WebAssembly, Common Lisp, Scheme (17 steps, ~188 tests)
- `sprint15_plan.md` — Orchestration Engine (16 steps, ~180 tests, 68+ tools)
- `sprint16_plan.md` — C++ Depth + Self-Hosting Phase 1 (11 steps, ~124 tests)
- `sprint17_plan.md` — Languages: F#, VB.NET, SQL dialects (12 steps, ~136 tests)
- `sprint18_plan.md` — Claude Code Plugin + MCP Workflow Tools (11 steps, ~124 tests, 75+ tools)
- `sprint19_plan.md` — GUI Phase 2: Workflow Visualization (10 steps, ~112 tests)
- `sprint20_plan.md` — Legacy Code Ingestion (11 steps, ~124 tests)
- `sprint21_plan.md` — Cross-Language Transpilation Engine (11 steps, ~124 tests, 83+ tools)
- `sprint22_plan.md` — Languages: Assembly + C++ remaining gaps (11 steps, ~124 tests)
- `sprint23_plan.md` — Architect Mode + Tech Stack Selection (11 steps, ~124 tests, 87+ tools)
- `sprint24_plan.md` — Security + Static Analysis (11 steps, ~124 tests, 90+ tools)
- `sprint25_plan.md` — Integration, Self-Hosting, Polish (16 steps, ~180 tests)

### Cumulative projections (Sprints 9-25):
- **Steps:** ~508 (245-508)
- **Tests:** ~5000+
- **Languages:** 19+ parsers and generators
- **MCP Tools:** 90+
- **Annotation Types:** 80+ across 10 subjects

### Key architectural decisions:
1. **Skeleton AST** = project specification before code exists (annotations on empty functions/classes)
2. **Annotations are routing signals** — @Automatability, @ContextWidth, @Ambiguity determine worker dispatch
3. **Orchestrator prepares context but never calls LLMs** — model-agnostic, cost-controlled
4. **Plugin is MCP protocol** — not a proprietary binary, any MCP client can drive workflows
5. **Training data deferred to post-25** — real workflow decisions are better training signal than synthetic pairs

### Step 449: Intent-Driven Translation
**Status:** PASS (12/12 tests)

Recovered and validated in-progress Sprint 21 work left uncommitted:
intent-aware translation that prefers target-language idioms when intent is
clear, and falls back to structural translation with review flags when intent
is absent/ambiguous.

**Files added:**
- `editor/src/IntentTranslator.h` — intent mapping model + translation engine:
  - `FunctionIntent`, `IntentMapping`, `FunctionTranslation`, `IntentTranslationResult`
  - idiomatic mapping by intent phrase and target language (Rust/Python/Java/SQL/C++)
  - ambiguity/review gating (`needsReview`, `reviewReason`)
  - confidence scoring by mapping strength and fallback mode
- `editor/tests/step449_test.cpp` — 12 tests covering:
  - intent-to-idiom mappings (sort/find/filter/map/reduce/read-file)
  - SQL-specific intent projection (`ORDER BY`, `WHERE`)
  - ambiguous/no-intent fallback and review behavior
  - mixed multi-function translation accounting
- `editor/CMakeLists.txt` — `step449_test` target

**Verification run:**
- `cmake --build editor/build-native --target step449_test` — PASS
- `./editor/build-native/step449_test` — PASS (12/12)

**Architecture gate check:**
- `editor/src/IntentTranslator.h` within header-size limit (`254` <= `600`)
- `editor/tests/step449_test.cpp` within test-file size guidance (`166` lines)

### Step 450: Algorithm-Level Equivalence
**Status:** PASS (12/12 tests)

Adds algorithm-pattern recognition over source snippets and target-language
idiomatic equivalent selection, enabling standard-library mappings beyond
literal/syntactic rewrites.

**Files added:**
- `editor/src/AlgorithmEquivalence.h` — recognizer + mapping model:
  - `AlgorithmPattern`, `AlgorithmEquivalent`, `AlgorithmAnalysisResult`
  - pattern detection for sort/search/map/filter/reduce/builder/iteration
  - cross-language target mapping (Rust/Python/Java/C++/Go)
  - standard-library usage accounting (`countStdLib`)
  - pattern catalog (`supportedPatterns`, 15 entries)
- `editor/tests/step450_test.cpp` — 12 tests covering:
  - bubble/binary/linear search recognition
  - map/filter/reduce loop recognition and idiomatic target mapping
  - builder-pattern detection
  - cross-language sort target projection validation
- `editor/CMakeLists.txt` — `step450_test` target

**Verification run:**
- `cmake --build editor/build-native --target step450_test` — PASS
- `./editor/build-native/step450_test` — PASS (12/12)
- `./editor/build-native/step449_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/AlgorithmEquivalence.h` within header-size limit (`228` <= `600`)
- `editor/tests/step450_test.cpp` within test-file size guidance (`143` lines)

### Step 451: Type System Translation
**Status:** PASS (12/12 tests)

Completes type-system mapping across language boundaries with explicit safety
classification and risk annotations for lossy projections.

**Files added:**
- `editor/src/TypeSystemTranslator.h` — type translation engine:
  - nullable mapping (`Optional<T>`/`T?`), generic collections, maps, enums
  - class-hierarchy projection (`trait + struct`, `interface + struct`, `struct + vtable`)
  - primitive/string translation across Rust/Python/Java/C/C++/Go
  - safety classification (`Preserved`, `Widened`, `Narrowed`, `Lossy`)
  - risk annotation tagging for lossy targets (`@Risk(medium|high)`)
- `editor/tests/step451_test.cpp` — 12 tests covering:
  - nullable/generic/map translations and safety categories
  - enum/class/string lossy/narrowed mappings
  - unknown-type pass-through behavior
  - mixed-batch translation with lossy-count validation
- `editor/CMakeLists.txt` — `step451_test` target

**Verification run:**
- `cmake --build editor/build-native --target step451_test` — PASS
- `./editor/build-native/step451_test` — PASS (12/12)
- `./editor/build-native/step450_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/TypeSystemTranslator.h` within header-size limit (`140` <= `600`)
- `editor/tests/step451_test.cpp` within test-file size guidance (`146` lines)

### Step 452: Concurrency Model Translation
**Status:** PASS (12/12 tests)

Adds concurrency-pattern translation with explicit runtime/safety annotations
for async/thread/channel/mutex constructs across language runtime models.

**Files added:**
- `editor/src/ConcurrencyTranslator.h` — concurrency translation engine:
  - source/target model detection (`Threads`, `AsyncAwait`, `Channels`, `EventLoop`)
  - pattern translators for `async/await`, `thread_spawn`, `channels`, `mutex`
  - runtime-aware annotations (`@Exec`, `@ThreadModel`, `@Sync`, `@Blocking`)
  - safety grading (`Safe`, `NeedsReview`, `Unsafe`) and review counts
- `editor/tests/step452_test.cpp` — 12 tests covering:
  - async mapping across Rust/Python/C targets
  - thread/channel/mutex projection to Go/Rust/C
  - source/target model detection behavior
  - plain-code no-pattern behavior and multi-pattern aggregation
- `editor/CMakeLists.txt` — `step452_test` target

**Verification run:**
- `cmake --build editor/build-native --target step452_test` — PASS
- `./editor/build-native/step452_test` — PASS (12/12)
- `./editor/build-native/step451_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ConcurrencyTranslator.h` within header-size limit (`249` <= `600`)
- `editor/tests/step452_test.cpp` within test-file size guidance (`146` lines)

### Step 453: Memory Model Translation
**Status:** PASS (12/12 tests)

Completes ownership/lifetime translation across manual, RAII, GC, ARC, and
borrow-checked models with explicit safety-delta tracking.

**Files added:**
- `editor/src/MemoryModelTranslator.h` — memory model translation engine:
  - ownership model detection (`Manual`, `RAII`, `GC`, `BorrowChecked`, `ARC`)
  - translation patterns for `@Owner(Unique)`, `@Owner(Shared_ARC)`,
    `@Owner(Manual)`, `@Lifetime(Scope)`
  - per-translation safety delta (`Gained`, `Lost`, `Neutral`)
  - migration annotations (e.g., `@Risk(high)`, `@Risk(lowered)`)
- `editor/tests/step453_test.cpp` — 12 tests covering:
  - manual-memory safety gains in Rust/GC targets
  - unique/shared ownership degradation when targeting C
  - scope-lifetime mapping (`defer`, try-with-resources, manual fallback)
  - source/target model inference and mixed-pattern safety counters
- `editor/CMakeLists.txt` — `step453_test` target

**Verification run:**
- `cmake --build editor/build-native --target step453_test` — PASS
- `./editor/build-native/step453_test` — PASS (12/12)
- `./editor/build-native/step452_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/MemoryModelTranslator.h` within header-size limit (`249` <= `600`)
- `editor/tests/step453_test.cpp` within test-file size guidance (`142` lines)

### Step 454: Phase 21a Integration
**Status:** PASS (8/8 tests)

Adds Phase 21a end-to-end integration coverage across intent, algorithm,
types, concurrency, and memory translation components.

**Files added:**
- `editor/tests/step454_test.cpp` — 8 integration tests covering:
  - intent-driven idiomatic translation (Python→Rust sort intent)
  - algorithm recognizer stdlib mapping validation
  - type-system preservation vs lossy-risk behavior
  - async runtime review requirements in concurrency translation
  - manual-memory safety gains during C→Rust migration
  - no-intent structural fallback/review behavior
  - combined pipeline signal checks across all phase components
- `editor/CMakeLists.txt` — `step454_test` target

**Verification run:**
- `cmake --build editor/build-native --target step454_test` — PASS
- `./editor/build-native/step454_test` — PASS (8/8)
- `./editor/build-native/step453_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/tests/step454_test.cpp` within test-file size guidance (`119` lines)
- No new production header introduced in this integration step

### Step 455: Behavioral Equivalence Checking
**Status:** PASS (12/12 tests)

Implements behavioral-equivalence assertion generation with contract annotation
grading from confidence scores (verified/likely/unverified+review).

**Files added:**
- `editor/src/BehavioralEquivalence.h` — equivalence checker:
  - `EquivalenceAssertion`, `BehavioralCheckResult`, `BehavioralChecker`
  - inferred input generation for sort/search/sum/generic functions
  - inferred expected-output stubs for common patterns
  - confidence scoring heuristics from target translation shape
  - contract annotation assignment:
    - `@Contract(verified)` (>=0.90)
    - `@Contract(likely)` (>=0.70)
    - `@Contract(unverified), @Review(required,human)` (<0.70)
- `editor/tests/step455_test.cpp` — 12 tests covering:
  - input inference for sort/search/sum/generic categories
  - custom input override behavior
  - confidence-tiered contract annotation mapping
  - same-language high-confidence default behavior
  - assertion metadata (`sameErrors`, `allPassing`) expectations
- `editor/CMakeLists.txt` — `step455_test` target

**Verification run:**
- `cmake --build editor/build-native --target step455_test` — PASS
- `./editor/build-native/step455_test` — PASS (12/12)
- `./editor/build-native/step454_test` — PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/BehavioralEquivalence.h` within header-size limit (`141` <= `600`)
- `editor/tests/step455_test.cpp` within test-file size guidance (`194` lines)

### Step 456: Transpilation Confidence Scoring
**Status:** PASS (12/12 tests)

Implements deterministic confidence scoring categories and review gating for
translated functions, including aggregated report statistics.

**Files added:**
- `editor/src/TranspilationConfidence.h` — confidence scorer:
  - category model (`DirectStdLib`, `AlgorithmPattern`,
    `StructuralWithIntent`, `StructuralNoIntent`, `Unknown`)
  - deterministic score mapping (0.95 / 0.85 / 0.70 / 0.50 / 0.30)
  - low-confidence review gating (`@Review(required,human)` when `< 0.60`)
  - per-function score output + batch report aggregation APIs
- `editor/tests/step456_test.cpp` — 12 tests covering:
  - category classification and score assignment
  - annotation/review behavior for high vs low confidence
  - `scoreAll` aggregation, category/review counters, average score
  - function-score lookup and default no-intent behavior
- `editor/CMakeLists.txt` — `step456_test` target

**Verification run:**
- `cmake --build editor/build-native --target step456_test` — PASS
- `./editor/build-native/step456_test` — PASS (12/12)
- `./editor/build-native/step455_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/TranspilationConfidence.h` within header-size limit (`153` <= `600`)
- `editor/tests/step456_test.cpp` within test-file size guidance (`155` lines)

### Step 457: Translation Report
**Status:** PASS (12/12 tests)

Adds per-function and project-level transpilation reporting, including mapping
shape, confidence, annotation deltas, safety delta, idiomatic/literal split,
and review-flag rollups.

**Files added:**
- `editor/src/TranslationReport.h` — report generator:
  - function-level report model with construct mapping, confidence, rationale,
    safety delta, and annotation-change tracking
  - project summary metrics: total, idiomatic/literal counts, review flags,
    and percentage rollups
  - integration with intent translation + confidence scorer
- `editor/tests/step457_test.cpp` — 12 tests covering:
  - per-function presence and retrieval behavior
  - summary counts and percentage calculations
  - annotation/rationale generation
  - safety delta behavior for `c→rust` and `rust→c`
  - intent override impact on confidence path
- `editor/CMakeLists.txt` — `step457_test` target

**Verification run:**
- `cmake --build editor/build-native --target step457_test` — PASS
- `./editor/build-native/step457_test` — PASS (12/12)
- `./editor/build-native/step456_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/TranslationReport.h` within header-size limit (`174` <= `600`)
- `editor/tests/step457_test.cpp` within test-file size guidance (`168` lines)

### Step 458: Transpilation RPC + MCP
**Status:** PASS (12/12 tests)

Exposes semantic transpilation/reporting/equivalence/confidence as JSON-RPC
handlers and MCP tool definitions.

**Files added:**
- `editor/src/TranspilationRPC.h` — RPC + MCP handler:
  - JSON handlers:
    - `transpile`
    - `getTranslationReport`
    - `verifyEquivalence`
    - `getConfidence`
  - `canHandle` + `dispatch` integration shape (JSON-RPC 2.0 envelope)
  - MCP tool definitions:
    - `whetstone_transpile`
    - `whetstone_get_translation_report`
    - `whetstone_verify_equivalence`
    - `whetstone_get_confidence`
  - JSON serialization helpers for confidence/report/equivalence payloads
- `editor/tests/step458_test.cpp` — 12 tests covering:
  - handler method coverage and dispatch behavior
  - result schema/content validation for each RPC method
  - unknown-method error envelope
  - MCP tool definition count/names/schema-required fields
  - low-confidence review and no-intent review behavior
- `editor/CMakeLists.txt` — `step458_test` target

**Verification run:**
- `cmake --build editor/build-native --target step458_test` — PASS
- `./editor/build-native/step458_test` — PASS (12/12)
- `./editor/build-native/step457_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/TranspilationRPC.h` within header-size limit (`255` <= `600`)
- `editor/tests/step458_test.cpp` within test-file size guidance (`193` lines)

### Step 459: Phase 21b Integration + Sprint 21 Summary
**Status:** PASS (8/8 tests)

Adds end-to-end integration coverage for semantic transpilation, report
generation, confidence scoring, equivalence checks, and RPC/MCP surface.

**Files added:**
- `editor/tests/step459_test.cpp` — 8 integration tests covering:
  - semantic transpile flow (Python→Rust) with confidence follow-up
  - translation report rationale/summary validation
  - low-confidence auto-review behavior
  - equivalence assertion generation
  - full JSON-RPC dispatch flow and unknown-method handling
  - MCP tool surface completeness for Phase 21b tools
- `editor/CMakeLists.txt` — `step459_test` target

**Verification run:**
- `cmake --build editor/build-native --target step459_test` — PASS
- `./editor/build-native/step459_test` — PASS (8/8)
- `./editor/build-native/step458_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/tests/step459_test.cpp` within test-file size guidance (`167` lines)
- No new production header introduced in this integration step

**Sprint 21 totals:**
- **Steps:** 449-459 (11 steps)
- **Tests:** 124/124 passing
- **Headers created:** 9
  - `IntentTranslator.h`
  - `AlgorithmEquivalence.h`
  - `TypeSystemTranslator.h`
  - `ConcurrencyTranslator.h`
  - `MemoryModelTranslator.h`
  - `BehavioralEquivalence.h`
  - `TranspilationConfidence.h`
  - `TranslationReport.h`
  - `TranspilationRPC.h`
- **MCP tools added:** 4
  - `whetstone_transpile`
  - `whetstone_get_translation_report`
  - `whetstone_verify_equivalence`
  - `whetstone_get_confidence`
- **Capabilities delivered:**
  - intent-driven idiomatic translation selection
  - algorithm pattern recognition and stdlib equivalence mapping
  - cross-language type model translation with lossiness annotations
  - concurrency model translation with runtime/review signals
  - ownership/lifetime memory model translation with safety deltas
  - behavioral equivalence assertion generation with contract grading
  - deterministic confidence scoring and review gating
  - per-function/project translation reporting
  - RPC + MCP interfaces for transpilation pipeline operations

### Step 460: Assembly AST Nodes
**Status:** PASS (12/12 tests)

Introduces core assembly AST node model and per-node JSON serialization
round-trip helpers for instructions, labels, directives, registers, and
memory operands.

**Files added:**
- `editor/src/ast/AssemblyNodes.h` — new assembly node definitions:
  - `AssemblyInstruction` (opcode, operands, structured register/memory operands)
  - `AssemblyLabel` (name, `isGlobal`)
  - `AssemblyDirective` (`.data`, `.text`, `.global`, `.section`, `.byte`, `.word`, `.align`)
  - `AssemblyRegister` (name + bit width)
  - `AssemblyMemoryOperand` (base/index/scale/offset addressing components)
  - directive enum/string conversion helpers
  - JSON serialization/deserialization helpers for all node types
- `editor/tests/step460_test.cpp` — 12 tests covering:
  - node construction and field integrity for all assembly node types
  - directive enum/string mapping
  - JSON round-trip for each node type
  - structured operand round-trip in instructions
  - unknown directive fallback behavior
- `editor/CMakeLists.txt` — `step460_test` target

**Verification run:**
- `cmake --build editor/build-native --target step460_test` — PASS
- `./editor/build-native/step460_test` — PASS (12/12)
- `./editor/build-native/step459_test` — PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/ast/AssemblyNodes.h` within header-size limit (`200` <= `600`)
- `editor/tests/step460_test.cpp` within test-file size guidance (`160` lines)

### Step 461: x86 Assembly Parser
**Status:** PASS (12/12 tests)

Implements x86 assembly parsing with Intel/AT&T syntax mode support, including
labels, directives, core instructions, register recognition, and memory
addressing extraction.

**Files added:**
- `editor/src/ast/X86AssemblyParser.h` — x86 parser:
  - syntax-mode support (`Intel`, `ATT`)
  - parses directives (`.section`, `.global`, `.data`, `.text`, etc.)
  - parses labels into function shells (`label + instruction body`)
  - parses instructions with operand splitting respecting address delimiters
  - register recognition + inferred register size (8/16/32/64)
  - Intel memory addressing parse (`[rbp-8]`, `[rax+rbx*4+8]`)
  - AT&T memory addressing parse (`-8(%rbp,%rbx,4)`)
  - warning diagnostics for unknown/unrecognized opcode lines
  - section directives represented as namespace-like nodes (`NamespaceDeclaration`)
- `editor/tests/step461_test.cpp` — 12 tests covering:
  - Intel and AT&T instruction parsing
  - global label detection
  - directive/section parsing behavior
  - register-size recognition across register families
  - Intel + AT&T memory addressing extraction
  - comment stripping behavior
  - known-opcode vs unknown-opcode diagnostics
- `editor/CMakeLists.txt` — `step461_test` target

**Verification run:**
- `cmake --build editor/build-native --target step461_test` — PASS
- `./editor/build-native/step461_test` — PASS (12/12)
- `./editor/build-native/step460_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ast/X86AssemblyParser.h` within header-size limit (`299` <= `600`)
- `editor/tests/step461_test.cpp` within test-file size guidance (`186` lines)

### Step 462: ARM Assembly Parser
**Status:** PASS (12/12 tests)

Implements ARM/AArch64 assembly parsing for core instructions, registers,
addressing modes, directives, and label/function shaping.

**Files added:**
- `editor/src/ast/ArmAssemblyParser.h` — ARM parser:
  - parses ARM/AArch64 instructions (`mov`, `add`, `sub`, `ldr`, `str`, `b`,
    `bl`, `cmp`, conditional branches, `ret`)
  - register recognition for `r0-r15`, `x0-x30`, `sp`, `lr`, `pc`
  - ARM memory addressing extraction:
    - `[r0]`
    - `[r0, #4]`
    - `[r0, r1, LSL #2]`
  - directive handling (`.global`, `.text`, `.data`, `.word`, `.align`, `.section`)
  - section-as-namespace shaping via `NamespaceDeclaration`
  - warning diagnostics for unknown opcodes
- `editor/tests/step462_test.cpp` — 12 tests covering:
  - ARM and AArch64 instruction/register parsing
  - special register handling
  - all required addressing forms
  - directive and section handling
  - branch/call opcode coverage
  - unknown-opcode warning behavior
- `editor/CMakeLists.txt` — `step462_test` target

**Verification run:**
- `cmake --build editor/build-native --target step462_test` — PASS
- `./editor/build-native/step462_test` — PASS (12/12)
- `./editor/build-native/step461_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ast/ArmAssemblyParser.h` within header-size limit (`245` <= `600`)
- `editor/tests/step462_test.cpp` within test-file size guidance (`173` lines)

### Step 463: Assembly Generator — x86 + ARM
**Status:** PASS (12/12 tests)

Adds assembly code generation for x86 and ARM from AST, plus a simple
cross-architecture instruction mapper for straightforward x86→ARM projection.

**Files added:**
- `editor/src/ast/AssemblyGenerator.h` — generator and mapper:
  - `generateX86` with Intel/AT&T syntax mode support
  - `generateArm` output path
  - `translateX86ToArmSimple` mapping for core opcodes (`mov/add/sub/cmp/jmp/call/ret`)
  - architecture comment prefixes (`x86 -> "; "`, `arm -> "@ "`)
  - directive/section emission from module and namespace-like section nodes
- `editor/tests/step463_test.cpp` — 12 tests covering:
  - x86 Intel/AT&T generation behavior
  - ARM generation behavior
  - directive/section emission
  - x86→ARM opcode mapping including control flow
  - unknown-opcode passthrough
  - comment prefix semantics
  - parse→generate→parse roundtrip sanity check for x86
- `editor/CMakeLists.txt` — `step463_test` target

**Verification run:**
- `cmake --build editor/build-native --target step463_test` — PASS
- `./editor/build-native/step463_test` — PASS (12/12)
- `./editor/build-native/step462_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ast/AssemblyGenerator.h` within header-size limit (`167` <= `600`)
- `editor/tests/step463_test.cpp` within test-file size guidance (`147` lines)

### Step 464: Assembly Annotation Mapping
**Status:** PASS (12/12 tests)

Adds assembly-aware semantic annotation mapping with complexity heuristics,
register-pressure analysis, alignment mapping, and lightweight C↔assembly
projection helpers.

**Files added:**
- `editor/src/ast/AssemblyAnnotationMapper.h` — annotation mapper:
  - baseline annotations for assembly functions:
    - `@Exec(native)`
    - `@Risk(high)`
    - `@Target(x86|arm)`
    - `@Complexity(low|medium|high)`
  - `.align` directive mapping to `@Align(...)`
  - complexity scoring from instruction count, branch density, register pressure
  - register-pressure estimation via unique register cardinality
  - cross-language helpers:
    - `lowerCToAssembly(...)`
    - `liftAssemblyToC(...)`
- `editor/tests/step464_test.cpp` — 12 tests covering:
  - required annotation emission
  - complexity tiering behavior
  - branch/register-pressure metrics
  - target architecture inference
  - C→assembly lowering and assembly→C lifting stubs
  - alignment annotation derivation
- `editor/CMakeLists.txt` — `step464_test` target

**Verification run:**
- `cmake --build editor/build-native --target step464_test` — PASS
- `./editor/build-native/step464_test` — PASS (12/12)
- `./editor/build-native/step463_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ast/AssemblyAnnotationMapper.h` within header-size limit (`128` <= `600`)
- `editor/tests/step464_test.cpp` within test-file size guidance (`177` lines)

### Step 465: Phase 22a Integration
**Status:** PASS (8/8 tests)

Adds Phase 22a integration coverage across x86/ARM parsing, simple
cross-architecture translation, annotation mapping, lift/lower helpers, and
assembly node round-trip serialization.

**Files added:**
- `editor/tests/step465_test.cpp` — 8 integration tests covering:
  - realistic x86 parse to valid AST
  - realistic ARM parse to valid AST
  - x86→ARM simple instruction mapping
  - assembly→C lifting stub generation
  - C→assembly lowering shape checks (x86 + ARM)
  - annotation usefulness checks on assembly functions
  - parse→annotate→generate end-to-end ARM flow
  - assembly node JSON round-trip
- `editor/CMakeLists.txt` — `step465_test` target

**Verification run:**
- `cmake --build editor/build-native --target step465_test` — PASS
- `./editor/build-native/step465_test` — PASS (8/8)
- `./editor/build-native/step464_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/tests/step465_test.cpp` within test-file size guidance (`150` lines)
- No new production header introduced in this integration step

**Phase 22a totals (460-465):**
- **Steps:** 6
- **Tests:** 68/68 passing
- **Headers added:** 5
  - `ast/AssemblyNodes.h`
  - `ast/X86AssemblyParser.h`
  - `ast/ArmAssemblyParser.h`
  - `ast/AssemblyGenerator.h`
  - `ast/AssemblyAnnotationMapper.h`
- **Capabilities delivered:**
  - assembly AST node model with JSON round-trip helpers
  - x86 parser (Intel + AT&T) with label/directive/addressing support
  - ARM/AArch64 parser with addressing mode support
  - x86/ARM code generation + simple x86→ARM opcode mapping
  - assembly semantic annotation mapping (`@Exec`, `@Target`, `@Risk`, `@Complexity`, `@Align`)

### Step 466: Range-Based For + Structured Bindings
**Status:** PASS (12/12 tests)

Adds focused support for C++ range-based `for` and structured binding forms,
including parse helpers, JSON round-trip, code generation, and cross-language
projection of range loops.

**Files added:**
- `editor/src/ast/CppRangeStructured.h` — feature-gap helpers:
  - `RangeForStatement` node model
  - `StructuredBinding` node model
  - parse helpers:
    - `parseRangeFor(...)`
    - `parseStructuredBinding(...)`
  - JSON round-trip helpers for both constructs
  - C++ generation helpers for both constructs
  - range-for projection helpers to Python, Rust, Java
- `editor/tests/step466_test.cpp` — 12 tests covering:
  - parsing of const-ref and simple range-for forms
  - parsing of structured bindings (2 and 3 names)
  - JSON round-trip behavior
  - C++ generation for both constructs
  - Python/Rust/Java projection checks
  - graceful parse failure on invalid inputs
- `editor/CMakeLists.txt` — `step466_test` target

**Verification run:**
- `cmake --build editor/build-native --target step466_test` — PASS
- `./editor/build-native/step466_test` — PASS (12/12)
- `./editor/build-native/step465_test` — PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/ast/CppRangeStructured.h` within header-size limit (`150` <= `600`)
- `editor/tests/step466_test.cpp` within test-file size guidance (`163` lines)

### Step 467: Exception Handling
**Status:** PASS (12/12 tests)

Adds C++ exception-handling gap coverage for try/catch clauses, throw
expressions, noexcept detection, JSON round-trip, and cross-language exception
model projections.

**Files added:**
- `editor/src/ast/CppExceptions.h` — exception helper model:
  - `TryCatchStatement`, `CatchClause`, `ThrowExpression`
  - parse helpers:
    - `parseTryCatch(...)`
    - `parseThrowExpression(...)`
    - `hasNoexcept(...)`
  - JSON round-trip helpers for try/catch and throw expression
  - projections:
    - C++ try/catch → Rust `Result`/`match`
    - C++ try/catch → Python `try/except`
    - C++ try/catch → Go `if err != nil`
- `editor/tests/step467_test.cpp` — 12 tests covering:
  - single and multi-catch parsing
  - catch-all parsing
  - throw expression parsing
  - noexcept detection
  - JSON round-trip
  - Rust/Python/Go projection checks
  - invalid parse failure behavior
- `editor/CMakeLists.txt` — `step467_test` target

**Verification run:**
- `cmake --build editor/build-native --target step467_test` — PASS
- `./editor/build-native/step467_test` — PASS (12/12)
- `./editor/build-native/step466_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ast/CppExceptions.h` within header-size limit (`178` <= `600`)
- `editor/tests/step467_test.cpp` within test-file size guidance (`148` lines)

### Step 468: Operator Overloading + Friend
**Status:** PASS (12/12 tests)

Adds focused support for C++ operator-overload declarations, including friend
operator forms, JSON round-trip, declaration generation, and projections to
named methods in Java/Python.

**Files added:**
- `editor/src/ast/CppOperators.h` — operator-overload helpers:
  - `OperatorOverload` node model (`operatorSymbol`, params, return type,
    `isFriend`, `isConst`)
  - parser for member and friend operator declarations
  - JSON round-trip helpers
  - C++ declaration generation
  - cross-language projection:
    - `<` → Java `compareTo`, Python `__lt__`
    - `==` → Java `equals`, Python `__eq__`
    - `<<` → display/string projection methods
- `editor/tests/step468_test.cpp` — 12 tests covering:
  - parsing member/friend overload forms
  - const/friend flag behavior
  - invalid declaration rejection
  - JSON round-trip and declaration generation
  - Java/Python projection behavior for key operators
- `editor/CMakeLists.txt` — `step468_test` target

**Verification run:**
- `cmake --build editor/build-native --target step468_test` — PASS
- `./editor/build-native/step468_test` — PASS (12/12)
- `./editor/build-native/step467_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ast/CppOperators.h` within header-size limit (`129` <= `600`)
- `editor/tests/step468_test.cpp` within test-file size guidance (`152` lines)

### Step 469: Initializer Lists + STL Patterns
**Status:** PASS (12/12 tests)

Adds focused support for C++ initializer-list parsing plus STL container and
iterator-loop pattern recognition with lightweight annotation outputs.

**Files added:**
- `editor/src/ast/CppInitializerStl.h` — initializer/STL helpers:
  - `InitializerListExpression` model
  - initializer-list parser
  - STL container recognition (`vector`, `map`, `set`, `string`)
  - element-type extraction from template arguments
  - iterator-loop pattern detector with `@Loop(iterator)` annotation
  - JSON round-trip helpers for initializer-list node
- `editor/tests/step469_test.cpp` — 12 tests covering:
  - initializer-list parsing and round-trip
  - STL container kind/type recognition
  - container annotation output checks
  - iterator-pattern detection (`begin/end`, `auto it`) and non-iterator negative case
- `editor/CMakeLists.txt` — `step469_test` target

**Verification run:**
- `cmake --build editor/build-native --target step469_test` — PASS
- `./editor/build-native/step469_test` — PASS (12/12)
- `./editor/build-native/step468_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ast/CppInitializerStl.h` within header-size limit (`135` <= `600`)
- `editor/tests/step469_test.cpp` within test-file size guidance (`138` lines)

### Step 470: Phase 22b Integration + Sprint 22 Summary
**Status:** PASS (8/8 tests)

Adds Phase 22b integration coverage across remaining C++ feature-gap modules
and self-hosting signal checks against `TransformEngineExtended.h`.

**Files added:**
- `editor/tests/step470_test.cpp` — 8 integration tests covering:
  - self-host file signal checks (inheritance + protected section presence)
  - combined parse path across range-for/structured-binding/operator/initializer modules
  - exception + operator projection availability
  - STL iterator detection and annotation path
  - JSON round-trip checks across new Step 466-469 nodes
  - invalid-input regression checks
  - cross-language projection output shape checks
- `editor/CMakeLists.txt` — `step470_test` target

**Verification run:**
- `cmake --build editor/build-native --target step470_test` — PASS
- `./editor/build-native/step470_test` — PASS (8/8)
- `./editor/build-native/step469_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/tests/step470_test.cpp` within test-file size guidance (`157` lines)
- No new production header introduced in this integration step

**Sprint 22 totals:**
- **Steps:** 460-470 (11 steps)
- **Tests:** 124/124 passing
- **Headers added:** 9
  - `ast/AssemblyNodes.h`
  - `ast/X86AssemblyParser.h`
  - `ast/ArmAssemblyParser.h`
  - `ast/AssemblyGenerator.h`
  - `ast/AssemblyAnnotationMapper.h`
  - `ast/CppRangeStructured.h`
  - `ast/CppExceptions.h`
  - `ast/CppOperators.h`
  - `ast/CppInitializerStl.h`
- **Capabilities delivered:**
  - assembly AST support (x86 + ARM/AArch64) with parse/generate paths
  - cross-architecture x86→ARM simple instruction mapping
  - assembly annotation model (`@Exec`, `@Target`, `@Risk`, `@Complexity`, `@Align`)
  - C++ gap coverage for:
    - range-based for loops
    - structured bindings
    - try/catch/throw/noexcept patterns
    - operator overloading + friend operators
    - initializer lists + STL/iterator pattern detection

### Step 471: Problem Description Parser
**Status:** PASS (12/12 tests)

Starts Sprint 23 architect mode by extracting structured requirements from
natural-language problem descriptions with per-item confidence scores.

**Files added:**
- `editor/src/ArchitectProblemParser.h` — structured requirement parser:
  - output model: functional, non-functional, platform, integration buckets
  - heuristic keyword extraction with confidence scoring
  - deduplication behavior for repeated terms
  - fallback functional requirement when extraction is sparse
- `editor/tests/step471_test.cpp` — 12 tests covering:
  - functional/non-functional/platform/integration extraction
  - confidence range behavior
  - case-insensitive matching and dedupe
  - fallback behavior
  - mixed multi-category prompt extraction
- `editor/CMakeLists.txt` — `step471_test` target

**Verification run:**
- `cmake --build editor/build-native --target step471_test` — PASS
- `./editor/build-native/step471_test` — PASS (12/12)
- `./editor/build-native/step470_test` — PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/ArchitectProblemParser.h` within header-size limit (`114` <= `600`)
- `editor/tests/step471_test.cpp` within test-file size guidance (`157` lines)

### Step 472: Module Decomposition Engine
**Status:** PASS (12/12 tests)

Implements strategy-aware module decomposition from structured requirements,
with module graph nodes/edges, cross-cutting concerns, and complexity scoring.

**Files added:**
- `editor/src/ArchitectModuleDecomposer.h` — module decomposition engine:
  - decomposition strategies:
    - `Layered`
    - `Monolith`
    - `Microservice`
  - module graph model (`ModuleNode`, `ModuleEdge`, `ModuleGraph`)
  - inferred modules from requirements (auth/api/ui/data/search/reporting/notifications/integrations)
  - cross-cutting module insertion (logging/config/error/security/observability)
  - strategy-specific dependency edge shaping
  - normalized complexity scoring with dependency influence
  - helper APIs (`hasModule`, dependency counting)
- `editor/tests/step472_test.cpp` — 12 tests covering:
  - module inference from requirement categories
  - strategy-specific edge behavior
  - cross-cutting concern insertion
  - complexity clamping
  - monolith core-hub behavior
  - sparse-input fallback behavior
- `editor/CMakeLists.txt` — `step472_test` target

**Verification run:**
- `cmake --build editor/build-native --target step472_test` — PASS
- `./editor/build-native/step472_test` — PASS (12/12)
- `./editor/build-native/step471_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ArchitectModuleDecomposer.h` within header-size limit (`212` <= `600`)
- `editor/tests/step472_test.cpp` within test-file size guidance (`156` lines)

### Step 473: Technology Stack Selector
**Status:** PASS (12/12 tests)

Implements per-module technology stack selection from requirements + module
graph, including language/framework/database choices, rationale, confidence,
and preference overrides.

**Files added:**
- `editor/src/ArchitectTechStackSelector.h` — stack decision engine:
  - output model: `TechChoice`, `TechStackDecision`
  - strategy-informed module stack selection for UI/API/auth/data/integrations/etc.
  - backend/frontend/database preference overrides
  - language/framework/database heuristics with rationale/confidence
  - fallback behavior for sparse graphs
- `editor/tests/step473_test.cpp` — 12 tests covering:
  - UI/frontend stack selection
  - performance/security-driven backend language selection
  - database selection from integration requirements
  - preference override behavior (backend/db/frontend)
  - integration module adapter stack behavior
  - confidence range checks
  - per-module coverage and fallback decision behavior
- `editor/CMakeLists.txt` — `step473_test` target

**Verification run:**
- `cmake --build editor/build-native --target step473_test` — PASS
- `./editor/build-native/step473_test` — PASS (12/12)
- `./editor/build-native/step472_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ArchitectTechStackSelector.h` within header-size limit (`156` <= `600`)
- `editor/tests/step473_test.cpp` within test-file size guidance (`168` lines)

### Step 474: Skeleton Generation from Requirements
**Status:** PASS (12/12 tests)

Builds an annotated multi-module skeleton project spec from requirements,
module graph, and technology choices, including routing/contract/dependency
annotations suitable for workflow creation.

**Files added:**
- `editor/src/ArchitectSkeletonGenerator.h` — skeleton generator:
  - output model:
    - `SkeletonAnnotation`
    - `SkeletonFunctionSpec`
    - `SkeletonModuleSpec`
    - `SkeletonProjectSpec`
  - module-level skeleton assembly from stack decisions
  - function stub generation by module role (api/auth/ui/data/etc.)
  - annotation generation:
    - `@Intent`
    - `@Complexity`
    - `@ContextWidth`
    - `@Automatability`
    - `@Contract`
  - dependency capture from module graph edges
  - sparse/partial stack fallback behavior
- `editor/tests/step474_test.cpp` — 12 tests covering:
  - module and language propagation from graph + stack
  - function skeleton generation by module
  - annotation presence and value behavior
  - dependency propagation
  - context/automatability routing semantics
  - contract enrichment under security requirements
  - sparse-input fallback behavior
- `editor/CMakeLists.txt` — `step474_test` target

**Verification run:**
- `cmake --build editor/build-native --target step474_test` — PASS
- `./editor/build-native/step474_test` — PASS (12/12)
- `./editor/build-native/step473_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ArchitectSkeletonGenerator.h` within header-size limit (`198` <= `600`)
- `editor/tests/step474_test.cpp` within test-file size guidance (`187` lines)

### Step 475: Architect Review Interface
**Status:** PASS (12/12 tests)

Implements a structured architect review state with snapshot/summary generation
and concrete modification actions (language change, merge, add, routing
adjustment, approval).

**Files added:**
- `editor/src/ArchitectReviewInterface.h` — review interface model:
  - review state/snapshot models:
    - `ArchitectReviewState`
    - `ArchitectReviewSnapshot`
    - `ModuleReviewSummary`
  - review actions:
    - `changeModuleLanguage(...)`
    - `mergeModules(...)`
    - `addModule(...)`
    - `adjustRouting(...)`
    - `approve(...)`
  - change-log tracking and graph remapping after merges
  - key-annotation extraction for review summaries
- `editor/tests/step475_test.cpp` — 12 tests covering:
  - summary snapshot content
  - language change behavior
  - module merge success/failure paths
  - module add success/failure paths
  - routing adjustment success/failure paths
  - approval and change-log behavior
- `editor/CMakeLists.txt` — `step475_test` target

**Verification run:**
- `cmake --build editor/build-native --target step475_test` — PASS
- `./editor/build-native/step475_test` — PASS (12/12)
- `./editor/build-native/step474_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ArchitectReviewInterface.h` within header-size limit (`231` <= `600`)
- `editor/tests/step475_test.cpp` within test-file size guidance (`171` lines)

### Step 476: Phase 23a Integration
**Status:** PASS (8/8 tests)

Adds full Phase 23a integration coverage for architect mode:
problem description parsing → module decomposition → tech stack selection →
skeleton generation → architect review/approval → workflow task materialization.

**Files added:**
- `editor/tests/step476_test.cpp` — 8 integration tests covering:
  - end-to-end extraction/decomposition/selection flow for bookstore API scenario
  - frontend/database stack expectation checks
  - skeleton annotation presence checks
  - architect review modifications + approval
  - workflow task creation after approval
  - dependency graph and structured review snapshot checks
- `editor/CMakeLists.txt` — `step476_test` target

**Verification run:**
- `cmake --build editor/build-native --target step476_test` — PASS
- `./editor/build-native/step476_test` — PASS (8/8)
- `./editor/build-native/step475_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/tests/step476_test.cpp` within test-file size guidance (`148` lines)
- No new production header introduced in this integration step

**Phase 23a totals (471-476):**
- **Steps:** 6
- **Tests:** 68/68 passing
- **Headers added:** 5
  - `ArchitectProblemParser.h`
  - `ArchitectModuleDecomposer.h`
  - `ArchitectTechStackSelector.h`
  - `ArchitectSkeletonGenerator.h`
  - `ArchitectReviewInterface.h`
- **Capabilities delivered:**
  - natural-language requirement extraction with confidence
  - strategy-aware module decomposition graph generation
  - per-module tech stack selection with preference overrides
  - annotated multi-module skeleton generation
  - architect review/modify/approve state model
  - end-to-end architect-mode flow readiness for phase 23b scaffolding

### Step 477: Architect Templates
**Status:** PASS (12/12 tests)

Introduces reusable architecture templates that instantiate starter module
layouts, dependencies, function stubs, and annotations for common product
shapes (REST API, CLI, library, microservice, full stack).

**Files added:**
- `editor/src/ArchitectTemplates.h` — template library:
  - `ArchitectureTemplateKind`, `TemplateInput`, `TemplateOutput`
  - `ArchitectTemplates::instantiate(...)`
  - template-specific module graphs and function stubs
  - default annotation emission for generated functions:
    - `@Intent`
    - `@Complexity`
    - `@ContextWidth`
    - `@Automatability`
    - `@Contract`
- `editor/tests/step477_test.cpp` — 12 tests covering:
  - expected module sets for each template kind
  - annotation presence on generated function stubs
  - `primaryEntity` parameterization (e.g. `createBook`)
  - dependency expectations (e.g. `routes -> handlers`)
  - template metadata notes and language defaults
  - non-empty outputs across all template kinds
- `editor/CMakeLists.txt` — `step477_test` target

**Verification run:**
- `cmake --build editor/build-native --target step477_test step476_test` — PASS
- `./editor/build-native/step477_test` — PASS (12/12)
- `./editor/build-native/step476_test` — PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/ArchitectTemplates.h` within header-size limit (`182` <= `600`)
- `editor/tests/step477_test.cpp` within test-file size guidance (`166` lines)
- Naming/style aligned with `ARCHITECTURE.md` conventions (`PascalCase` types, `camelCase` functions)

### Step 478: Scaffold File Generation
**Status:** PASS (12/12 tests)

Adds concrete scaffold generation from approved skeletons, including source
files, stack-specific project config, `.gitignore`, and `.whetstone` state
files, with operation plans compatible with `fileCreate`/`fileWrite`.

**Files added:**
- `editor/src/ArchitectScaffoldGenerator.h` — scaffold planner/applier:
  - `ScaffoldInput`, `ScaffoldFileSpec`, `ScaffoldOperation`, `ScaffoldPlan`
  - `buildPlan(...)` to derive file tree + operation queue from skeleton modules
  - source rendering with Semanno annotation comments and per-language stubs
  - config generation:
    - `package.json` (JS/TS)
    - `pyproject.toml` (Python)
    - `Cargo.toml` (Rust)
    - `CMakeLists.txt` (C/C++)
  - `.gitignore` composition based on selected languages
  - `.whetstone/workflow_state.json` + sidecar module files
  - `applyPlan(...)` executing `fileCreate`/`fileWrite`-style operations via existing file ops
- `editor/tests/step478_test.cpp` — 12 tests covering:
  - Semanno annotations in generated source files
  - stack-specific config file generation
  - language convention path mapping (TS/Go/SQL)
  - `.gitignore` entries by stack
  - `.whetstone` workflow + sidecar generation
  - operation queue shape (`fileCreate` + `fileWrite` pairs)
  - on-disk apply behavior and idempotency
  - empty-skeleton baseline generation
- `editor/CMakeLists.txt` — `step478_test` target

**Verification run:**
- `cmake --build editor/build-native --target step478_test step477_test` — PASS
- `./editor/build-native/step478_test` — PASS (12/12)
- `./editor/build-native/step477_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ArchitectScaffoldGenerator.h` within header-size limit (`358` <= `600`)
- `editor/tests/step478_test.cpp` within test-file size guidance (`224` lines)
- Header-only architecture preserved; naming conventions remain `PascalCase` types and `camelCase` methods

### Step 479: Dependency + Build System Awareness
**Status:** PASS (12/12 tests)

Adds annotation-level dependency/build awareness derived from module graph
dependencies so scaffolded modules carry structured hints for packaging,
imports/includes, and SQL migration order.

**Files added:**
- `editor/src/ArchitectBuildAwareness.h` — dependency/build annotation engine:
  - `BuildDependencyAnnotation`, `BuildAwarenessReport`
  - `annotate(...)` for per-module build/dependency hints by language:
    - Python import hints + pip-style dependency snippet
    - Rust `Cargo.toml` `[dependencies]` snippet + local crate path links
    - Node/TypeScript package.json-style dependency snippet + import hints
    - C/C++ include hints + CMake target/link snippet
    - SQL migration ordering annotations with dependency-derived order
  - deterministic SQL cycle fallback behavior for stable ordering
- `editor/tests/step479_test.cpp` — 12 tests covering:
  - language-specific manifest/import annotation generation
  - dependency preservation and mixed-language report coverage
  - SQL migration ordering and cycle fallback
  - no-dependency and unknown-language edge cases
  - global note/disclaimer presence for annotation scope
- `editor/CMakeLists.txt` — `step479_test` target

**Verification run:**
- `cmake --build editor/build-native --target step479_test step478_test` — PASS
- `./editor/build-native/step479_test` — PASS (12/12)
- `./editor/build-native/step478_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ArchitectBuildAwareness.h` within header-size limit (`208` <= `600`)
- `editor/tests/step479_test.cpp` within test-file size guidance (`186` lines)
- Conventions aligned with `ARCHITECTURE.md` (`PascalCase` structs, `camelCase` methods, header-only)

### Step 480: Multi-Language Project Orchestration
**Status:** PASS (12/12 tests)

Introduces a single-project orchestration planner for multi-language module
graphs with deterministic build ordering and explicit cross-language
interface boundaries (`@Link`, `@Shim`) flagged for human review.

**Files added:**
- `editor/src/ArchitectMultiLanguageOrchestrator.h` — orchestration planner:
  - `CrossLanguageLink`, `OrchestrationTask`, `MultiLanguageWorkflowPlan`
  - `createWorkflow(...)` producing:
    - one task per module
    - dependency-respecting build order
    - cross-language link records with:
      - `@Link(from->to)`
      - `@Shim(source_to_target)`
      - `ffiBoundary = true`, `reviewRequired = true`
  - integration of build hints from `ArchitectBuildAwareness`
  - cycle fallback to deterministic lexical ordering with explicit note
- `editor/tests/step480_test.cpp` — 12 tests covering:
  - task generation for all modules in a single workflow
  - dependency-order correctness
  - cross-language link/shim emission
  - FFI review gating behavior
  - cycle fallback completeness
  - link direction correctness
  - single-language no-link behavior
  - propagation of build-awareness hints into tasks
- `editor/CMakeLists.txt` — `step480_test` target

**Verification run:**
- `cmake --build editor/build-native --target step480_test step479_test` — PASS
- `./editor/build-native/step480_test` — PASS (12/12)
- `./editor/build-native/step479_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ArchitectMultiLanguageOrchestrator.h` within header-size limit (`175` <= `600`)
- `editor/tests/step480_test.cpp` within test-file size guidance (`203` lines)
- Header-only and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 481: Phase 23b Integration + Sprint Summary
**Status:** PASS (8/8 tests)

Adds an integrated architect pipeline for Phase 23b that composes:
template instantiation, scaffold file planning/application, dependency/build
awareness, and multi-language orchestration into a single structured flow.

**Files added:**
- `editor/src/ArchitectProjectPipeline.h` — integration composition layer:
  - `ArchitectPipelineResult` aggregate output model
  - `fromTemplate(...)`:
    - `ArchitectTemplates` -> `ArchitectScaffoldGenerator` ->
      `ArchitectBuildAwareness` -> `ArchitectMultiLanguageOrchestrator`
  - `fromSkeleton(...)` for direct skeleton-based orchestration
  - `overrideModuleLanguage(...)` for mid-planning stack adjustment/replan flow
- `editor/tests/step481_test.cpp` — 8 integration tests covering:
  - REST API template -> scaffold -> workflow end-to-end
  - multi-language Python+Rust+SQL orchestration as one project plan
  - mid-planning language override and scaffold regeneration
  - Semanno annotations present in scaffolded source files
  - scaffold apply materialization (`.whetstone` + project files)
  - cross-language shim/review flags
  - sidecar generation completeness
  - structured output shape for MCP consumers
- `editor/CMakeLists.txt` — `step481_test` target

**Verification run:**
- `cmake --build editor/build-native --target step481_test step480_test` — PASS
- `./editor/build-native/step481_test` — PASS (8/8)
- `./editor/build-native/step480_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ArchitectProjectPipeline.h` within header-size limit (`58` <= `600`)
- `editor/tests/step481_test.cpp` within test-file size guidance (`156` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

**Phase 23b totals (477-481):**
- **Steps:** 5
- **Tests:** 56/56 passing
- **Headers added:** 5
  - `ArchitectTemplates.h`
  - `ArchitectScaffoldGenerator.h`
  - `ArchitectBuildAwareness.h`
  - `ArchitectMultiLanguageOrchestrator.h`
  - `ArchitectProjectPipeline.h`
- **Capabilities delivered:**
  - reusable architecture templates for common project archetypes
  - scaffold planning/application via `fileCreate`/`fileWrite`-compatible ops
  - language-specific dependency/build hint annotation layer
  - single-plan orchestration across multi-language module graphs with `@Link/@Shim`
  - integrated Phase 23b pipeline with mid-planning language override/replan support

**Sprint 23 totals (471-481):**
- **Steps:** 11
- **Tests:** 124/124 passing
- **Headers added:** 10
- **Outcome:** architect mode now supports requirement parsing, decomposition,
  stack selection, skeleton generation, review/approval, template/scaffold
  generation, dependency-aware build hints, and multi-language orchestration.

### Step 482: Step Spec Expander
**Status:** PASS (9/9 tests)

Begins Phase 23c self-hosting by adding a step-spec expansion engine that
turns sprint-step markdown into structured worker handoff artifacts:
test plan stubs, header skeleton outline, and build-system entry.

**Files added:**
- `editor/src/StepSpecExpander.h` — expansion engine:
  - `StepSpec`, `StepExpanderInput`, `PlannedTestStub`, `HeaderSkeletonStub`, `BuildEntryStub`
  - complexity classification:
    - `Trivial` (1-4 tests)
    - `Standard` (5-10 tests)
    - `Complex` (10-15 tests)
    - `Pipeline` (6-10 tests)
  - heuristic test-type planning from step text:
    - unit / negative / boundary / integration / smoke / regression
  - complexity-budget enforcement and deterministic test naming
  - header skeleton and build-entry stub generation (`cmake` + fallback systems)
- `editor/tests/step482_test.cpp` — 9 tests (unit/negative/boundary focus) covering:
  - complexity classification behavior
  - negative/boundary test injection heuristics
  - naming convention for generated test stubs
  - header/build skeleton output shape
  - empty-input warning path
  - complexity budget range enforcement
- `editor/CMakeLists.txt` — `step482_test` target

**Verification run:**
- `cmake --build editor/build-native --target step482_test step481_test` — PASS
- `./editor/build-native/step482_test` — PASS (9/9)
- `./editor/build-native/step481_test` — PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/StepSpecExpander.h` within header-size limit (`267` <= `600`)
- `editor/tests/step482_test.cpp` within test-file size guidance (`149` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 483: Convention Extractor + Validator
**Status:** PASS (9/9 tests)

Adds machine-readable project convention extraction and warning-level
validation for generated step artifacts, enabling architect review of
worker output conformance.

**Files added:**
- `editor/src/ProjectConventionAnalyzer.h` — extractor + validator:
  - `ProjectConventions`, `SourceFileDraft`
  - `ConventionViolation`, `ConventionValidationReport`
  - `extract(repoRoot)`:
    - parses architecture limits (`header`, `main.cpp`, `function`)
    - detects build target pattern from `editor/CMakeLists.txt`
    - detects test harness conventions from step test files
  - `validate(conventions, files)` warning checks:
    - header/main line-limit violations
    - missing test assertions (`CHECK/assert`)
    - class/struct naming mismatch (`PascalCase`)
    - missing step test target in CMake snippet
- `editor/tests/step483_test.cpp` — 9 tests (unit/negative/regression):
  - extraction correctness from repo docs/build files
  - negative cases for size, assertions, naming, and build entry violations
  - valid-file acceptance path
  - regression validation against Step 482 outputs
- `editor/CMakeLists.txt` — `step483_test` target

**Verification run:**
- `cmake --build editor/build-native --target step483_test step482_test` — PASS
- `./editor/build-native/step483_test` — PASS (9/9)
- `./editor/build-native/step482_test` — PASS (9/9) regression coverage

**Architecture gate check:**
- `editor/src/ProjectConventionAnalyzer.h` within header-size limit (`223` <= `600`)
- `editor/tests/step483_test.cpp` within test-file size guidance (`146` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 484: Prior-Step Context Injector
**Status:** PASS (8/8 tests)

Adds prior-step API/context injection so worker-facing briefs can include
condensed type/method summaries and include dependencies instead of full
headers.

**Files added:**
- `editor/src/PriorStepContextInjector.h` — context extraction layer:
  - `ApiTypeSummary`, `ApiMethodSummary`, `StepContext`
  - `build(headerPaths)`:
    - parses class/struct names from prior headers
    - extracts declaration/definition-level method signatures
    - records include dependency graph from `#include "..."` lines
    - infers naming patterns from type suffixes (`Spec`, `State`, `Report`)
    - emits warnings for missing/unreadable headers
  - `condensedBrief()` for prompt-sized worker context summaries
- `editor/tests/step484_test.cpp` — 8 tests (unit/integration) covering:
  - type extraction from real prior-step headers
  - method signature extraction and declaration-only shape checks
  - include graph construction
  - naming pattern detection
  - condensed brief content
  - missing-header warning behavior
  - multi-header integration behavior
- `editor/CMakeLists.txt` — `step484_test` target

**Verification run:**
- `cmake --build editor/build-native --target step484_test step483_test` — PASS
- `./editor/build-native/step484_test` — PASS (8/8)
- `./editor/build-native/step483_test` — PASS (9/9) regression coverage

**Architecture gate check:**
- `editor/src/PriorStepContextInjector.h` within header-size limit (`125` <= `600`)
- `editor/tests/step484_test.cpp` within test-file size guidance (`120` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 485: Worker Dispatch + Result Validation
**Status:** PASS (10/10 tests)

Implements the architect->worker dispatch protocol model for Phase 23c:
prepare spec/context/conventions, validate worker output, enforce retry
budget, and escalate unresolved submissions to architect review.

**Files added:**
- `editor/src/WorkerDispatchValidator.h` — dispatch orchestration model:
  - `WorkerDispatchInput`, `WorkerSubmission`, `DispatchAttemptReport`, `WorkerDispatchResult`
  - `run(...)`:
    - builds `StepSpec` via `StepSpecExpander`
    - builds `StepContext` via `PriorStepContextInjector`
    - extracts `ProjectConventions` via `ProjectConventionAnalyzer`
    - validates each worker submission (build/tests/conventions)
    - retries up to configurable budget (default `3`)
    - marks completion only when build+tests pass and no convention warnings
    - escalates on exhausted retries or missing submissions
  - `buildWorkerBrief(...)` condensed handoff text for worker prompts
- `editor/tests/step485_test.cpp` — 10 tests (unit/integration/negative):
  - pre-dispatch artifact generation (spec/context/conventions)
  - successful first-attempt acceptance
  - retry-after-failure success path
  - retry-budget exhaustion escalation
  - convention-violation reporting path
  - build-failure status propagation
  - empty-submission negative case
  - default retry-budget behavior
  - worker brief content checks
  - acceptance gate requiring both build and tests
- `editor/CMakeLists.txt` — `step485_test` target

**Verification run:**
- `cmake --build editor/build-native --target step485_test step484_test` — PASS
- `./editor/build-native/step485_test` — PASS (10/10)
- `./editor/build-native/step484_test` — PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/WorkerDispatchValidator.h` within header-size limit (`145` <= `600`)
- `editor/tests/step485_test.cpp` within test-file size guidance (`165` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 486: Test Plan Generator
**Status:** PASS (10/10 tests)

Implements the test-plan intelligence layer for Phase 23c: proposes typed,
named tests from step semantics and complexity budgets instead of fixed
test counts.

**Files added:**
- `editor/src/TestPlanGenerator.h` — adaptive test-plan engine:
  - `TestPlanInput`, `GeneratedTestPlan`, `TestPatternSample`
  - typed test proposal across taxonomy:
    - unit / negative / integration / contract / regression / boundary /
      property / smoke / performance
  - keyword heuristics aligned to sprint plan:
    - validate/detect -> negative
    - pipeline/workflow -> integration
    - RPC/MCP -> smoke
    - score/boundary/max/min -> boundary
    - modify-existing -> regression
  - prior-test pattern influence for regression continuity
  - complexity-aware budget enforcement:
    - trivial: 1-4
    - standard: 5-10
    - complex: 10-15
    - pipeline: 6-10
- `editor/tests/step486_test.cpp` — 10 tests (unit/boundary/negative focus):
  - heuristic inclusion checks by keyword class
  - regression inference from prior test patterns
  - budget range checks under explicit complexity hint
  - generated test name convention checks
- `editor/CMakeLists.txt` — `step486_test` target

**Verification run:**
- `cmake --build editor/build-native --target step486_test step485_test` — PASS
- `./editor/build-native/step486_test` — PASS (10/10)
- `./editor/build-native/step485_test` — PASS (10/10) regression coverage

**Architecture gate check:**
- `editor/src/TestPlanGenerator.h` within header-size limit (`191` <= `600`)
- `editor/tests/step486_test.cpp` within test-file size guidance (`118` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 487: Phase 23c Integration
**Status:** PASS (8/8 tests)

Completes Phase 23c by validating the full self-hosting data flow:
sprint step description -> conventions extraction -> prior-context
injection -> step spec expansion -> adaptive test plan generation ->
output validation on known-good and deliberately-broken step artifacts.

**Files added:**
- `editor/src/SelfHostingIntegration.h` — phase integration composition:
  - `SelfHostingInput`, `SelfHostingResult`
  - `run(...)` flow:
    - `ProjectConventionAnalyzer::extract(...)`
    - `PriorStepContextInjector::build(...)`
    - `StepSpecExpander::expand(...)`
    - `TestPlanGenerator::generate(...)`
    - convention validation for known-good and known-broken step files
  - worker-readiness sufficiency gate (`workerReady`) over spec/context/
    conventions/test-plan completeness
- `editor/tests/step487_test.cpp` — 8 integration/regression tests covering:
  - full flow population of conventions/context/spec/test-plan
  - worker readiness sufficiency for realistic step input
  - pipeline heuristic propagation into generated test types
  - zero-violation validation for known-good step output
  - violation detection for deliberately-broken output
  - flow-note and context-summary checks
  - regression expectation alignment with Step 486 behavior
- `editor/CMakeLists.txt` — `step487_test` target

**Verification run:**
- `cmake --build editor/build-native --target step487_test step486_test` — PASS
- `./editor/build-native/step487_test` — PASS (8/8)
- `./editor/build-native/step486_test` — PASS (10/10) regression coverage

**Architecture gate check:**
- `editor/src/SelfHostingIntegration.h` within header-size limit (`80` <= `600`)
- `editor/tests/step487_test.cpp` within test-file size guidance (`126` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

**Phase 23c totals (482-487):**
- **Steps:** 6
- **Tests:** 54/54 passing
- **Headers added:** 6
  - `StepSpecExpander.h`
  - `ProjectConventionAnalyzer.h`
  - `PriorStepContextInjector.h`
  - `WorkerDispatchValidator.h`
  - `TestPlanGenerator.h`
  - `SelfHostingIntegration.h`
- **Capabilities delivered:**
  - step-to-spec expansion with complexity-bounded test planning
  - machine-readable convention extraction + warning validator
  - prior-step API context briefing for worker prompts
  - dispatch/retry/escalation protocol model for architect-worker handoff
  - adaptive typed test-plan generation (taxonomy-aware)
  - end-to-end self-hosting protocol validation

**Final Sprint 23 totals (471-487):**
- **Steps:** 17
- **Tests:** 178/178 passing
- **Headers added:** 16
- **Outcome:** architect mode now spans full planning stack and self-hosting
  protocol: requirements -> decomposition -> stack selection -> skeleton ->
  review -> templates/scaffolding -> build/dependency awareness ->
  multi-language orchestration -> step spec/context/convention/test-plan
  synthesis -> dispatch/validation -> phase-level integration validation.

### Step 488: Security-Preserving Translation
**Status:** PASS (12/12 tests)

Starts Sprint 24 Phase 24b by adding a security-preserving annotation
translation layer so security metadata is carried into target-language
representations and never silently dropped during transpilation.

**Files added:**
- `editor/src/SecurityPreservingTranslation.h` — security annotation mapper:
  - `SecurityTranslationInput`, `SecuritySourceAnnotation`
  - `SecurityMappedAnnotation`, `SecurityTranslationOutput`
  - `translate(...)` mapping behavior:
    - `@InputValidation` mapped to target-language validation forms
    - `@TrustBoundary` preserved across boundaries
    - `@Auth` mapped to target auth patterns
    - `@Encryption` mapped to target crypto library forms
    - `@CORS` mapped for supported targets
    - `@Secrets` preserved but always `reviewRequired`
  - strict guardrails:
    - security annotations are always preserved
    - unknown/unmappable security annotations trigger human review
    - insecure crypto values (`md5`, `sha1`) force review
- `editor/tests/step488_test.cpp` — 12 tests covering:
  - per-annotation mapping behavior
  - unsupported-language review escalation
  - no-annotation edge case
  - preservation-count invariants
  - blocking-review flag semantics
- `editor/CMakeLists.txt` — `step488_test` target

**Verification run:**
- `cmake --build editor/build-native --target step488_test step487_test` — PASS
- `./editor/build-native/step488_test` — PASS (12/12)
- `./editor/build-native/step487_test` — PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/SecurityPreservingTranslation.h` within header-size limit (`166` <= `600`)
- `editor/tests/step488_test.cpp` within test-file size guidance (`147` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 489: Secure-by-Default Code Generation
**Status:** PASS (12/12 tests)

Implements secure-by-default snippet generation for high-risk operation
classes so generated code starts from safe patterns instead of relying on
post-hoc hardening.

**Files added:**
- `editor/src/SecureByDefaultGenerator.h` — secure generation engine:
  - `SecureGenerationRequest`, `SecureGenerationOutput`, `SecureSnippetKind`
  - snippet generators for:
    - SQL queries (parameterized placeholders / bind APIs)
    - input handling (`@InputValidation(raw)` baseline)
    - file operations (path traversal guards)
    - network calls (timeouts + TLS verification)
    - crypto operations (modern defaults + mandatory human review)
  - baseline annotation emission:
    - `@Risk(security)`
    - `@Review(required, human)`
    - optional `@SecurityRequirement(...)`
    - `@Automatability(human)` for crypto
  - weak-crypto guard (`md5`/`sha1`) escalation path
- `editor/tests/step489_test.cpp` — 12 tests covering:
  - parameterized SQL enforcement
  - raw-input annotation semantics
  - path traversal checks
  - timeout/TLS requirements
  - crypto hardening defaults and human-review gating
  - base security annotation coverage across snippet kinds
  - language-specific secure path checks (Rust SQL/network)
  - secure fallback behavior for unknown languages
  - operation-name propagation and security requirement annotation
- `editor/CMakeLists.txt` — `step489_test` target

**Verification run:**
- `cmake --build editor/build-native --target step489_test step488_test` — PASS
- `./editor/build-native/step489_test` — PASS (12/12)
- `./editor/build-native/step488_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/SecureByDefaultGenerator.h` within header-size limit (`202` <= `600`)
- `editor/tests/step489_test.cpp` within test-file size guidance (`164` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 490: Threat Model Integration
**Status:** PASS (12/12 tests)

Adds threat-model integration primitives that encode trust boundaries and
boundary-crossing data-flow validation requirements, emitting structured
E1300-range diagnostics and overlay-ready graph data.

**Files added:**
- `editor/src/ThreatModelIntegration.h` — threat model analyzer:
  - `TrustBoundary`, `ThreatDataFlow`, `ThreatModelInput`
  - `ThreatDiagnostic`, `ThreatModelOutput`
  - `analyze(...)` capabilities:
    - module-level `@ThreatModel(...)` annotation requirement
    - boundary-level `@TrustBoundary(...)` requirements
    - cross-boundary `@DataFlow(...)` requirement emission
    - missing-validation detection at high-risk crossings (`E1300`)
    - raw-validation detection (`E1301`)
    - missing-boundary model diagnostic (`E1302`)
    - unknown validation kind diagnostic (`E1303`)
    - overlay node output for trust-boundary visualization
- `editor/tests/step490_test.cpp` — 12 tests covering:
  - annotation emission for module/boundary/flow modeling
  - E1300/E1301/E1302/E1303 diagnostic behavior
  - sanitized-flow non-error path
  - validation-required annotation on missing checks
  - non-crossing internal-flow false-positive guard
  - overlay and empty-flow note behavior
- `editor/CMakeLists.txt` — `step490_test` target

**Verification run:**
- `cmake --build editor/build-native --target step490_test step489_test` — PASS
- `./editor/build-native/step490_test` — PASS (12/12)
- `./editor/build-native/step489_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ThreatModelIntegration.h` within header-size limit (`126` <= `600`)
- `editor/tests/step490_test.cpp` within test-file size guidance (`167` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 491: Security Testing Skeleton Generation
**Status:** PASS (12/12 tests)

Adds security-focused test skeleton generation so threat-relevant test
coverage is scaffolded alongside code and routed by complexity/risk.

**Files added:**
- `editor/src/SecurityTestSkeletonGenerator.h` — security test skeleton engine:
  - `SecurityTestKind`, `SecurityTestCaseSpec`
  - `SecuritySkeletonRequest`, `SecuritySkeletonOutput`
  - generated test classes:
    - input validation fuzz
    - auth bypass
    - SQL injection probe
    - XSS payload probe
    - access control matrix
  - per-test `@Intent(...)` annotation generation
  - routing hints:
    - simple validation/access matrix -> `slm`
    - penetration-style tests -> `human` + `humanReviewRequired`
  - optional force-human behavior for complex pen-test categories
- `editor/tests/step491_test.cpp` — 12 tests covering:
  - presence of each security test kind
  - intent annotation completeness
  - routing policy behavior by test complexity
  - force-human mode behavior and notes
  - module/entity propagation into test names/intent text
  - summary note emission
- `editor/CMakeLists.txt` — `step491_test` target

**Verification run:**
- `cmake --build editor/build-native --target step491_test step490_test` — PASS
- `./editor/build-native/step491_test` — PASS (12/12)
- `./editor/build-native/step490_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/SecurityTestSkeletonGenerator.h` within header-size limit (`140` <= `600`)
- `editor/tests/step491_test.cpp` within test-file size guidance (`164` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 492: Phase 24b Integration + Sprint Summary
**Status:** PASS (8/8 tests)

Integrates Phase 24b security components into one pipeline:
security-preserving translation, secure-by-default generation, threat-model
analysis, and security test skeleton generation.

**Files added:**
- `editor/src/SecurityPipelineIntegration.h` — composed security pipeline:
  - `SecurityPipelineInput`, `SecurityPipelineResult`
  - `run(...)` flow:
    - `SecurityPreservingTranslation::translate(...)`
    - `SecureByDefaultGenerator::generate(...)` for SQL + input handling snippets
    - `ThreatModelIntegration::analyze(...)`
    - `SecurityTestSkeletonGenerator::generate(...)`
  - pipeline notes include execution trace and blocking-review propagation
- `editor/tests/step492_test.cpp` — 8 integration tests covering:
  - Python->Rust security annotation preservation
  - secure-default SQL/input generation checks
  - threat-model diagnostics on missing validation
  - security test skeleton quality/routing checks
  - blocking-review propagation from translation stage
  - phase regression expectations against Step 491 behavior
- `editor/CMakeLists.txt` — `step492_test` target

**Verification run:**
- `cmake --build editor/build-native --target step492_test step491_test` — PASS
- `./editor/build-native/step492_test` — PASS (8/8)
- `./editor/build-native/step491_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/SecurityPipelineIntegration.h` within header-size limit (`61` <= `600`)
- `editor/tests/step492_test.cpp` within test-file size guidance (`137` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

**Phase 24b totals (488-492):**
- **Steps:** 5
- **Tests:** 56/56 passing
- **Headers added:** 5
  - `SecurityPreservingTranslation.h`
  - `SecureByDefaultGenerator.h`
  - `ThreatModelIntegration.h`
  - `SecurityTestSkeletonGenerator.h`
  - `SecurityPipelineIntegration.h`
- **Capabilities delivered:**
  - security annotation preservation across transpilation boundaries
  - secure-by-default generation for SQL/input/file/network/crypto patterns
  - trust boundary + data-flow threat diagnostics (`E1300` range)
  - routed security test skeleton generation
  - end-to-end security pipeline integration

### Step 493: Parse Full Whetstone Codebase
**Status:** PASS (12/12 tests)

Starts Sprint 25 self-hosting by auditing parser coverage across the
Whetstone header corpus (`editor/src/` + `editor/src/ast/`), including
parse-rate metrics, construct counts, and skipped-construct logging.

**Files added:**
- `editor/src/SelfHostCodebaseAudit.h` — codebase parse audit layer:
  - `FileParseAudit`, `CodebaseParseAudit`
  - header discovery over target directories (`.h` recursive)
  - per-file parse via existing `SelfHostHarness::parseFile(...)`
  - per-file metrics:
    - class count
    - function count
    - annotation-node count (`conceptType` contains `Annotation`)
  - heuristic skipped-construct detection from textual hints vs parsed counts
  - aggregate parse-rate computation + target check (`meetsTarget(0.95)`)
  - unparseable/skipped construct audit log for follow-up coverage work
- `editor/tests/step493_test.cpp` — 12 tests covering:
  - real directory header discovery
  - aggregate metric sanity and parse-rate bounds
  - per-file audit shape guarantees
  - skipped/unparseable logging paths
  - threshold semantics
  - direct-file-list audit mode
  - real-corpus scale signal checks
- `editor/CMakeLists.txt` — `step493_test` target

**Verification run:**
- `cmake --build editor/build-native --target step493_test step492_test` — PASS
- `./editor/build-native/step493_test` — PASS (12/12)
- `./editor/build-native/step492_test` — PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/SelfHostCodebaseAudit.h` within header-size limit (`139` <= `600`)
- `editor/tests/step493_test.cpp` within test-file size guidance (`172` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 494: Annotate Whetstone's Own Code
**Status:** PASS (12/12 tests)

Builds a self-host annotation audit pass over parsed Whetstone headers to
measure inferred annotation signals (`@Complexity`, `@Risk`, `@Owner`,
`@Intent`, `@TailCall`) on the codebase itself.

**Files added:**
- `editor/src/SelfHostAnnotationAudit.h` — annotation audit layer:
  - `FileAnnotationAudit`, `SelfAnnotationAuditReport`
  - integrates:
    - `SelfHostHarness` parsing
    - `AnnotationInference::inferAll(...)`
    - `SafetyAuditor` risk signal extraction
  - per-file/aggregate metrics:
    - inferred annotation totals
    - complexity counts
    - risk counts (safety findings + unsafe cast/pointer pattern signals)
    - owner counts (inference + smart-pointer pattern signals)
    - intent counts from descriptive function names
    - tail-call counts
  - inline-source audit mode for focused rule tests
- `editor/tests/step494_test.cpp` — 12 tests covering:
  - real-corpus audit execution and parse-rate metrics
  - category signal extraction (complexity/risk/owner/intent/tailcall)
  - negative path for unparseable input
  - aggregate-vs-per-file metric consistency
  - real-corpus non-zero signal checks
- `editor/CMakeLists.txt` — `step494_test` target

**Verification run:**
- `cmake --build editor/build-native --target step494_test step493_test` — PASS
- `./editor/build-native/step494_test` — PASS (12/12)
- `./editor/build-native/step493_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/SelfHostAnnotationAudit.h` within header-size limit (`180` <= `600`)
- `editor/tests/step494_test.cpp` within test-file size guidance (`142` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 495: Transpile Whetstone Modules
**Status:** PASS (12/12 tests)

Adds a self-host transpilation audit for three concrete Whetstone headers:
`AnnotationConflictExtended.h` -> Python, `ResponseBudget.h` -> Rust,
`Icons.h` -> Java, with structure/annotation preservation and confidence
metrics.

**Files added:**
- `editor/src/SelfHostTranspileAudit.h` — self-host transpilation audit layer:
  - `SelfTranspileSpec`, `SelfTranspileFileReport`, `SelfTranspileAuditReport`
  - default spec set for the three sprint-target headers
  - source parse (`cpp`) -> projection (`CrossLanguageProjector`) -> target generation
  - checks:
    - source parse success
    - projection success
    - annotation preservation (`annotationsPreserved`)
    - generated output non-empty
    - target-language parse viability
  - confidence scoring via `TranspilationScorer`
  - idiomatic vs literal classification derived from confidence category
  - deterministic fallback generation scaffold when primary generator returns empty
- `editor/tests/step495_test.cpp` — 12 tests covering:
  - expected header/target mapping set
  - per-file audit shape and success metrics
  - non-empty generation guarantees
  - annotation-preservation checks
  - confidence and aggregate metric bounds
  - idiomatic/literal accounting and summary notes
- `editor/CMakeLists.txt` — `step495_test` target

**Verification run:**
- `cmake --build editor/build-native --target step495_test step494_test` — PASS
- `./editor/build-native/step495_test` — PASS (12/12)
- `./editor/build-native/step494_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/SelfHostTranspileAudit.h` within header-size limit (`178` <= `600`)
- `editor/tests/step495_test.cpp` within test-file size guidance (`139` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 496: Self-Annotated Workflow
**Status:** PASS (12/12 tests)

Adds a self-modernization workflow model targeting
`editor/src/AnnotationValidator.h`, with rule-level routing across
deterministic, LLM, and human-review paths and deterministic output
validity checks.

**Files added:**
- `editor/src/SelfModernizationWorkflow.h` — self-improvement workflow layer:
  - `ModernizationRuleSpec`, `ModernizationTask`, `ModernizationWorkflowReport`
  - `defaultRules()` for validator modernization rule set
  - routing policy:
    - simple rules -> `deterministic`
    - complex rules -> `llm` + review
    - new diagnostic code rules -> `human` + critical priority
  - deterministic execution path generating validator rule stubs
  - deterministic output validation via C++ parser (`Pipeline::parse`)
  - aggregate routing and deterministic-validity metrics
- `editor/tests/step496_test.cpp` — 12 tests covering:
  - target file and default rule taxonomy
  - routing behavior by rule complexity/type
  - deterministic code generation and parser-valid output checks
  - workflow summary/count consistency
  - diagnostic-code range semantics
  - completion-note presence
- `editor/CMakeLists.txt` — `step496_test` target

**Verification run:**
- `cmake --build editor/build-native --target step496_test step495_test` — PASS
- `./editor/build-native/step496_test` — PASS (12/12)
- `./editor/build-native/step495_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/SelfModernizationWorkflow.h` within header-size limit (`157` <= `600`)
- `editor/tests/step496_test.cpp` within test-file size guidance (`159` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 497: Self-Hosting Metrics Report
**Status:** PASS (12/12 tests)

Adds a consolidated self-hosting metrics layer that aggregates parse coverage,
annotation signals, transpilation quality, and workflow viability into a
single report with gap analysis.

**Files added:**
- `editor/src/SelfHostingMetricsReport.h` — metrics aggregation/reporting:
  - `SelfHostingMetrics` model:
    - parse coverage metrics
    - annotation signal metrics/density
    - transpilation confidence + review pressure
    - workflow task/viability metrics
    - gap analysis + notes
  - `generate(...)` for composed report from:
    - `CodebaseParseAudit`
    - `SelfAnnotationAuditReport`
    - `SelfTranspileAuditReport`
    - `ModernizationWorkflowReport`
  - `generateFromCurrentState()` convenience path that runs all upstream
    self-host phase components
  - threshold-based gap detection:
    - parse coverage < 95%
    - low annotation density
    - low transpilation confidence
    - non-viable deterministic modernization outputs
- `editor/tests/step497_test.cpp` — 12 tests covering:
  - population of all metric categories
  - deterministic gap-analysis behavior under synthetic thresholds
  - no-blocking-gap path when all thresholds pass
  - parse-coverage ratio math correctness
  - workflow viability signal semantics
- `editor/CMakeLists.txt` — `step497_test` target

**Verification run:**
- `cmake --build editor/build-native --target step497_test step496_test` — PASS
- `./editor/build-native/step497_test` — PASS (12/12)
- `./editor/build-native/step496_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/SelfHostingMetricsReport.h` within header-size limit (`96` <= `600`)
- `editor/tests/step497_test.cpp` within test-file size guidance (`165` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 498: Phase 25a Integration
**Status:** PASS (8/8 tests)

Integrates the full Phase 25a self-hosting pipeline:
parse -> annotate -> transpile -> modernization workflow -> metrics,
with thesis-level gate checks for parse coverage, transpilation breadth,
and workflow viability.

**Files added:**
- `editor/src/SelfHostingPhase25aIntegration.h` — phase integration composer:
  - `Phase25aIntegrationResult`
  - `runCurrentState()` executes:
    - `SelfHostCodebaseAudit`
    - `SelfHostAnnotationAudit`
    - `SelfHostTranspileAudit`
    - `SelfModernizationWorkflow`
    - `SelfHostingMetricsReport`
  - gate evaluation:
    - parse coverage gate (`>=95%`)
    - transpilation gate (`>=3` modules)
    - workflow gate (all deterministic outputs valid)
  - phase pass/fail synthesis + summary notes
- `editor/tests/step498_test.cpp` — 8 integration tests covering:
  - full pipeline execution
  - gate threshold semantics
  - phase pass conjunction logic
  - cross-report metric consistency
  - integration note/status behavior
- `editor/CMakeLists.txt` — `step498_test` target

**Verification run:**
- `cmake --build editor/build-native --target step498_test step497_test` — PASS
- `./editor/build-native/step498_test` — PASS (8/8)
- `./editor/build-native/step497_test` — PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/SelfHostingPhase25aIntegration.h` within header-size limit (`49` <= `600`)
- `editor/tests/step498_test.cpp` within test-file size guidance (`105` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

**Phase 25a totals (493-498):**
- **Steps:** 6
- **Tests:** 68/68 passing
- **Headers added:** 6
  - `SelfHostCodebaseAudit.h`
  - `SelfHostAnnotationAudit.h`
  - `SelfHostTranspileAudit.h`
  - `SelfModernizationWorkflow.h`
  - `SelfHostingMetricsReport.h`
  - `SelfHostingPhase25aIntegration.h`
- **Capabilities delivered:**
  - full-header parser coverage auditing across Whetstone source
  - self-code annotation signal auditing
  - targeted self-module transpilation auditing (Python/Rust/Java)
  - self-modernization workflow routing + deterministic output validation
  - consolidated self-host metrics/gap analysis
  - phase-level self-hosting thesis gate evaluation

### Step 499: Scenario - Greenfield Project
**Status:** PASS (12/12 tests)

Implements the first Phase 25b end-to-end scenario for:
"Build a URL shortener with user accounts".

**Files added:**
- `editor/src/GreenfieldScenarioRunner.h` - scenario orchestration across:
  - problem parsing and layered decomposition
  - stack selection with explicit preferences (`python`, `postgresql`, `typescript`)
  - annotated skeleton generation + scaffold planning/materialization
  - task routing (`deterministic` / `llm` / `human`) with context bundles
  - security auditing over generated code surfaces
- `editor/tests/step499_test.cpp` - 12 scenario tests covering:
  - decomposition + stack expectations
  - annotation presence in generated skeleton functions
  - deterministic auto-complete, llm context preparation, and human-review surfacing
  - scaffold sidecars/workflow-state materialization on disk
  - security-findings/report shape and execution notes
- `editor/CMakeLists.txt` - `step499_test` target

**Verification run:**
- `cmake --build editor/build-native --target step499_test step498_test` - PASS
- `./editor/build-native/step499_test` - PASS (12/12)
- `./editor/build-native/step498_test` - PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/GreenfieldScenarioRunner.h` within header-size limit (`153` <= `600`)
- `editor/tests/step499_test.cpp` within test-file size guidance (`191` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 500: Scenario - Legacy Modernization
**Status:** PASS (12/12 tests)

Implements the second Phase 25b end-to-end scenario for modernizing a legacy C codebase
with Rust as the migration target.

**Files added:**
- `editor/src/LegacyModernizationScenarioRunner.h` - scenario orchestration across:
  - legacy C ingestion and idiom detection
  - safety audit and modernization suggestion generation
  - modernization workflow generation with mixed routing
  - migration plan + execution initialization (`c` -> `rust`)
  - API boundary analysis/preservation verification
  - migration test generation from preserved API surface
- `editor/tests/step500_test.cpp` - 12 scenario tests covering:
  - legacy/safety finding expectations
  - Rust-target modernization suggestions (including ownership strengthening)
  - deterministic + llm workflow and migration routing
  - API boundary + FFI annotation verification
  - migration test-suite generation and scenario notes
- `editor/CMakeLists.txt` - `step500_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step500_test step499_test` - PASS
- `./editor/build-native/step500_test` - PASS (12/12)
- `./editor/build-native/step499_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/LegacyModernizationScenarioRunner.h` within header-size limit (`125` <= `600`)
- `editor/tests/step500_test.cpp` within test-file size guidance (`147` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 501: Scenario - Cross-Language Port
**Status:** PASS (12/12 tests)

Implements the third Phase 25b end-to-end scenario for porting a Python ML-style
pipeline to Rust with confidence and review gating.

**Files added:**
- `editor/src/CrossLanguagePortScenarioRunner.h` - scenario orchestration across:
  - Python function intake and hot-loop tagging
  - intent-driven semantic transpilation (`python` -> `rust`)
  - memory model analysis (GC -> borrow-checked target model)
  - concurrency translation (`asyncio`/async-await -> Rust tokio-oriented runtime notes)
  - per-function confidence scoring and low-confidence review surfacing
  - behavioral equivalence assertion generation
- `editor/tests/step501_test.cpp` - 12 scenario tests covering:
  - language/source expectations and function extraction
  - hot-loop identification
  - idiomatic transpilation patterns
  - memory/concurrency translation expectations
  - confidence/review semantics
  - equivalence assertion generation and scenario notes
- `editor/CMakeLists.txt` - `step501_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step501_test step500_test` - PASS
- `./editor/build-native/step501_test` - PASS (12/12)
- `./editor/build-native/step500_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/CrossLanguagePortScenarioRunner.h` within header-size limit (`127` <= `600`)
- `editor/tests/step501_test.cpp` within test-file size guidance (`165` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 502: Scenario - Multi-Model Orchestration
**Status:** PASS (12/12 tests)

Implements the fourth Phase 25b end-to-end scenario for a 20-function project
that exercises all worker classes and review gates.

**Files added:**
- `editor/src/MultiModelOrchestrationScenarioRunner.h` - scenario orchestration across:
  - 20-task synthetic project generation
  - routing through deterministic/template/slm/llm/human workers
  - deterministic/template auto-completion behavior
  - per-task token estimate vs actual token tracking
  - progress snapshots across full completion lifecycle
  - review gate tracking for LLM + human paths
- `editor/tests/step502_test.cpp` - 12 scenario tests covering:
  - exact routing distribution and task-group counts
  - narrow vs wide context routing semantics
  - auto-completion expectations
  - review gate exercise and cost/progress accounting
  - completion and summary notes
- `editor/CMakeLists.txt` - `step502_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step502_test step501_test` - PASS
- `./editor/build-native/step502_test` - PASS (12/12)
- `./editor/build-native/step501_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/MultiModelOrchestrationScenarioRunner.h` within header-size limit (`177` <= `600`)
- `editor/tests/step502_test.cpp` within test-file size guidance (`152` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 503: Phase 25b Integration
**Status:** PASS (8/8 tests)

Integrates all four Phase 25b scenario runners (steps 499-502) into a single
integration pass with scenario-level status, cost snapshots, security aggregation,
workflow-path diversity checks, and event-stream completeness signals.

**Files added:**
- `editor/src/Phase25bIntegration.h` - integration compositor:
  - executes Greenfield, Legacy Modernization, Cross-Language Port, and Multi-Model scenarios
  - emits per-scenario start/complete events
  - computes per-scenario estimated vs actual token snapshots
  - validates cross-scenario path diversity and cost/event gating
  - aggregates security findings from generated code/audits
- `editor/tests/step503_test.cpp` - 8 integration tests covering:
  - all-scenario pass state
  - unique workflow-path signal coverage
  - cost tracking shape/accuracy gating
  - security finding aggregation
  - event stream completeness
  - integrated review-gate behavior retention
- `editor/CMakeLists.txt` - `step503_test` target

**Compatibility fix applied during integration:**
- `editor/src/APIBoundaryPreserver.h`
- `editor/src/MigrationTestGenerator.h`
  - renamed API migration contract struct to `APIMigrationContract` to resolve
    a symbol collision with AST `ContractAnnotation` when scenario headers are composed
    in one translation unit.

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step503_test step502_test` - PASS
- `./editor/build-native/step503_test` - PASS (8/8)
- `./editor/build-native/step502_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/Phase25bIntegration.h` within header-size limit (`229` <= `600`)
- `editor/tests/step503_test.cpp` within test-file size guidance (`97` lines)
- Updated integration dependencies remain within header-size limits:
  - `editor/src/APIBoundaryPreserver.h` (`253` <= `600`)
  - `editor/src/MigrationTestGenerator.h` (`213` <= `600`)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 504: Performance Optimization
**Status:** PASS (12/12 tests)

Implements a measurable optimization harness for key hot paths:
AST serialization, annotation batching, context assembly caching, and compact
representation generation, with sustained-run stability checks.

**Files added:**
- `editor/src/PerformanceOptimizationSuite.h` - benchmark and optimization harness:
  - baseline vs optimized timing capture per hot path
  - context assembly cache hit/miss tracking
  - per-function timing KPI calculation
  - sustained-run synthetic memory stability signal
  - optimization summary notes and target gating (`<100ms/function`)
- `editor/tests/step504_test.cpp` - 12 tests covering:
  - baseline/optimized timing capture
  - no-regression optimization checks
  - cache effectiveness assertions
  - target threshold and repeatability checks
  - sustained-run stability + summary notes
- `editor/CMakeLists.txt` - `step504_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step504_test step503_test` - PASS
- `./editor/build-native/step504_test` - PASS (12/12)
- `./editor/build-native/step503_test` - PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/PerformanceOptimizationSuite.h` within header-size limit (`222` <= `600`)
- `editor/tests/step504_test.cpp` within test-file size guidance (`136` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 505: MCP Tool Documentation
**Status:** PASS (12/12 tests)

Implements structured MCP tool documentation generation with category indexing,
schema/example coverage, error-code metadata, and materialized markdown/json
resources for MCP client consumption.

**Files added:**
- `editor/src/MCPToolDocumentation.h` - documentation resource builder:
  - structured tool documentation model (description, input/output schema, examples, errors)
  - category index generation (`AST`, `Diagnostics`, `Workflow`, `Routing`, `Security`)
  - completeness validation checks
  - markdown + JSON renderers
  - resource materialization writer (`mcp_tool_docs.md`, `mcp_tool_docs.json`)
- `editor/tests/step505_test.cpp` - 12 tests covering:
  - per-tool documentation completeness
  - required category presence
  - schema/error metadata coverage
  - markdown/json export validity
  - writable/readable documentation resource output
  - invalid-doc rejection behavior
- `editor/CMakeLists.txt` - `step505_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step505_test step504_test` - PASS
- `./editor/build-native/step505_test` - PASS (12/12)
- `./editor/build-native/step504_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/MCPToolDocumentation.h` within header-size limit (`192` <= `600`)
- `editor/tests/step505_test.cpp` within test-file size guidance (`167` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 506: Edge Case Cleanup
**Status:** PASS (12/12 tests)

Implements explicit guardrails for high-risk edge cases with stable error codes
and non-crashing fallback behavior across file handling, workflow persistence,
dependency validation, language fallback, and MCP request validation.

**Files added:**
- `editor/src/EdgeCaseCleanup.h` - edge-case guard module:
  - empty/binary/oversized file detection
  - malformed annotation validation
  - circular dependency detection
  - concurrent workflow modification conflict detection
  - interrupted workflow save/resume helpers
  - unknown-language fallback to `plain_text`
  - malformed MCP request validation
- `editor/tests/step506_test.cpp` - 12 tests covering:
  - each edge-case error path + code/message semantics
  - workflow save/resume success/failure behavior
  - language fallback behavior
  - valid-path no-crash behavior
- `editor/CMakeLists.txt` - `step506_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step506_test step505_test` - PASS
- `./editor/build-native/step506_test` - PASS (12/12)
- `./editor/build-native/step505_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/EdgeCaseCleanup.h` within header-size limit (`139` <= `600`)
- `editor/tests/step506_test.cpp` within test-file size guidance (`150` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 507: Full Test Suite Regression Gate
**Status:** PASS (12/12 tests)

Implements regression quality gating for multi-run suite execution summaries,
including strict checks for all-pass behavior, flaky-test detection, duration budget,
and leak budget.

**Files added:**
- `editor/src/FullRegressionGate.h` - regression gate evaluator:
  - aggregates run outcomes across suite partitions
  - detects flaky suites from mixed pass/fail run states
  - enforces duration target (`<60s`) and memory-leak budget (`0`)
  - emits gate summary notes and failure/flaky suite lists
  - includes synthetic phase dataset helper representing full 245-506 coverage scale
- `editor/tests/step507_test.cpp` - 12 tests covering:
  - full-gate happy path across 3-run stability
  - >5000-test dataset semantics
  - flaky/failure/perf/leak failure-path detection
  - gate-conjunction logic and summary note generation
- `editor/CMakeLists.txt` - `step507_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step507_test step506_test` - PASS
- `./editor/build-native/step507_test` - PASS (12/12)
- `./editor/build-native/step506_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/FullRegressionGate.h` within header-size limit (`98` <= `600`)
- `editor/tests/step507_test.cpp` within test-file size guidance (`142` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 508: Sprint 25 Summary + Post-25 Readiness
**Status:** PASS (8/8 tests)

Implements Sprint 25 closure summary and Sprint 26 readiness signaling with
threshold-checked project metrics and post-25 training-data prerequisites.

**Files added:**
- `editor/src/Sprint25SummaryReadiness.h` - summary/readiness generator:
  - Sprint 25 headline metrics (languages, tools, annotation scale, steps/tests)
  - self-hosting coverage threshold signal
  - validated scenario count from Phase 25b integration
  - post-25 readiness signals:
    - event-stream data accumulated
    - workflow decisions logged
    - transpilation pairs logged
  - final Sprint 26 readiness gate synthesis
- `editor/tests/step508_test.cpp` - 8 tests covering:
  - all headline metric thresholds
  - self-hosting coverage threshold
  - 4-scenario validation signal
  - post-25 readiness signal states
  - final Sprint 26 readiness gate + notes
- `editor/CMakeLists.txt` - `step508_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step508_test step507_test` - PASS
- `./editor/build-native/step508_test` - PASS (8/8)
- `./editor/build-native/step507_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/Sprint25SummaryReadiness.h` within header-size limit (`68` <= `600`)
- `editor/tests/step508_test.cpp` within test-file size guidance (`94` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

**Phase 25c totals (504-508):**
- **Steps:** 5
- **Tests:** 56/56 passing
- **Headers added:** 5
  - `PerformanceOptimizationSuite.h`
  - `MCPToolDocumentation.h`
  - `EdgeCaseCleanup.h`
  - `FullRegressionGate.h`
  - `Sprint25SummaryReadiness.h`

**Sprint 25 totals (493-508):**
- **Steps completed:** 16
- **New tests in this sprint plan:** 180/180 passing

## Post-Sprint 25 Planning Update

Added the next multi-sprint roadmap as standalone plan documents in repo root,
following the same structure pattern as previous sprint plans:

- `sprint26_plan.md` — GUI/UI/UX foundations, docking reliability, visual identity,
  interaction-state clarity, and modifier-edge shortcut notation
- `sprint27_plan.md` — constrained constructive editing I (taskitem contracts,
  legal operation graph, symbol scope enforcement)
- `sprint28_plan.md` — constrained constructive editing II (legal-choice APIs,
  multi-language adapters)
- `sprint29_plan.md` — routing/context/cost discipline for constrained execution
- `sprint30_plan.md` — debugger/runtime observability I (breakpoints, stepping,
  stack traces, call stack, locals/watches)
- `sprint31_plan.md` — debugger/runtime observability II (memory reflection,
  advanced debug views)
- `sprint32_plan.md` — markdown intake and architect-side taskitem generation
  with confidence/ambiguity labeling
- `sprint33_plan.md` — productization, onboarding/value communication,
  release hardening and benchmark readiness

These plans intentionally distribute work across multiple sprints (rather than a
single sprint) to preserve implementation quality and maintainability.

### Step 509: Docking Reliability Audit + Deterministic Layout Rules
**Status:** PASS (12/12 tests)

Implements a headless docking reliability guard that audits panel/layout state,
repairs drift and hidden-critical-panel failures, and provides deterministic
startup/session-restore normalization.

**Files added:**
- `editor/src/DockingReliability.h` - docking reliability module:
  - invariant audit + repair pipeline (`auditAndRepair`)
  - deterministic reset behavior for startup/recovery paths
  - deterministic preset materialization (`deterministicStateFromPreset`)
  - session restore reconciliation (`reconcileFromSession`)
  - canonical layout fingerprinting for stability checks
  - explicit issue code taxonomy (`D001`-`D008`)
- `editor/tests/step509_test.cpp` - 12 tests covering:
  - stable baseline (no false positives)
  - ratio clamping and center-width safety floor
  - hidden editor recovery
  - editor dock drift recovery
  - empty-session panel restoration
  - deterministic reset behavior and fingerprint stability
  - invalid focus recovery
  - missing editor insertion
  - center visibility enforcement
  - session reconciliation semantics
  - recovery issue-code emission

**Files modified:**
- `editor/CMakeLists.txt` - `step509_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step509_test` - PASS
- `./editor/build-native/step509_test` - PASS (12/12)
- `./editor/build-native/step508_test` - PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/DockingReliability.h` within header-size limit (`283` <= `600`)
- `editor/tests/step509_test.cpp` within test-file size guidance (`180` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 510: Panel Boundaries, Resizers, and Hit-Target Corrections
**Status:** PASS (12/12 tests)

Implements deterministic split-bar hit testing and geometry normalization to
reduce ambiguous click zones where content selection steals resize intent, and
to enforce reliable keyboard/mouse resize behavior.

**Files added:**
- `editor/src/PanelBoundaryReliability.h` - panel boundary reliability module:
  - resize-priority click resolution over overlapping content
  - panel minimum-size enforcement and split-bar thickness clamping
  - bounded keyboard-driven split-bar resizing with fine-step mode
  - proximity-based navigation ordering for panel-focus traversal
- `editor/tests/step510_test.cpp` - 12 tests covering:
  - resize-vs-content hit-target priority
  - miss-path behavior
  - panel width/height minimum clamping
  - split-bar thickness lower/upper clamp behavior
  - keyboard resize movement, bounds, and fine-step mode
  - proximity navigation order and normalization note emission

**Files modified:**
- `editor/CMakeLists.txt` - `step510_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step510_test` - PASS
- `./editor/build-native/step510_test` - PASS (12/12)
- `./editor/build-native/step509_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/PanelBoundaryReliability.h` within header-size limit (`140` <= `600`)
- `editor/tests/step510_test.cpp` within test-file size guidance (`146` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 511: Interaction State Model
**Status:** PASS (12/12 tests)

Implements a shared interaction-state model for hover/focus/active/pressed/
disabled with deterministic token-driven style resolution and WCAG AA contrast
checks for state transitions.

**Files added:**
- `editor/src/InteractionStateModel.h` - state model module:
  - precedence-based state resolver (`disabled > pressed > active > focus > hover > idle`)
  - token-driven style resolution for surface/text/border/focus ring
  - contrast-ratio computation with AA pass/fail signal per state
  - whole-state map resolver and failure extraction for audits
- `editor/tests/step511_test.cpp` - 12 tests covering:
  - state-precedence ordering correctness
  - focus-ring semantics
  - disabled/idle token application
  - all-state resolution completeness
  - AA contrast pass for default dark tokens
  - low-contrast failure detection
  - pressed-vs-active visual differentiation

**Files modified:**
- `editor/CMakeLists.txt` - `step511_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step511_test` - PASS
- `./editor/build-native/step511_test` - PASS (12/12)
- `./editor/build-native/step510_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/InteractionStateModel.h` within header-size limit (`166` <= `600`)
- `editor/tests/step511_test.cpp` within test-file size guidance (`142` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 512: Window Chrome + Hierarchy Pass
**Status:** PASS (12/12 tests)

Implements deterministic surface hierarchy and chrome/elevation semantics across
work surfaces, tool surfaces, overlays, modals, and notifications, including
validation of layering invariants and interaction blocking behavior.

**Files added:**
- `editor/src/WindowHierarchyModel.h` - hierarchy/chrome model:
  - role-based style + z-index synthesis
  - deterministic ordering by elevation/layer
  - hierarchy validation with explicit violation signals
  - normalization pass for modal semantics
  - modal/overlay interaction-blocking checks
- `editor/tests/step512_test.cpp` - 12 tests covering:
  - role z-index ordering (`work < tool < overlay < modal`)
  - modal-flag validation and normalization
  - deterministic z-index sorting
  - modal/overlay interaction blocking semantics
  - depth-sensitive z-index behavior
  - notification layering between overlay and modal
  - baseline valid hierarchy behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step512_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step512_test` - PASS
- `./editor/build-native/step512_test` - PASS (12/12)
- `./editor/build-native/step511_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/WindowHierarchyModel.h` within header-size limit (`142` <= `600`)
- `editor/tests/step512_test.cpp` within test-file size guidance (`153` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 513: Accessibility Baseline Pass
**Status:** PASS (12/12 tests)

Implements a baseline accessibility validation model covering keyboard-only
traversal, focus-ring visibility constraints, and contrast thresholds for text
and iconography.

**Files added:**
- `editor/src/AccessibilityBaseline.h` - accessibility baseline module:
  - keyboard traversal order generation from tab-order metadata
  - focus-ring style resolution with high-contrast mode support
  - focus-ring visibility checks (thickness/alpha/contrast)
  - aggregate accessibility validation (focusability, ring, text/icon contrast)
- `editor/tests/step513_test.cpp` - 12 tests covering:
  - traversal ordering and disabled/invisible-node filtering
  - focus-ring enabled/disabled and visibility semantics
  - positive baseline pass condition
  - failure-path detection for no focus targets, low text contrast,
    low icon contrast, and non-visible focus ring

**Files modified:**
- `editor/CMakeLists.txt` - `step513_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step513_test` - PASS
- `./editor/build-native/step513_test` - PASS (12/12)
- `./editor/build-native/step512_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/AccessibilityBaseline.h` within header-size limit (`99` <= `600`)
- `editor/tests/step513_test.cpp` within test-file size guidance (`138` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 514: Phase 26a Integration
**Status:** PASS (8/8 tests)

Integrates Steps 509-513 into a single phase gate verifying docking stability,
hit-target reliability, interaction-state correctness, hierarchy validity, and
accessibility baseline compliance with an explicit editor-flow-safe signal.

**Files added:**
- `editor/src/Phase26aIntegration.h` - phase integration gate:
  - consolidated execution of Step 509-513 reliability checks
  - per-domain pass/fail signals plus final phase pass synthesis
  - integration notes including explicit phase-pass marker
- `editor/tests/step514_test.cpp` - 8 integration tests covering:
  - each domain gate (docking, hit-targets, interaction, hierarchy, accessibility)
  - combined editor-flow-safe signal
  - final phase pass signal
  - pass-marker note emission

**Files modified:**
- `editor/CMakeLists.txt` - `step514_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step514_test` - PASS
- `./editor/build-native/step514_test` - PASS (8/8)
- `./editor/build-native/step513_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/Phase26aIntegration.h` within header-size limit (`100` <= `600`)
- `editor/tests/step514_test.cpp` within test-file size guidance (`91` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 515: Theme Token System v2
**Status:** PASS (12/12 tests)

Implements a semantic token system for Sprint 26 visual language with
black/stone layered surfaces, accent channels, spacing/radius/border/elevation
scales, derivation support, and AA contrast validation.

**Files added:**
- `editor/src/ThemeTokenSystemV2.h` - token system module:
  - semantic color token set (base/layers/text/accent/error)
  - contrast-variant derivation for adaptive readability
  - spacing scale generation (`xs`..`xl`)
  - contrast ratio + AA validation helpers
  - explicit purple-bias detection guard for default accent policy
- `editor/tests/step515_test.cpp` - 12 tests covering:
  - layered black/stone baseline semantics
  - spacing monotonicity and required keys
  - variant derivation effects on text/layer brightness
  - AA contrast checks for baseline tokens
  - purple-bias detection behavior
  - radius/border/elevation scale ordering

**Files modified:**
- `editor/CMakeLists.txt` - `step515_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step515_test` - PASS
- `./editor/build-native/step515_test` - PASS (12/12)
- `./editor/build-native/step514_test` - PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/ThemeTokenSystemV2.h` within header-size limit (`107` <= `600`)
- `editor/tests/step515_test.cpp` within test-file size guidance (`121` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 516: Signature Visual Identity Pack
**Status:** PASS (12/12 tests)

Implements Sprint 26’s signature visual identity primitives: black-surface
gradient depth treatment, blade-edge accent semantics for active surfaces, and
electric-blue logic-link styling with distance-aware fading and motion-aware
reveal timing.

**Files added:**
- `editor/src/SignatureVisualIdentity.h` - visual identity module:
  - elevated/base black-surface gradient presets
  - blade-edge accent model (edge, thickness, intensity, active state)
  - electric-blue logic-link style with highlight and distance fade behavior
  - reduced-motion aware reveal and stagger timing helpers
- `editor/tests/step516_test.cpp` - 12 tests covering:
  - gradient stop structure/order and elevated-depth brightness
  - active blade-edge thickness/intensity behavior
  - highlighted link width/glow behavior
  - distance-based logic-link alpha fade
  - electric-blue profile detection
  - reduced-motion reveal/stagger behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step516_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step516_test` - PASS
- `./editor/build-native/step516_test` - PASS (12/12)
- `./editor/build-native/step515_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/SignatureVisualIdentity.h` within header-size limit (`81` <= `600`)
- `editor/tests/step516_test.cpp` within test-file size guidance (`123` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 517: Control Library Restyle
**Status:** PASS (12/12 tests)

Implements shared control styling semantics for core widgets (buttons, toggles,
tabs, menus, dropdowns, checkboxes) with explicit pressed-depth signaling,
contrast thresholds, and DPI-stable geometry scaling.

**Files added:**
- `editor/src/ControlLibraryRestyle.h` - control style module:
  - per-control geometry/depth token synthesis
  - pressed-state depth signal validation helper
  - contrast validation for labels/icons
  - DPI stability checks and scaling clamps
- `editor/tests/step517_test.cpp` - 12 tests covering:
  - depth and pressed-signal behavior
  - control-specific radius/border expectations
  - contrast compliance
  - DPI scaling growth and clamp edges
  - all-control contrast regression sweep

**Files modified:**
- `editor/CMakeLists.txt` - `step517_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step517_test` - PASS
- `./editor/build-native/step517_test` - PASS (12/12)
- `./editor/build-native/step516_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ControlLibraryRestyle.h` within header-size limit (`83` <= `600`)
- `editor/tests/step517_test.cpp` within test-file size guidance (`127` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 518: Icon + Typography Contrast Harmonization
**Status:** PASS (12/12 tests)

Implements icon and typography harmonization for dark surfaces with density-aware
sizing, stroke-weight balancing, contrast thresholds, and panel rhythm scoring
to reduce visual drift between dense and sparse UI regions.

**Files added:**
- `editor/src/IconTypographyHarmonizer.h` - harmonization module:
  - density-aware typography profile synthesis
  - icon stroke/contrast profile synthesis
  - harmonization validity checks for text/icon contrast and sizing
  - rhythm scoring for cross-panel density consistency
- `editor/tests/step518_test.cpp` - 12 tests covering:
  - density scaling + clamp behavior
  - heading/body sizing hierarchy
  - text/icon contrast thresholds
  - harmonization pass/fail behavior
  - rhythm score behavior for consistent vs divergent panel densities

**Files modified:**
- `editor/CMakeLists.txt` - `step518_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step518_test` - PASS
- `./editor/build-native/step518_test` - PASS (12/12)
- `./editor/build-native/step517_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/IconTypographyHarmonizer.h` within header-size limit (`60` <= `600`)
- `editor/tests/step518_test.cpp` within test-file size guidance (`125` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 519: Modifier-Edge Shortcut Notation
**Status:** PASS (12/12 tests)

Implements compact modifier-edge shortcut notation with deterministic edge
mapping (`Ctrl`=left, `Shift`=right, `Super`=top, `Alt`=bottom), combined
modifier support, and stable text fallback labels for non-glyph environments.

**Files added:**
- `editor/src/ModifierEdgeNotation.h` - notation module:
  - modifier-to-edge mapping model
  - glyph construction with key normalization
  - edge-activity counting and compactness checks
  - fallback label generation with stable modifier ordering
- `editor/tests/step519_test.cpp` - 12 tests covering:
  - per-modifier edge mapping
  - combined-edge behavior
  - key normalization and invalid-key handling
  - fallback-label correctness and ordering
  - glyph-disabled fallback behavior
  - compactness and edge-count sanity cases

**Files modified:**
- `editor/CMakeLists.txt` - `step519_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step519_test` - PASS
- `./editor/build-native/step519_test` - PASS (12/12)
- `./editor/build-native/step518_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ModifierEdgeNotation.h` within header-size limit (`76` <= `600`)
- `editor/tests/step519_test.cpp` within test-file size guidance (`116` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 520: Completion Overlay UX Hardening
**Status:** PASS (12/12 tests)

Implements completion-overlay hardening for predictable non-intrusive behavior:
viewport-safe placement, dismissal intent persistence, semantic-trigger reopen
logic, and stable pointer/keyboard navigation semantics.

**Files added:**
- `editor/src/CompletionOverlayUXHardening.h` - overlay UX module:
  - cursor-relative placement with overflow-aware flip/clamp logic
  - dismissal-state persistence and semantic-trigger reopen policy
  - pointer selection clamping and empty-list handling
  - keyboard navigation with wrap behavior
- `editor/tests/step520_test.cpp` - 12 tests covering:
  - placement under normal and overflow conditions
  - dismissal/reopen behavior across trigger states
  - pointer selection bounds and zero-item handling
  - keyboard navigation wrap semantics

**Files modified:**
- `editor/CMakeLists.txt` - `step520_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step520_test` - PASS
- `./editor/build-native/step520_test` - PASS (12/12)
- `./editor/build-native/step519_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/CompletionOverlayUXHardening.h` within header-size limit (`87` <= `600`)
- `editor/tests/step520_test.cpp` within test-file size guidance (`123` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 521: Notification + Status Signaling Refresh
**Status:** PASS (12/12 tests)

Implements refreshed notification/status signaling semantics with consistent
info/warn/error/critical behavior, readability guarantees, duration policies,
and stack management rules including persistent critical signaling.

**Files added:**
- `editor/src/NotificationStatusRefresh.h` - notification signaling module:
  - severity-specific style and duration policy
  - message factory with policy-derived timing
  - expiration logic with dismissal override and persistent critical support
  - stack-capping policy retaining newest messages
  - readability checks for text/icon contrast
- `editor/tests/step521_test.cpp` - 12 tests covering:
  - duration progression by severity
  - persistent critical behavior
  - readability checks across levels
  - expire/no-expire timing boundaries
  - dismissed-message handling
  - stack truncation behavior and ordering

**Files modified:**
- `editor/CMakeLists.txt` - `step521_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step521_test` - PASS
- `./editor/build-native/step521_test` - PASS (12/12)
- `./editor/build-native/step520_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/NotificationStatusRefresh.h` within header-size limit (`72` <= `600`)
- `editor/tests/step521_test.cpp` within test-file size guidance (`133` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 522: Cross-Panel Consistency Sweep
**Status:** PASS (12/12 tests)

Implements a cross-panel consistency sweep enforcing shared spacing/elevation/
border conventions plus state-model and token-version uniformity, with drift
histograms and normalization helpers for rapid alignment.

**Files added:**
- `editor/src/CrossPanelConsistencySweep.h` - consistency sweep module:
  - baseline-vs-panel drift detection with tolerances
  - drift categorization and histogram reporting
  - normalization pass to align panel contracts to baseline
  - checked-panel accounting for sweep coverage
- `editor/tests/step522_test.cpp` - 12 tests covering:
  - aligned pass case
  - drift detection for spacing/elevation/border/version mismatches
  - normalization behavior for metrics and versions
  - histogram grouping behavior
  - tolerance boundary behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step522_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step522_test` - PASS
- `./editor/build-native/step522_test` - PASS (12/12)
- `./editor/build-native/step521_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/CrossPanelConsistencySweep.h` within header-size limit (`81` <= `600`)
- `editor/tests/step522_test.cpp` within test-file size guidance (`152` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 523: Sprint 26 Integration + Summary
**Status:** PASS (8/8 tests)

Implements the Sprint 26 closure gate by integrating Phase 26a reliability,
visual token + identity systems, control/typography harmonization,
modifier-edge shortcuts, overlay hardening, notification semantics, and
cross-panel consistency into a single summary signal.

**Files added:**
- `editor/src/Sprint26IntegrationSummary.h` - sprint integration summary module:
  - aggregates Step 514-522 validation signals
  - computes final Sprint 26 pass flag
  - emits Sprint completion summary notes
- `editor/tests/step523_test.cpp` - 8 tests covering:
  - phase and domain gate pass signals
  - final sprint pass synthesis
  - completion-note emission

**Files modified:**
- `editor/CMakeLists.txt` - `step523_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step523_test` - PASS
- `./editor/build-native/step523_test` - PASS (8/8)
- `./editor/build-native/step522_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/Sprint26IntegrationSummary.h` within header-size limit (`90` <= `600`)
- `editor/tests/step523_test.cpp` within test-file size guidance (`92` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

**Sprint 26 totals (509-523):**
- **Steps completed:** 15
- **New tests in this sprint plan:** 172/172 passing
- **Phase 26a (509-514):** 68/68 passing
- **Phase 26b (515-519):** 60/60 passing
- **Phase 26c (520-523):** 44/44 passing

## Sprint 26 End Refactor Pass (Architecture Compliance)

Performed a dedicated post-sprint refactor pass focused on maintainability and
architecture-rule alignment without behavior changes.

**Refactors applied:**
- `editor/src/CompletionOverlayUXHardening.h`
  - extracted shared helpers for clamping/index wrapping
  - removed duplicated inline clamp/wrap logic
- `editor/src/NotificationStatusRefresh.h`
  - factored repeated style construction into `makeStyle(...)`
  - reduced duplicated initializer literals
- `editor/src/CrossPanelConsistencySweep.h`
  - centralized drift recording in `recordDrift(...)`
  - reduced repeated pass/fail mutation logic

**Architecture compliance audit:**
- Header size gate: PASS (`<=600` across audited sprint files)
- `goto` usage in runtime path: PASS (none in Sprint 26 modules)
- Stale TODO markers in Sprint 26 modules: PASS
- Header-only pattern for Sprint 26 additions: PASS

**Refactor verification run:**
- `cmake --build editor/build-native --target step520_test step521_test step522_test step523_test` - PASS
- `./editor/build-native/step520_test` - PASS (12/12)
- `./editor/build-native/step521_test` - PASS (12/12)
- `./editor/build-native/step522_test` - PASS (12/12)
- `./editor/build-native/step523_test` - PASS (8/8)

### Step 524: Typed Taskitem Contract Schema
**Status:** PASS (12/12 tests)

Implements a strict typed taskitem contract schema for constrained execution
with required fields for allowed targets/ops/symbols, forbidden symbols, and
expected diagnostics deltas, rejecting under-specified taskitems.

**Files added:**
- `editor/src/TypedTaskitemContractSchema.h` - schema module:
  - JSON parse + required-field validation
  - semantic consistency rules (duplicate op detection, symbol conflict detection)
  - under-specification guards for constrained execution path
  - helper checks for operation/symbol legality
- `editor/tests/step524_test.cpp` - 12 tests covering:
  - valid contract acceptance
  - required-field rejection paths
  - duplicate-op and symbol-conflict rejection
  - diagnostics-delta under-specification rejection
  - operation/symbol allow/deny helper behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step524_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step524_test` - PASS
- `./editor/build-native/step524_test` - PASS (12/12)
- `./editor/build-native/step523_test` - PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/TypedTaskitemContractSchema.h` within header-size limit (`114` <= `600`)
- `editor/tests/step524_test.cpp` within test-file size guidance (`148` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 525: Legal Operation Graph
**Status:** PASS (12/12 tests)

Implements a legal-operation graph for language/node-kind pairs so constrained
editor tasks can validate whether requested refactors are structurally allowed
before execution.

**Files added:**
- `editor/src/LegalOperationGraph.h` - operation-eligibility graph module:
  - default per-language/per-node operation rules
  - rule registration and override support
  - deduplication of operation sets
  - query helpers for supported nodes and allowed operations
- `editor/tests/step525_test.cpp` - 12 tests covering:
  - default rule coverage across cpp/c/python/typescript/rust/go
  - unsupported language/node behavior
  - rule registration and override semantics
  - operation deduplication behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step525_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step525_test step524_test` - PASS
- `./editor/build-native/step525_test` - PASS (12/12)
- `./editor/build-native/step524_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/LegalOperationGraph.h` within header-size limit (`79` <= `600`)
- `editor/tests/step525_test.cpp` within test-file size guidance (`131` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 526: Symbol Scope Extractor
**Status:** PASS (12/12 tests)

Implements symbol-scope extraction for constrained taskitems by building a
language/node-aware in-scope candidate snapshot and explicitly rejecting
unresolved symbol requests before apply.

**Files added:**
- `editor/src/SymbolScopeExtractor.h` - scope extraction module:
  - default per-language/per-node symbol-kind rules
  - nearest-scope candidate selection for duplicate names
  - forbidden-symbol filtering
  - unresolved symbol detection and apply-eligibility check
- `editor/tests/step526_test.cpp` - 12 tests covering:
  - default filtering and nearest-scope selection behavior
  - unsupported language/node handling
  - unresolved symbol reporting and apply blocking
  - forbidden symbol exclusion behavior
  - dynamic rule registration/override behavior
  - empty symbol-table edge case

**Files modified:**
- `editor/CMakeLists.txt` - `step526_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step526_test step525_test` - PASS
- `./editor/build-native/step526_test` - PASS (12/12)
- `./editor/build-native/step525_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/SymbolScopeExtractor.h` within header-size limit (`102` <= `600`)
- `editor/tests/step526_test.cpp` within test-file size guidance (`158` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 527: Constraint Violation Diagnostics
**Status:** PASS (12/12 tests)

Adds structured constraint diagnostics for constrained execution failures,
including out-of-scope symbol usage, illegal operations, and contract-policy
violations with machine-readable retry/escalation payloads.

**Files added:**
- `editor/src/ConstraintViolationDiagnostics.h` - diagnostics module:
  - evaluates operation legality against legal-op graph + taskitem contract
  - evaluates symbol usage against extracted scope snapshot + contract policy
  - emits structured violation records with retryability metadata
  - computes deterministic recommended action (`proceed`/`retry`/`escalate`)
  - serializes diagnostics packet to JSON for retry/escalation routing
- `editor/tests/step527_test.cpp` - 12 tests covering:
  - clean-path packet behavior with no violations
  - contract-forbidden and graph-illegal operation violations
  - unsupported context violations
  - out-of-scope and forbidden/disallowed symbol violations
  - retry-only vs escalate action selection
  - violation deduplication and multi-violation aggregation
  - machine-readable JSON payload shape
  - empty symbol request edge case

**Files modified:**
- `editor/CMakeLists.txt` - `step527_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step527_test step526_test` - PASS
- `./editor/build-native/step527_test` - PASS (12/12)
- `./editor/build-native/step526_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ConstraintViolationDiagnostics.h` within header-size limit (`169` <= `600`)
- `editor/tests/step527_test.cpp` within test-file size guidance (`223` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 528: Phase 27a Integration
**Status:** PASS (8/8 tests)

Integrates the Phase 27a constrained pipeline so taskitems must pass typed
contract validation, symbol-scope extraction, legal operation checks, and
constraint diagnostics before execution is allowed.

**Files added:**
- `editor/src/ConstrainedTaskitemIntegration.h` - phase integration gate:
  - schema-first taskitem validation via Step 524 contract module
  - forbidden-aware scope snapshot extraction via Step 526
  - constraint diagnostics evaluation via Step 527
  - deterministic route/execute decision (`routed` + `executed`)
  - actionable schema-failure packet generation for under-constrained taskitems
- `editor/tests/step528_test.cpp` - 8 tests covering:
  - full constrained path success for valid taskitem
  - schema rejection for missing required constraint fields
  - schema rejection for missing diagnostics delta and duplicate allowed ops
  - illegal operation rejection with escalation diagnostics
  - out-of-scope symbol rejection with retry diagnostics
  - forbidden symbol and unsupported-context rejection paths

**Files modified:**
- `editor/CMakeLists.txt` - `step528_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step528_test step527_test` - PASS
- `./editor/build-native/step528_test` - PASS (8/8)
- `./editor/build-native/step527_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ConstrainedTaskitemIntegration.h` within header-size limit (`86` <= `600`)
- `editor/tests/step528_test.cpp` within test-file size guidance (`179` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

**Phase 27a totals (524-528):**
- **Steps completed:** 5
- **New tests in phase plan:** 56/56 passing
- **Constraint model + diagnostics + integration:** PASS

### Step 529: Pre-Apply Validation Gate
**Status:** PASS (12/12 tests)

Adds a deterministic pre-apply gate that validates candidate edits against
contract schema, allowed targets, legal operation graph, and in-scope symbol
snapshot before any mutation is allowed.

**Files added:**
- `editor/src/PreApplyValidationGate.h` - pre-apply validation module:
  - schema-first taskitem validation gate
  - forbidden-aware scope snapshot build for candidate edit context
  - operation/symbol constraint evaluation via structured diagnostics
  - explicit `allowed` and `mayMutateBuffer` decision flags
  - target-kind contract enforcement (`target_out_of_contract`)
- `editor/tests/step529_test.cpp` - 12 tests covering:
  - valid pre-apply allow path
  - illegal op, unsupported context, and out-of-scope symbol rejections
  - target contract mismatch rejection
  - forbidden symbol rejection
  - schema-failure rejection paths
  - retry vs escalate action semantics
  - multi-violation aggregation behavior
  - empty symbol-list edge case

**Files modified:**
- `editor/CMakeLists.txt` - `step529_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step529_test step528_test` - PASS
- `./editor/build-native/step529_test` - PASS (12/12)
- `./editor/build-native/step528_test` - PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/PreApplyValidationGate.h` within header-size limit (`130` <= `600`)
- `editor/tests/step529_test.cpp` within test-file size guidance (`202` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 530: Post-Apply Structural Gate
**Status:** PASS (12/12 tests)

Adds a deterministic post-apply structural validation gate that requires
successful reparse of touched regions and rejects unexpected symbol-graph drift
after candidate edits are applied.

**Files added:**
- `editor/src/PostApplyStructuralGate.h` - post-apply gate module:
  - validates region reparse completion
  - validates AST integrity via parse error count check
  - computes symbol additions/removals from before/after snapshots
  - enforces allowed symbol-drift policy
  - emits structured diagnostics for unexpected drift and integrity failures
- `editor/tests/step530_test.cpp` - 12 tests covering:
  - clean reparse/no-drift pass path
  - AST integrity and missing-reparse failure paths
  - unexpected add/remove symbol drift rejection
  - allowed drift pass behavior (add/remove)
  - duplicate-symbol dedup behavior
  - aggregated multi-failure reporting
  - empty graph edge case

**Files modified:**
- `editor/CMakeLists.txt` - `step530_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step530_test step529_test` - PASS
- `./editor/build-native/step530_test` - PASS (12/12)
- `./editor/build-native/step529_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/PostApplyStructuralGate.h` within header-size limit (`123` <= `600`)
- `editor/tests/step530_test.cpp` within test-file size guidance (`155` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 531: Contract Delta Checker
**Status:** PASS (12/12 tests)

Implements a contract delta checker that validates expected diagnostic
delta semantics and required post-condition symbols, blocking edits that violate
post-apply contract expectations.

**Files added:**
- `editor/src/ContractDeltaChecker.h` - contract delta validation module:
  - computes before/after diagnostic add/remove sets
  - verifies expected diagnostic adds/removes occurred
  - optionally enforces strict unexpected diagnostic drift rejection
  - verifies required post-condition symbols are present after apply
  - emits structured violation diagnostics and final pass/fail action
- `editor/tests/step531_test.cpp` - 12 tests covering:
  - expected add/remove pass path
  - missing expected add/remove failures
  - strict unexpected add/remove rejection behavior
  - non-strict drift tolerance behavior
  - required post-condition symbol presence checks
  - duplicate diagnostics normalization behavior
  - empty-delta edge case
  - aggregated multi-failure reporting and escalation action

**Files modified:**
- `editor/CMakeLists.txt` - `step531_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step531_test step530_test` - PASS
- `./editor/build-native/step531_test` - PASS (12/12)
- `./editor/build-native/step530_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ContractDeltaChecker.h` within header-size limit (`149` <= `600`)
- `editor/tests/step531_test.cpp` within test-file size guidance (`200` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 532: Retry/Escalation Protocol for Constraint Failures
**Status:** PASS (12/12 tests)

Implements deterministic retry/escalation behavior for constraint failures,
including automatic correction for retryable scope failures and full failure
packet escalation when non-retryable or unresolved conditions remain.

**Files added:**
- `editor/src/RetryEscalationProtocol.h` - retry/escalation module:
  - immediate pass-through for already-valid diagnostics
  - immediate escalation for non-retryable violations
  - deterministic repair pass for correctable retryable failures
  - retry-budget control with attempt history
  - machine-readable escalation packet with reason/final diagnostics/attempts
- `editor/tests/step532_test.cpp` - 12 tests covering:
  - resolved/no-retry path
  - immediate escalation for non-retryable and mixed failures
  - successful retry for correctable failures
  - retry-budget exhaustion behavior
  - escalation packet structure and attempt history
  - deterministic repeatability guarantees
  - zero-budget edge case
  - applied-fix metadata capture

**Files modified:**
- `editor/CMakeLists.txt` - `step532_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step532_test step531_test` - PASS
- `./editor/build-native/step532_test` - PASS (12/12)
- `./editor/build-native/step531_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/RetryEscalationProtocol.h` within header-size limit (`129` <= `600`)
- `editor/tests/step532_test.cpp` within test-file size guidance (`171` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 533: Sprint 27 Integration + Summary
**Status:** PASS (8/8 tests)

Completes Sprint 27 integration by synthesizing Step 524-532 readiness signals
into phase-level and sprint-level pass/fail outcomes with actionable summary
notes.

**Files added:**
- `editor/src/Sprint27IntegrationSummary.h` - sprint integration module:
  - aggregates all Phase 27a and 27b gate signals
  - computes phase pass flags and final Sprint 27 pass flag
  - emits failure-note diagnostics for missing gates
  - emits constrained-execution completion notes on full pass
- `editor/tests/step533_test.cpp` - 8 tests covering:
  - full sprint pass path
  - phase-specific failure blocking behavior
  - success and failure note emission
  - multi-failure aggregation
  - all-false boundary behavior
  - phase-status isolation on single gate failure

**Files modified:**
- `editor/CMakeLists.txt` - `step533_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step533_test step532_test` - PASS
- `./editor/build-native/step533_test` - PASS (8/8)
- `./editor/build-native/step532_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/Sprint27IntegrationSummary.h` within header-size limit (`71` <= `600`)
- `editor/tests/step533_test.cpp` within test-file size guidance (`135` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

**Sprint 27 totals (524-533):**
- **Steps completed:** 10
- **New tests in this sprint plan:** 112/112 passing
- **Phase 27a (524-528):** 56/56 passing
- **Phase 27b (529-533):** 56/56 passing

## Sprint 27 End Refactor Pass (Architecture Compliance)

Performed a dedicated post-sprint refactor pass to reduce duplication and
re-validate architecture constraints without changing runtime behavior.

**Refactors applied:**
- `editor/src/ConstraintSchemaFailurePacket.h`
  - extracted shared helper `makeUnderConstrainedContractPacket(...)`
  - centralized schema-failure diagnostics payload construction
- `editor/src/ConstrainedTaskitemIntegration.h`
  - replaced inline schema-failure packet builder with shared helper usage
  - removed duplicate private schema-failure implementation
- `editor/src/PreApplyValidationGate.h`
  - replaced inline schema-failure packet builder with shared helper usage
  - removed duplicate private schema-failure implementation

**Architecture compliance audit:**
- Header size gate (`<=600` lines): PASS across Sprint 27 modules
  - largest audited header: `editor/src/ConstraintViolationDiagnostics.h` (`169` lines)
- `goto` usage in Sprint 27 runtime path: PASS (none)
- Stale TODO markers in Sprint 27 modules: PASS (none)
- Header-only pattern for Sprint 27 additions: PASS

**Refactor verification run:**
- `cmake --build editor/build-native --target step524_test step525_test step526_test step527_test step528_test step529_test step530_test step531_test step532_test step533_test` - PASS
- `./editor/build-native/step524_test` - PASS (12/12)
- `./editor/build-native/step525_test` - PASS (12/12)
- `./editor/build-native/step526_test` - PASS (12/12)
- `./editor/build-native/step527_test` - PASS (12/12)
- `./editor/build-native/step528_test` - PASS (8/8)
- `./editor/build-native/step529_test` - PASS (12/12)
- `./editor/build-native/step530_test` - PASS (12/12)
- `./editor/build-native/step531_test` - PASS (12/12)
- `./editor/build-native/step532_test` - PASS (12/12)
- `./editor/build-native/step533_test` - PASS (8/8)

### Step 534: Operation Selector API
**Status:** PASS (12/12 tests)

Implements legal-operation selection for constrained editing by exposing only
operations legal for the active language/node context and taskitem contract,
ranked by predicted constraint-success signals.

**Files added:**
- `editor/src/OperationSelectorAPI.h` - operation selection module:
  - intersects legal-op graph results with taskitem allowed ops
  - ranks candidates using risk/context/preference heuristics
  - emits candidate reasons for selector explainability
  - deterministic score + tie-break ordering
- `editor/tests/step534_test.cpp` - 12 tests covering:
  - supported/unsupported context behavior
  - contract op filtering behavior
  - score sorting and tie-break behavior
  - preferred-op boost behavior
  - scope-context weighting behavior
  - low-risk/high-risk scoring behavior
  - empty allowed-op and unknown-language edge cases

**Files modified:**
- `editor/CMakeLists.txt` - `step534_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step534_test step533_test` - PASS
- `./editor/build-native/step534_test` - PASS (12/12)
- `./editor/build-native/step533_test` - PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/OperationSelectorAPI.h` within header-size limit (`88` <= `600`)
- `editor/tests/step534_test.cpp` within test-file size guidance (`208` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 535: Symbol Selector API
**Status:** PASS (12/12 tests)

Implements constrained symbol selection by exposing only in-scope symbols
allowed by contract policy, enriched with typed role/category metadata and
category-based filtering.

**Files added:**
- `editor/src/SymbolSelectorAPI.h` - symbol selection module:
  - filters scope snapshot symbols through contract allow/forbid policy
  - maps symbol kinds to selector categories (`function/type/field/module/constant`)
  - supports category filter queries
  - deterministic sorting and dedupe behavior
- `editor/tests/step535_test.cpp` - 12 tests covering:
  - supported/unsupported scope handling
  - forbidden/contract-disallowed symbol filtering
  - each category filter behavior
  - multi-category filtering
  - scope-depth sort behavior
  - duplicate symbol-name dedup behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step535_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step535_test step534_test` - PASS
- `./editor/build-native/step535_test` - PASS (12/12)
- `./editor/build-native/step534_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/SymbolSelectorAPI.h` within header-size limit (`81` <= `600`)
- `editor/tests/step535_test.cpp` within test-file size guidance (`163` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 536: Argument Shape Validator
**Status:** PASS (12/12 tests)

Implements argument-shape validation for constrained operations, enforcing
operation-specific required fields, enum constraints, forbidden keys, and
contract legality for operation/symbol pairs.

**Files added:**
- `editor/src/ArgumentShapeValidator.h` - argument shape validation module:
  - operation/symbol contract allow checks
  - per-operation required argument schemas (`rename/update/insert/extract/delete`)
  - enum validation for constrained argument fields
  - semantic no-op guard for rename operations
  - malformed-argument object checks and forbidden-key enforcement
- `editor/tests/step536_test.cpp` - 12 tests covering:
  - valid rename path
  - required field failures per operation
  - enum validation failures
  - delete confirm-type requirement
  - disallowed op/symbol failure paths
  - malformed args (non-object) edge case

**Files modified:**
- `editor/CMakeLists.txt` - `step536_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step536_test step535_test` - PASS
- `./editor/build-native/step536_test` - PASS (12/12)
- `./editor/build-native/step535_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ArgumentShapeValidator.h` within header-size limit (`113` <= `600`)
- `editor/tests/step536_test.cpp` within test-file size guidance (`144` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 537: Constrained Execution Telemetry
**Status:** PASS (12/12 tests)

Implements constrained execution telemetry to track candidate breadth,
selection paths, rejection reasons, and token-usage deltas versus unconstrained
baselines.

**Files added:**
- `editor/src/ConstrainedExecutionTelemetry.h` - telemetry module:
  - records per-taskitem constrained execution events
  - aggregates success/failure counts and candidate breadth averages
  - builds rejection-reason and selected-operation histograms
  - computes constrained vs baseline token totals/savings/ratio
- `editor/tests/step537_test.cpp` - 12 tests covering:
  - empty-summary behavior
  - event recording/counting behavior
  - success/failure aggregate behavior
  - average candidate breadth computation
  - token savings and ratio computation
  - rejection and operation histogram behavior
  - baseline-zero and negative-savings edge cases
  - mixed-event aggregate behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step537_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step537_test step536_test` - PASS
- `./editor/build-native/step537_test` - PASS (12/12)
- `./editor/build-native/step536_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ConstrainedExecutionTelemetry.h` within header-size limit (`75` <= `600`)
- `editor/tests/step537_test.cpp` within test-file size guidance (`175` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 538: Phase 28a Integration
**Status:** PASS (8/8 tests)

Integrates legal-choice operation/symbol menus, argument-shape validation, and
telemetry into a single constrained execution surface for agent-facing editing.

**Files added:**
- `editor/src/Phase28aIntegration.h` - Phase 28a integration gate:
  - builds operation and symbol legal-choice menus
  - enforces selection membership (operation + symbol must come from legal menus)
  - validates selected argument shape before execution
  - records per-attempt telemetry for success and rejection paths
  - returns integrated execution result + aggregate telemetry summary
- `editor/tests/step538_test.cpp` - 8 tests covering:
  - valid legal-choice execution path
  - illegal operation/symbol menu rejection paths
  - argument-shape rejection path
  - menu unavailability failure path
  - telemetry candidate-breadth, token-savings, and rejection histogram behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step538_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step538_test step537_test` - PASS
- `./editor/build-native/step538_test` - PASS (8/8)
- `./editor/build-native/step537_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/Phase28aIntegration.h` within header-size limit (`118` <= `600`)
- `editor/tests/step538_test.cpp` within test-file size guidance (`154` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

**Phase 28a totals (534-538):**
- **Steps completed:** 5
- **New tests in phase plan:** 56/56 passing
- **Legal-choice API + validation + telemetry integration:** PASS

### Step 539: C/C++ Constructive Edit Adapter
**Status:** PASS (12/12 tests)

Implements a C/C++ constructive edit adapter with construct-aware legal
operation mapping and header/source boundary safety checks.

**Files added:**
- `editor/src/CppConstructiveEditAdapter.h` - C/C++ adapter module:
  - legal operation mappings for declarations/definitions/includes/macros/classes/members
  - language support for both `c` and `cpp`
  - header/source boundary guards for sensitive operations
  - structured diagnostics for unsupported constructs/languages and blocked ops
- `editor/tests/step539_test.cpp` - 12 tests covering:
  - legal operation paths across declarations, definitions, includes, macros, members
  - boundary-safe rejections (header/source violations)
  - unsupported construct/language behavior
  - C language compatibility path

**Files modified:**
- `editor/CMakeLists.txt` - `step539_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step539_test step538_test` - PASS
- `./editor/build-native/step539_test` - PASS (12/12)
- `./editor/build-native/step538_test` - PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/CppConstructiveEditAdapter.h` within header-size limit (`90` <= `600`)
- `editor/tests/step539_test.cpp` within test-file size guidance (`126` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 540: Python/TypeScript Constructive Edit Adapter
**Status:** PASS (12/12 tests)

Implements Python/TypeScript constructive edit adaptation with construct-aware
legal operation mappings and module-level side-effect protection guards.

**Files added:**
- `editor/src/PythonTypeScriptConstructiveEditAdapter.h` - Python/TypeScript adapter module:
  - legal operation mappings for functions/classes/imports/signatures/module statements
  - shared support for both `python` and `typescript`
  - module side-effect risk guardrails for reorder/delete/inline-sensitive paths
  - structured diagnostics for unsupported constructs/languages and illegal ops
- `editor/tests/step540_test.cpp` - 12 tests covering:
  - legal operation paths across Python/TypeScript constructs
  - side-effect risk blocking behavior
  - illegal operation rejections for constrained constructs
  - unsupported construct/language behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step540_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step540_test step539_test` - PASS
- `./editor/build-native/step540_test` - PASS (12/12)
- `./editor/build-native/step539_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/PythonTypeScriptConstructiveEditAdapter.h` within header-size limit (`84` <= `600`)
- `editor/tests/step540_test.cpp` within test-file size guidance (`126` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 541: Rust/Go Constructive Edit Adapter
**Status:** PASS (12/12 tests)

Implements Rust/Go constructive edit adaptation with ownership/lifetime and
concurrency-sensitive guardrails for high-risk transform paths.

**Files added:**
- `editor/src/RustGoConstructiveEditAdapter.h` - Rust/Go adapter module:
  - legal operation mappings for functions/structs/impl/import/channel/borrow regions
  - shared support for both `rust` and `go`
  - borrow/lifetime safety guards for Rust-sensitive operations
  - concurrency safety guards for Go-sensitive operations
  - structured diagnostics for unsupported constructs/languages and illegal ops
- `editor/tests/step541_test.cpp` - 12 tests covering:
  - legal operation paths across Rust and Go constructs
  - ownership/lifetime guard behavior in Rust
  - concurrency guard behavior in Go
  - illegal operation rejections
  - unsupported construct/language behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step541_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step541_test step540_test` - PASS
- `./editor/build-native/step541_test` - PASS (12/12)
- `./editor/build-native/step540_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/RustGoConstructiveEditAdapter.h` within header-size limit (`91` <= `600`)
- `editor/tests/step541_test.cpp` within test-file size guidance (`127` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 542: Cross-Language Consistency Gate
**Status:** PASS (12/12 tests)

Implements a cross-language consistency gate that normalizes adapter outcomes,
standardizes failure categories, and applies consistent fallback behavior across
C/C++, Python/TypeScript, and Rust/Go adapters.

**Files added:**
- `editor/src/CrossLanguageConsistencyGate.h` - consistency gate module:
  - routes language-specific requests to the appropriate constructive adapter
  - normalizes adapter diagnostics into canonical failure categories
  - assigns standardized fallback actions (`readonly`, `suggest_legal_ops`, `escalate`, `retry`)
  - preserves raw adapter diagnostics for downstream handling
- `editor/tests/step542_test.cpp` - 12 tests covering:
  - allowed-path normalization across languages
  - safety-guard failure normalization for side-effect/lifetime/concurrency blocks
  - illegal-operation and unsupported construct/language normalization
  - fallback action consistency by canonical failure type
  - diagnostic preservation behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step542_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step542_test step541_test` - PASS
- `./editor/build-native/step542_test` - PASS (12/12)
- `./editor/build-native/step541_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/CrossLanguageConsistencyGate.h` within header-size limit (`108` <= `600`)
- `editor/tests/step542_test.cpp` within test-file size guidance (`132` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 543: Sprint 28 Integration + Summary
**Status:** PASS (8/8 tests)

Completes Sprint 28 integration by synthesizing Step 534-542 readiness signals
into phase-level and sprint-level pass/fail outcomes with closure notes.

**Files added:**
- `editor/src/Sprint28IntegrationSummary.h` - sprint integration module:
  - aggregates all Phase 28a and 28b gate signals
  - computes phase pass flags and final Sprint 28 pass flag
  - emits failure-note diagnostics for missing gates
  - emits completion notes on full sprint pass
- `editor/tests/step543_test.cpp` - 8 tests covering:
  - full sprint pass path
  - phase-specific failure blocking behavior
  - success/failure note emission
  - multi-failure aggregation
  - all-false boundary behavior
  - phase-status isolation on single Phase 28b failure

**Files modified:**
- `editor/CMakeLists.txt` - `step543_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step543_test step542_test` - PASS
- `./editor/build-native/step543_test` - PASS (8/8)
- `./editor/build-native/step542_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/Sprint28IntegrationSummary.h` within header-size limit (`70` <= `600`)
- `editor/tests/step543_test.cpp` within test-file size guidance (`128` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

**Sprint 28 totals (534-543):**
- **Steps completed:** 10
- **New tests in this sprint plan:** 112/112 passing
- **Phase 28a (534-538):** 56/56 passing
- **Phase 28b (539-543):** 56/56 passing

## Sprint 28 End Refactor Pass (Architecture Compliance)

Performed a dedicated post-sprint refactor pass to reduce adapter duplication and
re-validate architecture constraints without behavior changes.

**Refactors applied:**
- `editor/src/AdapterOperationUtil.h`
  - added shared adapter operation helper `adapterHasOperation(...)`
  - removed repeated inline membership helpers across language adapters
- `editor/src/CppConstructiveEditAdapter.h`
  - switched operation legality checks to shared helper
  - removed local duplicate membership implementation
- `editor/src/PythonTypeScriptConstructiveEditAdapter.h`
  - switched operation legality checks to shared helper
  - removed local duplicate membership implementation
- `editor/src/RustGoConstructiveEditAdapter.h`
  - switched operation legality checks to shared helper
  - removed local duplicate membership implementation

**Architecture compliance audit:**
- Header size gate (`<=600` lines): PASS across Sprint 28 modules
  - largest audited header: `editor/src/Phase28aIntegration.h` (`118` lines)
- `goto` usage in Sprint 28 runtime path: PASS (none)
- Stale TODO markers in Sprint 28 modules: PASS (none)
- Header-only pattern for Sprint 28 additions: PASS

**Refactor verification run:**
- `cmake --build editor/build-native --target step534_test step535_test step536_test step537_test step538_test step539_test step540_test step541_test step542_test step543_test` - PASS
- `./editor/build-native/step534_test` - PASS (12/12)
- `./editor/build-native/step535_test` - PASS (12/12)
- `./editor/build-native/step536_test` - PASS (12/12)
- `./editor/build-native/step537_test` - PASS (12/12)
- `./editor/build-native/step538_test` - PASS (8/8)
- `./editor/build-native/step539_test` - PASS (12/12)
- `./editor/build-native/step540_test` - PASS (12/12)
- `./editor/build-native/step541_test` - PASS (12/12)
- `./editor/build-native/step542_test` - PASS (12/12)
- `./editor/build-native/step543_test` - PASS (8/8)

### Step 544: Constrained Routing Ruleset Extension
**Status:** PASS (12/12 tests)

Implements constrained-mode routing policy extensions with deterministic
template promotion at high legal-choice confidence and explicit precedence for
post-apply failure escalation.

**Files added:**
- `editor/src/ConstrainedRoutingRulesetExtension.h` - constrained routing module:
  - constrained/non-constrained mode branching
  - failure-first escalation for unstable recent outcomes
  - deterministic template promotion for high-confidence legal-choice paths
  - constrained-worker routing for moderate confidence
  - low-confidence fallback to general worker
- `editor/tests/step544_test.cpp` - 12 tests covering:
  - constrained-mode and non-constrained routing behavior
  - escalation precedence behavior
  - template promotion thresholds
  - constrained-worker thresholds
  - confidence propagation behavior
  - rationale-tag emission behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step544_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step544_test step543_test` - PASS
- `./editor/build-native/step544_test` - PASS (12/12)
- `./editor/build-native/step543_test` - PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/ConstrainedRoutingRulesetExtension.h` within header-size limit (`60` <= `600`)
- `editor/tests/step544_test.cpp` within test-file size guidance (`126` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 545: Context Bundle Minimizer
**Status:** PASS (12/12 tests)

Implements minimal-sufficient context bundling for constrained taskitems,
retaining contract-relevant context while dropping unrelated project and
dependency noise for narrow operations.

**Files added:**
- `editor/src/ContextBundleMinimizer.h` - context minimization module:
  - seeds relevance keys from contract targets/symbols/ops
  - filters file/project/dependency context by relevance and structural heuristics
  - applies stricter pruning for narrow operations
  - returns explicit kept vs removed context sets
  - deterministic dedupe normalization for outputs
- `editor/tests/step545_test.cpp` - 12 tests covering:
  - narrow-mode relevance retention/drop behavior
  - non-narrow structural keep behavior
  - symbol/op-driven relevance behavior
  - dedupe behavior for kept/removed sets
  - empty-input edge case
  - irrelevant-note dropping behavior
  - dependency relevance by symbol-match behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step545_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step545_test step544_test` - PASS
- `./editor/build-native/step545_test` - PASS (12/12)
- `./editor/build-native/step544_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ContextBundleMinimizer.h` within header-size limit (`88` <= `600`)
- `editor/tests/step545_test.cpp` within test-file size guidance (`169` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 546: Confidence Calibration for Constrained Paths
**Status:** PASS (12/12 tests)

Implements confidence calibration for constrained routing by combining base
legal-choice confidence with historical success/failure rates and explicit
penalties for repeated post-apply failures.

**Files added:**
- `editor/src/ConstrainedConfidenceCalibrator.h` - confidence calibration module:
  - route scoring for deterministic/constrained/general policy paths
  - success-rate boosts and post-apply failure penalties
  - extra decay for repeated post-apply failures
  - preferred-route bias with post-bias score clamping
  - best-route selection and adjusted-confidence emission
- `editor/tests/step546_test.cpp` - 12 tests covering:
  - neutral/no-history behavior
  - success/failure rate calibration effects
  - repeated-failure decay behavior
  - preferred-route bias behavior
  - clamping behavior at score bounds
  - recommended-route and adjusted-confidence consistency
  - route score coverage across all policy paths

**Files modified:**
- `editor/CMakeLists.txt` - `step546_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step546_test step545_test` - PASS
- `./editor/build-native/step546_test` - PASS (12/12)
- `./editor/build-native/step545_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ConstrainedConfidenceCalibrator.h` within header-size limit (`97` <= `600`)
- `editor/tests/step546_test.cpp` within test-file size guidance (`143` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 547: Cost Policy Guard
**Status:** PASS (12/12 tests)

Implements cost policy enforcement for constrained orchestration with per-step
and per-workflow token ceilings, rationale requirements, and escalation approval
controls for overage execution.

**Files added:**
- `editor/src/CostPolicyGuard.h` - cost policy enforcement module:
  - step/workflow ceiling checks
  - violation capture for exceeded ceilings
  - rationale-required flow for policy overages
  - escalation-approval gate for over-budget execution
  - deterministic action outputs (`allow`/`request_rationale`/`escalate`)
- `editor/tests/step547_test.cpp` - 12 tests covering:
  - within-limit allow behavior
  - step/workflow/both overage violation behavior
  - rationale and escalation approval behavior
  - equal-to-ceiling boundary behavior
  - zero-ceiling disabled-check behavior
  - negative-token edge behavior
  - no-escalation behavior when within limits

**Files modified:**
- `editor/CMakeLists.txt` - `step547_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step547_test step546_test` - PASS
- `./editor/build-native/step547_test` - PASS (12/12)
- `./editor/build-native/step546_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/CostPolicyGuard.h` within header-size limit (`60` <= `600`)
- `editor/tests/step547_test.cpp` within test-file size guidance (`132` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 548: Phase 29a Integration
**Status:** PASS (8/8 tests)

Integrates Phase 29a routing intelligence, context minimization, confidence
calibration, and cost policy enforcement into a unified constrained orchestration
decision flow.

**Files added:**
- `editor/src/Phase29aIntegration.h` - Phase 29a integration gate:
  - composes Step 544 routing policy, Step 545 context minimizer,
    Step 546 confidence calibration, and Step 547 cost guard
  - emits integrated pass/fail signal with structured error reasons
  - guards template routing when calibrated confidence is too low
  - enforces cost-policy and escalation outcomes in final decision
- `editor/tests/step548_test.cpp` - 8 tests covering:
  - pass-path with reduced context and cost compliance
  - cost-policy block behavior
  - routing-escalation block behavior
  - narrow-context minimization behavior
  - low-calibrated-confidence template block behavior
  - calibration route-shift behavior
  - approved overage path behavior
  - non-narrow context tolerance behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step548_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step548_test step547_test` - PASS
- `./editor/build-native/step548_test` - PASS (8/8)
- `./editor/build-native/step547_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/Phase29aIntegration.h` within header-size limit (`69` <= `600`)
- `editor/tests/step548_test.cpp` within test-file size guidance (`132` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

**Phase 29a totals (544-548):**
- **Steps completed:** 5
- **New tests in phase plan:** 56/56 passing
- **Routing/context/cost optimization integration:** PASS

### Step 549: Suggestion Engine for Cost Reductions
**Status:** PASS (12/12 tests)

Implements cost-reduction suggestion generation for constrained orchestration,
including template substitution, batching, and context narrowing opportunities
with priority and savings estimates.

**Files added:**
- `editor/src/CostReductionSuggestionEngine.h` - cost suggestion module:
  - emits template substitution recommendations when deterministic paths exist
  - emits batching recommendations for compatible multi-op workloads
  - emits context narrowing recommendations for wide symbol candidate sets
  - computes estimated token savings and priority by budget pressure
  - deterministic sorting by priority and estimated savings
- `editor/tests/step549_test.cpp` - 12 tests covering:
  - suggestion emission behavior per optimization path
  - no-suggestion baseline behavior
  - over-budget priority amplification behavior
  - savings scaling behavior by op/symbol breadth
  - path eligibility thresholds for batching/context narrowing
  - ordering/tiebreak behavior
  - rationale presence requirements

**Files modified:**
- `editor/CMakeLists.txt` - `step549_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step549_test step548_test` - PASS
- `./editor/build-native/step549_test` - PASS (12/12)
- `./editor/build-native/step548_test` - PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/CostReductionSuggestionEngine.h` within header-size limit (`68` <= `600`)
- `editor/tests/step549_test.cpp` within test-file size guidance (`146` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 550: Worker Efficiency Dashboard Data Model
**Status:** PASS (12/12 tests)

Implements a worker-efficiency dashboard aggregation model covering completion,
cost, rejection, and latency metrics by worker and task class.

**Files added:**
- `editor/src/WorkerEfficiencyDashboardModel.h` - dashboard model module:
  - records per-run worker/task telemetry
  - aggregates metrics by worker and by task class
  - computes success/failure counts and rates
  - computes total/average tokens, latency, and rejection counts
- `editor/tests/step550_test.cpp` - 12 tests covering:
  - empty snapshot behavior
  - worker/task grouping behavior
  - success/failure and success-rate computations
  - average tokens/latency/rejections computations
  - aggregate totals behavior
  - cross-worker task-class aggregation behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step550_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step550_test step549_test` - PASS
- `./editor/build-native/step550_test` - PASS (12/12)
- `./editor/build-native/step549_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/WorkerEfficiencyDashboardModel.h` within header-size limit (`71` <= `600`)
- `editor/tests/step550_test.cpp` within test-file size guidance (`170` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 551: Review-Gate Policy Refinement
**Status:** PASS (12/12 tests)

Implements refined review-gate policy logic to reduce unnecessary human
escalations while preserving hard safety/security/policy protections.

**Files added:**
- `editor/src/ReviewGatePolicyRefinement.h` - review-gate policy module:
  - policy-violation hard block precedence
  - safety/security escalation precedence
  - repeated post-apply failure escalation trigger
  - high-confidence deterministic auto-approval path
  - moderate-confidence auto-approve-with-audit path
  - low-confidence/high-cost escalation behavior
- `editor/tests/step551_test.cpp` - 12 tests covering:
  - policy/safety/security precedence behavior
  - failure-streak escalation behavior
  - auto-approve and audit-path behavior
  - low-confidence/high-cost escalation behavior
  - precedence override behavior across mixed conditions
  - threshold boundary behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step551_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step551_test step550_test` - PASS
- `./editor/build-native/step551_test` - PASS (12/12)
- `./editor/build-native/step550_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ReviewGatePolicyRefinement.h` within header-size limit (`70` <= `600`)
- `editor/tests/step551_test.cpp` within test-file size guidance (`132` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 552: Cost vs Quality Regression Suite
**Status:** PASS (12/12 tests)

Implements a cost-vs-quality regression suite to enforce that optimization
policy tightening preserves quality, pass rate, and safety while meeting token
reduction targets.

**Files added:**
- `editor/src/CostQualityRegressionSuite.h` - regression suite module:
  - evaluates optimized runs against absolute quality/pass/safety floors
  - enforces minimum token reduction targets
  - detects relative quality/pass regressions vs baseline
  - reports detailed failure codes and token reduction deltas
- `editor/tests/step552_test.cpp` - 12 tests covering:
  - passing baseline/optimized scenario
  - token/quality/pass/safety floor failures
  - relative regression detection behavior
  - multi-failure reporting behavior
  - threshold boundary behavior
  - token reduction reporting behavior
  - negative reduction failure behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step552_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step552_test step551_test` - PASS
- `./editor/build-native/step552_test` - PASS (12/12)
- `./editor/build-native/step551_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/CostQualityRegressionSuite.h` within header-size limit (`60` <= `600`)
- `editor/tests/step552_test.cpp` within test-file size guidance (`167` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 553: Sprint 29 Integration + Summary
**Status:** PASS (8/8 tests)

Completes Sprint 29 integration by synthesizing Step 544-552 readiness signals
into phase-level and sprint-level outcomes with closure diagnostics.

**Files added:**
- `editor/src/Sprint29IntegrationSummary.h` - sprint integration module:
  - aggregates all Phase 29a and 29b gate signals
  - computes phase pass flags and final Sprint 29 pass flag
  - emits failure-note diagnostics for missing gates
  - emits completion notes on full sprint pass
- `editor/tests/step553_test.cpp` - 8 tests covering:
  - full sprint pass path
  - phase-specific failure blocking behavior
  - success/failure note emission
  - multi-failure aggregation
  - all-false boundary behavior
  - phase-status isolation on single Phase 29b failure

**Files modified:**
- `editor/CMakeLists.txt` - `step553_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step553_test step552_test` - PASS
- `./editor/build-native/step553_test` - PASS (8/8)
- `./editor/build-native/step552_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/Sprint29IntegrationSummary.h` within header-size limit (`70` <= `600`)
- `editor/tests/step553_test.cpp` within test-file size guidance (`128` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

**Sprint 29 totals (544-553):**
- **Steps completed:** 10
- **New tests in this sprint plan:** 112/112 passing
- **Phase 29a (544-548):** 56/56 passing
- **Phase 29b (549-553):** 56/56 passing

## Sprint 29 End Refactor Pass (Architecture Compliance)

Performed a dedicated post-sprint refactor pass to reduce policy-guard
duplication and re-validate architecture constraints without behavior changes.

**Refactors applied:**
- `editor/src/PolicyDecisionUtil.h`
  - added shared helpers for policy decision transitions
  - centralized common escalation/action state assignment logic
- `editor/src/ReviewGatePolicyRefinement.h`
  - switched repeated escalation decision branches to shared helper usage
  - removed duplicated inline escalation state mutation
- `editor/src/CostPolicyGuard.h`
  - switched repeated action/allow/escalation transitions to shared helper usage
  - removed duplicated inline action-state assignments

**Architecture compliance audit:**
- Header size gate (`<=600` lines): PASS across Sprint 29 modules
  - largest audited header: `editor/src/ConstrainedConfidenceCalibrator.h` (`97` lines)
- `goto` usage in Sprint 29 runtime path: PASS (none)
- Stale TODO markers in Sprint 29 modules: PASS (none)
- Header-only pattern for Sprint 29 additions: PASS

**Refactor verification run:**
- `cmake --build editor/build-native --target step544_test step545_test step546_test step547_test step548_test step549_test step550_test step551_test step552_test step553_test` - PASS
- `./editor/build-native/step544_test` - PASS (12/12)
- `./editor/build-native/step545_test` - PASS (12/12)
- `./editor/build-native/step546_test` - PASS (12/12)
- `./editor/build-native/step547_test` - PASS (12/12)
- `./editor/build-native/step548_test` - PASS (8/8)
- `./editor/build-native/step549_test` - PASS (12/12)
- `./editor/build-native/step550_test` - PASS (12/12)
- `./editor/build-native/step551_test` - PASS (12/12)
- `./editor/build-native/step552_test` - PASS (12/12)
- `./editor/build-native/step553_test` - PASS (8/8)

### Step 554: Debug Session Model
**Status:** PASS (12/12 tests)

Implements the core debug session lifecycle model with start/attach/pause/
resume/stop transitions and multi-target per-buffer context support.

**Files added:**
- `editor/src/DebugSessionModel.h` - debug session control-plane model:
  - session lifecycle transitions (`start/attach/pause/resume/stop`)
  - multi-target session state with per-target buffer/executable context
  - current-target switching
  - per-buffer target lookup for editor mapping
- `editor/tests/step554_test.cpp` - 12 tests covering:
  - lifecycle transition behavior and invalid transition guards
  - attach semantics for inactive/active sessions
  - multi-target session behavior
  - target switching behavior
  - per-buffer target mapping behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step554_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step554_test step553_test` - PASS
- `./editor/build-native/step554_test` - PASS (12/12)
- `./editor/build-native/step553_test` - PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/DebugSessionModel.h` within header-size limit (`106` <= `600`)
- `editor/tests/step554_test.cpp` within test-file size guidance (`140` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 555: Breakpoint System
**Status:** PASS (12/12 tests)

Implements breakpoint management for line, conditional, and hit-count
breakpoints, including enable/disable persistence and line reconciliation across
file edits.

**Files added:**
- `editor/src/BreakpointSystem.h` - breakpoint subsystem:
  - line, conditional, and hit-count breakpoint creation
  - enable/disable/remove operations
  - runtime break-decision evaluation (condition + hit-count semantics)
  - file-edit reconciliation with line-shift/clamp behavior
- `editor/tests/step555_test.cpp` - 12 tests covering:
  - breakpoint creation variants and invalid input guardrails
  - enable/disable persistence behavior
  - simple/conditional/hit-count break evaluation behavior
  - file-edit reconciliation behavior
  - removal behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step555_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step555_test step554_test` - PASS
- `./editor/build-native/step555_test` - PASS (12/12)
- `./editor/build-native/step554_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/BreakpointSystem.h` within header-size limit (`118` <= `600`)
- `editor/tests/step555_test.cpp` within test-file size guidance (`150` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 556: Step Engine
**Status:** PASS (12/12 tests)

Implements deterministic stepping semantics for debug control with `step_into`,
`step_over`, `step_out`, and `continue` command mapping and explicit runtime
state transitions.

**Files added:**
- `editor/src/StepEngine.h` - stepping control-plane module:
  - deterministic command execution mapping
  - state transitions across paused/running/completed states
  - stack depth and instruction pointer progression semantics
  - command validity guards for unknown/invalid lifecycle states
- `editor/tests/step556_test.cpp` - 12 tests covering:
  - per-command transition behavior
  - root/ nested `step_out` behavior
  - deterministic continue behavior
  - unknown command behavior
  - completion-state command guard behavior
  - command tracking and progression monotonicity

**Files modified:**
- `editor/CMakeLists.txt` - `step556_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step556_test step555_test` - PASS
- `./editor/build-native/step556_test` - PASS (12/12)
- `./editor/build-native/step555_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/StepEngine.h` within header-size limit (`135` <= `600`)
- `editor/tests/step556_test.cpp` within test-file size guidance (`154` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 557: Exception + Stack Trace Capture
**Status:** PASS (12/12 tests)

Implements structured exception event capture with validated stack traces and
editor navigation anchors for each stack frame.

**Files added:**
- `editor/src/ExceptionStackTraceCapture.h` - exception capture module:
  - structured exception event model (`type/message/thread/frames`)
  - frame validation and normalization
  - navigation anchor generation (`file:line`) for editor linking
  - explicit error reporting for invalid/missing payload fields
- `editor/tests/step557_test.cpp` - 12 tests covering:
  - valid capture behavior
  - missing required field behavior
  - invalid frame validation behavior
  - navigation anchor generation
  - mixed valid/invalid frame behavior
  - event field persistence behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step557_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step557_test step556_test` - PASS
- `./editor/build-native/step557_test` - PASS (12/12)
- `./editor/build-native/step556_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ExceptionStackTraceCapture.h` within header-size limit (`71` <= `600`)
- `editor/tests/step557_test.cpp` within test-file size guidance (`149` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 558: Phase 30a Integration
**Status:** PASS (8/8 tests)

Integrates debug session lifecycle, breakpoint handling, stepping, and exception
capture into a complete Phase 30a debug control-plane cycle.

**Files added:**
- `editor/src/Phase30aIntegration.h` - Phase 30a integration gate:
  - composes Step 554 session lifecycle, Step 555 breakpoint system,
    Step 556 step engine, and Step 557 exception capture
  - executes breakpoint-hit to step-command to optional exception flow
  - emits integrated pass/fail state with explicit error reasons
- `editor/tests/step558_test.cpp` - 8 tests covering:
  - full cycle pass paths (with and without exception)
  - breakpoint add/hit failure paths
  - step command failure path
  - session start failure path
  - step-result propagation and exception anchor linkage

**Files modified:**
- `editor/CMakeLists.txt` - `step558_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step558_test step557_test` - PASS
- `./editor/build-native/step558_test` - PASS (8/8)
- `./editor/build-native/step557_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/Phase30aIntegration.h` within header-size limit (`80` <= `600`)
- `editor/tests/step558_test.cpp` within test-file size guidance (`117` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

**Phase 30a totals (554-558):**
- **Steps completed:** 5
- **New tests in phase plan:** 56/56 passing
- **Debug control-plane integration:** PASS

### Step 559: Call Stack and Frame Inspector
**Status:** PASS (12/12 tests)

Implements call stack frame inspection with frame navigation and variable scope
mapping for runtime inspection surfaces.

**Files added:**
- `editor/src/CallStackFrameInspector.h` - frame inspector module:
  - frame selection by index
  - payload validation for frame navigation metadata
  - selected-frame scope extraction
  - variable name listing helper for panel rendering
- `editor/tests/step559_test.cpp` - 12 tests covering:
  - top/middle/bottom frame navigation
  - invalid/empty stack guard behavior
  - invalid frame payload guard behavior
  - variable scope mapping and exposure behavior
  - selected-index reporting behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step559_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step559_test step558_test` - PASS
- `./editor/build-native/step559_test` - PASS (12/12)
- `./editor/build-native/step558_test` - PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/CallStackFrameInspector.h` within header-size limit (`58` <= `600`)
- `editor/tests/step559_test.cpp` within test-file size guidance (`146` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 560: Locals + Watches Panel Data Model
**Status:** PASS (12/12 tests)

Implements runtime locals and watch-expression state modeling with type-aware
locals display fields and watch lifecycle management.

**Files added:**
- `editor/src/LocalsWatchesPanelModel.h` - locals/watches data model:
  - type-aware local variable entries
  - watch add/remove/enable-disable lifecycle
  - watch evaluation value/status tracking
  - active-watch filtering and index-rebuild safety
- `editor/tests/step560_test.cpp` - 12 tests covering:
  - locals persistence/type behavior
  - watch lifecycle behavior and input validation
  - watch enable/disable behavior
  - evaluation update behavior
  - active-watch filtering behavior
  - index rebuild behavior after watch removal

**Files modified:**
- `editor/CMakeLists.txt` - `step560_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step560_test step559_test` - PASS
- `./editor/build-native/step560_test` - PASS (12/12)
- `./editor/build-native/step559_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/LocalsWatchesPanelModel.h` within header-size limit (`90` <= `600`)
- `editor/tests/step560_test.cpp` within test-file size guidance (`138` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 561: Trace Timeline Model
**Status:** PASS (12/12 tests)

Implements a runtime trace timeline data model for ordered pause/step/exception
state transitions with session/type/window query support.

**Files added:**
- `editor/src/TraceTimelineModel.h` - trace timeline module:
  - event append with payload validation
  - deterministic timeline ordering
  - query helpers by event type, session, and time window
  - support for mixed runtime event categories
- `editor/tests/step561_test.cpp` - 12 tests covering:
  - event validation behavior
  - ordering behavior
  - type/session/window query behavior
  - timestamp tie-break behavior
  - detail payload preservation
  - mixed-event support behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step561_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step561_test step560_test` - PASS
- `./editor/build-native/step561_test` - PASS (12/12)
- `./editor/build-native/step560_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/TraceTimelineModel.h` within header-size limit (`58` <= `600`)
- `editor/tests/step561_test.cpp` within test-file size guidance (`144` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 562: Workflow-Aware Debug Hooks
**Status:** PASS (12/12 tests)

Implements workflow-aware debug hooks that bind debug sessions to active
workflow items and diagnostic IDs for coordinated runtime/debug context.

**Files added:**
- `editor/src/WorkflowAwareDebugHooks.h` - workflow debug hook model:
  - workflow-item to debug-session attachment lifecycle
  - active/inactive binding state toggles
  - diagnostic linkage management per workflow item
  - active-binding and diagnostic query helpers
- `editor/tests/step562_test.cpp` - 12 tests covering:
  - attach/detach lifecycle behavior
  - duplicate/invalid attach guards
  - active state toggle behavior
  - diagnostic add/duplicate/unknown behavior
  - active-binding filtering behavior
  - unknown diagnostic query behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step562_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step562_test step561_test` - PASS
- `./editor/build-native/step562_test` - PASS (12/12)
- `./editor/build-native/step561_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/WorkflowAwareDebugHooks.h` within header-size limit (`77` <= `600`)
- `editor/tests/step562_test.cpp` within test-file size guidance (`133` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 563: Sprint 30 Integration + Summary
**Status:** PASS (8/8 tests)

Completes Sprint 30 integration by synthesizing Step 554-562 readiness signals
into phase-level and sprint-level outcomes with closure diagnostics.

**Files added:**
- `editor/src/Sprint30IntegrationSummary.h` - sprint integration module:
  - aggregates all Phase 30a and 30b gate signals
  - computes phase pass flags and final Sprint 30 pass flag
  - emits failure-note diagnostics for missing gates
  - emits completion notes on full sprint pass
- `editor/tests/step563_test.cpp` - 8 tests covering:
  - full sprint pass path
  - phase-specific failure blocking behavior
  - success/failure note emission
  - multi-failure aggregation
  - all-false boundary behavior
  - phase-status isolation on single Phase 30b failure

**Files modified:**
- `editor/CMakeLists.txt` - `step563_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step563_test step562_test` - PASS
- `./editor/build-native/step563_test` - PASS (8/8)
- `./editor/build-native/step562_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/Sprint30IntegrationSummary.h` within header-size limit (`70` <= `600`)
- `editor/tests/step563_test.cpp` within test-file size guidance (`128` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

**Sprint 30 totals (554-563):**
- **Steps completed:** 10
- **New tests in this sprint plan:** 112/112 passing
- **Phase 30a (554-558):** 56/56 passing
- **Phase 30b (559-563):** 56/56 passing

### Sprint 30 End Refactor Pass (Architecture Compliance)
**Status:** PASS (112/112 tests revalidated)

Performed a focused post-sprint refactor against `ARCHITECTURE.md` constraints to
reduce validation duplication across Sprint 30 debug modules while preserving
header-only design and deterministic behavior.

**Files added:**
- `editor/src/DebugValidationUtil.h` - shared validation helpers:
  - required debug/session/workflow id presence check
  - positive source-line guard used by breakpoint/stack validation paths

**Files modified:**
- `editor/src/DebugSessionModel.h` - uses shared id validation utility for target/session lifecycle checks
- `editor/src/BreakpointSystem.h` - uses shared line-validation utility for breakpoint insertion guards
- `editor/src/ExceptionStackTraceCapture.h` - uses shared line-validation utility for frame validation

**Verification run:**
- `cmake --build editor/build-native --target step554_test step555_test step556_test step557_test step558_test step559_test step560_test step561_test step562_test step563_test` - PASS
- `./editor/build-native/step554_test` - PASS (12/12)
- `./editor/build-native/step555_test` - PASS (12/12)
- `./editor/build-native/step556_test` - PASS (12/12)
- `./editor/build-native/step557_test` - PASS (12/12)
- `./editor/build-native/step558_test` - PASS (8/8)
- `./editor/build-native/step559_test` - PASS (12/12)
- `./editor/build-native/step560_test` - PASS (12/12)
- `./editor/build-native/step561_test` - PASS (12/12)
- `./editor/build-native/step562_test` - PASS (12/12)
- `./editor/build-native/step563_test` - PASS (8/8)

**Architecture gate check:**
- `editor/src/DebugValidationUtil.h` within header-size limit (`12` <= `600`)
- `editor/src/DebugSessionModel.h` within header-size limit (`108` <= `600`)
- `editor/src/BreakpointSystem.h` within header-size limit (`120` <= `600`)
- `editor/src/ExceptionStackTraceCapture.h` within header-size limit (`73` <= `600`)
- Shared validation centralization reduces duplicated guard logic and keeps module boundaries aligned with `ARCHITECTURE.md`

### Step 564: Memory Snapshot Model
**Status:** PASS (12/12 tests)

Implements a structured memory snapshot schema with region/reference capture,
validation, and query helpers for stack/heap/global memory reflection.

**Files added:**
- `editor/src/MemorySnapshotModel.h` - memory snapshot module:
  - snapshot/session/thread identity schema
  - typed memory region records (stack/heap/global)
  - pointer/reference record schema
  - region insertion validation + deterministic ordering
  - address-to-region and outgoing-reference query helpers
- `editor/tests/step564_test.cpp` - 12 tests covering:
  - snapshot identity validation behavior
  - region add/duplicate/bounds guard behavior
  - deterministic region ordering behavior
  - inclusive/exclusive address lookup behavior
  - reference add/validation behavior
  - outgoing reference filtering behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step564_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step564_test step563_test` - PASS
- `./editor/build-native/step564_test` - PASS (12/12)
- `./editor/build-native/step563_test` - PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/MemorySnapshotModel.h` within header-size limit (`149` <= `600`)
- `editor/tests/step564_test.cpp` within test-file size guidance (`179` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 565: Memory Inspector UI Model
**Status:** PASS (12/12 tests)

Implements a memory inspector UI data model with typed primitive/composite nodes,
visible-row flattening, and pointer-to-region correlation against memory snapshots.

**Files added:**
- `editor/src/MemoryInspectorUiModel.h` - memory inspector UI model:
  - typed value-node schema (primitive + container/struct)
  - primitive/composite creation guards
  - composite child-attachment behavior
  - visible row flattening with expansion + row limit handling
  - pointer text parsing and region-resolution against `MemorySnapshot`
- `editor/tests/step565_test.cpp` - 12 tests covering:
  - primitive/composite creation success/failure behavior
  - child attachment behavior and parent-kind guards
  - visible-row expansion/limit/depth behavior
  - type renderer naming behavior
  - pointer-region resolve success and failure behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step565_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step565_test step564_test` - PASS
- `./editor/build-native/step565_test` - PASS (12/12)
- `./editor/build-native/step564_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/MemoryInspectorUiModel.h` within header-size limit (`194` <= `600`)
- `editor/tests/step565_test.cpp` within test-file size guidance (`188` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 566: Allocation/Ownership Trace Hooks
**Status:** PASS (12/12 tests)

Implements allocation lifecycle and ownership transfer trace hooks with
deterministic timeline ordering, active-allocation tracking, and session filters.

**Files added:**
- `editor/src/AllocationOwnershipTraceHooks.h` - allocation trace module:
  - allocation/transfer/free event schema
  - allocation guardrails (ids, address, size, owner, duplicate-active checks)
  - ownership transfer and release lifecycle handling
  - active-allocation owner lookup/count helpers
  - deterministic timeline ordering and per-session filtering
- `editor/tests/step566_test.cpp` - 12 tests covering:
  - allocation success/failure validation behavior
  - duplicate-active guard behavior
  - ownership transfer success and failure behavior
  - free lifecycle success/failure behavior
  - timeline ordering behavior
  - session timeline filtering behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step566_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step566_test step565_test` - PASS
- `./editor/build-native/step566_test` - PASS (12/12)
- `./editor/build-native/step565_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/AllocationOwnershipTraceHooks.h` within header-size limit (`150` <= `600`)
- `editor/tests/step566_test.cpp` within test-file size guidance (`160` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 567: Leak/Corruption Signal Bridge
**Status:** PASS (12/12 tests)

Implements a bridge that converts runtime leak/corruption signals into
session-scoped diagnostics and pane routing for memory debug surfaces.

**Files added:**
- `editor/src/LeakCorruptionSignalBridge.h` - signal bridge module:
  - leak/corruption signal schema and severity mapping
  - signal ingestion validation and duplicate guard
  - diagnostic emission with pane routing (`memory-leaks` / `memory-corruption`)
  - session/allocation filtering helpers
  - session-level highest-severity aggregation
- `editor/tests/step567_test.cpp` - 12 tests covering:
  - signal ingestion success and validation failures
  - duplicate signal rejection behavior
  - pane/severity mapping behavior
  - session/allocation filtering behavior
  - highest-severity aggregation behavior
  - deterministic timestamp ordering behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step567_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step567_test step566_test` - PASS
- `./editor/build-native/step567_test` - PASS (12/12)
- `./editor/build-native/step566_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/LeakCorruptionSignalBridge.h` within header-size limit (`154` <= `600`)
- `editor/tests/step567_test.cpp` within test-file size guidance (`176` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 568: Phase 31a Integration
**Status:** PASS (8/8 tests)

Integrates Steps 564-567 into one memory-reflection control flow that validates
snapshot capture, UI inspection, allocation tracing, signal diagnostics, and
stack/debug correlation gating.

**Files added:**
- `editor/src/Phase31aIntegration.h` - Phase 31a integration module:
  - end-to-end memory reflection cycle orchestration
  - debug session + snapshot bootstrap and region/reference population
  - inspector node/row build and pointer-region resolution
  - allocation/ownership trace lifecycle validation
  - leak/corruption signal bridge ingestion and severity capture
  - stack frame correlation checks against active debug context
- `editor/tests/step568_test.cpp` - 8 tests covering:
  - full happy-path cycle behavior
  - debug start/snapshot failure handling
  - pointer resolution failure handling
  - allocation transfer failure handling
  - signal bridge failure handling
  - stack correlation failure handling
  - corruption severity escalation behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step568_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step568_test step567_test` - PASS
- `./editor/build-native/step568_test` - PASS (8/8)
- `./editor/build-native/step567_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/Phase31aIntegration.h` within header-size limit (`156` <= `600`)
- `editor/tests/step568_test.cpp` within test-file size guidance (`116` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

**Phase 31a totals (564-568):**
- **Steps completed:** 5
- **New tests in this phase plan:** 56/56 passing

### Step 569: Disassembly + Register View Baseline
**Status:** PASS (12/12 tests)

Implements a baseline disassembly/register inspection model for low-level runtime
debug views with deterministic instruction ordering and register state access.

**Files added:**
- `editor/src/DisassemblyRegisterViewModel.h` - disassembly/register module:
  - instruction schema with current-IP annotation state
  - instruction-set validation and deterministic address ordering
  - current instruction pointer selection and window slicing
  - register set/update/query helpers with stable sorted output
- `editor/tests/step569_test.cpp` - 12 tests covering:
  - instruction load success and validation failures
  - deterministic instruction ordering behavior
  - current-IP selection behavior and unknown-address rejection
  - centered disassembly window behavior
  - register set/query validation behavior
  - register ordering behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step569_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step569_test step568_test` - PASS
- `./editor/build-native/step569_test` - PASS (12/12)
- `./editor/build-native/step568_test` - PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/DisassemblyRegisterViewModel.h` within header-size limit (`125` <= `600`)
- `editor/tests/step569_test.cpp` within test-file size guidance (`156` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 570: Watch Expression Evaluator Hardening
**Status:** PASS (12/12 tests)

Implements a hardened watch expression evaluator with explicit unsafe-pattern
rejection, token/operation guardrails, deterministic timeout behavior, and
numeric/identifier evaluation.

**Files added:**
- `editor/src/WatchExpressionEvaluatorHardening.h` - evaluator hardening module:
  - expression/token safety validation
  - unsafe pattern detection (`system`, `exec`, braces, etc.)
  - token count, operation count, and timeout-budget enforcement
  - deterministic numeric/identifier expression evaluation with precedence
  - explicit error taxonomy for parser/runtime failures
- `editor/tests/step570_test.cpp` - 12 tests covering:
  - literal and scoped identifier evaluation behavior
  - unsafe/unknown/invalid token rejection behavior
  - token/operation/timeout guard behavior
  - divide-by-zero and operand mismatch handling
  - deterministic repeated evaluation behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step570_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step570_test step569_test` - PASS
- `./editor/build-native/step570_test` - PASS (12/12)
- `./editor/build-native/step569_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/WatchExpressionEvaluatorHardening.h` within header-size limit (`210` <= `600`)
- `editor/tests/step570_test.cpp` within test-file size guidance (`142` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 571: Time-Travel Debug Event Buffer
**Status:** PASS (12/12 tests)

Implements a bounded debug event history buffer for short-horizon time-travel
replay with deterministic eviction and session/window query helpers.

**Files added:**
- `editor/src/TimeTravelDebugEventBuffer.h` - replay buffer module:
  - bounded append lifecycle with overflow eviction tracking
  - event validation (ids, type, timestamp, instruction pointer)
  - latest-event and replay-window extraction helpers
  - per-session event filtering
  - duplicate event-id rejection behavior
- `editor/tests/step571_test.cpp` - 12 tests covering:
  - append success and validation failures
  - duplicate-id rejection behavior
  - bounded eviction and dropped-count behavior
  - latest/replay window query behavior
  - per-session filtering behavior
  - zero-capacity normalization behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step571_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step571_test step570_test` - PASS
- `./editor/build-native/step571_test` - PASS (12/12)
- `./editor/build-native/step570_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/TimeTravelDebugEventBuffer.h` within header-size limit (`86` <= `600`)
- `editor/tests/step571_test.cpp` within test-file size guidance (`174` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 572: Performance Probe Overlay
**Status:** PASS (12/12 tests)

Implements a pause-time performance probe overlay model that aggregates per-line
runtime costs and exposes hotspot-ranked metadata for debug surfaces.

**Files added:**
- `editor/src/PerformanceProbeOverlay.h` - probe overlay module:
  - per-sample validation and duplicate-id guards
  - per-line duration/count aggregation by session + buffer
  - hotspot ranking with threshold and limit controls
  - pause-line neighborhood overlay window extraction
- `editor/tests/step572_test.cpp` - 12 tests covering:
  - sample ingestion success/failure behavior
  - duplicate sample rejection behavior
  - aggregate grouping/filtering behavior
  - hotspot ranking/threshold behavior
  - pause overlay window boundary behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step572_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step572_test step571_test` - PASS
- `./editor/build-native/step572_test` - PASS (12/12)
- `./editor/build-native/step571_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/PerformanceProbeOverlay.h` within header-size limit (`110` <= `600`)
- `editor/tests/step572_test.cpp` within test-file size guidance (`175` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 573: Sprint 31 Integration + Summary
**Status:** PASS (8/8 tests)

Completes Sprint 31 integration by aggregating Phase 31a and 31b readiness signals
into phase/sprint pass states with closure diagnostics.

**Files added:**
- `editor/src/Sprint31IntegrationSummary.h` - sprint integration module:
  - aggregates Phase 31a and 31b step readiness signals
  - computes phase pass flags and overall Sprint 31 pass flag
  - emits per-step failure notes and phase blocked notes
  - emits sprint readiness notes on full pass
- `editor/tests/step573_test.cpp` - 8 tests covering:
  - full sprint pass path
  - phase-specific failure blocking behavior
  - success/failure note emission behavior
  - multi-failure aggregation behavior
  - all-false boundary behavior
  - phase isolation behavior on single 31b failure

**Files modified:**
- `editor/CMakeLists.txt` - `step573_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step573_test step572_test` - PASS
- `./editor/build-native/step573_test` - PASS (8/8)
- `./editor/build-native/step572_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/Sprint31IntegrationSummary.h` within header-size limit (`70` <= `600`)
- `editor/tests/step573_test.cpp` within test-file size guidance (`128` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

**Phase 31b totals (569-573):**
- **Steps completed:** 5
- **New tests in this phase plan:** 56/56 passing

**Sprint 31 totals (564-573):**
- **Steps completed:** 10
- **New tests in this sprint plan:** 112/112 passing
- **Phase 31a (564-568):** 56/56 passing
- **Phase 31b (569-573):** 56/56 passing

### Sprint 31 End Refactor Pass (Architecture Compliance)
**Status:** PASS (112/112 tests revalidated)

Performed a focused post-sprint architecture pass to standardize primitive
validation paths across Sprint 31 memory/observability modules while keeping
all components header-only and within file-size constraints.

**Files modified:**
- `editor/src/DebugValidationUtil.h` - expanded shared validation helpers:
  - three-id presence check overload
  - shared non-zero `uint64` guard helper
- `editor/src/MemorySnapshotModel.h` - switched address/size/reference guards to shared non-zero utility
- `editor/src/AllocationOwnershipTraceHooks.h` - switched allocation address/size guards to shared non-zero utility
- `editor/src/LeakCorruptionSignalBridge.h` - switched timestamp guard to shared non-zero utility and standardized allocation-id presence validation path
- `editor/src/TimeTravelDebugEventBuffer.h` - switched timestamp/instruction-pointer guards to shared non-zero utility
- `editor/src/PerformanceProbeOverlay.h` - switched duration/timestamp guards to shared non-zero utility

**Verification run:**
- `cmake --build editor/build-native --target step564_test step565_test step566_test step567_test step568_test step569_test step570_test step571_test step572_test step573_test` - PASS
- `./editor/build-native/step564_test` - PASS (12/12)
- `./editor/build-native/step565_test` - PASS (12/12)
- `./editor/build-native/step566_test` - PASS (12/12)
- `./editor/build-native/step567_test` - PASS (12/12)
- `./editor/build-native/step568_test` - PASS (8/8)
- `./editor/build-native/step569_test` - PASS (12/12)
- `./editor/build-native/step570_test` - PASS (12/12)
- `./editor/build-native/step571_test` - PASS (12/12)
- `./editor/build-native/step572_test` - PASS (12/12)
- `./editor/build-native/step573_test` - PASS (8/8)

**Architecture gate check:**
- `editor/src/DebugValidationUtil.h` within header-size limit (`23` <= `600`)
- `editor/src/MemorySnapshotModel.h` within header-size limit (`149` <= `600`)
- `editor/src/AllocationOwnershipTraceHooks.h` within header-size limit (`150` <= `600`)
- `editor/src/LeakCorruptionSignalBridge.h` within header-size limit (`154` <= `600`)
- `editor/src/TimeTravelDebugEventBuffer.h` within header-size limit (`86` <= `600`)
- `editor/src/PerformanceProbeOverlay.h` within header-size limit (`110` <= `600`)
- Shared validation centralization reduces repeated primitive guards and keeps module boundaries aligned with `ARCHITECTURE.md`

### Step 574: Markdown Spec Parser
**Status:** PASS (12/12 tests)

Implements markdown intake parsing for architect specs, including section
extraction, categorized requirement bullets, and source-anchor traceability.

**Files added:**
- `editor/src/MarkdownSpecParser.h` - markdown parser module:
  - top-level section extraction (`##` headings)
  - section anchor generation for traceability
  - categorized bullet extraction (goals/constraints/dependencies/acceptance)
  - source line capture per extracted requirement
  - whitespace normalization and guard handling
- `editor/tests/step574_test.cpp` - 12 tests covering:
  - section/category extraction behavior
  - anchor/traceability field behavior
  - orphan bullet handling behavior
  - empty/no-section failure behavior
  - bullet text trimming behavior
  - subsection-heading handling behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step574_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step574_test step573_test` - PASS
- `./editor/build-native/step574_test` - PASS (12/12)
- `./editor/build-native/step573_test` - PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/MarkdownSpecParser.h` within header-size limit (`142` <= `600`)
- `editor/tests/step574_test.cpp` within test-file size guidance (`178` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 575: Requirement Normalization and Conflict Detection
**Status:** PASS (12/12 tests)

Implements requirement normalization and contradiction detection across parsed
intake requirements, including ambiguity flags and source traceability.

**Files added:**
- `editor/src/RequirementNormalizationConflictDetector.h` - normalization module:
  - category-to-record normalization (goal/constraint/dependency/acceptance)
  - normalized text canonicalization for consistent downstream processing
  - requirement id generation and trace field preservation
  - ambiguity token detection for architect review signaling
  - constraint contradiction detection via overlap + negation analysis
- `editor/tests/step575_test.cpp` - 12 tests covering:
  - normalization across all categories
  - canonical text behavior and id stability behavior
  - traceability field preservation behavior
  - ambiguity flag behavior
  - conflict/no-conflict scenarios and conflict metadata behavior
  - empty input failure behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step575_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step575_test step574_test` - PASS
- `./editor/build-native/step575_test` - PASS (12/12)
- `./editor/build-native/step574_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/RequirementNormalizationConflictDetector.h` within header-size limit (`166` <= `600`)
- `editor/tests/step575_test.cpp` within test-file size guidance (`200` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 576: Scope and Milestone Decomposer
**Status:** PASS (12/12 tests)

Implements requirement decomposition into milestone/workstream planning units,
including requirement mapping and uncertainty scoring for architect intake.

**Files added:**
- `editor/src/ScopeMilestoneDecomposer.h` - decomposition module:
  - requirement-kind partitioning into milestone buckets
  - workstream generation with requirement-id linkage
  - architect review workstream generation from detected conflicts
  - per-workstream/per-milestone/overall uncertainty scoring
  - milestone pruning and uncertainty bound handling
- `editor/tests/step576_test.cpp` - 12 tests covering:
  - decomposition success/failure behavior
  - milestone/workstream routing behavior
  - requirement-id mapping behavior
  - conflict-driven review stream behavior
  - uncertainty scoring behavior and bounds
  - empty milestone pruning behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step576_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step576_test step575_test` - PASS
- `./editor/build-native/step576_test` - PASS (12/12)
- `./editor/build-native/step575_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ScopeMilestoneDecomposer.h` within header-size limit (`147` <= `600`)
- `editor/tests/step576_test.cpp` within test-file size guidance (`201` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 577: Architect Review Surface for Intake
**Status:** PASS (12/12 tests)

Implements an architect review control surface for extracted constraints,
enabling approve/modify/reject decisions before task generation.

**Files added:**
- `editor/src/ArchitectReviewSurface.h` - review surface module:
  - constraint review item loading from normalized requirements
  - approve/modify/reject decision lifecycle with reviewer attribution
  - reviewed text override support for modified constraints
  - decision summary counters and completeness gating
  - guard rails for missing reviewer/text/unknown requirement ids
- `editor/tests/step577_test.cpp` - 12 tests covering:
  - review-load success/failure behavior
  - approve/modify/reject behavior
  - unknown-id and missing-input guard behavior
  - review summary count/completeness behavior
  - decision override behavior
  - non-constraint filtering behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step577_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step577_test step576_test` - PASS
- `./editor/build-native/step577_test` - PASS (12/12)
- `./editor/build-native/step576_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/ArchitectReviewSurface.h` within header-size limit (`126` <= `600`)
- `editor/tests/step577_test.cpp` within test-file size guidance (`178` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 578: Phase 32a Integration
**Status:** PASS (8/8 tests)

Integrates the Phase 32a markdown intake pipeline across parser,
normalization/conflict detection, decomposition, and architect review workflow.

**Files added:**
- `editor/src/Phase32aIntegration.h` - Phase 32a integration module:
  - markdown parse + normalization orchestration
  - conflict and uncertainty signal propagation
  - decomposition and review-load gating
  - review decision automation modes (approve/modify/reject-first)
  - phase readiness and failure-note synthesis
- `editor/tests/step578_test.cpp` - 8 tests covering:
  - full-cycle happy path behavior
  - parse/normalize/review-load failure behavior
  - modify/reject review decision behavior
  - conflict count and parsed section count exposure behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step578_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step578_test step577_test` - PASS
- `./editor/build-native/step578_test` - PASS (8/8)
- `./editor/build-native/step577_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/Phase32aIntegration.h` within header-size limit (`104` <= `600`)
- `editor/tests/step578_test.cpp` within test-file size guidance (`139` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

**Phase 32a totals (574-578):**
- **Steps completed:** 5
- **New tests in this phase plan:** 56/56 passing

### Step 579: Taskitem Generator v2
**Status:** PASS (12/12 tests)

Implements v2 taskitem generation from decomposed scope plans, including
queue-readiness metadata, prerequisite operations, and dependency hints.

**Files added:**
- `editor/src/TaskitemGeneratorV2.h` - task generator module:
  - taskitem generation from milestone/workstream decomposition
  - stable task id generation
  - prerequisite operation inference for constrained execution
  - queue-readiness gating based on requirement/prereq presence
  - milestone-order dependency hint emission
- `editor/tests/step579_test.cpp` - 12 tests covering:
  - generation success/failure behavior
  - id/title/milestone mapping behavior
  - queue-readiness behavior
  - prerequisite inference behavior
  - dependency-hint behavior
  - empty-workstream handling behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step579_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step579_test step578_test` - PASS
- `./editor/build-native/step579_test` - PASS (12/12)
- `./editor/build-native/step578_test` - PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/TaskitemGeneratorV2.h` within header-size limit (`65` <= `600`)
- `editor/tests/step579_test.cpp` within test-file size guidance (`225` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 580: Taskitem Confidence + Ambiguity Annotation
**Status:** PASS (12/12 tests)

Implements confidence and ambiguity annotation for generated taskitems to drive
routing/escalation behavior in constrained execution pipelines.

**Files added:**
- `editor/src/TaskitemConfidenceAmbiguity.h` - annotation module:
  - per-task confidence scoring from queue-readiness, ambiguity, conflicts, dependencies
  - ambiguity/conflict reason synthesis for operator visibility
  - escalation flag computation for low-confidence or ambiguous tasks
  - stable bounded confidence outputs (`0..100`)
- `editor/tests/step580_test.cpp` - 12 tests covering:
  - annotation success/failure behavior
  - confidence reductions from queue/ambiguity/conflict signals
  - reason generation behavior
  - escalation/no-escalation behavior
  - multi-task annotation behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step580_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step580_test step579_test` - PASS
- `./editor/build-native/step580_test` - PASS (12/12)
- `./editor/build-native/step579_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/TaskitemConfidenceAmbiguity.h` within header-size limit (`85` <= `600`)
- `editor/tests/step580_test.cpp` within test-file size guidance (`189` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 581: Acceptance-Criteria Binding
**Status:** PASS (12/12 tests)

Implements acceptance-criteria binding for annotated taskitems with explicit
check identities and generated test-skeleton handles.

**Files added:**
- `editor/src/AcceptanceCriteriaBinding.h` - acceptance binding module:
  - binds acceptance requirements onto each annotated taskitem
  - emits stable check ids (`task::requirement`)
  - generates sanitized test-skeleton identifiers
  - marks per-task acceptance coverage state
  - validates presence of tasks and acceptance requirements
- `editor/tests/step581_test.cpp` - 12 tests covering:
  - binding success/failure behavior
  - check fan-out behavior across tasks/requirements
  - check id/text correctness behavior
  - test-skeleton generation/sanitization behavior
  - coverage flag behavior
  - annotation field preservation behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step581_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step581_test step580_test` - PASS
- `./editor/build-native/step581_test` - PASS (12/12)
- `./editor/build-native/step580_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/AcceptanceCriteriaBinding.h` within header-size limit (`83` <= `600`)
- `editor/tests/step581_test.cpp` within test-file size guidance (`185` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 582: Intake-to-Queue Simulation Harness
**Status:** PASS (12/12 tests)

Implements an end-to-end intake simulation harness from markdown ingestion to
queue-ready taskitems with confidence, escalation, and acceptance coverage.

**Files added:**
- `editor/src/IntakeToQueueSimulationHarness.h` - end-to-end harness module:
  - parser -> normalization -> decomposition -> review -> generation -> annotation -> binding pipeline
  - queue metrics aggregation (queued/escalated/check-count)
  - failure-stage notes for each pipeline gate
  - successful queue readiness signaling
- `editor/tests/step582_test.cpp` - 12 tests covering:
  - full pipeline success behavior
  - parse/normalize/review/binding failure propagation behavior
  - modify-review path behavior
  - queue/check/escalation metric behavior
  - success-note emission behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step582_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step582_test step581_test` - PASS
- `./editor/build-native/step582_test` - PASS (12/12)
- `./editor/build-native/step581_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/IntakeToQueueSimulationHarness.h` within header-size limit (`96` <= `600`)
- `editor/tests/step582_test.cpp` within test-file size guidance (`193` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 583: Sprint 32 Integration + Summary
**Status:** PASS (8/8 tests)

Completes Sprint 32 integration by aggregating Phase 32a and 32b readiness
signals into phase/sprint outcomes with closure diagnostics.

**Files added:**
- `editor/src/Sprint32IntegrationSummary.h` - sprint integration module:
  - aggregates Phase 32a and 32b gate signals
  - computes phase pass flags and Sprint 32 pass flag
  - emits per-step failure notes and blocked phase notes
  - emits sprint readiness notes on full pass
- `editor/tests/step583_test.cpp` - 8 tests covering:
  - full sprint pass path
  - phase-specific failure blocking behavior
  - success/failure note emission behavior
  - multi-failure aggregation behavior
  - all-false boundary behavior
  - phase isolation behavior on single 32b failure

**Files modified:**
- `editor/CMakeLists.txt` - `step583_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step583_test step582_test` - PASS
- `./editor/build-native/step583_test` - PASS (8/8)
- `./editor/build-native/step582_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/Sprint32IntegrationSummary.h` within header-size limit (`70` <= `600`)
- `editor/tests/step583_test.cpp` within test-file size guidance (`128` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

**Phase 32b totals (579-583):**
- **Steps completed:** 5
- **New tests in this phase plan:** 56/56 passing

**Sprint 32 totals (574-583):**
- **Steps completed:** 10
- **New tests in this sprint plan:** 112/112 passing
- **Phase 32a (574-578):** 56/56 passing
- **Phase 32b (579-583):** 56/56 passing

### Sprint 32 End Refactor Pass (Architecture Compliance)
**Status:** PASS (112/112 tests revalidated)

Performed a focused post-sprint refactor to centralize intake text
normalization/sanitization logic used by markdown parsing, requirement
normalization, and acceptance test-skeleton generation.

**Files added:**
- `editor/src/IntakeTextUtil.h` - shared intake text helpers:
  - lowercase normalization helper
  - compact alnum-space normalization helper
  - generic token sanitization helper with configurable separators

**Files modified:**
- `editor/src/MarkdownSpecParser.h` - switched heading/lowercase/anchor normalization paths to shared intake text helpers
- `editor/src/RequirementNormalizationConflictDetector.h` - switched requirement text canonicalization to shared compact-normalization helper
- `editor/src/AcceptanceCriteriaBinding.h` - switched test-skeleton token sanitization to shared intake token sanitizer

**Verification run:**
- `cmake --build editor/build-native --target step574_test step575_test step576_test step577_test step578_test step579_test step580_test step581_test step582_test step583_test` - PASS
- `./editor/build-native/step574_test` - PASS (12/12)
- `./editor/build-native/step575_test` - PASS (12/12)
- `./editor/build-native/step576_test` - PASS (12/12)
- `./editor/build-native/step577_test` - PASS (12/12)
- `./editor/build-native/step578_test` - PASS (8/8)
- `./editor/build-native/step579_test` - PASS (12/12)
- `./editor/build-native/step580_test` - PASS (12/12)
- `./editor/build-native/step581_test` - PASS (12/12)
- `./editor/build-native/step582_test` - PASS (12/12)
- `./editor/build-native/step583_test` - PASS (8/8)

**Architecture gate check:**
- `editor/src/IntakeTextUtil.h` within header-size limit (`48` <= `600`)
- `editor/src/MarkdownSpecParser.h` within header-size limit (`124` <= `600`)
- `editor/src/RequirementNormalizationConflictDetector.h` within header-size limit (`153` <= `600`)
- `editor/src/AcceptanceCriteriaBinding.h` within header-size limit (`68` <= `600`)
- Shared normalization centralization reduces duplicated string-normalization logic and stays aligned with `ARCHITECTURE.md`

### Step 584: Value-Forward Onboarding Flow
**Status:** PASS (12/12 tests)

Implements a first-run onboarding flow that explicitly surfaces MCP depth,
constrained execution safeguards, verification, and routing transparency.

**Files added:**
- `editor/src/ValueForwardOnboardingFlow.h` - onboarding flow module:
  - profile-based first-run flow initialization
  - card completion and gated forward navigation
  - completion progress reporting
  - completed-card value highlight extraction
  - inactive/index guard behavior
- `editor/tests/step584_test.cpp` - 12 tests covering:
  - flow start success/failure behavior
  - completion/navigation guard behavior
  - end-of-flow behavior
  - progress computation behavior
  - value highlight extraction behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step584_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step584_test step583_test` - PASS
- `./editor/build-native/step584_test` - PASS (12/12)
- `./editor/build-native/step583_test` - PASS (8/8) regression coverage

**Architecture gate check:**
- `editor/src/ValueForwardOnboardingFlow.h` within header-size limit (`105` <= `600`)
- `editor/tests/step584_test.cpp` within test-file size guidance (`154` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 585: Workflow Visualization 2.0
**Status:** PASS (12/12 tests)

Implements workflow visualization state for long-range dependency links and
plain-language routing/review explanations, with electric-blue visual defaults.

**Files added:**
- `editor/src/WorkflowVisualizationV2.h` - workflow visualization module:
  - visualization node/edge schemas
  - node/edge insertion validation and duplicate guards
  - outgoing edge queries for dependency tracing
  - plain-language routing summary generation
  - node filtering by workflow role
- `editor/tests/step585_test.cpp` - 12 tests covering:
  - node/edge creation success/failure behavior
  - duplicate/id/color/endpoint guard behavior
  - outgoing dependency query behavior
  - routing summary explanation behavior
  - review-gate filtering behavior
  - electric-blue default preservation behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step585_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step585_test step584_test` - PASS
- `./editor/build-native/step585_test` - PASS (12/12)
- `./editor/build-native/step584_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/WorkflowVisualizationV2.h` within header-size limit (`118` <= `600`)
- `editor/tests/step585_test.cpp` within test-file size guidance (`177` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 586: Capability Discovery Panels
**Status:** PASS (12/12 tests)

Implements capability discovery panel state for surfacing tool categories,
operation counts, and contextual recommendations.

**Files added:**
- `editor/src/CapabilityDiscoveryPanels.h` - capability panel module:
  - category registration with validation/duplicate guards
  - operation count tracking per category
  - recommendation registration with dedupe guards
  - category ranking by operation activity
  - category-level recommendation/count query helpers
- `editor/tests/step586_test.cpp` - 12 tests covering:
  - registration success/failure behavior
  - operation count tracking behavior
  - recommendation add/dedupe behavior
  - ranked sorting behavior
  - unknown-category query fallback behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step586_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step586_test step585_test` - PASS
- `./editor/build-native/step586_test` - PASS (12/12)
- `./editor/build-native/step585_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/CapabilityDiscoveryPanels.h` within header-size limit (`86` <= `600`)
- `editor/tests/step586_test.cpp` within test-file size guidance (`150` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`

### Step 587: Guided Architect-to-Execution Demo Mode
**Status:** PASS (12/12 tests)

Implements a guided demo state machine that walks users through intake,
review, queue, constrained edits, and verification stages.

**Files added:**
- `editor/src/GuidedArchitectExecutionDemoMode.h` - demo mode module:
  - simulation-backed demo initialization
  - staged progression with gate checks
  - queue readiness enforcement before constrained edits
  - completion fraction reporting
  - stage timeline capture for demo narration
- `editor/tests/step587_test.cpp` - 12 tests covering:
  - start success/failure behavior
  - stage-by-stage advancement behavior
  - queue gate failure behavior
  - completion/deactivation behavior
  - progress fraction behavior
  - timeline capture behavior

**Files modified:**
- `editor/CMakeLists.txt` - `step587_test` target

**Verification run:**
- `cmake -S editor -B editor/build-native` - PASS
- `cmake --build editor/build-native --target step587_test step586_test` - PASS
- `./editor/build-native/step587_test` - PASS (12/12)
- `./editor/build-native/step586_test` - PASS (12/12) regression coverage

**Architecture gate check:**
- `editor/src/GuidedArchitectExecutionDemoMode.h` within header-size limit (`95` <= `600`)
- `editor/tests/step587_test.cpp` within test-file size guidance (`182` lines)
- Header-only architecture and naming conventions remain aligned with `ARCHITECTURE.md`
