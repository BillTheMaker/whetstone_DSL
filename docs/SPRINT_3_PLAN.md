# Sprint 3 Plan: Whetstone Editor - Core Functionality & Classical Mode

37 steps (global steps 39–75). Build one thing, test it, move on. If something breaks, the problem is in that step.

> **Step numbering:** Sprint 2 ended at Step 38. Sprint 3 steps are numbered 39–75 to continue the global sequence. Phase-local references (e.g., "Phase 3a Step 1") map to global Step 39, etc.

For detailed architecture and design rationale, see:
- `REQUIREMENTS_OVERVIEW.md` — Core vision, SemAnno schema, memory strategies
- `annotations/Memory strategy.md` — **Canonical memory annotation reference** (10 strategies across 20 languages)
- `annotations/6 optimization and intent.md` — **Canonical optimization annotation reference** (`@Hot`/`@Cold`, `@Inline`, `@Pure`, etc.)
- `annotations/8 strategy choice and policy.md` — Policy annotations and strategy menus
- `annotations/C++ Implementation Roadmap.md` — C++ generator and memory management details

> **Note on annotation naming:** Sprint 2 used a simplified 4-strategy `@deref` system from `REQUIREMENTS_OVERVIEW.md`. Sprint 3 adopts the **canonical annotation system** defined in `annotations/Memory strategy.md`, which is the authoritative source. The canonical system uses distinct annotation families — `@Deallocate`, `@Lifetime`, `@Reclaim`, `@Owner`, `@Allocate` — each targeting a specific memory paradigm. The old `DerefStrategy` class will be refactored into these granular annotation types. See the Migration Notes at the bottom for the full mapping.

> **Test quality requirement:** All step tests MUST contain real assertions that exercise the code under test. Sprint 2 tests were placeholder stubs that print "PASS" without verifying behavior. Sprint 3 tests must `#include` the relevant headers, construct real AST objects, call real functions, and assert expected outputs. A test that just prints "PASS" is not a test.

---

## Canonical Memory Annotation Reference

From `annotations/Memory strategy.md` — these are the annotations Sprint 3 implements:

| Annotation | Paradigm | Description | Key Languages |
|---|---|---|---|
| `@Deallocate(Explicit)` | Manual (MM) | Explicit allocation/deallocation, direct `free()`/`delete` | C, C++ (manual mode), Zig |
| `@Lifetime(RAII)` | RAII | Destructor-based cleanup at scope end | C++ |
| `@Reclaim(Tracing)` | GC | Automatic runtime reclamation via tracing/mark-sweep | Python, Java, JS, Go, C#, Ruby |
| `@Reclaim(Cycle)` | GC | Cycle-detection garbage collection | PHP |
| `@Reclaim(Escape)` | GC | Escape-analysis-based heap vs stack decision | Go |
| `@Owner(Single)` | Ownership | Compile-time single-owner lifetime tracking (Rust-like) | Rust |
| `@Owner(Shared_ARC)` | ARC | Deterministic reference counting | Swift, Objective-C |
| `@Allocate(Static)` | Static | Fixed memory buffers, no dynamic allocation | Fortran |
| `@Allocate(Register)` | Register | Direct register mapping or stack spill | Assembly |
| `@Allocate(Allocator)` | Allocator | Explicit allocator parameter threading | Zig |

From `annotations/6 optimization and intent.md` — optimization annotations:

| Annotation | Purpose |
|---|---|
| `@Hot` / `@Cold` | Profile-guided branch prediction hints → C++ `[[likely]]`/`[[unlikely]]` |
| `@Inline(Always\|Never\|Hint)` | Function inlining control |
| `@Pure` | No side effects (referential transparency) |
| `@TailCall` | Tail call optimization eligible |
| `@Loop(Unroll, N)` / `@Loop(Vectorize)` / `@Loop(Fuse)` | Loop transformation hints |
| `@Data(Prefetch)` / `@Data(Restrict)` | Memory locality hints |
| `@Align(N)` / `@Pack` / `@ConstExpr` | Hardware layout and compile-time evaluation |

