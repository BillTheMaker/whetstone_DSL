# Sprint 6: UX & Editor Polish — Plan

> **Goal:** Transform Whetstone from a feature-complete prototype into a
> polished, discoverable, accessible editor that users actually enjoy using.
> Address accumulated structural debt, add visual refinement, improve
> onboarding, and integrate the security/semantic feature requests.
>
> **Prerequisites:** Sprint 5 Phase 5a–5b complete (library-aware coding flow).
> Sprint 5 Phases 5c–5f may still be in progress — Sprint 6 work is
> independent and can proceed in parallel.
>
> **Key themes:**
> 1. Structural refactor — enforce architecture limits, enable faster UI iteration
> 2. Theme engine and visual design system
> 3. Editor UX enhancements (multi-cursor, find/replace, diagnostics)
> 4. Onboarding, discoverability, and documentation
> 5. Security vulnerability awareness (from feature requests)
> 6. Accessibility and performance

---

## Phase 6a: Structural Refactor (Steps 166–170)

The foundation: main.cpp is 2600+ lines and EditorState.h is 1800+ lines,
both far over the architecture limits (1500 and 600 respectively). Fix this
first — all UX work becomes easier when panels are modular.

- [x] **Step 166: Extract main.cpp into panel headers**
  Create `editor/src/panels/` directory. Extract each major UI section from
  main.cpp's render loop into its own header with a `renderXxx(EditorState&)`
  free function:
  - `panels/MenuBarPanel.h` — menu bar and toolbar
  - `panels/ExplorerPanel.h` — file tree and workspace browser
  - `panels/EditorPanel.h` — code editor area, tabs, split view
  - `panels/BottomPanel.h` — output, AST, terminal, problems, agents tabs
  - `panels/StatusBarPanel.h` — status bar with mode/language/position
  - `panels/SidePanels.h` — outline, dependencies, library browser, etc.
  main.cpp reduced to: init, event loop, docking layout, panel dispatch.
  Target: main.cpp under 800 lines.
  *New:* `panels/` directory with 6+ headers. *Modifies:* `main.cpp`

- [x] **Step 167: Split EditorState into focused sub-states**
  Break the 1800-line EditorState.h into composable sub-structs:
  - `SearchState` — find/replace, project search, go-to-line fields
  - `AgentState` — WebSocket server, agent log, permissions
  - `BuildState` — build system, errors, run state
  - `LibraryState` — dependency panel, library browser, composition panel
  - `EmacsState` — packages, function index, keybinding state, org doc
  - `UIFlags` — panel visibility toggles, bottom tab selection
  EditorState composes these via named members. Target: EditorState.h under
  400 lines, each sub-state under 200 lines.
  *Modifies:* `EditorState.h`. *New:* sub-state headers or inline structs.

- [x] **Step 168: Split oversized component headers**
  Enforce the 600-line limit on remaining violators:
  - `CodeEditorWidget.h` (1112 lines): extract rendering helpers into
    `CodeEditorRendering.h`, selection logic into `CodeEditorSelection.h`
  - `ast/Parser.h` (1093 lines): extract per-language CST-to-AST into
    `ast/PythonParser.h`, `ast/CppParser.h`, `ast/ElispParser.h` etc.
  - `SyntaxHighlighter.h` (762 lines): extract language keyword tables
    into `SyntaxLanguages.h`
  - `ast/CppGenerator.h` (708 lines): extract helper functions
  All files under 600 lines after split.
  *Modifies:* 5+ oversized headers

- [x] **Step 169: Notification / toast system**
  Replace ad-hoc `outputLog +=` with a structured notification system.
  `NotificationSystem.h`:
  - Notification types: Success, Warning, Error, Info
  - Toast popups in bottom-right corner, auto-dismiss after 4s (configurable)
  - Notification history accessible from status bar icon
  - Click toast to navigate to source (e.g., click error toast → jump to line)
  - Panels use `notify(level, message)` instead of appending to outputLog
  *New:* `NotificationSystem.h`. *Modifies:* panels that use outputLog

- [x] **Step 170: UI event bus for decoupled updates**
  Create `UIEventBus.h` — lightweight pub/sub for UI events:
  - Events: ASTChanged, BufferSwitched, DiagnosticsUpdated, ThemeChanged,
    SettingsChanged, FileModified, AgentConnected, NotificationPosted
  - Panels subscribe to events they care about
  - Replaces polling patterns and tightly-coupled re-render triggers
  - Debounce support for high-frequency events (typing, diagnostics)
  *New:* `UIEventBus.h`. *Modifies:* panels to subscribe instead of poll

