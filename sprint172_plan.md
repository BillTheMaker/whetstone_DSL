# Sprint 172 Plan: C++ Diagnostic Auto-Fix for Missing Standard Includes

## Context
Path B failures are currently dominated by deterministic compile errors like missing
`<vector>` include despite emitted `std::vector` usage.

## Goals
1. Auto-repair common missing STL includes from compile diagnostics
2. Integrate repair into strict production loop before declaring blocked
3. Keep repair deterministic and fully traceable

## Steps

### Step 1904: C++ include fixer from gate diagnostics (10 tests)
Implement diagnostic-driven include insertion tool:
- parse missing-header hints from compiler stderr
- add needed headers (`vector`, `map`, `set`, `optional`, `tuple`, `memory`)
- preserve include ordering and idempotency

### Step 1905: Loop integration for auto-fix attempt (8 tests)
Wire include fixer into production loop:
- run fix attempt after compile/test fail
- re-run gates after fix
- trace both pre/post fix gate states

### Step 1906: Sprint 172 Integration Summary (8 tests)
Add `editor/src/Sprint172IntegrationSummary.h` with:
- steps_completed=3
- cpp_include_autofix_active=true
- loop_autofix_attempted=true
- success=true

## Architecture Gate
- Missing STL include should not require manual prompt rewrite
- Auto-fix must be auditable and idempotent
