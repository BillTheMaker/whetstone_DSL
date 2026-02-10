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
| Sprint 1 | — | **Complete** | MPS-based prototype (JetBrains MPS language plugin) |
| Sprint 2 | 1–38 | **Complete** | C++ editor stack: AST, serialization, generators, ImGui shell, orchestrator, agents |
| Sprint 3 | 39–75 | **Complete** | Core functionality: C++ generator, tree-sitter, classical editing, optimization |
| Sprint 4 | 76–126 | **Complete** | Professional editor: layout presets, code editor, LSP, annotation UI, terminal, build |
| Sprint 5 | 127–165 | **Complete** | Library-aware coding: package registries, constructive coding, Emacs ecosystem, full language coverage, agents |
| Sprint 6 | 166–201 | **Complete** | UX & editor polish: structural refactor, themes, multi-cursor, onboarding, security, accessibility, performance |
| Sprint 7 | 202–234 | **In Progress** | MCP Bridge & agent tooling: API docs/schemas, MCP server, synthetic traces, eval harness, model-specific tools |

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

## Sprint 4: Professional Editor (Steps 76–126) — COMPLETE

### Phase 4a: Layout & Code Editor Core (Steps 76–83) — COMPLETE
- [x] Step 76: **IMPLEMENTED** — LayoutManager: 3 preset docking layouts (VSCode/Emacs/JetBrains), panel visibility/ratio queries, dirty flag for rebuild, save/load persistence, preset name round-trip (10/10 tests pass)
- [x] Step 77: **IMPLEMENTED** — Custom CodeEditorWidget renderer: per-token coloring, cursor/selection/input handling, monospace grid, blinking cursor, visible whitespace toggle (3/3 tests pass)
- [x] Step 78: **IMPLEMENTED** — Line numbers and gutter: fixed-width gutter with right-aligned line numbers, current line highlight, gutter click selects line (2/2 tests pass)
- [x] Step 79: **IMPLEMENTED** — Auto-indent and smart editing: EditorMode integration, enter auto-indent, tab inserts spaces, auto-close brackets/quotes (3/3 tests pass)
- [x] Step 80: **IMPLEMENTED** — Scroll/selection/clipboard: shift-extend selection, double/triple click selection, Ctrl+C/X/V clipboard, drag select (3/3 tests pass)
- [x] Step 81: **IMPLEMENTED** — Code folding: tree-sitter fold regions, gutter fold toggles, hidden ranges with placeholders (2/2 tests pass)
- [x] Step 82: **IMPLEMENTED** — Minimap: right-side overview with viewport indicator and click-to-scroll (1/1 tests pass)
- [x] Step 83: **IMPLEMENTED** — Additional tree-sitter grammars (JS/TS/Java/Rust/Go), new EditorMode configs, syntax highlighting for 8 languages (2/2 tests pass)

### Phase 4b: File Management (Steps 84–89) — COMPLETE
- [x] Step 84: **IMPLEMENTED** — Native file dialogs via tinyfiledialogs, FileDialog wrapper with test provider injection (2/2 tests pass)
- [x] Step 85: **IMPLEMENTED** — Filesystem tree with .gitignore filtering, recursive Explorer rendering (1/1 tests pass)
- [x] Step 86: **IMPLEMENTED** — Multi-tab editing with BufferManager and per-buffer editor state (3/3 tests pass)
- [x] Step 87: **IMPLEMENTED** — Welcome screen wired with recent files persistence and quick actions (2/2 tests pass)
- [x] Step 88: **IMPLEMENTED** — Drag-and-drop file/folder open via SDL_DROPFILE (2/2 tests pass)
- [x] Step 89: **IMPLEMENTED** — File watcher polling with auto-reload for clean buffers (1/1 tests pass)

### Phase 4c: LSP Client & Diagnostics (Steps 90–96) — COMPLETE

### Phase 4d: Annotation UI (Steps 97–102) — COMPLETE

### Phase 4e: Cross-Language & Optimization UI (Steps 103–107c) — COMPLETE

### Phase 4f: Navigation & Search (Steps 108–114) — COMPLETE

### Phase 4g: Project & Session Management (Steps 115–119) — COMPLETE

### Phase 4h: Integration & Build (Steps 120–126) — COMPLETE
- [x] Step 90: **IMPLEMENTED** — LSPClient core with JSON-RPC initialize/shutdown, injectable transport (2/2 tests pass)
- [x] Step 91: **IMPLEMENTED** — LSP diagnostics: didOpen/didChange/didSave notifications, publishDiagnostics parsing/storage, Problems panel UI (1/1 tests pass)
- [x] Step 92: **IMPLEMENTED** — LSP completion requests, response parsing, and completion popup with filtering/acceptance (1/1 tests pass)
- [x] Step 93: **IMPLEMENTED** — LSP hover and signature help requests, response parsing, and inline tooltip/popup (2/2 tests pass)
- [x] Step 94: **IMPLEMENTED** — Whetstone diagnostics aggregation via Pipeline, merged Problems panel, and gutter markers (1/1 tests pass)
- [x] Step 94a: **IMPLEMENTED** — AST source span tracking via tree-sitter, serialized in JSON (2/2 tests pass)
- [x] Step 95: **IMPLEMENTED** — Diagnostic gutter markers with tooltips and inline squiggles (1/1 tests pass)
- [x] Step 96: **IMPLEMENTED** — LSP server settings UI with defaults, auto-detect, and schema validation hook (2/2 tests pass)
- [x] Step 97: **IMPLEMENTED** — Annotation gutter markers with hover details and click-to-open placeholder editor (1/1 tests pass)
- [x] Step 98: **IMPLEMENTED** — Inline annotation tags with layout-aware placement and view toggle (1/1 tests pass)
- [x] Step 99: **IMPLEMENTED** — Annotation context menu for add/edit/remove via ASTMutationAPI (2/2 tests pass)
- [x] Step 100: **IMPLEMENTED** — Memory strategy suggestions with lightbulb gutter icons and apply popup (1/1 tests pass)
- [x] Step 101: **IMPLEMENTED** — Annotation conflict highlighting with gutter linkage and quick-fix actions (1/1 tests pass)
- [x] Step 102: **IMPLEMENTED** — Memory strategy dashboard panel with counts and JSON export (1/1 tests pass)
- [x] Step 103: **IMPLEMENTED** — Side-by-side generated code view with scroll lock, target language selection, and line highlighting (1/1 tests pass)
- [x] Step 104: **IMPLEMENTED** — Cross-language projection button with read-only projected tabs and annotation summary (2/2 tests pass)
- [x] Step 105: **IMPLEMENTED** — Optimization controls panel with constraint-aware buttons and summaries (2/2 tests pass)
- [x] Step 106: **IMPLEMENTED** — Transform history panel with undo controls and provenance coloring (1/1 tests pass)
- [x] Step 107: **IMPLEMENTED** — Before/after diff view with preview and accept/reject (1/1 tests pass)
- [x] Step 107a: **IMPLEMENTED** — Text-Editor Mode toggle and per-buffer mode tracking (2/2 tests pass)
- [x] Step 107b: **IMPLEMENTED** — Mode-specific UI behavior and feature gating (2/2 tests pass)
- [x] Step 107c: **IMPLEMENTED** — Per-buffer mode persistence in recent files (2/2 tests pass)
- [x] Step 108: **IMPLEMENTED** — Batch mutation UI with refactor actions and diff preview (3/3 tests pass)
- [x] Step 109: **IMPLEMENTED** — Command palette with fuzzy search and MRU ranking (2/2 tests pass)
- [x] Step 110: **IMPLEMENTED** — Go-to-definition (LSP + Whetstone) with hover preview (2/2 tests pass)
- [x] Step 111: **IMPLEMENTED** — Symbol outline panel with LSP + AST fallback (2/2 tests pass)
- [x] Step 112: **IMPLEMENTED** — Breadcrumb navigation with scope roles (1/1 tests pass)
- [x] Step 113: **IMPLEMENTED** — Project-wide search panel with regex and glob filters (3/3 tests pass)
- [x] Step 114: **IMPLEMENTED** — Go-to-line popup with :line:col parsing (4/4 tests pass)
- [x] Step 115: **IMPLEMENTED** — Orchestrator wired for structured mutations; Emacs config path setting (3/3 tests pass)
- [x] Step 116: **IMPLEMENTED** — Project save/load (.whetstone) with AST serialization (4/4 tests pass)
- [x] Step 117: **IMPLEMENTED** — Session persistence (layout, buffers, cursors, folds) (5/5 tests pass)
- [x] Step 118: **IMPLEMENTED** — Settings panel + persistence (11/11 tests pass)
- [x] Step 119: **IMPLEMENTED** — Zoom controls and status bar indicator (5/5 tests pass)

