# Sprint 9: Agent-First Tooling — Plan

> **Goal:** Make Whetstone a practical tool that AI agents use from a terminal
> to write better code with fewer tokens and fewer errors. Strip away the GUI
> dependency. Optimize every RPC round-trip for agent consumption. Ship a
> standalone MCP server that Claude Code (or any MCP client) can talk to
> directly.
>
> **Prerequisites:** Sprint 8 complete (244 steps). Full agent API (25+ RPC
> methods), MCP server/bridge headers, orchestrator, eval harness, and
> pipeline tool all in place.
>
> **Key insight:** Agents don't need squiggly lines, floating windows, or
> syntax highlighting. They need error codes, AST node references, structured
> diagnostics, and compact diffs. The entire LSP surface can be reduced to
> what's actionable in a JSON response.
>
> **Key themes:**
> 1. Headless-first: `whetstone_mcp` as a standalone binary, no SDL required
> 2. Agent-optimized RPC: compact responses, token budgets, delta encoding
> 3. Lean diagnostics: error codes + node references, not human-readable prose
> 4. File-level operations: create, read, write, diff without a GUI
> 5. Real integration testing with Claude Code via MCP

---

## Phase 9a: Standalone MCP Server (Steps 245–249)

Ship `whetstone_mcp` as a headless binary that any MCP client can launch.
No SDL, no ImGui, no OpenGL. Just JSON-RPC over stdio.

- [ ] **Step 245: Headless EditorState (no ImGui dependency)**
  Extract a `HeadlessEditorState` that provides the agent API surface without
  any ImGui or SDL includes:
  - Wraps the AST engine, tree-sitter parser, pipeline, and code generators
  - Provides `processAgentRequest(json, sessionId)` identical to EditorState
  - No `ImGui::GetTime()` — uses `std::chrono` for timestamps
  - No notification toasts — logs to stderr or collects in a vector
  - Shares all headers: ASTNode.h, Pipeline.h, AgentRPCHandler.h (refactored
    to accept a trait/interface rather than full EditorState)
  - Test: HeadlessEditorState can parse source, mutate AST, generate code
    without linking SDL or ImGui
  *New:* `editor/src/HeadlessEditorState.h`

- [ ] **Step 246: mcp_main.cpp entry point**
  Create the standalone MCP server executable:
  - `main()` creates HeadlessEditorState + MCPBridge
  - Wires `processAgentRequest` as the RPC callback
  - Wires resource reader for `whetstone://` URIs
  - Calls `bridge.runStdio()` — reads Content-Length framed JSON-RPC
  - CLI flags: `--workspace <dir>` (set working directory),
    `--language <lang>` (default language), `--verbose` (stderr logging)
  - Graceful shutdown on SIGINT/SIGTERM
  - Test: launch process, send `initialize` + `tools/list`, verify 10+ tools
  *New:* `editor/src/mcp_main.cpp`
  *Modifies:* `editor/CMakeLists.txt` (add `whetstone_mcp` target, no SDL link)

- [ ] **Step 247: File operation tools for MCP**
  Agents need to create, read, write, and diff files without a GUI:
  - `whetstone_file_read` — read file contents (with optional line range)
  - `whetstone_file_write` — write content to a file (creates if needed)
  - `whetstone_file_create` — create a new file with optional template
  - `whetstone_file_diff` — unified diff between current buffer and disk
  - `whetstone_workspace_list` — list files in workspace (with glob filter)
  - All operations respect workspace root from `--workspace`
  - RPC methods added to AgentRPCHandler, MCP tools registered in MCPServer
  - Test: create file via RPC, read it back, diff after modification
  *Modifies:* `AgentRPCHandler.h`, `MCPServer.h`

- [ ] **Step 248: Compact AST response format**
  Full AST dumps waste tokens. Add a compact mode:
  - `getAST` gains `compact: true` parameter — returns abbreviated nodes:
    `{id, type, name, line, children: [ids]}` instead of full property dump
  - `getASTSubtree` — new method: returns only the subtree rooted at a nodeId
  - `getASTDiff` — new method: returns only nodes changed since last query
    (uses a version counter incremented on each mutation)
  - Response includes `tokenEstimate` field (rough char count / 4)
  - Test: compact AST is <30% the size of full AST for a 50-function module
  *Modifies:* `AgentRPCHandler.h`

