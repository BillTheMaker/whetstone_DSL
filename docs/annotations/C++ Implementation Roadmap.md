# C++ Implementation Roadmap: Whetstone Editor Stack

## Overview
This document outlines the implementation steps to build the Whetstone standalone editor — a C++ application combining Dear ImGui, headless Emacs servers, and an orchestrator node that operates on a native C++ AST. Sprint 1 (MPS) proved out the AST schema and dual projection. Sprint 2 ports that schema into a self-contained runtime and builds the editor around it.

The three deliverables are:
1. **C++ AST Runtime** — The SemAnno graph as native C++ data structures
2. **Dear ImGui Editor Shell** — Rendering, structured editing UI, construct chooser
3. **Orchestrator Node** — Server coordination, AST validation, agent API, Elisp↔C++ projection

---

## 1. C++ AST Runtime

The 33 SemAnno concepts currently live in MPS XML. They need to become a C++ object graph that the editor manipulates at runtime.

### 1a. Node Base Class and Concept Hierarchy
- Base `ASTNode` with: unique ID, concept type tag, parent pointer, annotation list
- Concept classes mirroring the MPS structure hierarchy:
  - `Module`, `Function`, `Parameter`, `Variable` (top-level containers)
  - `Statement` (abstract) → `Block`, `Assignment`, `Return`, `IfStatement`, `WhileLoop`, `ForLoop`
  - `Expression` (abstract) → `BinaryOperation`, `UnaryOperation`, `FunctionCall`, `VariableReference`, `IndexAccess`, `MemberAccess`, `ListLiteral`
  - Literals: `IntegerLiteral`, `FloatLiteral`, `StringLiteral`, `BooleanLiteral`
  - Types: `PrimitiveType`, `ListType`, `SetType`, `MapType`, `TupleType`, `ArrayType`, `OptionalType`, `CustomType`
  - Annotations: Memory strategy annotations (`DeallocateAnnotation`, `LifetimeAnnotation`, `ReclaimAnnotation`, `OwnerAnnotation`, `AllocateAnnotation` — initially implemented as `DerefStrategy`), `OptimizationLock`, `LangSpecific`
- Child links as `std::vector<std::unique_ptr<ASTNode>>` for 0..n, `std::unique_ptr<ASTNode>` for 1
- Properties as typed fields (string, int, enum)

### 1b. Schema-Driven Validation
- Each concept declares its legal children (by type, cardinality, role name)
- `ASTSchema` class loaded at startup — mirrors the SemAnno structure definitions
- Validation: given a parent concept + role, return the set of legal child concept types
- This is what powers the structured editing construct chooser

### 1c. AST Serialization
- **Primary format: JSON** — human-readable, debuggable, matches the Agent API contract
- Schema: mirrors MPS XML structure but in JSON (concept, id, properties, children keyed by role)
- Round-trip: load JSON → C++ graph → save JSON with no information loss
- Future option: SQLite-backed graph for large projects (deferred — JSON is sufficient for MVP)

### 1d. Annotation System
- Annotations attach to any node via the `annotations` child link
- Memory strategy annotations (canonical system from `Memory strategy.md`):
  - `DeallocateAnnotation`: strategy="Explicit" + deallocationLocation, deallocationTime
  - `LifetimeAnnotation`: strategy="RAII"
  - `ReclaimAnnotation`: strategy="Tracing"|"Cycle"|"Escape"
  - `OwnerAnnotation`: strategy="Single"|"Shared_ARC"
  - `AllocateAnnotation`: strategy="Static"|"Register"|"Allocator"
  - (Initially implemented as single `DerefStrategy` class — refactored in Sprint 3 Step 43)
- `OptimizationLock`: lockedBy, lockReason, lockLevel (warning/soft/hard), affectedStrategies, timestamp
- `LangSpecific`: language, idiomType, rawSyntax, semanticHint, position
- Annotations are metadata — they affect projection output, not the AST structure itself

---

## 2. Projection Generators (C++ and Elisp)

Sprint 1's MPS textGen rules proved the projection model. Sprint 2 needs equivalent generators as C++ code operating on the C++ AST.

### 2a. Generator Architecture
- `ProjectionGenerator` base class with `generate(ASTNode*, OutputStream&)` interface
- Per-concept visitor pattern: each concept type has a generate method
- Generator holds state: current indentation level, target language context
- Output is plain text (source code string)

