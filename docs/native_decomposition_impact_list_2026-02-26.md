# Native Decomposition Impact List - 2026-02-26

Status: Active
Purpose: Enumerate all known areas affected by shallow native decomposition and map each to enforceable tooling.

## Impact List (One-by-One)

1. API contract evolution
- Problem: compatibility/versioning work collapses into generic tasks.
- Tooling: `impact_api_contract_evolution` profile in `tools/mcp/profiles/native_decomposition_impact_profiles.json`.

2. State migration and rollback safety
- Problem: migration/backfill/rollback choreography omitted.
- Tooling: `impact_state_migration_rollback` profile.

3. Security policy propagation
- Problem: cross-layer auth/deny-by-default checks missing.
- Tooling: `impact_security_policy_propagation` profile.

4. SLO/latency budget constraints
- Problem: performance requirements missing from decomposition rationale.
- Tooling: `impact_slo_latency_budget` profile.

5. Rollout choreography
- Problem: staged rollout/abort hooks not represented as tasks.
- Tooling: `impact_rollout_choreography` profile.

6. Multi-file transactional edits
- Problem: partial cross-file updates pass as “done”.
- Tooling: `impact_multifile_transactional_edit` profile.

7. Concurrency/locking semantics
- Problem: race/lock/deadlock work under-scoped.
- Tooling: `impact_concurrency_locking` profile.

8. Projection/environment constraints
- Problem: embedded/mobile/serverless constraints ignored.
- Tooling: `impact_projection_environment_constraints` profile.

9. Polyglot boundary changes
- Problem: SDK/client/server coupling not decomposed.
- Tooling: `impact_polyglot_boundary_changes` profile.

10. Observability/telemetry integrity
- Problem: logging/metrics/tracing semantics omitted.
- Tooling: `impact_observability_telemetry_integrity` profile.

11. Data integrity and precision
- Problem: financial/scientific precision risks hidden in generic tasks.
- Tooling: `impact_data_integrity_precision` profile.

12. Recovery/replay-critical flows
- Problem: recovery/rollback/replay pathways not represented.
- Tooling: `impact_disaster_recovery_replay` profile.

## Enforcement Tooling

- Checker:
  - `tools/mcp/check_native_decomposition_impact_coverage.py`
- Profiles:
  - `tools/mcp/profiles/native_decomposition_impact_profiles.json`
- Standalone gate wrapper:
  - `tools/mcp/run_native_impact_coverage_gate.sh`
- Pipeline integration toggles in `tools/mcp/run_sprint_taskitem_pipeline.sh`:
  - `WSTONE_NATIVE_IMPACT_COVERAGE_GATE`
  - `WSTONE_NATIVE_IMPACT_COVERAGE_ENFORCE`
  - `WSTONE_NATIVE_IMPACT_COVERAGE_PROFILES`

