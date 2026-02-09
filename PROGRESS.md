# Whetstone DSL - Progress Tracker

> **Purpose:** Living document for cross-session, cross-LLM continuity.
> Updated after each work session. The git log is the authoritative source of truth.

---

## Project Overview

Whetstone is a semantic annotation DSL (SemAnno) and structured editor for cross-language code generation with memory strategy annotations. The core idea: annotate code with memory management intent (`@Reclaim(Tracing)`, `@Owner(Single)`, `@Lifetime(RAII)`, etc.) and generate correct target-language code from a single AST representation.

**Stack:** C++20, Dear ImGui + SDL2 + OpenGL3, nlohmann-json, vcpkg, CMake

---

## Sprint Summary

| Sprint | Steps | Status | Description |
|--------|-------|--------|-------------|
| Sprint 1 | — | Complete | MPS-based prototype (JetBrains MPS language plugin) |
| Sprint 2 | 1–38 | **Complete** | C++ editor stack: AST, serialization, generators, ImGui shell, orchestrator, agents |
| Sprint 3 | 39–75 | **Complete** | Core functionality: C++ generator, tree-sitter, classical editing, optimization |
| Sprint 4 | 76–126 | **In Progress** | Professional editor: layout presets, code editor, LSP, annotation UI |

---

## Sprint 1: MPS Prototype (Complete)

Built the initial SemAnno language in JetBrains MPS:
- 33 AST concepts (Module, Function, Variable, statements, expressions, types, annotations)
- Editor definitions for all concepts
- TextGen definitions for Python output
- Phase1Example sandbox model

**Key files:** `languages/SemAnno/`, `solutions/SemAnno.sandbox/`

---

## Sprint 2: C++ Editor Stack (Steps 1–38) — COMPLETE

All 38 steps implemented and passing. Each step has a corresponding test (`step1_test.exe` through `step38_test.exe`).

### Phase 2a: AST in C++ (Steps 1–6)
- [x] Step 1: Base `ASTNode` class + `Module` concept
- [x] Step 2: Child links, `Function` and `Variable`
- [x] Step 3: All Statement, Expression, Type, Parameter concepts
- [x] Step 4: Annotation concepts (`DerefStrategy`, `OptimizationLock`, `LangSpecific`)
- [x] Step 5: JSON serialization (`toJson`)
- [x] Step 6: JSON deserialization + round-trip verification

### Phase 2b: Validation & Generation (Steps 7–11)
- [x] Step 7: Schema validation (`ASTSchema`)
- [x] Step 8: Python generator base (Module/Function)
- [x] Step 9: Python generator — statements and expressions
- [x] Step 10: Python generator — remaining concepts
- [x] Step 11: Python generator — annotation output

### Phase 2c: ImGui Editor Shell (Steps 12–15)
- [x] Step 12: Dear ImGui shell scaffolding (SDL2 + OpenGL3)
- [x] Step 13: Text viewport
- [x] Step 14: AST-to-text viewport
- [x] Step 15: Projection toggle

### Phase 2d: Orchestrator (Steps 16–22)
- [x] Step 16: Orchestrator process
- [x] Step 17: JSON-RPC server
- [x] Step 18: AST mutation via RPC
- [x] Step 19: Connect ImGui to orchestrator
- [x] Step 20: Mutation via UI
- [x] Step 21: Undo/Redo journal
- [x] Step 22: Undo/Redo UI

### Phase 2e: Emacs Integration (Steps 23–29)
- [x] Step 23: Emacs integration
- [x] Step 24: Orchestrator spawns Emacs
- [x] Step 25: AST-to-Elisp projection
- [x] Step 26: Orchestrator-to-Emacs RPC
- [x] Step 27: File operations via Emacs
- [x] Step 28: File operations via RPC
- [x] Step 29: AST-to-File synchronization

### Phase 2f: Round-trip & Agents (Steps 30–38)
- [x] Step 30: Emacs-to-MPS synchronization
- [x] Step 31: Tree-sitter integration (stubs)
- [x] Step 32: Import via tree-sitter
- [x] Step 33: Export via generator
- [x] Step 34: Full round-trip / C++ generator basic output
- [x] Step 35: C++ generator — memory strategies
- [x] Step 36: Agent API
- [x] Step 37: Agent batch mode
- [x] Step 38: OptimizationLock warnings