---

## Phase 3a: C++ Generator Implementation

Implement the C++ code generator with proper memory management strategy handling. Currently `Orchestrator::saveFile()` falls back to `PythonGenerator` for `.cpp` files with a `// TODO: Implement CppGenerator when ready` — this phase fills that gap.

### Step 39: Basic C++ generator skeleton
- Create `CppGenerator` class inheriting from `ProjectionGenerator` in `Generator.h`
- Implement basic concept mappings: Module → `namespace`, Function → function signature with return type
- Wire into `Orchestrator::saveFile()` to replace the PythonGenerator fallback
- Test: `generate(module)` → basic C++ skeleton with correct function signatures; assert output contains `namespace`, `{`, `}`

### Step 40: Statement generation for C++
- Implement Assignment → `target = value;`, Return → `return value;`, IfStatement → `if (...) {...} else {...}`
- Handle C++-specific syntax: semicolons, braces, no colons
- Test: AST with assignments/returns/if → valid C++ code; assert semicolons present, braces balanced

### Step 41: Expression generation for C++
- Implement BinaryOperation → `left op right`, VariableReference → `varName`, Literals → C++ equivalents
- Handle operator precedence with parentheses where needed
- Test: AST with nested expressions → valid C++ expressions; assert parenthesization is correct

### Step 42: Type generation for C++
- Implement PrimitiveType → `int`, `double`, `bool`, `std::string`, etc.
- Implement complex types: ListType → `std::vector<T>`, MapType → `std::map<K,V>`, SetType → `std::unordered_set<T>`, OptionalType → `std::optional<T>`
- Test: AST with typed parameters → correct C++ type declarations

### Step 43: Memory strategy code generation
- Refactor Sprint 2's `DerefStrategy` class into the canonical annotation types:
  - `@Deallocate(Explicit)` → raw pointers with explicit `new`/`delete`; flag missing deallocation points as "Missing Intent" errors
  - `@Lifetime(RAII)` → `std::unique_ptr<T>`, RAII destructors, move semantics; inject destructor calls at scope end
  - `@Reclaim(Tracing)` → `std::shared_ptr<T>` for GC-like reference counting; no explicit deallocation needed
  - `@Owner(Single)` → `std::unique_ptr<T>` with strict single-ownership enforcement; reject aliasing at compile time
  - `@Owner(Shared_ARC)` → `std::shared_ptr<T>` with deterministic ref-counting semantics
- Create new annotation classes: `DeallocateAnnotation`, `LifetimeAnnotation`, `ReclaimAnnotation`, `OwnerAnnotation`, `AllocateAnnotation`
- Keep `DerefStrategy` as deprecated wrapper that maps to the new types during transition
- Test: AST with each annotation type → C++ with correct memory management; `@Deallocate(Explicit)` produces raw pointers, `@Lifetime(RAII)` produces `unique_ptr`, `@Reclaim(Tracing)` produces `shared_ptr`

### Step 44: C++-specific idioms
- FunctionCall → function calls with proper argument passing (by value, by reference, by const reference)
- Handle C++-specific features: `const` correctness, `&` references, `&&` move references
- LangSpecific annotation for C++ → emit `#include` directives, `using namespace` declarations
- Test: AST → idiomatic C++ code; assert `const`, references appear where annotated

> **CHECKPOINT:** C++ generator produces valid, compilable code with correct memory management for all canonical annotation types. Stop here until this passes.

---

## Phase 3b: Tree-sitter Integration (Replace Placeholders)

Sprint 2 Steps 30-34 created placeholder tree-sitter integration in `Parser.h` with stub implementations. This phase replaces those stubs with real tree-sitter C bindings and proper CST-to-AST mapping.

