# Sprint 252 Plan: Closure Ladder Policy Routing from Raw Uplift History

## Goal
Reduce wasted raw-only attempts by making closure ladder mode selection aware of recent raw candidate uplift performance.

## Steps
- Step 2305: Add raw uplift history analyzer.
- Step 2306: Add ladder policy controls for skip conditions.
- Step 2307: Skip `raw_only` when historical uplift rate is below threshold.
- Step 2308: Emit policy evidence artifact in ladder output directory.
- Step 2309: Add `Sprint252IntegrationSummary.h` and execution tracker.
