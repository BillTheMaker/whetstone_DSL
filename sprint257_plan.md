# Sprint 257 Plan: Top-Gap Uplift Hard Gate

## Goal
Convert top-gap weighted raw scoring from advisory telemetry into an enforceable policy gate so non-improving raw-only runs fail fast.

## Steps
- Step 2332: Add raw top-gap uplift gate env control.
- Step 2333: Fail raw search when weighted top-gap mode is not properly configured.
- Step 2334: Fail raw search when best top-gap score does not improve baseline.
- Step 2335: Emit top-gap uplift gate policy fields in raw search summary.
- Step 2336: Add `Sprint257IntegrationSummary.h` and execution tracker.
