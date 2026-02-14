# Sprint 13 Plan: GUI Overhaul Phase 1

## Context

The human-in-the-loop thesis is load-bearing. If humans are supposed to be architects
and reviewers — not button-pushers — then the interface must make the system intuitive
to understand. The current GUI has floating windows, no key combination symbols, and
doesn't look like a professional product. This sprint fixes that.

Sprint 12 delivered the workflow model and routing engine. The GUI needs to surface
that workflow state — but the heavy workflow visualization comes in Sprint 19. This
sprint focuses on the structural foundation: docking, styling, navigation, and key
symbols so the editor feels like a real tool.

**Current GUI state:**
- ImGui-based with SDL backend
- Text editor with syntax highlighting
- AST tree view panel
- Diagnostics panel
- Annotation panel
- Floating windows (not docked)
- No keyboard shortcut display or customization
- Default ImGui color scheme

**Sprint 13 Deliverables:**
- All windows docked in a professional layout
- Keyboard shortcut system with displayable key combination symbols
- Professional dark color scheme with consistent visual language
- Clean navigation: panel switching, breadcrumbs, command palette
- Status bar with project/workflow/language info
- Resizable, saveable layout that persists across sessions

---

## Phase 13a: Docking Layout (Steps 342-346)

*Convert every floating window to a docked panel in a professional layout.*

### Step 342: ImGui Docking Infrastructure (12 tests)
**Goal:** Set up the ImGui docking system and define the default layout.

**Modifies: `editor/src/EditorState.h`** (or new `editor/src/DockingLayout.h`)
- Enable ImGui docking via `io.ConfigFlags |= ImGuiConfigFlags_DockingEnable`
- Define DockSpace as the root of the editor window
- Default layout:
  ```
  ┌──────────────┬──────────────────────────────┬──────────────┐
  │  File Tree   │        Code Editor           │  AST View    │
  │              │                               │              │
  │              │                               │              │
  │              ├──────────────────────────────┤              │
  │              │  Diagnostics / Output         │              │
  └──────────────┴──────────────────────────────┴──────────────┘
  ```
- Left dock: file tree / project navigator (20% width)
- Center: code editor (60% width, split top/bottom)
- Right dock: AST view / annotations (20% width)
- Bottom: diagnostics, output, workflow status (25% height)

**Tests:** docking enabled, DockSpace renders, default layout has 4 regions,
panels render in correct regions, no floating windows, window positions persist
after frame, layout ID stable across frames, DockSpace fills viewport.

### Step 343: Panel Registration System (12 tests)
**Goal:** Every panel registers itself with a panel manager that controls
docking position, visibility, and ordering.

**New file: `editor/src/PanelManager.h`** (~300 lines)
- `PanelInfo` struct: id, title, defaultDock (left/center/right/bottom),
  isVisible, order, icon
- `PanelManager` class:
  - `registerPanel(id, title, dock, renderFn)` — register a panel
  - `setVisible(id, bool)` — show/hide
  - `isVisible(id) -> bool`
  - `renderAll()` — render all visible panels in docked positions
  - `getPanelList() -> vector<PanelInfo>` — for UI menus
  - `togglePanel(id)` — flip visibility
  - `focusPanel(id)` — bring panel to front in its dock

**Registered panels:**
- `file-tree` — project file browser (left)
- `code-editor` — main text editor (center-top)
- `ast-view` — AST tree visualization (right)
- `annotations` — annotation list/editor (right, tabbed with AST)
- `diagnostics` — error/warning list (bottom)
- `output` — generated code output (bottom, tabbed with diagnostics)
- `workflow` — workflow status summary (bottom, tabbed)
- `properties` — selected node properties (right, tabbed)

**Tests:** register 8 panels, all render without error, visibility toggle,
focus panel brings to front, default dock positions, panel ordering,
hidden panel doesn't render, getPanelList returns all registered.

### Step 344: File Tree Panel (12 tests)
**Goal:** Replace any floating file browser with a docked file tree that
shows the workspace structure with icons and expand/collapse.