- [ ] **Step 249: MCP server integration tests**
  End-to-end tests for the standalone MCP server:
  1. Launch `whetstone_mcp`, complete MCP handshake (initialize/initialized)
  2. `tools/list` returns all registered tools with valid schemas
  3. `tools/call whetstone_get_ast` on empty state returns valid response
  4. `tools/call whetstone_run_pipeline` with Python source → generates code
  5. `resources/read whetstone://diagnostics` returns structured diagnostics
  6. `prompts/list` returns all registered prompts
  7. File operations: create, read, write, diff cycle
  8. Compact AST vs full AST size comparison
  *New:* `step249_test.cpp`

---

## Phase 9b: Agent-Optimized Diagnostics (Steps 250–253)

Replace human-readable LSP diagnostics with agent-optimized structured
output. Agents need error codes and AST references, not squiggly lines.

- [ ] **Step 250: Structured diagnostic format**
  Define a compact diagnostic format for agent consumption:
  ```json
  {
    "code": "E0302",
    "severity": "error",
    "nodeId": "func_42",
    "line": 17, "col": 4,
    "message": "type mismatch: expected int, got str",
    "fix": {"type": "setProperty", "nodeId": "func_42", "property": "returnType", "value": "str"}
  }
  ```
  - `getDiagnostics` RPC method: returns array of structured diagnostics
  - Each diagnostic includes: error code, severity, node reference, location,
    short message, and optional machine-applicable fix
  - Combines tree-sitter parse errors, Whetstone validation errors, and
    LSP diagnostics into one unified stream
  - Severity levels: error, warning, info, hint (numeric codes for filtering)
  - Test: parse invalid Python, verify diagnostics have nodeIds and fix hints
  *New:* `editor/src/StructuredDiagnostics.h`
  *Modifies:* `AgentRPCHandler.h`, `MCPServer.h`

- [ ] **Step 251: Quick-fix actions as RPC mutations**
  Turn diagnostic fixes into one-shot RPC calls:
  - `applyQuickFix` method: takes diagnostic code + nodeId, applies the
    suggested fix as a mutation
  - `getQuickFixes` method: for a given nodeId, returns all applicable fixes
    as mutation objects the agent can review before applying
  - Fix categories: type correction, missing import, unused variable removal,
    rename suggestion, missing return statement
  - Each fix is a concrete mutation (or batch) — no human interpretation needed
  - Test: parse code with type error, get fix, apply fix, verify diagnostic clears
  *Modifies:* `AgentRPCHandler.h`, `StructuredDiagnostics.h`

- [ ] **Step 252: Diagnostic delta streaming**
  After a mutation, agents shouldn't re-fetch all diagnostics:
  - `getDiagnosticsDelta` method: returns only diagnostics that changed since
    a given version number
  - Response: `{added: [...], removed: [...], version: N}`
  - Version counter increments on every AST mutation or reparse
  - Removed diagnostics identified by their `(code, nodeId, line)` tuple
  - Enables efficient "mutate → check if error cleared" loops
  - Test: introduce error, get diagnostics, fix error, get delta showing removal
  *Modifies:* `AgentRPCHandler.h`, `StructuredDiagnostics.h`

- [ ] **Step 253: Diagnostic and delta tests**
  Comprehensive tests for the diagnostic pipeline:
  1. Parse valid code → zero diagnostics
  2. Parse invalid code → structured diagnostics with nodeIds
  3. Diagnostics include fix suggestions where applicable
  4. `applyQuickFix` resolves the diagnostic
  5. Delta after fix shows removal
  6. Delta after introducing new error shows addition
  7. Combined tree-sitter + Whetstone diagnostics in one stream
  8. Severity filtering works
  *New:* `step253_test.cpp`

---

## Phase 9c: Token-Efficient Agent Protocol (Steps 254–257)

Minimize token cost per interaction. Every byte in a response costs the
agent (and the user) money and context window space.

- [ ] **Step 254: Response budget system**
  Let agents request only what they need:
  - All query methods gain optional `budget` parameter (max response chars)
  - If response exceeds budget, it's truncated with `"truncated": true` and
    a `"continuation"` token for paginated follow-up
  - `getAST` with budget: returns top-level nodes first, details on demand
  - `getDiagnostics` with budget: returns highest-severity first
  - Default budget: unlimited (backward compatible)
  - Test: getAST with budget=500 returns truncated response with continuation
  *Modifies:* `AgentRPCHandler.h`

