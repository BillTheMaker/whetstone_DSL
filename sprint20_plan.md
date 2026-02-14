# Sprint 20 Plan: Legacy Code Ingestion

## Context

This is what started the whole project. Take old code — code written before modern
safety practices, before ownership models, before async/await, before anyone thought
about annotation-driven workflows — and bring it into the modern world.

Sprint 20 builds the ingestion pipeline: parse legacy code, infer what it does and
what's dangerous about it, annotate it with the full taxonomy, and produce a modernization
plan as a workflow. The output is a skeleton of the modernized version with annotations
guiding the orchestrator.

**Prerequisites delivered by earlier sprints:**
- 17+ language parsers (Sprint 14, 17) including C, the primary legacy language
- Annotation inference across all 8 subjects (Sprint 11e)
- Routing annotations for workflow dispatch (Sprint 11e)
- Workflow model + orchestration engine (Sprints 12, 15)
- C++ depth for parsing complex legacy C++ (Sprints 12, 16)

---

## Phase 20a: Legacy Analysis Engine (Steps 438-443)

### Step 438: Code Age + Idiom Detection (12 tests)
- Detect legacy patterns: K&R function declarations, pre-C99 variable declarations,
  manual memory management without RAII, raw pointer arithmetic, goto-heavy control flow,
  deprecated API usage (gets, sprintf, strcpy)
- Language version detection: C89/C99/C11, C++03/11/14/17/20, Java 6/8/11/17,
  Python 2/3
- Idiom scoring: how "legacy" is this code on a 0-10 scale
- Output: per-function and per-file legacy score with specific findings

### Step 439: Safety Audit via Annotations (12 tests)
- Run annotation inference focused on safety:
  - Buffer overflow risk: @BoundsCheck(unchecked) on array operations
  - Use-after-free risk: @Owner(Manual) without clear @Lifetime tracking
  - Null dereference: @Nullability(nullable) on unchecked pointers
  - Race conditions: shared mutable state without @Sync or @Atomic
  - Integer overflow: arithmetic without @Overflow annotation
- Generate structured safety report with risk levels per function/file
- Map findings to CWE (Common Weakness Enumeration) codes where applicable

### Step 440: Modernization Suggestions (12 tests)
- For each legacy pattern, suggest a modernization path:
  - `malloc/free` → smart pointers (C++) or GC language (Java/Python/Rust)
  - `sprintf` → `std::format` or safe string builders
  - Raw loops → range-based for or iterators
  - `goto` → structured control flow
  - Manual vtable → virtual methods or trait objects
  - Global mutable state → dependency injection or module pattern
- Each suggestion is an annotation: @Modernize(from="pattern", to="replacement", risk="level")
- Suggestions grouped by effort: quick wins vs deep refactors

### Step 441: Modernization Workflow Generation (12 tests)
- Given a legacy file with analysis + suggestions:
  1. Create skeleton of modernized version (same structure, modern idioms)
  2. Annotate skeleton with routing: quick wins → deterministic, complex → LLM, risky → human
  3. Generate workflow from skeleton (WorkItems with dependencies)
  4. Order: safe changes first, risky changes last (human review on risky)
- Output: a ready-to-execute WorkflowState that the orchestrator can run

### Step 442: Modernization RPC + MCP (12 tests)
- `whetstone_analyze_legacy` — run legacy analysis on a file/project
- `whetstone_suggest_modernization` — get modernization suggestions
- `whetstone_create_modernization_workflow` — generate the full workflow
- `whetstone_get_safety_report` — structured safety audit
- 79+ MCP tools total

### Step 443: Phase 20a Integration (8 tests)
- Ingest a C file with legacy patterns → analyze → suggest → create workflow →
  orchestrate → deterministic modernizations applied → complex items prepared for LLM
- Safety report flags real issues (buffer overflow, null deref)
- Modernization workflow respects risk ordering
- Cross-language modernization: C → Rust (ownership safety)

---

## Phase 20b: Cross-Language Migration (Steps 444-448)

### Step 444: Migration Plan Generator (12 tests)
- Given source language + target language + codebase:
  1. Analyze entire project structure (modules, dependencies, APIs)
  2. Identify migration units (which files/modules migrate together)
  3. Determine migration order (dependencies first, leaf modules first)
  4. Annotate each unit: effort, risk, routing
- Output: MigrationPlan with ordered phases

### Step 445: API Boundary Preservation (12 tests)
- When migrating a module, preserve its API surface:
  - Function signatures maintained (or mapped to target-language equivalents)
  - Public types preserved with cross-language compatibility notes
  - FFI annotations where mixed-language boundaries exist
  - @Contract annotations verify pre/post conditions match
- Test: migrate internal implementation, verify external API unchanged

### Step 446: Test Generation for Migration Validation (12 tests)
- For each migrated module, auto-generate validation tests:
  - Input/output equivalence tests (same inputs → same outputs)
  - Edge case coverage from @Contract pre/post conditions
  - Performance regression tests from @Complexity annotations
- Tests generated as skeleton work items (routed to SLM/LLM)
- Test language matches target language

### Step 447: Migration Execution Integration (12 tests)
- Hook migration plan into the orchestration engine:
  - Each migration unit becomes a workflow
  - Cross-unit dependencies respected in execution order
  - Rollback annotations: if a unit fails validation, flag for human review
- Progressive migration: some units migrated while others remain in source language
- FFI boundary management during partial migration

### Step 448: Phase 20b Integration + Sprint Summary (8 tests)
- Full migration: 3-file C project → Rust, preserving API boundaries
- Generated tests validate behavioral equivalence
- Partial migration: 2 files migrated, 1 remains C with FFI annotations
- Sprint 20 totals: legacy analysis, modernization, migration all operational

---

## Step & Test Summary

| Phase | Steps | Tests | Theme |
|-------|-------|-------|-------|
| 20a | 438-443 | 68 | Legacy analysis, safety audit, modernization workflow |
| 20b | 444-448 | 56 | Migration planning, API preservation, test generation |
| **Total** | **438-448** | **~124** | 11 steps |
