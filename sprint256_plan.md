# Sprint 256 Plan: Top-Gap Weighted Raw Candidate Selection

## Goal
Use prioritized raw-gap backlog signals directly in raw-only candidate scoring so variant selection is guided by historically missing intrinsic signals, not only profile fail count.

## Steps
- Step 2326: Extend raw candidate scorer with optional top-gap backlog input.
- Step 2327: Compute weighted `top_gap_score` and hit counts from candidate tasks.
- Step 2328: Add pipeline controls for weighted raw selection.
- Step 2329: Make raw candidate tie-break use `top_gap_score` when enabled.
- Step 2330: Emit baseline/best top-gap score telemetry in raw search summary.
- Step 2331: Add `Sprint256IntegrationSummary.h` and execution tracker.