**New file: `editor/src/panels/FileTreePanel.h`** (~250 lines)
- Tree view of workspace files using FileTree.h data
- Expand/collapse directories
- File type icons (or text indicators): source files, headers, configs
- Click to open file in editor (calls openFile RPC internally)
- Highlight active file
- Respect .gitignore (already in FileTree.h)
- Right-click context menu: new file, rename, delete (future)

**Tests:** renders workspace tree, directories expandable, click opens file,
active file highlighted, .gitignore respected, empty workspace shows message,
nested directories render correctly, file count matches workspace index.

### Step 345: Tabbed Bottom Panel (12 tests)
**Goal:** Diagnostics, output, and workflow share the bottom dock as tabs.

**Modifies: existing diagnostic/output rendering**
- Tab bar: Diagnostics | Output | Workflow | Console
- Diagnostics tab: structured diagnostic list with severity icons,
  click-to-navigate (jumps to error line in editor)
- Output tab: generated code with syntax highlighting
- Workflow tab: summary stats (placeholder for Sprint 19 deep visualization)
- Console tab: MCP tool log / agent interaction log
- Tab badge: unread count (e.g., "Diagnostics (3)" when 3 new errors)

**Tests:** 4 tabs render, tab switching works, diagnostics click navigates
to line, output shows generated code, workflow shows stats placeholder,
badge count updates on new diagnostic, default tab is diagnostics, tabs
persist across frames.

### Step 346: Phase 13a Integration — Layout Persistence (8 tests)
**Goal:** Save and restore dock layout across sessions.

**Modifies: `editor/src/DockingLayout.h`** (or PanelManager)
- `saveLayout(path)` — serialize ImGui dock state + panel visibility to JSON
- `loadLayout(path)` — restore from JSON
- Auto-save on exit, auto-load on startup
- Default layout if no saved state exists
- Path: `.whetstone/layout.json` in workspace

**Tests:**
1. Default layout applied on first run
2. Resize panel → save → reload → same size
3. Hide panel → save → reload → still hidden
4. Move panel to different dock → save → reload → same dock
5. Tab order preserved
6. Multiple workspaces have independent layouts
7. Corrupt layout file → falls back to default
8. Layout file location correct (.whetstone/layout.json)

---

## Phase 13b: Color Scheme + Visual Polish (Steps 347-350)

*Professional dark theme with consistent visual hierarchy.*

### Step 347: Theme System (12 tests)
**Goal:** Centralized theme definition that every panel reads from.

**New file: `editor/src/Theme.h`** (~300 lines)
- `WhetstoneTheme` struct with named semantic colors:
  - `bg`, `bgAlt`, `bgPanel`, `bgPopup`
  - `text`, `textDim`, `textAccent`, `textError`, `textWarning`, `textSuccess`
  - `border`, `borderFocused`
  - `accent`, `accentHover`, `accentActive` — primary brand color
  - `selectionBg`, `selectionText`
  - `scrollbar`, `scrollbarHover`
  - `diagnostic_error`, `diagnostic_warning`, `diagnostic_info`, `diagnostic_hint`
  - `syntax_keyword`, `syntax_string`, `syntax_number`, `syntax_comment`,
    `syntax_function`, `syntax_type`, `syntax_operator`, `syntax_annotation`
- `applyTheme(theme)` — sets all ImGui style colors from theme
- `getDefaultDarkTheme() -> WhetstoneTheme` — professional dark theme
- `getDefaultLightTheme() -> WhetstoneTheme` — clean light alternative
- Font configuration: primary (monospace), UI (sans-serif), sizes