### Step 45: Tree-sitter Python integration
- Link with `tree-sitter` and `tree-sitter-python` C libraries via CMake FetchContent
- Replace `TreeSitterParser::parsePython()` stub with real implementation
- Map Python CST nodes to SemAnno AST: `function_definition` → Function, `assignment` → Assignment, etc.
- Auto-annotate with `@Reclaim(Tracing)` since Python uses tracing GC
- Test: `parsePython("def f(x): return x + 1")` → AST with Function(name="f"), Parameter(name="x"), Return with BinaryOperation; memory annotation is `@Reclaim(Tracing)`

### Step 46: Tree-sitter C++ integration
- Link with `tree-sitter-cpp` library
- Replace `TreeSitterParser::parseCpp()` stub with real implementation
- Map C++ CST nodes: `function_definition` → Function, `declaration` → Variable, `if_statement` → IfStatement
- Detect memory patterns: `unique_ptr` → `@Lifetime(RAII)`, `shared_ptr` → `@Owner(Shared_ARC)`, raw `new/delete` → `@Deallocate(Explicit)`
- Test: `parseCpp("int f(int x) { return x + 1; }")` → correct AST; `std::unique_ptr<int> p` → `@Lifetime(RAII)` annotation

### Step 47: Tree-sitter Elisp integration
- Link with `tree-sitter-elisp` library
- Replace `TreeSitterParser::parseElisp()` stub with real implementation
- Map Elisp CST nodes: `defun` → Function, `defvar` → Variable, `if` → IfStatement
- Auto-annotate with `@Reclaim(Tracing)` (Elisp uses GC)
- Test: `parseElisp("(defun f (x) (+ x 1))")` → correct AST matching Python/C++ equivalents

### Step 48: CST to AST mapping refinement
- Handle complex constructs: nested functions, classes/structs, lambdas, comprehensions
- Preserve source location info (line/column) on AST nodes for error reporting
- Map language-specific constructs to nearest SemAnno equivalent with `LangSpecific` annotations for lossless round-trip
- Test: Complex multi-function Python/C++/Elisp → correct SemAnno AST with source locations

### Step 49: Error recovery and diagnostics
- Handle malformed source gracefully (partial parse, not crash)
- Report parse errors with line/column information and error node markers in AST
- Test: Invalid syntax → ParseResult with errors list and partial AST; assert no crashes, assert error positions correct

> **CHECKPOINT:** All three languages parse correctly to SemAnno AST via real tree-sitter. `parsePython` → AST → `PythonGenerator` produces semantically equivalent output. Stop here until this passes.

---

## Phase 3c: Classical Editing Mode

Add traditional text editing alongside the existing structured editing in the ImGui shell.

### Step 50: Text editor component
- Add raw text editor pane (ImGui multiline text input or integrate ImGuiColorTextEdit) alongside structured editor
- Toggle between structured view and classical text view
- Test: Can type text in classical mode; text persists across view switches

### Step 51: Text → AST synchronization
- When text changes, re-parse via tree-sitter and update the in-memory AST
- When AST changes (via structured editing or agent API), regenerate text and update the text pane
- Debounce re-parsing to avoid thrashing on every keystroke
- Test: Edit text → AST updates within debounce window; edit AST via structured editor → text updates immediately

### Step 52: Syntax highlighting
- Use tree-sitter's incremental parsing for syntax highlighting in the text pane
- Highlight keywords, strings, comments, types, function names with distinct colors
- Test: Python/C++/Elisp code highlighted correctly; assert color spans match tree-sitter node types

### Step 53: Classical editing operations
- Copy/paste, undo/redo, find/replace in text mode
- Text undo/redo integrates with orchestrator's operation journal (text edit → AST mutation → journal entry)
- Test: Ctrl+Z in text mode undoes both text and AST; Ctrl+F finds and replaces text

### Step 54: Emacs-style keybindings
- Ctrl+S → save, Ctrl+X Ctrl+F → open file, Ctrl+G → cancel, M-x → command palette
- Keybinding layer configurable (Emacs mode vs. standard mode)
- Test: Emacs keybindings trigger correct orchestrator RPCs

