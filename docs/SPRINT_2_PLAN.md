# Sprint 2 Plan: Whetstone Editor Stack

38 steps. Build one thing, test it, move on. If something breaks, the problem is in that step.

For detailed architecture and design rationale, see:
- `SPRINT_2_VISION.md` — Architecture diagrams, structured editing UI, agent API design
- `annotations/C++ Implementation Roadmap.md` — Full technical specs for each component (§1–§7)

---

## Phase 2a: AST in C++

Port the 33 SemAnno concepts from MPS XML into C++ data structures.

### Step 1: Base node + one concept
- `ASTNode` base class with ID, concept type tag, parent pointer
- `Module` concept only — properties (name, targetLanguage), empty child vectors
- **Test:** create a Module in C++, verify fields work

### Step 2: Child links
- Add child link support to `ASTNode` (single-valued and multi-valued)
- Add `Function` and `Variable` concepts as children of Module
- **Test:** build Module → Function tree in code, walk it, verify parent/child pointers

### Step 3: Remaining concepts
- Add all Statement, Expression, Literal, and Type concepts
- Each one gets its own properties and child link definitions
- **Test:** build the Calculator example as a C++ object graph

### Step 4: Annotation concepts
- Add `DerefStrategy`, `OptimizationLock`, `LangSpecific`
- Attach to Module/Function/Variable via annotations link
- **Test:** build SimpleFunctionExample with @deref(batched), verify annotation reads back

### Step 5: JSON serialization (save)
- Serialize the C++ AST graph to JSON
- One function: `toJson(ASTNode*) → nlohmann::json`
- **Test:** build Calculator graph → serialize → inspect JSON output by hand

### Step 6: JSON deserialization (load)
- Parse JSON back into C++ AST graph
- One function: `fromJson(nlohmann::json) → ASTNode*`
- **Test:** save Calculator → load it back → save again → compare JSON (byte-identical)

> **CHECKPOINT:** AST round-trips through JSON. Save → load → save produces identical output. Stop here until this passes.

### Step 7: Schema validation
- `ASTSchema` class: for each concept, list legal child types per role + cardinalities
- Validation function: `isLegalChild(parentConcept, role, childConcept) → bool`
- **Test:** legal placements accepted, illegal placements rejected (e.g., Module as child of IntegerLiteral)

---

## Phase 2b: First Generator

Python generator — port of the working MPS textGen rules into C++ visitor methods.

### Step 8: Generator base + Module/Function
- `ProjectionGenerator` base with visitor dispatch
- `PythonGenerator`: generate Module header and Function signatures only
- **Test:** Calculator AST → Python output has correct `def add(x, y):` signature

### Step 9: Statements and expressions
- Add Assignment, Return, BinaryOperation, VariableReference to Python generator
- **Test:** Calculator AST → full Python output matches MPS textGen output

> **CHECKPOINT:** Python generator matches MPS. Calculator output identical to what MPS produces. Stop here until this passes.

### Step 10: Remaining Python concepts
- Literals, control flow (If/While/For), types, FunctionCall, IndexAccess, MemberAccess
- **Test:** ConditionalExample AST → correct Python with if/else

### Step 11: Annotation output
- DerefStrategy → `# @deref(strategy)` comment in Python
- OptimizationLock → `# @lock(...)` comment
- **Test:** SimpleFunctionExample with annotations → comments appear in output

---

## Phase 2c: Dear ImGui Shell

Pure presentation layer — no business logic.

### Step 12: Window + empty layout
- Dear ImGui + SDL2 scaffold: opens a window with docking enabled
- Empty panes: file tree (left), editor area (center), panel (bottom)
- **Test:** app launches, window renders, panes are resizable

### Step 13: Text viewport
- Load a hardcoded Python string into the editor pane
- Render with monospace font, line numbers in gutter
- **Test:** text displays correctly, scrolls

### Step 14: AST → text viewport
- Connect: load Calculator JSON → build AST → run Python generator → display in viewport
- **Test:** change the JSON file, restart, see different output