- [ ] **Step 255: Symbol-only mode for scope queries**
  `getInScopeSymbols` currently returns full node data. Add lean mode:
  - `symbols` parameter (default): returns `[{name, type, kind, nodeId}]`
  - `detailed` parameter: returns full node JSON (current behavior)
  - `getCallHierarchy` lean mode: returns `{callers: [name], callees: [name]}`
    instead of full node trees
  - `getDependencyGraph` lean mode: adjacency list of nodeIds only
  - Test: lean scope query is <20% the size of detailed query
  *Modifies:* `AgentRPCHandler.h`

- [ ] **Step 256: Batch query endpoint**
  Agents often need AST + diagnostics + scope in one round-trip:
  - `batchQuery` method: accepts array of query requests, returns array of
    results in the same order
  - Single JSON-RPC call, single response — no protocol overhead per query
  - Each sub-query is a normal method name + params
  - Errors in one sub-query don't fail the batch
  - Test: batch of [getAST, getDiagnostics, getInScopeSymbols] returns 3 results
  *Modifies:* `AgentRPCHandler.h`, `MCPServer.h`

- [ ] **Step 257: Token efficiency tests + benchmarks**
  Measure and verify token savings:
  1. Compact AST vs full AST: measure ratio for 10/50/200 node modules
  2. Lean scope vs detailed: measure ratio
  3. Delta diagnostics vs full: measure ratio
  4. Budget truncation: verify continuation works across 3 pages
  5. Batch query vs sequential: measure total bytes
  6. Benchmark: time for 100 parse→mutate→diagnose cycles in headless mode
  *New:* `step257_test.cpp`

---

## Phase 9d: Multi-File Project Support (Steps 258–262)

Agents work on projects, not single files. Add project-level operations.

- [ ] **Step 258: Project model and workspace indexing**
  HeadlessEditorState gains project awareness:
  - `ProjectState`: tracks open files, workspace root, language per file
  - `openFile` / `closeFile` RPC methods manage multiple buffers
  - `listBuffers` returns all open files with language and dirty status
  - `setActiveBuffer` switches which buffer `getAST` etc. operate on
  - Workspace scan on startup: index file paths (not content) for fast lookup
  - Test: open 3 files, switch active, verify getAST returns correct AST
  *New:* `editor/src/ProjectState.h`
  *Modifies:* `HeadlessEditorState.h`, `AgentRPCHandler.h`

- [ ] **Step 259: Cross-file symbol resolution**
  When an agent queries scope, include symbols from other open files:
  - `getInScopeSymbols` gains `crossFile: true` parameter
  - Scans all open buffers for exported symbols (functions, classes, modules)
  - Returns source file path with each cross-file symbol
  - Import graph: track which files import which, update on file change
  - Test: file A imports from file B, scope query in A includes B's exports
  *Modifies:* `AgentRPCHandler.h`, `ProjectState.h`

- [ ] **Step 260: Project-wide diagnostics**
  Diagnostics across all open files in one call:
  - `getProjectDiagnostics` method: returns diagnostics grouped by file path
  - Compact format: `{"/path/foo.py": [{code, line, message}], ...}`
  - Includes cross-file errors (undefined import, circular dependency)
  - Optional severity filter and file path glob filter
  - Test: two files with errors, project diagnostics returns both sets
  *Modifies:* `AgentRPCHandler.h`, `StructuredDiagnostics.h`

- [ ] **Step 261: Project-wide search and refactor**
  Agents need to find and rename across files:
  - `searchProject` method: find symbol references across all open files
    by name or nodeId, returns `[{file, line, col, nodeId, context}]`
  - `renameSymbol` method: rename a symbol across all files that reference it
    returns a preview of all changes, then applies on confirmation
  - Uses the import graph to find cross-file references
  - Test: rename a function, verify all call sites across 2 files are updated
  *Modifies:* `AgentRPCHandler.h`, `ProjectState.h`

