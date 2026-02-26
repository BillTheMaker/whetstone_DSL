#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SPEC_INPUT="${1:?usage: run_spec_hardening_gate.sh <spec.md> [out_dir]}"
OUT_DIR="${2:-$ROOT_DIR/logs/taskitem_runs/spec_hardening_gate_$(date +%Y%m%d_%H%M%S)}"
MIN_SCORE="${WSTONE_SPEC_READINESS_MIN_SCORE:-65}"
RUN_PIPELINE="${WSTONE_SPEC_HARDENING_RUN_PIPELINE:-1}"

if [[ "$SPEC_INPUT" = /* ]]; then
  SPEC_PATH="$SPEC_INPUT"
else
  SPEC_PATH="$ROOT_DIR/$SPEC_INPUT"
fi

if [[ ! -f "$SPEC_PATH" ]]; then
  echo "error: spec not found: $SPEC_PATH" >&2
  exit 2
fi

mkdir -p "$OUT_DIR"

BEFORE_JSON="$OUT_DIR/before_readiness.json"
HARDENED_MD="$OUT_DIR/hardened_spec.md"
AFTER_JSON="$OUT_DIR/after_readiness.json"
REPORT_JSON="$OUT_DIR/report.json"

python3 "$ROOT_DIR/tools/mcp/spec_planning_readiness.py" --spec "$SPEC_PATH" --out "$BEFORE_JSON" >/dev/null
python3 "$ROOT_DIR/tools/mcp/spec_planning_hardener.py" --spec "$SPEC_PATH" --readiness "$BEFORE_JSON" --out "$HARDENED_MD" > "$OUT_DIR/hardener_report.json"
python3 "$ROOT_DIR/tools/mcp/spec_planning_readiness.py" --spec "$HARDENED_MD" --out "$AFTER_JSON" >/dev/null

ORIG_RC=null
HARD_RC=null
ORIG_LOG=""
HARD_LOG=""

if [[ "$RUN_PIPELINE" == "1" ]]; then
  ORIG_LOG="$OUT_DIR/original_pipeline.log"
  HARD_LOG="$OUT_DIR/hardened_pipeline.log"

  set +e
  WSTONE_RUN_READINESS_SUITE=0 WSTONE_SPEC_READINESS_PRECHECK=1 WSTONE_SPEC_READINESS_HARD_GATE=1 WSTONE_SPEC_READINESS_MIN_SCORE="$MIN_SCORE" \
    "$ROOT_DIR/tools/mcp/run_sprint_taskitem_pipeline.sh" "$SPEC_PATH" >"$ORIG_LOG" 2>&1
  ORIG_RC=$?

  WSTONE_RUN_READINESS_SUITE=0 WSTONE_SPEC_READINESS_PRECHECK=1 WSTONE_SPEC_READINESS_HARD_GATE=1 WSTONE_SPEC_READINESS_MIN_SCORE="$MIN_SCORE" \
    "$ROOT_DIR/tools/mcp/run_sprint_taskitem_pipeline.sh" "$HARDENED_MD" >"$HARD_LOG" 2>&1
  HARD_RC=$?
  set -e
fi

jq -nc \
  --arg spec "$SPEC_PATH" \
  --arg out_dir "$OUT_DIR" \
  --argjson before "$(cat "$BEFORE_JSON")" \
  --argjson after "$(cat "$AFTER_JSON")" \
  --argjson min_score "$MIN_SCORE" \
  --argjson run_pipeline "$RUN_PIPELINE" \
  --argjson original_rc "$ORIG_RC" \
  --argjson hardened_rc "$HARD_RC" \
  --arg original_log "$ORIG_LOG" \
  --arg hardened_log "$HARD_LOG" \
  '{
    spec:$spec,
    out_dir:$out_dir,
    readiness:{
      before:{score:$before.scores.total,verdict:$before.verdict,missing_count:($before.missing_actions|length)},
      after:{score:$after.scores.total,verdict:$after.verdict,missing_count:($after.missing_actions|length)},
      delta_score:($after.scores.total-$before.scores.total)
    },
    hard_gate:{
      min_score:$min_score,
      run_pipeline:($run_pipeline==1),
      original_rc:$original_rc,
      hardened_rc:$hardened_rc,
      original_log:$original_log,
      hardened_log:$hardened_log
    }
  }' > "$REPORT_JSON"

echo "Spec hardening gate complete."
echo "Output dir: $OUT_DIR"
cat "$REPORT_JSON"
