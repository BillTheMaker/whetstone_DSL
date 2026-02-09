# Sprint 4: Professional Editor — Checklist (Revised)

> **Goal:** Transform the proof-of-concept GUI into a professional structured code editor
> by wiring the 18 unused backend components, integrating LSP for language intelligence,
> and adding real editor features with proper layout presets.
>
> **Current state:** Basic ImGui InputTextMultiline with preview panels. Tiny dockable
> windows that require manual resizing. Backend has 33 components; GUI uses only 5.
> Only Python/C++/Elisp tree-sitter grammars.
>
> **Target state:** A usable code editor with layout presets (VSCode/Emacs/JetBrains),
> inline syntax highlighting, LSP-powered diagnostics and navigation, native file dialogs,
> annotation UI, cross-language projection, optimization controls, and support for 8+ languages.
>
> **Key architectural decisions (from feature review):**
> - LSP integration provides standard language intelligence; Whetstone layers annotation-specific features on top
> - Layout presets are a first-class concept like keybinding profiles (not an afterthought)
> - AST extended with Import/ExternalModule concepts to prepare for Sprint 5 library-aware coding
> - Tree-sitter grammars added for JS/TS, Java, Rust, Go alongside existing Python/C++/Elisp

---

## Phase 4a: Layout & Code Editor Core (Steps 76–83)

The two highest-impact gaps: the docking layout starts broken (tiny panels)
and the editor area is a plain textbox. Fix both before anything else.

- [ ] **Step 76: Layout presets and docking configuration**
  Create `LayoutManager.h` with preset docking layouts, selectable like keybinding
  profiles. Programmatically configure ImGui DockBuilder on first launch or profile switch.
  Presets:
  - **VSCode**: Explorer 20% left, Editor 60% center, Panel 20% bottom, status bar
  - **Emacs**: Full-frame editor, optional side panels, minibuffer-style bottom bar
  - **JetBrains**: Project tree left, Editor center, tool windows right and bottom
  Menu: View > Layout > {VSCode, Emacs, JetBrains}. Default = VSCode.
  Store user's choice in `~/.whetstone/layout.json`. Restore on next launch.
  *New:* `LayoutManager.h`, modifies `main.cpp`

- [ ] **Step 77: Custom code editor renderer**
  Replace InputTextMultiline with a custom ImGui text renderer that draws
  characters individually with per-token colors from SyntaxHighlighter.
  Cursor position, text selection, keyboard input handled manually.
  Monospace grid layout. Blinking cursor. Visible whitespace toggle.
  *New:* `CodeEditorWidget.h`

- [ ] **Step 78: Line numbers and gutter**
  Fixed-width gutter on the left with line numbers (gray, right-aligned).
  Gutter also serves as anchor for future markers (errors, annotations, breakpoints).
  Click gutter = select entire line. Current line highlighted.
  Gutter width auto-adjusts for file length (3-digit vs 4-digit line numbers).
  *Modifies:* `CodeEditorWidget.h`

- [ ] **Step 79: Auto-indent and smart editing**
  Wire `EditorMode.h` into the editor widget. On Enter: auto-indent based on
  language rules (Python: indent after colon, C++: indent after brace).
  Tab key inserts spaces per language config. Auto-close brackets/quotes.
  *Wires:* `EditorMode.h`

- [ ] **Step 80: Scroll, selection, clipboard**
  Smooth scrolling with mousewheel. Shift+click and Shift+arrow for selection.
  Ctrl+C/X/V clipboard integration (SDL clipboard API). Ctrl+A select all.
  Drag to select. Double-click selects word, triple-click selects line.
  *Modifies:* `CodeEditorWidget.h`

- [ ] **Step 81: Code folding**
  Collapse/expand function bodies and block statements. Fold indicators in the
  gutter (▸/▾). Folded regions show `{...}` placeholder. Fold state per-buffer.
  Uses tree-sitter node types to identify foldable ranges.
  *Modifies:* `CodeEditorWidget.h`

