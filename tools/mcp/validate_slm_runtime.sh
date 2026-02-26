#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
OLLAMA_URL="${OLLAMA_URL:-http://127.0.0.1:11434/api/generate}"
OLLAMA_TAGS_URL="${OLLAMA_TAGS_URL:-http://127.0.0.1:11434/api/tags}"
MODEL="${OLLAMA_MODEL:-qwen2.5-coder:14b}"
BIN="${WSTONE_MCP_BIN:-$ROOT_DIR/editor/build-native/whetstone_mcp_stable}"
WORKSPACE="${WSTONE_WORKSPACE:-$ROOT_DIR}"
LANGUAGE="${WSTONE_LANGUAGE:-cpp}"
MCP_TIMEOUT_SECONDS="${MCP_TIMEOUT_SECONDS:-20}"
OUT_DIR="${OUT_DIR:-$ROOT_DIR/logs/taskitem_runs/preflight}"
STRICT_GRAMMARS="${STRICT_GRAMMARS:-1}"
GRAMMARS_DIR="${GRAMMARS_DIR:-$ROOT_DIR/tools/mcp/grammars}"

mkdir -p "$OUT_DIR"
STAMP="$(date +%Y%m%d_%H%M%S)"
OUT_FILE="$OUT_DIR/preflight_${STAMP}.json"

need_cmd() {
  local c="$1"
  command -v "$c" >/dev/null 2>&1
}

fail=0
errs=()
warns=()

if ! need_cmd jq; then errs+=("missing:jq"); fail=1; fi
if ! need_cmd curl; then errs+=("missing:curl"); fail=1; fi
if ! need_cmd ollama; then errs+=("missing:ollama"); fail=1; fi
if [[ ! -x "$BIN" ]]; then errs+=("mcp_not_executable:$BIN"); fail=1; fi

ollama_up=0
if [[ "$fail" -eq 0 ]]; then
  if curl -sS --max-time 3 "$OLLAMA_TAGS_URL" >/tmp/ollama_tags_preflight.json 2>/tmp/ollama_tags_preflight.err; then
    ollama_up=1
  else
    errs+=("ollama_unreachable:$(tr '\n' ' ' </tmp/ollama_tags_preflight.err | sed 's/[[:space:]]\\+/ /g')")
    fail=1
  fi
fi

mcp_ok=0
mcp_tool_count=0
if [[ "$fail" -eq 0 ]]; then
  init_req='{"jsonrpc":"2.0","id":1,"method":"initialize","params":{"protocolVersion":"2025-11-25","capabilities":{},"clientInfo":{"name":"preflight","version":"1.0"}}}'
  list_req='{"jsonrpc":"2.0","id":2,"method":"tools/list","params":{}}'
  resp="$(printf '%s\n%s\n' "$init_req" "$list_req" | timeout "${MCP_TIMEOUT_SECONDS}s" "$BIN" --workspace "$WORKSPACE" --language "$LANGUAGE" 2>/dev/null | tail -n1 || true)"
  if [[ -z "$resp" ]]; then
    errs+=("mcp_tools_list_empty_response")
    fail=1
  else
    mcp_tool_count="$(printf '%s' "$resp" | jq -r '.result.tools | length // 0' 2>/dev/null || echo 0)"
    if [[ "$mcp_tool_count" -eq 0 ]]; then
      errs+=("mcp_tools_list_zero_tools")
      fail=1
    else
      mcp_ok=1
    fi
  fi
fi

strict_ok=1
if [[ "$fail" -eq 0 && "$STRICT_GRAMMARS" == "1" ]]; then
  if [[ ! -f "$GRAMMARS_DIR/manifest.json" || ! -f "$GRAMMARS_DIR/manifest.lock" ]]; then
    errs+=("strict_missing_manifest:$GRAMMARS_DIR")
    strict_ok=0
    fail=1
  elif [[ ! -f "$GRAMMARS_DIR/dispatch.gbnf" || ! -f "$GRAMMARS_DIR/dispatch_schema.json" || ! -f "$GRAMMARS_DIR/per_tool_schemas.json" ]]; then
    errs+=("strict_missing_dispatch_artifacts:$GRAMMARS_DIR")
    strict_ok=0
    fail=1
  else
    if ! python3 "$ROOT_DIR/tools/mcp/verify_grammar_manifest.py" \
      --schemas "$ROOT_DIR/tools/mcp/whetstone_tool_schemas.json" \
      --grammars-dir "$GRAMMARS_DIR" >/tmp/strict_manifest_check.out 2>/tmp/strict_manifest_check.err; then
      errs+=("strict_manifest_verify_failed:$(tr '\n' ' ' </tmp/strict_manifest_check.err | sed 's/[[:space:]]\\+/ /g')")
      strict_ok=0
      fail=1
    fi
    dispatch_count="$(python3 - "$GRAMMARS_DIR/dispatch_schema.json" <<'PY'
