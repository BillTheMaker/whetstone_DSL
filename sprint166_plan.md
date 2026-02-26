# Sprint 166 Plan: `whetstone_generate_code` for Production Class/Module Output

## Context

Current A/B rerun (2026-02-26) confirms:
- `whetstone_generate_code` still returns a single-function `printf` stub for
  class-level specs.
- `whetstone_run_pipeline` now emits class structure, but still uses placeholder
  typing and incomplete bodies for production targets.

This sprint upgrades `whetstone_generate_code` from function-stub behavior to a
deterministic class/module generator suitable for real implementation tasks.

---

## Goals

1. Replace `printf` fallback path for class/module specs
2. Emit complete class/data-model structure with concrete typed fields/methods
3. Support PriorityQueue-class pattern end-to-end in generated C++
4. Return explicit generation quality metadata (not just code text)

---

## Steps

### Step 1874: Spec classifier for function/class/module intent (10 tests)

Add robust intent parsing for `whetstone_generate_code`:
- identify data model vs service class vs utility function
- detect multi-method class requests
- route to class/module builder instead of function stub path

Tests (10): function-only routing, class routing, module routing, mixed intent,
false-positive rejection, deterministic classification, empty-spec handling,
high-entropy prompt handling, fallback reason correctness, API compatibility.

### Step 1875: C++ class/data-model emitter with concrete typing (12 tests)

Implement structured C++ emitter for:
- fields (`std::string`, `int`, `bool`, `double`, vectors/maps where detected)
- constructors and method signatures with concrete types
- includes + pragma once + namespace handling

Tests (12): typed field emission, constructor emission, method signature typing,
include minimization, namespace stability, header parseability, no `auto` in
public API for inferred primitives, deterministic member ordering, no raw stub
fallback, unknown-type diagnostic, compatibility with existing `schema_to_cpp`,
regression on function-only generation.

### Step 1876: PriorityQueue pattern builder (10 tests)

Add first-class support for queue patterns:
- internal heap storage type
- enqueue/dequeue/peek/size/empty bodies
- ordering by priority

Tests (10): method presence, max-heap behavior shape, empty-queue guard paths,
return type correctness, const-correctness for observers, compile success of
generated snippet, deterministic output, no placeholder body markers, no `printf`
fallback, class+data model co-generation.

### Step 1877: Generation quality metadata contract (8 tests)

Extend output payload with machine-checkable fields:
- `quality.compilable_estimate`
- `quality.placeholder_count`
- `quality.todo_count`
- `quality.fallback_used`
- `quality.warnings[]`

Tests (8): fields always present, counts accurate, fallback flag accuracy,
warnings non-empty when uncertain inference used, backward compatibility with
legacy callers, JSON schema update, deterministic values, docs update.

### Step 1878: Sprint 166 Integration Summary (8 tests)

Add `editor/src/Sprint166IntegrationSummary.h`.
Record: steps_completed=5 (1874-1878), generate_code_stub_removed_for_classes=true,
priority_queue_pattern_supported=true, quality_contract_exposed=true, success=true.

Tests (8): constructable summary struct, steps_completed==5, class stub fallback
removed, PriorityQueue generation path works, metadata present, regression checks
pass, no API break in tool schema, success==true.

---

## Architecture Gate

- No external dependencies added
- Keep existing tool name/signature (`whetstone_generate_code`) backward-compatible
- Class/module specs must never silently degrade to generic `printf` function stubs