---

## Phase 6b: Theme Engine & Visual Design (Steps 171–176)

Full visual customization. Ship with popular themes. Make the editor
look professional and feel personal.

- [x] **Step 171: Theme engine core**
  `ThemeEngine.h` with theme data model:
  - Color categories: editor (bg, fg, cursor, selection, currentLine),
    syntax (keyword, string, comment, number, type, function, operator),
    gutter (bg, lineNumber, marker colors), panels (bg, border, header),
    annotations (per-strategy colors), statusBar, scrollbar, tooltip
  - Load from JSON files in `~/.whetstone/themes/`
  - Bundled themes embedded as defaults
  - Hot-reload on file change
  - `ThemeEngine::getColor(ThemeColor::Keyword)` API for all panels
  *New:* `ThemeEngine.h`

- [x] **Step 172: Bundled theme pack**
  Ship 7 themes, each a JSON file with full color definitions:
  - **Whetstone Dark** (default) — custom dark theme with annotation-aware
    accent colors
  - **Whetstone Light** — clean light theme
  - **Monokai Pro** — warm dark with vivid syntax colors
  - **Solarized Dark** / **Solarized Light**
  - **Dracula**
  - **Nord**
  Each theme tested with all annotation types visible.
  *New:* 7 theme JSON files

- [x] **Step 173: Theme gallery UI**
  Settings > Themes section with visual browser:
  - Grid of theme cards showing name + color swatch preview
  - Live preview on hover (entire editor updates temporarily)
  - "Apply" confirms, "Reset" reverts to previous
  - "Open Themes Folder" button for custom theme creation
  - Active theme highlighted with checkmark
  *Modifies:* Settings panel, `ThemeEngine.h`

- [x] **Step 174: Icon system**
  `IconSet.h` — consistent icons rendered via ImGui drawing primitives:
  - File type icons (Python, C++, JS, Rust, Go, Java, Elisp, JSON, etc.)
  - Panel icons (explorer, outline, terminal, agents, dependencies, etc.)
  - Diagnostic icons (error circle, warning triangle, info, hint lightbulb)
  - Annotation strategy icons (distinct shapes, not just colors)
  - Icons scale with zoom level
  - Theme-aware: icon colors adapt to current theme
  *New:* `IconSet.h`

- [x] **Step 175: Typography and spacing**
  Configurable typography beyond just font size:
  - Code font family selection (from system monospace fonts or bundled)
  - UI font separate from code font
  - Line height multiplier (1.0–2.0)
  - Letter spacing adjustment
  - Consistent panel padding/margins defined in theme
  - Settings UI for all typography options
  *Modifies:* `SettingsManager.h`, `CodeEditorWidget.h`, `ThemeEngine.h`

- [x] **Step 176: Smooth UI transitions**
  Subtle animations that make the editor feel polished:
  - Panel open/close: smooth slide (not instant pop)
  - Tab switch: brief cross-fade on editor content
  - Tooltip: fade in after delay, fade out on leave
  - Notification toasts: slide in from right, fade out on dismiss
  - Search match highlight: brief pulse on "next result" navigation
  - Cursor blink: configurable rate, smooth fade (not hard on/off)
  All animations respect a "Reduce Motion" accessibility setting.
  *New:* `AnimationUtils.h`. *Modifies:* panels using transitions

---

## Phase 6c: Editor UX Enhancements (Steps 177–183)

Core editing improvements that make daily use smooth.

- [x] **Step 177: Enhanced find/replace**
  Upgrade the find/replace bar:
  - Match count display ("3 of 17 results")
  - Live regex preview with captured groups highlighted
  - Replace preview: show inline what the replacement will look like
  - Find-in-selection mode
  - Toggle buttons with clear visual state: Match Case [Aa], Whole Word [W],
    Regex [.*]
  - Ctrl+H opens replace (Ctrl+F opens find-only)
  - Preserve search history (last 10 queries, accessible via dropdown)
  *Modifies:* `CodeEditorWidget.h`, find/replace rendering

