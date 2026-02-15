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