### 2b. Python Generator (port from MPS)
- Direct port of the working MPS textGen rules into C++ visitor methods
- Module → `def`-based output with Python syntax
- Memory annotations ignored (Python is GC — `@Reclaim(Tracing)` is the default, has no effect on output)
- `LangSpecific(python)` annotations emit their `rawSyntax` inline

### 2c. C++ Generator
- Module → `#include`-guarded output with C++ syntax
- Functions → typed signatures with `auto` where appropriate
- Memory strategy application (canonical annotations from `Memory strategy.md`):
  - `@Deallocate(Explicit)` → raw pointers, explicit `new`/`delete`
  - `@Reclaim(Tracing)` → `std::shared_ptr` / `std::weak_ptr`
  - `@Lifetime(RAII)` / `@Owner(Single)` → `std::unique_ptr`, move semantics
  - `@Owner(Shared_ARC)` → `std::shared_ptr` with deterministic ref-counting
  - `@Allocate(Static)` + `@ConstExpr` → `const` types, content-hash identity
- Type mapping: `list[T]` → `std::vector<T>`, `map[K,V]` → `std::unordered_map<K,V>`, `optional[T]` → `std::optional<T>`
- `LangSpecific(cpp)` annotations emit their `rawSyntax` inline
- Optimization hints: `@Inline` → `[[gnu::always_inline]]`, `@Pure` → `[[nodiscard]]` + `const`, `@Loop(Unroll)` → `#pragma unroll`

### 2d. Elisp Generator
- Module → `(provide 'module-name)` with `defun`/`defvar` output
- Functions → `(defun name (params) body)`
- Variables → `(defvar name value)`
- Control flow: `IfStatement` → `(if cond then else)`, `WhileLoop` → `(while cond body)`
- Type annotations as comments (Elisp is dynamically typed)
- `LangSpecific(elisp)` annotations emit original forms: `(interactive ...)`, `(with-temp-buffer ...)`, `(advice-add ...)`

### 2e. Elisp Parser (tree-sitter)
- Integrate `tree-sitter-elisp` grammar
- CST → AST mapping: `defun` → Function, `let`/`let*` → Variable + Block, `if`/`cond`/`when` → IfStatement
- Elisp-specific constructs that don't map cleanly get `@LangSpecific(elisp, ...)` annotations
- Key Elisp idioms to handle:
  - `defvar`/`defcustom`/`defconst` → Variable with annotation metadata
  - `interactive` forms → `@LangSpecific(elisp, "special_form", ...)`
  - `advice-add`/`advice-remove` → `@LangSpecific(elisp, "advice", ...)`
  - `condition-case`/`unwind-protect` → error handling mapped to IfStatement + annotation
  - Dynamic binding (`let` vs `lexical-let`) → `@Binding(Dynamic)` / `@Binding(Static)` annotations
  - Macros → expand first, annotate with `@Meta(Expanded)` + `@Original(source)`

---

## 3. Dear ImGui Editor Shell

The rendering surface. Pure presentation — no business logic.

### 3a. Application Scaffold
- Dear ImGui + SDL2/GLFW backend (cross-platform)
- Docking layout: file tree, editor tabs, terminal panel, structured edit panel, command palette
- Font rendering with ligature support for code display
- Theme system (dark/light, configurable)

### 3b. Text Viewport
- Display projected source code (read from generator output)
- Syntax highlighting via token classification from the AST (not regex)
- Cursor and selection rendering
- Gutter: line numbers, annotation indicators, lock icons for `OptimizationLock`

### 3c. Structured Editing Panel
- **Construct Chooser**: grid of legal construct types for current cursor position (queried from AST schema)
- **Field Editors**: constrained inputs for each concept's properties
  - Name fields: identifier validation
  - Type fields: dropdown of available types (PrimitiveType kinds + user-defined)
  - Expression fields: recursive construct chooser
  - Enum fields: dropdown of legal values (e.g., TargetLanguage, deref strategies)
- **Scope-Aware Dropdowns**: only show in-scope variables, functions, types
- Keyboard shortcuts: single-key construct selection (f=Function, v=Variable, i=If, etc.)

### 3d. AST Tree View
- Collapsible tree showing the live AST structure
- Click node → highlight in text viewport
- Right-click → context menu (edit, delete, move, add annotation)
- Drag-and-drop reordering of statement lists

