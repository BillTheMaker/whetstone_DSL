#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
RUNS_ROOT="${WSTONE_RUNS_ROOT:-$ROOT_DIR/logs/taskitem_runs}"
INCLUDE_GLOB="${WSTONE_SEMANTIC_FALLBACK_INCLUDE_GLOB:-*}"
OUT_DIR="${1:-$RUNS_ROOT/TEST_ONLY_semantic_fallback_budget_gate_$(date +%Y%m%d_%H%M%S)}"
MAX_RATE="${WSTONE_SEMANTIC_MAX_FALLBACK_RATE:-0.35}"
MIN_RECORDS="${WSTONE_SEMANTIC_FALLBACK_MIN_RECORDS:-5}"

mkdir -p "$OUT_DIR"

python3 "$ROOT_DIR/tools/mcp/analyze_semantic_fallback_gaps.py" \
  --runs-root "$RUNS_ROOT" \
  --include-glob "$INCLUDE_GLOB" \
  --out-dir "$OUT_DIR" >/dev/null

if python3 "$ROOT_DIR/tools/mcp/check_semantic_fallback_budget.py" \
  --summary "$OUT_DIR/semantic_fallback_summary.json" \
  --out "$OUT_DIR/semantic_fallback_budget_gate.json" \
  --max-fallback-rate "$MAX_RATE" \
  --min-record-count "$MIN_RECORDS" >/dev/null; then
  jq -n \
    --arg out_dir "$OUT_DIR" \
    --argjson gate "$(cat "$OUT_DIR/semantic_fallback_budget_gate.json")" \
    '{status:"pass", out_dir:$out_dir, gate:$gate}'
else
  jq -n \
    --arg out_dir "$OUT_DIR" \
    --argjson gate "$(cat "$OUT_DIR/semantic_fallback_budget_gate.json")" \
    '{status:"fail", out_dir:$out_dir, gate:$gate}'
  exit 9
fi
