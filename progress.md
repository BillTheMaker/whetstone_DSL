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