---

## Sprint 5: Library-Aware Constructive Coding (Steps 127–165) — COMPLETE

### Phase 5a: Library & Dependency Management (Steps 127–133) — COMPLETE
- [x] Step 127: Package registry abstraction stubs (PyPI, npm, crates.io, Maven, Go, vcpkg). 4/4 tests pass.
- [x] Step 128: Dependency file parsing with Import population. 6/6 tests pass.
- [x] Step 129: Dependency management UI panel with add/remove/update and writeback. 8/8 tests pass.
- [x] Step 130: LSP workspace symbol + completion indexing for dependencies. 7/7 tests pass.
- [x] Step 131: Stub-based library indexing (pyi/d.ts/headers/lib.rs). 8/8 tests pass.
- [x] Step 132: Library symbol browser panel with filtering and insert templates. 5/5 tests pass.
- [x] Step 133: Import statement generation + unused import warnings across 6 languages. 7/7 tests pass.

### Phase 5b: Constructive Coding Flow (Steps 134–140) — COMPLETE
- [x] Step 134: PrimitivesRegistry aggregating builtins, imports, and in-scope symbols. 6/6 tests pass.
- [x] Step 135: Library-aware completion ordering with auto-import hints. 5/5 tests pass.
- [x] Step 136: Agent preferImports/strictMode policy checks on mutations. 4/4 tests pass.
- [x] Step 137: Composition builder panel with pipeline generation and code insertion. 4/4 tests pass.
- [x] Step 138: Type-aware C++ generation via library call mappings. 2/2 tests pass.
- [x] Step 139: Library compatibility matrix with default mappings. 3/3 tests pass.
- [x] Step 140: Constructive coding integration tests. 7/7 tests pass.

### Phase 5c: Emacs Ecosystem Integration (Steps 141–146) — COMPLETE
- [x] Step 141: Emacs daemon startup with user config path and init logging. 4/4 tests pass.
- [x] Step 142: Emacs package browser panel with load actions and queries. 7/7 tests pass.
- [x] Step 143: Elisp function discovery via apropos/describe-function. 7/7 tests pass.
- [x] Step 144: Emacs keybinding integration (prefix handling, M-x, mode line). 11/11 tests pass.
- [x] Step 145: Org-mode rendering with source blocks, inline results, tree-sitter-org. 11/11 tests pass.
- [x] Step 146: Emacs-Whetstone bridge with buffer pull/push and sync commands. 5/5 tests pass.

### Phase 5d: Full Language Coverage (Steps 147–153) — COMPLETE
- [x] Step 147: JavaScript/TypeScript CST-to-AST parsing. 4/4 tests pass.
- [x] Step 148: JavaScript/TypeScript generator with imports, classes, WeakRef mapping. 6/6 tests pass.
- [x] Step 149: Java CST-to-AST + generator with class/method/constructor handling. 8/8 tests pass.
- [x] Step 150: Rust CST-to-AST + generator with impl methods, RAII annotations. 7/7 tests pass.
- [x] Step 151: Go CST-to-AST + generator with receivers, Go-style control flow. 8/8 tests pass.
- [x] Step 152: Cross-language projection extended for all new languages. 6/6 tests pass.
- [x] Step 153: Language coverage integration tests (full projection matrix). 208/208 tests pass.

### Phase 5e: Advanced Agent Capabilities (Steps 154–159) — COMPLETE
- [x] Step 154: Agent library context payload on connect with primitives snapshot. 4/4 tests pass.
- [x] Step 155: Agent library-aware code generation RPC + generator helper. 3/3 tests pass.
- [x] Step 156: Agent annotation assistant RPC with suggestions and feedback learning. 5/5 tests pass.
- [x] Step 157: Multi-agent roles with permission policy and provenance tracking. 8/8 tests pass.
- [x] Step 158: Agent workflow recorder with JSON export and replay. 5/5 tests pass.
- [x] Step 159: Agent marketplace registry with UI panel and install toggles. 6/6 tests pass.

### Phase 5f: Polish & Ecosystem (Steps 160–165) — COMPLETE
- [x] Step 160: Theme engine with JSON themes, ImGui styling, syntax color mapping. 4/4 tests pass.
- [x] Step 161: Plugin API + loader with registry hooks for concepts/generators/panels/commands. 10/10 tests pass.
- [x] Step 162: Help panel with Markdown sections + docs/help.md. 4/4 tests pass.
- [x] Step 163: Telemetry + crash logging (opt-in) with settings toggle. 3/3 tests pass.
- [x] Step 164: Update checker stub + installer manifests/config. 3/3 tests pass.
- [x] Step 165: Sprint 5 integration tests across primitives, projection, pipeline, composition. 8/8 tests pass.