---

## Sprint 3: Core Functionality (Steps 39–75) — COMPLETE

### Phase 3a: C++ Generator Implementation (Steps 39–44) — COMPLETE
- [x] Step 39: CppGenerator basic skeleton
- [x] Step 40: Statement generation (Assignment, Return, If)
- [x] Step 41: Expression generation (BinaryOp, literals, precedence)
- [x] Step 42: Type generation (PrimitiveType → C++ types, containers)
- [x] Step 43: Memory strategy code generation (canonical annotations)
- [x] Step 44: C++-specific idioms (const, references, includes)

> Steps 39–44 have TDD tests that compile and pass. The CppGenerator and canonical
> memory annotation classes (`DeallocateAnnotation`, `LifetimeAnnotation`,
> `ReclaimAnnotation`, `OwnerAnnotation`, `AllocateAnnotation`) are implemented in
> `editor/src/ast/Generator.h`.

### Phase 3b: Tree-sitter Integration (Steps 45–49) — COMPLETE
- [x] Step 45: Real tree-sitter Python parsing (6/6 tests pass)
- [x] Step 46: Real tree-sitter C++ parsing with memory pattern detection (7/7 tests pass)
- [x] Step 47: Real tree-sitter Elisp parsing + cross-language equivalence (6/6 tests pass)
- [x] Step 48: CST-to-AST mapping refinement — default params, if/for, multi-statement, void, multi-function, string literals (8/8 tests pass)
- [x] Step 49: Error recovery and diagnostics — ParseResult/ParseDiagnostic types, partial parse, hasErrors() (7/7 tests pass)

> Tree-sitter core via vcpkg (`tree-sitter` 0.26.5). Language grammars via FetchContent:
> `tree-sitter-python` v0.23.6, `tree-sitter-cpp` v0.23.4, `tree-sitter-elisp` 1.3.0.
> `TreeSitterParser` in `Parser.h` implements full CST-to-AST conversion for all three languages
> with auto-annotation (Python/Elisp → @Reclaim(Tracing), C++ → memory pattern detection).

### Phase 3c: Classical Editing Mode (Steps 50–54) — COMPLETE
- [x] Step 50: **IMPLEMENTED** — TextEditor component + TextASTSync (8/8 tests pass)
- [x] Step 51: **IMPLEMENTED** — Text↔AST bidirectional sync with debounce (5/5 tests pass)
- [x] Step 52: **IMPLEMENTED** — Syntax highlighting via tree-sitter CST walk (8/8 tests pass)
- [x] Step 53: **IMPLEMENTED** — Classical editing ops: undo/redo, find/replace, selection (6/6 tests pass)
- [x] Step 54: **IMPLEMENTED** — Configurable keybinding profiles: VSCode (default), JetBrains, Emacs (10/10 tests pass)

> Phase 3c complete (37/37 tests pass). Backend components:
> - `TextASTSync.h`: bidirectional text↔AST sync with debounce
> - `TextEditor.h`: edit ops, undo/redo with AST tracking, find/replace, selection
> - `SyntaxHighlighter.h`: tree-sitter CST walk → colored spans (keyword, string, comment, etc.)
> - `KeybindingManager.h`: swappable profiles (VSCode/JetBrains/Emacs), custom overrides
>
> **Design direction:** Editor UI targets VSCode/JetBrains look-and-feel (not Emacs).
> Keybinding profile is selectable at startup or via settings. VSCode is default.

### Phase 3d: Editor Infrastructure (Steps 55–58) — COMPLETE
- [x] Step 55: **IMPLEMENTED** — WelcomeScreen component (actions, recent files, tips, visibility toggle) (7/7 tests pass)
- [x] Step 56: **IMPLEMENTED** — ElispCommandBuilder (escape, findFile, saveBuffer) + MockEmacsConnection (daemon lifecycle) (6/6 tests pass)
- [x] Step 57: **IMPLEMENTED** — BufferManager (open/close/switch, multi-buffer tracking, language/modified info) (7/7 tests pass)
- [x] Step 58: **IMPLEMENTED** — EditorMode (per-language indent, comment toggle, bracket pairs, snippets for Python/C++/Elisp) (8/8 tests pass)

