# Sprint 31 Plan: Debugger + Runtime Observability II

## Context

Sprint 31 extends debugger capabilities with memory reflection,
advanced runtime views, and performance-aware introspection.

Primary goals:
1. Memory reflection as first-class editor capability
2. Advanced runtime introspection
3. Better debugging ergonomics for complex systems

---

## Phase 31a: Memory Reflection Core (Steps 564-568)

### Step 564: Memory Snapshot Model (12 tests)
- Structured snapshot schema for stack, heap regions, and references

### Step 565: Memory Inspector UI Model (12 tests)
- Typed renderers for primitives, pointers/references, containers, structs

### Step 566: Allocation/Ownership Trace Hooks (12 tests)
- Allocation timeline and ownership transition markers

### Step 567: Leak/Corruption Signal Bridge (12 tests)
- Integrate leak/corruption signals into diagnostics + debug panes

### Step 568: Phase 31a Integration (8 tests)
- Memory reflection workflow: snapshot -> inspect -> correlate with stack/debug state

---

## Phase 31b: Advanced Debug Views (Steps 569-573)

### Step 569: Disassembly + Register View Baseline (12 tests)
- Optional architecture-aware low-level inspection surface

### Step 570: Watch Expression Evaluator Hardening (12 tests)
- Safe evaluation boundaries and timeout protections

### Step 571: Time-Travel Debug Event Buffer (12 tests)
- Bounded historical state replay for recent execution steps

### Step 572: Performance Probe Overlay (12 tests)
- Inline timing/hotspot metadata tied to debug pauses

### Step 573: Sprint 31 Integration + Summary (8 tests)
- Memory reflection + advanced views usable without destabilizing core editor flow

---

## Step & Test Summary

| Phase | Steps | Tests | Theme |
|-------|-------|-------|-------|
| 31a | 564-568 | 56 | Memory reflection core |
| 31b | 569-573 | 56 | Advanced runtime debug views |
| **Total** | **564-573** | **~112** | 10 steps |
