# Sprint 163 Plan: Strict Grammar Foundation and Schema Normalization

## Context

Current MCP grammar coverage is complete (347 tools), but strictness is not maximal.
The grammar generator still falls back to broad productions (`any-object`,
`any-array`, `any-value`) for many nested argument shapes, which permits syntactic
validity without full schema-level correctness.

This sprint builds the strictness foundation:

1. define a measurable strictness score
2. normalize per-tool schemas into a canonical, recursion-safe form
3. refactor grammar generation so strict handling is possible for every schema node

No MCP tool names change. Tool contracts stay backward-compatible at the API level.

---

## Goals

1. Add strictness metrics for all tool schemas and generated grammars
2. Introduce canonical schema normalization for recursive codegen
3. Eliminate fallback productions at top-level tool arguments
4. Keep generated artifacts deterministic across runs

---

## Steps

### Step 1859: Strictness audit utility and baseline report (10 tests)

Add `tools/mcp/audit_grammar_strictness.py`.

Input:
- `tools/mcp/whetstone_tool_schemas.json`
- `tools/mcp/grammars/per_tool_schemas.json`
- `tools/mcp/grammars/*.gbnf`

Output:
- `tools/mcp/grammars/strictness_report.json`
- per-tool metrics: fallback count, nested coverage, enum coverage, unsupported keywords

Tests (10): deterministic output, counts align with 347 tools, missing-file handling,
invalid schema handling, fallback detection, enum detection, nested object detection,
array-item strictness detection, summary rollup correctness, JSON schema for report.

### Step 1860: Canonical schema normalizer (12 tests)

Add `tools/mcp/schema_normalizer.py`.

Normalize each tool schema into a canonical AST-like structure:
- explicit node kinds: object, array, string, integer, number, boolean, null
- preserved constraints: required, additionalProperties, enum, const, pattern,
  min/max, minItems/maxItems, oneOf/anyOf/allOf
- `$ref` expansion within document scope

Tests (12): nested object normalization, array-of-object normalization, enum/const
preservation, required/optional split, additionalProperties preservation, oneOf,
anyOf, allOf flattening semantics, `$ref` resolution, cycle guard, stable key order,
unsupported keyword capture, malformed schema fallback with diagnostics.

### Step 1861: Refactor grammar generator to use normalized schema IR (10 tests)

Update `tools/mcp/generate_tool_grammars.py` to consume normalized schema IR from
Step 1860 rather than ad-hoc field inspection.

No strict keyword expansion yet; this step is structural refactor only.

Tests (10): generated grammar parity for simple tools, deterministic ordering,
dispatch includes all tools, per-tool files still generated for all tools,
no regression in existing enum behavior, integer/number separation preserved,
boolean/null handling preserved, optional field handling preserved, required field
ordering stable, script exit code semantics unchanged.

### Step 1862: Top-level strictness gate (8 tests)

Disallow `any-object`/`any-array` at top-level argument properties unless explicitly
annotated with `x-whetstone-allow-broad: true` in schema metadata.

Tests (8): gate fails on broad top-level property, gate passes when annotated,
gate passes when strict object spec exists, failure message includes tool+field,
report includes waiver count, waivers are deterministic, unknown annotation ignored,
CI-friendly nonzero exit on violation.

### Step 1863: Sprint 163 Integration Summary (8 tests)

Add `editor/src/Sprint163IntegrationSummary.h`.
Record: steps_completed=5 (1859-1863), strictness_baseline_established=true,
top_level_broad_fallback_blocked=true, success=true.

Tests (8): constructable summary struct, steps_completed==5, baseline artifact exists,
normalizer available, generator refactor active, strictness gate active, no tool-count
regression (347), success==true.

---

## Architecture Gate

- No MCP runtime behavior changes; this sprint is tooling + generation pipeline only
- Generated grammar artifacts remain in `tools/mcp/grammars/`
- Deterministic generation is mandatory (stable ordering)
- All strictness checks must be machine-readable JSON

