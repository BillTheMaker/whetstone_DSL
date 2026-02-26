#!/usr/bin/env bash
set -euo pipefail

if ! command -v jq >/dev/null 2>&1; then
  echo "error: jq is required" >&2
  exit 1
fi

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
if [[ -z "${WSTONE_MCP_BIN:-}" ]]; then
  if [[ -x "$ROOT_DIR/editor/build-native/whetstone_mcp" ]]; then
    BIN="$ROOT_DIR/editor/build-native/whetstone_mcp"
  else
    BIN="$ROOT_DIR/editor/build-native/whetstone_mcp_stable"
  fi
else
  BIN="$WSTONE_MCP_BIN"
fi
WORKSPACE="${WSTONE_WORKSPACE:-$ROOT_DIR}"
LANGUAGE="${WSTONE_LANGUAGE:-cpp}"
STRICT_EXECUTION_CONTRACT="${WSTONE_STRICT_EXECUTION_CONTRACT:-1}"
CALIBRATE_AFTER_RUN="${WSTONE_CALIBRATE_AFTER_RUN:-0}"
RUN_READINESS_SUITE="${WSTONE_RUN_READINESS_SUITE:-1}"
SPEC_READINESS_PRECHECK="${WSTONE_SPEC_READINESS_PRECHECK:-1}"
SPEC_READINESS_HARD_GATE="${WSTONE_SPEC_READINESS_HARD_GATE:-0}"
SPEC_READINESS_MIN_SCORE="${WSTONE_SPEC_READINESS_MIN_SCORE:-65}"
SEMANTIC_PLANNING_BRIDGE="${WSTONE_SEMANTIC_PLANNING_BRIDGE:-1}"
CAPABILITY_SIGNALS_JSON="${WSTONE_CAPABILITY_SIGNALS_JSON:-}"
if [[ -z "$CAPABILITY_SIGNALS_JSON" ]]; then
  CAPABILITY_SIGNALS_JSON='{}'