---

## Sprint 6: UX & Editor Polish (Steps 166–201) — COMPLETE

### Phase 6a: Structural Refactor (Steps 166–170) — COMPLETE
- [x] Step 166: Panel extraction to `panels/` directory (main.cpp 4,102 → 444 lines). Tests pass.
- [x] Step 167: EditorState split into focused sub-states (Search/Agent/Build/Library/Emacs/UIFlags). 1/1 tests pass.
- [x] Step 168: Split oversized component headers (CodeEditorWidget, Parser, SyntaxHighlighter, CppGenerator). 79/79 tests pass.
- [x] Step 169: Notification/toast system with status bar history and output log rewire. 2/2 tests pass.
- [x] Step 170: UI event bus with debounced dispatch, settings/theme events. 2/2 tests pass.

### Phase 6b: Theme Engine & Visual Design (Steps 171–176) — COMPLETE
- [x] Step 171: ThemeEngine core enhancements (ThemeColor API, user theme dir, hot reload). 2/2 tests pass.
- [x] Step 172: Bundled theme pack (Whetstone Dark/Light, Monokai Pro). 2/2 tests pass.
- [x] Step 173: Theme gallery UI with hover preview, apply/reset, swatches. 2/2 tests pass.
- [x] Step 174: IconSet system with theme-aware, zoom-scaled icons. 2/2 tests pass.
- [x] Step 175: Typography controls (fonts, line height, letter spacing). 2/2 tests pass.
- [x] Step 176: Smooth UI transitions (panel slides, tab fade, toast animations, reduce-motion). 2/2 tests pass.

### Phase 6c: Editor UX Enhancements (Steps 177–183) — COMPLETE
- [x] Step 177: Enhanced find/replace (match counts, regex preview, selection scope, history). 2/2 tests pass.
- [x] Step 178: Multi-cursor editing (Alt+Click, Ctrl+D, Ctrl+Alt+Up/Down, column selection). 2/2 tests pass.
- [x] Step 179: Rainbow brackets, bracket-pair highlight, scope tint, auto-surround. 2/2 tests pass.
- [x] Step 180: Rich tooltip system with markdown rendering, pinning, max size/scrolling. 2/2 tests pass.
- [x] Step 181: Enhanced status bar (segments, notifications, selection counts, git branch). 2/2 tests pass.
- [x] Step 182: Problems panel overhaul with grouping, sortable table, diagnostic badges. 2/2 tests pass.
- [x] Step 183: Tab reordering and untitled rename flow. 2/2 tests pass.

### Phase 6d: Onboarding & Discoverability (Steps 184–189) — COMPLETE
- [x] Step 184: First-run wizard (layout/theme/keybindings + get-started actions). 2/2 tests pass.
- [x] Step 185: Contextual feature hints with persistence + dismiss/disable. 2/2 tests pass.
- [x] Step 186: Keyboard shortcut reference panel with filter and live bindings. 2/2 tests pass.
- [x] Step 187: In-editor help panel with Markdown renderer reuse. 2/2 tests pass.
- [x] Step 188: Command palette upgrade (categories, recent section, fuzzy highlights). 1/1 tests pass.
- [x] Step 189: Guided workflow wizards (annotate file, cross-language, connect agent). 1/1 tests pass.

### Phase 6e: Security & Library UX (Steps 190–195) — COMPLETE
- [x] Step 190: Vulnerability database (OSV parsing, cache+TTL, offline mode). 1/1 tests pass.
- [x] Step 191: Dependency security badges with severity colors and safe upgrade path. 1/1 tests pass.
- [x] Step 192: Security diagnostics integration (gutter shields, [Security] problems). 1/1 tests pass.
- [x] Step 193: Semantic library tags (auto-tagging heuristics, 10+ predefined tags). 1/1 tests pass.
- [x] Step 194: Semantic-filtered library browser (tag filter bar, context-aware boosting). 1/1 tests pass.
- [x] Step 195: Security & semantic UX integration tests. 1/1 tests pass.

### Phase 6f: Accessibility & Performance (Steps 196–201) — COMPLETE
- [x] Step 196: High contrast + colorblind modes (annotation shapes, diagnostic patterns). 1/1 tests pass.
- [x] Step 197: Keyboard navigation audit (F6 panel cycle, Esc focus, focus ring). 1/1 tests pass.
- [x] Step 198: Virtual scrolling for large files (viewport + buffered rendering). 1/1 tests pass.
- [x] Step 199: Large file handling (size thresholds, text-mode fallback, memory indicator). 1/1 tests pass.
- [x] Step 200: Startup/perf (LSP/highlight debounce, deferred AST sync, frame budget warnings). 1/1 tests pass.
- [x] Step 201: Sprint 6 integration tests (panel wiring, themes, multicursor, wizard, security, keyboard nav). 1/1 tests pass.

---

## Sprint 7: MCP Bridge & Agent Tooling (Steps 202–234) — IN PROGRESS

### Phase 7a: API Documentation & Schemas (Steps 202–206) — COMPLETE
- [x] Step 202: JSON-RPC API reference document (`docs/AGENT_API.md`): 23 methods documented with schemas, examples, error codes.
- [x] Step 203: JSON Schema definitions (`schemas/`): 8 type schemas + 12 method schemas for request/response validation.
- [x] Step 204: Exposed ContextAPI and BatchMutationAPI via RPC (getInScopeSymbols, getCallHierarchy, getDependencyGraph, applyBatch).
- [x] Step 205: Exposed Pipeline operations via RPC (runPipeline, parseSource, generateFromAST, projectLanguage).
- [x] Step 206: API schema validation tests. 51/51 tests pass.

### Phase 7b: MCP Server (Steps 207–213) — COMPLETE
- [x] Step 207: MCP server core (`MCPServer.h`): JSON-RPC 2.0 with initialize handshake, protocol version "2024-11-05".
- [x] Step 208: MCP tools — AST query and mutation (5 tools: get_ast, mutate, batch_mutate, get_scope, get_call_hierarchy).
- [x] Step 209: MCP tools — annotation and generation (5 tools: suggest_annotations, apply_annotation, generate_code, run_pipeline, project_language).
- [x] Step 210: MCP resources (5 resources: ast, diagnostics, libraries, annotations, settings).
- [x] Step 211: MCP prompts (4 prompts: annotate_module, cross_language_projection, security_audit, refactor_memory).
- [x] Step 212: MCP bridge (`MCPBridge.h`): stdio transport with Content-Length framing, embedded mode support.
- [x] Step 213: MCP server tests. 90/90 tests pass.

