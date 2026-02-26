# Sprint 263 Plan: Raw Structural Projector Before Scoring

## Goal
Add a deterministic structural projector for raw candidate tasks before scoring so intrinsic search evaluates schema-normalized candidates (without full post-hardening stage).

## Steps
- Step 2363: Add raw candidate structural projector tool.
- Step 2364: Apply projector to baseline/candidate/retry raw tasks before scoring.
- Step 2365: Emit structural projector telemetry in summary.
- Step 2366: Validate OFF/ON behavior on hard sample.
- Step 2367: Add `Sprint263IntegrationSummary.h` and execution tracker.
