# Sprint 30 Plan: Debugger + Runtime Observability I

## Context

Sprint 30 starts runtime observability features expected in full editors:
breakpoints, stepping, stack frames, locals, watches, and trace surfaces.

Primary goals:
1. Core debugger control plane
2. Runtime state inspection surfaces
3. Integration with existing workflow/task model

---

## Phase 30a: Debug Control Plane (Steps 554-558)

### Step 554: Debug Session Model (12 tests)
- Session lifecycle: start/attach/pause/resume/stop
- Multi-target and per-buffer debug context support

### Step 555: Breakpoint System (12 tests)
- Line breakpoints, conditional breakpoints, hit-count breakpoints
- Enable/disable persistence and reconciliation on file edits

### Step 556: Step Engine (12 tests)
- Step-into, step-over, step-out, continue semantics
- Deterministic state transitions and UI command mapping

### Step 557: Exception + Stack Trace Capture (12 tests)
- Structured exception events and stack traces
- Link stack frames to editor navigation anchors

### Step 558: Phase 30a Integration (8 tests)
- Full breakpoint-to-step-to-exception cycle executes reliably

---

## Phase 30b: Runtime Inspection Surfaces (Steps 559-563)

### Step 559: Call Stack and Frame Inspector (12 tests)
- Frame navigation and variable scope mapping

### Step 560: Locals + Watches Panel Data Model (12 tests)
- Type-aware variable display, watch expression lifecycle

### Step 561: Trace Timeline Model (12 tests)
- Event timeline for pause/step/exception transitions

### Step 562: Workflow-Aware Debug Hooks (12 tests)
- Attach debug sessions to active workflow items and diagnostics

### Step 563: Sprint 30 Integration + Summary (8 tests)
- Debugger baseline reaches practical daily-use utility

---

## Step & Test Summary

| Phase | Steps | Tests | Theme |
|-------|-------|-------|-------|
| 30a | 554-558 | 56 | Debug session, breakpoints, stepping, stack traces |
| 30b | 559-563 | 56 | Runtime inspectors and workflow integration |
| **Total** | **554-563** | **~112** | 10 steps |
