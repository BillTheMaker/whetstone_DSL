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
