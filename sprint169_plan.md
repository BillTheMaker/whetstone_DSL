# Sprint 169 Plan: Production-Grade `whetstone_run_pipeline` Output

## Context

After Sprint 166-168:
- `whetstone_generate_code` can produce production-shaped class/module code for the
  benchmark task.
- `whetstone_run_pipeline` still emits placeholder-style signatures for some
  cross-language class outputs (e.g. `auto self`).

This sprint upgrades `run_pipeline` output quality so cross-language generation
can reach production gates, not just structural completeness.

---

## Goals

1. Eliminate placeholder method signatures in `run_pipeline` output
2. Emit concrete data-model fields for parsed classes (e.g., WorkItem fields)
3. Add language-aware return/parameter typing policy for pipeline generation
4. Make PriorityQueue Python→C++ path pass production gates

---

## Steps

### Step 1889: Typed method signature normalization in pipeline projections (10 tests)

Normalize class method signatures post-projection:
- remove synthetic receiver placeholders (`auto self`)
- infer method parameter and return types from AST context and naming heuristics
- preserve language-specific idioms

Tests (10): Python->C++ receiver removal, return typing for queue operations,
parameter typing for enqueue/dequeue/peek family, no regression for non-class
functions, deterministic signature order, no placeholder tokens in signatures,
language coverage (cpp/rust/go), empty-method safety, parser round-trip, legacy
compatibility on previously passing cases.

### Step 1890: Data-model field materialization from source AST (10 tests)

Ensure parsed class/data models carry concrete field definitions through
cross-language lowering:
- dataclass fields and typed attributes become concrete target fields
- no empty data structs for known typed source classes

Tests (10): WorkItem field mapping, primitive type mapping correctness, nullable/
optional handling policy, ordering stability, no dropped fields in nested classes,
no accidental duplicate fields, parser span preservation, unknown-type diagnostics,
JSON snapshot stability, no regression on non-dataclass code.

### Step 1891: Language-specific method body scaffolding for core patterns (12 tests)

Add deterministic body templates for common containers/patterns:
- queue push/pop/peek/size/empty
- simple guard clauses and return fallbacks

Tests (12): queue method bodies non-empty, compile-clean output for C++ sample,
Rust/Go structural validity, no TODO markers, no placeholder comments, predictable
imports/includes, safe empty-queue behavior scaffold, deterministic output across
runs, regression on existing function bodies, no accidental semantic inversions,
syntax check pass, benchmark sample pass.

### Step 1892: `run_pipeline` quality payload hardening (8 tests)

Align `run_pipeline` quality payload with production criteria:
- gate failures clearly tied to generated code deficiencies
- structured reasons for non-ready outputs

Tests (8): quality/gates always present, failure reasons populated, placeholder
count accuracy, compile/test gate integration signals, backward-compatible payload
fields, deterministic warning ordering, no false-green on placeholder signatures,
schema sync update.

### Step 1893: Sprint 169 Integration Summary (8 tests)

Add `editor/src/Sprint169IntegrationSummary.h`.
Record: steps_completed=5 (1889-1893), pipeline_signatures_typed=true,
data_models_materialized=true, priorityqueue_pipeline_ready=true, success=true.

Tests (8): constructable summary struct, steps_completed==5, signature typing
active, field materialization active, benchmark pipeline ready, gate payload
valid, regression suite pass, success==true.

---

## Architecture Gate

- `run_pipeline` must not emit receiver placeholders in production target languages
- Class/data-model outputs must include concrete fields when source types exist
- Gate status must be truthful and machine-checkable

