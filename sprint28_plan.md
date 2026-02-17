# Sprint 28 Plan: Constrained Constructive Editing II

## Context

Sprint 28 hardens constrained execution from schema-level guardrails into
practical multi-language constructive editing with legal-choice UIs and
agent-facing operation selectors.

Primary goals:
1. Legal-choice edit UX for agents/humans
2. Multi-language constructive edit adapters
3. Token-budget reductions through constrained option sets

---

## Phase 28a: Legal-Choice Execution Surfaces (Steps 534-538)

### Step 534: Operation Selector API (12 tests)
- Expose only legal operations for active taskitem and AST focus node
- Rank suggestions by expected constraint success

### Step 535: Symbol Selector API (12 tests)
- Expose only in-scope legal symbols with typed role metadata
- Add category filters (function/type/field/module/constant)

### Step 536: Argument Shape Validator (12 tests)
- Validate operation arguments against selected op+symbol contract
- Prevent malformed or semantically invalid argument combinations

### Step 537: Constrained Execution Telemetry (12 tests)
- Log constraint candidate count, selected op path, rejection reasons
- Track token usage deltas vs unconstrained baseline path

### Step 538: Phase 28a Integration (8 tests)
- Agent receives legal-choice menus and executes without out-of-scope references
- Telemetry shows reduced candidate breadth and stable success rate

---

## Phase 28b: Multi-Language Adapters + Quality Gates (Steps 539-543)

### Step 539: C/C++ Constructive Edit Adapter (12 tests)
- Legal op mappings for declarations/definitions/includes/macros
- Header/source boundary-safe operations

### Step 540: Python/TypeScript Constructive Edit Adapter (12 tests)
- Legal op mappings for functions/classes/imports/typing signatures
- Module-level side-effect protections

### Step 541: Rust/Go Constructive Edit Adapter (12 tests)
- Legal op mappings for ownership/concurrency-sensitive constructs
- Borrow/lifetime-unsafe op guardrails

### Step 542: Cross-Language Consistency Gate (12 tests)
- Standardized constraint failures and diagnostics across adapters
- Common fallback behavior on unsupported constructs

### Step 543: Sprint 28 Integration + Summary (8 tests)
- Multi-language constrained constructive editing stable across adapters
- Out-of-scope/hallucinated edits remain blocked across languages

---

## Step & Test Summary

| Phase | Steps | Tests | Theme |
|-------|-------|-------|-------|
| 28a | 534-538 | 56 | Legal-choice APIs, validation, telemetry |
| 28b | 539-543 | 56 | Multi-language adapters and consistency gates |
| **Total** | **534-543** | **~112** | 10 steps |