### Phase 3e: Agent API Extend (Steps 59–63) — COMPLETE
- [x] Step 59: **IMPLEMENTED** — WebSocketAgentServer with session management, JSON-RPC routing, MockWebSocketTransport (10/10 tests pass)
- [x] Step 60: **IMPLEMENTED** — ASTQueryAPI with getAST, findNodesByType/Property/Annotation, getSubtree (8/8 tests pass)
- [x] Step 61: **IMPLEMENTED** — ASTMutationAPI with setProperty, updateNode, deleteNode, insertNode, journal recording, OptimizationLock warning (6/6 tests pass)
- [x] Step 62: **IMPLEMENTED** — ContextAPI with getInScopeSymbols (scope walk), getCallHierarchy (caller/callee resolution), getDependencyGraph (VariableRef data flow) (6/6 tests pass)
- [x] Step 63: **IMPLEMENTED** — BatchMutationAPI with atomic applySequence, reverse-order rollback on failure, journal recording (5/5 tests pass)

### Phase 3f: Advanced Memory Management (Steps 64–67) — COMPLETE
- [x] Step 64: **IMPLEMENTED** — AnnotationValidator: @Deallocate(Explicit) missing-intent check, @Owner(Single) alias detection, parent/child conflict detection (5/5 tests pass)
- [x] Step 65: **IMPLEMENTED** — MemoryStrategyInference: language-based defaults (Python/Elisp→Tracing, C→Explicit, Rust→Single), per-function pattern analysis, confidence scores (5/5 tests pass)
- [x] Step 66: **IMPLEMENTED** — CrossLanguageProjector: deep-copy AST projection with language change, annotation-aware CppGenerator (shared_ptr for @Reclaim(Tracing), unique_ptr for @Lifetime(RAII)), round-trip annotation preservation (5/5 tests pass)
- [x] Step 67: **IMPLEMENTED** — Optimization annotations: HotColdAnnotation(@Hot→__attribute__((hot)), @Cold→__attribute__((cold))), InlineAnnotation(@Inline(Always)→[[gnu::always_inline]]), PureAnnotation(@Pure→[[nodiscard]]), ConstExprAnnotation(@ConstExpr→constexpr); all 3 generators updated (9/9 tests pass)

### Phase 3g: Optimization Pipeline (Steps 68–71) — COMPLETE
- [x] Step 68: **IMPLEMENTED** — TransformEngine: constant folding (BinaryOp on IntegerLiterals), dead code elimination (after Return), OptimizationLock warning (4/4 tests pass)
- [x] Step 69: **IMPLEMENTED** — StrategyAwareOptimizer: annotation-constrained optimization (@Reclaim→allow, @Owner(Single)→block duplication, @Deallocate(Explicit)→block reorder, @Allocate(Static)→block dynamic alloc) (4/4 tests pass)
- [x] Step 70: **IMPLEMENTED** — StrategyValidator: post-optimization invariant checking (use-after-free, leak, aliasing under @Owner(Single), clean code passes) (5/5 tests pass)
- [x] Step 71: **IMPLEMENTED** — IncrementalOptimizer: transform journal with unique IDs, undoLast, undoTransform(id), provenance tracking (4/4 tests pass)

### Phase 3h: Integration & Validation (Steps 72–75) — COMPLETE
- [x] Step 72: **IMPLEMENTED** — Pipeline: end-to-end parse→infer→validate→optimize→generate for Python and C++, cross-language projection, constant folding in pipeline (6/6 tests pass)
- [x] Step 73: **IMPLEMENTED** — Error handling: empty AST, null roots, unannotated code, nonexistent node IDs, empty history, empty module (8/8 tests pass)
- [x] Step 74: **IMPLEMENTED** — Performance benchmarks: 1000-function AST creation (1ms), 500-function constant fold (0ms), 100-variable leak detection (0ms), JSON round-trip 100 functions (4ms), 50 incremental transforms+undo (1ms) (6/6 tests pass)
- [x] Step 75: **IMPLEMENTED** — APIDocGenerator: structured docs for 23 components across 6 categories, markdown generation, coverage verification (6/6 tests pass)