### Phase 7c: Synthetic Trace Generation (Steps 214–219) — COMPLETE
- [x] Step 214: Trace data model (`TraceGenerator.h`): TraceStep (user/assistant/tool_call/tool_result), Trace with metadata.
- [x] Step 215: 6 scenario templates: ReadAndUnderstand, AddAnnotations, CrossLanguage, Refactor, SecurityAudit, MultiStepDebug.
- [x] Step 216: Built-in code corpus: 9 samples across 4 languages (Python, C++, JavaScript, Rust).
- [x] Step 217: Generator engine with deterministic seeding, batch generation, and real Pipeline.parse() integration.
- [x] Step 218: Trace export pipeline (`TraceExporter.h`): Anthropic Messages, OpenAI Chat, JSONL, Markdown formats. Filtering and statistics.
- [x] Step 219: Trace generation tests. 294/294 tests pass.

### Phase 7d: Evaluation Harness (Steps 220–224) — PENDING
### Phase 7e: Model-Specific Tool Definitions (Steps 225–229) — PENDING
### Phase 7f: Session Recording Pipeline (Steps 230–234) — PENDING

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

**All 219 steps compile and pass.** 350+ test executables in `editor/build/Release/`.

**Sprint 2 (Steps 1–38):** All pass.
**Sprint 3 (Steps 39–75):** All pass. Highlights: step53 6/6, step54 10/10, step72 6/6, step74 6/6.
**Sprint 4 (Steps 76–126):** All pass. Highlights: step76 10/10, step118 11/11, step125 3/3 integration.
**Sprint 5 (Steps 127–165):** All pass. Highlights: step144 11/11, step145 11/11, step153 208/208 (cross-language matrix), step161 10/10.
**Sprint 6 (Steps 166–201):** All pass. Highlights: step168 79/79 (split + integration), step201 integration tests.
**Sprint 7 (Steps 202–219):** All pass. Highlights: step206 51/51, step213 90/90, step219 294/294.
**Architecture test:** `file_limits_test` 4/4 passes (enforces header size limits).

---

## Key Source Files

### Core AST & Generation
| File | Contents |
|------|----------|
| `editor/src/ast/ASTNode.h` | All 33+ AST node classes, Import, ExternalModule, TypeSignature, JSON serialization |
| `editor/src/ast/ProjectionGenerator.h` | Base generator class + shared dispatch helper |
| `editor/src/ast/PythonGenerator.h` | Python code generator |
| `editor/src/ast/ElispGenerator.h` | Elisp code generator |
| `editor/src/ast/CppGenerator.h` | C++ code generator with memory annotations |
| `editor/src/ast/JavaScriptGenerator.h` | JS/TS generator with WeakRef/FinalizationRegistry mapping |
| `editor/src/ast/JavaGenerator.h` | Java generator with class wrapping and GC annotations |
| `editor/src/ast/RustGenerator.h` | Rust generator with ownership semantics |
| `editor/src/ast/GoGenerator.h` | Go generator with escape analysis annotations |
| `editor/src/ast/Parser.h` | TreeSitterParser dispatcher for all languages |
| `editor/src/ast/PythonParser.h` | Python CST-to-AST conversion |
| `editor/src/ast/CppParser.h` | C++ CST-to-AST with memory pattern detection |
| `editor/src/ast/ElispParser.h` | Elisp CST-to-AST conversion |
| `editor/src/ast/Schema.h` | AST schema validation |

### Editor Infrastructure
| File | Contents |
|------|----------|
| `editor/src/main.cpp` | ImGui shell: init, event loop, docking, panel dispatch (444 lines) |
| `editor/src/EditorState.h` | Top-level state composing sub-states |
| `editor/src/state/SearchState.h` | Find/replace, project search state |
| `editor/src/state/AgentState.h` | WebSocket server, agent log, permissions |
| `editor/src/state/BuildState.h` | Build system, errors, run state |
| `editor/src/state/LibraryState.h` | Dependency panel, library browser, composition |
| `editor/src/state/EmacsState.h` | Packages, function index, keybindings, org doc |
| `editor/src/state/UIFlags.h` | Panel visibility toggles, bottom tab selection |
| `editor/src/CodeEditorWidget.h` | Custom ImGui code editor (core) |
| `editor/src/CodeEditorRendering.h` | Editor rendering helpers |
| `editor/src/TextEditor.h` | Edit ops, undo/redo, find/replace, selection |
| `editor/src/TextASTSync.h` | Bidirectional text↔AST synchronization with debounce |
| `editor/src/SyntaxHighlighter.h` | Tree-sitter CST walk → colored token spans (8 languages) |
| `editor/src/KeybindingManager.h` | Configurable keybinding profiles (VSCode/JetBrains/Emacs) |
| `editor/src/BufferManager.h` | Multi-buffer management (open/close/switch/track) |
| `editor/src/LayoutManager.h` | Docking layout presets, panel queries, persistence |
| `editor/src/LSPClient.h` | Language Server Protocol client |

### Panels (extracted Sprint 6)
| File | Contents |
|------|----------|
| `editor/src/panels/MenuBarPanel.h` | Menu bar and toolbar |
| `editor/src/panels/ExplorerPanel.h` | File tree and workspace browser |
| `editor/src/panels/EditorPanel.h` | Code editor area, tabs, split view |
| `editor/src/panels/BottomPanel.h` | Output, AST, terminal, problems, agents tabs |
| `editor/src/panels/StatusBarPanel.h` | Status bar with mode/language/position |
| `editor/src/panels/SidePanels.h` | Outline, dependencies, library browser, etc. |
| `editor/src/panels/SearchPanels.h` | Find/replace and project search |
| `editor/src/panels/SettingsPanel.h` | Settings UI |
| `editor/src/panels/WizardPanels.h` | Guided workflow wizards |
| `editor/src/panels/DialogPanels.h` | Dialog panels |

### Intelligence & Analysis
| File | Contents |
|------|----------|
| `editor/src/Pipeline.h` | End-to-end: parse → infer → validate → optimize → generate |
| `editor/src/CrossLanguageProjector.h` | AST deep-copy with language change and annotation adaptation |
| `editor/src/MemoryStrategyInference.h` | Language defaults, pattern analysis, confidence scoring |
| `editor/src/AnnotationValidator.h` | Missing intent, alias detection, conflict checking |
| `editor/src/TransformEngine.h` | Constant folding, dead code elimination, OptimizationLock |
| `editor/src/StrategyAwareOptimizer.h` | Annotation-constrained optimization |
| `editor/src/StrategyValidator.h` | Post-optimization invariant validation |
| `editor/src/IncrementalOptimizer.h` | Incremental transforms with journal and provenance |