- [ ] **Step 262: Multi-file project tests**
  Full integration tests for project operations:
  1. Open workspace with 5 Python files
  2. Index finds all files
  3. Open 3 files, verify buffer list
  4. Cross-file scope resolution works
  5. Project-wide diagnostics aggregates correctly
  6. Search finds references across files
  7. Rename updates all references atomically
  8. Close file removes it from scope resolution
  *New:* `step262_test.cpp`

---

## Phase 9e: Claude Code MCP Integration (Steps 263–267)

Make `whetstone_mcp` work as a real MCP server that Claude Code can use.
This is the "install a plugin and activate you in my terminal" goal.

- [ ] **Step 263: MCP server configuration for Claude Code**
  Create the config and documentation for Claude Code integration:
  - `claude_desktop_config.json` snippet for `whetstone_mcp` server
  - Auto-detection: if `whetstone_mcp` is in PATH, register it
  - Server args: `--workspace $(pwd)` to set project root
  - Startup health check: verify MCP handshake completes in <2s
  - Test: config generation produces valid JSON matching Claude Code schema
  *New:* `editor/src/MCPConfig.h`
  *New:* `mcp-config.example.json`

- [ ] **Step 264: Agent workflow prompts for real coding tasks**
  Expand MCP prompts beyond annotations to practical coding workflows:
  - `implement_function` — spec → AST → code → validate → diagnostics loop
  - `fix_errors` — get diagnostics → apply quick-fixes → verify
  - `add_tests` — analyze function → generate test cases → write test file
  - `refactor_extract` — select code region → extract to new function
  - `explain_code` — get AST subtree → describe structure and behavior
  - Each prompt guides the agent through the optimal tool sequence
  - Test: each prompt generates valid multi-step instruction messages
  *Modifies:* `MCPServer.h`

- [ ] **Step 265: Tool result formatting for LLM consumption**
  Optimize tool results so Claude Code can parse them efficiently:
  - Error results include actionable next steps (not just error messages)
  - Success results include a one-line summary before detailed JSON
  - Large results (>2000 chars) auto-paginate with continuation tokens
  - Code blocks in results are language-tagged for model comprehension
  - Diagnostics include human-readable explanation alongside error codes
  - Test: tool results match expected format for 10 common operations
  *Modifies:* `MCPServer.h`, `AgentRPCHandler.h`

- [ ] **Step 266: End-to-end MCP workflow test**
  Simulate a complete coding session through MCP:
  1. Initialize MCP server with a workspace
  2. Create a new Python file via `whetstone_file_create`
  3. Write function stubs via `whetstone_file_write`
  4. Parse the file via `whetstone_run_pipeline`
  5. Get AST and verify structure via `whetstone_get_ast`
  6. Mutate: add a parameter via `whetstone_mutate`
  7. Get diagnostics: verify type-awareness via `whetstone_get_diagnostics`
  8. Generate code from spec via `whetstone_generate_code`
  9. Write generated code to new file
  10. Project-wide diagnostics show no errors
  *New:* `step266_test.cpp`

- [ ] **Step 267: MCP integration documentation**
  Write the integration guide (in code comments, not standalone docs):
  - MCPConfig.h: detailed comments explaining setup for Claude Code,
    Cursor, Continue, and generic MCP clients
  - Tool descriptions: ensure every MCP tool description is clear enough
    that an LLM can use it without examples
  - Example session transcript as a code comment in mcp_main.cpp
  - Error recovery guide: common errors and how the agent should respond
  *Modifies:* `mcp_main.cpp`, `MCPServer.h`, `MCPConfig.h`

---

## Phase 9f: Performance & Stability (Steps 268–272)

Agents hit the RPC surface hard and fast. Make sure it can keep up.

- [ ] **Step 268: Headless performance profiling**
  Establish baseline performance for agent workloads:
  - Benchmark harness: measures time for parse, mutate, generate, diagnose
  - Target: parse 1000-line file in <100ms, single mutation in <5ms,
    getAST (compact) in <10ms, diagnostics in <20ms
  - Profile memory: HeadlessEditorState with 10 open files <50MB
  - Report: JSON output with p50/p95/p99 latencies per operation
  - Test: all operations meet target latencies
  *New:* `editor/src/HeadlessBenchmark.h`
  *New:* `step268_test.cpp`