**Color palette (dark theme):**
- Background: deep blue-grey (#1a1d23, #22252b, #2a2d35)
- Text: warm white (#e0e0e0), dim (#808890), accent (#7aa2f7)
- Borders: subtle (#3a3d45), focused (#7aa2f7)
- Errors: soft red (#f7768e), warnings: amber (#e0af68),
  success: green (#9ece6a), info: blue (#7dcfff)
- Syntax: keywords blue (#7aa2f7), strings green (#9ece6a),
  numbers orange (#ff9e64), comments grey (#565f89)

**Tests:** dark theme applies without crash, all semantic colors defined,
no color is pure black or pure white (contrast), light theme alternative,
applyTheme sets ImGui colors, font sizes reasonable, theme roundtrip to JSON,
syntax colors distinct from each other, diagnostic colors match severity,
accent color used for focused elements.

### Step 348: Syntax Highlighting Refresh (12 tests)
**Goal:** Update the text editor's syntax highlighting to use theme colors
and cover all 10 supported languages.

**Modifies: syntax highlighting in text editor component**
- Use theme's syntax colors instead of hardcoded values
- Language-aware highlighting (keywords differ per language)
- Semanno comment highlighting: `@semanno:type(key=value)` gets distinct
  annotation color
- Line numbers in `textDim` color
- Current line highlight with subtle `bgAlt` background
- Selection uses `selectionBg` / `selectionText`
- Matching bracket highlight

**Tests:** Python keywords highlighted, C++ keywords highlighted, Kotlin
keywords highlighted, Semanno comments get annotation color, strings in
string color, numbers in number color, comments in comment color, current
line highlighted, line numbers dimmed, selection visible, theme change
updates highlighting, bracket matching visible.

### Step 349: Icon System + Visual Indicators (12 tests)
**Goal:** Consistent iconography across all panels using Unicode symbols
(no external icon font dependency — keep it header-only).

**New file: `editor/src/Icons.h`** (~100 lines)
- Unicode symbol constants for common items:
  - File types: source, header, config, folder, image
  - Diagnostics: error circle, warning triangle, info, hint
  - Workflow: pending clock, in-progress spinner, complete check,
    rejected cross, review eye
  - AST: function, class, variable, annotation, module
  - Navigation: arrow right, arrow down, search, settings
- `renderIcon(iconType, color)` — render icon with theme color
- Fallback: if Unicode doesn't render, use text abbreviations (ERR, WARN, etc.)

**Tests:** all icon constants defined, renderIcon doesn't crash, error icon
is distinct from warning icon, file type icons cover 10 languages, workflow
status icons cover all states, fallback text works, icon + text alignment
correct, icon colors respect theme.

### Step 350: Phase 13b Integration — Visual Consistency (8 tests)
**Goal:** Every panel uses the theme and icons consistently.

**Tests:**
1. All panels use theme background colors (no default ImGui grey)
2. Diagnostic panel uses severity icons + colors from theme
3. AST panel uses node type icons
4. File tree uses folder/file icons
5. Status text uses textDim for secondary info
6. Focused panel border uses accent color
7. Theme switch (dark→light) updates all panels
8. No visual artifacts: no overlapping text, no clipped content, no invisible text

---

## Phase 13c: Keyboard System + Key Symbols (Steps 351-355)

*The key combination symbol system the user has been asking for.*

### Step 351: Keybinding Registry (12 tests)
**Goal:** Centralized keybinding system where every action has a named binding
that can be displayed, customized, and persisted.

**New file: `editor/src/KeybindingRegistry.h`** (~350 lines)

**KeyCombo struct:**
- `modifiers: uint8_t` — bitmask: Ctrl, Alt, Shift, Super/Meta
- `key: int` — ImGui key code
- `toString() -> string` — "Ctrl+Shift+P", "Alt+F4", etc.
- `toSymbols() -> string` — "⌃⇧P", "⌥F4" (platform-aware symbols)
  - macOS: ⌘ ⌥ ⌃ ⇧
  - Linux/Windows: Ctrl Alt Shift Super
- `matches(ImGuiIO&) -> bool` — check if currently pressed

**KeyAction enum** (or string-based for extensibility):
- `OpenFile`, `SaveBuffer`, `SaveAll`, `CloseBuffer`
- `TogglePanel_FileTree`, `TogglePanel_AST`, `TogglePanel_Diagnostics`
- `FocusEditor`, `FocusCommandPalette`
- `NextDiagnostic`, `PrevDiagnostic`
- `RunPipeline`, `GenerateCode`
- `Undo`, `Redo`
- `FindInFile`, `FindInProject`
- `ToggleAnnotations`, `InferAnnotations`
- `CreateWorkflow`, `RouteAll`

**KeybindingRegistry class:**
- `bind(action, combo)` — set binding for action
- `getBinding(action) -> KeyCombo` — get current binding
- `getSymbols(action) -> string` — get display symbols for an action
- `processInput(ImGuiIO&) -> optional<KeyAction>` — check all bindings
- `getAll() -> map<KeyAction, KeyCombo>` — for settings UI
- `loadDefaults()` — standard keybindings
- `saveToJson(path)` / `loadFromJson(path)` — persistence

**Default bindings:**
- Ctrl+S: SaveBuffer, Ctrl+Shift+S: SaveAll
- Ctrl+P: CommandPalette
- Ctrl+B: TogglePanel_FileTree
- Ctrl+Shift+D: TogglePanel_Diagnostics
- F5: RunPipeline
- Ctrl+Z: Undo, Ctrl+Shift+Z: Redo
- Ctrl+F: FindInFile, Ctrl+Shift+F: FindInProject
- F8: NextDiagnostic, Shift+F8: PrevDiagnostic

**Tests:** bind and retrieve, default bindings loaded, key combo toString,
key combo toSymbols (platform-aware), processInput detects active combo,
save/load roundtrip, rebind action, conflict detection (same combo for
two actions), getSymbols returns displayable string, modifier bitmask
correct, matches detects key press.

### Step 352: Key Symbol Rendering (12 tests)
**Goal:** Display key combination symbols inline in menus, tooltips, and
the command palette — like every professional editor does.

**New file: `editor/src/KeySymbolRenderer.h`** (~150 lines)
- `renderKeySymbol(combo, theme)` — renders a styled key badge inline
  - Background: slightly lighter than panel bg
  - Border: subtle rounded rect
  - Text: key symbol in small monospace font
  - Example: `[⌃S]` rendered as a neat badge
- `renderActionWithKey(label, action, registry, theme)` — renders
  "Save Buffer    ⌃S" with right-aligned key symbols (menu item style)
- `renderKeyChord(combos, theme)` — for multi-key sequences
  ("⌃K ⌃S" style chords)

**Tests:** single key renders, modifier+key renders, platform symbols correct,
menu item layout (label left, key right), chord rendering, key badge has
background, theme colors used, empty binding shows nothing, special keys
(Enter, Escape, Tab, F-keys) render correctly, long label doesn't overlap
key badge.

### Step 353: Command Palette (12 tests)
**Goal:** Ctrl+P opens a fuzzy-search command palette (VS Code style) that
lists all available actions with their key symbols.

**New file: `editor/src/CommandPalette.h`** (~300 lines)
- Modal popup centered in editor
- Text input at top with fuzzy search
- List of matching actions below, each showing:
  - Icon (from Icons.h)
  - Action name ("Save Buffer", "Toggle AST Panel", "Run Pipeline")
  - Key binding symbol (right-aligned)
- Enter executes selected action
- Escape closes
- Arrow keys navigate list
- Search matches action name, panel name, and command aliases
- Recently used actions appear first

**Action sources:**
- All KeyAction entries from KeybindingRegistry
- Panel toggles from PanelManager
- RPC methods (RunPipeline, GenerateCode, InferAnnotations, etc.)
- File operations (Open, Save, Close)

**Tests:** palette opens on Ctrl+P, fuzzy search filters actions, Enter
executes action, Escape closes, key symbols displayed, arrow navigation,
recent actions first, empty search shows all, partial match works,
palette closes after execution, multiple action sources merged, no
duplicate entries.

### Step 354: Menu Bar with Key Symbols (12 tests)
**Goal:** Top menu bar (File, Edit, View, Tools, Workflow) with key
symbols on every menu item.

**Modifies: main editor rendering**
- **File:** New | Open | Save (⌃S) | Save All (⌃⇧S) | Close
- **Edit:** Undo (⌃Z) | Redo (⌃⇧Z) | Find (⌃F) | Find in Project (⌃⇧F)
- **View:** Toggle File Tree (⌃B) | Toggle AST | Toggle Diagnostics (⌃⇧D) |
  Toggle Annotations | Reset Layout
- **Tools:** Run Pipeline (F5) | Generate Code | Infer Annotations |
  Validate | Export Semanno
- **Workflow:** Create Workflow | Route All | View Ready Tasks | Save Workflow
- **Help:** About | Keyboard Shortcuts

Every menu item uses `renderActionWithKey()` for consistent key symbol display.

**Tests:** all 5 menus render, File menu has Save with ⌃S, Edit menu has
Undo with ⌃Z, View menu toggles panels, Tools menu runs pipeline, menu
items execute correct actions, key symbols visible, menu closes after
selection, separator lines between groups, workflow menu items present,
submenu rendering.

### Step 355: Phase 13c Integration — Keyboard Shortcuts Help Panel (8 tests)
**Goal:** A dedicated keyboard shortcuts panel and verification that the
entire keybinding system works end-to-end.

**New panel: Keyboard Shortcuts** (accessible from Help menu or Ctrl+?)
- Grouped by category: File, Edit, View, Tools, Workflow, Navigation
- Each row: action name | key symbol | description
- Search/filter bar
- "Customize" button (opens keybinding editor — basic version)

**Tests:**
1. All default bindings displayed in shortcuts panel
2. Ctrl+S triggers save action
3. Ctrl+P opens command palette
4. Command palette shows key symbols for all actions
5. Menu items show correct key symbols
6. Rebind action → shortcut panel updates
7. Key symbols render correctly on current platform
8. Full keyboard-driven workflow: Ctrl+P → type "pipeline" → Enter → pipeline runs

---

## Phase 13d: Navigation + Status Bar (Steps 356-360)

*Clean navigation: breadcrumbs, status bar, and go-to-definition.*

### Step 356: Breadcrumb Navigation (12 tests)
**Goal:** Breadcrumb bar above the code editor showing the current location
in the AST hierarchy: `Module > Class > Method > Function`.

**New file: `editor/src/panels/BreadcrumbBar.h`** (~150 lines)
- Displays path from module root to cursor position's AST node
- Each breadcrumb is clickable → navigates to that node
- Dropdown on each segment → shows siblings (other methods in class, etc.)
- Updates as cursor moves
- Uses compact AST names from CompactAST::getNodeName()

**Tests:** breadcrumb shows module name, nested path updates with cursor,
click navigates to node, dropdown shows siblings, empty file shows module only,
deep nesting (4+ levels) renders without overflow, breadcrumb uses theme
colors, annotation nodes appear in path.

### Step 357: Status Bar (12 tests)
**Goal:** Bottom status bar showing project state at a glance.

**New file: `editor/src/panels/StatusBar.h`** (~200 lines)
- Left section: language indicator + file path
- Center section: cursor position (Ln 42, Col 8) | encoding | line ending
- Right section: workflow status (if active), MCP connection indicator,
  diagnostic summary (3 errors, 2 warnings)

**Status bar segments:**
- Language badge: colored badge with language name ("Python", "C++", "Kotlin")
- Workflow badge: phase indicator if workflow active
  ("Executing 3/10" with progress ring)
- Diagnostic summary: clickable → jumps to diagnostics panel
- MCP indicator: green dot if MCP connected, grey if headless-only

**Tests:** language badge shows current buffer language, cursor position
updates, diagnostic count accurate, workflow badge shows when active,
click diagnostic count focuses diagnostics panel, status bar uses theme
colors, language changes when switching buffers, encoding display,
all sections fit without overflow.

### Step 358: Go-to-Definition + Go-to-Symbol (12 tests)
**Goal:** Navigate by AST structure, not just text search.

**Modifies: `editor/src/panels/` or code editor component**
- **Go-to-Definition (F12):** Click on a function call → jump to its definition
  (uses cross-file symbol resolution from Sprint 9d)
- **Go-to-Symbol (Ctrl+Shift+O):** Open a symbol picker showing all
  functions/classes/variables in current file with icons
- **Go-to-Symbol in Project (Ctrl+T):** Same but across all open buffers
- Uses existing `searchProject` and `getInScopeSymbols` RPC infrastructure

**Tests:** F12 on function call jumps to definition, Ctrl+Shift+O lists file
symbols, Ctrl+T lists project symbols, symbol list shows icons by type,
filter works on symbol list, cross-file jump opens target buffer, no match
shows message, symbols sorted by line number.

### Step 359: Find and Replace Improvements (12 tests)
**Goal:** In-file and project-wide find/replace with result highlighting.

**Modifies: text editor component**
- Ctrl+F: find bar appears at top of editor (not floating window)
- Result highlighting in editor (all matches highlighted, current match
  distinct color)
- Match count display ("3 of 17")
- Enter: next match, Shift+Enter: previous match
- Ctrl+H: replace mode (adds replace input field)
- Ctrl+Shift+F: project-wide find (results in bottom panel)
- Search uses `searchProject` for AST-aware results when possible

**Tests:** Ctrl+F opens find bar, typing filters matches, match highlighting
visible, match count accurate, Enter cycles through matches, Ctrl+H shows
replace field, replace works, project-wide search shows results in bottom
panel, Escape closes find bar, empty search clears highlights.

### Step 360: Phase 13d Integration + Sprint 13 Summary (8 tests)
**Goal:** Full navigation workflow and sprint verification.

**Tests:**
1. Full keyboard workflow: open file → navigate to function → go-to-definition → breadcrumb click back
2. Command palette → "find" → project-wide search → navigate to result
3. Status bar reflects buffer switches (language, cursor, diagnostics)
4. All panels docked, no floating windows
5. Theme consistent across all panels and navigation elements
6. Key symbols visible in menus, palette, and shortcuts panel
7. Layout save → restart → same layout restored
8. Sprint 13 totals: all panels docked, theme applied, keybindings functional, navigation operational

---

## Step & Test Summary

| Phase | Steps | Tests | Theme |
|-------|-------|-------|-------|
| 13a | 342-346 | 56 | Docking layout, panel system, file tree, tabbed bottom |
| 13b | 347-350 | 44 | Theme system, syntax highlighting, icons, visual consistency |
| 13c | 351-355 | 56 | Keybinding registry, key symbols, command palette, menus |
| 13d | 356-360 | 56 | Breadcrumbs, status bar, go-to-definition, find/replace |
| **Total** | **342-360** | **~212** | 19 steps |

---

## Key Files

**New files:**
- `editor/src/DockingLayout.h` — ImGui docking setup, default layout, persistence
- `editor/src/PanelManager.h` — panel registration, visibility, rendering
- `editor/src/Theme.h` — semantic color themes, dark/light, font configuration
- `editor/src/Icons.h` — Unicode icon constants, renderIcon
- `editor/src/KeybindingRegistry.h` — keybindings, key combos, persistence
- `editor/src/KeySymbolRenderer.h` — key badge rendering, menu item key display
- `editor/src/CommandPalette.h` — fuzzy-search command palette
- `editor/src/panels/FileTreePanel.h` — docked file tree browser
- `editor/src/panels/BreadcrumbBar.h` — AST breadcrumb navigation
- `editor/src/panels/StatusBar.h` — status bar with language/workflow/diagnostic info

**Modified files:**
- `editor/src/EditorState.h` — docking enable, layout integration
- `editor/src/main.cpp` — theme application, panel manager initialization
- All existing panel renderers — migrate to PanelManager registration
- `editor/CMakeLists.txt` — test targets for steps 342-360

---

## Design Principles

1. **No floating windows.** Every panel docks. Period.
2. **Key symbols everywhere.** If an action has a keybinding, its symbol is visible
   wherever that action appears — menus, palette, tooltips, shortcuts panel.
3. **Theme is semantic.** Colors are named by purpose (textError, syntax_keyword)
   not by value. Changing the theme changes everything.
4. **Keyboard-first.** Every action reachable by keyboard. Mouse is optional.
5. **Professional defaults.** First-run experience should look polished, not like
   a developer's test harness.
