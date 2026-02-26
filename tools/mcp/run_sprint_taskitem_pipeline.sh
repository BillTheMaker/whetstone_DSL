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
SEMANTIC_INTAKE_AUGMENT="${WSTONE_SEMANTIC_INTAKE_AUGMENT:-1}"
SEMANTIC_REQUIREMENT_INJECTION="${WSTONE_SEMANTIC_REQUIREMENT_INJECTION:-1}"
SEMANTIC_TASK_EXPANSION="${WSTONE_SEMANTIC_TASK_EXPANSION:-1}"
SEMANTIC_TASK_EXPANSION_MODE="${WSTONE_SEMANTIC_TASK_EXPANSION_MODE:-fallback_only}"
SEMANTIC_COVERAGE_GATE="${WSTONE_SEMANTIC_COVERAGE_GATE:-0}"
SEMANTIC_MIN_COVERAGE="${WSTONE_SEMANTIC_MIN_COVERAGE:-0.9}"
SEMANTIC_REQUIRE_CAPS_FOR_COMPLEX="${WSTONE_SEMANTIC_REQUIRE_CAPS_FOR_COMPLEX:-1}"
SEMANTIC_MIN_COMPLEXITY_SCORE="${WSTONE_SEMANTIC_MIN_COMPLEXITY_SCORE:-2}"
NATIVE_DECOMP_HARD_GATE="${WSTONE_NATIVE_DECOMP_HARD_GATE:-0}"
NATIVE_TASK_MIN_COUNT="${WSTONE_NATIVE_TASK_MIN_COUNT:-0}"
NATIVE_SEMANTIC_SIGNAL_MIN="${WSTONE_NATIVE_SEMANTIC_SIGNAL_MIN:-0}"
NATIVE_REASON_ENRICHMENT="${WSTONE_NATIVE_REASON_ENRICHMENT:-1}"
NATIVE_DECOMP_RETRY="${WSTONE_NATIVE_DECOMP_RETRY:-1}"
NATIVE_DECOMP_TARGET_MIN_TASKS="${WSTONE_NATIVE_DECOMP_TARGET_MIN_TASKS:-5}"
NATIVE_IMPACT_COVERAGE_GATE="${WSTONE_NATIVE_IMPACT_COVERAGE_GATE:-0}"
NATIVE_IMPACT_COVERAGE_ENFORCE="${WSTONE_NATIVE_IMPACT_COVERAGE_ENFORCE:-0}"
NATIVE_IMPACT_COVERAGE_PROFILES="${WSTONE_NATIVE_IMPACT_COVERAGE_PROFILES:-$ROOT_DIR/tools/mcp/profiles/native_decomposition_impact_profiles.json}"
NATIVE_PROFILE_AUTOFILL="${WSTONE_NATIVE_PROFILE_AUTOFILL:-0}"
NATIVE_PROFILE_AUTOFILL_MAX_TASKS="${WSTONE_NATIVE_PROFILE_AUTOFILL_MAX_TASKS:-12}"
NATIVE_INTRINSIC_BOOST="${WSTONE_NATIVE_INTRINSIC_BOOST:-0}"
NATIVE_SINGLESHOT_PROFILE_SHAPE="${WSTONE_NATIVE_SINGLESHOT_PROFILE_SHAPE:-0}"
NATIVE_MULTISHOT_DECOMP="${WSTONE_NATIVE_MULTISHOT_DECOMP:-0}"
NATIVE_MULTISHOT_MAX_PROFILES="${WSTONE_NATIVE_MULTISHOT_MAX_PROFILES:-12}"
NATIVE_MULTISHOT_APPLY_MODE="${WSTONE_NATIVE_MULTISHOT_APPLY_MODE:-if_improves}"
NATIVE_RAW_CANDIDATE_SEARCH="${WSTONE_NATIVE_RAW_CANDIDATE_SEARCH:-0}"
NATIVE_RAW_CANDIDATE_MAX_VARIANTS="${WSTONE_NATIVE_RAW_CANDIDATE_MAX_VARIANTS:-8}"
NATIVE_RAW_CANDIDATE_REQUIRE_UPLIFT="${WSTONE_NATIVE_RAW_CANDIDATE_REQUIRE_UPLIFT:-0}"
NATIVE_RAW_HARDEN_TOP_GAPS="${WSTONE_NATIVE_RAW_HARDEN_TOP_GAPS:-0}"
NATIVE_RAW_TOP_GAP_WEIGHTED_SELECT="${WSTONE_NATIVE_RAW_TOP_GAP_WEIGHTED_SELECT:-0}"
NATIVE_RAW_TOP_GAP_BACKLOG_FILE="${WSTONE_NATIVE_RAW_TOP_GAP_BACKLOG_FILE:-}"
NATIVE_RAW_TOP_GAP_REQUIRE_UPLIFT="${WSTONE_NATIVE_RAW_TOP_GAP_REQUIRE_UPLIFT:-0}"
NATIVE_RAW_TOP_GAP_REQUIREMENTS="${WSTONE_NATIVE_RAW_TOP_GAP_REQUIREMENTS:-0}"
NATIVE_RAW_TOP_GAP_MAX_SIGNALS="${WSTONE_NATIVE_RAW_TOP_GAP_MAX_SIGNALS:-5}"
EXTRA_NORMALIZED_REQUIREMENTS_FILE="${WSTONE_EXTRA_NORMALIZED_REQUIREMENTS_FILE:-}"
EXTRA_TASKS_FILE="${WSTONE_EXTRA_TASKS_FILE:-}"
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
SEMANTIC_AUGMENT_JSON='{}'
SEMANTIC_REQUIREMENT_INJECTION_JSON='{}'
SEMANTIC_TASK_EXPANSION_JSON='{}'
SEMANTIC_GATE_JSON='{}'
NATIVE_DECOMP_GATE_JSON='{}'
NATIVE_REASON_ENRICHMENT_JSON='{}'
NATIVE_DECOMP_RETRY_JSON='{}'
NATIVE_IMPACT_COVERAGE_JSON='{}'
NATIVE_PROFILE_AUTOFILL_JSON='{}'
NATIVE_INTRINSIC_BOOST_JSON='{}'
NATIVE_SINGLESHOT_PROFILE_SHAPE_JSON='{}'
NATIVE_MULTISHOT_JSON='{}'
NATIVE_RAW_CANDIDATE_SEARCH_JSON='{}'
NATIVE_RAW_HARDENING_JSON='{}'
NATIVE_RAW_TOP_GAP_REQUIREMENTS_JSON='{}'
INTRINSIC_REQS_JSON='[]'
EXTRA_NORMALIZED_REQUIREMENTS_JSON='[]'
EXTRA_TASKS_JSON='[]'
if [[ "$SEMANTIC_PLANNING_BRIDGE" == "1" ]]; then
  python3 "$ROOT_DIR/tools/mcp/markdown_to_semantic_annotations.py" \
    --spec "$INPUT_FILE" \
    --out "$OUT_DIR/00b_semantic_planning_annotations.json" >/dev/null
  SEMANTIC_PLANNING_JSON="$(cat "$OUT_DIR/00b_semantic_planning_annotations.json")"
  if [[ "$SEMANTIC_COVERAGE_GATE" == "1" ]]; then
    gate_flags=()
    if [[ "$SEMANTIC_REQUIRE_CAPS_FOR_COMPLEX" == "1" ]]; then
      gate_flags+=(--require-caps-for-complex)
    fi
    if python3 "$ROOT_DIR/tools/mcp/check_semantic_planning_gate.py" \
      --packet "$OUT_DIR/00b_semantic_planning_annotations.json" \
      --out "$OUT_DIR/00bb_semantic_planning_gate.json" \
      --min-coverage "$SEMANTIC_MIN_COVERAGE" \
      --min-complexity-score "$SEMANTIC_MIN_COMPLEXITY_SCORE" \
      "${gate_flags[@]}" >/dev/null; then
      SEMANTIC_GATE_JSON="$(cat "$OUT_DIR/00bb_semantic_planning_gate.json")"
    else
      SEMANTIC_GATE_JSON="$(cat "$OUT_DIR/00bb_semantic_planning_gate.json")"
      echo "error: semantic planning gate failed" >&2
      echo "error: see $OUT_DIR/00bb_semantic_planning_gate.json" >&2
      exit 8
    fi
  fi
  if [[ "$SEMANTIC_INTAKE_AUGMENT" == "1" ]]; then
    python3 "$ROOT_DIR/tools/mcp/augment_spec_with_semantic_packet.py" \
      --spec "$INPUT_FILE" \
      --packet "$OUT_DIR/00b_semantic_planning_annotations.json" \
      --out "$OUT_DIR/00c_semantic_augmented_spec.md" > "$OUT_DIR/00c_semantic_augmented_spec_report.json"
    SEMANTIC_AUGMENT_JSON="$(cat "$OUT_DIR/00c_semantic_augmented_spec_report.json")"
  fi
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

