#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SPEC_PATH="${1:-}"
if [[ -z "$SPEC_PATH" ]]; then
  echo "usage: $0 <spec_path> [out_dir]" >&2
  exit 2
fi
if [[ ! -f "$SPEC_PATH" ]]; then
  echo "error: spec not found: $SPEC_PATH" >&2
  exit 2
fi

OUT_DIR="${2:-$ROOT_DIR/logs/taskitem_runs/TEST_ONLY_native_impact_remediation_loop_$(date +%Y%m%d_%H%M%S)}"
mkdir -p "$OUT_DIR"

BASE_NAME="$(basename "$SPEC_PATH" .md)"

run_pipeline_once() {
  local extra_req_file="$1"
  local extra_task_file="$2"
  if [[ -n "$extra_req_file" || -n "$extra_task_file" ]]; then
    WSTONE_NATIVE_IMPACT_COVERAGE_GATE=1 \
    WSTONE_NATIVE_IMPACT_COVERAGE_ENFORCE=0 \
    WSTONE_EXTRA_NORMALIZED_REQUIREMENTS_FILE="$extra_req_file" \
    WSTONE_EXTRA_TASKS_FILE="$extra_task_file" \
    "$ROOT_DIR/tools/mcp/run_sprint_taskitem_pipeline.sh" "$SPEC_PATH" >/dev/null
  else
    WSTONE_NATIVE_IMPACT_COVERAGE_GATE=1 \
    WSTONE_NATIVE_IMPACT_COVERAGE_ENFORCE=0 \
    "$ROOT_DIR/tools/mcp/run_sprint_taskitem_pipeline.sh" "$SPEC_PATH" >/dev/null
  fi
  ls -1dt "$ROOT_DIR/logs/taskitem_runs/${BASE_NAME}_"* | head -n1
}

RUN1_DIR="$(run_pipeline_once "" "")"
RUN1_REPORT="$RUN1_DIR/06_native_impact_coverage.json"
if [[ ! -f "$RUN1_REPORT" ]]; then
  echo "error: missing first-run coverage report: $RUN1_REPORT" >&2
  exit 3
fi

REQ_FILE="$OUT_DIR/extra_normalized_requirements.json"
TASK_FILE="$OUT_DIR/extra_tasks.json"
python3 "$ROOT_DIR/tools/mcp/synthesize_native_impact_remediation_requirements.py" \
  --coverage-report "$RUN1_REPORT" \
  --profiles "${WSTONE_NATIVE_IMPACT_COVERAGE_PROFILES:-$ROOT_DIR/tools/mcp/profiles/native_decomposition_impact_profiles.json}" \
  --out "$REQ_FILE" >/dev/null

python3 "$ROOT_DIR/tools/mcp/synthesize_native_impact_remediation_tasks.py" \
  --coverage-report "$RUN1_REPORT" \
  --profiles "${WSTONE_NATIVE_IMPACT_COVERAGE_PROFILES:-$ROOT_DIR/tools/mcp/profiles/native_decomposition_impact_profiles.json}" \
  --out "$TASK_FILE" >/dev/null

RUN2_DIR="$(run_pipeline_once "$REQ_FILE" "$TASK_FILE")"
RUN2_REPORT="$RUN2_DIR/06_native_impact_coverage.json"
if [[ ! -f "$RUN2_REPORT" ]]; then
  echo "error: missing second-run coverage report: $RUN2_REPORT" >&2
  exit 4
fi

jq -n \
  --arg run1 "$RUN1_DIR" \
  --arg run2 "$RUN2_DIR" \
  --arg req_file "$REQ_FILE" \
  --arg task_file "$TASK_FILE" \
  --argjson first "$(cat "$RUN1_REPORT")" \
  --argjson second "$(cat "$RUN2_REPORT")" \
  '{
    status:"ok",
    run1:$run1,
    run2:$run2,
    remediation_requirements_file:$req_file,
    remediation_tasks_file:$task_file,
    before:{active_profile_count:$first.active_profile_count,failing_profile_count:$first.failing_profile_count,status:$first.status},
    after:{active_profile_count:$second.active_profile_count,failing_profile_count:$second.failing_profile_count,status:$second.status},
    delta:{failing_profile_count: (($second.failing_profile_count // 0) - ($first.failing_profile_count // 0))}
  }' | tee "$OUT_DIR/remediation_loop_summary.json"