> **CHECKPOINT:** ImGui shows live AST. Change JSON, restart, see different output. Stop here until this passes.

### Step 15: Projection toggle
- Toolbar with [Python] [AST] buttons
- Python: show generated code. AST: show JSON tree dump
- **Test:** toggle between views, both render correctly

---

## Phase 2d: Orchestrator (Standalone Process)

The brain — separate C++ process that owns the AST and coordinates everything.

### Step 16: Orchestrator process + AST ownership
- Separate C++ process that loads AST from JSON and holds it in memory
- Exposes nothing yet — just loads, holds, saves on exit
- **Test:** start orchestrator with Calculator.json, kill it, verify JSON saved back

### Step 17: JSON-RPC server
- Orchestrator listens on a Unix socket (or named pipe on Windows)
- Responds to `ping` and `getAST` methods
- **Test:** send JSON-RPC from a Python script, get response

### Step 18: AST mutation via RPC
- Add `setProperty` method: change a node's property value via RPC
- Validate against schema before applying
- **Test:** change Calculator function name via RPC, query AST back, see new name

### Step 19: Insert and delete via RPC
- `insertNode(parentId, role, conceptType, properties)` — creates node, validates, inserts
- `deleteNode(nodeId)` — removes node and subtree
- **Test:** insert a new Parameter via RPC, delete it, verify AST integrity

### Step 20: Undo/redo
- Operation journal: each mutation recorded as a reversible op
- `undo` and `redo` RPC methods
- **Test:** insert node → undo → node gone → redo → node back

### Step 21: Connect ImGui to orchestrator
- ImGui shell connects to orchestrator via JSON-RPC instead of loading JSON directly
- Requests AST on startup, displays projection
- **Test:** launch orchestrator, launch ImGui shell, see Calculator output

> **CHECKPOINT:** ImGui talks to orchestrator. Two processes running, viewport shows AST content. Stop here until this passes.

---

## Phase 2e: Structured Editing

Build code by choosing from valid options — no typing syntax.

### Step 22: Construct chooser (read-only)
- ImGui panel queries orchestrator: `getValidConstructs(parentId, role)` → list of legal types
- Display as button grid
- **Test:** click on Function body → see [Assignment] [Return] [If] [While] [For] etc.

### Step 23: Insert from chooser
- Click a construct button → orchestrator creates node with default properties
- Viewport refreshes with new projection
- **Test:** add a Return statement to a function, see it in Python output

### Step 24: Property editing
- Click a node → property panel shows editable fields (name, type, operator, etc.)
- Edit field → `setProperty` RPC → viewport refreshes
- **Test:** rename a function, see output update

### Step 25: Scope-aware dropdowns
- Orchestrator implements `getInScopeSymbols(nodeId)` → variables, functions, types visible at that position
- VariableReference name field shows dropdown of in-scope variables
- **Test:** inside Calculator.add body, dropdown shows [x, y, result]

> **CHECKPOINT:** Structured editing works. Build a function entirely through the UI. Stop here until this passes.

---

## Phase 2f: Emacs Integration

Headless Emacs servers for file ops, search, git, completion.

### Step 26: Spawn one headless Emacs
- Orchestrator spawns `emacs --daemon=whetstone-file --load whetstone-bridge.el`
- Sends a test command via `emacsclient`, gets response
- **Test:** orchestrator starts, Emacs server responds to ping

### Step 27: File operations
- `openFile(path)` → routes to Emacs file-server → returns contents
- `saveFile(path, contents)` → writes via Emacs
- **Test:** open a file through orchestrator, verify contents match

### Step 28: Search
- `search(query)` → routes to Emacs search-server → returns file:line results
- Results displayed in ImGui panel
- **Test:** search for "calculate_sum", get result pointing to SimpleFunctionExample

### Step 29: Server pool
- Multiple Emacs daemons: file-server, search-server, git-server
- Orchestrator routes by method name
- Health monitoring: restart crashed servers
- **Test:** kill a server, verify orchestrator restarts it and retries the request

