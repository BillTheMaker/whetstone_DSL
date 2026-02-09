# Sprint 5: Library-Aware Constructive Coding — Plan

> **Goal:** Make Whetstone a library-aware, multi-language, agent-powered coding
> environment where imported libraries inform suggestions, completions, and agent
> recommendations — as a convenience, not a hard constraint. Users can always write
> whatever code they want. Integrate Emacs ecosystem for Elisp users. Expand language
> coverage with full generators. Build the constructive coding flow that makes
> Whetstone unique.
>
> **Prerequisites:** Sprint 4 complete (LSP integration, layout presets, annotation UI,
> AST Import/ExternalModule concept stubs, Orchestrator wired, 8+ tree-sitter grammars).
>
> **Key themes:**
> 1. Library/dependency management and API indexing
> 2. Constructive coding flow — agents prioritize imported library primitives (convenience, not constraint)
> 3. Emacs ecosystem integration (user libraries, packages, init.el)
> 4. Full code generators for all supported languages
> 5. Advanced agent capabilities

---

## Phase 5a: Library & Dependency Management (Steps 127–133)

The foundation: import libraries, parse their APIs, index their symbols.

- [x] **Step 127: Package registry abstraction**
  Create `PackageRegistry.h` with a unified interface for querying package
  metadata across ecosystems. Adapters for:
  - Python: PyPI (REST API for package info, version listing)
  - JavaScript/TypeScript: npm registry
  - Rust: crates.io
  - Go: pkg.go.dev / Go module proxy
  - Java: Maven Central
  - C/C++: vcpkg manifest / Conan
  Each adapter returns: package name, available versions, description,
  dependencies, homepage URL.
  *New:* `PackageRegistry.h`

- [x] **Step 128: Dependency file parsing**
  Parse existing dependency files to discover what's already imported:
  - Python: `requirements.txt`, `pyproject.toml`, `setup.py`
  - JS/TS: `package.json`
  - Rust: `Cargo.toml`
  - Go: `go.mod`
  - Java: `pom.xml`, `build.gradle`
  - C/C++: `vcpkg.json`, `CMakeLists.txt` (find_package)
  Populate `Import` AST nodes from parsed dependencies.
  *New:* `DependencyParser.h`, modifies `ast/ASTNode.h` (Import concept)

- [x] **Step 129: Dependency management UI**
  New dockable panel: "Dependencies". Lists all imported packages with version.
  "Add Package" button → search dialog (queries PackageRegistry). Version
  selector dropdown. "Remove" button. "Update" shows available newer versions.
  Changes write back to the appropriate dependency file.
  *New:* `DependencyPanel` in `main.cpp`

- [x] **Step 130: Library API indexing via LSP**
  When a package is imported, leverage the LSP server to extract its public API
  surface. Send `workspace/symbol` queries and `textDocument/completion` with
  library prefix to discover available functions, classes, constants.
  Cache results in `ExternalModule` AST nodes (name, public symbols, type signatures).
  *Modifies:* `LSPClient.h`, `ExternalModule` AST concept

- [ ] **Step 131: Library API indexing via type stubs**
  Fallback for when LSP isn't available or insufficient. Parse type stub files:
  - Python: `.pyi` stub files (typeshed, bundled stubs)
  - TypeScript: `.d.ts` declaration files
  - C/C++: header files (tree-sitter parse for function signatures)
  - Rust: parse `lib.rs` public items
  Extract function signatures, class/struct definitions, constants into
  `TypeSignature` AST nodes attached to `ExternalModule`.
  *New:* `StubParser.h`, modifies `ExternalModule` AST concept

- [ ] **Step 132: Library symbol browser**
  New panel or Explorer sub-section: "Libraries". Tree view of imported
  libraries → modules → classes/functions → parameters. Click to see
  documentation (from LSP hover or stub docstrings). Search/filter box.
  Drag a symbol into the editor → inserts a usage template.
  *New:* `LibraryBrowser` panel