### 3e. Dual Projection Toggle
- Toolbar buttons: [Python] [C++] [Elisp] [AST]
- Switching regenerates the text viewport from the same AST via different generators
- Annotations panel shows/hides based on relevance to current projection

---

## 4. Orchestrator / Management Node

The brain. C++ process that coordinates everything.

### 4a. Core Responsibilities
- Owns the in-memory AST graph (single source of truth)
- Routes messages between ImGui shell, Emacs servers, and agent clients
- Validates all AST mutations against the schema
- Manages undo/redo stack (AST operation journal)
- Persists AST to disk (JSON serialization from §1c)

### 4b. AST Mutation API
- All changes go through the orchestrator — no direct graph manipulation
- Operations: `insertNode(parentId, role, conceptType, properties)`, `deleteNode(nodeId)`, `moveNode(nodeId, newParentId, role)`, `setProperty(nodeId, propertyName, value)`
- Each operation validated against schema before applying
- Each operation recorded for undo and provenance tracking

### 4c. IPC Protocol (JSON-RPC over Unix Sockets)
- **ImGui → Orchestrator**: user input events, construct choices, property edits
- **Orchestrator → ImGui**: AST updates, projection text, validation errors, scope options
- **Orchestrator → Emacs**: file I/O, search queries, completion requests, git operations
- **Emacs → Orchestrator**: results, file contents, candidate lists
- Message format: JSON-RPC 2.0 (`{ "jsonrpc": "2.0", "method": "...", "params": {...}, "id": N }`)

### 4d. Emacs Server Pool Management
- Spawn headless Emacs daemons on startup: `emacs --daemon=serverN --load whetstone-bridge.el`
- Specialized servers (file-server, search-server, complete-server, git-server, org-server)
- Health monitoring: heartbeat, restart on crash
- Request routing: match method name to responsible server
- Response caching: file contents, completion results (invalidated on change)

### 4e. Agent API
- External JSON interface for AI agents to submit construct choices
- Same validation as human UI — invalid choices rejected with `valid_options` list
- Workflow:
  1. Agent queries: `getValidConstructs(parentId, role)` → list of legal concept types
  2. Agent submits: `insertNode(parentId, role, "Function", { name: "process_data", ... })`
  3. Orchestrator validates → applies → returns updated AST subtree
- Batch mode: agent submits an entire subtree as JSON, orchestrator validates recursively
- Trust levels (configurable): auto-accept, review-required, sandbox-only

### 4f. Scope Resolution Engine
- Walks the AST to determine what's in scope at any given node position
- Used by: structured editing dropdowns, agent API validation, completion
- Tracks: variables, functions, parameters, types, imported modules
- Respects: block scoping, function boundaries, module boundaries
- Closure capture analysis: which outer-scope bindings are referenced

### 4g. Warning System (OptimizationLock)
- When a node has `OptimizationLock` and the current user/agent tier < lock level:
  - Structured editing panel shows lock icon and warning banner
  - Mutation still allowed (warning only, not blocking)
  - Original optimization preserved as shadow (annotation chain via `@provenance`)
  - Undo restores the locked state

---

## 5. Emacs Integration Layer

### 5a. whetstone-bridge.el
- Elisp package loaded by each headless Emacs server
- Exposes server capabilities as JSON-RPC endpoints
- Translates between Emacs primitives and Whetstone protocol:
  - `find-file` → buffer contents as JSON response
  - `consult-ripgrep` → search results as JSON array
  - `magit-status` → git status as structured data
  - `completion-at-point` → candidate list with metadata

### 5b. Buffer↔AST Synchronization
- Emacs buffers display projected text (generated from AST)
- Edits in Emacs buffer → parse diff → translate to AST mutations → send to orchestrator
- Orchestrator regenerates projection → pushes updated text back to Emacs buffer
- This enables using Emacs keybindings for text navigation while the AST remains authoritative

### 5c. Org-mode Integration
- Documentation lives alongside code as AST annotations
- Org-server renders `@doc` annotations as org-mode formatted text
- Edit documentation in Emacs → annotation updated in AST

---

## 6. Elisp↔C++ Transpilation (Dogfooding)

The editor itself is the first real Whetstone project. Write editor logic in Elisp, generate C++ for performance-critical paths.

