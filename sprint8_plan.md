# Sprint 8: Refactor & UX Polish — Plan

> **Goal:** Make Whetstone feel like a real editor out of the box. Fix the
> critical EditorState.h architecture violation, make text-first the default
> experience, properly dock panels on first launch, and add visual polish
> that shows off Dear ImGui capabilities.
>
> **Prerequisites:** Sprint 7 complete (234 steps). All agent tooling and
> MCP infrastructure in place.
>
> **Key themes:**
> 1. Refactor EditorState.h (3,107 lines → multiple focused headers)
> 2. Text-first default experience
> 3. Proper DockBuilder layout on first launch
> 4. Branded splash screen
> 5. Visual effects (whetstone grinding sparks)
> 6. Visual shortcut representation language

---

## Phase 8a: Refactor EditorState.h (Steps 235–240)

EditorState.h is 3,107 lines (5.2x over the 600-line limit). Split it
into domain-specific headers following the Sprint 6 panel extraction
pattern.

- [ ] **Step 235: Extract RPC handler to AgentRPCHandler.h**
  Move `processAgentRequest()` and all its JSON-RPC method handlers
  into a new header. This is the single largest block (~500+ lines)
  added across Sprints 5–7.
  - Free function: `json processAgentRequest(EditorState&, const json&)`
  - Includes all method dispatch (getAST, applyMutation, runPipeline, etc.)
  - EditorState keeps a one-liner that delegates to it
  *New:* `editor/src/AgentRPCHandler.h`

- [ ] **Step 236: Extract buffer operations to BufferOps.h**
  Move file/buffer operations out of EditorState:
  - `doOpen`, `doSave`, `doClose`, `createBuffer`, `makeUntitledName`
  - `handleFileChanges`, `refreshBuildSystem`
  - File watcher logic, auto-save logic
  - Recent files management
  *New:* `editor/src/BufferOps.h`

- [ ] **Step 237: Extract editing operations to EditOps.h**
  Move text editing, undo/redo, and navigation:
  - `doUndo`, `doRedo`, `doFindNext`, `doReplace`
  - `navigateToTarget`, `goToLine`
  - Multi-cursor operations
  - Clipboard operations
  *New:* `editor/src/EditOps.h`

- [ ] **Step 238: Extract LSP and diagnostics to LspOps.h**
  Move LSP client management and diagnostic aggregation:
  - `pollLspMessages`, `flushLspDidChange`
  - `publishDiagnostics` handling
  - Whetstone diagnostic merging
  - LSP initialization and shutdown
  *New:* `editor/src/LspOps.h`

- [ ] **Step 239: Extract Emacs integration to EmacsOps.h**
  Move Emacs-specific operations:
  - `handleEmacsKeyChord`, `refreshEmacsModeLine`
  - `updateEmacsFunctionIndex`
  - Emacs buffer sync, package queries
  *New:* `editor/src/EmacsOps.h`

- [ ] **Step 240: Verify EditorState.h under 600 lines + refactor tests**
  After extraction, EditorState.h should be a thin shell:
  - State member variables and sub-state includes
  - `init()`, `active()`, `notify()` convenience methods
  - Delegation calls to extracted headers
  Tests:
  1. EditorState.h is under 600 lines
  2. All extracted headers are under 600 lines
  3. No function exceeds 80 lines
  4. Existing step206_test and step213_test still pass
  5. `file_limits_test` passes with new headers added
  *New:* `step240_test.cpp`

---

## Phase 8b: Text-First Default & Dock Layout (Steps 241–244)

Make the editor feel familiar on first launch — a normal text editor
with optional structured features.

- [x] **Step 241: Text-first default mode**
  Change the default buffer mode from `Structured` to `Text`:
  - `BufferManager::BufferMode` default → `Text` in `BufferInfo` and `openBuffer()`
  - `getBufferMode()` fallback → `Text`
  - First-run wizard: add "Editor Mode" step (Text recommended, Structured for power users)
  - Welcome screen tips updated to mention mode toggle
  - Settings: `defaultBufferMode` preference (text/structured)
  *Modifies:* `BufferManager.h`, `FirstRunWizard.h`, `WelcomeScreen.h`

- [ ] **Step 242: DockBuilder initial layout**
  Apply the LayoutManager preset using ImGui DockBuilder on first launch:
  - On first run (no session), call `ImGui::DockBuilderAddNode` + `DockBuilderSplitNode`
  - VSCode preset: Explorer 20% left, Editor 60% center, Bottom panel 20%
  - Panels docked by name to the correct dock node IDs
  - Only runs once — after that, imgui.ini state takes over
  - Layout reset action in menu: "View → Reset Layout" re-applies DockBuilder
  *Modifies:* `main.cpp`, `LayoutManager.h`

- [ ] **Step 243: Clean first-launch experience**
  Polish the out-of-box experience:
  - Window starts maximized (already done: `SDL_WINDOW_MAXIMIZED`)
  - Welcome tab opens in the editor area (not floating)
  - Explorer panel shows "Open Folder" prompt if no workspace
  - Status bar shows "Text Mode" / "Structured Mode" indicator
  - Bottom panel starts collapsed (user expands when needed)
  *Modifies:* `main.cpp`, `panels/EditorPanel.h`, `panels/StatusBarPanel.h`

