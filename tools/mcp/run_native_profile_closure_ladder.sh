#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SPEC_PATH="${1:-}"
OUT_DIR="${2:-$ROOT_DIR/logs/taskitem_runs/TEST_ONLY_native_profile_closure_ladder_$(date +%Y%m%d_%H%M%S)}"
if [[ -z "$SPEC_PATH" ]]; then
  echo "usage: $0 <spec_path> [out_dir]" >&2
  exit 2
fi
if [[ ! -f "$SPEC_PATH" ]]; then
  echo "error: spec not found: $SPEC_PATH" >&2
  exit 2
fi
mkdir -p "$OUT_DIR"

SKIP_RAW_WHEN_NO_UPLIFT="${WSTONE_CLOSURE_LADDER_SKIP_RAW_WHEN_NO_UPLIFT:-1}"
RAW_HISTORY_GLOB="${WSTONE_CLOSURE_LADDER_RAW_HISTORY_GLOB:-TEST_ONLY_sprint24*_raw*}"
RAW_HISTORY_MAX_RUNS="${WSTONE_CLOSURE_LADDER_RAW_HISTORY_MAX_RUNS:-20}"
RAW_MIN_UPLIFT_RATE="${WSTONE_CLOSURE_LADDER_RAW_MIN_UPLIFT_RATE:-0.05}"
RAW_MIN_RECORDS="${WSTONE_CLOSURE_LADDER_RAW_MIN_RECORDS:-3}"

effective_modes=(raw_only single_shot_shape multishot autofill)
if [[ "$SKIP_RAW_WHEN_NO_UPLIFT" == "1" ]]; then
  python3 "$ROOT_DIR/tools/mcp/analyze_raw_candidate_uplift_history.py" \
    --runs-root "$ROOT_DIR/logs/taskitem_runs" \
    --include-glob "$RAW_HISTORY_GLOB" \
    --max-runs "$RAW_HISTORY_MAX_RUNS" \
    --out "$OUT_DIR/raw_uplift_history.json" >/dev/null || true
  if [[ -f "$OUT_DIR/raw_uplift_history.json" ]]; then
    raw_record_count="$(jq '.record_count // 0' "$OUT_DIR/raw_uplift_history.json")"
    raw_uplift_rate="$(jq '.uplift_rate // 0' "$OUT_DIR/raw_uplift_history.json")"
    if [[ "$raw_record_count" -ge "$RAW_MIN_RECORDS" ]]; then
      awk_cmp="$(awk -v a="$raw_uplift_rate" -v b="$RAW_MIN_UPLIFT_RATE" 'BEGIN{if (a < b) print 1; else print 0;}')"
      if [[ "$awk_cmp" == "1" ]]; then
        effective_modes=(single_shot_shape multishot autofill)
      fi
    fi
  fi
fi

run_mode() {
  local mode="$1"
  local logfile="$OUT_DIR/${mode}.log"
  local rcfile="$OUT_DIR/${mode}.rc"

  local env_common=(
    WSTONE_NATIVE_IMPACT_COVERAGE_GATE=1
    WSTONE_NATIVE_IMPACT_COVERAGE_ENFORCE=1
    WSTONE_NATIVE_INTRINSIC_BOOST=1
    WSTONE_NATIVE_RAW_CANDIDATE_SEARCH=0
    WSTONE_NATIVE_RAW_CANDIDATE_REQUIRE_UPLIFT=0
    WSTONE_NATIVE_SINGLESHOT_PROFILE_SHAPE=0
    WSTONE_NATIVE_MULTISHOT_DECOMP=0
    WSTONE_NATIVE_PROFILE_AUTOFILL=0
  )

  case "$mode" in
    raw_only)
      env_common+=(WSTONE_NATIVE_RAW_CANDIDATE_SEARCH=1)
      ;;
    single_shot_shape)
      env_common+=(WSTONE_NATIVE_SINGLESHOT_PROFILE_SHAPE=1)
      ;;
    multishot)
      env_common+=(WSTONE_NATIVE_MULTISHOT_DECOMP=1)
      ;;
    autofill)
      env_common+=(WSTONE_NATIVE_PROFILE_AUTOFILL=1)
      ;;
    *)
      echo "unknown mode: $mode" >&2
      exit 3
      ;;
  esac

  set +e
  env "${env_common[@]}" "$ROOT_DIR/tools/mcp/run_sprint_taskitem_pipeline.sh" "$SPEC_PATH" >"$logfile" 2>&1
  local rc=$?
  set -e
  echo "$rc" > "$rcfile"

  local latest
  latest=$(ls -1dt "$ROOT_DIR/logs/taskitem_runs/$(basename "$SPEC_PATH" .md)_"* | head -n1)
  echo "$latest" > "$OUT_DIR/${mode}.run_dir"

  if [[ "$rc" -eq 0 ]]; then
    return 0
  fi
  return "$rc"
}

selected=""
selected_run=""
selected_rc=1

for mode in "${effective_modes[@]}"; do
  if run_mode "$mode"; then
    selected="$mode"
    selected_run="$(cat "$OUT_DIR/${mode}.run_dir")"
    selected_rc=0
    break
  fi
done

jq -n \
  --arg spec "$SPEC_PATH" \
  --arg out_dir "$OUT_DIR" \
  --arg selected_mode "$selected" \
  --arg selected_run "$selected_run" \
  --argjson selected_rc "$selected_rc" \
  --argjson attempted_modes "$(printf '%s\n' "${effective_modes[@]}" | jq -R . | jq -s .)" \
  '{
    status: (if $selected_rc == 0 then "ok" else "fail" end),
    spec:$spec,
    out_dir:$out_dir,
    attempted_modes:$attempted_modes,
    selected_mode:$selected_mode,
    selected_run:$selected_run,
    selected_rc:$selected_rc
  }' | tee "$OUT_DIR/closure_ladder_summary.json"

if [[ "$selected_rc" -ne 0 ]]; then
  exit 21
fi