> **CHECKPOINT:** Emacs servers respond. File open, search, and git status all return results. Stop here until this passes.

---

## Phase 2g: Elisp Projection

Generate Elisp from AST, and parse Elisp into AST.

### Step 30: Elisp generator — functions and variables
- Module → `(provide 'name)`, Function → `(defun name (params) body)`, Variable → `(defvar name value)`
- **Test:** Calculator AST → valid Elisp that loads in Emacs without errors

### Step 31: Elisp generator — statements and expressions
- Assignment → `(setq target value)`, Return → value (last expression), If → `(if cond then else)`
- BinaryOperation → `(op left right)`, Literals → values
- **Test:** ConditionalExample AST → Elisp with `(if (= status_code 200) "OK" "Error")`

### Step 32: Elisp parser (tree-sitter)
- Integrate tree-sitter-elisp, parse a `.el` file into CST
- Map `defun` → Function, `defvar` → Variable, `if` → IfStatement
- **Test:** parse a small hand-written `.el` file → inspect resulting AST

### Step 33: Elisp round-trip
- Parse `.el` → AST → generate Elisp → compare with original
- Handle `@LangSpecific` for idioms that don't map cleanly
- **Test:** write a 3-function `.el` file, round-trip it, verify semantic equivalence

> **CHECKPOINT:** Elisp round-trips. Parse .el → AST → generate .el → semantically equivalent. Stop here until this passes.

---

## Phase 2h: C++ Generator + Agent API

The final projection and the external agent interface.

### Step 34: C++ generator — basic output
- Module → `#include` guards, Function → typed signatures, variables → declarations
- No memory strategy yet — just syntactically valid C++
- **Test:** Calculator AST → compilable C++ with g++

### Step 35: C++ generator — memory strategies
- `@dealloc(manual)` → raw pointers, `@dealloc(gc)` → shared_ptr, `@dealloc(ownership)` → unique_ptr
- **Test:** same AST with different @dealloc values → different C++ output, all compile

> **CHECKPOINT:** C++ compiles from AST. Generated C++ compiles with g++ for all 3 example models. Stop here until this passes.

### Step 36: Agent API
- Orchestrator accepts JSON-RPC from external clients (same protocol as ImGui)
- `getValidConstructs`, `insertNode`, `setProperty`, `getInScopeSymbols`
- **Test:** Python script acts as agent, builds a Function node by node via RPC

### Step 37: Agent batch mode
- `insertSubtree(parentId, role, jsonSubtree)` — submit entire subtree at once
- Orchestrator validates recursively, applies atomically (all or nothing)
- **Test:** agent submits full ConditionalExample as JSON, orchestrator builds the AST

> **CHECKPOINT:** Agent builds AST remotely. Python script creates full ConditionalExample via RPC. Stop here until this passes.

### Step 38: OptimizationLock warnings
- When mutating a locked node, orchestrator returns warning (not rejection)
- ImGui shows lock icon + banner
- Shadow system: original annotation preserved, modification tracked via `@provenance`
- **Test:** lock a function, modify it, verify warning fires and original preserved

---

## Summary

| Phase | Steps | What You Have When Done |
|-------|-------|------------------------|
| 2a: AST in C++ | 1–7 | 33 concepts as C++ objects, JSON round-trip, schema validation |
| 2b: First Generator | 8–11 | Python output matching MPS for all examples |
| 2c: Dear ImGui Shell | 12–15 | Window showing live AST projection with toggle |
| 2d: Orchestrator | 16–21 | Separate process, RPC mutations, undo/redo, ImGui connected |
| 2e: Structured Editing | 22–25 | Build code through UI, scope-aware dropdowns |
| 2f: Emacs Integration | 26–29 | File ops, search, git via headless Emacs servers |
| 2g: Elisp Projection | 30–33 | Generate and parse Elisp, round-trip verified |
| 2h: C++ + Agent API | 34–38 | C++ generator with memory strategies, agent RPC interface |

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
