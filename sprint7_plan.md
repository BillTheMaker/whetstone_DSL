# Sprint 7: MCP Bridge & Agent Tooling — Plan

> **Goal:** Make Whetstone's agent API accessible to LLMs (Claude, Codex,
> open-source models) via the Model Context Protocol (MCP). Document the
> existing JSON-RPC API, build an MCP server that wraps it, generate
> synthetic interaction traces for evaluation and fine-tuning, and create
> an evaluation harness to measure LLM tool-use accuracy against Whetstone.
>
> **Prerequisites:** Sprint 6 complete (201 steps). All agent API methods
> wired through WebSocket JSON-RPC. WorkflowRecorder captures sessions.
>
> **Key themes:**
> 1. API documentation and JSON schemas
> 2. MCP server exposing editor operations as tools/resources
> 3. Synthetic trace generation for training data
> 4. Evaluation harness for LLM tool-use accuracy
> 5. Model-specific tool definitions (Claude, Codex)
> 6. Session recording pipeline

---

## Existing Agent API Surface (Reference)

Before planning steps, here's what already exists:

**WebSocket JSON-RPC Methods (~25 methods):**

| Category | Methods |
|----------|---------|
| Session | `ping`, `getSessionInfo`, `setAgentName`, `listSessions` |
| AST Access | `getAST` |
| Code Generation | `generateCode` |
| Mutations | `applyMutation` (subtypes: setProperty, updateNode, deleteNode, insertNode) |
| Annotations | `getAnnotationSuggestions`, `applyAnnotationSuggestion`, `recordAnnotationFeedback` |
| Workflow | `startWorkflowRecording`, `stopWorkflowRecording`, `getWorkflowRecording`, `replayWorkflow` |
| Role Management | `setAgentRole` |

**C++ API Only (not yet exposed via RPC):**
- `ContextAPI`: `getInScopeSymbols`, `getCallHierarchy`, `getDependencyGraph`
- `BatchMutationAPI`: `applySequence`
- `Pipeline`: `run`, `parse`, `generate`

**Orchestrator-Only RPC (separate process):**
- `setNodeProperty`, `insertNode`, `undo`, `redo`, `getLocks`
- Emacs bridge: `sendToEmacs`, `loadFile`, `saveFile`

---

## Phase 7a: API Documentation & Schemas (Steps 202–206)

Formal documentation so LLMs can understand and use the API correctly.

- [ ] **Step 202: JSON-RPC API reference document**
  Create `docs/AGENT_API.md` with complete API reference:
  - Every method: name, description, parameter schema, response schema, examples
  - Grouped by category (Session, Query, Mutation, Annotation, Workflow, Generation)
  - Error codes and error response format
  - Authentication/role requirements noted per method
  - "Quick Start" section: connect → set role → get AST → mutate → verify
  *New:* `docs/AGENT_API.md`

- [ ] **Step 203: JSON Schema definitions for all RPC methods**
  Create machine-readable JSON Schema files for request/response validation:
  - `schemas/methods/getAST.json` — request params + response format
  - `schemas/methods/applyMutation.json` — all 4 subtypes with discriminated union
  - `schemas/methods/generateCode.json` — spec + preferImports params
  - `schemas/methods/getAnnotationSuggestions.json` — nodeId or line/col params
  - One schema file per method, plus shared `schemas/types/` for reusable types:
    `ASTNode.schema.json`, `Suggestion.schema.json`, `Diagnostic.schema.json`,
    `MutationResult.schema.json`, `Symbol.schema.json`, `CallInfo.schema.json`
  - Schemas validate against actual API behavior (tested in Step 206)
  *New:* `schemas/` directory with ~30 schema files

