#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
RUN_DIR_INPUT="${1:-}"
if [[ -z "$RUN_DIR_INPUT" ]]; then
  echo "usage: $0 <run_dir>" >&2
  exit 1
fi

if [[ "$RUN_DIR_INPUT" = /* ]]; then
  RUN_DIR="$RUN_DIR_INPUT"
else
  RUN_DIR="$ROOT_DIR/$RUN_DIR_INPUT"
fi

SUMMARY_FILE="$RUN_DIR/00_summary.json"
INTAKE_FILE="$RUN_DIR/01_intake.json"
GEN_FILE="$RUN_DIR/02_generate_taskitems.json"
QUEUE_FILE="$RUN_DIR/03_queue_ready.json"
VALIDATE_FILE="$RUN_DIR/04_validate_taskitem.json"

for f in "$SUMMARY_FILE" "$INTAKE_FILE" "$GEN_FILE" "$QUEUE_FILE" "$VALIDATE_FILE"; do
  if [[ ! -f "$f" ]]; then
    echo "error: missing file $f" >&2
    exit 2
  fi
done

OUT_DIR="$ROOT_DIR/training_data/lora"
OUT_FILE="$OUT_DIR/taskitem_pipeline_runs.jsonl"
mkdir -p "$OUT_DIR"

RECORD="$(
  jq -nc \
    --arg run_dir "$RUN_DIR" \
    --arg captured_at "$(date -u +%Y-%m-%dT%H:%M:%SZ)" \
    --argjson summary "$(cat "$SUMMARY_FILE")" \
    --argjson intake "$(cat "$INTAKE_FILE")" \
    --argjson generated "$(cat "$GEN_FILE")" \
    --argjson queue "$(cat "$QUEUE_FILE")" \
    --argjson validate "$(cat "$VALIDATE_FILE")" \
    '{
      schema_version: 1,
      capture_type: "taskitem_pipeline_run",
      captured_at: $captured_at,
      run_dir: $run_dir,
      summary: $summary,
      artifacts: {
        intake: $intake,
        generated_taskitems: $generated,
        queue_ready: $queue,
        validate_taskitem: $validate
      }
    }'
)"

printf '%s\n' "$RECORD" >> "$OUT_FILE"
echo "Appended LoRA capture record to:"
echo "  $OUT_FILE"
