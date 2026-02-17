# Sprint 32 Plan: Architect Intake + Markdown-to-Taskitem

## Context

Sprint 32 addresses the fuzzy intake side explicitly:
high-context problem descriptions (markdown PRDs/specs) into structured
project plans and taskitems for constrained execution.

Primary goals:
1. Robust markdown intake and requirement extraction
2. Architect-level planning outputs with review controls
3. Reliable taskitem generation with explicit confidence and uncertainty labels

---

## Phase 32a: Markdown Intake Pipeline (Steps 574-578)

### Step 574: Markdown Spec Parser (12 tests)
- Parse sections, goals, constraints, dependencies, acceptance criteria
- Preserve traceability back to source section anchors

### Step 575: Requirement Normalization and Conflict Detection (12 tests)
- Normalize ambiguous language into structured requirement records
- Detect contradictory constraints and flag for architect review

### Step 576: Scope and Milestone Decomposer (12 tests)
- Convert requirements into epics/milestones/workstreams
- Include uncertainty scoring per decomposed item

### Step 577: Architect Review Surface for Intake (12 tests)
- Approve/modify/reject extracted constraints before task generation

### Step 578: Phase 32a Integration (8 tests)
- Markdown spec -> normalized plan with conflict signals and review traceability

---

## Phase 32b: Taskitem Generation for Constrained Execution (Steps 579-583)

### Step 579: Taskitem Generator v2 (12 tests)
- Generate taskitems with legal-op prerequisites and dependency graph hints

### Step 580: Taskitem Confidence + Ambiguity Annotation (12 tests)
- Attach confidence and ambiguity fields that drive routing/escalation

### Step 581: Acceptance-Criteria Binding (12 tests)
- Bind each taskitem to explicit acceptance checks and test skeletons

### Step 582: Intake-to-Queue Simulation Harness (12 tests)
- End-to-end harness from markdown ingestion to queue-ready constrained taskitems

### Step 583: Sprint 32 Integration + Summary (8 tests)
- Intake pipeline produces high-quality architect-reviewed task queues

---

## Step & Test Summary

| Phase | Steps | Tests | Theme |
|-------|-------|-------|-------|
| 32a | 574-578 | 56 | Markdown intake and requirement normalization |
| 32b | 579-583 | 56 | Taskitem generation with constrained execution metadata |
| **Total** | **574-583** | **~112** | 10 steps |
