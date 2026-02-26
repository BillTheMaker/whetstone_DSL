# Sprint 237 Plan: Semantic Fallback Budget Gate

## Goal
Turn semantic fallback audit signal into an enforceable gate for CI and batch execution.

## Steps
- Step 2228: Add fallback-budget gate checker.
- Step 2229: Add wrapper that runs audit + gate in one command.
- Step 2230: Support configurable fallback budget and minimum sample size.
- Step 2231: Emit deterministic gate artifact with pass/fail reason.
- Step 2232: Add `Sprint237IntegrationSummary.h` and execution tracker.
