#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BIN="${WSTONE_MCP_BIN:-$ROOT_DIR/editor/build-native/whetstone_mcp}"
WORKSPACE="${WSTONE_WORKSPACE:-$ROOT_DIR}"
LANGUAGE="${WSTONE_LANGUAGE:-cpp}"
STRICT_MODE="${STRICT_MODE:-1}"

SPEC="${1:-Generate WorkItem and PriorityQueue classes with enqueue, dequeue, peek, size, empty}"

PY_SRC='class WorkItem:
    def __init__(self, job_id: str, priority: int, payload: str):
        self.job_id = job_id
        self.priority = priority
        self.payload = payload

class PriorityQueue:
    def __init__(self):
        self.items = []

    def enqueue(self, item: WorkItem):
        self.items.append(item)

    def dequeue(self) -> WorkItem:
        return self.items.pop(0)

    def peek(self) -> WorkItem:
        return self.items[0]

    def size(self) -> int:
        return len(self.items)

    def empty(self) -> bool:
        return len(self.items) == 0'

OUT_DIR="${OUT_DIR:-$ROOT_DIR/logs/taskitem_runs/ab_test_ast_vs_language_first_$(date +%Y%m%d_%H%M%S)}"
mkdir -p "$OUT_DIR"

code_ext="txt"
case "$LANGUAGE" in
  cpp|c++) code_ext="cpp" ;;
  python) code_ext="py" ;;
  go) code_ext="go" ;;
  rust) code_ext="rs" ;;
esac

INIT='{"jsonrpc":"2.0","id":1,"method":"initialize","params":{"protocolVersion":"2025-11-25","capabilities":{},"clientInfo":{"name":"ab-test","version":"1.0"}}}'

call_tool() {
  local request_json="$1"
  printf '%s\n%s\n' "$INIT" "$request_json" | "$BIN" --workspace "$WORKSPACE" --language "$LANGUAGE" 2>/dev/null || true
}

token_json() {
  local file="$1"
  python3 "$ROOT_DIR/tools/mcp/estimate_tokens.py" --file "$file"
}

append_lang_contract() {
  local base_spec="$1"
  local lang="$2"
  local contract=""
  case "$lang" in
    go)
      contract=$'Language contract (Go):\n- output valid Go source with explicit package declaration (`package generated` unless main is required)\n- every function parameter must include a type\n- if `fmt.` is used, include `import "fmt"`\n- do not emit Python tokens like `pass`'
      ;;
    rust)
      contract=$'Language contract (Rust):\n- every function parameter must include an explicit type\n- use Rust macros correctly (`print!`/`println!`) with format strings\n- do not emit Python tokens like `pass`'
      ;;
    python)
      contract=$'Language contract (Python):\n- emit syntactically valid Python 3.12+\n- avoid undefined names and placeholder statements'
      ;;
    cpp|c++)
      contract=$'Language contract (C++):\n- include required STL headers for all used std symbols\n- emit concrete, compile-ready declarations without placeholders'
      ;;
  esac
  if [[ -n "$contract" ]]; then
    printf '%s\n\n%s\n' "$base_spec" "$contract"
  else
    printf '%s\n' "$base_spec"
  fi
}

strict_arg=()
if [[ "$STRICT_MODE" == "1" ]]; then
  strict_arg+=(--strict)
fi

# Path A: whetstone_generate_code
SPEC_A="$(append_lang_contract "$SPEC" "$LANGUAGE")"
REQ_A=$(jq -nc --arg spec "$SPEC_A" '{jsonrpc:"2.0",id:2,method:"tools/call",params:{name:"whetstone_generate_code",arguments:{spec:$spec,preferImports:true}}}')
printf '%s\n' "$REQ_A" > "$OUT_DIR/path_a_request.json"
call_tool "$REQ_A" > "$OUT_DIR/path_a_ndjson.txt"
tail -n1 "$OUT_DIR/path_a_ndjson.txt" > "$OUT_DIR/path_a_raw.json"
jq -r '.result.content[0].text // "{}"' "$OUT_DIR/path_a_raw.json" | jq '.' > "$OUT_DIR/path_a_payload.json"
jq -r '.generatedCode // .note // ""' "$OUT_DIR/path_a_payload.json" > "$OUT_DIR/path_a_generated.$code_ext"
python3 "$ROOT_DIR/tools/mcp/evaluate_generated_code_gates.py" \
  --code-file "$OUT_DIR/path_a_generated.$code_ext" \
  --language "$LANGUAGE" \
  "${strict_arg[@]}" \
  --out "$OUT_DIR/path_a_gates.json" >/dev/null