> **CHECKPOINT:** Classical text editing mode works with bidirectional AST sync. Users can freely switch between structured and text editing. Stop here until this passes.

---

## Phase 3d: Emacs Integration (Complete Placeholders)

Sprint 2 Steps 26-29 built Emacs daemon spawning, `sendToEmacs()`, `loadFile()`, and `saveFile()` in `Orchestrator.h`. This phase completes the integration with a proper splash screen, robust command routing, and real buffer management (Sprint 2 implementations use basic `emacsclient -e` calls without error handling or multi-buffer tracking).

### Step 55: Emacs splash screen
- Create `whetstone-splash.el` that displays on Emacs daemon startup
- Show Whetstone version, available commands, recent files, and keybinding cheat sheet
- Test: `startEmacsDaemon()` → Emacs buffer contains splash content; assert buffer name is `*Whetstone*`

### Step 56: Robust command integration
- Replace raw `sendToEmacs()` string concatenation with proper Elisp command builder (escape special chars, handle errors)
- `M-x whetstone-find-file` → `loadFile()` RPC, `C-x C-s` → `saveFile()` RPC
- Error handling: if Emacs daemon dies, detect and restart automatically
- Test: Emacs commands trigger orchestrator RPCs; simulate daemon crash → auto-restart

### Step 57: Buffer management
- Track open buffers in orchestrator (map of path → buffer state)
- Switch between buffers, save/close individual buffers
- Sync buffer list between Emacs and ImGui file tree
- Test: Open 3 files → all appear in buffer list; close one → removed from list; switch → correct content displayed

### Step 58: Mode-specific behavior
- `whetstone-python-mode`, `whetstone-cpp-mode`, `whetstone-elisp-mode` with language-appropriate features
- Mode activates based on file extension; sets tree-sitter parser and generator accordingly
- Test: Open `.py` → Python mode active; open `.cpp` → C++ mode active; verify correct generator used

> **CHECKPOINT:** Emacs integration is robust with splash screen, error recovery, multi-buffer management, and language-aware modes. Stop here until this passes.

---

## Phase 3e: Agent API (Extend Existing)

Sprint 2 Steps 34-38 built basic Agent API with RPC-based `insertNode`, `deleteNode`, `setProperty`, and `insertSubtree` in the orchestrator. This phase adds a proper network transport (WebSocket), authentication, and richer query capabilities for external AI agents.

### Step 59: WebSocket agent endpoint
- Add WebSocket server (via WebSocket++ or Boost.Beast) alongside existing stdin/stdout JSON-RPC
- Agents connect via `ws://localhost:PORT/agent` and send JSON-RPC messages
- Session management: track connected agents, assign session IDs
- Test: Agent connects via WebSocket, sends `ping`, receives `pong`; assert session ID assigned

### Step 60: AST query API
- `getAST(nodeId)` → return AST subtree as JSON (already exists as `getAST` RPC — extend with depth limit and filtering)
- `findNodes(pattern)` → pattern-based node search (by concept type, property values, annotation presence)
- `getSubtree(nodeId, depth)` → return limited-depth subtree for large ASTs
- Test: Agent queries return correct AST data; `findNodes({concept: "Function"})` returns all functions

### Step 61: AST mutation API (extend)
- Extend existing `insertNode`/`deleteNode`/`setProperty` with validation and lock checking
- `updateNode(nodeId, properties)` → bulk property update with journal recording
- All mutations check memory annotation consistency and `OptimizationLock` warnings
- Test: Agent mutations update AST correctly; locked node mutation produces warning (not rejection)

### Step 62: Context API
- `getInScopeSymbols(nodeId)` → return variables/functions/parameters visible at that AST position
- `getCallHierarchy(functionId)` → return callers and callees across the module
- `getDependencyGraph(nodeId)` → return data flow dependencies
- Test: Agent gets correct scope at different AST positions; call hierarchy matches actual function calls