### 6a. Target: whetstone-bridge.el
- Parse `whetstone-bridge.el` into Whetstone AST via tree-sitter-elisp
- Generate equivalent C++ for the Management Node's hot paths
- Verify semantic equivalence: same JSON-RPC responses for same inputs

### 6b. Elisp→C++ Mapping Challenges
- **Dynamic typing** → `std::variant` or tagged union with `@Candidate(Type)` annotations
- **Symbol tables** → `std::unordered_map<std::string, Value>` with `@Binding(Dynamic)` annotation
- **Garbage collection** → `std::shared_ptr` (default) or `std::unique_ptr` where ownership is clear
- **Homoiconicity** → AST nodes ARE the code-as-data representation (Whetstone is inherently homoiconic)
- **Buffer model** → C++ string with gap buffer or rope, annotated with `@LangSpecific(elisp, "buffer", ...)`
- **Event loop** → `@Exec(Event)` annotation, map to C++ event loop (SDL2 or libuv)

### 6c. Incremental Approach
- Start with pure functions (no side effects, no dynamic binding) — easiest to transpile
- Add stateful functions with explicit `@Capture` and `@Binding` annotations
- Tackle Emacs-specific idioms last (interactive forms, advice, buffer-local variables)
- Each function gets a confidence score: high = auto-transpile, low = human review

---

## 7. Testing and Validation

### 7a. AST Runtime Tests
- Round-trip: JSON → C++ graph → JSON (byte-identical)
- Schema validation: reject illegal child placements, accept legal ones
- Mutation: insert/delete/move operations maintain graph invariants

### 7b. Generator Tests
- Per-concept: each of the 33 concepts generates valid output in each target language
- Integration: example modules (Calculator, SimpleFunctionExample, ConditionalExample) generate compilable code
- Cross-language: Python output and C++ output are semantically equivalent for the same AST

### 7c. Editor Integration Tests
- Structured editing: construct chooser offers exactly the legal options
- Agent API: JSON submissions create correct AST structures
- Emacs IPC: file operations, search, completion respond within latency budget

### 7d. Transpilation Tests
- Elisp → AST → C++ → compile → run → compare output with Elisp evaluation
- Start with pure arithmetic/string functions, build up to stateful operations
- Annotation preservation: `@LangSpecific` survives round-trip

---

## Implementation Order

Each step is small enough to build, test, and verify before moving on. If a step breaks, the problem is in that step — not in 4 things you changed at once.

### Phase 2a: AST in C++

**Step 1: Base node + one concept**
- `ASTNode` base class with ID, concept tag, parent pointer
- `Module` concept only — properties (name, targetLanguage), empty child vectors
- Test: create a Module in C++, verify fields work

**Step 2: Child links**
- Add child link support to `ASTNode` (single-valued and multi-valued)
- Add `Function` and `Variable` concepts as children of Module
- Test: build Module → Function tree in code, walk it, verify parent/child pointers

**Step 3: Remaining concepts**
- Add all Statement, Expression, Literal, and Type concepts
- Each one gets its own properties and child link definitions
- Test: build the Calculator example as a C++ object graph

**Step 4: Annotation concepts**
- Add memory strategy annotations (initially as `DerefStrategy`), `OptimizationLock`, `LangSpecific`
- Attach to Module/Function/Variable via annotations link
- Test: build SimpleFunctionExample with `@Reclaim(Tracing)`, verify annotation reads back

**Step 5: JSON serialization (save)**
- Serialize the C++ AST graph to JSON
- One function: `toJson(ASTNode*) → nlohmann::json`
- Test: build Calculator graph → serialize → inspect JSON output by hand

**Step 6: JSON deserialization (load)**
- Parse JSON back into C++ AST graph
- One function: `fromJson(nlohmann::json) → ASTNode*`
- Test: save Calculator → load it back → save again → compare JSON (byte-identical)

**Step 7: Schema validation**
- `ASTSchema` class: for each concept, list legal child types per role + cardinalities
- Validation function: `isLegalChild(parentConcept, role, childConcept) → bool`
- Test: legal placements accepted, illegal placements rejected (e.g., Module as child of IntegerLiteral)

### Phase 2b: First Generator

**Step 8: Generator base + Module/Function**
- `ProjectionGenerator` base with visitor dispatch
- `PythonGenerator`: generate Module header and Function signatures only
- Test: Calculator AST → Python output has correct `def add(x, y):` signature

