# Sprint 19 Plan: GUI Phase 2 — Workflow Visualization

## Context

Sprint 13 gave us docked panels, themes, key symbols, and navigation. Sprint 19
adds the workflow-specific visualization. The architect should see the project as
a **structured flow** — task dependencies, routing decisions, execution progress,
review queues — not just code and files.

This is the GUI that makes the AI+Human thesis tangible. When an architect looks
at the screen, they should immediately understand: what's done, what's in progress,
what's blocked, what needs their attention, and why.

---

## Phase 19a: Task Board Panel (Steps 428-432)

### Step 428: Kanban-Style Task Board (12 tests)
- New docked panel: Task Board
- Columns: Pending | Ready | In Progress | Review | Complete
- Cards show: task name, worker type icon, priority badge, annotation tags
- Drag (or click) to manually reassign between columns
- Filter by: worker type, priority, language, file

### Step 429: Task Detail View (12 tests)
- Click a task card → side panel with full details:
  - Skeleton code (what was specified)
  - Generated code (what the worker produced)
  - Diff view: skeleton vs result
  - Routing decision explanation
  - Rejection history with feedback
  - Annotations on the target node
- Approve/reject buttons for review items

### Step 430: Dependency Graph Visualization (12 tests)
- Visual DAG rendering of task dependencies
- Uses DependencyGraph.h data from Sprint 15
- Nodes colored by status (green=complete, blue=in-progress, grey=pending, red=blocked)
- Critical path highlighted
- Hover shows task details
- Click navigates to task in board and code editor

### Step 431: Progress Dashboard (12 tests)
- Workflow progress ring (% complete)
- Throughput chart (items completed over time)
- Worker breakdown pie chart (deterministic vs SLM vs LLM vs human)
- Cost tracker (tokens used, estimated remaining)
- Blocker summary with action buttons

### Step 432: Phase 19a Integration (8 tests)
- Task board syncs with live workflow state
- Dependency graph updates as tasks complete
- Progress dashboard reflects real orchestration events
- Full visual workflow: create → route → execute → review → complete, all visible

---

## Phase 19b: Code Annotation Overlay (Steps 433-437)

### Step 433: Inline Annotation Badges (12 tests)
- In the code editor, show annotation badges in the gutter (left margin)
- Each annotation type has an icon and color
- Click badge → expand to show full annotation details
- Multiple annotations on same line → stacked badges

### Step 434: Annotation Heatmap (12 tests)
- Color-code the code editor background by annotation density/complexity
- High complexity regions: warm colors (orange/red tint)
- Well-annotated safe regions: cool colors (green/blue tint)
- Toggle on/off from View menu
- Helps architect quickly spot the hard parts of the codebase

### Step 435: Workflow Status Overlay (12 tests)
- In multi-file view, each function/class has a status indicator:
  - Skeleton (outline icon), In Progress (spinner), Complete (check), Review (eye)
- Color matches task board status
- Click status → opens task detail for that work item

### Step 436: Review Comparison View (12 tests)
- Side-by-side or inline diff for review items:
  - Left: skeleton / original code
  - Right: generated code
  - Inline annotations showing what changed and why
- Approve/reject buttons integrated into the diff view
- Keyboard shortcuts: Ctrl+Shift+A (approve), Ctrl+Shift+R (reject)

### Step 437: Phase 19b Integration + Sprint Summary (8 tests)
- Annotation badges visible on all annotated code
- Heatmap correctly highlights complex regions
- Workflow overlay reflects live orchestration state
- Review comparison accessible from task board and code editor
- Sprint 19 totals: all visualization panels operational

---

## Step & Test Summary

| Phase | Steps | Tests | Theme |
|-------|-------|-------|-------|
| 19a | 428-432 | 56 | Task board, detail view, dependency graph, progress dashboard |
| 19b | 433-437 | 56 | Annotation badges, heatmap, workflow overlay, review diff |
| **Total** | **428-437** | **~112** | 10 steps |
