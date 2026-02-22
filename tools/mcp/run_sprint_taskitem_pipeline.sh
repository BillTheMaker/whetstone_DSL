#!/usr/bin/env bash
set -euo pipefail

if ! command -v jq >/dev/null 2>&1; then
  echo "error: jq is required" >&2
  exit 1
fi

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BIN="${WSTONE_MCP_BIN:-$ROOT_DIR/editor/build-native/whetstone_mcp_stable}"
WORKSPACE="${WSTONE_WORKSPACE:-$ROOT_DIR}"
LANGUAGE="${WSTONE_LANGUAGE:-cpp}"
SPRINT_FILE_REL="${1:-sprint46_plan.md}"
SPRINT_FILE="$ROOT_DIR/$SPRINT_FILE_REL"

if [[ ! -x "$BIN" ]]; then
  echo "error: MCP binary not executable: $BIN" >&2
  exit 1
fi
if [[ ! -f "$SPRINT_FILE" ]]; then
  echo "error: sprint file not found: $SPRINT_FILE" >&2
  exit 1
fi

STAMP="$(date +%Y%m%d_%H%M%S)"
SPRINT_BASENAME="$(basename "$SPRINT_FILE_REL" .md)"
OUT_DIR="$ROOT_DIR/logs/taskitem_runs/${SPRINT_BASENAME}_${STAMP}"
mkdir -p "$OUT_DIR"

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
  local tmp_steps tmp_constraints
  tmp_steps="$(mktemp)"
  tmp_constraints="$(mktemp)"

  awk '/^### Step /{sub(/^### /, "", $0); print "- " $0}' "$source_file" > "$tmp_steps"
  awk '
    /^## Architecture Gate/ {in_gate=1; next}
    /^## / && in_gate {in_gate=0}
    in_gate && /^- / {print}
  ' "$source_file" > "$tmp_constraints"

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
    echo
    echo "## Acceptance Criteria"
    echo "- All sprint step tests pass"
    echo "- No regression in existing MCP tool behavior"
    echo "- Generated task queue is ready or blockers are explicit"
  } | sed '/^[[:space:]]*$/N;/^\n$/D'

  rm -f "$tmp_steps" "$tmp_constraints"
}

MARKDOWN_CONTENT="$(cat "$SPRINT_FILE")"
INTAKE_ARGS="$(jq -nc --arg md "$MARKDOWN_CONTENT" '{markdown:$md}')"
INTAKE_RESP_RAW="$(call_tool "whetstone_architect_intake" "$INTAKE_ARGS")"
printf '%s\n' "$INTAKE_RESP_RAW" > "$OUT_DIR/01_intake_raw.ndjson.json"
INTAKE_JSON="$(extract_tool_text_json "$INTAKE_RESP_RAW")"
printf '%s\n' "$INTAKE_JSON" > "$OUT_DIR/01_intake.json"

if [[ "$(printf '%s' "$INTAKE_JSON" | jq -r '.success // false')" != "true" ]] &&
   [[ "$(printf '%s' "$INTAKE_JSON" | jq -r '.error // ""')" == "no_requirements_found" ]]; then
  FALLBACK_SPEC="$(build_fallback_intake_spec "$SPRINT_FILE")"
  printf '%s\n' "$FALLBACK_SPEC" > "$OUT_DIR/01a_fallback_intake_spec.md"
  INTAKE_ARGS="$(jq -nc --arg md "$FALLBACK_SPEC" '{markdown:$md}')"
  INTAKE_RESP_RAW="$(call_tool "whetstone_architect_intake" "$INTAKE_ARGS")"
  printf '%s\n' "$INTAKE_RESP_RAW" > "$OUT_DIR/01b_intake_retry_raw.ndjson.json"
  INTAKE_JSON="$(extract_tool_text_json "$INTAKE_RESP_RAW")"
  printf '%s\n' "$INTAKE_JSON" > "$OUT_DIR/01b_intake_retry.json"
fi

if [[ "$(printf '%s' "$INTAKE_JSON" | jq -r '.success // false')" != "true" ]]; then
  echo "error: intake failed, see logs in $OUT_DIR" >&2
  exit 3
fi

