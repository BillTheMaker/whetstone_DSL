# Sprint 26 Plan: GUI/UI/UX Foundations

## Context

Sprint 26 is the first focused UX sprint after core infrastructure maturity.
The target is not feature count. The target is interaction clarity and visual
legibility at the level expected from modern professional editors.

Primary goals:
1. Stabilize docking and panel behavior
2. Introduce a coherent visual system (tokens, elevation, borders, contrast)
3. Establish a distinct visual identity: black/white/stone with electric-blue
   logic links and "blade edge" highlights
4. Introduce compact modifier-edge shortcut notation

---

## Phase 26a: Interaction and Layout Reliability (Steps 509-514)

### Step 509: Docking Reliability Audit + Deterministic Layout Rules (12 tests)
- Reproduce and eliminate panel drift, hidden tabs, and unexpected undock states
- Add explicit dock-state invariants and recovery paths
- Ensure layout reset is deterministic across startup/session restore

### Step 510: Panel Boundaries, Resizers, and Hit-Target Corrections (12 tests)
- Normalize panel borders, resize affordances, and split-bar hit regions
- Resolve ambiguous click zones where content steals resize focus
- Add regression coverage for mouse + keyboard panel navigation

### Step 511: Interaction State Model (12 tests)
- Define shared state model for hover/focus/active/pressed/disabled
- Replace ad hoc per-widget styling with token-driven state application
- Add contrast checks for every state transition

### Step 512: Window Chrome + Hierarchy Pass (12 tests)
- Introduce consistent hierarchy between work surfaces, tool surfaces, and overlays
- Apply elevation system with subtle shadows + edge highlights
- Ensure modal/non-modal layering semantics are unambiguous

### Step 513: Accessibility Baseline Pass (12 tests)
- Keyboard-only traversal for top-level surfaces and core controls
- Focus-ring visibility and consistency under dark theme
- High-contrast checks for text, iconography, and interactive controls

### Step 514: Phase 26a Integration (8 tests)
- Full docking cycle: open/close/move/reset/restart remains stable
- Interaction states and panel hierarchy render consistently across all major panels
- No regressions in editor typing/focus after layout interaction

---

## Phase 26b: Visual Language and Theming (Steps 515-519)

### Step 515: Theme Token System v2 (12 tests)
- Define semantic color, spacing, radius, border, and elevation tokens
- Support theme derivations without hardcoded widget colors
- Encode high-contrast black/white base with gray-stone layers

### Step 516: Signature Visual Identity Pack (12 tests)
- Add gradient treatment for depth in black surfaces
- Add blade-edge accents for active surfaces and controls
- Add electric-blue accents for long-range logic links and workflow leaps

### Step 517: Control Library Restyle (12 tests)
- Restyle buttons, toggles, tabs, menus, dropdowns, checkboxes
- Clear pressed/depressed depth signals (no ambiguous controls)
- Ensure components render and animate consistently across DPI scales

### Step 518: Icon + Typography Contrast Harmonization (12 tests)
- Harmonize icon stroke weight and text contrast on dark surfaces
- Remove low-information gray-on-black regions
- Improve visual rhythm between dense and sparse panels

### Step 519: Modifier-Edge Shortcut Notation (12 tests)
- Introduce compact glyph system with edge markers on mnemonic letters:
  - left edge lit = Ctrl
  - right edge lit = Shift
  - top edge lit = Super/Win
  - bottom edge lit = Alt
- Support combined modifiers in a compact footprint
- Add fallback textual labels for environments that disable glyph rendering

---

## Phase 26c: Final UX Hardening (Steps 520-523)

### Step 520: Completion Overlay UX Hardening (12 tests)
- Eliminate intrusive completion placement over active typing region
- Respect dismissal intent until new semantic trigger appears
- Ensure pointer + keyboard control is predictable

### Step 521: Notification + Status Signaling Refresh (12 tests)
- Distinguish info/warn/error/critical through consistent visual semantics
- Ensure status elements read clearly against dark layers
- Improve temporal behavior (duration, stacking, dismissal)

### Step 522: Cross-Panel Consistency Sweep (12 tests)
- Verify all panels consume shared tokens/state model
- Remove panel-specific style drift
- Enforce spacing/elevation/border consistency checks

### Step 523: Sprint 26 Integration + Summary (8 tests)
- Docking stable, visual language coherent, interaction clarity improved
- Modifier-edge shortcut system functional and space-efficient
- Editor flow no longer blocked by intrusive overlays

---

## Step & Test Summary

| Phase | Steps | Tests | Theme |
|-------|-------|-------|-------|
| 26a | 509-514 | 68 | Docking, layout stability, interaction-state reliability |
| 26b | 515-519 | 60 | Theme system, visual identity, modifier-edge shortcuts |
| 26c | 520-523 | 44 | Overlay/notification hardening and final consistency |
| **Total** | **509-523** | **~172** | 15 steps |