- [ ] **Step 82: Minimap**
  Narrow column on the right side of the editor showing a zoomed-out view of
  the full file. Click to scroll. Highlight viewport position. Color-coded
  to match syntax highlighting. Toggle via View menu.
  *Modifies:* `CodeEditorWidget.h`

- [ ] **Step 83: Additional tree-sitter grammars**
  Add tree-sitter grammars via FetchContent for JavaScript, TypeScript, Java,
  Rust, and Go. Add `EditorMode` configs for each (indent rules, comment style,
  bracket pairs). Language menu updated. Syntax highlighting works for all 8
  languages. Parser CST-to-AST stubs (full conversion is Sprint 5).
  *Modifies:* `CMakeLists.txt`, `EditorMode.h`, `SyntaxHighlighter.h`

---

## Phase 4b: File Management (Steps 84–89)

Real file operations: native dialogs, filesystem tree, multi-tab.

- [ ] **Step 84: Native file dialogs**
  Integrate `tinyfiledialogs` (single C file, no dependency) or Portable File
  Dialogs via FetchContent. Replace the manual path-entry popup with OS-native
  Open/Save dialogs. Save As when file is `(untitled)`. Filter by language
  extension. Also used for "Open Folder" to set workspace root.
  *New dependency + modifies:* `main.cpp`

- [ ] **Step 85: Filesystem tree in Explorer**
  Replace hardcoded file list with a real recursive directory tree.
  Root = workspace folder (CWD or user-selected via Open Folder). Tree nodes
  for folders (expandable) and files (click to open). File/folder icons by
  extension/type. Respects `.gitignore` patterns for hiding.
  *Modifies:* `main.cpp`

- [ ] **Step 86: Multi-tab editing**
  Wire `BufferManager.h` into the GUI. Each open file = a tab in the editor.
  Click tab to switch. Middle-click or X button to close. Modified indicator (dot)
  on tab. Ctrl+Tab to cycle. Close confirmation if modified. Each buffer gets
  its own TextEditor + TextASTSync + CodeEditorWidget state.
  *Wires:* `BufferManager.h`

- [ ] **Step 87: Welcome screen**
  Wire `WelcomeScreen.h`. Show as a tab on startup when no file is open.
  Recent files list (persisted to `~/.whetstone/recent.json`). Quick actions:
  New File, Open File, Open Folder. Whetstone feature highlights. Keyboard
  shortcut cheat sheet. Dismisses when a file is opened.
  *Wires:* `WelcomeScreen.h`

- [ ] **Step 88: Drag-and-drop file open**
  Handle SDL_DROPFILE events. Drop a file onto the editor window → open it
  in a new tab. Drop a folder → set as workspace root. Visual feedback
  during drag (overlay "Drop to open").
  *Modifies:* `main.cpp`

- [ ] **Step 89: File watching and auto-reload**
  Detect external file changes (stat-based polling every 2s). If file changed
  on disk and buffer is clean → auto-reload. If buffer is dirty → prompt
  "File changed on disk. Reload?" Auto-save option (configurable interval).
  *New:* `FileWatcher.h`

---

## Phase 4c: LSP Client & Diagnostics (Steps 90–96)

Two-layer intelligence: LSP for standard language features, Whetstone for annotations.
This is the biggest architectural change from the original plan.

- [ ] **Step 90: LSP client core**
  Create `LSPClient.h` implementing the Language Server Protocol client.
  Communicates with external LSP servers via JSON-RPC over stdin/stdout pipes.
  Handles: initialize, initialized, shutdown, exit lifecycle.
  Spawns LSP server processes (configurable per language).
  *New:* `LSPClient.h`

- [ ] **Step 91: LSP diagnostics integration**
  Wire `textDocument/didOpen`, `textDocument/didChange`, `textDocument/didSave`.
  Receive `textDocument/publishDiagnostics` from LSP server. Parse diagnostic
  ranges, severity, messages. Display in new "Problems" tab in bottom panel.
  Click to jump to location. Merge with Whetstone-specific diagnostics.
  *Modifies:* `LSPClient.h`, `main.cpp`