# Path B: whetstone_run_pipeline (python->target language)
REQ_B=$(jq -nc --arg src "$PY_SRC" --arg target "$LANGUAGE" '{jsonrpc:"2.0",id:2,method:"tools/call",params:{name:"whetstone_run_pipeline",arguments:{source:$src,sourceLanguage:"python",targetLanguage:$target}}}')
printf '%s\n' "$REQ_B" > "$OUT_DIR/path_b_request.json"
call_tool "$REQ_B" > "$OUT_DIR/path_b_ndjson.txt"
tail -n1 "$OUT_DIR/path_b_ndjson.txt" > "$OUT_DIR/path_b_raw.json"
jq -r '.result.content[0].text // "{}"' "$OUT_DIR/path_b_raw.json" | jq '.' > "$OUT_DIR/path_b_payload.json"
jq -r '.generatedCode // ""' "$OUT_DIR/path_b_payload.json" > "$OUT_DIR/path_b_generated.$code_ext"
python3 "$ROOT_DIR/tools/mcp/repair_pipeline_codegen.py" \
  --language "$LANGUAGE" \
  --in-file "$OUT_DIR/path_b_generated.$code_ext" \
  --out-file "$OUT_DIR/path_b_generated.$code_ext" \
  --meta-out "$OUT_DIR/path_b_repair_meta.json"
python3 "$ROOT_DIR/tools/mcp/evaluate_generated_code_gates.py" \
  --code-file "$OUT_DIR/path_b_generated.$code_ext" \
  --language "$LANGUAGE" \
  "${strict_arg[@]}" \
  --out "$OUT_DIR/path_b_gates.json" >/dev/null

A_REQ_TOKENS=$(token_json "$OUT_DIR/path_a_request.json")
A_RESP_TOKENS=$(token_json "$OUT_DIR/path_a_raw.json")
A_CODE_TOKENS=$(token_json "$OUT_DIR/path_a_generated.$code_ext")
B_REQ_TOKENS=$(token_json "$OUT_DIR/path_b_request.json")
B_RESP_TOKENS=$(token_json "$OUT_DIR/path_b_raw.json")
B_CODE_TOKENS=$(token_json "$OUT_DIR/path_b_generated.$code_ext")

jq -nc \
  --arg out_dir "$OUT_DIR" \
  --arg spec "$SPEC" \
  --arg language "$LANGUAGE" \
  --argjson strict_mode "$STRICT_MODE" \
  --argjson path_a_request_tokens "$A_REQ_TOKENS" \
  --argjson path_a_response_tokens "$A_RESP_TOKENS" \
  --argjson path_a_code_tokens "$A_CODE_TOKENS" \
  --argjson path_b_request_tokens "$B_REQ_TOKENS" \
  --argjson path_b_response_tokens "$B_RESP_TOKENS" \
  --argjson path_b_code_tokens "$B_CODE_TOKENS" \
  --argjson path_a_gates "$(cat "$OUT_DIR/path_a_gates.json")" \
  --argjson path_b_gates "$(cat "$OUT_DIR/path_b_gates.json")" \
  '{
    out_dir:$out_dir,
    spec:$spec,
    language:$language,
    strict_mode:($strict_mode==1),
    path_a:{
      gate_overall_ready:$path_a_gates.gates.overall_ready,
      failure_reasons:$path_a_gates.gates.failure_reasons,
      token_accounting:{request:$path_a_request_tokens,response:$path_a_response_tokens,generated_code:$path_a_code_tokens,total_tokens:($path_a_request_tokens.tokens + $path_a_response_tokens.tokens + $path_a_code_tokens.tokens)}
    },
    path_b:{
      gate_overall_ready:$path_b_gates.gates.overall_ready,
      failure_reasons:$path_b_gates.gates.failure_reasons,
      token_accounting:{request:$path_b_request_tokens,response:$path_b_response_tokens,generated_code:$path_b_code_tokens,total_tokens:($path_b_request_tokens.tokens + $path_b_response_tokens.tokens + $path_b_code_tokens.tokens)}
    }
  }' > "$OUT_DIR/00_summary.json"

cat "$OUT_DIR/00_summary.json"
