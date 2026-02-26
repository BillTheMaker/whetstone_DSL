# Sprint 173 Plan: Pre-Gate Static Analysis Hook

## Context
Compile/test gates catch hard failures, but we still need pre-gate static checks to
surface maintainability and risk signals before final gate decision.

## Goals
1. Add static-analysis hook (best-effort, tool-aware)
2. Report normalized lint diagnostics with deterministic schema
3. Support optional strict lint blocking mode

## Steps

### Step 1907: Lint executor integration (10 tests)
Add lint stage to gate evaluator:
- C++: `cppcheck` when available
- Python: `python -m pyflakes` when available
- fallback to skipped with explicit reason

### Step 1908: Lint diagnostics normalization (8 tests)
Normalize lint outputs into shared diagnostic payload:
- severity/category/file/line/message/raw_excerpt
- deterministic ordering and truncation

### Step 1909: Sprint 173 Integration Summary (8 tests)
Add `editor/src/Sprint173IntegrationSummary.h` with:
- steps_completed=3
- static_lint_hook_active=true
- normalized_lint_diagnostics_active=true
- success=true

## Architecture Gate
- Lint status must never be silently dropped
- Missing lint tool must be explicit (`skipped` + reason)
