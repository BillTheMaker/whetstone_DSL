# Sprint 164 Plan: Recursive Max-Strict GBNF and JSON Schema Dispatch

## Context

Sprint 163 establishes strictness metrics and normalized schemas. This sprint
implements maximal practical strictness in generated grammars by replacing broad
fallback productions with recursively generated rules across nested objects,
arrays, unions, and constraints.

Target: generated artifacts should encode real argument shape constraints for each
tool, not only top-level scalar types.

---

## Goals

1. Generate recursive GBNF rules for nested object/array structures
2. Support union schemas (`oneOf`, `anyOf`, `allOf`) in both GBNF and JSON Schema
3. Encode common validation constraints directly in grammar where feasible
4. Reduce broad fallback usage to waived exceptions only

---

## Steps

### Step 1864: Recursive object/array rule generator (12 tests)

Extend `tools/mcp/generate_tool_grammars.py` with recursive rule emission:
- object properties become named subrules
- arrays become item-specific list rules
- nested object arrays generate composed rules
- tuple arrays (`prefixItems`) supported where present

Tests (12): nested object grammar, nested array grammar, array-of-object grammar,
object-in-array-in-object grammar, empty-object behavior, optional nested fields,
required nested fields, unique rule naming, no name collisions, deterministic output,
deep nesting limit guard, parseability of generated grammar.

### Step 1865: Union support (`oneOf`/`anyOf`/`allOf`) (10 tests)

Add union handling from normalized schema:
- `oneOf`: exclusive grammar alternatives
- `anyOf`: permissive alternatives with merged required handling
- `allOf`: merged intersection object for compatible branches

Update `dispatch_schema.json` generation to preserve exact union semantics.

Tests (10): oneOf generation, anyOf generation, allOf merge, conflicting allOf
detection, dispatch schema oneOf count preserved, per-tool union schema validity,
deterministic alternative ordering, error diagnostics for unsupported mixed unions,
no regression on non-union tools, parser acceptance for valid branch samples.

### Step 1866: Constraint encoding and strict scalar handling (10 tests)

Implement strict handling for:
- `const`, `enum`
- `pattern` (string)
- `minLength`/`maxLength` (string, with bounded grammar where feasible)
- `minimum`/`maximum`, `exclusiveMinimum`/`exclusiveMaximum`
- `minItems`/`maxItems`
- `additionalProperties: false` object closure

Tests (10): enum/const enforced, bounded integer range handling, min/max items,
closed object rejects unknown keys, pattern passthrough metadata captured when not
grammatically representable, length constraints encoded or explicitly reported,
numeric constraint metadata propagation, strict scalar no-fallback on supported
cases, diagnostics for unsupported constraints, deterministic behavior.

### Step 1867: Strictness regression suite (8 tests)

Add `tools/mcp/test_strict_grammar_roundtrip.py` and golden fixtures for a focused
tool subset with complex arguments (e.g. `whetstone_set_environment`,
`whetstone_mutate`, `whetstone_generate_taskitems`, `whetstone_run_pipeline`,
`whetstone_validate_taskitem`).

Tests (8): valid samples accepted, invalid shape rejected, invalid enum rejected,
invalid nested field rejected, unknown property rejected when closed, array item
type mismatch rejected, union mismatch rejected, golden output unchanged unless
intentional update.

### Step 1868: Sprint 164 Integration Summary (8 tests)

Add `editor/src/Sprint164IntegrationSummary.h`.
Record: steps_completed=5 (1864-1868), recursive_codegen_enabled=true,
union_support_enabled=true, strict_fallback_reduction_verified=true, success=true.

Tests (8): constructable summary struct, steps_completed==5, recursive generation
active, union support active, strictness report improved vs Sprint 163 baseline,
fixtures pass, dispatch schema still covers all tools, success==true.

---

## Architecture Gate

- Keep generator output deterministic
- No removal of existing tools or contract fields
- Unsupported JSON Schema constructs must be explicitly reported, never silently
  downgraded to broad fallback without a waiver reason
- Grammar generation runtime remains practical for 347 tools