- [x] **Step 178: Multi-cursor editing**
  Support for multiple simultaneous cursors:
  - Ctrl+D: select next occurrence of current selection (add cursor)
  - Alt+Click: add cursor at click position
  - Ctrl+Alt+Up/Down: add cursor above/below
  - Alt+Shift+Drag: column/rectangular selection
  - All cursors type, delete, and navigate simultaneously
  - Escape to collapse to single cursor
  - Works with auto-indent and bracket completion
  *Modifies:* `CodeEditorWidget.h`

- [x] **Step 179: Rainbow brackets and delimiter intelligence**
  Smart bracket handling:
  - Rainbow bracket coloring: nested brackets get rotating colors
    (configurable, theme-aware, toggleable)
  - Bracket pair highlight: matching bracket highlighted on cursor adjacency
  - Bracket scope highlight: subtle background tint for current block
  - Auto-surround: select text then type `(` → wraps selection in parens
  - Configurable auto-close per language (e.g., disable for Lisps)
  *Modifies:* `CodeEditorWidget.h`, `EditorMode.h`

- [x] **Step 180: Rich tooltip system**
  Replace basic ImGui tooltips with rich formatted tooltips:
  - Markdown rendering in tooltips (bold, code, lists)
  - Multi-section tooltips (e.g., type info + docs + source link)
  - "Pin" button to keep tooltip open while reading
  - Click-through links in tooltips (navigate to definition)
  - Configurable hover delay (default 500ms)
  - Max width/height with scrolling for long content
  *New:* `RichTooltip.h`. *Modifies:* hover handlers in editor/panels

- [x] **Step 181: Enhanced status bar**
  Context-rich status bar showing everything relevant at a glance:
  - Left: mode indicator (Text/Structured), language selector (clickable),
    encoding (UTF-8, clickable to change), line ending (LF/CRLF, clickable)
  - Center: notification area (last toast message, click for history)
  - Right: Ln/Col, selection count (chars/lines), zoom %, git branch/status
  - Background color subtly adapts to state (normal, error, running)
  - Each segment is clickable for relevant actions
  *Modifies:* `panels/StatusBarPanel.h` (extracted in Step 166)

- [x] **Step 182: Improved diagnostics experience**
  Make errors and warnings easier to understand and act on:
  - Problems panel with sortable columns (Severity, Source, File, Message)
  - Group by file with collapsible sections
  - Quick-fix actions inline ("Apply suggestion" button on fixable diagnostics)
  - "Fix All" button for batch-applicable fixes of same type
  - Peek view: Ctrl+Shift+M shows diagnostic inline without leaving context
  - Diagnostic count badges on file tabs (red dot = errors, yellow = warnings)
  *Modifies:* diagnostic rendering, `panels/BottomPanel.h`

- [x] **Step 183: Tab and panel drag-and-drop**
  Fluid drag interactions for workspace customization:
  - Drag tabs to reorder within the tab bar
  - Drag tab to split view (left/right/top/bottom drop zones)
  - Drag files from explorer to specific tab position
  - Drag panel headers to rearrange docked layout
  - Visual feedback during drag: ghost preview, drop zone highlights
  - Double-click tab → rename (for untitled files)
  *Modifies:* tab rendering, `LayoutManager.h`

---

## Phase 6d: Onboarding & Discoverability (Steps 184–189)

Help users discover what Whetstone can do. Reduce the learning curve
without being annoying.

- [x] **Step 184: First-run experience**
  Interactive setup wizard on first launch (no session.json found):
  1. "Welcome to Whetstone" splash with logo
  2. Choose layout preset (VSCode/Emacs/JetBrains) with visual previews
  3. Choose theme (grid of 4 most popular themes)
  4. Choose keybinding profile with common shortcut preview
  5. "Open Example Project" or "Open Folder" or "New File"
  Settings persisted immediately. Can be re-run from Help > Setup Wizard.
  *New:* `FirstRunWizard.h`

- [x] **Step 185: Contextual feature hints**
  Non-intrusive discovery system:
  - Hints appear as subtle bar below toolbar, not modal dialogs
  - Triggered by context: first time opening annotations panel →
    "Tip: Right-click a function to add memory annotations"
  - First time connecting an agent → "Tip: Agents can query and modify
    your AST via the JSON-RPC API"
  - Max 1 hint per session, max 3 per day
  - "Got it" dismisses, "Don't show tips" disables permanently
  - Hint state tracked in `~/.whetstone/hints.json`
  *New:* `FeatureHints.h`