- [ ] **Step 133: Import statement generation**
  When user references a symbol from an `ExternalModule`, auto-generate
  the correct import statement for the target language:
  - Python: `from numpy import array`
  - JS: `import { useState } from 'react'`
  - C++: `#include <vector>`
  - Rust: `use std::collections::HashMap;`
  - Go: `import "fmt"`
  - Elisp: `(require 'cl-lib)`
  Imports auto-sorted per language convention. Unused imports flagged.
  *Modifies:* generators in `Generator.h`, `CodeEditorWidget.h`

---

## Phase 5b: Constructive Coding Flow (Steps 134–140)

The core innovation: agents and completions **prioritize** the primitives
available in the user's imported libraries. Code is built UP from library
building blocks rather than hallucinated from training data. Imported libraries
are a convenience, not a 100% constraint — users can always write any code
they want, but the system surfaces library functions first and nudges toward
what's already available.

- [ ] **Step 134: Available primitives registry**
  Create `PrimitivesRegistry.h` that aggregates all available symbols:
  - Built-in language primitives (Python builtins, C++ std library, etc.)
  - Imported library symbols (from `ExternalModule` AST nodes)
  - User-defined symbols (from the current module's AST)
  Registry provides: `getAvailableFunctions()`, `getAvailableTypes()`,
  `getAvailableConstants()`, filtered by scope and import state.
  *New:* `PrimitivesRegistry.h`

- [ ] **Step 135: Library-aware completion**
  Modify the completion system (LSP + Whetstone) to **prioritize** symbols
  from `PrimitivesRegistry` without excluding others. When an agent or user
  types, completion candidates are ranked: imported library functions first,
  then builtins, then other known symbols. Unimported symbols shown dimmed
  with "auto-import" action. Users can always type and use anything —
  library awareness is a convenience, not a gate. Completions include
  parameter types from `TypeSignature`.
  *Modifies:* `CodeEditorWidget.h`, `LSPClient.h`

- [ ] **Step 136: Agent library-aware mode**
  New agent capability flag: `preferImports`. When set, the agent
  (via WebSocket API) **prefers** inserting code that references symbols in
  `PrimitivesRegistry`, and warns (not rejects) when using symbols outside
  imported libraries. The `ASTMutationAPI` checks mutations: if a mutation
  references a function not in any `ExternalModule` or the current module,
  return a warning listing available alternatives — but still allow the
  mutation. An optional `strictMode` flag is available for users who DO want
  hard enforcement, but it defaults to off.
  *Modifies:* `ASTMutationAPI.h`, `WebSocketServer.h`

- [ ] **Step 137: Function composition builder**
  New UI mode: "Compose". Shows a visual flow of available library functions.
  User picks a function → sees its inputs/outputs → picks the next function
  whose input matches the previous output. Builds a pipeline/chain visually.
  Generates code from the composition. Think: visual dataflow programming
  backed by real library APIs.
  *New:* `CompositionBuilder.h`, `CompositionPanel` in `main.cpp`

- [ ] **Step 138: Type-aware code generation**
  Enhance generators to use `TypeSignature` information from imported libraries.
  When generating C++ from Python code that uses numpy, the generator knows
  `numpy.array` maps to `std::vector<double>` (or Eigen::MatrixXd).
  Cross-language projection respects library type mappings.
  *Modifies:* `CrossLanguageProjector.h`, `Generator.h`

- [ ] **Step 139: Library compatibility matrix**
  Track which libraries have equivalents across languages. Example:
  `numpy` (Python) ↔ `Eigen` (C++) ↔ `ndarray` (Rust).
  When projecting code cross-language, suggest equivalent libraries.
  User confirms library mapping before projection. Stored as a
  configurable JSON mapping file.
  *New:* `LibraryCompatibility.h`

- [ ] **Step 140: Constructive coding tests**
  End-to-end tests verifying:
  1. Import numpy → agent prioritizes numpy/builtin functions in suggestions
  2. Remove import → agent's prioritized primitives shrink (but arbitrary code still allowed)
  3. Agent with `preferImports` warns on out-of-library usage but doesn't block
  4. Agent with optional `strictMode` does block out-of-library usage
  5. Cross-language projection with library mapping works
  6. Function composition builder generates correct code
  7. Type-aware generation produces compilable output
  *New:* `step140_test.cpp` and supporting test infrastructure

---

## Phase 5c: Emacs Ecosystem Integration (Steps 141–146)

For Emacs users: respect their init.el, load their packages, expose Emacs
functionality through the Whetstone UI.

- [ ] **Step 141: Emacs daemon with user config**
  Modify the Orchestrator's Emacs daemon to start with the user's init file.
  Setting: "Emacs Config Path" (defaults to `~/.emacs.d/init.el`).
  Daemon loads user's packages (MELPA, ELPA, straight.el, use-package).
  Startup log shown in Output panel. Errors in init.el reported as diagnostics.
  *Modifies:* `EmacsIntegration.h`, `Orchestrator.h`

- [ ] **Step 142: Elisp package browser**
  New panel section for Emacs users: "Emacs Packages". Lists loaded packages
  from the running daemon (query via `package-alist`). Shows: package name,
  version, description. "Load Package" button sends `(require 'package-name)`
  to the daemon. Status: loaded/available/not-installed.
  *New:* `EmacsPackageBrowser` panel

- [ ] **Step 143: Elisp function discovery**
  Query the running Emacs daemon for available functions from loaded packages.
  Send `(apropos-internal "prefix")` → get function list.
  Send `(describe-function 'name)` → get docstring and signature.
  Populate `ExternalModule` AST nodes for Elisp libraries.
  Feed into `PrimitivesRegistry` for constrained coding.
  *Modifies:* `ElispCommandBuilder`, `PrimitivesRegistry.h`

- [ ] **Step 144: Emacs keybinding deep integration**
  Go beyond the simple keybinding profile. In Emacs layout mode, support:
  - M-x command execution (sends to Emacs daemon)
  - Emacs-style prefix keys (C-x C-f, C-x C-s, C-c prefix maps)
  - Minibuffer emulation at the bottom of the editor
  - Mode line showing major/minor modes
  Keybindings loaded from user's Emacs config (key-binding query to daemon).
  *Modifies:* `KeybindingManager.h`, `LayoutManager.h`

- [ ] **Step 145: Org-mode and Literate programming support**
  Parse `.org` files using a tree-sitter-org grammar. Render org-mode
  documents with headings, source blocks, and prose. Source blocks
  are editable with full syntax highlighting and LSP support.
  Execute source blocks via the Emacs daemon or language runners.
  Results rendered inline (like Jupyter notebooks).
  *New:* tree-sitter-org grammar, `OrgMode.h`

- [ ] **Step 146: Emacs-Whetstone bridge**
  Bidirectional sync: Emacs buffers ↔ Whetstone buffers. When the user
  has the Emacs layout active, they can switch to pure Emacs (daemon
  connected to a graphical frame) for operations Whetstone doesn't
  support yet, then switch back. Buffer state synchronized via
  the Orchestrator's RPC channel.
  *Modifies:* `Orchestrator.h`, `EmacsIntegration.h`

---

## Phase 5d: Full Language Coverage (Steps 147–153)

Complete generators and CST-to-AST conversion for all supported languages.
Sprint 4 added tree-sitter grammars for syntax highlighting; Sprint 5
adds full semantic support.

- [ ] **Step 147: JavaScript/TypeScript CST-to-AST**
  Full tree-sitter CST-to-AST conversion for JavaScript and TypeScript.
  Handle: functions (arrow, declaration, expression), classes, async/await,
  destructuring, template literals, modules (import/export), JSX.
  TypeScript: type annotations mapped to Whetstone Type AST nodes.
  *Modifies:* `Parser.h`

- [ ] **Step 148: JavaScript/TypeScript generator**
  Generate JavaScript and TypeScript source from Whetstone AST.
  Handle: function declarations, arrow functions, classes, async/await,
  destructuring, template literals, import/export statements.
  TypeScript generator includes type annotations. Memory annotations
  map to WeakRef/FinalizationRegistry for @Reclaim, ownership comments
  for documentation.
  *Modifies:* `Generator.h`

- [ ] **Step 149: Java CST-to-AST and generator**
  Full tree-sitter Java parsing: classes, interfaces, methods, generics,
  annotations (Java @annotations map to Whetstone annotations), try/catch,
  lambdas. Generator produces compilable Java with proper class wrapping,
  imports, access modifiers. Memory annotations map to: @Reclaim(Tracing)
  → GC-managed (default), @Owner(Single) → AutoCloseable/try-with-resources.
  *Modifies:* `Parser.h`, `Generator.h`

- [ ] **Step 150: Rust CST-to-AST and generator**
  Full tree-sitter Rust parsing: functions, structs, enums, traits, impls,
  lifetimes, pattern matching, Result/Option. Generator produces Rust with
  proper ownership semantics. Memory annotations map naturally: @Owner(Single)
  → owned types, @Owner(Shared_ARC) → Arc<T>, @Lifetime(RAII) → Drop trait,
  @Reclaim(Tracing) → Rc<T> with caution note.
  *Modifies:* `Parser.h`, `Generator.h`

- [ ] **Step 151: Go CST-to-AST and generator**
  Full tree-sitter Go parsing: functions, structs, interfaces, goroutines,
  channels, defer, multiple return values. Generator produces Go with
  proper package/import structure. Memory annotations map to: @Reclaim(Escape)
  → heap allocation via escape analysis, @Deallocate(Explicit) → manual
  Close()/Release() patterns, @Owner(Single) → ownership convention comments.
  *Modifies:* `Parser.h`, `Generator.h`

- [ ] **Step 152: Cross-language projection for new languages**
  Extend `CrossLanguageProjector` to handle projection between all pairs of
  supported languages (Python, C++, Elisp, JS/TS, Java, Rust, Go).
  Annotation adaptation rules for each target. Library compatibility
  mappings (Phase 5b) used to suggest equivalent libraries.
  *Modifies:* `CrossLanguageProjector.h`

- [ ] **Step 153: Language coverage tests**
  End-to-end tests for each new language: parse → annotate → validate →
  optimize → generate. Cross-language projection tests between all supported
  pairs. Verify memory annotation semantics are preserved across projections.
  *New:* `step153_test.cpp` (multi-language integration test)

---

## Phase 5e: Advanced Agent Capabilities (Steps 154–159)

Make agents truly useful: they understand libraries, prioritize available
primitives, and assist with constructive coding.

- [ ] **Step 154: Agent library context**
  When an agent connects via WebSocket, it receives the current
  `PrimitivesRegistry` state: available libraries, their functions,
  type signatures. The agent's `getAST` response includes `ExternalModule`
  nodes. Agents can query: `getAvailablePrimitives(scope)` to see what
  libraries are imported — this informs their suggestions but doesn't
  limit what they can produce.
  *Modifies:* `WebSocketServer.h`, `ASTQueryAPI`

- [ ] **Step 155: Agent library-aware code generation**
  New agent RPC method: `generateCode(spec, preferences)`. Agent provides
  a natural language spec ("sort this list, then filter by threshold"),
  Whetstone generates code preferring available library primitives.
  Uses `PrimitivesRegistry` to select appropriate functions when possible,
  falls back to standard library or raw code when no library match exists.
  Returns AST nodes that the agent can review before insertion.
  *New:* `AgentCodeGen.h`

- [ ] **Step 156: Agent annotation assistant**
  Agent can request annotation suggestions for a code region. Whetstone
  runs `MemoryStrategyInference` and `AnnotationValidator`, returns
  suggestions with confidence scores. Agent can apply suggestions via
  `ASTMutationAPI`. The agent learns from user's accept/reject decisions
  to improve future suggestions.
  *Modifies:* `WebSocketServer.h`, `MemoryStrategyInference.h`

- [ ] **Step 157: Multi-agent collaboration**
  Support multiple agents connected simultaneously with different roles:
  - "Linter" agent: reads AST, reports issues, doesn't mutate
  - "Refactor" agent: can mutate with user approval
  - "Generator" agent: can insert new code
  Permission model: each agent role has allowed RPC methods.
  Agent actions appear in the transform history with agent name as provenance.
  *Modifies:* `WebSocketServer.h`, `IncrementalOptimizer.h`

- [ ] **Step 158: Agent workflow recording**
  Record agent actions as reproducible workflows. A sequence of agent
  RPC calls → saved as a "workflow script" (JSON). User can replay
  workflows on different codebases. Workflows are shareable.
  *New:* `WorkflowRecorder.h`

- [ ] **Step 159: Agent marketplace / plugin registry**
  Directory of available Whetstone agents (local + remote). Each agent
  has: name, description, capabilities, required permissions.
  "Install Agent" connects to the agent's WebSocket endpoint.
  Community agents discoverable via a registry URL (configurable).
  *New:* `AgentRegistry.h`, `AgentMarketplace` panel

---

## Phase 5f: Polish & Ecosystem (Steps 160–165)

Final polish, documentation, and ecosystem features.

- [ ] **Step 160: Theme engine**
  Full theme system beyond Dark/Light. Load themes from JSON files.
  Theme affects: editor colors, syntax highlighting colors, gutter colors,
  annotation marker colors, panel backgrounds. Ship with: VSCode Dark,
  Monokai, Solarized Dark, Solarized Light, Dracula, Nord.
  Theme gallery in Settings panel.
  *New:* `ThemeEngine.h`

- [ ] **Step 161: Extension/plugin API**
  Define a stable plugin interface. Plugins can:
  - Register new AST concept types
  - Add generators for new languages
  - Add tree-sitter grammars
  - Register custom annotations
  - Add panels to the UI
  - Register commands and keybindings
  Plugins loaded from `~/.whetstone/plugins/`. Each plugin = a shared
  library (.dll/.so) with a defined entry point.
  *New:* `PluginAPI.h`, `PluginLoader.h`

- [ ] **Step 162: User documentation**
  In-editor help system. Help > Documentation opens a panel with:
  - Getting Started guide
  - Annotation reference (all annotation types with examples)
  - Keyboard shortcuts (generated from KeybindingManager)
  - Language support matrix
  - Agent API reference
  Content in Markdown, rendered in ImGui.
  *New:* `HelpPanel.h`, markdown renderer

- [ ] **Step 163: Telemetry and crash reporting (opt-in)**
  Optional, anonymous usage telemetry to understand feature adoption.
  Crash handler that captures stack traces on unhandled exceptions.
  User must explicitly opt-in during first launch. Clear privacy policy.
  Data sent to configurable endpoint (or saved locally).
  *New:* `Telemetry.h`

- [ ] **Step 164: Installer and distribution updates**
  Update the installer (Inno Setup for Windows, .deb/.rpm for Linux) to
  include all new dependencies. Auto-update check on launch (checks a
  release URL). Portable mode (no install, run from folder).
  *Modifies:* `installer/`

- [ ] **Step 165: Sprint 5 integration tests**
  Comprehensive integration tests covering the full Sprint 5 feature set:
  1. Import library → API indexed → library-aware completion prioritizes imports
  2. Emacs daemon with user config → packages loaded → Elisp functions available
  3. Agent connects → library primitives prioritized → generates valid code (can use non-imported symbols too)
  4. Cross-language projection Python→Rust with library mapping
  5. Function composition builder → generated code compiles
  6. Full pipeline across all 8+ languages
  *New:* `step165_test.cpp`

---

## Summary

| Phase | Steps | Description |
|-------|-------|-------------|
| 5a | 127–133 | Library & dependency management (package registry, API indexing, import generation) |
| 5b | 134–140 | Constructive coding flow (primitives registry, library-aware completion, agent library preference, composition builder) |
| 5c | 141–146 | Emacs ecosystem (user config, package browser, function discovery, Org-mode, Emacs-Whetstone bridge) |
| 5d | 147–153 | Full language coverage (JS/TS, Java, Rust, Go generators + CST-to-AST) |
| 5e | 154–159 | Advanced agents (library context, library-aware generation, multi-agent, workflows, marketplace) |
| 5f | 160–165 | Polish & ecosystem (themes, plugins, docs, distribution) |

**Total: 39 steps across 6 phases.**

---

## Context for Future Agents

### What exists after Sprint 4 (prerequisites):
- 33+ backend components all wired into the GUI
- LSP client communicating with external language servers
- Layout presets (VSCode/Emacs/JetBrains) with proper docking
- Custom code editor widget with syntax highlighting, line numbers, gutter
- Annotation UI (gutter markers, inline decorations, context menu, suggestions)
- Cross-language projection and optimization controls
- AST with Import/ExternalModule/TypeSignature stub concepts
- 8+ tree-sitter grammars (Python, C++, Elisp, JS/TS, Java, Rust, Go)
- Orchestrator wired with Emacs daemon config as a setting
- WebSocket agent server running with JSON-RPC

### Key source files to understand:
| File | What it does |
|------|-------------|
| `editor/src/ast/ASTNode.h` | Base AST node + Import/ExternalModule (added Sprint 4 Step 121) |
| `editor/src/ast/Parser.h` | TreeSitterParser — CST-to-AST for Python/C++/Elisp (extend for new languages) |
| `editor/src/ast/Generator.h` | PythonGenerator, CppGenerator, ElispGenerator (extend for new languages) |
| `editor/src/LSPClient.h` | LSP protocol client (added Sprint 4 Phase 4c) |
| `editor/src/Pipeline.h` | End-to-end parse→infer→validate→optimize→generate |
| `editor/src/CrossLanguageProjector.h` | AST deep-copy with language change and annotation adaptation |
| `editor/src/MemoryStrategyInference.h` | Infers annotations from language + patterns |
| `editor/src/WebSocketServer.h` | Agent WebSocket endpoint with JSON-RPC |
| `editor/src/ASTMutationAPI.h` | AST mutations with OptimizationLock checking |
| `editor/src/EmacsIntegration.h` | ElispCommandBuilder + EmacsConnection daemon lifecycle |
| `editor/src/Orchestrator.h` | Central state manager, undo journal, Emacs integration |
| `editor/src/CodeEditorWidget.h` | Custom ImGui code editor (added Sprint 4 Phase 4a) |
| `editor/src/LayoutManager.h` | Docking layout presets (added Sprint 4 Step 76) |

### Architecture principles:
1. **Two-layer intelligence**: LSP for standard language features, Whetstone for annotations/memory/optimization
2. **Constructive coding**: imported libraries are a convenience, not a constraint — they inform suggestions and prioritization, but users/agents can always write arbitrary code
3. **Cross-language**: single AST, multiple generators, annotation-aware projection
4. **Agent-friendly**: all features accessible via WebSocket JSON-RPC, not just GUI
5. **User choice**: layout/keybinding/theme presets let users work in their preferred style

### Build system:
- C++20, CMake, vcpkg (Windows), FetchContent for tree-sitter grammars
- Dear ImGui + SDL2 + OpenGL3 for GUI
- nlohmann-json for JSON
- tree-sitter for parsing (grammars: python, cpp, elisp, + 5 more in Sprint 4)
- Build: `cmake --preset windows-msvc && cmake --build build --config Release`
- Tests: `build/Release/stepNN_test.exe` (each step has its own test executable)

### Git structure:
- Branch: `001-core-ast-structure` (all work so far)
- Main branch: `main` (merge target)
- Sprint plans: `sprint4_plan.md`, `sprint5_plan.md`
- Progress tracker: `progress.md` (authoritative, updated after each session)