---

## Sprint 4: Professional Editor (Steps 76–126) — In Progress

### Phase 4a: Layout & Code Editor Core (Steps 76–83) — In Progress
- [x] Step 76: **IMPLEMENTED** — LayoutManager: 3 preset docking layouts (VSCode/Emacs/JetBrains), panel visibility/ratio queries, dirty flag for rebuild, save/load persistence, preset name round-trip (10/10 tests pass)
- [x] Step 77: **IMPLEMENTED** — Custom CodeEditorWidget renderer: per-token coloring, cursor/selection/input handling, monospace grid, blinking cursor, visible whitespace toggle (3/3 tests pass)
- [x] Step 78: **IMPLEMENTED** — Line numbers and gutter: fixed-width gutter with right-aligned line numbers, current line highlight, gutter click selects line (2/2 tests pass)
- [x] Step 79: **IMPLEMENTED** — Auto-indent and smart editing: EditorMode integration, enter auto-indent, tab inserts spaces, auto-close brackets/quotes (3/3 tests pass)
- [x] Step 80: **IMPLEMENTED** — Scroll/selection/clipboard: shift-extend selection, double/triple click selection, Ctrl+C/X/V clipboard, drag select (3/3 tests pass)
- [x] Step 81: **IMPLEMENTED** — Code folding: tree-sitter fold regions, gutter fold toggles, hidden ranges with placeholders (2/2 tests pass)
- [x] Step 82: **IMPLEMENTED** — Minimap: right-side overview with viewport indicator and click-to-scroll (1/1 tests pass)
- [x] Step 83: **IMPLEMENTED** — Additional tree-sitter grammars (JS/TS/Java/Rust/Go), new EditorMode configs, syntax highlighting for 8 languages (2/2 tests pass)

### Phase 4b: File Management (Steps 84–89) — In Progress
- [x] Step 84: **IMPLEMENTED** — Native file dialogs via tinyfiledialogs, FileDialog wrapper with test provider injection (2/2 tests pass)
- [x] Step 85: **IMPLEMENTED** — Filesystem tree with .gitignore filtering, recursive Explorer rendering (1/1 tests pass)
- [x] Step 86: **IMPLEMENTED** — Multi-tab editing with BufferManager and per-buffer editor state (3/3 tests pass)
- [x] Step 87: **IMPLEMENTED** — Welcome screen wired with recent files persistence and quick actions (2/2 tests pass)
- [x] Step 88: **IMPLEMENTED** — Drag-and-drop file/folder open via SDL_DROPFILE (2/2 tests pass)
- [x] Step 89: **IMPLEMENTED** — File watcher polling with auto-reload for clean buffers (1/1 tests pass)

### Phase 4c: LSP Client & Diagnostics (Steps 90–96) — In Progress
- [x] Step 90: **IMPLEMENTED** — LSPClient core with JSON-RPC initialize/shutdown, injectable transport (2/2 tests pass)
- [x] Step 91: **IMPLEMENTED** — LSP diagnostics: didOpen/didChange/didSave notifications, publishDiagnostics parsing/storage, Problems panel UI (1/1 tests pass)

---

## Build Infrastructure

Created in the most recent session:

| File | Purpose |
|------|---------|
| `editor/vcpkg.json` | vcpkg manifest (dependencies declaration) |
| `editor/CMakePresets.json` | Standardized CMake presets (Windows/Linux) |
| `installer/windows/build.ps1` | PowerShell build script (prerequisites, vcpkg, cmake, staging) |
| `installer/windows/setup.iss` | Inno Setup installer script (produces `.exe` installer) |
| `installer/linux/build.sh` | Bash build script for Linux |
| `installer/linux/install.sh` | Linux install script (system deps, desktop entry) |