- [ ] **Step 92: LSP auto-completion**
  Send `textDocument/completion` requests as user types (debounced).
  Show completion popup near cursor with candidates from LSP. Icons by
  completion kind (function, variable, keyword, snippet). Tab/Enter to accept.
  Escape to dismiss. Filter as user types.
  *Modifies:* `LSPClient.h`, `CodeEditorWidget.h`

- [ ] **Step 93: LSP hover and signature help**
  On hover over a symbol, send `textDocument/hover` → show tooltip with
  type info and docs. On typing `(` after a function, send
  `textDocument/signatureHelp` → show parameter info popup.
  *Modifies:* `LSPClient.h`, `CodeEditorWidget.h`

- [ ] **Step 94: Whetstone diagnostics layer**
  Run `AnnotationValidator` and `StrategyValidator` alongside LSP diagnostics.
  Whetstone diagnostics appear in the same Problems panel, tagged "[Whetstone]".
  Gutter shows both LSP errors (red) and Whetstone annotation warnings (orange).
  Wire `Pipeline.h` for full analysis pass (debounced, cached).
  *Wires:* `AnnotationValidator.h`, `StrategyValidator.h`, `Pipeline.h`

- [ ] **Step 94a: Source span tracking**
  Add source spans (start/end line/column) to AST nodes and propagate them from
  tree-sitter CST nodes. Serialize spans in `Serialization.h`. Whetstone diagnostics
  map to exact ranges using node spans (no heuristic line guessing).
  *Modifies:* `ast/ASTNode.h`, `ast/Parser.h`, `ast/Serialization.h`

- [ ] **Step 95: Gutter markers and inline squiggles**
  Red circle (error) and yellow triangle (warning) icons in the gutter for
  lines with diagnostics (from both LSP and Whetstone). Hover shows message
  as tooltip. Red wavy underline under the specific token range. Uses
  LSP diagnostic ranges directly; maps Whetstone node IDs → source positions.
  *Modifies:* `CodeEditorWidget.h`

- [ ] **Step 96: LSP server configuration**
  Settings panel section for LSP servers. Default configs:
  - Python: `pylsp` or `pyright`
  - C/C++: `clangd`
  - Rust: `rust-analyzer`
  - Go: `gopls`
  - JS/TS: `typescript-language-server`
  - Java: `jdtls`
  User can override paths, add flags. Auto-detect installed servers.
  Schema validation via `ast/Schema.h` for Whetstone's AST layer.
  *Wires:* `ast/Schema.h`, modifies `SettingsManager.h`

---

## Phase 4d: Annotation UI (Steps 97–102)

Make Whetstone's unique memory annotations visible and editable.
This is what differentiates Whetstone from every other editor.

- [ ] **Step 97: Annotation gutter markers**
  Colored icons in the gutter for annotated functions/variables:
  blue = @Reclaim(Tracing), orange = @Owner(Single), red = @Deallocate(Explicit),
  green = @Lifetime(RAII), gray = @Allocate(Static). Hover shows annotation
  details. Click opens annotation editor.
  *Modifies:* `CodeEditorWidget.h`

- [ ] **Step 98: Inline annotation decorations**
  Render annotation tags inline above or beside the annotated code:
  `@Reclaim(Tracing)` as a subtle colored label. Click to edit.
  Toggle visibility via View > Show Annotations. Respects layout preset
  (VSCode shows above, Emacs shows in margin, JetBrains shows beside).
  *Modifies:* `CodeEditorWidget.h`

- [ ] **Step 99: Add/edit annotation context menu**
  Right-click on a function/variable → context menu with "Add Annotation...",
  "Edit Annotation", "Remove Annotation". Sub-menu lists available annotation
  types with strategy options. Mutations go through `ASTMutationAPI`.
  Changes recorded in orchestrator journal for undo.
  *Wires:* `ASTMutationAPI.h`

- [ ] **Step 100: Memory strategy suggestions**
  Wire `MemoryStrategyInference`. Show lightbulb icon in the gutter when
  the inferrer has a suggestion with confidence > 0.5. Click → popup showing
  suggested annotation with confidence score and reason. "Apply" button adds
  it via `ASTMutationAPI`. Suggestions refresh on language/code change.
  *Wires:* `MemoryStrategyInference.h`

