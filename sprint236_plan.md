# Sprint 236 Plan: Semantic Fallback Gap Audit + Tool Metadata

## Goal
Produce deterministic telemetry showing where semantic fallback is compensating for weak native decomposition, and attach tool/constraint metadata to close those gaps.

## Steps
- Step 2223: Add semantic fallback gap analyzer for pipeline summaries.
- Step 2224: Classify fallback root causes using native task and semantic-signal counts.
- Step 2225: Emit per-run remediation metadata: recommended tools + taskitem constraints.
- Step 2226: Generate dated TEST_ONLY audit artifacts from sprint 235 runs.
- Step 2227: Add `Sprint236IntegrationSummary.h` and execution tracker.