### Building on Windows

```powershell
# Prerequisites: VS2022, CMake 3.20+, vcpkg at E:\vcpkg
cd E:\Whetstone_DSL\installer\windows
.\build.ps1 -Config Release -VcpkgRoot E:\vcpkg

# Then compile installer (requires Inno Setup 6):
& "C:\Users\Bill\AppData\Local\Programs\Inno Setup 6\ISCC.exe" setup.iss
```

### vcpkg Packages Required
- `nlohmann-json:x64-windows`
- `sdl2:x64-windows`
- `imgui[docking-experimental,opengl3-binding]:x64-windows`
- `glad:x64-windows`
- `tree-sitter:x64-windows`

### Known Issue: imgui SDL2 Backend
vcpkg's imgui 1.91.9 removed the `sdl2-binding` feature (only `sdl3-binding` exists). The SDL2 backend files are vendored locally in `editor/src/imgui_backends/` and compiled directly by CMake.

---

## Test Results (Last Verified)

**Steps 1–49:** All compile and pass (49 executables in `editor/build/Release/`)
**Steps 50–54:** All compile and pass (step50: 8/8, step51: 5/5, step52: 8/8, step53: 6/6, step54: 10/10)
**Steps 55–58:** All compile and pass (step55: 7/7, step56: 6/6, step57: 7/7, step58: 8/8)
**Step 60:** Compile and pass (8/8)
**Step 59:** Compile and pass (10/10)
**Step 61:** Compile and pass (6/6)
**Step 62:** Compile and pass (6/6)
**Step 63:** Compile and pass (5/5)
**Step 64:** Compile and pass (5/5)
**Step 65:** Compile and pass (5/5)
**Steps 66–67:** Compile and pass (step66: 5/5, step67: 9/9)
**Steps 68–71:** All compile and pass (step68: 4/4, step69: 4/4, step70: 5/5, step71: 4/4)
**Steps 72–75:** All compile and pass (step72: 6/6, step73: 8/8, step74: 6/6, step75: 6/6)
**Step 76:** Compile and pass (10/10)
**Step 77:** Compile and pass (3/3)
**Step 78:** Compile and pass (2/2)
**Step 79:** Compile and pass (3/3)
**Step 80:** Compile and pass (3/3)
**Step 81:** Compile and pass (2/2)
**Step 82:** Compile and pass (1/1)
**Step 83:** Compile and pass (2/2)
**Step 84:** Compile and pass (2/2)
**Step 85:** Compile and pass (1/1)
**Step 86:** Compile and pass (3/3)
**Step 87:** Compile and pass (2/2)
**Step 88:** Compile and pass (2/2)
**Step 89:** Compile and pass (1/1)
**Step 90:** Compile and pass (2/2)
**Step 91:** Compile and pass (1/1)

---

## Key Source Files

