#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
START="${1:-}"
END="${2:-}"

if [[ -z "$START" || -z "$END" ]]; then
  echo "usage: $0 <start_sprint_number> <end_sprint_number>" >&2
  exit 1
fi

if ! [[ "$START" =~ ^[0-9]+$ && "$END" =~ ^[0-9]+$ ]]; then
  echo "error: start/end must be integers" >&2
  exit 2
fi

if (( START > END )); then
  echo "error: start must be <= end" >&2
  exit 3
fi

BATCH_STAMP="$(date +%Y%m%d_%H%M%S)"
BATCH_DIR="$ROOT_DIR/logs/taskitem_runs/batches"
mkdir -p "$BATCH_DIR"
BATCH_SUMMARY="$BATCH_DIR/batch_${START}_${END}_${BATCH_STAMP}.json"

success_count=0
fail_count=0
results='[]'

for n in $(seq "$START" "$END"); do
  sprint_file="sprint${n}_plan.md"
  abs_sprint="$ROOT_DIR/$sprint_file"
  if [[ ! -f "$abs_sprint" ]]; then
    fail_count=$((fail_count + 1))
    results="$(jq -nc --argjson prev "$results" --arg s "$sprint_file" \
      '$prev + [{sprint:$s,status:"missing"}]')"
    continue
  fi

  set +e
  run_out="$(cd "$ROOT_DIR" && ./tools/mcp/run_sprint_taskitem_pipeline.sh "$sprint_file" 2>&1)"
  rc=$?
  set -e

  if (( rc != 0 )); then
    fail_count=$((fail_count + 1))
    results="$(jq -nc --argjson prev "$results" --arg s "$sprint_file" --arg out "$run_out" \
      '$prev + [{sprint:$s,status:"failed",error:$out}]')"
    continue
  fi

  run_dir="$(printf '%s\n' "$run_out" | sed -n 's/^Output dir: //p' | tail -n1)"
  if [[ -z "$run_dir" || ! -d "$run_dir" ]]; then
    fail_count=$((fail_count + 1))
    results="$(jq -nc --argjson prev "$results" --arg s "$sprint_file" \
      '$prev + [{sprint:$s,status:"failed",error:"run_dir_not_found"}]')"
    continue
  fi

  set +e
  export_out="$(cd "$ROOT_DIR" && ./tools/mcp/export_taskitem_run_for_lora.sh "$run_dir" 2>&1)"
  ex_rc=$?
  set -e
  if (( ex_rc != 0 )); then
    fail_count=$((fail_count + 1))
    results="$(jq -nc --argjson prev "$results" --arg s "$sprint_file" --arg d "$run_dir" --arg out "$export_out" \
      '$prev + [{sprint:$s,status:"failed",run_dir:$d,error:$out}]')"
    continue
  fi

  success_count=$((success_count + 1))
  results="$(jq -nc --argjson prev "$results" --arg s "$sprint_file" --arg d "$run_dir" \
    '$prev + [{sprint:$s,status:"ok",run_dir:$d}]')"
done

jq -nc \
  --arg started_at "$(date -u +%Y-%m-%dT%H:%M:%SZ)" \
  --argjson start "$START" \
  --argjson end "$END" \
  --argjson ok "$success_count" \
  --argjson fail "$fail_count" \
  --argjson results "$results" \
  '{
    started_at: $started_at,
    sprint_range: {start: $start, end: $end},
    success_count: $ok,
    fail_count: $fail,
    results: $results
  }' > "$BATCH_SUMMARY"

echo "Batch complete."
echo "Summary: $BATCH_SUMMARY"
echo "Success: $success_count"
echo "Failed : $fail_count"