- [ ] **Step 204: Expose ContextAPI and BatchMutationAPI via RPC**
  Wire the remaining C++ APIs into the WebSocket JSON-RPC handler:
  - `getInScopeSymbols` — params: `{nodeId}` → Symbol[]
  - `getCallHierarchy` — params: `{functionId}` → CallInfo
  - `getDependencyGraph` — params: `{nodeId}` → string[] (node IDs)
  - `applyBatch` — params: `{mutations: Mutation[]}` → BatchResult
  - All respect `AgentPermissionPolicy` (queries = all roles, batch = Refactor/Generator)
  *Modifies:* `EditorState.h` (processAgentRequest handler)

- [ ] **Step 205: Expose Pipeline operations via RPC**
  Add RPC methods for the end-to-end pipeline:
  - `runPipeline` — params: `{source, sourceLanguage, targetLanguage}` → PipelineResult
  - `parseSource` — params: `{source, language}` → `{ast, diagnostics[]}`
  - `generateFromAST` — params: `{language}` → `{code}` (uses current AST)
  - `projectLanguage` — params: `{targetLanguage}` → `{ast, annotationChanges[]}`
  These are high-level operations LLMs can use without manual AST manipulation.
  *Modifies:* `EditorState.h` (processAgentRequest handler)

- [ ] **Step 206: API schema validation tests**
  Tests verifying schemas match actual API behavior:
  1. Each method's request schema validates against test request payloads
  2. Each method's response schema validates against actual responses
  3. Invalid requests rejected with correct error codes
  4. Role-gated methods return permission errors for wrong roles
  5. Batch mutation rollback verified via schema-validated responses
  6. Pipeline methods return schema-compliant PipelineResult
  *New:* `step206_test.cpp`

---

## Phase 7b: MCP Server (Steps 207–213)

Wrap the Whetstone agent API as an MCP server so Claude, Codex, and other
MCP-compatible clients can interact with the editor natively.

- [ ] **Step 207: MCP server core with stdio transport**
  Create `MCPServer.h` implementing the MCP protocol:
  - JSON-RPC 2.0 over stdin/stdout (MCP stdio transport)
  - `initialize` / `initialized` handshake with capabilities declaration
  - Server info: name="whetstone-mcp", version from build
  - Capabilities: `tools`, `resources`, `prompts`
  - Connection lifecycle: initialize → ready → serve → shutdown
  - Error handling: MCP-compliant error responses
  *New:* `editor/src/MCPServer.h`

- [ ] **Step 208: MCP tools — AST query and mutation**
  Register MCP tools that map to existing JSON-RPC methods:
  - `whetstone_get_ast` — Returns the current AST as JSON
    - Input schema: `{}` (no params) or `{nodeId}` for subtree
    - Returns: AST JSON with annotation counts and diagnostics
  - `whetstone_mutate` — Apply a mutation to the AST
    - Input schema: `{type, nodeId, property, value, ...}` (same as applyMutation)
    - Returns: `{success, warning}`
  - `whetstone_batch_mutate` — Apply multiple mutations atomically
    - Input schema: `{mutations: [{type, ...}]}`
    - Returns: `{success, appliedCount, error}`
  - `whetstone_get_scope` — Get symbols in scope at a node
    - Input schema: `{nodeId}`
    - Returns: Symbol[]
  - `whetstone_get_call_hierarchy` — Call hierarchy for a function
    - Input schema: `{functionId}`
    - Returns: CallInfo
  Each tool has a complete JSON Schema for its input, and a description
  string tuned for LLM comprehension.
  *Modifies:* `MCPServer.h`

- [ ] **Step 209: MCP tools — annotation and generation**
  Additional MCP tools for higher-level operations:
  - `whetstone_suggest_annotations` — Get annotation suggestions for a code region
    - Input: `{nodeId}` or `{line, col}`
    - Returns: suggestions with confidence scores + diagnostics
  - `whetstone_apply_annotation` — Apply a suggested annotation
    - Input: suggestion object
    - Returns: `{success, warning}`
  - `whetstone_generate_code` — Generate code from a natural language spec
    - Input: `{spec, preferImports?}`
    - Returns: `{node, usedSymbols, language}`
  - `whetstone_run_pipeline` — Full parse→infer→validate→optimize→generate
    - Input: `{source, sourceLanguage, targetLanguage}`
    - Returns: PipelineResult (generated code + diagnostics)
  - `whetstone_project_language` — Cross-language projection
    - Input: `{targetLanguage}`
    - Returns: projected AST + annotation adaptation notes
  *Modifies:* `MCPServer.h`