| File | Contents |
|------|----------|
| `editor/src/ast/ASTNode.h` | All 33+ AST node classes, JSON serialization |
| `editor/src/ast/Generator.h` | PythonGenerator, CppGenerator, ElispGenerator, all canonical annotation classes |
| `editor/src/ast/Schema.h` | AST schema validation |
| `editor/src/ast/Parser.h` | TreeSitterParser (real tree-sitter CST-to-AST for Python/C++/Elisp) |
| `editor/src/TextASTSync.h` | Bidirectional text↔AST synchronization with debounce |
| `editor/src/TextEditor.h` | Classical text editor: edit ops, undo/redo, find/replace, selection |
| `editor/src/SyntaxHighlighter.h` | Tree-sitter CST walk → colored token spans (Python/C++/Elisp) |
| `editor/src/KeybindingManager.h` | Configurable keybinding profiles (VSCode/JetBrains/Emacs) |
| `editor/src/WelcomeScreen.h` | Welcome screen component (actions, recent files, tips) |
| `editor/src/EmacsIntegration.h` | ElispCommandBuilder + EmacsConnection/MockEmacsConnection |
| `editor/src/BufferManager.h` | Multi-buffer management (open/close/switch/track) |
| `editor/src/EditorMode.h` | Per-language editor behavior (indent, comment, brackets, snippets) |
| `editor/src/WebSocketServer.h` | WebSocket agent endpoint: transport abstraction, session management, JSON-RPC routing |
| `editor/src/ASTMutationAPI.h` | AST mutation API: setProperty, updateNode, deleteNode, insertNode with lock checking and journal |
| `editor/src/ContextAPI.h` | Context API: scope analysis, call hierarchy, data-flow dependency graph |
| `editor/src/BatchMutationAPI.h` | Atomic batch mutations with reverse-order rollback on failure |
| `editor/src/AnnotationValidator.h` | Memory annotation validation: missing intent, alias detection, conflict checking |
| `editor/src/MemoryStrategyInference.h` | Memory strategy inference: language defaults, pattern analysis, confidence scoring |
| `editor/src/TransformEngine.h` | AST transformation engine: constant folding, dead code elimination, OptimizationLock |
| `editor/src/StrategyAwareOptimizer.h` | Annotation-constrained optimization (respects @Owner, @Deallocate, @Allocate, @Reclaim) |
| `editor/src/StrategyValidator.h` | Post-optimization invariant validation (use-after-free, leak, aliasing) |
| `editor/src/IncrementalOptimizer.h` | Incremental transforms with journal, undo by ID, provenance tracking |
| `editor/src/Pipeline.h` | End-to-end pipeline: parse → infer → validate → optimize → generate |
| `editor/src/APIDocGenerator.h` | Structured API documentation for all 23 components |
| `editor/src/LayoutManager.h` | Docking layout presets (VSCode/Emacs/JetBrains), panel queries, persistence |
| `editor/src/Orchestrator.h` | Orchestrator: Emacs integration, file ops, undo/redo, agent API |
| `editor/src/main.cpp` | ImGui editor shell (SDL2 + OpenGL3, VSCode Dark theme, docking) |
| `editor/src/orchestrator_main.cpp` | Orchestrator standalone process (JSON-RPC server) |
| `editor/CMakeLists.txt` | CMake build config (vcpkg-based) |

---

## Architecture Notes

- **Editor** (`whetstone_editor.exe`): ImGui-based GUI with VSCode Dark theme. Docking layout: Explorer (left), Editor with tabs (center), Panel with Output/AST/Highlighted/Generated tabs (bottom), blue status bar. Editable text via InputTextMultiline backed by TextEditor + TextASTSync. Syntax-highlighted preview, live AST view, generated code preview. Configurable keybinding profiles (VSCode/JetBrains/Emacs). Find/Replace dialog. File open/save with language auto-detection.
- **Orchestrator** (`orchestrator.exe`): Standalone JSON-RPC server. Manages AST state, undo/redo journal, file I/O, Emacs daemon integration, agent API.
- **Communication**: Editor ↔ Orchestrator via JSON-RPC over stdin/stdout pipes. When launched standalone (no pipe), the editor runs in disconnected mode.
- **Generators**: AST → Python, C++, Elisp text output. C++ generator handles canonical memory annotations.
- **Parsers**: Text → AST via tree-sitter (currently stub implementations from Sprint 2).

---

## What's Next

Sprint 4 in progress. Step 76 (LayoutManager) done. Next: Step 77 (custom code editor renderer).

---

## Session Log