### Library & Security
| File | Contents |
|------|----------|
| `editor/src/PrimitivesRegistry.h` | Available symbols aggregation (builtins + imports + scope) |
| `editor/src/PackageRegistry.h` | Multi-ecosystem package queries |
| `editor/src/DependencyParser.h` | Parse requirements/package.json/Cargo.toml/go.mod |
| `editor/src/LibraryIndexer.h` | API surface indexing via stubs and LSP |
| `editor/src/SemanticTags.h` | Semantic library annotations (@serialize, @crypto, @io, etc.) |
| `editor/src/VulnerabilityDatabase.h` | OSV-based vulnerability tracking with cache |
| `editor/src/LibraryCompatibility.h` | Cross-language library equivalents |

### Agent System
| File | Contents |
|------|----------|
| `editor/src/WebSocketServer.h` | Agent WebSocket endpoint with JSON-RPC routing |
| `editor/src/ASTMutationAPI.h` | AST mutations with lock checking and journal |
| `editor/src/ASTQueryAPI.h` | AST queries (findByType, findByAnnotation, subtree) |
| `editor/src/ContextAPI.h` | Scope analysis, call hierarchy, dependency graph |
| `editor/src/BatchMutationAPI.h` | Atomic batch mutations with rollback |
| `editor/src/AgentPermissionPolicy.h` | Role-based agent permissions |
| `editor/src/AgentAnnotationAssistant.h` | Annotation suggestion system with feedback |
| `editor/src/WorkflowRecorder.h` | Workflow recording and replay |

### Sprint 6 UX Components
| File | Contents |
|------|----------|
| `editor/src/ThemeEngine.h` | Theme loading, color lookup, hot-reload, bundled themes |
| `editor/src/IconSet.h` | Theme-aware, zoom-scaled file/panel/diagnostic icons |
| `editor/src/NotificationSystem.h` | Toast notifications with history |
| `editor/src/UIEventBus.h` | Pub/sub for decoupled panel updates |
| `editor/src/RichTooltip.h` | Markdown-capable hover tooltips with pinning |
| `editor/src/SearchUtils.h` | Advanced search (regex, match counts, history) |
| `editor/src/AnimationUtils.h` | Smooth transitions and easing functions |
| `editor/src/FirstRunWizard.h` | First-launch setup experience |
| `editor/src/WizardFramework.h` | Multi-step wizard UI component |
| `editor/src/FeatureHints.h` | Contextual tip system |
| `editor/src/ShortcutReference.h` | Keyboard shortcut browser panel |

### MCP & Training Data (Sprint 7)
| File | Contents |
|------|----------|
| `editor/src/MCPServer.h` | MCP protocol server: 10 tools, 5 resources, 4 prompts, JSON-RPC 2.0 |
| `editor/src/MCPBridge.h` | MCP stdio transport bridge to Whetstone internal RPC |
| `editor/src/TraceGenerator.h` | Synthetic trace generation: 6 scenarios, code corpus, batch engine |
| `editor/src/TraceExporter.h` | Multi-format trace export (Anthropic/OpenAI/JSONL/Markdown), filtering, stats |
| `docs/AGENT_API.md` | Complete JSON-RPC API reference (23 methods) |
| `schemas/` | 20 JSON Schema files (8 types + 12 methods) |

### Build & Infrastructure
| File | Contents |
|------|----------|
| `editor/CMakeLists.txt` | CMake build config (vcpkg-based) |
| `editor/src/orchestrator_main.cpp` | Orchestrator standalone process (JSON-RPC server) |
| `editor/src/Orchestrator.h` | Central state manager, undo journal, Emacs integration |
| `editor/src/EmacsIntegration.h` | ElispCommandBuilder + EmacsConnection lifecycle |

---

## Architecture Notes

- **Editor** (`whetstone_editor.exe`): ImGui-based GUI (SDL2 + OpenGL3). Modular panel architecture (Sprint 6 extraction: main.cpp = 444 lines). Docking layout with 3 presets (VSCode/Emacs/JetBrains). Custom CodeEditorWidget with multi-cursor, rainbow brackets, virtual scrolling. Theme engine with hot-reloadable JSON themes. 8 language parsers and generators. LSP client integration. Notification/toast system. Event-driven updates via UIEventBus.
- **Orchestrator** (`orchestrator.exe`): Standalone JSON-RPC server. Manages AST state, undo/redo journal, file I/O, Emacs daemon integration, agent API.
- **Communication**: Editor ↔ Orchestrator via JSON-RPC over stdin/stdout pipes. Editor runs in disconnected mode when no pipe detected. Agent access via WebSocket JSON-RPC.
- **Generators**: AST → Python, C++, Elisp, JavaScript/TypeScript, Java, Rust, Go. All generators handle canonical memory annotations with language-appropriate mappings.
- **Parsers**: Text → AST via tree-sitter for all 8 supported languages. Full CST-to-AST conversion with memory pattern detection and auto-annotation.
- **Library System**: Package registry abstraction (PyPI, npm, crates.io, Maven, Go, vcpkg). PrimitivesRegistry for constructive coding. Vulnerability database (OSV). Semantic library tags.
- **Agent System**: WebSocket server with role-based permissions, library context, annotation assistant, workflow recording, agent marketplace. MCP server (10 tools, 5 resources, 4 prompts) for LLM integration via stdio transport.
- **Training Data**: Synthetic trace generator (6 scenario types, 4-language code corpus) with multi-format export (Anthropic Messages, OpenAI Chat, JSONL, Markdown).

---

## What's Next