- [ ] **Step 101: Annotation conflict highlighting**
  When `AnnotationValidator` detects a parent/child conflict, highlight both
  annotations with a red border and connecting line in the gutter. Tooltip
  explains the conflict. Quick-fix actions: "Remove child annotation",
  "Change to match parent".
  *Modifies:* `CodeEditorWidget.h`

- [ ] **Step 102: Memory strategy dashboard**
  New dockable panel: "Memory Strategies". Overview of all annotations in the
  current file. Grouped by type with counts. Click to navigate. Shows
  inference confidence for unannotated functions. Module-level strategy summary.
  Export annotations as JSON.
  *New:* `StrategyDashboard` panel in `main.cpp`

---

## Phase 4e: Cross-Language & Optimization (Steps 103–108)

Surface the code generation and optimization pipeline in the UI.

- [ ] **Step 103: Side-by-side generated code view**
  Split the editor area: source on left, generated code on right (read-only,
  syntax highlighted). Scroll-locked. Clicking a line in source highlights
  the corresponding generated output. Target language selector dropdown.
  *Wires:* `ast/Generator.h` (already used, now in split view)

- [ ] **Step 104: Cross-language projection button**
  Toolbar button: "Project to..." with dropdown (C++, Python, Elisp, plus
  new languages as generators are added). Uses `CrossLanguageProjector`
  to create a projected AST, opens generated code in a new tab (read-only,
  with annotations adapted). Shows annotation mapping summary in Output.
  *Wires:* `CrossLanguageProjector.h`

- [ ] **Step 105: Optimization controls panel**
  New "Optimize" panel or toolbar section. Buttons: "Constant Fold", "Dead Code
  Elimination", "Apply All". Each shows a result summary (nodes modified, warnings).
  Respects `StrategyAwareOptimizer` constraints — blocked operations grayed-out
  with tooltip explaining which annotation prevents them.
  *Wires:* `TransformEngine.h`, `StrategyAwareOptimizer.h`

- [ ] **Step 106: Transform history panel**
  New panel showing `IncrementalOptimizer.getTransformHistory()`. Each entry
  shows transform name, affected nodes, time. "Undo" button per entry.
  "Undo All" button. Provenance: hover a node in AST view → see which
  transform created/modified it. Color-coded by transform type.
  *Wires:* `IncrementalOptimizer.h`

- [x] **Step 107: Before/after diff view**
    When a transform is applied, show a diff view: original code on left,
    transformed on right. Changed lines highlighted in green/red.
    Accept/reject per-transform. "Preview" mode shows diff without applying.
    *Modifies:* `main.cpp`

  - [x] **Step 107a: Text-Editor Mode toggle**
    Add a per-buffer mode selector (Text vs Structured) in status bar or View menu.
    Text mode disables AST sync, generator/projection, and optimization panels.
    *Modifies:* `main.cpp`, `TextASTSync.h` (if needed for bypass)

  - [x] **Step 107b: UI behavior by mode**
    Hide or disable AST/Generated/Transforms/Optimize panels in Text mode.
    Generated split view collapses to single editor in Text mode.
    *Modifies:* `main.cpp`, `CodeEditorWidget.h`

  - [x] **Step 107c: Buffer persistence**
    Store mode per buffer and preserve on open/reopen; include in recent files.
    *Modifies:* `BufferManager.h`, `main.cpp`, `WelcomeScreen` persistence if needed

  - [x] **Step 108: Batch mutation UI**
    "Refactor" menu with batch operations: "Rename Variable", "Extract Function",
    "Inline Variable". Each uses `BatchMutationAPI.applySequence()` with atomic
    rollback on failure. Preview changes in diff view before applying.
    *Wires:* `BatchMutationAPI.h`

---

## Phase 4f: Navigation & Commands (Steps 109–114)

IDE-quality navigation, powered by LSP for standard features and ContextAPI for
Whetstone-specific AST navigation.