| Date | Agent | Work Done |
|------|-------|-----------|
| Pre-Sprint 2 | Multiple | Sprint 1 MPS prototype, language definitions, textgen |
| Sprint 2 | Multiple | Steps 1–38 implemented sequentially |
| Sprint 3 start | Unknown | Sprint 3 plan written, TDD stubs for steps 39–71 committed |
| Sprint 3 | Unknown | Phase 3a steps 39–44 implemented (CppGenerator + canonical annotations) |
| Sprint 3 | Unknown | Steps 34–38 re-committed with different implementations (agent API, batch mode) |
| 2025-latest | Claude | Build infrastructure: vcpkg.json, CMakePresets.json, build.ps1, setup.iss, Linux scripts |
| 2025-latest | Claude | Fixed 7 build errors (ElispGenerator, Orchestrator, main.cpp, orchestrator_main.cpp) |
| 2025-latest | Claude | Vendored imgui SDL2 backend (vcpkg removed sdl2-binding feature) |
| 2025-latest | Claude | Created Windows installer (Inno Setup), verified it installs and runs |
| 2025-latest | Claude | Fixed editor hang when launched without orchestrator pipe |
| 2026-02-07 | Claude Opus 4.6 | Step 60: Implemented ASTQueryAPI (tree walk, JSON output, find by type/property/annotation). 8/8 tests pass. Added PROGRESS.md. |
| 2026-02-08 | Claude Opus 4.6 | Phase 3b: Real tree-sitter integration (Steps 45–49). Replaced Parser.h stubs with real tree-sitter C bindings. Python/C++/Elisp CST-to-AST conversion, memory pattern detection, error recovery. 34/34 tests pass. |
| 2026-02-08 | Claude Opus 4.6 | Phase 3c: Steps 50–53. TextASTSync (bidirectional text↔AST sync with debounce) and TextEditor (edit ops, undo/redo with AST tracking, find/replace, selection). 19/19 tests pass. Fixed step53 find-position bug (was off-by-one). |
| 2026-02-08 | Claude Opus 4.6 | Phase 3c complete: Steps 52+54. SyntaxHighlighter (tree-sitter CST→color spans for Python/C++/Elisp). KeybindingManager (VSCode/JetBrains/Emacs profiles, default VSCode). 37/37 total Phase 3c tests pass. Design pivot: editor targets VSCode/JetBrains look, not Emacs. |
| 2026-02-08 | Claude Opus 4.6 | GUI wiring: Rewrote main.cpp from Step 12 scaffold to functional editor. VSCode Dark theme, docking layout, editable text with TextEditor/TextASTSync, syntax-highlighted preview, live AST view, generated code tab, find/replace, keybinding profile selector, status bar. Fixed WMOD_ prefix (Windows MOD_SHIFT/MOD_ALT macro conflicts). whetstone_editor.exe builds and links. |
| 2026-02-08 | Claude Opus 4.6 | Phase 3d complete: Steps 55–58. WelcomeScreen (actions, recent files, tips). ElispCommandBuilder+MockEmacsConnection (Elisp escaping, daemon lifecycle). BufferManager (multi-file open/close/switch). EditorMode (per-language indent/comment/brackets/snippets for Python/C++/Elisp). 28/28 tests pass. |
| 2026-02-08 | Claude Opus 4.6 | Step 59: WebSocketAgentServer with transport abstraction (WebSocketTransport base + MockWebSocketTransport), session management (connect/disconnect/unique IDs), JSON-RPC routing (ping/pong, setAgentName, listSessions, getSessionInfo), custom handler delegation. 10/10 tests pass. |
| 2026-02-08 | Claude Opus 4.6 | Step 61: ASTMutationAPI with setProperty, updateNode (bulk), deleteNode, insertNode. OptimizationLock walk-up warning (warn, don't reject). Operation journal. Added removeChild() to ASTNode. 6/6 tests pass. |
| 2026-02-08 | Claude Opus 4.6 | Step 62: ContextAPI with getInScopeSymbols (ancestor scope walk: function params/locals + module vars/functions), getCallHierarchy (FunctionCall scan → caller/callee resolution), getDependencyGraph (VariableReference → declaration lookup). 6/6 tests pass. |
| 2026-02-08 | Claude Opus 4.6 | Step 63: BatchMutationAPI with atomic applySequence. Captures undo closures per mutation; on failure, rolls back in reverse order. Supports setProperty/insertNode/deleteNode. Journal recorded on success. Phase 3e complete (35/35 tests across steps 59–63). 5/5 tests pass. |
| 2026-02-08 | Claude Opus 4.6 | Step 64: AnnotationValidator. @Deallocate(Explicit) → checks for dealloc evidence (FunctionCall to delete/free), emits "Missing Intent" error if absent. @Owner(Single) → scans for aliasing assignments to local vars. Conflicting same-family annotations on parent/child → conflict error. 5/5 tests pass. |
| 2026-02-08 | Claude Opus 4.6 | Step 65: MemoryStrategyInference. Language-based defaults (Python/Elisp/Ruby/JS/Java→Tracing, C→Explicit, Rust→Single, Swift→Shared_ARC, C++→RAII). Per-function alloc/dealloc pattern scan. Confidence scores. Read-only (never modifies AST). 5/5 tests pass. |
| 2026-02-09 | Claude Opus 4.6 | Phase 3g complete: Optimization Pipeline (Steps 68–71). TransformEngine (constant folding, DCE, OptimizationLock warning). StrategyAwareOptimizer (annotation-constrained: @Reclaim allows, @Owner blocks duplication, @Deallocate blocks reorder, @Allocate blocks dynamic). StrategyValidator (use-after-free, leak, aliasing detection). IncrementalOptimizer (journal, undo by ID, provenance). 17/17 tests pass. |
| 2026-02-09 | Claude Opus 4.6 | Phase 3h complete: Integration & Validation (Steps 72–75). Pipeline (end-to-end parse→infer→validate→optimize→generate across Python/C++). Error handling (8 edge cases: null roots, empty ASTs, nonexistent IDs). Performance benchmarks (1000-fn AST in 1ms, JSON round-trip 4ms). APIDocGenerator (23 components, 6 categories, markdown output). 26/26 tests pass. **Sprint 3 complete: all 75 steps done.** |
| 2026-02-09 | Claude Opus 4.6 | Step 76: LayoutManager with 3 preset docking layouts (VSCode/Emacs/JetBrains). Panel visibility/ratio queries, dirty flag, save/load persistence. 10/10 tests pass. Sprint 4 started. |
| 2026-02-09 | Codex | Step 77: Custom code editor renderer (CodeEditorWidget). Per-token coloring, cursor/selection/input, blinking cursor, whitespace toggle. 3/3 tests pass. |
| 2026-02-09 | Codex | Step 78: Line numbers and gutter. Gutter width auto-adjusts, current line highlight, gutter click selects line. 2/2 tests pass. |
| 2026-02-09 | Codex | Step 79: Auto-indent and smart editing. EditorMode wired for newline indent, tab spacing, and bracket auto-close. 3/3 tests pass. |
| 2026-02-09 | Codex | Step 80: Selection and clipboard operations (Ctrl+C/X/V, shift-extend, double/triple click). 3/3 tests pass. |
| 2026-02-09 | Codex | Step 81: Code folding with tree-sitter fold regions and gutter toggles. 2/2 tests pass. |
| 2026-02-09 | Codex | Step 82: Minimap overview with viewport indicator and click-to-scroll. 1/1 tests pass. |
| 2026-02-09 | Codex | Step 83: Added tree-sitter grammars for JS/TS/Java/Rust/Go and editor modes; syntax highlighting extended to 8 languages. 2/2 tests pass. |
| 2026-02-09 | Codex | Step 84: Native file dialogs via tinyfiledialogs; FileDialog wrapper with injectable provider. 2/2 tests pass. |
| 2026-02-09 | Codex | Step 85: Filesystem tree with .gitignore filtering and Explorer rendering. 1/1 tests pass. |
| 2026-02-09 | Codex | Step 86: Multi-tab editing wired via BufferManager with per-buffer state. 3/3 tests pass. |
| 2026-02-09 | Codex | Step 87: Welcome screen wired with recent files persistence and quick actions. 2/2 tests pass. |
| 2026-02-09 | Codex | Step 88: Drag-and-drop open for files and folders. 2/2 tests pass. |
| 2026-02-09 | Codex | Step 89: File watcher polling with auto-reload for clean buffers. 1/1 tests pass. |
| 2026-02-09 | Codex | Step 90: LSPClient core with JSON-RPC initialize/shutdown and injectable transport. 2/2 tests pass. |
| 2026-02-09 | Codex | Step 91: LSP diagnostics integration: didOpen/didChange/didSave notifications, publishDiagnostics parsing, Problems panel with jump-to-location. 1/1 tests pass. |