**Step 9: Statements and expressions**
- Add Assignment, Return, BinaryOperation, VariableReference to Python generator
- Test: Calculator AST → full Python output matches MPS textGen output

**Step 10: Remaining Python concepts**
- Literals, control flow (If/While/For), types, FunctionCall, IndexAccess, MemberAccess
- Test: ConditionalExample AST → correct Python with if/else

**Step 11: Annotation output**
- Memory annotations → `# @Reclaim(Tracing)` etc. comment in Python (initially emits as `# @deref(strategy)`)
- OptimizationLock → `# @lock(...)` comment
- Test: SimpleFunctionExample with annotations → comments appear in output

### Phase 2c: Dear ImGui Shell

**Step 12: Window + empty layout**
- Dear ImGui + SDL2 scaffold: opens a window with docking enabled
- Empty panes: file tree (left), editor area (center), panel (bottom)
- Test: app launches, window renders, panes are resizable

**Step 13: Text viewport**
- Load a hardcoded Python string into the editor pane
- Render with monospace font, line numbers in gutter
- Test: text displays correctly, scrolls

**Step 14: AST → text viewport**
- Connect: load Calculator JSON → build AST → run Python generator → display in viewport
- Test: change the JSON file, restart, see different output

**Step 15: Projection toggle**
- Toolbar with [Python] [AST] buttons
- Python: show generated code. AST: show JSON tree dump
- Test: toggle between views, both render correctly

### Phase 2d: Orchestrator (Standalone Process)

**Step 16: Orchestrator process + AST ownership**
- Separate C++ process that loads AST from JSON and holds it in memory
- Exposes nothing yet — just loads, holds, saves on exit
- Test: start orchestrator with Calculator.json, kill it, verify JSON saved back

**Step 17: JSON-RPC server**
- Orchestrator listens on a Unix socket (or named pipe on Windows)
- Responds to `ping` and `getAST` methods
- Test: send JSON-RPC from a Python script, get response

**Step 18: AST mutation via RPC**
- Add `setProperty` method: change a node's property value via RPC
- Validate against schema before applying
- Test: change Calculator function name via RPC, query AST back, see new name

**Step 19: Insert and delete via RPC**
- `insertNode(parentId, role, conceptType, properties)` — creates node, validates, inserts
- `deleteNode(nodeId)` — removes node and subtree
- Test: insert a new Parameter via RPC, delete it, verify AST integrity

**Step 20: Undo/redo**
- Operation journal: each mutation recorded as a reversible op
- `undo` and `redo` RPC methods
- Test: insert node → undo → node gone → redo → node back

**Step 21: Connect ImGui to orchestrator**
- ImGui shell connects to orchestrator via JSON-RPC instead of loading JSON directly
- Requests AST on startup, displays projection
- Test: launch orchestrator, launch ImGui shell, see Calculator output

### Phase 2e: Structured Editing

**Step 22: Construct chooser (read-only)**
- ImGui panel queries orchestrator: `getValidConstructs(parentId, role)` → list of legal types
- Display as button grid
- Test: click on Function body → see [Assignment] [Return] [If] [While] [For] etc.

**Step 23: Insert from chooser**
- Click a construct button → orchestrator creates node with default properties
- Viewport refreshes with new projection
- Test: add a Return statement to a function, see it in Python output

**Step 24: Property editing**
- Click a node → property panel shows editable fields (name, type, operator, etc.)
- Edit field → `setProperty` RPC → viewport refreshes
- Test: rename a function, see output update

**Step 25: Scope-aware dropdowns**
- Orchestrator implements `getInScopeSymbols(nodeId)` → variables, functions, types visible at that position
- VariableReference name field shows dropdown of in-scope variables
- Test: inside Calculator.add body, dropdown shows [x, y, result]

### Phase 2f: Emacs Integration

**Step 26: Spawn one headless Emacs**
- Orchestrator spawns `emacs --daemon=whetstone-file --load whetstone-bridge.el`
- Sends a test command via `emacsclient`, gets response
- Test: orchestrator starts, Emacs server responds to ping

**Step 27: File operations**
- `openFile(path)` → routes to Emacs file-server → returns contents
- `saveFile(path, contents)` → writes via Emacs
- Test: open a file through orchestrator, verify contents match

