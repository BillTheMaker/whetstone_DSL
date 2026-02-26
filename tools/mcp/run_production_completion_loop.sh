#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BIN="${WSTONE_MCP_BIN:-$ROOT_DIR/editor/build-native/whetstone_mcp_stable}"
WORKSPACE="${WSTONE_WORKSPACE:-$ROOT_DIR}"
LANGUAGE="${WSTONE_LANGUAGE:-cpp}"
MAX_ITERS="${MAX_ITERS:-3}"
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

loop_spec="$SPEC"
final_ready=0

for i in $(seq 1 "$MAX_ITERS"); do
  args="$(jq -nc --arg s "$loop_spec" '{spec:$s,preferImports:true}')"
  raw="$(call_tool whetstone_generate_code "$args")"
  printf '%s\n' "$raw" > "$OUT_DIR/iter_${i}_generate_raw.json"
  parsed="$(extract_json "$raw")"
  printf '%s\n' "$parsed" > "$OUT_DIR/iter_${i}_generate.json"

  code="$(printf '%s' "$parsed" | jq -r '.generatedCode // ""')"
  if [[ -z "$code" ]]; then
    code="$(printf '%s' "$parsed" | jq -r '.note // ""')"
  fi
  printf '%s\n' "$code" > "$OUT_DIR/iter_${i}_generated_code.txt"

  python3 "$ROOT_DIR/tools/mcp/evaluate_generated_code_gates.py" \
    --code-file "$OUT_DIR/iter_${i}_generated_code.txt" \
    --language "$LANGUAGE" \
    --out "$OUT_DIR/iter_${i}_gates.json" >/tmp/production_loop_gate_${i}.json

  ready="$(jq -r '.gates.overall_ready' "$OUT_DIR/iter_${i}_gates.json")"
  if [[ "$ready" == "true" ]]; then
    final_ready=1
    break
  fi

  # Simple remediation strategy: force class/module details + no placeholders.
  loop_spec="$loop_spec
Return production-ready code only:
- no TODO/FIXME/placeholder markers
- concrete types for fields and methods
- class bodies for queue operations
- compile-clean output."
done

jq -nc \
  --arg out_dir "$OUT_DIR" \
  --arg spec "$SPEC" \
  --argjson max_iters "$MAX_ITERS" \
  --argjson overall_ready "$final_ready" \
  '{
    out_dir:$out_dir,
    spec:$spec,
    max_iters:$max_iters,
    overall_ready:($overall_ready==1)
  }' > "$OUT_DIR/00_summary.json"

echo "Production completion loop output: $OUT_DIR"
cat "$OUT_DIR/00_summary.json"

