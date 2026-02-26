#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SPEC_LIST_FILE="${1:-}"
OUT_DIR="${2:-$ROOT_DIR/logs/taskitem_runs/TEST_ONLY_native_profile_closure_ladder_batch_$(date +%Y%m%d_%H%M%S)}"
if [[ -z "$SPEC_LIST_FILE" ]]; then
  echo "usage: $0 <spec_list_file> [out_dir]" >&2
  exit 2
fi
if [[ ! -f "$SPEC_LIST_FILE" ]]; then
  echo "error: spec list not found: $SPEC_LIST_FILE" >&2
  exit 2
fi
mkdir -p "$OUT_DIR"

while IFS= read -r spec; do
  [[ -z "$spec" ]] && continue
  [[ "$spec" =~ ^# ]] && continue
  if [[ ! -f "$spec" ]]; then
    echo "skip missing spec: $spec" >&2
    continue
  fi
  safe_name="$(basename "$spec" | sed 's/[^A-Za-z0-9._-]/_/g')"
  run_out="$OUT_DIR/$safe_name"
  mkdir -p "$run_out"
  "$ROOT_DIR/tools/mcp/run_native_profile_closure_ladder.sh" "$spec" "$run_out" >/dev/null || true
done < "$SPEC_LIST_FILE"

python3 "$ROOT_DIR/tools/mcp/analyze_closure_ladder_outcomes.py" \
  --runs-root "$OUT_DIR" \
  --include-glob "*" \
  --out "$OUT_DIR/closure_ladder_batch_summary.json" >/dev/null

jq -n --arg out_dir "$OUT_DIR" --argjson summary "$(cat "$OUT_DIR/closure_ladder_batch_summary.json")" '{status:"ok", out_dir:$out_dir, summary:$summary}'
