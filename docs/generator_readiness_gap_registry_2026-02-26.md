# Generator Readiness Gap Registry (Dated)

- Date created: 2026-02-26
- Last reviewed: 2026-02-26
- Status: Active
- Scope: Whetstone autonomous code generation and deterministic execution readiness.

## Purpose

This is the canonical dated registry for "not production-ready" generator gaps. It consolidates scattered notes from feature requests, A/B reports, and sprint execution logs.

## Source Documents

- `docs/taskitem_pipeline_gap_log_2026-02-26.md`
- `docs/ab_test_ast_vs_language_first_2026-02-25.md`
- `FEATURE_REQUESTS.md`
- `docs/sprint175_179_execution_tracker_2026-02-26.md`
- `docs/sprint186_205_execution_tracker_2026-02-26.md`

## Status Legend

- `done`: implemented and validated in code/tests for this scope
- `partial`: implemented foundation, but not full production closure
- `open`: still a feature request or known unresolved reliability gap

## Gap Registry

| Gap ID | Gap | Source(s) | Current Status | Coverage Notes | Next Target |
|---|---|---|---|---|---|
| GR-001 | Generic taskitems and weak execution constraints | taskitem_pipeline_gap_log (items 1,2,4) | `done` | Strict execution contract mode, queue blockers, specificity scoring, and deterministic metadata implemented in sprints 180-182 and verified in strict runs. | Monitor drift in future runs |
| GR-002 | Validation over-scores weak plans | taskitem_pipeline_gap_log (item 3) | `partial` | Specificity scoring and penalties added; Sprint 186 delivered calibration artifacts and threshold analyzer (`tools/mcp/analyze_taskitem_calibration.py`) with run evidence at `logs/taskitem_runs/sprint186_plan_20260226_110123`. Cross-project threshold stability still pending. | Sprints 186-187 |
| GR-003 | Missing deterministic replay/rollback compatibility enforcement | taskitem_pipeline_gap_log (item 5) | `partial` | Sprint 188-189 added deterministic enforcement checks via readiness suite (`execution_contract_enforcement`) and surfaced policy packets in run summaries; production-loop hard gate still needs deeper enforcement. | Follow-up hard-gate tightening |
| GR-004 | Capability-gap routing lacks executable handoff | FEATURE_REQUESTS (deterministic debugging workflow), sprint184 tracker | `done` | Queue + validation now emit executable call plans with deterministic fingerprints (`debugLoopCallPlan`, `capability_gap_call_plan`) and capability-gap run evidence is present. | Monitor for drift |
| GR-005 | Intake drops functional requirements in some markdown specs | ab_test_ast_vs_language_first (Path B failure point #1) | `partial` | Sprint 192-193 added deterministic intake quality/drop diagnostics in pipeline postchecks; core parser recall still needs deeper model improvements. | Parser recall hardening follow-up |
| GR-006 | Complex multi-method class generation unreliable/unsupported in direct generator path | ab_test_ast_vs_language_first (Path A stop) | `partial` | Sprint 194 added class-generation observability and gating signals, but not full direct generator parity closure. | Direct generator class-body completion |
| GR-007 | Cross-language class-node emission gaps (classes silently dropped) | ab_test_ast_vs_language_first (post-test audit note) | `partial` | Prior notes indicate root cause identified and partial fixes in earlier sprint track; this registry has no fresh dated verification run proving all language emitters are closed. | Sprint 195 |
| GR-008 | Concurrency/resource mutual exclusion not modeled in taskitems (`resourceLocks`) | FEATURE_REQUESTS resource lock request | `partial` | Sprint 200-202 added `resourceLocks` emission, queue conflict warning telemetry, and validation lock-semantic checks; scheduler-level enforcement remains project-integration dependent. | Scheduler integration follow-up |
| GR-009 | Long-range/cross-file edit reliability under constrained autonomous execution | implied by prior handoff concerns + feature request direction | `partial` | Sprint 196-199 added long-range contract/reliability observability checks and promotion-packet gating; benchmark corpus depth still needs expansion. | Benchmark expansion and hard thresholds |
| GR-010 | Semantic + recursive completion gates beyond compile/test | FEATURE_REQUESTS derived API request | `partial` | Sprint 203-205 added validation `promotion_packet` and readiness promotion gate synthesis; deeper semantic equivalence hooks remain to be integrated. | Semantic equivalence engine follow-up |
| GR-011 | False-green production gating on hard specs | `logs/taskitem_runs/challenging_subset_prod_20260226`, `logs/taskitem_runs/challenging_subset_prod_20260226_r2`, `docs/deterministic_gap_hunt_2026-02-26.md` | `partial` | Sprint 206 added hard gate-evidence fields and anti-false-green blocked reasons in production loop summaries. Matrix-level `false_green_candidates` signal still non-zero on focused subset, requiring deeper equivalence checks. | Sprint 212 follow-up |
| GR-012 | Projection/environment constraints not first-class in execution path | `datasets/project_benchmarks/challenging_projection_projects_2026-02-26.jsonl`, `logs/taskitem_runs/challenging_projection_invalid_20260226/summary.json`, `docs/deterministic_gap_hunt_2026-02-26.md` | `partial` | Sprint 207-208 added projection contract ingestion and strict invalid-contract blocking in benchmark orchestration. Full generator/runtime enforcement of per-target budgets/APIs remains incomplete. | Sprint 213 follow-up |
| GR-013 | Cross-language hard-spec readiness asymmetry | `logs/taskitem_runs/challenging_matrix_20260226/summary.json`, `logs/taskitem_runs/challenging_matrix_20260226/parity_skew.json` | `partial` | Sprint 209 added parity skew analyzer and threshold pass/fail output. Skew remains high (`overall_skew=1.0`). | Sprint 214 follow-up |
| GR-014 | Language-first token blowup under hard semantics | `logs/taskitem_runs/challenging_matrix_20260226/summary.json`, `logs/taskitem_runs/challenging_matrix_20260226/efficiency_report.json` | `partial` | Sprint 210 added low-yield efficiency diagnostics. Low-yield runs remain high (`36`). | Sprint 215 follow-up |
| GR-015 | Intent-vs-closeout drift | `docs/sprint186_205_intent_audit_2026-02-26.md`, `docs/sprint186_205_closeout_report_2026-02-26.md`, `docs/sprint186_205_intent_closeout_consistency_2026-02-26.json` | `partial` | Sprint 211 added consistency checker and explicit inconsistency count (`19`). Closeout gating not yet bound to this consistency check as hard fail. | Sprint 216 follow-up |
| GR-016 | Multi-file transactional closure gap | `docs/gap_hunt_fullstack_multifile_2026-02-26.md`, `logs/taskitem_runs/challenging_fullstack_multifile_20260226_r7/summary.json`, `docs/sprint225_227_execution_tracker_2026-02-26.md` | `partial` | Contract gating is enforced and parity blockers/divergence are both `0` on current fullstack hard catalog (`r7`). Remaining risk is generalization breadth: current repair closure is strongest on queue-shaped transpile artifacts used in these runs. | Expand non-queue multi-file corpus |
| GR-017 | Full-stack contract propagation gap | `docs/gap_hunt_fullstack_multifile_2026-02-26.md`, `logs/taskitem_runs/challenging_fullstack_multifile_20260226_r5/fullstack_contract_closure.json` | `partial` | Full-stack contract packet + strict validation added in benchmark orchestration and summary aggregation. Contract invalid rate is `0%` on valid catalog, but cross-layer semantic consistency is not yet equivalence-verified. | Semantic propagation verifier sprint |
| GR-018 | Performance/security/rollout constrained refactor enforcement gap | `docs/gap_hunt_fullstack_multifile_2026-02-26.md`, `logs/taskitem_runs/challenging_fullstack_multifile_20260226_r5/results.jsonl` | `partial` | Hard checks now enforce migration rollback + data-loss policy, security deny-by-default, SLO p95 presence, and rollout staged+abort policy. Enforcement is contract-level; generator capability under these constraints is still weak in C++ AB path. | Constraint-aware generation follow-up |
| GR-019 | Parity-blocked readiness load (gating without capability closure) | `logs/taskitem_runs/challenging_fullstack_multifile_20260226_r7/summary.json`, `logs/taskitem_runs/challenging_subset_prod_20260226_r7/summary.json`, `docs/sprint225_227_execution_tracker_2026-02-26.md` | `partial` | Sprint 225-227 reduced blocked parity load from `6` to `0` on both tracked hard catalogs while keeping unresolved divergence at `0`. This closes immediate safety debt for current corpora, but robustness is still contingent on pattern-driven repair classes. | Generalize repairs beyond queue-shaped transpile outputs |
| GR-020 | Semantic fallback overuse masks weak native decomposition | `logs/taskitem_runs/TEST_ONLY_sprint236_semantic_fallback_audit_20260226/semantic_fallback_summary.json`, `logs/taskitem_runs/TEST_ONLY_sprint237_semantic_fallback_gate_fail_20260226/semantic_fallback_budget_gate.json`, `logs/taskitem_runs/TEST_ONLY_sprint238_native_gate_fail_20260226.json`, `logs/taskitem_runs/TEST_ONLY_sprint239_semantic_fallback_audit_20260226/semantic_fallback_summary.json`, `logs/taskitem_runs/TEST_ONLY_sprint240_retry_20260226_145631/00_summary.json`, `docs/sprint236_execution_tracker_2026-02-26.md`, `docs/sprint237_execution_tracker_2026-02-26.md`, `docs/sprint238_execution_tracker_2026-02-26.md`, `docs/sprint239_execution_tracker_2026-02-26.md`, `docs/sprint240_execution_tracker_2026-02-26.md` | `partial` | Sprint 236 added deterministic fallback-gap auditing with tool + constraint metadata. Sprint 237 added fallback-budget hard gate (`max-fallback-rate`) and produced expected fail/pass artifacts. Sprint 238 added native decomposition hard gate in pipeline (`min task count`, `min semantic signal count`) so weak native generation can be blocked before fallback masking. Sprint 239 added native reason enrichment and improved semantic signal density (`0 -> 6`). Sprint 240 added native decomposition retry with explicit minimum-task policy; on current sample retry attempted but did not improve depth (`2 -> 2`). | Native decomposition task-depth upgrade (increase native task granularity beyond 2) |
| GR-021 | Impact-specific native decomposition coverage not enforced uniformly | `docs/native_decomposition_impact_list_2026-02-26.md`, `tools/mcp/profiles/native_decomposition_impact_profiles.json`, `logs/taskitem_runs/TEST_ONLY_sprint241_fullstack_impact_20260226_150416/00_summary.json`, `logs/taskitem_runs/TEST_ONLY_sprint241_native_impact_aggregate_20260226/native_impact_coverage_aggregate.json`, `logs/taskitem_runs/TEST_ONLY_sprint242_impact_remediation_loop_20260226/remediation_loop_summary.json`, `logs/taskitem_runs/TEST_ONLY_sprint243_impact_remediation_tasks_loop_20260226_r2/remediation_loop_summary.json`, `logs/taskitem_runs/TEST_ONLY_sprint244_autofill_20260226_151651/00_summary.json`, `logs/taskitem_runs/TEST_ONLY_sprint244_autofill_enforce_20260226_151709/00_summary.json`, `logs/taskitem_runs/TEST_ONLY_sprint245_intrinsic_20260226_151835/00_summary.json`, `logs/taskitem_runs/TEST_ONLY_sprint246_multishot_20260226_153033/00_summary.json`, `logs/taskitem_runs/TEST_ONLY_sprint246_multishot_enforce_20260226_153045/00_summary.json`, `logs/taskitem_runs/TEST_ONLY_sprint247_singleshot_20260226_153230/00_summary.json`, `logs/taskitem_runs/TEST_ONLY_sprint247_singleshot_enforce_20260226_153241/00_summary.json`, `logs/taskitem_runs/TEST_ONLY_sprint248_rawsearch_20260226_153903/00_summary.json`, `logs/taskitem_runs/TEST_ONLY_sprint249_rawguard_20260226_154028/02ae_raw_candidate_search.json`, `logs/taskitem_runs/TEST_ONLY_sprint250_closure_ladder_20260226/closure_ladder_summary.json`, `logs/taskitem_runs/TEST_ONLY_sprint251_rawvariants_20260226_154355/00_summary.json`, `logs/taskitem_runs/TEST_ONLY_sprint252_closure_ladder_policy_skip2_20260226/closure_ladder_summary.json`, `logs/taskitem_runs/TEST_ONLY_sprint253_closure_ladder_batch_20260226/closure_ladder_batch_summary.json`, `logs/taskitem_runs/TEST_ONLY_sprint254_closure_ladder_batch_20260226_r2/raw_gap_backlog.json`, `logs/taskitem_runs/TEST_ONLY_sprint255_rawharden_20260226_161924/00_summary.json`, `logs/taskitem_runs/01a_fallback_intake_spec_20260226_162423/00_summary.json`, `logs/taskitem_runs/01a_fallback_intake_spec_20260226_162633/02ae_raw_candidate_search.json`, `logs/taskitem_runs/01a_fallback_intake_spec_20260226_162924/01e_raw_top_gap_requirements_report.json`, `logs/taskitem_runs/01a_fallback_intake_spec_20260226_163148/02af_raw_adaptive_retry_score.json`, `logs/taskitem_runs/01a_fallback_intake_spec_20260226_163556/02ae_raw_signal_targeted_variants.json`, `logs/taskitem_runs/01a_fallback_intake_spec_20260226_163855/01f_raw_intrinsic_prompt_pack_report.json` | `partial` | Sprint 241 introduced impact profiles/gates; 242-243 remediation loops; 244 first-pass autofill; 245 intrinsic constraints only (no uplift); 246 multishot closure; 247 single-shot shaping closure; 248 raw candidate search no uplift; 249 no-uplift guardrail; 250 closure ladder; 251 richer raw variants no uplift; 252 history-aware ladder routing; 253 batch ladder analytics; 254 raw gap backlog synthesis from `raw_only` attempts identifies top missing raw signals; 255 adds first-pass raw top-gap hardening that closes hard-sample profile coverage (`failing_profile_count 7 -> 0`) by injecting missing ops/contracts/reasons and minimum task depth; 256 adds top-gap weighted candidate selection telemetry and tie-break policy; 257 adds hard fail policy when top-gap uplift is required but absent (`exit 18`); 258 injects backlog-driven top-gap normalized requirements before raw generation; 259 adds adaptive retry with expanded top-gap requirements and deterministic retry scoring; 260 adds signal-targeted variant expansion keyed to missing-signal classes and increases intrinsic search breadth (`available_variants 2 -> 8`); 261 adds intrinsic prompt-pack constraint injection keyed to gap signals. Measured intrinsic output still shows no uplift on this sample (`best_top_gap_score=0`, `failing_profile_count=1`). | Add explicit generator-side output-shape control (schema-guided decoder or task-template synthesis pass) before candidate scoring |

## What Was Covered Today (Sprints 175-184)

Covered heavily:
- Taskitem strict execution contracts
- Queue blocker metadata + deterministic gap classes
- Capability-gap classification + remediation routing hints
- Pipeline strict-mode wiring and summaries

Not fully covered:
- Concurrency/resource lock schema + scheduler semantics
- Intake robustness for complex requirements markdown
- Full complex class generation reliability and long-range edit reliability benchmarks
- Full semantic/recursive completion gate APIs

## Planned Sprint Coverage (New)

- `sprint186_plan.md` to `sprint205_plan.md` created and executed on 2026-02-26.
- Coverage map:
  - calibration: 186-187
  - replay/rollback enforcement: 188-189
  - capability-gap executable handoff: 190-191 (plus 185)
  - intake hardening: 192-193
  - complex class generation + emitter parity: 194-195
  - long-range edit reliability: 196-199
  - resource lock constraints/concurrency semantics: 200-202
  - semantic + recursive promotion gates: 203-205

## Freshness Protocol For Next Agents

When continuing work, update this file in-place:
1. Set `Last reviewed` to current date.
2. For any changed gap, update status and "Coverage Notes" with evidence artifact paths.
3. If a new major generator limitation appears, add a new `GR-xxx` row.
4. If no updates in 3+ days, treat this registry as potentially stale and re-validate against latest commits and run artifacts.