**Sprint 7 Phase 7d–7f remaining.** Phases 7a–7c complete (Steps 202–219, 435 combined assertions passing). Next: Phase 7d Evaluation Harness (Steps 220–224), Phase 7e Model-Specific Tool Definitions (Steps 225–229), Phase 7f Session Recording Pipeline (Steps 230–234). Full plan: `sprint7_plan.md`.

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
| 2026-02-09 | Codex | Step 92: LSP completion requests/response parsing; completion popup with filtering and acceptance. 1/1 tests pass. |
| 2026-02-09 | Codex | Step 93: LSP hover and signature help requests/response parsing; tooltip and signature popup. 2/2 tests pass. |
| 2026-02-09 | Codex | Step 94: Whetstone diagnostics aggregation via Pipeline; merged Problems panel and gutter markers. 1/1 tests pass. |
| 2026-02-09 | Codex | Step 94a: AST source span tracking (tree-sitter) with JSON serialization. 2/2 tests pass. |
| 2026-02-09 | Codex | Step 95: Diagnostic gutter markers with tooltips and inline squiggles. 1/1 tests pass. |
| 2026-02-09 | Codex | Step 96: LSP server settings UI with defaults, auto-detect, and schema validation hook. 2/2 tests pass. |
| 2026-02-09 | Codex | Step 97: Annotation gutter markers with hover details and click-to-open placeholder editor. 1/1 tests pass. |
| 2026-02-09 | Codex | Step 98: Inline annotation tags with layout-aware placement and view toggle. 1/1 tests pass. |
| 2026-02-09 | Codex | Step 99: Annotation context menu (add/edit/remove) via ASTMutationAPI. 2/2 tests pass. |
| 2026-02-09 | Codex | Step 100: Memory strategy suggestions with lightbulb icons and apply popup. 1/1 tests pass. |
| 2026-02-09 | Codex | Step 101: Annotation conflict highlighting with quick-fix actions. 1/1 tests pass. |
| 2026-02-09 | Codex | Step 102: Memory strategy dashboard panel with counts and JSON export. 1/1 tests pass. |
| 2026-02-09 | Codex | Step 103: Side-by-side generated code view with scroll lock, target language selector, and line highlighting. 1/1 tests pass. |
| 2026-02-09 | Codex | Step 104: Cross-language projection button with read-only projected tabs and annotation summary. 2/2 tests pass. |
| 2026-02-09 | Codex | Step 105: Optimization controls panel with constraint-aware buttons and summaries. 2/2 tests pass. |
| 2026-02-09 | Codex | Step 106: Transform history panel with undo controls and provenance coloring. 1/1 tests pass. |
| 2026-02-09 | Codex | Planned Step 107a–107c: add Text-Editor Mode toggle, mode-specific UI behavior, and per-buffer persistence. |
| 2026-02-09 | Codex | Step 107: Before/after diff view with preview and accept/reject. 1/1 tests pass. |
| 2026-02-09 | Codex | Step 107a: Text-Editor Mode toggle and per-buffer mode tracking. 2/2 tests pass. |
| 2026-02-09 | Codex | Step 107b: Mode-specific UI behavior and feature gating. 2/2 tests pass. |
| 2026-02-09 | Codex | Step 107c: Per-buffer mode persistence in recent files. 2/2 tests pass. |
| 2026-02-09 | Codex | Step 108: Batch mutation UI with refactor actions and diff preview. 3/3 tests pass. |
| 2026-02-09 | Codex | Step 109: Command palette with fuzzy search and MRU ranking. 2/2 tests pass. |
| 2026-02-09 | Codex | Step 110: Go-to-definition (LSP + Whetstone) with hover preview. 2/2 tests pass. |
| 2026-02-09 | Codex | Step 111: Symbol outline panel with LSP + AST fallback. 2/2 tests pass. |
| 2026-02-09 | Codex | Step 112: Breadcrumb navigation with scope roles. 1/1 tests pass. |
| 2026-02-09 | Codex | Step 113: Project-wide search panel with regex and glob filters. 3/3 tests pass. |
| 2026-02-09 | Codex | Step 114: Go-to-line popup with :line:col parsing. 4/4 tests pass. |
| 2026-02-09 | Codex | Step 115: Orchestrator wired for structured mutations; Emacs config path setting. 3/3 tests pass. |
| 2026-02-09 | Codex | Step 116: Project save/load (.whetstone) with AST serialization. 4/4 tests pass. |
| 2026-02-09 | Codex | Step 117: Session persistence (layout, buffers, cursors, folds). 5/5 tests pass. |
| 2026-02-09 | Codex | Step 118: Settings panel + persistence. 11/11 tests pass. |
| 2026-02-09 | Codex | Step 119: Zoom controls and status bar indicator. 5/5 tests pass. |
| 2026-02-09 | Codex | Step 120: Unified undo/redo via orchestrator snapshots (per-buffer), status bar undo depth, snapshot-based undo/redo wiring. 4/4 tests pass. |
| 2026-02-09 | Claude Opus 4.6 | Pre-Sprint 5 refactoring: Created ARCHITECTURE.md (coding standards). Split Generator.h (2150→4 files, shared dispatch eliminates 279 lines duplication). Extracted EditorState.h and EditorUtils.h from main.cpp (4102→~2235 lines). Pinned 6 FetchContent deps to release tags. Orchestrator.h cleanup: wired CppGenerator, added platform guards, fixed shell injection, replaced stale TODOs with STUB markers. Sprint 4 marked complete (Steps 121–126 deferred to Sprint 5). |
| 2026-02-09 | Codex | Step 121: Added Import, ExternalModule, TypeSignature AST concepts with serialization + schema wiring. 10/10 tests pass. |
| 2026-02-09 | Codex | Step 122: Integrated terminal panel with command runner, ANSI color rendering, and Ctrl+` toggle. 3/3 tests pass. |
| 2026-02-09 | Codex | Step 123: Run/build command support with toolbar buttons, build menu, and status bar run state. 7/7 tests pass. |
| 2026-02-09 | Codex | Step 124: Agent server wiring (Mock transport), RPC handler, and Agents tab/log. 5/5 tests pass. |
| 2026-02-09 | Codex | Step 125: Agents panel enhancements (permissions, disconnect, request logging). 2/2 tests pass. |
| 2026-02-09 | Codex | Added integration test for Step 125 agent mutation flow; updated testing policy in ARCHITECTURE.md. 3/3 integration tests pass. |
| 2026-02-09 | Codex | Step 126: Build system detection, build commands, and error parsing/jump. 7/7 tests pass. |
| 2026-02-09 | Codex | Step 127: Package registry abstraction stubs for major ecosystems. 4/4 tests pass. |
| 2026-02-09 | Codex | Step 128: Dependency file parsing with Import population. 6/6 tests pass. |
| 2026-02-09 | Codex | Step 129: Dependency management UI panel with add/remove/update, source targeting, and writeback support for requirements.txt/package.json/Cargo.toml/go.mod/vcpkg.json. 8/8 tests pass. |
| 2026-02-09 | Codex | Step 130: LSP workspace symbol + completion indexing for dependencies with ExternalModule population and library index polling. 7/7 tests pass. |
| 2026-02-09 | Codex | Step 131: Stub-based library indexing (pyi/d.ts/headers/lib.rs) with workspace scan fallback. 8/8 tests pass. |
| 2026-02-09 | Codex | Step 132: Library symbol browser panel with filtering, doc detail display, and insert template helpers. 5/5 tests pass. |
| 2026-02-09 | Codex | Step 133: Import statement generation + unused import warnings across Python/JS/Rust/Go/Elisp with auto-insert on library symbol use. 7/7 tests pass. |
| 2026-02-09 | Codex | Step 134: PrimitivesRegistry aggregating builtins, imports, and in-scope symbols. 6/6 tests pass. |
| 2026-02-09 | Codex | Step 135: Library-aware completion ordering with auto-import hints for non-primitive symbols. 5/5 tests pass. |
| 2026-02-09 | Codex | Step 136: Agent preferImports/strictMode policy checks on mutations with warnings or blocking. 4/4 tests pass. |
| 2026-02-09 | Codex | Step 137: Composition builder panel with pipeline generation and code insertion. 4/4 tests pass. |
| 2026-02-09 | Codex | Step 138: Type-aware C++ generation via library call mappings. 2/2 tests pass. |
| 2026-02-09 | Codex | Step 139: Library compatibility matrix with default mappings. 3/3 tests pass. |
| 2026-02-09 | Codex | Step 140: Constructive coding integration tests covering imports, policy modes, composition, and type-aware generation. 7/7 tests pass. |
| 2026-02-09 | Codex | Step 141: Emacs daemon startup uses user config path with init logging and diagnostics. 4/4 tests pass. |
| 2026-02-09 | Codex | Added FEATURE_REQUESTS.md to track security vulnerability awareness and semantic library annotations for future sprints. |
| 2026-02-09 | Codex | Added LLM tooling + MCP bridge feature request to FEATURE_REQUESTS.md. |
| 2026-02-10 | Codex | Step 142: Emacs package browser panel with loaded/available status, load actions, and package queries via emacsclient. 7/7 tests pass. |
| 2026-02-10 | Codex | Step 143: Elisp function discovery via apropos/describe-function, Emacs function indexing into ExternalModule + LibraryIndex, and elisp primitive updates. 7/7 tests pass. |
| 2026-02-10 | Codex | Step 144: Emacs keybinding integration (prefix handling, M-x minibuffer, mode line display, key-binding lookup via daemon). 11/11 tests pass. |
| 2026-02-10 | Codex | Step 145: Org-mode rendering with headings/blocks, editable source blocks, inline results, org temp runner, and tree-sitter-org integration. 11/11 tests pass. |
| 2026-02-10 | Codex | Step 146: Emacs-Whetstone bridge with buffer pull/push, Emacs frame opening, and sync commands. 5/5 tests pass. |
| 2026-02-10 | Codex | Step 147: JavaScript/TypeScript CST-to-AST parsing with functions, params, classes, and arrow-function discovery. 4/4 tests pass. |
| 2026-02-10 | Codex | Step 148: JavaScript/TypeScript generator with imports, class method grouping, type annotations, and WeakRef/FinalizationRegistry mapping. 6/6 tests pass. |
| 2026-02-10 | Codex | Step 149: Java CST-to-AST parsing + generator with class/method/constructor handling, import support, and GC-aware annotations. 8/8 tests pass. |
| 2026-02-10 | Codex | Step 150: Rust CST-to-AST parsing + generator with impl methods, imports, and RAII annotations. 7/7 tests pass. |
| 2026-02-10 | Codex | Step 151: Go CST-to-AST parsing + generator with receivers, imports, and Go-style control flow. 8/8 tests pass. |
| 2026-02-10 | Codex | Step 152: Cross-language projection extended for new languages with annotation adaptation and richer node cloning. 6/6 tests pass. |
| 2026-02-10 | Codex | Step 153: Language coverage integration tests across all supported languages + full projection matrix. 208/208 tests pass. |
| 2026-02-10 | Codex | Step 154: Agent library context payload sent on connect with primitives snapshot. 4/4 tests pass. |
| 2026-02-10 | Codex | Step 155: Agent library-aware code generation RPC + generator helper. 3/3 tests pass. |
| 2026-02-10 | Codex | Step 156: Agent annotation assistant RPC with suggestions/diagnostics, feedback learning, and mutation apply helpers. 5/5 tests pass. |
| 2026-02-10 | Codex | Step 157: Multi-agent roles with permission policy, agent provenance in transform history, and UI role controls. 8/8 tests pass. |
| 2026-02-10 | Codex | Step 158: Agent workflow recorder with JSON export, replay support, and RPC wiring. 5/5 tests pass. |
| 2026-02-10 | Codex | Step 159: Agent marketplace registry with UI panel, install toggles, and registry serialization. 6/6 tests pass. |
| 2026-02-10 | Codex | Step 160: Theme engine with JSON themes, ImGui styling, and syntax/editor color mapping. 4/4 tests pass. |
| 2026-02-10 | Codex | Step 161: Plugin API + loader with registry hooks for concepts/generators/grammars/annotations/panels/commands. 10/10 tests pass. |
| 2026-02-10 | Codex | Step 162: Help panel with Markdown sections + docs/help.md wired in UI. 4/4 tests pass. |
| 2026-02-10 | Codex | Step 163: Telemetry + crash logging (opt-in), settings toggle, and log writer. 3/3 tests pass. |
| 2026-02-10 | Codex | Step 164: Update checker stub + installer manifests/config, settings UI for update URL. 3/3 tests pass. |
| 2026-02-10 | Codex | Step 165: Sprint 5 integration tests across primitives, projection, pipeline, composition, and Emacs indexing. 8/8 tests pass. |
| 2026-02-10 | Codex | Added `file_limits_test` to enforce architecture file size limits (with temporary allowlist for known oversize headers). 4/4 tests pass. |
| 2026-02-10 | Codex | Step 167: Split EditorState into focused sub-states (Search/Agent/Build/Library/Emacs/UI), updated panels/handlers, and added step167_test. 1/1 tests pass. |
| 2026-02-10 | Codex | Step 168: Split oversized editor/AST component headers (CodeEditorWidget, SyntaxHighlighter, Parser, CppGenerator) with new step168 unit + integration tests. 79/79 tests pass (step168_test 75/75, step168_integration_test 4/4). file_limits_test 4/4 passes. |
| 2026-02-10 | Codex | Step 169: Notification/toast system with status bar history, output log rewire, and notification tests. 2/2 tests pass (step169_test, step169_integration_test). file_limits_test 4/4 passes. |
| 2026-02-10 | Codex | Step 170: UI event bus with debounced dispatch, editor wiring, and settings/theme events. 2/2 tests pass (step170_test, step170_integration_test). file_limits_test 4/4 passes. |
| 2026-02-10 | Codex | Step 171: Theme engine core enhancements (ThemeColor API, user theme dir, hot reload) with tests. 2/2 tests pass (step171_test, step171_integration_test). file_limits_test 4/4 passes. |
| 2026-02-10 | Codex | Step 172: Bundled theme pack renamed to Whetstone Dark/Light and Monokai Pro with tests. 2/2 tests pass (step172_test, step172_integration_test). file_limits_test 4/4 passes. |
| 2026-02-10 | Codex | Step 173: Theme gallery UI with hover preview, apply/reset, and swatches. 2/2 tests pass (step173_test, step173_integration_test). file_limits_test 4/4 passes. |
| 2026-02-10 | Codex | Step 174: Added IconSet system with theme-aware, zoom-scaled icons + tests. 2/2 tests pass (step174_test, step174_integration_test). file_limits_test 4/4 passes. |
| 2026-02-10 | Codex | Step 175: Typography controls (fonts, line height, letter spacing) + theme layout spacing. 2/2 tests pass (step175_test, step175_integration_test). file_limits_test 4/4 passes. |
| 2026-02-10 | Codex | Step 176: Smooth UI transitions (panel slides, tab fade, tooltip fade, toast animations, search pulse, cursor blink) + reduce-motion/cursor blink settings. 2/2 tests pass (step176_test, step176_integration_test). |
| 2026-02-10 | Codex | Step 177: Enhanced find/replace (match counts, regex preview, replace preview, selection scope, history, toggles, find next/prev) with SearchUtils. 2/2 tests pass (step177_test, step177_integration_test). |
| 2026-02-10 | Codex | Step 178: Multi-cursor editing (Alt+Click, Ctrl+D, Ctrl+Alt+Up/Down, column selection) with multi-caret rendering and shared edits. 2/2 tests pass (step178_test, step178_integration_test). |
| 2026-02-10 | Codex | Step 179: Rainbow brackets, bracket-pair highlight, scope tint, and auto-surround with language-specific auto-close. 2/2 tests pass (step179_test, step179_integration_test). |
| 2026-02-10 | Codex | Step 180: Rich tooltip system with markdown rendering, pinning, max size/scrolling, and tooltip replacements. 2/2 tests pass (step180_test, step180_integration_test). |
| 2026-02-10 | Codex | Step 181: Enhanced status bar (mode/language/encoding segments, centered notifications, selection counts, git branch, adaptive background). 2/2 tests pass (step181_test, step181_integration_test). |
| 2026-02-10 | Codex | Step 182: Problems panel overhaul with grouping, sortable table, and diagnostic badges on tabs. 2/2 tests pass (step182_test, step182_integration_test). |
| 2026-02-10 | Codex | Step 183: Tab reordering and untitled rename flow, plus buffer rename support. 2/2 tests pass (step183_test, step183_integration_test). |
| 2026-02-10 | Codex | Step 184: First-run wizard (layout/theme/keybindings + get-started actions) wired on missing session. 2/2 tests pass (step184_test, step184_integration_test). |
| 2026-02-10 | Codex | Step 185: Contextual feature hints with persistence + dismiss/disable. 2/2 tests pass (step185_test, step185_integration_test). |
| 2026-02-10 | Codex | Step 186: Keyboard shortcut reference panel with filter and live bindings. 2/2 tests pass (step186_test, step186_integration_test). |
| 2026-02-10 | Codex | Step 187: In-editor help panel window wired from menu with Markdown renderer reuse. 2/2 tests pass (step187_test, step187_integration_test). |
| 2026-02-10 | Codex | Step 188: Command palette upgrade (command/file modes, categories, recent section, context-aware filtering, inline params, fuzzy highlights). 1/1 tests pass (step188_test). |
| 2026-02-10 | Codex | Step 189: Guided workflow wizards (annotate file, cross-language project, connect agent) with shared wizard framework. 1/1 tests pass (step189_test). |
| 2026-02-10 | Codex | Step 190: Vulnerability database (OSV parsing, cache+TTL, offline mode, query/range matching, background refresh hook). 1/1 tests pass (step190_test). |
| 2026-02-10 | Codex | Step 191: Dependency security badges with severity colors, advisory details, safe upgrade path, and ignore list persistence. 1/1 tests pass (step191_test). |
| 2026-02-10 | Codex | Step 192: Security diagnostics integration (gutter shields, [Security] problems, vulnerable import blocking, and agent import deprioritization). 1/1 tests pass (step192_test). |
| 2026-02-10 | Codex | Step 193: Semantic library tags (semantic_tags.json storage, auto-tagging heuristics, tags attached to ExternalModule/TypeSignature). 1/1 tests pass (step193_test). |
| 2026-02-10 | Codex | Step 194: Semantic-filtered library browser (tag filter bar, active tag chips, tag badges, context-aware completion boosting, agent tag context). 1/1 tests pass (step194_test). |
| 2026-02-10 | Codex | Step 195: Security & semantic UX tests (vulnerability badges/advisories, security diagnostics severity mapping, semantic tag auto-assign, library tag filtering, vulnerable import deprioritization, safe upgrade writeback). 1/1 tests pass (step195_test). |
| 2026-02-10 | Codex | Step 196: High contrast + colorblind modes (high contrast theme, annotation shapes toggle, diagnostic pattern underlines). 1/1 tests pass (step196_test). |
| 2026-02-10 | Codex | Step 197: Keyboard navigation audit (F6 panel cycle, Esc focus return, focus ring defaults, nav focus enabled on panels). 1/1 tests pass (step197_test). |
| 2026-02-10 | Codex | Step 198: Virtual scrolling for large files (viewport + buffered rendering, fold hiding optimized). 1/1 tests pass (step198_test). |
| 2026-02-10 | Codex | Step 199: Large file handling (size thresholds, text-mode fallback, large file prompt, highlight disable, memory indicator). 1/1 tests pass (step199_test). |
| 2026-02-10 | Codex | Step 200: Startup/perf (LSP & highlight debounce, deferred AST sync on session restore, debug frame budget warnings). 1/1 tests pass (step200_test). |
| 2026-02-10 | Codex | Step 201: Sprint 6 integration checks (panel wiring, theme switching, multicursor, first-run wizard, security badge, keyboard navigation, large file settings, notifications). 1/1 tests pass (step201_test). |
| 2026-02-10 | Codex | Step 220: Evaluation framework scaffolding (EvalHarness, task/trace loading, scoring). 1/1 tests pass (step220_test). |
| 2026-02-10 | Codex | Step 166: Extract main.cpp into panel headers marked complete (already implemented). step166_test passes; file_limits_test 4/4 passes. |
| 2026-02-10 | Claude Opus 4.6 | Sprint 7 Phase 7a (Steps 202–206): API docs (AGENT_API.md, 23 methods), JSON schemas (20 files), exposed ContextAPI/BatchMutationAPI/Pipeline via RPC, permission policy updates. 51/51 tests pass. |
| 2026-02-10 | Claude Opus 4.6 | Sprint 7 Phase 7b (Steps 207–213): MCPServer.h (10 tools, 5 resources, 4 prompts, JSON-RPC 2.0 initialize handshake), MCPBridge.h (stdio transport). 90/90 tests pass. |
| 2026-02-10 | Claude Opus 4.6 | Sprint 7 Phase 7c (Steps 214–219): TraceGenerator.h (6 scenario templates, 9-sample code corpus, batch engine), TraceExporter.h (Anthropic/OpenAI/JSONL/Markdown export, filtering, statistics). 294/294 tests pass. |
