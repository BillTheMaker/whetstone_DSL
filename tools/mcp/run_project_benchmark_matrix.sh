#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
CATALOG="${1:-$ROOT_DIR/datasets/project_benchmarks/common_projects_100.jsonl}"
OUT_DIR="${OUT_DIR:-$ROOT_DIR/logs/taskitem_runs/project_benchmark_matrix_$(date +%Y%m%d_%H%M%S)}"
LANG_MATRIX="${LANG_MATRIX:-cpp,python,go,rust}"
RUN_AB="${RUN_AB:-1}"
RUN_PROD="${RUN_PROD:-0}"
STRICT_MODE="${STRICT_MODE:-1}"
LIMIT="${LIMIT:-0}"
REQUIRE_AB_PARITY="${REQUIRE_AB_PARITY:-1}"

if [[ ! -f "$CATALOG" ]]; then
  echo "error: catalog not found: $CATALOG" >&2
  exit 1
fi

if ! command -v jq >/dev/null 2>&1; then
  echo "error: jq required" >&2
  exit 1
fi

mkdir -p "$OUT_DIR"
RESULTS_JSONL="$OUT_DIR/results.jsonl"
touch "$RESULTS_JSONL"

IFS=',' read -r -a LANGS <<< "$LANG_MATRIX"
total_specs=$(wc -l < "$CATALOG" | tr -d ' ')
if [[ "$LIMIT" -gt 0 && "$LIMIT" -lt "$total_specs" ]]; then
  total_specs="$LIMIT"
fi