if [[ "$SEMANTIC_PLANNING_BRIDGE" == "1" && "$SEMANTIC_INTAKE_AUGMENT" == "1" && -f "$OUT_DIR/00c_semantic_augmented_spec.md" ]]; then
  MARKDOWN_CONTENT="$(cat "$OUT_DIR/00c_semantic_augmented_spec.md")"
else
  MARKDOWN_CONTENT="$(cat "$INPUT_FILE")"
fi
INTAKE_ARGS="$(jq -nc --arg md "$MARKDOWN_CONTENT" '{markdown:$md}')"
INTAKE_RESP_RAW="$(call_tool "whetstone_architect_intake" "$INTAKE_ARGS")"
printf '%s\n' "$INTAKE_RESP_RAW" > "$OUT_DIR/01_intake_raw.ndjson.json"
INTAKE_JSON="$(extract_tool_text_json "$INTAKE_RESP_RAW")"
printf '%s\n' "$INTAKE_JSON" > "$OUT_DIR/01_intake.json"
EFFECTIVE_INTAKE_JSON_PATH="$OUT_DIR/01_intake.json"

if [[ "$(printf '%s' "$INTAKE_JSON" | jq -r '.success // false')" != "true" ]] &&
   [[ "$(printf '%s' "$INTAKE_JSON" | jq -r '.error // ""')" == "no_requirements_found" ]]; then
  FALLBACK_SOURCE_FILE="$INPUT_FILE"
  if [[ "$SEMANTIC_PLANNING_BRIDGE" == "1" && "$SEMANTIC_INTAKE_AUGMENT" == "1" && -f "$OUT_DIR/00c_semantic_augmented_spec.md" ]]; then
    FALLBACK_SOURCE_FILE="$OUT_DIR/00c_semantic_augmented_spec.md"
  fi
  FALLBACK_SPEC="$(build_fallback_intake_spec "$FALLBACK_SOURCE_FILE")"
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
if [[ "$SEMANTIC_PLANNING_BRIDGE" == "1" && "$SEMANTIC_REQUIREMENT_INJECTION" == "1" && -f "$OUT_DIR/00b_semantic_planning_annotations.json" ]]; then
  SEM_REQS="$(jq -nc --argjson packet "$(cat "$OUT_DIR/00b_semantic_planning_annotations.json")" '
    def mk($kind; $text; $id):
      {requirementId:$id, kind:$kind, normalizedText:$text, anchor:"semantic_packet", sourceLine:0, ambiguous:false};
    [
      (if ($packet.annotations[]? | select(.type=="intent") | .fields.summary // "" | length) > 0
       then mk("goal"; ($packet.annotations[] | select(.type=="intent") | .fields.summary); "semantic-intent-summary")
       else empty end),
      (($packet.annotations[]? | select(.type=="complexity") | .fields.markers[]?) as $m
        | mk("constraint"; ("semantic complexity marker: " + $m); ("semantic-complexity-" + ($m|gsub("[^A-Za-z0-9]+";"_"))))),
      (($packet.annotations[]? | select(.type=="risk") | .fields.risks[]?) as $r
        | mk("constraint"; ("semantic risk guardrail: " + $r); ("semantic-risk-" + ($r|gsub("[^A-Za-z0-9]+";"_"))))),
      (($packet.annotations[]? | select(.type=="contract") | .fields.constraints[]?) as $c
        | mk("constraint"; ("semantic contract clause: " + $c); ("semantic-contract-" + ($c|gsub("[^A-Za-z0-9]+";"_"))))),
      (($packet.annotations[]? | select(.type=="capability_requirement") | .fields.capability?) as $cap
        | mk("dependency"; ("semantic capability requirement: " + $cap); ("semantic-capability-" + ($cap|gsub("[^A-Za-z0-9]+";"_")))))
    ] | map(select(.normalizedText | length > 0))
  ')"
  printf '%s\n' "$SEM_REQS" > "$OUT_DIR/01c_semantic_injected_requirements.json"
  NORMALIZED_REQS="$(jq -nc --argjson base "$NORMALIZED_REQS" --argjson sem "$SEM_REQS" '$base + $sem')"
  SEMANTIC_REQUIREMENT_INJECTION_JSON="$(jq -nc --argjson sem "$SEM_REQS" '{injected_count: ($sem|length), injected_requirements:$sem}')"
fi
if [[ -n "$EXTRA_NORMALIZED_REQUIREMENTS_FILE" ]]; then
  if [[ ! -f "$EXTRA_NORMALIZED_REQUIREMENTS_FILE" ]]; then
    echo "error: extra normalized requirements file not found: $EXTRA_NORMALIZED_REQUIREMENTS_FILE" >&2
    exit 13
  fi
  if ! jq -e 'type == "array"' "$EXTRA_NORMALIZED_REQUIREMENTS_FILE" >/dev/null 2>&1; then
    echo "error: extra normalized requirements file must be a JSON array: $EXTRA_NORMALIZED_REQUIREMENTS_FILE" >&2
    exit 14
  fi
  EXTRA_NORMALIZED_REQUIREMENTS_JSON="$(cat "$EXTRA_NORMALIZED_REQUIREMENTS_FILE")"
  NORMALIZED_REQS="$(jq -nc --argjson base "$NORMALIZED_REQS" --argjson extra "$EXTRA_NORMALIZED_REQUIREMENTS_JSON" '$base + $extra')"
fi
if [[ "$NATIVE_INTRINSIC_BOOST" == "1" ]]; then
  python3 "$ROOT_DIR/tools/mcp/synthesize_native_intrinsic_boost_requirements.py" \
    --spec "$INPUT_FILE" \
    --profiles "$NATIVE_IMPACT_COVERAGE_PROFILES" \
    --out "$OUT_DIR/01d_native_intrinsic_boost_requirements.json" >/dev/null
  INTRINSIC_REQS_JSON="$(cat "$OUT_DIR/01d_native_intrinsic_boost_requirements.json")"
  NORMALIZED_REQS="$(jq -nc --argjson base "$NORMALIZED_REQS" --argjson intrinsic "$INTRINSIC_REQS_JSON" '$base + $intrinsic')"
  NATIVE_INTRINSIC_BOOST_JSON="$(jq -nc --argjson reqs "$INTRINSIC_REQS_JSON" '{enabled:true, requirement_count:($reqs|length)}')"
else
  NATIVE_INTRINSIC_BOOST_JSON='{"enabled":false,"requirement_count":0}'
fi
if [[ "$NATIVE_RAW_TOP_GAP_REQUIREMENTS" == "1" && -n "$NATIVE_RAW_TOP_GAP_BACKLOG_FILE" && -f "$NATIVE_RAW_TOP_GAP_BACKLOG_FILE" ]]; then
  python3 "$ROOT_DIR/tools/mcp/synthesize_raw_top_gap_requirements.py" \
    --top-gaps "$NATIVE_RAW_TOP_GAP_BACKLOG_FILE" \
    --max-signals "$NATIVE_RAW_TOP_GAP_MAX_SIGNALS" \
    --out "$OUT_DIR/01e_raw_top_gap_requirements.json" \
    --out-report "$OUT_DIR/01e_raw_top_gap_requirements_report.json" >/dev/null
  RAW_TOP_GAP_REQS_JSON="$(cat "$OUT_DIR/01e_raw_top_gap_requirements.json")"
  NORMALIZED_REQS="$(jq -nc --argjson base "$NORMALIZED_REQS" --argjson extra "$RAW_TOP_GAP_REQS_JSON" '$base + $extra')"
  NATIVE_RAW_TOP_GAP_REQUIREMENTS_JSON="$(jq -nc \
    --argjson enabled true \
    --arg backlog_file "$NATIVE_RAW_TOP_GAP_BACKLOG_FILE" \
    --argjson report "$(cat "$OUT_DIR/01e_raw_top_gap_requirements_report.json")" \
    '{enabled:$enabled, backlog_file:$backlog_file, report:$report}')"
else
  NATIVE_RAW_TOP_GAP_REQUIREMENTS_JSON='{"enabled":false}'
fi
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
native_task_count_initial="$(printf '%s' "$TASKS" | jq 'length')"
if [[ "$NATIVE_DECOMP_RETRY" == "1" && "$native_task_count_initial" -lt "$NATIVE_DECOMP_TARGET_MIN_TASKS" ]]; then
  RETRY_REQS="$(jq -nc --argjson base "$NORMALIZED_REQS" --arg min "$NATIVE_DECOMP_TARGET_MIN_TASKS" '
    $base + [{
      requirementId: "native-decomp-min-task-count",
      kind: "constraint",
      normalizedText: ("native decomposition policy: produce at least " + $min + " concrete taskitems with deterministic execution contracts"),
      anchor: "pipeline_native_retry",
      sourceLine: 0,
      ambiguous: false
    }]
  ')"
  RETRY_GEN_ARGS="$(jq -nc --argjson nr "$RETRY_REQS" --argjson cf "$CONFLICTS" --arg strict "$STRICT_EXECUTION_CONTRACT" \
    '{normalizedRequirements:$nr,conflicts:$cf,strictExecutionContract:($strict == "1")}')"
  RETRY_GEN_RESP_RAW="$(call_tool "whetstone_generate_taskitems" "$RETRY_GEN_ARGS")"
  printf '%s\n' "$RETRY_GEN_RESP_RAW" > "$OUT_DIR/02aa_generate_taskitems_retry_raw.ndjson.json"
  RETRY_GEN_JSON="$(extract_tool_text_json "$RETRY_GEN_RESP_RAW")"
  printf '%s\n' "$RETRY_GEN_JSON" > "$OUT_DIR/02aa_generate_taskitems_retry.json"
  retry_success="$(printf '%s' "$RETRY_GEN_JSON" | jq -r '.success // false')"
  retry_task_count="$(printf '%s' "$RETRY_GEN_JSON" | jq '.tasks | length')"
  if [[ "$retry_success" == "true" && "$retry_task_count" -gt "$native_task_count_initial" ]]; then
    GEN_JSON="$RETRY_GEN_JSON"
    TASKS="$(printf '%s' "$GEN_JSON" | jq '.tasks')"
    NATIVE_DECOMP_RETRY_JSON="$(jq -nc \
      --argjson attempted true \
      --argjson applied true \
      --argjson initial_task_count "$native_task_count_initial" \
      --argjson retry_task_count "$retry_task_count" \
      --argjson target_min_task_count "$NATIVE_DECOMP_TARGET_MIN_TASKS" \
      '{attempted:$attempted, applied:$applied, initial_task_count:$initial_task_count, retry_task_count:$retry_task_count, target_min_task_count:$target_min_task_count}')"
  else
    NATIVE_DECOMP_RETRY_JSON="$(jq -nc \
      --argjson attempted true \
      --argjson applied false \
      --argjson initial_task_count "$native_task_count_initial" \
      --argjson retry_task_count "$retry_task_count" \
      --argjson target_min_task_count "$NATIVE_DECOMP_TARGET_MIN_TASKS" \
      '{attempted:$attempted, applied:$applied, initial_task_count:$initial_task_count, retry_task_count:$retry_task_count, target_min_task_count:$target_min_task_count}')"
  fi
else
  NATIVE_DECOMP_RETRY_JSON="$(jq -nc \
    --argjson attempted false \
    --argjson applied false \
    --argjson initial_task_count "$native_task_count_initial" \
    --argjson target_min_task_count "$NATIVE_DECOMP_TARGET_MIN_TASKS" \
    '{attempted:$attempted, applied:$applied, initial_task_count:$initial_task_count, target_min_task_count:$target_min_task_count}')"
fi
if [[ "$NATIVE_RAW_CANDIDATE_SEARCH" == "1" ]]; then
  top_gap_score_args=()
  use_top_gap_weighted_select=false
  if [[ "$NATIVE_RAW_TOP_GAP_WEIGHTED_SELECT" == "1" && -n "$NATIVE_RAW_TOP_GAP_BACKLOG_FILE" && -f "$NATIVE_RAW_TOP_GAP_BACKLOG_FILE" ]]; then
    top_gap_score_args=(--top-gaps "$NATIVE_RAW_TOP_GAP_BACKLOG_FILE")
    use_top_gap_weighted_select=true
  fi
  python3 "$ROOT_DIR/tools/mcp/synthesize_raw_candidate_requirement_variants.py" \
    --spec "$INPUT_FILE" \
    --profiles "$NATIVE_IMPACT_COVERAGE_PROFILES" \
    --max-variants "$NATIVE_RAW_CANDIDATE_MAX_VARIANTS" \
    --out "$OUT_DIR/02ae_raw_candidate_variants.json" >/dev/null
  VARIANTS_JSON="$(cat "$OUT_DIR/02ae_raw_candidate_variants.json")"

  printf '%s\n' "$TASKS" > "$OUT_DIR/02ae_candidate_0_tasks.json"
  python3 "$ROOT_DIR/tools/mcp/score_native_tasks_profile_coverage.py" \
    --spec "$INPUT_FILE" \
    --profiles "$NATIVE_IMPACT_COVERAGE_PROFILES" \
    --tasks "$OUT_DIR/02ae_candidate_0_tasks.json" \
    "${top_gap_score_args[@]}" \
    --out "$OUT_DIR/02ae_candidate_0_score.json" >/dev/null
  best_variant="0"
  best_fail="$(jq '.failing_profile_count // 999' "$OUT_DIR/02ae_candidate_0_score.json")"
  best_task_count="$(jq '.task_count // 0' "$OUT_DIR/02ae_candidate_0_score.json")"
  best_top_gap_score="$(jq '.top_gap_score // 0' "$OUT_DIR/02ae_candidate_0_score.json")"
  baseline_fail="$best_fail"
  baseline_task_count="$best_task_count"
  baseline_top_gap_score="$best_top_gap_score"
  attempted=1
  successful=1

  variant_total="$(printf '%s' "$VARIANTS_JSON" | jq '.variants | length')"
  max_variants="$NATIVE_RAW_CANDIDATE_MAX_VARIANTS"
  idx=0
  variant=1
  while [[ "$variant" -lt "$max_variants" && "$idx" -lt "$variant_total" ]]; do
    req_bundle="$(printf '%s' "$VARIANTS_JSON" | jq ".variants[$idx]")"
    cand_reqs="$(jq -nc --argjson base "$NORMALIZED_REQS" --argjson bundle "$req_bundle" '$base + $bundle')"
    cand_args="$(jq -nc --argjson nr "$cand_reqs" --argjson cf "$CONFLICTS" --arg strict "$STRICT_EXECUTION_CONTRACT" \
      '{normalizedRequirements:$nr,conflicts:$cf,strictExecutionContract:($strict == "1")}')"
    cand_raw="$(call_tool "whetstone_generate_taskitems" "$cand_args")"
    printf '%s\n' "$cand_raw" > "$OUT_DIR/02ae_candidate_${variant}_raw.ndjson.json"
    cand_json="$(extract_tool_text_json "$cand_raw")"
    printf '%s\n' "$cand_json" > "$OUT_DIR/02ae_candidate_${variant}.json"
    attempted=$((attempted + 1))
    if [[ "$(printf '%s' "$cand_json" | jq -r '.success // false')" == "true" ]]; then
      cand_tasks="$(printf '%s' "$cand_json" | jq '.tasks // []')"
      printf '%s\n' "$cand_tasks" > "$OUT_DIR/02ae_candidate_${variant}_tasks.json"
      python3 "$ROOT_DIR/tools/mcp/score_native_tasks_profile_coverage.py" \
        --spec "$INPUT_FILE" \
        --profiles "$NATIVE_IMPACT_COVERAGE_PROFILES" \
        --tasks "$OUT_DIR/02ae_candidate_${variant}_tasks.json" \
        "${top_gap_score_args[@]}" \
        --out "$OUT_DIR/02ae_candidate_${variant}_score.json" >/dev/null
      cand_fail="$(jq '.failing_profile_count // 999' "$OUT_DIR/02ae_candidate_${variant}_score.json")"
      cand_task_count="$(jq '.task_count // 0' "$OUT_DIR/02ae_candidate_${variant}_score.json")"
      cand_top_gap_score="$(jq '.top_gap_score // 0' "$OUT_DIR/02ae_candidate_${variant}_score.json")"
      should_select=false
      if [[ "$cand_fail" -lt "$best_fail" ]]; then
        should_select=true
      elif [[ "$cand_fail" -eq "$best_fail" ]]; then
        if [[ "$use_top_gap_weighted_select" == true && "$cand_top_gap_score" -gt "$best_top_gap_score" ]]; then
          should_select=true
        elif [[ "$cand_top_gap_score" -eq "$best_top_gap_score" && "$cand_task_count" -gt "$best_task_count" ]]; then
          should_select=true
        elif [[ "$use_top_gap_weighted_select" != true && "$cand_task_count" -gt "$best_task_count" ]]; then
          should_select=true
        fi
      fi
      if [[ "$should_select" == true ]]; then
        best_variant="$variant"
        best_fail="$cand_fail"
        best_task_count="$cand_task_count"
        best_top_gap_score="$cand_top_gap_score"
        TASKS="$cand_tasks"
      fi
      successful=$((successful + 1))
    fi
    idx=$((idx + 1))
    variant=$((variant + 1))
  done

  NATIVE_RAW_CANDIDATE_SEARCH_JSON="$(jq -nc \
    --argjson enabled true \
    --argjson attempted "$attempted" \
    --argjson successful "$successful" \
    --arg best_variant "$best_variant" \
    --argjson baseline_failing_profile_count "$baseline_fail" \
    --argjson baseline_task_count "$baseline_task_count" \
    --argjson baseline_top_gap_score "$baseline_top_gap_score" \
    --argjson best_failing_profile_count "$best_fail" \
    --argjson best_task_count "$best_task_count" \
    --argjson best_top_gap_score "$best_top_gap_score" \
    --argjson available_variants "$variant_total" \
    --argjson top_gap_weighted_select "$use_top_gap_weighted_select" \
    --arg top_gap_backlog_file "$NATIVE_RAW_TOP_GAP_BACKLOG_FILE" \
    --argjson top_gap_require_uplift "$NATIVE_RAW_TOP_GAP_REQUIRE_UPLIFT" \
    '{enabled:$enabled, attempted_variants:$attempted, successful_variants:$successful, available_variants:$available_variants, selected_variant:$best_variant, baseline_failing_profile_count:$baseline_failing_profile_count, baseline_task_count:$baseline_task_count, baseline_top_gap_score:$baseline_top_gap_score, best_failing_profile_count:$best_failing_profile_count, best_task_count:$best_task_count, best_top_gap_score:$best_top_gap_score, top_gap_weighted_select:$top_gap_weighted_select, top_gap_backlog_file:$top_gap_backlog_file, top_gap_require_uplift:$top_gap_require_uplift}')"
  printf '%s\n' "$NATIVE_RAW_CANDIDATE_SEARCH_JSON" > "$OUT_DIR/02ae_raw_candidate_search.json"
  if [[ "$NATIVE_RAW_CANDIDATE_REQUIRE_UPLIFT" == "1" && "$best_fail" -ge "$baseline_fail" ]]; then
    echo "error: raw candidate search did not improve failing profile count" >&2
    echo "error: see $OUT_DIR/02ae_raw_candidate_search.json" >&2
    exit 17
  fi
  if [[ "$NATIVE_RAW_TOP_GAP_REQUIRE_UPLIFT" == "1" ]]; then
    if [[ "$use_top_gap_weighted_select" != true ]]; then
      echo "error: raw top-gap uplift requires weighted select with a valid backlog file" >&2
      echo "error: set WSTONE_NATIVE_RAW_TOP_GAP_WEIGHTED_SELECT=1 and WSTONE_NATIVE_RAW_TOP_GAP_BACKLOG_FILE=<path>" >&2
      exit 18
    fi
    if [[ "$best_top_gap_score" -le "$baseline_top_gap_score" ]]; then
      echo "error: raw candidate search did not improve top-gap score" >&2
      echo "error: see $OUT_DIR/02ae_raw_candidate_search.json" >&2
      exit 18
    fi
  fi
else
  NATIVE_RAW_CANDIDATE_SEARCH_JSON='{"enabled":false}'
fi
if [[ "$NATIVE_RAW_HARDEN_TOP_GAPS" == "1" ]]; then
  printf '%s\n' "$TASKS" > "$OUT_DIR/02af_raw_hardening_input_tasks.json"
  python3 "$ROOT_DIR/tools/mcp/harden_native_tasks_top_gaps.py" \
    --spec "$INPUT_FILE" \
    --profiles "$NATIVE_IMPACT_COVERAGE_PROFILES" \
    --tasks "$OUT_DIR/02af_raw_hardening_input_tasks.json" \
    --out "$OUT_DIR/02af_raw_hardening_output_tasks.json" \
    --out-report "$OUT_DIR/02af_raw_hardening_report.json" >/dev/null
  hardened_tasks_json="$(cat "$OUT_DIR/02af_raw_hardening_output_tasks.json")"
  input_count_h="$(printf '%s' "$TASKS" | jq 'length')"
  output_count_h="$(printf '%s' "$hardened_tasks_json" | jq 'length')"
  if [[ "$output_count_h" -gt "$input_count_h" ]]; then
    added_h_tasks="$(jq -nc --argjson shaped "$hardened_tasks_json" --argjson n "$input_count_h" '$shaped[$n:]')"
    EXTRA_TASKS_JSON="$(jq -nc --argjson base "$EXTRA_TASKS_JSON" --argjson extra "$added_h_tasks" '$base + $extra')"
  fi
  TASKS="$hardened_tasks_json"
  NATIVE_RAW_HARDENING_JSON="$(jq -nc \
    --argjson rep "$(cat "$OUT_DIR/02af_raw_hardening_report.json")" \
    --argjson input_count "$input_count_h" \
    --argjson output_count "$output_count_h" \
    '{enabled:true, input_task_count:$input_count, output_task_count:$output_count, report:$rep}')"
else
  NATIVE_RAW_HARDENING_JSON='{"enabled":false}'
fi
if [[ "$NATIVE_SINGLESHOT_PROFILE_SHAPE" == "1" ]]; then
  printf '%s\n' "$TASKS" > "$OUT_DIR/02ad_single_shot_base_tasks.json"
  python3 "$ROOT_DIR/tools/mcp/synthesize_single_shot_profile_shape_tasks.py" \
    --spec "$INPUT_FILE" \
    --profiles "$NATIVE_IMPACT_COVERAGE_PROFILES" \
    --tasks "$OUT_DIR/02ad_single_shot_base_tasks.json" \
    --out "$OUT_DIR/02ad_single_shot_shaped_tasks.json" \
    --out-report "$OUT_DIR/02ad_single_shot_shape_report.json" >/dev/null
  shaped_tasks_json="$(cat "$OUT_DIR/02ad_single_shot_shaped_tasks.json")"
  base_count_ss="$(printf '%s' "$TASKS" | jq 'length')"
  shaped_count_ss="$(printf '%s' "$shaped_tasks_json" | jq 'length')"
  if [[ "$shaped_count_ss" -gt "$base_count_ss" ]]; then
    added_shape_tasks="$(jq -nc --argjson shaped "$shaped_tasks_json" --argjson n "$base_count_ss" '$shaped[$n:]')"
    EXTRA_TASKS_JSON="$(jq -nc --argjson base "$EXTRA_TASKS_JSON" --argjson extra "$added_shape_tasks" '$base + $extra')"
    TASKS="$shaped_tasks_json"
  fi
  NATIVE_SINGLESHOT_PROFILE_SHAPE_JSON="$(jq -nc \
    --argjson rep "$(cat "$OUT_DIR/02ad_single_shot_shape_report.json")" \
    --argjson base_count "$base_count_ss" \
    --argjson shaped_count "$shaped_count_ss" \
    '{enabled:true, base_task_count:$base_count, shaped_task_count:$shaped_count, report:$rep}')"
else
  NATIVE_SINGLESHOT_PROFILE_SHAPE_JSON='{"enabled":false}'
fi
if [[ "$NATIVE_MULTISHOT_DECOMP" == "1" ]]; then
  if [[ "$(printf '%s' "$INTRINSIC_REQS_JSON" | jq 'length')" -eq 0 ]]; then
    python3 "$ROOT_DIR/tools/mcp/synthesize_native_intrinsic_boost_requirements.py" \
      --spec "$INPUT_FILE" \
      --profiles "$NATIVE_IMPACT_COVERAGE_PROFILES" \
      --out "$OUT_DIR/01e_native_multishot_requirements.json" >/dev/null
    INTRINSIC_REQS_JSON="$(cat "$OUT_DIR/01e_native_multishot_requirements.json")"
  fi
  base_multishot_count="$(printf '%s' "$TASKS" | jq 'length')"
  shot_extra='[]'
  shot_attempted=0
  shot_success=0
  req_total="$(printf '%s' "$INTRINSIC_REQS_JSON" | jq 'length')"
  max_profiles="$NATIVE_MULTISHOT_MAX_PROFILES"
  if [[ "$max_profiles" -gt "$req_total" ]]; then
    max_profiles="$req_total"
  fi
  i=0
  while [[ "$i" -lt "$max_profiles" ]]; do
    req_i="$(printf '%s' "$INTRINSIC_REQS_JSON" | jq ".[$i]")"
    req_id="$(printf '%s' "$req_i" | jq -r '.requirementId // empty')"
    if [[ -z "$req_id" || "$req_id" == "null" ]]; then
      req_id="intrinsic-boost-profile-$i"
    fi
    profile_id="$(printf '%s' "$req_id" | sed 's/^intrinsic-boost-//')"
    req_ops_json="$(jq -nc --arg pid "$profile_id" --argjson prof "$(cat "$NATIVE_IMPACT_COVERAGE_PROFILES")" \
      '[($prof.profiles[]? | select(.id == $pid) | .required_prerequisite_ops[]?) // empty]')"
    req_reason_json="$(jq -nc --arg pid "$profile_id" --argjson prof "$(cat "$NATIVE_IMPACT_COVERAGE_PROFILES")" \
      '[($prof.profiles[]? | select(.id == $pid) | .required_reason_keywords[]?) // empty]')"
    req_contract_json="$(jq -nc --arg pid "$profile_id" --argjson prof "$(cat "$NATIVE_IMPACT_COVERAGE_PROFILES")" \
      '[($prof.profiles[]? | select(.id == $pid) | .required_execution_contract[]?) // empty]')"

    shot_reqs="$(jq -nc --argjson base "$NORMALIZED_REQS" --argjson req "$req_i" '$base + [$req]')"
    shot_args="$(jq -nc --argjson nr "$shot_reqs" --argjson cf "$CONFLICTS" --arg strict "$STRICT_EXECUTION_CONTRACT" \
      '{normalizedRequirements:$nr,conflicts:$cf,strictExecutionContract:($strict == "1")}')"
    shot_raw="$(call_tool "whetstone_generate_taskitems" "$shot_args")"
    printf '%s\n' "$shot_raw" > "$OUT_DIR/02ac_generate_taskitems_multishot_${i}_raw.ndjson.json"
    shot_json="$(extract_tool_text_json "$shot_raw")"
    printf '%s\n' "$shot_json" > "$OUT_DIR/02ac_generate_taskitems_multishot_${i}.json"
    shot_attempted=$((shot_attempted + 1))
    if [[ "$(printf '%s' "$shot_json" | jq -r '.success // false')" == "true" ]]; then
      shot_tasks="$(printf '%s' "$shot_json" | jq '.tasks // []')"
      shot_tasks_enriched="$(jq -nc \
        --argjson tasks "$shot_tasks" \
        --argjson req_ops "$req_ops_json" \
        --argjson req_reason "$req_reason_json" \
        --argjson req_contract "$req_contract_json" \
        --arg profile "$profile_id" '
          ($tasks | to_entries | map(
            . as $e
            | ($e.value.executionContract // {}) as $ec
            | (reduce $req_contract[]? as $f ($ec; .[$f] = true)) as $ec2
            | ($e.value + {
                taskId: (($e.value.taskId // ("task-" + (($e.key + 1)|tostring))) + "-ms-" + ($profile|gsub("[^A-Za-z0-9]+";"_")) + "-" + (($e.key + 1)|tostring)),
                prerequisiteOps: ((($e.value.prerequisiteOps // []) + $req_ops) | unique),
                reasons: ((($e.value.reasons // []) + $req_reason + ["native_multishot_profile", $profile]) | unique),
                executionContract: ($ec2 + {
                  deterministic: true,
                  rollbackRequired: ($ec2.rollbackRequired // true),
                  replayValidationRequired: ($ec2.replayValidationRequired // true)
                })
              })
          ))')"
      shot_extra="$(jq -nc --argjson base "$shot_extra" --argjson extra "$shot_tasks_enriched" '$base + $extra')"
      shot_success=$((shot_success + 1))
    fi
    i=$((i + 1))
  done
  candidate_tasks="$(jq -nc --argjson base "$TASKS" --argjson extra "$shot_extra" '$base + $extra')"
  candidate_count="$(printf '%s' "$candidate_tasks" | jq 'length')"
  multishot_applied="false"
  if [[ "$NATIVE_MULTISHOT_APPLY_MODE" == "always" ]]; then
    TASKS="$candidate_tasks"
    EXTRA_TASKS_JSON="$(jq -nc --argjson base "$EXTRA_TASKS_JSON" --argjson extra "$shot_extra" '$base + $extra')"
    multishot_applied="true"
  elif [[ "$candidate_count" -gt "$base_multishot_count" ]]; then
    TASKS="$candidate_tasks"
    EXTRA_TASKS_JSON="$(jq -nc --argjson base "$EXTRA_TASKS_JSON" --argjson extra "$shot_extra" '$base + $extra')"
    multishot_applied="true"
  fi
  NATIVE_MULTISHOT_JSON="$(jq -nc \
    --arg mode "$NATIVE_MULTISHOT_APPLY_MODE" \
    --argjson attempted "$shot_attempted" \
    --argjson successful "$shot_success" \
    --argjson generated_extra_count "$(printf '%s' "$shot_extra" | jq 'length')" \
    --argjson base_task_count "$base_multishot_count" \
    --argjson candidate_task_count "$candidate_count" \
    --argjson applied "$([[ "$multishot_applied" == "true" ]] && echo true || echo false)" \
    '{enabled:true, mode:$mode, attempted_profiles:$attempted, successful_profiles:$successful, generated_extra_task_count:$generated_extra_count, base_task_count:$base_task_count, candidate_task_count:$candidate_task_count, applied:$applied}')"
else
  NATIVE_MULTISHOT_JSON='{"enabled":false,"applied":false}'
fi
if [[ -n "$EXTRA_TASKS_FILE" ]]; then
  if [[ ! -f "$EXTRA_TASKS_FILE" ]]; then
    echo "error: extra tasks file not found: $EXTRA_TASKS_FILE" >&2
    exit 15
  fi
  if ! jq -e 'type == "array"' "$EXTRA_TASKS_FILE" >/dev/null 2>&1; then
    echo "error: extra tasks file must be a JSON array: $EXTRA_TASKS_FILE" >&2
    exit 16
  fi
  EXTRA_TASKS_JSON="$(cat "$EXTRA_TASKS_FILE")"
  TASKS="$(jq -nc --argjson base "$TASKS" --argjson extra "$EXTRA_TASKS_JSON" '$base + $extra')"
fi
if [[ "$NATIVE_PROFILE_AUTOFILL" == "1" ]]; then
  printf '%s\n' "$TASKS" > "$OUT_DIR/02ab_native_tasks_pre_autofill.json"
  python3 "$ROOT_DIR/tools/mcp/synthesize_native_profile_autofill_tasks.py" \
    --spec "$INPUT_FILE" \
    --tasks "$OUT_DIR/02ab_native_tasks_pre_autofill.json" \
    --profiles "$NATIVE_IMPACT_COVERAGE_PROFILES" \
    --out-tasks "$OUT_DIR/02ab_profile_autofill_tasks.json" \
    --out-report "$OUT_DIR/02ab_profile_autofill_report.json" \
    --max-tasks "$NATIVE_PROFILE_AUTOFILL_MAX_TASKS" >/dev/null
  PROFILE_AUTOFILL_TASKS_JSON="$(cat "$OUT_DIR/02ab_profile_autofill_tasks.json")"
  if [[ "$(printf '%s' "$PROFILE_AUTOFILL_TASKS_JSON" | jq 'length')" -gt 0 ]]; then
    TASKS="$(jq -nc --argjson base "$TASKS" --argjson extra "$PROFILE_AUTOFILL_TASKS_JSON" '$base + $extra')"
    EXTRA_TASKS_JSON="$(jq -nc --argjson base "$EXTRA_TASKS_JSON" --argjson extra "$PROFILE_AUTOFILL_TASKS_JSON" '$base + $extra')"
  fi
  NATIVE_PROFILE_AUTOFILL_JSON="$(jq -nc \
    --argjson report "$(cat "$OUT_DIR/02ab_profile_autofill_report.json")" \
    --argjson task_count "$(printf '%s' "$PROFILE_AUTOFILL_TASKS_JSON" | jq 'length')" \
    '{enabled:true, report:$report, injected_task_count:$task_count}')"
else
  NATIVE_PROFILE_AUTOFILL_JSON='{"enabled":false,"injected_task_count":0}'
fi
if [[ "$NATIVE_REASON_ENRICHMENT" == "1" && -f "$OUT_DIR/01c_semantic_injected_requirements.json" ]]; then
  TASKS="$(jq -nc \
    --argjson tasks "$TASKS" \
    --argjson sem "$(cat "$OUT_DIR/01c_semantic_injected_requirements.json")" '
    ($sem
      | map(.kind // "")
      | map(
          if . == "constraint" then "semantic_risk_contract_constraint"
          elif . == "dependency" then "semantic_capability_requirement"
          elif . == "goal" then "semantic_goal_alignment"
          else empty end
        )
      | unique) as $tags
    | ($tasks | to_entries | map(
        .value.reasons = (((.value.reasons // []) + $tags) | unique)
        | .value
      ))')"
  NATIVE_REASON_ENRICHMENT_JSON="$(jq -nc \
    --argjson tags "$(jq -nc --argjson sem "$(cat "$OUT_DIR/01c_semantic_injected_requirements.json")" '
      ($sem
      | map(.kind // "")
      | map(
          if . == "constraint" then "semantic_risk_contract_constraint"
          elif . == "dependency" then "semantic_capability_requirement"
          elif . == "goal" then "semantic_goal_alignment"
          else empty end
        )
      | unique)
    ')" \
    --argjson enriched_task_count "$(printf '%s' "$TASKS" | jq 'length')" \
    '{enabled:true, enriched_task_count:$enriched_task_count, tags:$tags}')"
else
  NATIVE_REASON_ENRICHMENT_JSON='{"enabled":false,"enriched_task_count":0,"tags":[]}'
fi
native_task_count="$(printf '%s' "$TASKS" | jq 'length')"
native_semantic_signal_count="$(printf '%s' "$TASKS" | jq '[.[] | ((.reasons // [])[]? | tostring) | select(test("semantic|risk|contract|capability"; "i"))] | length')"
native_task_count_ok="true"
native_semantic_signal_ok="true"
if [[ "$native_task_count" -lt "$NATIVE_TASK_MIN_COUNT" ]]; then
  native_task_count_ok="false"
fi
if [[ "$native_semantic_signal_count" -lt "$NATIVE_SEMANTIC_SIGNAL_MIN" ]]; then
  native_semantic_signal_ok="false"
fi
native_gate_passed="true"
if [[ "$native_task_count_ok" != "true" || "$native_semantic_signal_ok" != "true" ]]; then
  native_gate_passed="false"
fi
NATIVE_DECOMP_GATE_JSON="$(jq -nc \
  --argjson enabled "$([[ "$NATIVE_DECOMP_HARD_GATE" == "1" ]] && echo true || echo false)" \
  --argjson passed "$([[ "$native_gate_passed" == "true" ]] && echo true || echo false)" \
  --argjson native_task_count "$native_task_count" \
  --argjson native_semantic_signal_count "$native_semantic_signal_count" \
  --argjson min_task_count "$NATIVE_TASK_MIN_COUNT" \
  --argjson min_semantic_signal_count "$NATIVE_SEMANTIC_SIGNAL_MIN" \
  '{
    enabled:$enabled,
    passed:$passed,
    native_task_count:$native_task_count,
    native_semantic_signal_count:$native_semantic_signal_count,
    min_task_count:$min_task_count,
    min_semantic_signal_count:$min_semantic_signal_count
  }')"
printf '%s\n' "$NATIVE_DECOMP_GATE_JSON" > "$OUT_DIR/02b_native_decomposition_gate.json"
if [[ "$NATIVE_DECOMP_HARD_GATE" == "1" && "$native_gate_passed" != "true" ]]; then
  echo "error: native decomposition gate failed" >&2
  echo "error: see $OUT_DIR/02b_native_decomposition_gate.json" >&2
  exit 10
fi

semantic_fallback_needed="false"
if [[ "$SEMANTIC_TASK_EXPANSION_MODE" == "always" ]]; then
  semantic_fallback_needed="true"
elif [[ "$SEMANTIC_TASK_EXPANSION_MODE" == "fallback_only" ]]; then
  if [[ "$native_task_count" -le 2 ]]; then
    semantic_fallback_needed="true"
  elif [[ "$native_task_count" -le 4 && "$native_semantic_signal_count" -eq 0 ]]; then
    semantic_fallback_needed="true"
  fi
fi

if [[ "$SEMANTIC_PLANNING_BRIDGE" == "1" && "$SEMANTIC_REQUIREMENT_INJECTION" == "1" && "$SEMANTIC_TASK_EXPANSION" == "1" && "$semantic_fallback_needed" == "true" && -f "$OUT_DIR/01c_semantic_injected_requirements.json" ]]; then
  SEMANTIC_EXPANDED_TASKS="$(jq -nc \
    --argjson base "$TASKS" \
    --argjson sem "$(cat "$OUT_DIR/01c_semantic_injected_requirements.json")" '
      def mk_task($idx; $req):
        {
          taskId: ("semantic-task-" + ($idx|tostring)),
          title: ("Semantic Guardrail: " + ($req.normalizedText // "requirement")),
          prerequisiteOps: ["whetstone_architect_intake","whetstone_generate_taskitems","whetstone_queue_ready","whetstone_validate_taskitem"],
          reasons: ["semantic_annotation_bridge", ($req.kind // "constraint"), ($req.requirementId // ("semantic-" + ($idx|tostring)))],
          confidence: 0.92,
          dependencyTaskIds: ["task-1"],
          resourceLocks: [],
          executionContract: {
            deterministic: true,
            rollbackRequired: true,
            replayValidationRequired: true,
            maxFileTouches: 3,
            maxContextTokens: 4096,
            maxExecutionSteps: 6,
            executionSpecificityScore: 88
          }
        };
      ($sem[:6] | to_entries | map(mk_task(.key; .value))) as $extra
      | ($base + $extra)
    ')"
  TASKS="$SEMANTIC_EXPANDED_TASKS"
  SEMANTIC_TASK_EXPANSION_JSON="$(jq -nc \
    --argjson sem "$(cat "$OUT_DIR/01c_semantic_injected_requirements.json")" \
    --arg mode "$SEMANTIC_TASK_EXPANSION_MODE" \
    --argjson native_task_count "$native_task_count" \
    --argjson native_semantic_signal_count "$native_semantic_signal_count" \
    '{enabled:true, mode:$mode, fallback_applied:true, expanded_task_count: (($sem[:6])|length), native_task_count:$native_task_count, native_semantic_signal_count:$native_semantic_signal_count}')"
else
  if [[ "$SEMANTIC_PLANNING_BRIDGE" == "1" && "$SEMANTIC_REQUIREMENT_INJECTION" == "1" && "$SEMANTIC_TASK_EXPANSION" == "1" ]]; then
    SEMANTIC_TASK_EXPANSION_JSON="$(jq -nc \
      --arg mode "$SEMANTIC_TASK_EXPANSION_MODE" \
      --argjson native_task_count "$native_task_count" \
      --argjson native_semantic_signal_count "$native_semantic_signal_count" \
      '{enabled:true, mode:$mode, fallback_applied:false, expanded_task_count:0, native_task_count:$native_task_count, native_semantic_signal_count:$native_semantic_signal_count, skipped_reason:"native_sufficient_or_missing_semantic_requirements"}')"
  else
    SEMANTIC_TASK_EXPANSION_JSON="$(jq -nc \
      --arg mode "$SEMANTIC_TASK_EXPANSION_MODE" \
      --argjson native_task_count "$native_task_count" \
      --argjson native_semantic_signal_count "$native_semantic_signal_count" \
      '{enabled:false, mode:$mode, fallback_applied:false, expanded_task_count:0, native_task_count:$native_task_count, native_semantic_signal_count:$native_semantic_signal_count, skipped_reason:"semantic_expansion_disabled"}')"
  fi
fi
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
  --argjson semantic_gate "$SEMANTIC_GATE_JSON" \
  --argjson semantic_intake_augment "$SEMANTIC_AUGMENT_JSON" \
  --argjson semantic_requirement_injection "$SEMANTIC_REQUIREMENT_INJECTION_JSON" \
  --argjson semantic_task_expansion "$SEMANTIC_TASK_EXPANSION_JSON" \
  --argjson native_decomposition_gate "$NATIVE_DECOMP_GATE_JSON" \
  --argjson native_reason_enrichment "$NATIVE_REASON_ENRICHMENT_JSON" \
  --argjson native_decomposition_retry "$NATIVE_DECOMP_RETRY_JSON" \
  --argjson native_profile_autofill "$NATIVE_PROFILE_AUTOFILL_JSON" \
  --argjson native_intrinsic_boost "$NATIVE_INTRINSIC_BOOST_JSON" \
  --argjson native_raw_top_gap_requirements "$NATIVE_RAW_TOP_GAP_REQUIREMENTS_JSON" \
  --argjson native_raw_candidate_search "$NATIVE_RAW_CANDIDATE_SEARCH_JSON" \
  --argjson native_raw_hardening "$NATIVE_RAW_HARDENING_JSON" \
  --argjson native_single_shot_profile_shape "$NATIVE_SINGLESHOT_PROFILE_SHAPE_JSON" \
  --argjson native_multishot "$NATIVE_MULTISHOT_JSON" \
  --argjson extra_normalized_requirements "$EXTRA_NORMALIZED_REQUIREMENTS_JSON" \
  --argjson extra_tasks "$EXTRA_TASKS_JSON" \
  --argjson intake "$INTAKE_JSON" \
  --argjson generated "$GEN_JSON" \
  --argjson effective_task_count "$(printf '%s' "$TASKS" | jq 'length')" \
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
    semantic_planning_gate: $semantic_gate,
    semantic_intake_augment: $semantic_intake_augment,
    semantic_requirement_injection: $semantic_requirement_injection,
    semantic_task_expansion: $semantic_task_expansion,
    native_decomposition_gate: $native_decomposition_gate,
    native_reason_enrichment: $native_reason_enrichment,
    native_decomposition_retry: $native_decomposition_retry,
    native_profile_autofill: $native_profile_autofill,
    native_intrinsic_boost: $native_intrinsic_boost,
    native_raw_top_gap_requirements: $native_raw_top_gap_requirements,
    native_raw_candidate_search: $native_raw_candidate_search,
    native_raw_hardening: $native_raw_hardening,
    native_single_shot_profile_shape: $native_single_shot_profile_shape,
    native_multishot: $native_multishot,
    extra_normalized_requirements: $extra_normalized_requirements,
    extra_tasks: $extra_tasks,
    intake: {
      success: ($intake.success // false),
      normalized_requirement_count: (($intake.normalizedRequirements // [])|length),
      conflict_count: (($intake.conflicts // [])|length),
      ambiguous_requirement_count: ($intake.conflictSignals.ambiguousRequirementCount // 0)
    },
    taskitems: {
      success: ($generated.success // false),
      task_count: (($generated.tasks // [])|length),
      effective_task_count: $effective_task_count,
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

if [[ "$NATIVE_IMPACT_COVERAGE_GATE" == "1" ]]; then
  # Emit a provisional summary so coverage checker can read run metadata.
  printf '%s\n' "$SUMMARY_JSON" > "$OUT_DIR/00_summary.json"
  if python3 "$ROOT_DIR/tools/mcp/check_native_decomposition_impact_coverage.py" \
    --run-dir "$OUT_DIR" \
    --profiles "$NATIVE_IMPACT_COVERAGE_PROFILES" \
    --out "$OUT_DIR/06_native_impact_coverage.json" \
    $([[ "$NATIVE_IMPACT_COVERAGE_ENFORCE" == "1" ]] && echo --enforce) >/dev/null; then
    NATIVE_IMPACT_COVERAGE_JSON="$(cat "$OUT_DIR/06_native_impact_coverage.json")"
  else
    NATIVE_IMPACT_COVERAGE_JSON="$(cat "$OUT_DIR/06_native_impact_coverage.json")"
    SUMMARY_JSON="$(printf '%s' "$SUMMARY_JSON" | jq --argjson nic "$NATIVE_IMPACT_COVERAGE_JSON" '.native_impact_coverage = $nic')"
    printf '%s\n' "$SUMMARY_JSON" > "$OUT_DIR/00_summary.json"
    echo "error: native impact coverage gate failed" >&2
    echo "error: see $OUT_DIR/06_native_impact_coverage.json" >&2
    exit 11
  fi
  SUMMARY_JSON="$(printf '%s' "$SUMMARY_JSON" | jq --argjson nic "$NATIVE_IMPACT_COVERAGE_JSON" '.native_impact_coverage = $nic')"
fi

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