### Step 63: Batch operations (extend)
- Extend existing `insertSubtree()` with full transactional `applySequence(mutations[])` → atomic all-or-nothing
- On failure mid-sequence, roll back all preceding mutations in the batch
- Test: Batch of 5 mutations applies atomically; batch with error at step 3 → all 5 rolled back

> **CHECKPOINT:** Agents can connect via WebSocket, query AST with rich patterns, mutate with validation, get scope/call context, and apply atomic batches. Stop here until this passes.

---

## Phase 3f: Advanced Memory Management

Enhanced memory strategy annotations using the canonical system from `annotations/Memory strategy.md`. Implements validation, inference, cross-language projection, and optimization hints.

### Step 64: Memory annotation validation
- Validate all canonical memory annotations for consistency:
  - `@Owner(Single)` on a variable that is aliased → error
  - `@Deallocate(Explicit)` without a corresponding deallocation point → "Missing Intent" error (the Unfilled Node Constraint)
  - `@Reclaim(Tracing)` on a real-time-critical path with `@Policy(Perf: Critical)` → warning
  - Conflicting annotations on parent/child (e.g., parent `@Owner(Single)`, child `@Owner(Shared_ARC)`) → error
- Test: Invalid/conflicting annotations flagged with specific error messages; valid annotations pass

### Step 65: Memory strategy inference
- Infer appropriate annotation from usage patterns (per `Memory strategy.md` Section 2):
  - Python source → `@Reclaim(Tracing)` (ref counting + cycle detection)
  - C++ `unique_ptr` patterns → `@Lifetime(RAII)` or `@Owner(Single)`
  - C++ `shared_ptr` patterns → `@Owner(Shared_ARC)`
  - C raw `malloc`/`free` → `@Deallocate(Explicit)`
  - Rust ownership patterns → `@Owner(Single)`
  - Immutable data with no mutation → `@Allocate(Static)` candidate
- Surface inferred annotations in the IDE as suggestions (not auto-applied)
- Test: System suggests correct strategies based on usage analysis; Python function → `@Reclaim(Tracing)` suggested

### Step 66: Cross-language memory projection
- Implement the "Lossless Projection Logic" from `Memory strategy.md` Section 3:
  - **High → Low** (Python → C): `@Reclaim(Tracing)` → inject `INC_REF`/`DEC_REF` shim with `free()` on zero
  - **Low → High** (C → Java): `@Deallocate(Explicit)` → wrap in `try-with-resources` or `Cleaner` API
  - **Strict → Permissive** (Rust → Python): `@Owner(Single)` → preserve in AST metadata but don't enforce in generated Python
  - **C++ specifics**: `@Lifetime(RAII)` → inject destructor calls at scope end; `@Owner(Shared_ARC)` → `std::shared_ptr`