- [x] **Step 186: Keyboard shortcut reference**
  Dedicated shortcut panel (Ctrl+K Ctrl+S to open):
  - Full list grouped by category (File, Edit, View, Navigate, Annotate,
    Agent, Debug)
  - Search/filter box
  - Shows current keybinding profile's mappings
  - "Record Keys" mode: press any key combo → shows what it does
  - Highlights customized vs default bindings
  - Export to printable HTML/PDF
  *New:* `ShortcutReference.h`

- [x] **Step 187: In-editor help system**
  Help > Documentation opens a dockable panel:
  - Searchable documentation browser
  - Sections: Getting Started, Annotation Reference, Language Support,
    Agent API Quick Reference, Keyboard Shortcuts
  - Annotation Reference: every annotation type with code examples,
    valid strategies, language-specific behavior
  - Content authored in Markdown, rendered in ImGui with basic formatting
    (headings, code blocks, lists, bold/italic)
  - "Open in Browser" button for full online docs
  *New:* `HelpPanel.h`, `MarkdownRenderer.h`

- [x] **Step 188: Enhanced command palette**
  Upgrade the existing command palette:
  - Commands grouped by category with section headers
  - "Recently Used" section pinned at top (last 5 commands)
  - Context-aware: shows different commands based on active panel
  - Inline parameter input: "Go to Line" → type number inline
  - ">" prefix for commands, no prefix for file search (like VSCode)
  - Fuzzy matching highlights matched characters
  *Modifies:* `CommandPalette.h`

- [x] **Step 189: Guided workflow wizards**
  Multi-step wizards for complex operations:
  - **"Annotate File"** wizard: iterates through unannotated functions,
    shows inference suggestions, user confirms/skips each. Summary at end.
  - **"Cross-Language Project"** wizard: select source language → target →
    preview annotation adaptation → confirm → new tab with projected code
  - **"Connect Agent"** wizard: endpoint URL → test connection → set
    permissions → show API capabilities
  - Each wizard shows step progress (Step 2 of 4) and allows Back/Skip
  *New:* `WizardFramework.h`, wizard implementations

---

## Phase 6e: Security & Library UX (Steps 190–195)

Integrate the security vulnerability awareness and semantic annotation
feature requests with polished UX.

- [x] **Step 190: Vulnerability knowledge base**
  `VulnerabilityDatabase.h`:
  - Data model: `VulnerabilityRecord` (ecosystem, package, affected_versions,
    severity, CVE ID, summary, references, fix_versions)
  - Parse OSV (Open Source Vulnerabilities) JSON format
  - Local cache in `~/.whetstone/vuln_cache/` with TTL (default 24h)
  - Offline mode: use cached data when no network
  - Lookup: `query(ecosystem, package, version) → vector<VulnerabilityRecord>`
  - Background refresh on editor startup (non-blocking)
  *New:* `VulnerabilityDatabase.h`

- [x] **Step 191: Dependency security badges**
  Enhance the Dependencies panel with security awareness:
  - Warning badge (shield icon) on packages with known vulnerabilities
  - Badge color by max severity: red (Critical/High), orange (Medium),
    gray (Low)
  - Click badge → expandable advisory details (CVE, severity, summary,
    affected versions, fixed version)
  - "Upgrade to Safe Version" button (writes to dependency file)
  - "Ignore" option with reason (persisted to `.whetstone/vuln_ignore.json`)
  *Modifies:* `DependencyPanel.h`, `VulnerabilityDatabase.h`

- [x] **Step 192: Security diagnostics integration**
  Security findings appear alongside other diagnostics:
  - Problems panel: `[Security]` tagged diagnostics with severity
  - Gutter: shield icon on `import` lines of vulnerable packages
  - Tooltip on gutter icon shows vulnerability summary
  - Agent hints: when `preferImports` is active, deprioritize vulnerable
    package symbols in completion ranking
  - Configurable: "Block vulnerable imports" option (default off, warns only)
  *Modifies:* diagnostic rendering, `PrimitivesRegistry.h`