- [x] **Step 109: Command palette**
  Ctrl+Shift+P opens a fuzzy-searchable command list. All menu actions registered
  as commands. Type to filter. Shows keybinding next to each command.
  Most-recently-used commands float to top. Extensible for plugins.
  *New:* `CommandPalette.h`

- [x] **Step 110: Go-to-definition (LSP + Whetstone)**
  Ctrl+click on a symbol → jump to its definition. Primary: send
  `textDocument/definition` to LSP server. Fallback: use `ContextAPI.getInScopeSymbols()`
  for Whetstone AST-level resolution. Hover shows definition preview popup.
  Works across files when LSP supports it.
  *Wires:* `ContextAPI.h`, `LSPClient.h`

- [x] **Step 111: Symbol outline panel**
  New "Outline" panel. Primary: use LSP `textDocument/documentSymbol` for the
  symbol tree. Fallback: walk Whetstone AST for functions/variables/parameters.
  Click to navigate. Icons by symbol kind. Search/filter box.
  *Wires:* `ContextAPI.h`, `LSPClient.h`

- [x] **Step 112: Breadcrumb navigation**
  Bar above the editor showing the current scope path:
  `Module > Function:add > body > Return`. Click any segment to jump.
  Updates as cursor moves. Uses Whetstone AST parent chain (our unique feature —
  LSPs don't provide this annotation-aware scope path).
  *Modifies:* `CodeEditorWidget.h`

- [x] **Step 113: Project-wide search**
  Ctrl+Shift+F opens search panel. Searches all files in the workspace.
  Results grouped by file with line previews. Click to open file at location.
  Regex support. Include/exclude glob filters.
  *New:* `ProjectSearch.h`

- [x] **Step 114: Go-to-line**
  Ctrl+G opens a quick input: type line number → jump. Supports `:line:col`
  format. Shows total line count.
  *Modifies:* `main.cpp`

---

## Phase 4g: Infrastructure & Persistence (Steps 115–121)

Wire the orchestrator, serialization, session management, and prepare
AST extensions for Sprint 5.

- [x] **Step 115: Wire Orchestrator**
  Connect `Orchestrator.h` to the GUI. All AST mutations go through the
  orchestrator's undo/redo journal (replacing TextEditor's simple stack).
  Orchestrator manages the canonical AST state. GUI reads from it.
  Emacs daemon config path exposed as a setting (prepares for Sprint 5
  Emacs library integration — defaults to system Emacs, user can point to
  their own `~/.emacs.d/`).
  *Wires:* `Orchestrator.h`

- [x] **Step 116: Project save/load (AST serialization)**
  Save/load `.whetstone` project files using `ast/Serialization.h`.
  JSON format preserving the full AST with annotations. File > Save Project /
  Open Project. Includes workspace root path and open buffer list.
  *Wires:* `ast/Serialization.h`

- [x] **Step 117: Session persistence**
  Remember window layout (docking configuration), open files, active tab,
  cursor positions, fold state, and layout preset across sessions.
  Saved to `~/.whetstone/session.json`. Restore on next launch.
  *New:* `SessionManager.h`

- [x] **Step 118: Settings panel**
  View > Settings opens a dockable panel. Options: font size (12–24),
  tab size (2/4/8), theme variant (Dark/Light), auto-save interval,
  show minimap, show line numbers, LSP server paths, layout preset,
  keybinding profile. Persisted to `~/.whetstone/settings.json`.
  *New:* `SettingsManager.h`

- [ ] **Step 119: Zoom**
  Ctrl+= zoom in, Ctrl+- zoom out, Ctrl+0 reset. Scales the editor font
  size dynamically. Status bar shows current zoom level.
  *Modifies:* `main.cpp`

- [ ] **Step 120: Undo/Redo rework**
  Unify undo stacks: orchestrator journal is the single source of truth.
  TextEditor undo feeds into orchestrator. Undo/redo affects both text and
  AST atomically. Status bar shows undo depth.
  *Modifies:* `Orchestrator.h`, `main.cpp`