**Step 28: Search**
- `search(query)` → routes to Emacs search-server → returns file:line results
- Results displayed in ImGui panel
- Test: search for "calculate_sum", get result pointing to SimpleFunctionExample

**Step 29: Server pool**
- Multiple Emacs daemons: file-server, search-server, git-server
- Orchestrator routes by method name
- Health monitoring: restart crashed servers
- Test: kill a server, verify orchestrator restarts it and retries the request

### Phase 2g: Elisp Projection

**Step 30: Elisp generator — functions and variables**
- Module → `(provide 'name)`, Function → `(defun name (params) body)`, Variable → `(defvar name value)`
- Test: Calculator AST → valid Elisp that loads in Emacs without errors

**Step 31: Elisp generator — statements and expressions**
- Assignment → `(setq target value)`, Return → value (last expression), If → `(if cond then else)`
- BinaryOperation → `(op left right)`, Literals → values
- Test: ConditionalExample AST → Elisp with `(if (= status_code 200) "OK" "Error")`

**Step 32: Elisp parser (tree-sitter)**
- Integrate tree-sitter-elisp, parse a `.el` file into CST
- Map `defun` → Function, `defvar` → Variable, `if` → IfStatement
- Test: parse a small hand-written `.el` file → inspect resulting AST

**Step 33: Elisp round-trip**
- Parse `.el` → AST → generate Elisp → compare with original
- Handle `@LangSpecific` for idioms that don't map cleanly
- Test: write a 3-function `.el` file, round-trip it, verify semantic equivalence

### Phase 2h: C++ Generator + Agent API

**Step 34: C++ generator — basic output**
- Module → `#include` guards, Function → typed signatures, variables → declarations
- No memory strategy yet — just syntactically valid C++
- Test: Calculator AST → compilable C++ with g++

**Step 35: C++ generator — memory strategies**
- `@Deallocate(Explicit)` → raw pointers, `@Reclaim(Tracing)` → shared_ptr, `@Lifetime(RAII)` / `@Owner(Single)` → unique_ptr
- Test: same AST with different memory annotations → different C++ output, all compile

**Step 36: Agent API**
- Orchestrator accepts JSON-RPC from external clients (same protocol as ImGui)
- `getValidConstructs`, `insertNode`, `setProperty`, `getInScopeSymbols`
- Test: Python script acts as agent, builds a Function node by node via RPC

**Step 37: Agent batch mode**
- `insertSubtree(parentId, role, jsonSubtree)` — submit entire subtree at once
- Orchestrator validates recursively, applies atomically (all or nothing)
- Test: agent submits full ConditionalExample as JSON, orchestrator builds the AST

**Step 38: OptimizationLock warnings**
- When mutating a locked node, orchestrator returns warning (not rejection)
- ImGui shows lock icon + banner
- Shadow system: original annotation preserved, modification tracked via `@provenance`
- Test: lock a function, modify it, verify warning fires and original preserved

---

## Milestone Checkpoints

Each checkpoint is a "stop and verify" gate. Don't proceed until it passes.

| After Step | Checkpoint | How to Verify |
|------------|-----------|---------------|
| 6 | AST round-trips through JSON | Save → load → save produces identical JSON |
| 9 | Python generator matches MPS | Calculator output identical to MPS textGen |
| 14 | ImGui shows live AST | Change JSON, restart, see different output |
| 21 | ImGui talks to orchestrator | Two processes running, viewport shows AST content |
| 25 | Structured editing works | Build a function entirely through the UI |
| 29 | Emacs servers respond | File open, search, and git status all return results |
| 33 | Elisp round-trips | Parse .el → AST → generate .el → semantically equivalent |
| 35 | C++ compiles from AST | Generated C++ compiles with g++ for all 3 example models |
| 37 | Agent builds AST remotely | Python script creates full ConditionalExample via RPC |

## Dependencies

| Dependency | Version | Purpose |
|------------|---------|---------|
| Dear ImGui | 1.90+ | GUI rendering |
| SDL2 or GLFW | latest | Window/input backend for ImGui |
| Emacs | 29+ | Headless servers |
| nlohmann/json | 3.11+ | JSON serialization, JSON-RPC |
| tree-sitter | 0.22+ | Elisp/Python/C++ parsing |
| tree-sitter-elisp | latest | Elisp grammar |
| C++ compiler | C++20 | Entire C++ stack |
| GoogleTest | latest | C++ unit tests |