- [x] **Step 193: Semantic annotation tags for libraries**
  `SemanticTags.h`:
  - Predefined tag vocabulary: `@serialize`, `@crypto`, `@io`, `@network`,
    `@math`, `@collection`, `@concurrency`, `@test`, `@ui`, `@parse`
  - Tag storage: `~/.whetstone/semantic_tags.json` mapping
    `{ library: { symbol: [tags...] } }`
  - Auto-tag common libraries (numpy→@math, requests→@network,
    cryptography→@crypto, json→@serialize, etc.)
  - Tag inference from function names and docstrings (heuristic)
  - Tags attached to `ExternalModule` / `TypeSignature` AST nodes
  *New:* `SemanticTags.h`

- [ ] **Step 194: Semantic-filtered library browser**
  Enhance Library Browser with tag-based discovery:
  - Tag filter bar above symbol list: click tags to filter
  - Active tags shown as chips, click X to remove
  - Symbol list shows tag badges next to each item
  - "Show @crypto functions" → filters across all imported libraries
  - Completion ranking boosted when tag matches user's context
    (editing a serialize function → @serialize symbols rank higher)
  - Agent prompts include semantic tag context for better suggestions
  *Modifies:* `LibraryBrowserPanel.h`, completion system

- [ ] **Step 195: Security & semantic UX tests**
  End-to-end tests:
  1. Add vulnerable package → badge appears → advisory details correct
  2. Security diagnostic appears in Problems panel with correct severity
  3. Semantic tag auto-assignment for numpy → @math
  4. Library browser tag filter reduces visible symbols correctly
  5. Agent completion deprioritizes vulnerable package symbols
  6. "Upgrade to Safe Version" writes correct version to dependency file
  *New:* `step195_test.cpp`

---

## Phase 6f: Accessibility & Performance (Steps 196–201)

Make Whetstone usable by everyone and fast with large files.

- [ ] **Step 196: High contrast and colorblind modes**
  Accessibility-first visual options:
  - "High Contrast" theme (WCAG AAA contrast ratios, bold borders)
  - Colorblind-safe annotation markers: use **shapes** (circle, triangle,
    square, diamond, star) in addition to colors
  - Diagnostic markers use patterns (solid, striped, dotted) not just color
  - Configurable: "Use shapes for annotations" toggle (default on)
  - Test: all information conveyed by color also conveyed by shape/pattern
  *New:* High contrast theme. *Modifies:* annotation/diagnostic rendering

- [ ] **Step 197: Keyboard navigation completeness audit**
  Ensure every feature is keyboard-accessible:
  - All panels navigable with Tab/Shift+Tab
  - Focus ring indicator on active element (visible in all themes)
  - Escape consistently returns focus to editor from any panel
  - All context menu actions have keyboard alternatives
  - F6 cycles between major panels (editor, explorer, bottom, side)
  - All dialog buttons reachable via Tab, Enter to confirm, Escape to cancel
  - Audit checklist: test every panel with mouse unplugged
  *Modifies:* all panels for focus handling

- [ ] **Step 198: Virtual scrolling for large files**
  Performance optimization for files with 10k+ lines:
  - Only render visible lines + small buffer (±50 lines)
  - Virtual scroll: total scroll range = total lines, rendered = viewport
  - Syntax highlighting computed lazily (visible range + prefetch ahead)
  - Line number gutter only draws visible range
  - Smooth scroll maintained at all file sizes
  - Benchmark: 50k line file opens in <500ms, scrolls at 60fps
  *Modifies:* `CodeEditorWidget.h`

- [ ] **Step 199: Large file handling**
  Graceful degradation for very large files:
  - Files >1MB: show size warning, offer "Open in Text Mode"
  - Files >5MB: auto-open in Text Mode (no AST sync, no annotations)
  - Files >10MB: disable syntax highlighting, show "Large File Mode" badge
  - Memory usage indicator in status bar
  - Configurable thresholds in Settings
  - "Reopen in Structured Mode" option if user wants full features
  *Modifies:* buffer opening logic, `panels/StatusBarPanel.h`

