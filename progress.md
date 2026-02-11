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