- [ ] **Step 210: MCP resources — editor state and project info**
  Expose read-only editor state as MCP resources:
  - `whetstone://ast` — Current AST as JSON (subscribable)
  - `whetstone://diagnostics` — Current diagnostics list
  - `whetstone://libraries` — Imported libraries with available symbols
  - `whetstone://annotations` — All annotations in current module
  - `whetstone://buffer/{path}` — Buffer content by file path
  - `whetstone://settings` — Editor settings (read-only)
  Resources support MCP `resources/list` and `resources/read`.
  Optional: `resources/subscribe` for AST and diagnostics (change notifications).
  *Modifies:* `MCPServer.h`

- [ ] **Step 211: MCP prompts — guided workflows**
  Pre-built MCP prompts that guide LLMs through complex operations:
  - `annotate_module` — "Analyze this module and suggest memory annotations
    for all unannotated functions. Show confidence scores."
  - `cross_language_projection` — "Project this {sourceLanguage} code to
    {targetLanguage}, adapting annotations appropriately."
  - `security_audit` — "Check all dependencies for known vulnerabilities
    and suggest safe alternatives."
  - `refactor_memory` — "Analyze memory strategy annotations and suggest
    improvements for safety and performance."
  Each prompt includes argument schemas and returns structured messages.
  *Modifies:* `MCPServer.h`

- [ ] **Step 212: MCP server entry point and transport wiring**
  Create the MCP server executable and wire it to the editor:
  - `editor/src/mcp_main.cpp` — Standalone MCP server process
    - Reads stdin, writes stdout (MCP stdio transport)
    - Connects to the editor's WebSocket server as an internal client
    - Translates MCP tool calls → JSON-RPC requests → MCP responses
  - Configuration: `mcp_config.json` specifying tool/resource visibility
  - Startup: `whetstone_mcp.exe` (or `whetstone_mcp` on Linux)
  - Can also run embedded within the editor process (in-process mode)
  *New:* `editor/src/mcp_main.cpp`, `editor/src/MCPBridge.h`

- [ ] **Step 213: MCP server tests**
  End-to-end tests for the MCP server:
  1. Initialize handshake returns correct capabilities
  2. `tools/list` returns all registered tools with schemas
  3. `tools/call` for each tool produces correct results
  4. `resources/list` returns all resources
  5. `resources/read` for each resource returns valid data
  6. `prompts/list` returns all prompts with argument schemas
  7. Invalid tool calls return MCP-compliant errors
  8. Role enforcement: tools respect agent permissions
  *New:* `step213_test.cpp`

---

## Phase 7c: Synthetic Trace Generation (Steps 214–219)

Generate realistic interaction traces for fine-tuning LLMs on Whetstone
tool use. Traces should look like real Claude/Codex sessions working with
the editor.

- [ ] **Step 214: Trace data model and format**
  Define the trace format in `TraceGenerator.h`:
  - `Trace` struct: `{id, scenario, steps: TraceStep[]}`
  - `TraceStep`: `{role: "user"|"assistant"|"tool_call"|"tool_result", content}`
  - Tool calls include: tool name, input JSON, output JSON
  - Scenarios tagged with difficulty: basic, intermediate, advanced
  - Export formats: JSONL (one trace per line), OpenAI chat format, Anthropic
    messages format
  - Trace metadata: language, annotation types used, tools invoked, success/failure
  *New:* `editor/src/TraceGenerator.h`