spec_idx=0
while IFS= read -r row; do
  spec_idx=$((spec_idx + 1))
  if [[ "$LIMIT" -gt 0 && "$spec_idx" -gt "$LIMIT" ]]; then
    break
  fi

  id="$(printf '%s' "$row" | jq -r '.id')"
  project="$(printf '%s' "$row" | jq -r '.project')"
  category="$(printf '%s' "$row" | jq -r '.category')"
  language_declared="$(printf '%s' "$row" | jq -r '.language')"
  spec="$(printf '%s' "$row" | jq -r '.spec')"
  core_semantics="$(printf '%s' "$row" | jq -c '.core_semantics // {}')"
  projection_targets="$(printf '%s' "$row" | jq -c '.projection_targets // []')"
  cross_target_invariants="$(printf '%s' "$row" | jq -c '.cross_target_invariants // []')"
  degradation_policy="$(printf '%s' "$row" | jq -c '.degradation_policy // {}')"
  constraints="$(printf '%s' "$row" | jq -c '.constraints // {}')"

  projection_contract_ok="true"
  projection_contract_reason=""
  if [[ "$category" == "projection_constraints" ]]; then
    target_count="$(printf '%s' "$projection_targets" | jq 'length')"
    invariants_count="$(printf '%s' "$core_semantics" | jq '.invariants // [] | length')"
    if [[ "$target_count" -lt 1 ]]; then
      projection_contract_ok="false"
      projection_contract_reason="projection_targets_missing"
    elif [[ "$invariants_count" -lt 1 ]]; then
      projection_contract_ok="false"
      projection_contract_reason="core_semantics_invariants_missing"
    fi
  fi

  fullstack_contract_ok="true"
  fullstack_contract_reason=""
  required_artifact_count="$(printf '%s' "$constraints" | jq '.required_artifacts // [] | length')"
  require_multifile="$(printf '%s' "$constraints" | jq '.require_multifile // false')"
  if [[ "$require_multifile" == "true" && "$required_artifact_count" -lt 3 ]]; then
    fullstack_contract_ok="false"
    fullstack_contract_reason="required_artifacts_insufficient_for_multifile"
  fi
  if [[ "$category" == "fullstack_api_evolution" ]]; then
    compat_days="$(printf '%s' "$constraints" | jq '.compatibility.backward_compatible_window_days // 0')"
    if [[ "$compat_days" -le 0 ]]; then
      fullstack_contract_ok="false"
      fullstack_contract_reason="compatibility_window_missing"
    fi
  fi
  if [[ "$category" == "state_migration" ]]; then
    rollback_required="$(printf '%s' "$constraints" | jq '.migration.rollback_required // false')"
    if [[ "$rollback_required" != "true" ]]; then
      fullstack_contract_ok="false"
      fullstack_contract_reason="migration_rollback_requirement_missing"
    fi
    data_loss_allowed="$(printf '%s' "$constraints" | jq -r 'if (.migration | type) == "object" and (.migration | has("data_loss_allowed")) then .migration.data_loss_allowed else true end')"
    if [[ "$data_loss_allowed" != "false" ]]; then
      fullstack_contract_ok="false"
      fullstack_contract_reason="migration_data_loss_policy_invalid"
    fi
  fi
  if [[ "$category" == "security_propagation" ]]; then
    deny_default="$(printf '%s' "$constraints" | jq '.security.deny_by_default // false')"
    if [[ "$deny_default" != "true" ]]; then
      fullstack_contract_ok="false"
      fullstack_contract_reason="security_deny_by_default_missing"
    fi
  fi
  if [[ "$category" == "performance_budgeted_change" ]]; then
    p95="$(printf '%s' "$constraints" | jq '.slo.p95_latency_ms // 0')"
    if [[ "$p95" -le 0 ]]; then
      fullstack_contract_ok="false"
      fullstack_contract_reason="slo_p95_latency_missing"
    fi
  fi
  if [[ "$category" == "rollout_choreography" ]]; then
    rollout_staged="$(printf '%s' "$constraints" | jq '.rollout.staged // false')"
    rollout_abort_on_breach="$(printf '%s' "$constraints" | jq '.rollout.auto_abort_on_error_budget_breach // false')"
    if [[ "$rollout_staged" != "true" ]]; then
      fullstack_contract_ok="false"
      fullstack_contract_reason="rollout_staged_policy_missing"
    elif [[ "$rollout_abort_on_breach" != "true" ]]; then
      fullstack_contract_ok="false"
      fullstack_contract_reason="rollout_auto_abort_policy_missing"
    fi
  fi

  for lang in "${LANGS[@]}"; do
    run_key="${id}_${lang}"
    run_dir="$OUT_DIR/$run_key"
    mkdir -p "$run_dir"

    ab_status="skipped"
    ab_ready_a="false"
    ab_ready_b="false"
    ab_fail_a="[]"
    ab_fail_b="[]"
    ab_tok_a=0
    ab_tok_b=0
    ab_out_dir=""
    ab_err=""

    if [[ "$RUN_AB" == "1" ]]; then
      ab_out_dir="$run_dir/ab"
      mkdir -p "$ab_out_dir"
      if [[ "$STRICT_MODE" == "1" && "$projection_contract_ok" != "true" ]]; then
        ab_status="failed"
        ab_err="projection_contract_invalid:${projection_contract_reason}"
      elif [[ "$STRICT_MODE" == "1" && "$fullstack_contract_ok" != "true" ]]; then
        ab_status="failed"
        ab_err="fullstack_contract_invalid:${fullstack_contract_reason}"
      elif OUT_DIR="$ab_out_dir" WSTONE_LANGUAGE="$lang" STRICT_MODE="$STRICT_MODE" \
        "$ROOT_DIR/tools/mcp/run_ab_test_ast_vs_language_first.sh" "$spec" > "$run_dir/ab_stdout.log" 2>"$run_dir/ab_stderr.log"; then
        ab_status="ok"
      else
        ab_status="failed"
        ab_err="$(tail -n 5 "$run_dir/ab_stderr.log" | tr '\n' ' ' | sed 's/"/\\"/g')"
      fi
      if [[ -f "$ab_out_dir/00_summary.json" ]]; then
        ab_ready_a="$(jq -r '.path_a.gate_overall_ready // false' "$ab_out_dir/00_summary.json")"
        ab_ready_b="$(jq -r '.path_b.gate_overall_ready // false' "$ab_out_dir/00_summary.json")"
        ab_fail_a="$(jq -c '.path_a.failure_reasons // []' "$ab_out_dir/00_summary.json")"
        ab_fail_b="$(jq -c '.path_b.failure_reasons // []' "$ab_out_dir/00_summary.json")"
        ab_tok_a="$(jq -r '.path_a.token_accounting.total_tokens // 0' "$ab_out_dir/00_summary.json")"
        ab_tok_b="$(jq -r '.path_b.token_accounting.total_tokens // 0' "$ab_out_dir/00_summary.json")"
      fi
    fi

    prod_status="skipped"
    prod_ready="false"
    prod_blocked_reason=""
    prod_gate_evidence_complete="false"
    prod_compile_pass="false"
    prod_tests_pass="false"
    prod_ab_divergence="false"
    prod_ab_consistency_blocked="false"
    prod_out_dir=""
    prod_err=""
    if [[ "$RUN_PROD" == "1" ]]; then
      prod_out_dir="$run_dir/prod"
      mkdir -p "$prod_out_dir"
      if [[ "$STRICT_MODE" == "1" && "$projection_contract_ok" != "true" ]]; then
        prod_status="failed"
        prod_err="projection_contract_invalid:${projection_contract_reason}"
      elif [[ "$STRICT_MODE" == "1" && "$fullstack_contract_ok" != "true" ]]; then
        prod_status="failed"
        prod_err="fullstack_contract_invalid:${fullstack_contract_reason}"
      elif OUT_DIR="$prod_out_dir" WSTONE_LANGUAGE="$lang" STRICT_MODE="$STRICT_MODE" \
        "$ROOT_DIR/tools/mcp/run_production_completion_loop.sh" "$spec" > "$run_dir/prod_stdout.log" 2>"$run_dir/prod_stderr.log"; then
        prod_status="ok"
      else
        prod_status="failed"
        prod_err="$(tail -n 5 "$run_dir/prod_stderr.log" | tr '\n' ' ' | sed 's/"/\\"/g')"
      fi
      if [[ -f "$prod_out_dir/00_summary.json" ]]; then
        prod_ready="$(jq -r '.overall_ready // false' "$prod_out_dir/00_summary.json")"
        prod_blocked_reason="$(jq -r '.blocked_reason // ""' "$prod_out_dir/00_summary.json" | sed 's/"/\\"/g')"
        prod_gate_evidence_complete="$(jq -r '.gate_evidence_complete // false' "$prod_out_dir/00_summary.json")"
        prod_compile_pass="$(jq -r '.gate_proofs.compile.passed // false' "$prod_out_dir/00_summary.json")"
        prod_tests_pass="$(jq -r '.gate_proofs.tests.passed // false' "$prod_out_dir/00_summary.json")"
        # Divergence flag: language-first AB failed compile/tests while production reports both compile+tests passed.
        if [[ "$ab_ready_b" == "false" ]] &&
           [[ "$ab_fail_b" == *"compile_failed:non_zero_exit"* || "$ab_fail_b" == *"tests_failed:non_zero_exit"* ]] &&
           [[ "$prod_compile_pass" == "true" && "$prod_tests_pass" == "true" ]]; then
          prod_ab_divergence="true"
          if [[ "$REQUIRE_AB_PARITY" == "1" ]]; then
            prod_ab_consistency_blocked="true"
            prod_ab_divergence="false"
            prod_ready="false"
            prod_status="failed"
            prod_blocked_reason="ab_parity_blocked:path_b_compile_or_test_failed"
          fi
        fi
      fi
    fi

    jq -nc \
      --arg id "$id" \
      --arg project "$project" \
      --arg category "$category" \
      --arg language_declared "$language_declared" \
      --arg language_exec "$lang" \
      --arg spec "$spec" \
      --argjson core_semantics "$core_semantics" \
      --argjson projection_targets "$projection_targets" \
      --argjson cross_target_invariants "$cross_target_invariants" \
      --argjson degradation_policy "$degradation_policy" \
      --argjson constraints "$constraints" \
      --argjson projection_contract_ok "$projection_contract_ok" \
      --arg projection_contract_reason "$projection_contract_reason" \
      --argjson fullstack_contract_ok "$fullstack_contract_ok" \
      --arg fullstack_contract_reason "$fullstack_contract_reason" \
      --arg ab_status "$ab_status" \
      --argjson ab_ready_a "$ab_ready_a" \
      --argjson ab_ready_b "$ab_ready_b" \
      --argjson ab_fail_a "$ab_fail_a" \
      --argjson ab_fail_b "$ab_fail_b" \
      --argjson ab_tokens_a "$ab_tok_a" \
      --argjson ab_tokens_b "$ab_tok_b" \
      --arg ab_out_dir "$ab_out_dir" \
      --arg ab_err "$ab_err" \
      --arg prod_status "$prod_status" \
      --argjson prod_ready "$prod_ready" \
      --arg prod_blocked_reason "$prod_blocked_reason" \
      --argjson prod_gate_evidence_complete "$prod_gate_evidence_complete" \
      --argjson prod_compile_pass "$prod_compile_pass" \
      --argjson prod_tests_pass "$prod_tests_pass" \
      --argjson prod_ab_divergence "$prod_ab_divergence" \
      --argjson prod_ab_consistency_blocked "$prod_ab_consistency_blocked" \
      --arg prod_out_dir "$prod_out_dir" \
      --arg prod_err "$prod_err" \
      '{
        id:$id,
        project:$project,
        category:$category,
        language_declared:$language_declared,
        language_exec:$language_exec,
        spec:$spec,
        projection_contract:{
          ok:$projection_contract_ok,
          reason:$projection_contract_reason,
          core_semantics:$core_semantics,
          projection_targets:$projection_targets,
          cross_target_invariants:$cross_target_invariants,
          degradation_policy:$degradation_policy
        },
        fullstack_contract:{
          ok:$fullstack_contract_ok,
          reason:$fullstack_contract_reason,
          constraints:$constraints
        },
        ab:{
          status:$ab_status,
          path_a_ready:$ab_ready_a,
          path_b_ready:$ab_ready_b,
          path_a_failure_reasons:$ab_fail_a,
          path_b_failure_reasons:$ab_fail_b,
          path_a_total_tokens:$ab_tokens_a,
          path_b_total_tokens:$ab_tokens_b,
          out_dir:$ab_out_dir,
          error:$ab_err
        },
        production_loop:{
          status:$prod_status,
          overall_ready:$prod_ready,
          blocked_reason:$prod_blocked_reason,
          gate_evidence_complete:$prod_gate_evidence_complete,
          compile_pass:$prod_compile_pass,
          tests_pass:$prod_tests_pass,
          ab_divergence:$prod_ab_divergence,
          ab_consistency_blocked:$prod_ab_consistency_blocked,
          out_dir:$prod_out_dir,
          error:$prod_err
        }
      }' >> "$RESULTS_JSONL"
  done
done < "$CATALOG"

python3 "$ROOT_DIR/tools/mcp/summarize_project_benchmark_matrix.py" \
  --results "$RESULTS_JSONL" \
  --out "$OUT_DIR/summary.json"

echo "Benchmark matrix output: $OUT_DIR"
cat "$OUT_DIR/summary.json"