NORMALIZED_REQS="$(printf '%s' "$INTAKE_JSON" | jq '.normalizedRequirements')"
CONFLICTS="$(printf '%s' "$INTAKE_JSON" | jq '.conflicts // []')"
GEN_ARGS="$(jq -nc --argjson nr "$NORMALIZED_REQS" --argjson cf "$CONFLICTS" '{normalizedRequirements:$nr,conflicts:$cf}')"
GEN_RESP_RAW="$(call_tool "whetstone_generate_taskitems" "$GEN_ARGS")"
printf '%s\n' "$GEN_RESP_RAW" > "$OUT_DIR/02_generate_taskitems_raw.ndjson.json"
GEN_JSON="$(extract_tool_text_json "$GEN_RESP_RAW")"
printf '%s\n' "$GEN_JSON" > "$OUT_DIR/02_generate_taskitems.json"
if [[ "$(printf '%s' "$GEN_JSON" | jq -r '.success // false')" != "true" ]]; then
  echo "error: taskitem generation failed, see logs in $OUT_DIR" >&2
  exit 4
fi

TASKS="$(printf '%s' "$GEN_JSON" | jq '.tasks')"
QUEUE_ARGS="$(jq -nc --argjson t "$TASKS" --argjson nr "$NORMALIZED_REQS" '{tasks:$t,normalizedRequirements:$nr}')"
QUEUE_RESP_RAW="$(call_tool "whetstone_queue_ready" "$QUEUE_ARGS")"
printf '%s\n' "$QUEUE_RESP_RAW" > "$OUT_DIR/03_queue_ready_raw.ndjson.json"
QUEUE_JSON="$(extract_tool_text_json "$QUEUE_RESP_RAW")"
printf '%s\n' "$QUEUE_JSON" > "$OUT_DIR/03_queue_ready.json"

VALIDATE_TASKS="$(printf '%s' "$TASKS" | jq '[.[] | {
  task_id: .taskId,
  title: .title,
  prerequisite_ops: (.prerequisiteOps // []),
  reasons: (.reasons // []),
  confidence: (.confidence // 0),
  dependency_task_ids: (.dependencyTaskIds // [])
}]')"
VALIDATE_ARGS="$(jq -nc --argjson items "$VALIDATE_TASKS" --arg ws "$WORKSPACE" '{taskitems:$items,workspace:$ws}')"
VALIDATE_RESP_RAW="$(call_tool "whetstone_validate_taskitem" "$VALIDATE_ARGS")"
printf '%s\n' "$VALIDATE_RESP_RAW" > "$OUT_DIR/04_validate_taskitem_raw.ndjson.json"
VALIDATE_JSON="$(extract_tool_text_json "$VALIDATE_RESP_RAW")"
printf '%s\n' "$VALIDATE_JSON" > "$OUT_DIR/04_validate_taskitem.json"

SUMMARY_JSON="$(jq -nc \
  --arg sprint "$SPRINT_FILE_REL" \
  --arg outDir "$OUT_DIR" \
  --arg bin "$BIN" \
  --arg workspace "$WORKSPACE" \
  --argjson intake "$INTAKE_JSON" \
  --argjson generated "$GEN_JSON" \
  --argjson queue "$QUEUE_JSON" \
  --argjson validate "$VALIDATE_JSON" \
  '{
    sprint: $sprint,
    timestamp: now|todate,
    mcp_binary: $bin,
    workspace: $workspace,
    output_dir: $outDir,
    intake: {
      success: ($intake.success // false),
      normalized_requirement_count: (($intake.normalizedRequirements // [])|length),
      conflict_count: (($intake.conflicts // [])|length),
      ambiguous_requirement_count: ($intake.conflictSignals.ambiguousRequirementCount // 0)
    },
    taskitems: {
      success: ($generated.success // false),
      task_count: (($generated.tasks // [])|length),
      escalate_count: ($generated.escalateCount // 0)
    },
    queue_ready: {
      success: ($queue.success // false),
      ready: ($queue.ready // false),
      ready_count: ($queue.readyCount // 0),
      blocker_count: (($queue.blockers // [])|length)
    },
    validation: {
      success: ($validate.success // false),
      total_taskitems: ($validate.report.total_taskitems // 0),
      average_score: ($validate.report.average_score // 0),
      self_contained_count: ($validate.report.self_contained_count // 0),
      failing_count: ($validate.report.failing_count // 0)
    }
  }')"
printf '%s\n' "$SUMMARY_JSON" > "$OUT_DIR/00_summary.json"

echo "Taskitem pipeline complete."
echo "Output dir: $OUT_DIR"
echo "$SUMMARY_JSON" | jq .