- [ ] **Step 215: Scenario templates**
  Define parameterized scenario templates that can generate many traces:
  - **Read & Understand**: get AST → navigate structure → describe annotations
  - **Add Annotations**: get AST → identify unannotated functions → suggest →
    apply annotations → verify
  - **Cross-Language**: get AST → run pipeline Python→C++ → review generated
    code → fix annotation issues
  - **Refactor**: get AST → find anti-patterns → batch mutate → validate →
    verify improvements
  - **Security Audit**: get libraries → check vulnerabilities → suggest
    safe alternatives → update dependencies
  - **Multi-Step Debugging**: get AST → run validation → find errors →
    fix annotations → re-validate → confirm clean
  Each template parameterized by: language, code complexity, number of functions,
  annotation density, error injection rate.
  *New:* scenario template data structures in `TraceGenerator.h`

- [ ] **Step 216: Code corpus for trace generation**
  Build a corpus of diverse code samples for trace generation:
  - 20 Python modules (data processing, web handlers, ML pipelines, utils)
  - 20 C++ modules (data structures, algorithms, RAII patterns, templates)
  - 10 JavaScript modules (React components, Node.js servers, async patterns)
  - 10 Rust modules (ownership patterns, trait implementations, error handling)
  - 5 Go modules (goroutines, channels, interfaces)
  - 5 Java modules (classes, generics, streams)
  - Each module: 3–10 functions with varied complexity
  - Some modules pre-annotated, some intentionally unannotated
  - Some modules with annotation errors (for debugging scenarios)
  Corpus stored as JSON-serialized ASTs in `traces/corpus/`.
  *New:* `traces/corpus/` directory with ~70 sample modules

- [ ] **Step 217: Trace generator engine**
  Engine that combines templates + corpus to produce traces:
  - `TraceGenerator::generate(scenario, params)` → Trace
  - Simulates tool calls with real API responses (uses Pipeline, ContextAPI, etc.)
  - Injects realistic "thinking" steps for the assistant role
  - Handles error paths: tool call fails → assistant recovers → retries
  - Configurable: noise level (typos in specs), difficulty, verbosity
  - Deterministic with seed for reproducibility
  - Batch generation: `generateBatch(count, distribution)` → Trace[]
  *Modifies:* `TraceGenerator.h`

- [ ] **Step 218: Trace export pipeline**
  Export traces in formats suitable for fine-tuning and evaluation:
  - **Anthropic Messages format**: `{role, content}` with tool_use/tool_result blocks
  - **OpenAI Chat format**: `{role, content, tool_calls, tool_call_id}`
  - **JSONL**: one trace per line, streaming-friendly
  - **Markdown**: human-readable trace for review/documentation
  - Filtering: by scenario type, difficulty, language, success/failure
  - Statistics: tool call distribution, average trace length, success rate
  - CLI: `whetstone_traces --count 1000 --format anthropic --output traces/`
  *New:* `editor/src/TraceExporter.h`, export logic in `TraceGenerator.h`

- [ ] **Step 219: Trace generation tests**
  Tests for trace generation:
  1. Generated traces conform to schema (valid JSON, correct roles)
  2. Tool calls in traces produce valid responses when replayed
  3. Each scenario template generates distinct traces with different seeds
  4. Export formats are well-formed (Anthropic/OpenAI/JSONL)
  5. Batch generation produces correct count with expected distribution
  6. Error-path traces include recovery sequences
  *New:* `step219_test.cpp`

---

## Phase 7d: Evaluation Harness (Steps 220–224)

Test suites that measure how accurately an LLM can use Whetstone tools.

- [x] **Step 220: Evaluation framework**
  Create `EvalHarness.h` with the evaluation infrastructure:
  - `EvalTask` struct: `{id, description, setupAST, expectedOutcome, tools, maxSteps}`
  - `EvalResult` struct: `{taskId, passed, steps, toolCallCount, errors, duration}`
  - `EvalRunner`: loads tasks, executes agent traces, compares outcomes
  - Outcome comparison: AST diff (structural equality modulo IDs),
    annotation presence, generated code equivalence
  - Scoring: binary pass/fail + partial credit for intermediate progress
  *New:* `editor/src/EvalHarness.h`