- [ ] **Step 269: Incremental reparse optimization**
  Don't reparse the entire file on every mutation:
  - Track byte ranges changed by each mutation
  - Use tree-sitter's incremental parse: `ts_parser_parse` with edit info
  - Only regenerate diagnostics for affected subtrees
  - Cache: last parse tree kept in memory, reused for incremental update
  - Test: editing one function in a 1000-line file is >5x faster than full reparse
  *Modifies:* `HeadlessEditorState.h`, `Pipeline.h` or relevant parser header

- [ ] **Step 270: AST mutation batching optimization**
  `applyBatch` should be faster than N individual mutations:
  - Defer reparse until batch completes (single reparse at end)
  - Defer diagnostic regeneration until batch completes
  - Coalesce adjacent mutations (e.g., two setProperty on same node)
  - Validation: run structural validation once after batch, not per-mutation
  - Test: batch of 20 mutations is >3x faster than 20 individual calls
  *Modifies:* `AgentRPCHandler.h`

- [ ] **Step 271: Stress test suite**
  Verify stability under sustained agent load:
  1. 1000 sequential parse→mutate→diagnose cycles without crash or leak
  2. 100 batch mutations of 50 operations each
  3. Rapid buffer switching (10 files, random access pattern)
  4. Concurrent-style stress: interleaved reads and writes
  5. Large file handling: 5000-line file parse + mutation
  6. Memory stable: RSS doesn't grow >2x after 1000 cycles
  *New:* `step271_test.cpp`

- [ ] **Step 272: Error recovery and graceful degradation**
  Agents will send bad requests. Handle them gracefully:
  - Malformed JSON → clean error, no crash
  - Invalid nodeId → error with suggestion (nearest valid node)
  - Mutation on deleted node → error explaining node was removed
  - Parse failure → partial AST with error markers, not empty response
  - OOM protection: if AST exceeds size limit, refuse new insertions
  - Test: 10 different error scenarios all return structured errors, no crashes
  *Modifies:* `AgentRPCHandler.h`, `HeadlessEditorState.h`

---

## Summary

| Phase | Steps | Description |
|-------|-------|-------------|
| 9a | 245–249 | Standalone MCP server (`whetstone_mcp` binary) |
| 9b | 250–253 | Agent-optimized diagnostics (error codes, nodeIds, fixes) |
| 9c | 254–257 | Token-efficient protocol (budgets, deltas, batching) |
| 9d | 258–262 | Multi-file project support |
| 9e | 263–267 | Claude Code MCP integration |
| 9f | 268–272 | Performance, stability, and stress testing |

**Total: 28 steps across 6 phases.**

---

## Architecture Notes

- **No SDL/ImGui dependency for agent path:** `whetstone_mcp` links only
  against tree-sitter, nlohmann-json, and the Whetstone core headers.
  This means it builds and runs on any Linux box, CI server, or container
  without display server or GPU.
- **HeadlessEditorState vs EditorState:** HeadlessEditorState is not a
  subclass — it's a parallel implementation that shares the same headers
  (ASTNode.h, Pipeline.h, CodeGen.h, etc.) but replaces ImGui-dependent
  code with std::chrono timestamps and stderr logging.
- **Agent RPC interface trait:** AgentRPCHandler.h should be refactored to
  accept an interface (getAST, mutateAST, getDiagnostics) rather than
  taking `EditorState&` directly. Both EditorState and HeadlessEditorState
  implement this interface. This avoids code duplication.
- **600-line limit enforced:** All new headers must pass file_limits_test.
- **Test-first:** Every step has real assertions testing actual behavior.
  No placeholder tests.
- **MCP protocol version:** `2024-11-05` (current stable). Content-Length
  framing over stdio.
- **Token accounting:** Responses include `tokenEstimate` (chars/4) so
  agents can make informed decisions about what to query.

---

## Sprint 10 Preview (for planning)

**Sprint 10: Language Intelligence & GUI Rehabilitation**
1. LSP client for headless mode (clangd, pyright, rust-analyzer)
2. Language-specific code actions (Go imports, Rust borrow fixes)
3. GUI overhaul: pin floating windows, sane defaults, usable color scheme
4. Terminal UI mode (TUI) as alternative to ImGui GUI
5. Plugin system: load language support and tools dynamically