fi
INPUT_FILE_ARG="${1:-sprint46_plan.md}"
if [[ "$INPUT_FILE_ARG" = /* ]]; then
  INPUT_FILE="$INPUT_FILE_ARG"
  INPUT_FILE_LABEL="$INPUT_FILE_ARG"
else
  INPUT_FILE="$ROOT_DIR/$INPUT_FILE_ARG"
  INPUT_FILE_LABEL="$INPUT_FILE_ARG"
fi

if [[ ! -x "$BIN" ]]; then
  echo "error: MCP binary not executable: $BIN" >&2
  exit 1
fi
if [[ ! -f "$INPUT_FILE" ]]; then
  echo "error: input file not found: $INPUT_FILE" >&2
  exit 1
fi
if ! printf '%s' "$CAPABILITY_SIGNALS_JSON" | jq -e . >/dev/null 2>&1; then
  echo "error: WSTONE_CAPABILITY_SIGNALS_JSON is not valid JSON" >&2
  exit 1
fi

STAMP="$(date +%Y%m%d_%H%M%S)"
INPUT_BASENAME="$(basename "$INPUT_FILE_LABEL" .md)"
OUT_DIR="$ROOT_DIR/logs/taskitem_runs/${INPUT_BASENAME}_${STAMP}"
mkdir -p "$OUT_DIR"

SPEC_READINESS_JSON='{}'
if [[ "$SPEC_READINESS_PRECHECK" == "1" ]]; then
  python3 "$ROOT_DIR/tools/mcp/spec_planning_readiness.py" \
    --spec "$INPUT_FILE" \
    --out "$OUT_DIR/00a_spec_readiness.json" >/dev/null
  SPEC_READINESS_JSON="$(cat "$OUT_DIR/00a_spec_readiness.json")"
  SPEC_READINESS_SCORE="$(printf '%s' "$SPEC_READINESS_JSON" | jq -r '.scores.total // 0')"
  SPEC_READINESS_VERDICT="$(printf '%s' "$SPEC_READINESS_JSON" | jq -r '.verdict // "unknown"')"
  if [[ "$SPEC_READINESS_HARD_GATE" == "1" ]] && [[ "$SPEC_READINESS_SCORE" -lt "$SPEC_READINESS_MIN_SCORE" ]]; then
    echo "error: spec readiness hard gate failed ($SPEC_READINESS_SCORE < $SPEC_READINESS_MIN_SCORE, verdict=$SPEC_READINESS_VERDICT)" >&2
    echo "error: see $OUT_DIR/00a_spec_readiness.json for missing_actions and recommended_template" >&2
    exit 7
  fi
fi

SEMANTIC_PLANNING_JSON='{}'
if [[ "$SEMANTIC_PLANNING_BRIDGE" == "1" ]]; then
  python3 "$ROOT_DIR/tools/mcp/markdown_to_semantic_annotations.py" \
    --spec "$INPUT_FILE" \
    --out "$OUT_DIR/00b_semantic_planning_annotations.json" >/dev/null
  SEMANTIC_PLANNING_JSON="$(cat "$OUT_DIR/00b_semantic_planning_annotations.json")"
fi

call_tool() {
  local tool_name="$1"
  local args_json="$2"
  local init_req tool_req responses tool_resp
  init_req="$(jq -nc \
    '{jsonrpc:"2.0",id:1,method:"initialize",params:{protocolVersion:"2025-11-25",capabilities:{},clientInfo:{name:"sprint-pipeline",version:"1.0"}}}')"
  tool_req="$(jq -nc --arg t "$tool_name" --argjson a "$args_json" \
    '{jsonrpc:"2.0",id:2,method:"tools/call",params:{name:$t,arguments:$a}}')"

  responses="$(printf '%s\n%s\n' "$init_req" "$tool_req" | "$BIN" --workspace "$WORKSPACE" --language "$LANGUAGE" 2>/dev/null || true)"
  tool_resp="$(printf '%s\n' "$responses" | tail -n1)"

  if [[ -z "$tool_resp" ]]; then
    echo "error: empty response for tool $tool_name" >&2
    exit 2
  fi

  printf '%s' "$tool_resp"
}

extract_tool_text_json() {
  local raw_resp="$1"
  local text
  text="$(printf '%s' "$raw_resp" | jq -r '.result.content[0].text // "{}"')"
  if [[ -z "$text" || "$text" == "null" ]]; then
    echo "{}"
    return 0
  fi
  printf '%s' "$text" | jq '.'
}

build_fallback_intake_spec() {
  local source_file="$1"
  local tmp_steps tmp_constraints tmp_refs tmp_tools
  tmp_steps="$(mktemp)"
  tmp_constraints="$(mktemp)"
  tmp_refs="$(mktemp)"
  tmp_tools="$(mktemp)"

  awk '/^### Step /{sub(/^### /, "", $0); print "- " $0}' "$source_file" > "$tmp_steps"
  awk '
    /^## Architecture Gate/ {in_gate=1; next}
    /^## / && in_gate {in_gate=0}
    in_gate && /^- / {print}
  ' "$source_file" > "$tmp_constraints"
  awk '
    {
      line=$0
      while (match(line, /`[^`]+`/)) {
        tok=substr(line, RSTART+1, RLENGTH-2)
        if (tok ~ /[\/.]/) print "- " tok
        line=substr(line, RSTART+RLENGTH)
      }
    }
  ' "$source_file" | sort -u > "$tmp_refs"
  awk '
    {
      line=tolower($0)
      while (match(line, /whetstone_[a-z0-9_]+/)) {
        tok=substr(line, RSTART, RLENGTH)
        print "- " tok
        line=substr(line, RSTART+RLENGTH)
      }
    }
  ' "$source_file" | sort -u > "$tmp_tools"

  {
    echo "## Goals"
    cat "$tmp_steps"
    echo
    echo "## Constraints"
    if [[ -s "$tmp_constraints" ]]; then
      cat "$tmp_constraints"
    else
      echo "- Follow architecture gate requirements from sprint plan"
      echo "- Keep changes deterministic and test-backed"
    fi
    echo
    echo "## Dependencies"
    echo "- Existing editor/src modules and MCP toolchain"
    echo "- whetstone_mcp stable binary and workspace config"
    if [[ -s "$tmp_refs" ]]; then
      cat "$tmp_refs"
    fi
    if [[ -s "$tmp_tools" ]]; then
      cat "$tmp_tools"
    fi
    echo
    echo "## Acceptance Criteria"
    echo "- All sprint step tests pass"
    echo "- No regression in existing MCP tool behavior"
    echo "- Generated task queue is ready or blockers are explicit"
    if [[ -s "$tmp_steps" ]]; then
      awk '{print "- " $0}' "$tmp_steps"
    fi
  } | sed '/^[[:space:]]*$/N;/^\n$/D'

  rm -f "$tmp_steps" "$tmp_constraints" "$tmp_refs" "$tmp_tools"
}

MARKDOWN_CONTENT="$(cat "$INPUT_FILE")"
INTAKE_ARGS="$(jq -nc --arg md "$MARKDOWN_CONTENT" '{markdown:$md}')"
INTAKE_RESP_RAW="$(call_tool "whetstone_architect_intake" "$INTAKE_ARGS")"
printf '%s\n' "$INTAKE_RESP_RAW" > "$OUT_DIR/01_intake_raw.ndjson.json"
INTAKE_JSON="$(extract_tool_text_json "$INTAKE_RESP_RAW")"
printf '%s\n' "$INTAKE_JSON" > "$OUT_DIR/01_intake.json"
EFFECTIVE_INTAKE_JSON_PATH="$OUT_DIR/01_intake.json"

if [[ "$(printf '%s' "$INTAKE_JSON" | jq -r '.success // false')" != "true" ]] &&
   [[ "$(printf '%s' "$INTAKE_JSON" | jq -r '.error // ""')" == "no_requirements_found" ]]; then
  FALLBACK_SPEC="$(build_fallback_intake_spec "$INPUT_FILE")"
  printf '%s\n' "$FALLBACK_SPEC" > "$OUT_DIR/01a_fallback_intake_spec.md"
  INTAKE_ARGS="$(jq -nc --arg md "$FALLBACK_SPEC" '{markdown:$md}')"
  INTAKE_RESP_RAW="$(call_tool "whetstone_architect_intake" "$INTAKE_ARGS")"
  printf '%s\n' "$INTAKE_RESP_RAW" > "$OUT_DIR/01b_intake_retry_raw.ndjson.json"
  INTAKE_JSON="$(extract_tool_text_json "$INTAKE_RESP_RAW")"
  printf '%s\n' "$INTAKE_JSON" > "$OUT_DIR/01b_intake_retry.json"
  EFFECTIVE_INTAKE_JSON_PATH="$OUT_DIR/01b_intake_retry.json"
fi

if [[ "$(printf '%s' "$INTAKE_JSON" | jq -r '.success // false')" != "true" ]]; then
  echo "error: intake failed, see logs in $OUT_DIR" >&2
  exit 3
fi

NORMALIZED_REQS="$(printf '%s' "$INTAKE_JSON" | jq '.normalizedRequirements')"
CONFLICTS="$(printf '%s' "$INTAKE_JSON" | jq '.conflicts // []')"
GEN_ARGS="$(jq -nc --argjson nr "$NORMALIZED_REQS" --argjson cf "$CONFLICTS" --arg strict "$STRICT_EXECUTION_CONTRACT" \
  '{normalizedRequirements:$nr,conflicts:$cf,strictExecutionContract:($strict == "1")}')"
GEN_RESP_RAW="$(call_tool "whetstone_generate_taskitems" "$GEN_ARGS")"
printf '%s\n' "$GEN_RESP_RAW" > "$OUT_DIR/02_generate_taskitems_raw.ndjson.json"
GEN_JSON="$(extract_tool_text_json "$GEN_RESP_RAW")"
printf '%s\n' "$GEN_JSON" > "$OUT_DIR/02_generate_taskitems.json"
if [[ "$(printf '%s' "$GEN_JSON" | jq -r '.success // false')" != "true" ]]; then
  echo "error: taskitem generation failed, see logs in $OUT_DIR" >&2
  exit 4
fi

TASKS="$(printf '%s' "$GEN_JSON" | jq '.tasks')"
QUEUE_ARGS="$(jq -nc --argjson t "$TASKS" --argjson nr "$NORMALIZED_REQS" --arg strict "$STRICT_EXECUTION_CONTRACT" --argjson cs "$CAPABILITY_SIGNALS_JSON" \
  '{tasks:$t,normalizedRequirements:$nr,strictExecutionContract:($strict == "1"),capabilitySignals:$cs}')"
QUEUE_RESP_RAW="$(call_tool "whetstone_queue_ready" "$QUEUE_ARGS")"
printf '%s\n' "$QUEUE_RESP_RAW" > "$OUT_DIR/03_queue_ready_raw.ndjson.json"
QUEUE_JSON="$(extract_tool_text_json "$QUEUE_RESP_RAW")"
printf '%s\n' "$QUEUE_JSON" > "$OUT_DIR/03_queue_ready.json"

VALIDATE_TASKS="$(printf '%s' "$TASKS" | jq --argjson cs "$CAPABILITY_SIGNALS_JSON" '[.[] | {
  task_id: .taskId,
  title: .title,
  prerequisite_ops: (.prerequisiteOps // []),
  reasons: (.reasons // []),
  confidence: (.confidence // 0),
  dependency_task_ids: (.dependencyTaskIds // []),
  resource_locks: (.resourceLocks // []),
  execution_contract: (.executionContract // {}),
  capability_signals: $cs
}]')"
VALIDATE_ARGS="$(jq -nc --argjson items "$VALIDATE_TASKS" --arg ws "$WORKSPACE" --arg strict "$STRICT_EXECUTION_CONTRACT" \
  '{taskitems:$items,workspace:$ws,strict_execution_contract:($strict == "1")}')"
VALIDATE_RESP_RAW="$(call_tool "whetstone_validate_taskitem" "$VALIDATE_ARGS")"
printf '%s\n' "$VALIDATE_RESP_RAW" > "$OUT_DIR/04_validate_taskitem_raw.ndjson.json"
VALIDATE_JSON="$(extract_tool_text_json "$VALIDATE_RESP_RAW")"
printf '%s\n' "$VALIDATE_JSON" > "$OUT_DIR/04_validate_taskitem.json"

SUMMARY_JSON="$(jq -nc \
  --arg input_file "$INPUT_FILE_LABEL" \
  --arg outDir "$OUT_DIR" \
  --arg bin "$BIN" \
  --arg workspace "$WORKSPACE" \
  --argjson planning_readiness "$SPEC_READINESS_JSON" \
  --argjson semantic_planning "$SEMANTIC_PLANNING_JSON" \
  --argjson intake "$INTAKE_JSON" \
  --argjson generated "$GEN_JSON" \
  --argjson queue "$QUEUE_JSON" \
  --argjson validate "$VALIDATE_JSON" \
  '{
    sprint: $input_file,
    input_file: $input_file,
    timestamp: now|todate,
    mcp_binary: $bin,
    workspace: $workspace,
    output_dir: $outDir,
    planning_readiness: $planning_readiness,
    semantic_planning_annotations: $semantic_planning,
    intake: {
      success: ($intake.success // false),
      normalized_requirement_count: (($intake.normalizedRequirements // [])|length),
      conflict_count: (($intake.conflicts // [])|length),
      ambiguous_requirement_count: ($intake.conflictSignals.ambiguousRequirementCount // 0)
    },
    taskitems: {
      success: ($generated.success // false),
      task_count: (($generated.tasks // [])|length),
      escalate_count: ($generated.escalateCount // 0),
      strict_execution_contract: ($generated.strictExecutionContract // false),
      missing_execution_contract_count: ($generated.missingExecutionContractCount // 0)
    },
    queue_ready: {
      success: ($queue.success // false),
      ready: ($queue.ready // false),
      ready_count: ($queue.readyCount // 0),
      blocker_count: (($queue.blockers // [])|length),
      queue_warnings: ($queue.queueWarnings // []),
      resource_conflict_count: ($queue.resourceConflictCount // 0),
      strict_execution_contract: ($queue.strictExecutionContract // false),
      execution_contract_missing_count: ($queue.executionContractMissingCount // 0),
      execution_specificity_low_count: ($queue.executionSpecificityLowCount // 0),
      gap_class_counts: ($queue.gap_class_counts // {}),
      remediation_routing_hints: ($queue.remediationRoutingHints // {}),
      debug_loop_call_plan: ($queue.debugLoopCallPlan // {})
    },
    validation: {
      success: ($validate.success // false),
      total_taskitems: ($validate.report.total_taskitems // 0),
      average_score: ($validate.report.average_score // 0),
      average_execution_specificity_score: ($validate.report.average_execution_specificity_score // 0),
      self_contained_count: ($validate.report.self_contained_count // 0),
      failing_count: ($validate.report.failing_count // 0),
      gap_class_counts: ($validate.report.gap_class_counts // {}),
      top_recommended_tools: ($validate.report.top_recommended_tools // []),
      capability_gap_call_plan: ($validate.report.capability_gap_call_plan // {}),
      promotion_packet: ($validate.report.promotion_packet // {})
    }
  }')"

if [[ "$CALIBRATE_AFTER_RUN" == "1" ]]; then
  CALIBRATION_OUT_DIR="$OUT_DIR/calibration"
  CALIBRATION_JSON="$(python3 "$ROOT_DIR/tools/mcp/analyze_taskitem_calibration.py" \
    --runs-root "$ROOT_DIR/logs/taskitem_runs" \
    --out-dir "$CALIBRATION_OUT_DIR")"
  if [[ -n "$CALIBRATION_JSON" ]] && printf '%s' "$CALIBRATION_JSON" | jq -e . >/dev/null 2>&1; then
    SUMMARY_JSON="$(printf '%s' "$SUMMARY_JSON" | jq --argjson calibration "$CALIBRATION_JSON" '.calibration = $calibration')"
  fi
fi

# Persist current summary before optional post-check suites that read summary fields.
printf '%s\n' "$SUMMARY_JSON" > "$OUT_DIR/00_summary.json"

if [[ "$RUN_READINESS_SUITE" == "1" ]]; then
  READINESS_JSON="$(python3 "$ROOT_DIR/tools/mcp/run_generator_readiness_suite.py" \
    --repo-root "$ROOT_DIR" \
    --out-dir "$OUT_DIR" \
    --summary-json "$OUT_DIR/00_summary.json" \
    --intake-json "$EFFECTIVE_INTAKE_JSON_PATH" \
    --generated-json "$OUT_DIR/02_generate_taskitems.json" \
    --queue-json "$OUT_DIR/03_queue_ready.json" \
    --validate-json "$OUT_DIR/04_validate_taskitem.json" \
    --input-file "$INPUT_FILE")"
  if [[ -n "$READINESS_JSON" ]] && printf '%s' "$READINESS_JSON" | jq -e . >/dev/null 2>&1; then
    SUMMARY_JSON="$(printf '%s' "$SUMMARY_JSON" | jq --argjson readiness "$READINESS_JSON" '.readiness = $readiness')"
  fi
fi

printf '%s\n' "$SUMMARY_JSON" > "$OUT_DIR/00_summary.json"

echo "Taskitem pipeline complete."
echo "Output dir: $OUT_DIR"
echo "$SUMMARY_JSON" | jq .