import json,sys
try:
    d=json.load(open(sys.argv[1]))
    print(len(d.get("oneOf",[])))
except Exception:
    print(0)
PY
)"
    if [[ "$dispatch_count" -ne "$mcp_tool_count" ]]; then
      errs+=("strict_tool_count_mismatch:dispatch=$dispatch_count,mcp=$mcp_tool_count")
      strict_ok=0
      fail=1
    fi
  fi
fi

gen_ok=0
if [[ "$fail" -eq 0 && "$ollama_up" -eq 1 ]]; then
  req="$(jq -nc --arg model "$MODEL" '
    {
      model: $model,
      prompt: "Return exactly JSON: {\"action\":\"final\",\"status\":\"blocked\",\"summary\":\"preflight\",\"next_steps\":[\"none\"]}",
      stream: false,
      keep_alive: "0",
      options: {temperature: 0.0, num_predict: 120}
    }')"
  if curl -sS --max-time 20 -X POST "$OLLAMA_URL" -H "Content-Type: application/json" -d "$req" >/tmp/ollama_gen_preflight.json 2>/tmp/ollama_gen_preflight.err; then
    raw="$(jq -r '.response // ""' /tmp/ollama_gen_preflight.json 2>/dev/null || true)"
    if [[ -z "$raw" ]]; then
      errs+=("ollama_empty_response")
      fail=1
    else
      gen_ok=1
    fi
  else
    errs+=("ollama_generate_failed:$(tr '\n' ' ' </tmp/ollama_gen_preflight.err | sed 's/[[:space:]]\\+/ /g')")
    fail=1
  fi
fi

jq -nc \
  --arg ts "$(date -u +%Y-%m-%dT%H:%M:%SZ)" \
  --arg model "$MODEL" \
  --arg bin "$BIN" \
  --arg workspace "$WORKSPACE" \
  --arg language "$LANGUAGE" \
  --argjson ok "$((fail==0 ? 1 : 0))" \
  --argjson ollama_up "$ollama_up" \
  --argjson mcp_ok "$mcp_ok" \
  --argjson strict_ok "$strict_ok" \
  --argjson strict_enabled "$((STRICT_GRAMMARS==1 ? 1 : 0))" \
  --argjson mcp_tool_count "$mcp_tool_count" \
  --argjson gen_ok "$gen_ok" \
  --argjson errors "$(printf '%s\n' "${errs[@]-}" | jq -R -s -c 'split("\n")|map(select(length>0))')" \
  --argjson warnings "$(printf '%s\n' "${warns[@]-}" | jq -R -s -c 'split("\n")|map(select(length>0))')" \
  '{
    timestamp: $ts,
    ok: ($ok == 1),
    model: $model,
    checks: {
      ollama_reachable: ($ollama_up == 1),
      mcp_available: ($mcp_ok == 1),
      strict_grammars_enabled: ($strict_enabled == 1),
      strict_grammars_valid: ($strict_ok == 1),
      mcp_tool_count: $mcp_tool_count,
      generation_non_empty: ($gen_ok == 1)
    },
    config: {
      mcp_bin: $bin,
      workspace: $workspace,
      language: $language
    },
    errors: $errors,
    warnings: $warnings
  }' > "$OUT_FILE"

echo "Preflight report: $OUT_FILE"
if [[ "$fail" -ne 0 ]]; then
  echo "Preflight FAILED"
  cat "$OUT_FILE"
  exit 2
fi
echo "Preflight PASSED"
cat "$OUT_FILE"
