# Sprint 27 Plan: Constrained Constructive Editing I

## Context

Sprint 27 begins the reliability path for token-efficient, low-hallucination
execution once taskitems are queued. Markdown-to-taskitem remains a fuzzy
architect problem; taskitem-to-edit becomes a constrained deterministic problem.

Primary goals:
1. Define legal operation surface for each taskitem
2. Restrict symbol/action space to in-scope options
3. Encode contract checks before and after every edit

---

## Phase 27a: Taskitem Constraint Model (Steps 524-528)

### Step 524: Typed Taskitem Contract Schema (12 tests)
- Expand taskitem schema with strict fields: allowed targets, allowed ops,
  allowed symbols, forbidden symbols, expected diagnostics deltas
- Reject under-specified taskitems for constrained execution path

### Step 525: Legal Operation Graph (12 tests)
- Build operation graph: insert/update/delete/rename/refactor primitives
- Bind each primitive to language-specific legal contexts
- Enforce op availability by AST node kind

### Step 526: Symbol Scope Extractor (12 tests)
- Build in-scope symbol snapshot per taskitem context
- Provide explicit candidate set to execution engines
- Block unresolved symbol references before apply

### Step 527: Constraint Violation Diagnostics (12 tests)
- New diagnostics for out-of-scope symbol usage and forbidden ops
- Structured machine-readable error payloads for retry/escalation

### Step 528: Phase 27a Integration (8 tests)
- Taskitem with complete constraints routes and executes
- Under-constrained taskitem gets rejected with actionable diagnostics

---

## Phase 27b: Deterministic Apply Guardrails (Steps 529-533)

### Step 529: Pre-Apply Validation Gate (12 tests)
- Validate candidate edit against legal op graph + symbol snapshot
- Reject illegal edits before touching source buffer

### Step 530: Post-Apply Structural Gate (12 tests)
- Re-parse touched region and validate AST integrity
- Validate no unexpected symbol graph drift

### Step 531: Contract Delta Checker (12 tests)
- Compare before/after contract set per taskitem expectations
- Block edits that violate required post-conditions

### Step 532: Retry/Escalation Protocol for Constraint Failures (12 tests)
- Deterministic retry for correctable failures
- Escalate to higher-context worker/human only with full failure packet

### Step 533: Sprint 27 Integration + Summary (8 tests)
- Constrained pipeline prevents out-of-scope edits by construction
- Hallucinated symbol calls fail fast with explicit diagnostics

---

## Step & Test Summary

| Phase | Steps | Tests | Theme |
|-------|-------|-------|-------|
| 27a | 524-528 | 56 | Contract schema, legal ops, scope control |
| 27b | 529-533 | 56 | Guardrails, structural validation, retry/escalation |
| **Total** | **524-533** | **~112** | 10 steps |