- [ ] **Step 121: AST Import/ExternalModule concepts**
  Extend `ASTNode.h` with new concept types: `Import` (represents `import numpy`,
  `#include <vector>`, `(require 'cl-lib)`), `ExternalModule` (represents an
  external library with its public API surface), `TypeSignature` (function
  parameter and return types from external APIs). Serialization support.
  These are data-only stubs in Sprint 4 — Sprint 5 populates them with real
  library metadata.
  *Modifies:* `ast/ASTNode.h`, `ast/Serialization.h`

---

## Phase 4h: Agent & Terminal (Steps 122–126)

External tool integration and agent connectivity.

- [ ] **Step 122: Integrated terminal**
  New "Terminal" tab in the bottom panel. Subprocess output capture (not a
  full terminal emulator — captures stdout/stderr from spawned processes).
  Run shell commands, see output. Ctrl+` to toggle. Working directory =
  workspace root. Scrollable output with ANSI color support.
  *New:* `TerminalPanel.h`

- [ ] **Step 123: Run code button**
  Toolbar button (play icon) to run the current file. Auto-detects runner:
  Python: `python file.py`, C++: compile and run, Rust: `cargo run`,
  Go: `go run`, JS: `node file.js`. Output goes to Terminal panel.
  Status bar shows "Running..." / exit code. Ctrl+Shift+B for build.
  *Modifies:* `main.cpp`

- [ ] **Step 124: Wire WebSocket agent server**
  Start `WebSocketAgentServer` when the editor launches (configurable port).
  External AI agents can connect, query the AST, and apply mutations via
  JSON-RPC. Agent activity logged in "Agents" tab in the bottom panel.
  Agents receive LSP diagnostics and Whetstone annotations as context.
  *Wires:* `WebSocketServer.h`

- [ ] **Step 125: Connected agents panel**
  "Agents" tab shows currently connected agents: name, session ID, last activity.
  Manual disconnect button. Live activity log showing JSON-RPC method calls
  and results. Agents can be granted/revoked mutation permissions.
  *Modifies:* `main.cpp`

- [ ] **Step 126: Build system integration**
  Detect project type (CMakeLists.txt, setup.py, Cargo.toml, package.json,
  Makefile, go.mod) and offer build commands. Build output in Terminal panel.
  Error parsing: click compiler errors to jump to source location.
  *New:* `BuildSystem.h`

---

## Summary

| Phase | Steps | Description | Key Components Wired |
|-------|-------|-------------|---------------------|
| 4a | 76–83 | Layout presets + code editor core | LayoutManager, EditorMode, 5 new tree-sitter grammars |
| 4b | 84–89 | File management | BufferManager, WelcomeScreen, FileWatcher |
| 4c | 90–96 | LSP client + diagnostics | LSPClient, AnnotationValidator, StrategyValidator, Pipeline, Schema |
| 4d | 97–102 | Annotation UI | ASTMutationAPI, MemoryStrategyInference |
| 4e | 103–108 | Cross-language + optimization | CrossLanguageProjector, TransformEngine, StrategyAwareOptimizer, IncrementalOptimizer, BatchMutationAPI |
| 4f | 109–114 | Navigation + commands | ContextAPI, LSPClient |
| 4g | 115–121 | Infrastructure + persistence + AST extensions | Orchestrator, Serialization, Import/ExternalModule AST concepts |
| 4h | 122–126 | Agent + terminal | WebSocketServer, BuildSystem |

**Total: 51 steps across 8 phases.**

**Key changes from original plan:**
1. Layout presets added as Step 76 (first thing built)
2. Phase 4c rebuilt around LSP client as primary intelligence, Whetstone as overlay
3. Phase 4f navigation uses LSP with Whetstone AST fallback
4. 5 new tree-sitter grammars added (JS/TS, Java, Rust, Go) in Phase 4a
5. Import/ExternalModule AST concepts added (Step 121) to prepare for Sprint 5
6. Emacs daemon config exposed as a setting (Step 115) for Sprint 5 Emacs libs