- [ ] **Step 221: Evaluation task suite — basic operations**
  30 basic tasks testing individual tool use:
  - 5 tasks: read AST and answer questions about structure
  - 5 tasks: apply a single mutation (rename function, change type, etc.)
  - 5 tasks: add a specific annotation to a specific function
  - 5 tasks: get annotation suggestions and apply the best one
  - 5 tasks: generate code from a simple spec
  - 5 tasks: run pipeline and report diagnostics
  Each task has: setup AST, natural language instruction, expected outcome.
  *New:* `eval/basic/` directory with 30 task definitions

- [ ] **Step 222: Evaluation task suite — multi-step workflows**
  20 advanced tasks requiring multiple tool calls:
  - 5 tasks: annotate all functions in a module (iterate, suggest, apply)
  - 5 tasks: cross-language projection with annotation fixes
  - 5 tasks: refactor (find pattern → batch mutate → validate)
  - 5 tasks: debug annotation errors (validate → identify → fix → re-validate)
  Each task has multiple acceptable solution paths.
  *New:* `eval/workflows/` directory with 20 task definitions

- [ ] **Step 223: Evaluation runner CLI**
  Command-line tool to run evaluations:
  - `whetstone_eval --tasks eval/basic/ --trace agent_trace.jsonl --report report.json`
  - Accepts pre-recorded traces (for offline evaluation) or live agent connection
  - Produces: per-task pass/fail, aggregate scores, tool-call statistics
  - Comparison mode: evaluate multiple traces against same tasks
  - Leaderboard output: rank agents by accuracy, efficiency (fewer tool calls = better)
  *New:* `editor/src/eval_main.cpp`

- [ ] **Step 224: Evaluation harness tests**
  Tests for the evaluation framework itself:
  1. Known-good traces score 100% on basic tasks
  2. Known-bad traces (wrong mutations) score 0%
  3. Partial-credit scoring works for multi-step tasks
  4. AST diff correctly identifies structural changes
  5. Runner handles malformed traces gracefully
  6. Statistics aggregation is correct
  *New:* `step224_test.cpp`

---

## Phase 7e: Model-Specific Tool Definitions (Steps 225–229)

Optimized tool definitions and prompts for specific LLM families.

- [ ] **Step 225: Claude tool definitions**
  Anthropic-format tool definitions optimized for Claude:
  - Each Whetstone MCP tool as a Claude `tool_use` definition
  - Descriptions tuned for Claude's strengths (detailed, structured)
  - Input schemas with examples and constraints
  - System prompt template: "You are a Whetstone coding assistant with
    access to a semantic annotation editor. You can read and modify ASTs,
    suggest memory annotations, and project code across languages."
  - Few-shot examples embedded in tool descriptions
  *New:* `tools/claude/` directory with tool definitions + system prompt

- [ ] **Step 226: Codex / GPT tool definitions**
  OpenAI-format function definitions for Codex and GPT models:
  - Each tool as an OpenAI `function` definition
  - Descriptions tuned for OpenAI models (concise, example-heavy)
  - `strict: true` schemas where applicable
  - System prompt template adapted for OpenAI message format
  - Parallel tool calling hints for independent operations
  *New:* `tools/openai/` directory with function definitions + system prompt

- [ ] **Step 227: Open-source model tool definitions**
  Generic tool definitions for open-source models (Llama, Mistral, etc.):
  - ReAct-style prompting with tool descriptions in system prompt
  - XML-tagged tool call format (for models without native tool calling)
  - Simplified schemas (fewer optional fields, more defaults)
  - Adapter that parses model output into structured tool calls
  *New:* `tools/generic/` directory with definitions + adapter