- Preserve all annotations through round-trips (annotation survives even if target language doesn't enforce it)
- Test: Python module with `@Reclaim(Tracing)` → C++ with ref-counting shim → back to Python with annotation intact

### Step 67: Optimization annotations
- Implement annotations from `annotations/6 optimization and intent.md`:
  - `@Hot` / `@Cold` → C++ `[[likely]]`/`[[unlikely]]`, `__attribute__((hot))` / `__attribute__((cold))`
  - `@Inline(Always|Never|Hint)` → C++ `[[gnu::always_inline]]` / `[[gnu::noinline]]`
  - `@Pure` → C++ `[[gnu::pure]]` or `[[nodiscard]]`; enable aggressive constant folding
  - `@ConstExpr` → C++ `constexpr`
- Integrate with code generation for all three target languages
- Test: `@Hot` function → C++ output contains `__attribute__((hot))`; `@Pure` function → `[[nodiscard]]`; `@ConstExpr` → `constexpr`

> **CHECKPOINT:** Memory annotations are validated, inferred, projected across languages per the canonical spec, and optimization hints influence code generation. Stop here until this passes.

---

## Phase 3g: Optimization Pipeline

Transformation engine that operates on the AST respecting memory annotations and optimization locks.

### Step 68: Transformation engine
- AST transformation framework: define transforms as AST → AST functions with pre/post conditions
- Built-in transforms: constant folding, dead code elimination, loop invariant hoisting
- Each transform checks `OptimizationLock` before modifying (warn, don't reject)
- Respect `@Loop` annotations: `@Loop(Vectorize)` → don't break iteration independence; `@Loop(Fuse)` → attempt adjacent loop fusion
- Test: Constant folding simplifies `3 + 4` → `7` in AST; locked node emits warning but still transforms

### Step 69: Strategy-aware optimization
- Transforms respect memory annotations:
  - `@Owner(Single)` → can inline/move but not duplicate ownership (no aliasing)
  - `@Deallocate(Explicit)` → cannot reorder around deallocation points
  - `@Reclaim(Tracing)` → can freely restructure (GC handles cleanup)
  - `@Allocate(Static)` → cannot dynamically allocate; optimize for fixed buffers
- Respect `@Data` annotations: `@Data(Restrict)` allows more aggressive alias analysis
- Test: Optimization of `@Owner(Single)` variable doesn't create aliasing; `@Reclaim(Tracing)` enables free restructuring

### Step 70: Cross-strategy validation
- After optimization, verify memory annotation invariants still hold:
  - `@Deallocate(Explicit)`: no use-after-free, no leaked allocations
  - `@Owner(Single)`: no double-move, no aliasing
  - `@Owner(Shared_ARC)`: no circular references without weak refs
  - `@Lifetime(RAII)`: destructor reachable on all code paths
- Emit diagnostics with AST node references
- Test: Intentionally break invariants → validator catches each violation type

### Step 71: Incremental optimization with rollback
- Apply optimizations incrementally, recording each transform in the operation journal
- Undo individual transforms or entire optimization passes
- Track provenance: each optimized node knows which transform created it
- Test: Apply 3 transforms → undo middle one → AST reflects only transforms 1 and 3

> **CHECKPOINT:** Optimization pipeline transforms AST safely, respects locks and memory annotations, validates invariants, and supports incremental rollback. Stop here until this passes.

---

## Phase 3h: Integration & Validation

Full system integration testing and documentation. Consolidated from original 7 steps to 4 focused steps.

### Step 72: End-to-end pipeline test
- Full workflow: Python file → tree-sitter parse → AST → optimize → C++ generation → compile with g++/clang
- Verify: generated C++ compiles without errors and produces correct output
- Round-trip: Python → AST → C++ → AST → Python preserves semantics and all memory annotations
- Test: `Calculator.py` → parse → optimize → `Calculator.cpp` → compile → run → correct output; round-trip preserves annotations

### Step 73: Error handling and edge cases
- Graceful handling of all error conditions across the full pipeline
- Edge cases: empty modules, deeply nested ASTs, circular references, very large files
- Error reporting with source location, AST node context, and actionable messages
- Test: Each error condition → meaningful error message (not crash); recovery to valid state

### Step 74: Performance profiling and benchmarks
- Profile bottlenecks: parsing time, generation time, optimization time, UI responsiveness
- Benchmark generated code vs. hand-written equivalents
- Set baseline metrics for future regression testing
- Test: Parse/generate cycle completes in < 100ms for typical module; UI stays responsive during background operations

### Step 75: API documentation
- Document all RPC methods (JSON-RPC + WebSocket agent API) with request/response schemas
- Document all canonical memory annotations with examples for each language target
- Document optimization transforms with before/after AST examples
- Test: Every public RPC method has a documented example that can be copy-pasted and run

> **CHECKPOINT:** Complete system validated end-to-end. All features work together. Documentation complete. Stop here — Sprint 3 is done.

---

## Summary

| Phase | Global Steps | What You Have When Done |
|-------|-------------|------------------------|
| 3a: C++ Generator | 39–44 | C++ code generator with canonical memory annotation support |
| 3b: Tree-sitter (replace stubs) | 45–49 | Real parsing for Python, C++, and Elisp via tree-sitter C bindings |
| 3c: Classical Mode | 50–54 | Traditional text editing with bidirectional AST sync |
| 3d: Emacs Integration (complete) | 55–58 | Robust Emacs integration with splash screen, error recovery, multi-buffer |
| 3e: Agent API (extend) | 59–63 | WebSocket-based agent API with rich queries, validation, atomic batches |
| 3f: Memory Management | 64–67 | Canonical memory annotation validation, inference, cross-language projection, `@Hot`/`@Cold`/`@Inline`/`@Pure` |
| 3g: Optimization | 68–71 | Strategy-aware optimization pipeline with incremental rollback |
| 3h: Integration | 72–75 | End-to-end validation, profiling, and documentation |

## Dependencies

| Dependency | Version | Purpose |
|------------|---------|---------|
| tree-sitter | 0.22+ | Parsing Python, C++, and Elisp (C library) |
| tree-sitter-python | latest | Python grammar |
| tree-sitter-cpp | latest | C++ grammar |
| tree-sitter-elisp | latest | Elisp grammar |
| Dear ImGui | 1.90+ | GUI rendering |
| SDL2 | latest | Window/input backend |
| WebSocket++ | latest | Agent API network transport |
| spdlog | latest | Logging |

## Migration Notes

### Sprint 2 `DerefStrategy` → Sprint 3 Canonical Annotations

Sprint 2 used a single `DerefStrategy` class with a `strategy` string field. Sprint 3 replaces this with the canonical annotation classes from `annotations/Memory strategy.md`:

| Sprint 2 (`DerefStrategy.strategy`) | Sprint 3 Annotation Class | C++ Code Generation |
|---|---|---|
| `"imperative"` | `DeallocateAnnotation(Explicit)` | Raw pointers, explicit `new`/`delete` |
| `"streamed"` | `LifetimeAnnotation(RAII)` + `OwnerAnnotation(Single)` | `unique_ptr`, RAII destructors, move semantics |
| `"batched"` | `ReclaimAnnotation(Tracing)` + `OwnerAnnotation(Shared_ARC)` | `shared_ptr`, reference counting |
| `"content-addressed"` | `AllocateAnnotation(Static)` + `@ConstExpr` | `const` immutable objects, compile-time where possible |

### New annotation classes to implement in Step 43:

```
DeallocateAnnotation  — strategy: "Explicit"
LifetimeAnnotation    — strategy: "RAII"
ReclaimAnnotation     — strategy: "Tracing" | "Cycle" | "Escape"
OwnerAnnotation       — strategy: "Single" | "Shared_ARC"
AllocateAnnotation    — strategy: "Static" | "Register" | "Allocator"
```

### Backward compatibility

- Keep `DerefStrategy` as a deprecated typedef/wrapper that maps old strategy strings to new annotation types
- Serialization: read both old `"DerefStrategy"` JSON nodes and new annotation names
- Generators: accept both old and new annotations, emit only new annotation names in output
- Sprint 2 test files are not modified (they use the old naming and are stubs anyway)

### Optimization annotation classes to implement in Step 67:

From `annotations/6 optimization and intent.md`:

```
HotColdAnnotation     — hint: "Hot" | "Cold"
InlineAnnotation      — mode: "Always" | "Never" | "Hint"
PureAnnotation        — (no parameters)
TailCallAnnotation    — (no parameters)
LoopAnnotation        — transform: "Unroll(N)" | "Vectorize" | "Fuse"
DataAnnotation        — hint: "Prefetch" | "Restrict"
AlignAnnotation       — bytes: N
PackAnnotation        — (no parameters)
ConstExprAnnotation   — (no parameters)
```
