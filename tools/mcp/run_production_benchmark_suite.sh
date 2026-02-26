#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
OUT_DIR="${OUT_DIR:-$ROOT_DIR/logs/taskitem_runs/production_benchmark_$(date +%Y%m%d_%H%M%S)}"
RUNS="${RUNS:-3}"
STRICT_MODE="${STRICT_MODE:-1}"
LANGUAGE="${WSTONE_LANGUAGE:-cpp}"
mkdir -p "$OUT_DIR"

spec_a="Generate WorkItem and PriorityQueue classes with enqueue, dequeue, peek, size, empty"
spec_b="Generate DataStore class with put/get/delete/contains methods and concrete typed fields"

run_case() {
  local name="$1"
  local spec="$2"
  local pass=0
  local blocked=0
  local total_ms=0

  for i in $(seq 1 "$RUNS"); do
    local run_dir="$OUT_DIR/${name}_run_${i}"
    mkdir -p "$run_dir"
    local start_ms
    start_ms=$(date +%s%3N)
    OUT_DIR="$run_dir" STRICT_MODE="$STRICT_MODE" WSTONE_LANGUAGE="$LANGUAGE" \
      "$ROOT_DIR/tools/mcp/run_production_completion_loop.sh" "$spec" > "$run_dir/stdout.log"
    local end_ms
    end_ms=$(date +%s%3N)
    local dur=$((end_ms - start_ms))
    total_ms=$((total_ms + dur))

    local status
    status=$(jq -r '.status' "$run_dir/00_summary.json")
    if [[ "$status" == "green" ]]; then
      pass=$((pass + 1))
    else
      blocked=$((blocked + 1))
    fi
  done

  jq -nc \
    --arg name "$name" \
    --arg spec "$spec" \
    --argjson runs "$RUNS" \
    --argjson pass "$pass" \
    --argjson blocked "$blocked" \
    --argjson avg_ms "$((total_ms / RUNS))" \
    '{name:$name,spec:$spec,runs:$runs,pass:$pass,blocked:$blocked,pass_rate:($pass/$runs),avg_latency_ms:$avg_ms}'
}

case_a=$(run_case "priorityqueue" "$spec_a")
case_b=$(run_case "datastore" "$spec_b")

jq -nc \
  --arg out_dir "$OUT_DIR" \
  --arg language "$LANGUAGE" \
  --argjson strict_mode "$STRICT_MODE" \
  --argjson a "$case_a" \
  --argjson b "$case_b" \
  '{
    out_dir:$out_dir,
    language:$language,
    strict_mode:($strict_mode==1),
    cases:[$a,$b]
  }' > "$OUT_DIR/00_benchmark_summary.json"

echo "Benchmark suite output: $OUT_DIR"
cat "$OUT_DIR/00_benchmark_summary.json"
