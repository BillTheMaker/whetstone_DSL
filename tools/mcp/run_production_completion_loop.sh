#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BIN="${WSTONE_MCP_BIN:-$ROOT_DIR/editor/build-native/whetstone_mcp_stable}"
WORKSPACE="${WSTONE_WORKSPACE:-$ROOT_DIR}"
LANGUAGE="${WSTONE_LANGUAGE:-cpp}"
MAX_ITERS="${MAX_ITERS:-3}"
STRICT_MODE="${STRICT_MODE:-1}"
SPEC="${1:-}"

if [[ -z "$SPEC" ]]; then
  echo "usage: $0 \"<generation spec>\""
  exit 1
fi

if ! command -v jq >/dev/null 2>&1; then
  echo "error: jq required"
  exit 1
fi

OUT_DIR="${OUT_DIR:-$ROOT_DIR/logs/taskitem_runs/production_loop_$(date +%Y%m%d_%H%M%S)}"
mkdir -p "$OUT_DIR"
TRACE_FILE="$OUT_DIR/trace.jsonl"

call_tool() {
  local tool_name="$1"
  local args_json="$2"
  local init_req tool_req responses tool_resp
  init_req='{"jsonrpc":"2.0","id":1,"method":"initialize","params":{"protocolVersion":"2025-11-25","capabilities":{},"clientInfo":{"name":"production-loop","version":"1.0"}}}'
  tool_req="$(jq -nc --arg t "$tool_name" --argjson a "$args_json" '{jsonrpc:"2.0",id:2,method:"tools/call",params:{name:$t,arguments:$a}}')"
  responses="$(printf '%s\n%s\n' "$init_req" "$tool_req" | "$BIN" --workspace "$WORKSPACE" --language "$LANGUAGE" 2>/dev/null || true)"
  tool_resp="$(printf '%s\n' "$responses" | tail -n1)"
  printf '%s' "$tool_resp"
}

extract_json() {
  local raw="$1"
  printf '%s' "$raw" | jq -r '.result.content[0].text // "{}"' | jq '.'
}

log_trace() {
  local phase="$1"
  local iter="$2"
  local payload="$3"
  jq -nc --arg phase "$phase" --arg iter "$iter" --argjson payload "$payload" \
    '{phase:$phase, iteration:$iter, payload:$payload}' >> "$TRACE_FILE"
}

loop_spec="$SPEC"
status="blocked"
blocked_reason="max_iterations_exhausted"
final_ready=0

for i in $(seq 1 "$MAX_ITERS"); do
  iter_id="iter_$(printf '%02d' "$i")"

  args="$(jq -nc --arg s "$loop_spec" '{spec:$s,preferImports:true}')"
  raw="$(call_tool whetstone_generate_code "$args")"
  printf '%s\n' "$raw" > "$OUT_DIR/${iter_id}_generate_raw.json"
  parsed="$(extract_json "$raw")"
  printf '%s\n' "$parsed" > "$OUT_DIR/${iter_id}_generate.json"
  log_trace "generate" "$iter_id" "$parsed"

  code="$(printf '%s' "$parsed" | jq -r '.generatedCode // ""')"
  if [[ -z "$code" ]]; then
    code="$(printf '%s' "$parsed" | jq -r '.note // ""')"
  fi
  printf '%s\n' "$code" > "$OUT_DIR/${iter_id}_generated_code.txt"

  strict_arg=()
  if [[ "$STRICT_MODE" == "1" ]]; then
    strict_arg+=(--strict)
  fi

  python3 "$ROOT_DIR/tools/mcp/evaluate_generated_code_gates.py" \
    --code-file "$OUT_DIR/${iter_id}_generated_code.txt" \
    --language "$LANGUAGE" \
    "${strict_arg[@]}" \
    --out "$OUT_DIR/${iter_id}_gates.json" >/tmp/production_loop_gate_${i}.json

  gates_payload="$(cat "$OUT_DIR/${iter_id}_gates.json")"
  log_trace "gates" "$iter_id" "$gates_payload"

  ready="$(jq -r '.gates.overall_ready' "$OUT_DIR/${iter_id}_gates.json")"
  if [[ "$ready" == "true" ]]; then
    status="green"
    blocked_reason=""
    final_ready=1
    break
  fi

  # Run same-language pipeline for diagnostics when compile/test fail.
  pipeline_args="$(jq -nc --arg src "$code" --arg lang "$LANGUAGE" '{source:$src,sourceLanguage:$lang,targetLanguage:$lang}')"
  pipeline_raw="$(call_tool whetstone_run_pipeline "$pipeline_args")"
  printf '%s\n' "$pipeline_raw" > "$OUT_DIR/${iter_id}_pipeline_raw.json"
  pipeline_parsed="$(extract_json "$pipeline_raw")"
  printf '%s\n' "$pipeline_parsed" > "$OUT_DIR/${iter_id}_pipeline.json"
  log_trace "pipeline_diagnostics" "$iter_id" "$pipeline_parsed"

  python3 "$ROOT_DIR/tools/mcp/remediation_router.py" \
    --gates-json "$OUT_DIR/${iter_id}_gates.json" \
    --out "$OUT_DIR/${iter_id}_route.json" >/tmp/production_loop_route_${i}.json

  route_payload="$(cat "$OUT_DIR/${iter_id}_route.json")"
  log_trace "route" "$iter_id" "$route_payload"

  blocked="$(jq -r '.blocked' "$OUT_DIR/${iter_id}_route.json")"
  if [[ "$blocked" == "true" ]]; then
    status="blocked"
    blocked_reason="$(jq -r '.blocked_reason' "$OUT_DIR/${iter_id}_route.json")"
    break
  fi

  hints="$(jq -r '[.actions[].hint] | unique | join("\n")' "$OUT_DIR/${iter_id}_route.json")"
  loop_spec="$loop_spec

Remediation directives (iteration $i):
$hints

Requirements:
- no TODO/FIXME/placeholder markers
- concrete field and method types
- compile-clean output for target language
- queue operations must be behaviorally complete: enqueue/dequeue/peek/size/empty"
done

if [[ "$status" == "green" ]]; then
  blocked_reason=""
fi

jq -nc \
  --arg out_dir "$OUT_DIR" \
  --arg spec "$SPEC" \
  --arg language "$LANGUAGE" \
  --arg status "$status" \
  --arg blocked_reason "$blocked_reason" \
  --argjson strict_mode "$STRICT_MODE" \
  --argjson max_iters "$MAX_ITERS" \
  --argjson overall_ready "$final_ready" \
  '{
    out_dir:$out_dir,
    spec:$spec,
    language:$language,
    strict_mode:($strict_mode==1),
    max_iters:$max_iters,
    overall_ready:($overall_ready==1),
    status:$status,
    blocked_reason:$blocked_reason
  }' > "$OUT_DIR/00_summary.json"

echo "Production completion loop output: $OUT_DIR"
cat "$OUT_DIR/00_summary.json"