- [ ] **Step 228: Prompt engineering templates**
  Reusable prompt templates for common Whetstone tasks:
  - `annotate_module.prompt` — System + user messages for annotation workflow
  - `cross_language.prompt` — System + user for cross-language projection
  - `code_review.prompt` — System + user for reviewing annotations
  - `refactor.prompt` — System + user for refactoring workflows
  - `security.prompt` — System + user for security audit
  Each template has: system prompt, user prompt template with `{{variables}}`,
  expected tool sequence, success criteria.
  *New:* `tools/prompts/` directory with 5+ prompt templates

- [ ] **Step 229: Tool definition tests**
  Tests verifying tool definitions are correct and complete:
  1. All MCP tools have corresponding Claude/OpenAI/generic definitions
  2. Tool input schemas match MCP tool schemas
  3. System prompts reference all available tools
  4. Few-shot examples in tool descriptions produce valid tool calls
  5. Prompt templates have all required variables
  *New:* `step229_test.cpp`

---

## Phase 7f: Session Recording Pipeline (Steps 230–234)

Capture real editing sessions, anonymize them, and export as training data.

- [ ] **Step 230: Enhanced session recorder**
  Extend `WorkflowRecorder` for full session capture:
  - Record all JSON-RPC traffic (not just agent-initiated)
  - Capture editor events: file open, buffer switch, theme change, etc.
  - Timestamp all events with millisecond precision
  - Session metadata: editor version, OS, language, project type
  - Start/stop recording from UI (status bar indicator)
  - Auto-recording mode: always capture (configurable in settings)
  *Modifies:* `WorkflowRecorder.h`, `panels/StatusBarPanel.h`

- [ ] **Step 231: Session anonymizer**
  Strip PII and sensitive data from recorded sessions:
  - Replace file paths with generic paths (`/project/src/module.py`)
  - Replace variable/function names with synthetic names (preserve structure)
  - Strip comments and string literal contents
  - Remove user-specific settings (paths, API keys)
  - Configurable: anonymization level (light/medium/full)
  - Deterministic: same input produces same anonymized output (with seed)
  *New:* `editor/src/SessionAnonymizer.h`

- [ ] **Step 232: Session-to-trace converter**
  Convert recorded sessions into training trace format:
  - Map editor events → user messages ("I opened file X and want to annotate it")
  - Map RPC requests → tool calls
  - Map RPC responses → tool results
  - Insert synthetic assistant "thinking" between tool calls
  - Handle session gaps (user pauses → split into separate traces)
  - Quality filter: discard sessions with too many errors or no meaningful work
  *New:* `editor/src/SessionToTrace.h`

- [ ] **Step 233: Training data pipeline CLI**
  End-to-end pipeline from raw sessions to training data:
  - `whetstone_pipeline --input sessions/ --anonymize medium --format anthropic --output training/`
  - Steps: load sessions → anonymize → convert to traces → filter → export
  - Statistics: sessions processed, traces generated, quality distribution
  - Deduplication: detect near-duplicate traces
  - Validation: all output traces pass schema validation
  *New:* `editor/src/pipeline_main.cpp`

- [ ] **Step 234: Session pipeline tests**
  Tests for the recording and conversion pipeline:
  1. Recorded session round-trips through anonymizer preserving structure
  2. Anonymized sessions have no file paths or identifiable content
  3. Session-to-trace conversion produces valid traces
  4. Pipeline CLI produces correct output count
  5. Quality filter removes degenerate sessions
  6. Deduplication identifies similar traces
  *New:* `step234_test.cpp`

---

## Summary

| Phase | Steps | Description |
|-------|-------|-------------|
| 7a | 202–206 | API documentation & JSON schemas |
| 7b | 207–213 | MCP server (tools, resources, prompts, transport) |
| 7c | 214–219 | Synthetic trace generation (templates, corpus, export) |
| 7d | 220–224 | Evaluation harness (tasks, runner, scoring) |
| 7e | 225–229 | Model-specific tool definitions (Claude, Codex, open-source) |
| 7f | 230–234 | Session recording pipeline (capture, anonymize, convert) |

