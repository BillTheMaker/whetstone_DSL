# Sprint 29 Plan: Workflow Intelligence + Cost Discipline

## Context

Sprint 29 tightens orchestration intelligence around constrained editing:
better routing, better context bundles, and explicit cost-control policies.

Primary goals:
1. Context width minimization without quality regression
2. Routing confidence calibration for constrained mode
3. Cost policy enforcement and optimization feedback

---

## Phase 29a: Routing and Context Optimization (Steps 544-548)

### Step 544: Constrained Routing Ruleset Extension (12 tests)
- Add constrained-mode routing policy variants
- Promote deterministic/template paths when legal-choice confidence is high

### Step 545: Context Bundle Minimizer (12 tests)
- Build minimal sufficient context bundles per taskitem contract
- Remove irrelevant file/project context from narrow operations

### Step 546: Confidence Calibration for Constrained Paths (12 tests)
- Calibrate routing confidence with historical constraint outcomes
- Penalize paths with repeated post-apply failures

### Step 547: Cost Policy Guard (12 tests)
- Enforce per-step and per-workflow token ceilings
- Require rationale/escalation when exceeding policy

### Step 548: Phase 29a Integration (8 tests)
- Lower context budgets with maintained pass rates
- Cost guard blocks runaway orchestration paths

---

## Phase 29b: Optimization Feedback Loops (Steps 549-553)

### Step 549: Suggestion Engine for Cost Reductions (12 tests)
- Suggest template substitution, batching, and context narrowing opportunities

### Step 550: Worker Efficiency Dashboard Data Model (12 tests)
- Aggregate completion/cost/rejection/latency by worker and task class

### Step 551: Review-Gate Policy Refinement (12 tests)
- Reduce unnecessary human escalations while preserving safety/quality

### Step 552: Cost vs Quality Regression Suite (12 tests)
- Assert quality does not regress as optimization policies tighten

### Step 553: Sprint 29 Integration + Summary (8 tests)
- Routing intelligence and cost controls operate as a coherent policy layer

---

## Step & Test Summary

| Phase | Steps | Tests | Theme |
|-------|-------|-------|-------|
| 29a | 544-548 | 56 | Routing/context/cost policy optimization |
| 29b | 549-553 | 56 | Feedback loops and quality-preserving optimization |
| **Total** | **544-553** | **~112** | 10 steps |