- [ ] **Step 244: First-launch UX tests**
  Tests verifying default experience:
  1. New buffers default to Text mode
  2. Text mode hides AST-specific UI (annotation gutter, AST tab)
  3. Structured mode shows full UI
  4. Mode toggle works and persists
  5. LayoutManager preset data matches DockBuilder application
  6. Settings `defaultBufferMode` preference respected
  *New:* `step244_test.cpp`

---

## Phase 8c: Splash Screen & Branding (Steps 245–247)

A branded splash screen and polished startup feel.

- [ ] **Step 245: Startup splash overlay**
  Render a branded splash on launch before editor loads:
  - Centered overlay: "Whetstone" logo text + version
  - Subtle animation (fade in, maybe spark trail — see Phase 8d)
  - Shows for ~1.5 seconds or until first interaction
  - "Loading..." progress text during init
  - Rendered as an ImGui overlay window (fullscreen, no decorations)
  - Configurable: "Show splash on startup" setting
  *Modifies:* `main.cpp`
  *New:* `editor/src/SplashScreen.h`

- [ ] **Step 246: Bottom notification bar**
  A persistent slim bar at the bottom of the splash / welcome screen:
  - Shows tips, version info, or "What's New" on first launch after update
  - Can show keyboard shortcut of the day
  - Dismissable, remembers dismissed state
  - Separate from status bar — this is for onboarding/tips only
  *Modifies:* `panels/StatusBarPanel.h` or new `panels/TipBar.h`

- [ ] **Step 247: Splash and branding tests**
  1. Splash state initializes correctly
  2. Splash dismisses after timeout or interaction
  3. Splash respects "don't show" setting
  4. Tip bar content rotates and dismisses persist
  *New:* `step247_test.cpp`

---

## Phase 8d: Visual Effects — Whetstone Sparks (Steps 248–250)

Show off Dear ImGui's drawing capabilities with a signature visual effect:
grinding sparks flying from a whetstone, rendered as an overlay.

- [ ] **Step 248: Particle system engine**
  A lightweight 2D particle system using ImGui's `ImDrawList`:
  - `Particle` struct: position, velocity, lifetime, color, size
  - `ParticleEmitter`: spawn rate, direction cone, speed range, color gradient
  - `ParticleSystem`: update (gravity, fade, lifetime), render via `GetForegroundDrawList()`
  - Configurable: max particles, gravity, wind
  - Performance-safe: capped at 200 particles, skipped if reduce-motion enabled
  *New:* `editor/src/ParticleSystem.h`

- [ ] **Step 249: Whetstone spark effect**
  Specific spark configuration for the whetstone theme:
  - Sparks fly from a configurable origin (e.g., bottom-left corner or cursor position)
  - Orange/yellow/white color gradient fading to dark
  - Short-lived particles (0.3–0.8s) with gravity pulling down
  - Sparks rendered on the foreground draw list (over all windows)
  - Triggered on: splash screen, save action, build success, or manual toggle
  - "Sparks" toggle in View menu and settings
  *New:* `editor/src/SparkEffect.h`
  *Modifies:* `main.cpp`

- [ ] **Step 250: Particle system tests**
  1. Particle spawn and lifetime decay work correctly
  2. Dead particles are recycled
  3. Reduce-motion setting disables particles
  4. Particle count stays within cap
  5. Spark effect produces particles with correct color range
  *New:* `step250_test.cpp`

---

## Phase 8e: Visual Shortcut Representation (Steps 251–254)

Visual encoding of keyboard shortcuts as modifier-box glyphs. Each key
is a letter/symbol inside a square where the **four edges encode modifiers**:

```
  ┌───┐      ╔───┐      ╔───╗      ╔───╗
  │ K │      ║ K │      ║ K ║      ║ K ║
  └───┘      ╚───┘      ╚───╝      ╚═══╝
  (none)     Ctrl+K    Ctrl+Shift+K  All mods
```

- **Left edge** = Ctrl — bright when active, dim when inactive
- **Bottom edge** = Alt
- **Right edge** = Shift
- **Top edge** = Win/Super/Meta

Active modifier = bright white/accent line. Inactive = low-light gray.
The glyph is overlaid **on any widget** — buttons, icons, tabs, panels.
Every interactive element can show its shortcut visually, enabling
mouse-free discovery. Since ImGui is immediate mode, glyphs update
instantly when shortcuts are reassigned by external tools or IPC.