**Total: 33 steps across 6 phases.**

---

## Context for Agents

### What exists after Sprint 6 (prerequisites):
- 80+ header files in editor/src/ including panels/, state/, ast/
- ~25 JSON-RPC methods via WebSocket agent server
- WorkflowRecorder for session capture/replay
- AgentPermissionPolicy with Linter/Refactor/Generator roles
- Full pipeline: parse→infer→validate→optimize→generate for 8 languages
- ContextAPI, BatchMutationAPI (C++ only, not yet RPC-exposed)
- 347+ passing test executables

### Architecture principles (from Sprint 6, still apply):
1. **600-line header limit** — split files that exceed this
2. **Panel extraction pattern** — `void renderXxx(EditorState&)` free functions
3. **Event-driven updates** — use UIEventBus, not polling
4. **Theme-aware rendering** — use `ThemeEngine::getColor()`, not hardcoded colors
5. **Test-first** — minimum 2 tests per step, real assertions only
6. **Header-only** — all components in `.h` files (only main/orchestrator/mcp are `.cpp`)

### Key new files introduced in Sprint 7:
| File | What it does |
|------|-------------|
| `docs/AGENT_API.md` | Complete JSON-RPC API reference |
| `schemas/` | Machine-readable JSON Schema for all methods |
| `MCPServer.h` | MCP protocol implementation |
| `MCPBridge.h` | MCP ↔ WebSocket JSON-RPC translation |
| `mcp_main.cpp` | Standalone MCP server executable |
| `TraceGenerator.h` | Synthetic interaction trace generation |
| `TraceExporter.h` | Multi-format trace export (Anthropic/OpenAI/JSONL) |
| `EvalHarness.h` | LLM tool-use evaluation framework |
| `eval_main.cpp` | Evaluation runner CLI |
| `SessionAnonymizer.h` | PII removal from recorded sessions |
| `SessionToTrace.h` | Session → training trace conversion |
| `pipeline_main.cpp` | Training data pipeline CLI |
| `tools/claude/` | Claude-optimized tool definitions |
| `tools/openai/` | OpenAI-format function definitions |
| `tools/generic/` | Open-source model tool definitions |
| `tools/prompts/` | Reusable prompt engineering templates |
| `traces/corpus/` | Code sample corpus for trace generation |
| `eval/basic/` | 30 basic evaluation tasks |
| `eval/workflows/` | 20 multi-step evaluation tasks |

### Build notes:
- No new external C++ dependencies required for Phases 7a–7b
  (MCP is JSON-RPC 2.0 over stdio, implemented with nlohmann-json)
- Phase 7c corpus files are JSON data, no code changes to add more
- Evaluation tasks are JSON definitions, extensible without recompilation
- Tool definitions (Phase 7e) are JSON/Markdown files, not compiled code

### MCP Protocol Reference:
- MCP spec: JSON-RPC 2.0 over stdio (stdin/stdout)
- Three primitives: **tools** (model-invoked), **resources** (context/data),
  **prompts** (user-facing templates)
- Lifecycle: `initialize` → `initialized` → serve → `shutdown`
- Capabilities negotiated during handshake

---

## Sprint 8 Preview (for planning)

**Sprint 8: Real-World Integration & Production Hardening**

Planned themes (to be fully specified after Sprint 7):
1. End-to-end testing with real LLM agents (Claude, GPT-4, Llama)
2. Performance profiling and optimization with real workloads
3. Multi-file project support (project-wide AST, cross-file references)
4. Git integration (diff view, blame annotations, branch-aware sessions)
5. Collaborative editing (multiple users/agents on same project)
6. Production packaging and distribution (stable release)