- [ ] **Step 200: Startup and panel performance**
  Faster startup and smoother runtime:
  - Lazy-load panels: only parse/render on first show
  - Background grammar loading (don't block editor open for tree-sitter init)
  - Session restore: open tabs immediately with cached text, parse AST
    asynchronously
  - Debounce settings: configurable delays for LSP requests, diagnostics,
    syntax highlighting refresh
  - Frame time budget monitor in debug builds (warn if frame >16ms)
  *Modifies:* initialization code, panel rendering

- [ ] **Step 201: Sprint 6 integration tests**
  Comprehensive tests covering the full Sprint 6 feature set:
  1. Panel extraction: main.cpp compiles and all panels render
  2. Theme switch: all annotation colors, syntax colors, panel colors update
  3. Multi-cursor: 3 cursors typing simultaneously produce correct text
  4. First-run wizard: all settings persisted correctly
  5. Vulnerability badge: shows on import of vulnerable package
  6. Keyboard-only: complete a full workflow without mouse
  7. Large file (50k lines): opens, scrolls, searches within time budget
  8. Notification toast: appears, auto-dismisses, appears in history
  *New:* `step201_test.cpp`

---

## Summary

| Phase | Steps | Description |
|-------|-------|-------------|
| 6a | 166–170 | Structural refactor (panel extraction, state split, notifications, event bus) |
| 6b | 171–176 | Theme engine & visual design (themes, icons, typography, animations) |
| 6c | 177–183 | Editor UX enhancements (find/replace, multi-cursor, brackets, tooltips, diagnostics) |
| 6d | 184–189 | Onboarding & discoverability (first-run, hints, shortcuts, help, wizards) |
| 6e | 190–195 | Security & library UX (vulnerability DB, security badges, semantic tags) |
| 6f | 196–201 | Accessibility & performance (high contrast, keyboard nav, virtual scroll, large files) |

**Total: 36 steps across 6 phases.**

---

## Context for Agents

### What exists after Sprint 5 (prerequisites):
- 65+ header files in editor/src/ with 20 AST headers
- Library-aware constructive coding flow (PrimitivesRegistry, library completion)
- 8+ language grammars with syntax highlighting
- LSP client, WebSocket agent server, cross-language projection
- Dependencies panel, Library Browser, Composition Builder
- Emacs ecosystem integration (packages, functions, keybindings)
- Full test coverage (step-per-test pattern, real assertions)

### Architecture principles (reinforced in Sprint 6):
1. **600-line header limit** — split files that exceed this
2. **1500-line main.cpp limit** — extract panels to `panels/` directory
3. **50-field struct limit** — compose sub-structs for large state
4. **Panel extraction pattern** — `void renderXxx(EditorState&)` free functions
5. **Event-driven updates** — use UIEventBus, not polling
6. **Notification system** — use `notify()`, not `outputLog +=`
7. **Theme-aware rendering** — use `ThemeEngine::getColor()`, not hardcoded colors

### Key new files introduced in Sprint 6:
| File | What it does |
|------|-------------|
| `panels/*.h` | Extracted panel rendering functions |
| `ThemeEngine.h` | Theme loading, color lookup, hot-reload |
| `IconSet.h` | Consistent file/panel/diagnostic icons |
| `NotificationSystem.h` | Toast notifications with history |
| `UIEventBus.h` | Pub/sub for decoupled panel updates |
| `RichTooltip.h` | Markdown-capable hover tooltips |
| `VulnerabilityDatabase.h` | CVE/OSV lookup and caching |
| `SemanticTags.h` | Library symbol semantic annotations |
| `FirstRunWizard.h` | First-launch setup experience |
| `FeatureHints.h` | Contextual tip system |
| `HelpPanel.h` | In-editor documentation browser |
| `MarkdownRenderer.h` | Markdown rendering in ImGui |
| `WizardFramework.h` | Multi-step wizard UI component |
| `ShortcutReference.h` | Keyboard shortcut browser panel |
| `AnimationUtils.h` | Smooth transitions and easing functions |

### Build notes:
- No new external dependencies required for Phase 6a–6d
- Phase 6e (vulnerability DB): may add a JSON HTTP client for OSV API
  (or reuse existing HTTP if available), otherwise offline-first with
  bundled snapshot
- All themes are JSON data files, no code changes needed to add more

---

## Sprint 7 Preview (for planning)

**Sprint 7: MCP Bridge & Synthetic Training Data**

Planned themes (to be fully specified after Sprint 6):
1. Document the JSON-RPC agent API with schemas and examples
2. MCP server wrapper exposing editor operations as MCP tools/resources
3. Synthetic interaction trace generator for fine-tuning tool-use models
4. Agent API evaluation harness (test suites for LLM tool use accuracy)
5. Claude/Codex-specific tool definitions and prompts
6. Training data pipeline: record real sessions → anonymize → export