- [ ] **Step 251: Modifier-box glyph renderer**
  Core rendering engine for the visual shortcut encoding:
  - `ModifierBoxGlyph` struct: key character/label, 4 modifier flags (ctrl, alt, shift, super)
  - `renderModifierBox(ImDrawList*, ImVec2 pos, float size, ModifierBoxGlyph, theme)`:
    draws a square with 4 independently styled edges + centered key label
  - Active edge: bright color (theme accent or white), 2px line
  - Inactive edge: dim color (theme's muted gray), 1px line
  - Key label: monospace font, centered in box
  - Scalable: `size` parameter controls glyph dimensions (respects zoom)
  - Theme-aware: colors from ThemeEngine (active edge, inactive edge, label, background)
  - Chord sequences: multiple boxes with `>` arrow separator
  - Supports special key labels: "⏎" (Enter), "⇥" (Tab), "⎋" (Esc), "⌫" (Backspace),
    "↑↓←→" (arrows), mouse button glyphs, scroll wheel
  *New:* `editor/src/ModifierBoxGlyph.h`

- [ ] **Step 252: Widget shortcut overlay system**
  Overlay modifier-box glyphs on any ImGui widget:
  - `WidgetShortcutOverlay` system: registry mapping widget IDs → shortcut glyphs
  - `registerWidgetShortcut(widgetId, KeyCombo)` → stores glyph data
  - `renderWidgetOverlay(ImDrawList*, ImRect widgetBounds)` → draws glyph
    in corner or center of widget (configurable placement: corner, center, beside)
  - Auto-detect: if a KeybindingManager action maps to a widget, overlay appears
  - Visibility modes: always, on hover, on Alt hold, never
  - Glyph placement: inside widget (bottom-right corner default) or beside label
  - Non-interactive: overlay doesn't consume clicks
  *New:* `editor/src/WidgetShortcutOverlay.h`

- [ ] **Step 253: Integration across all panels + dynamic reassignment API**
  Wire the overlay system into every panel and expose it for external control:
  - Menu bar items: glyph beside each menu action
  - Toolbar buttons: glyph overlay on each button
  - Command palette: glyph column replaces text shortcuts
  - Explorer panel: glyph on "New File", "New Folder" actions
  - Tab bar: glyph on tab switch shortcuts (Ctrl+1..9)
  - Shortcut reference panel: full glyph rendering instead of text
  - **Dynamic reassignment API** (for external tools / window manager):
    - `setWidgetShortcut(widgetId, KeyCombo)` via JSON-RPC
    - `getWidgetShortcuts()` → returns all widget→shortcut mappings
    - `clearWidgetShortcut(widgetId)` → removes overlay
    - These methods added to `processAgentRequest` (or new IPC channel)
  - Settings: "Show shortcut glyphs" toggle, placement preference, visibility mode
  *Modifies:* `panels/MenuBarPanel.h`, `panels/EditorPanel.h`, `panels/BottomPanel.h`,
  `panels/ExplorerPanel.h`, `ShortcutReference.h`, `WelcomeScreen.h`, `EditorState.h`

- [ ] **Step 254: Shortcut glyph tests**
  1. Modifier-box renders correct edges for each modifier combination (16 combos)
  2. Inactive edges are dim, active edges are bright
  3. Key label is centered in the box
  4. Chord sequences render as multiple boxes with separator
  5. Special key labels render correctly (Enter, Tab, arrows, mouse)
  6. Widget overlay appears at correct position relative to widget bounds
  7. Dynamic reassignment via API updates glyph immediately
  8. Visibility modes work (always, hover, alt-hold, never)
  9. Theme change updates glyph colors
  10. Zoom scaling applies to glyph size
  *New:* `step254_test.cpp`

---

## Summary

| Phase | Steps | Description |
|-------|-------|-------------|
| 8a | 235–240 | Refactor EditorState.h (3,107 → <600 lines each) |
| 8b | 241–244 | Text-first default + DockBuilder layout |
| 8c | 245–247 | Splash screen & branding |
| 8d | 248–250 | Whetstone spark particle effects |
| 8e | 251–254 | Modifier-box shortcut glyphs + widget overlay system |

**Total: 20 steps across 5 phases.**

---

## Architecture Notes

- **Extraction pattern:** Same as Sprint 6 panel extraction. Free functions
  taking `EditorState&` as first parameter. EditorState becomes a thin
  coordinator that includes and delegates.
- **600-line limit enforced:** `file_limits_test` must pass after Phase 8a.
- **Particle system:** Uses `ImGui::GetForegroundDrawList()` for overlay
  rendering. No OpenGL calls — pure ImDrawList for portability.
- **Reduce-motion:** All animations and particles respect the existing
  `settings.getReduceMotion()` flag.
- **Modifier-box glyphs:** Rendered with `ImDrawList::AddLine` for edges and
  `AddText` for key labels. Each edge independently styled. The overlay system
  uses `ImGui::GetForegroundDrawList()` to render above widgets without
  interfering with widget interaction. External tools can reassign shortcuts
  via JSON-RPC, and glyphs update on the next frame (immediate mode).
- **No new dependencies:** Everything uses existing ImGui, SDL2, nlohmann-json.

---

## Sprint 9 Preview (for planning)

**Sprint 9: Production Integration & Real-World Testing**
1. End-to-end testing with real LLM agents (Claude, GPT-4, Llama)
2. Multi-file project support (project-wide AST, cross-file references)
3. Git integration (diff view, blame, branch-aware sessions)
4. Performance profiling with real workloads
5. New language support (Phase A: PHP, C#, Kotlin)
6. Production packaging and stable release
