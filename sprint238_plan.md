# Sprint 238 Plan: Native Decomposition Quality Gate

## Goal
Detect and optionally block weak native decomposition before semantic fallback hides generation deficiencies.

## Steps
- Step 2233: Add native decomposition gate packet to pipeline summary.
- Step 2234: Support configurable minimum native task count and semantic signal count.
- Step 2235: Add optional hard-fail mode for gate violations.
- Step 2236: Persist gate artifact for every pipeline run.
- Step 2237: Add `Sprint238IntegrationSummary.h` and execution tracker.
