# Sprint 235 Plan: Native-First Semantic Expansion Policy

## Goal
Prevent semantic task expansion from overriding already-strong native decomposition while preserving deterministic fallback for underspecified outputs.

## Steps
- Step 2217: Add semantic expansion policy mode (`always` vs `fallback_only`).
- Step 2218: Compute native decomposition signals before semantic expansion.
- Step 2219: Apply expansion only when native decomposition is too shallow under `fallback_only`.
- Step 2220: Emit expansion telemetry with fallback decision metadata.
- Step 2221: Distinguish "feature disabled" from "enabled but skipped" in summary output.
- Step 2222: Add `Sprint235IntegrationSummary.h` and execution tracker.
