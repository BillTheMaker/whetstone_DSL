#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
RUN_DIR="${1:-}"
if [[ -z "$RUN_DIR" ]]; then
  echo "usage: $0 <run_dir> [out_json]" >&2
  exit 2
fi
OUT_JSON="${2:-$RUN_DIR/06_native_impact_coverage.json}"
PROFILES="${WSTONE_NATIVE_IMPACT_COVERAGE_PROFILES:-$ROOT_DIR/tools/mcp/profiles/native_decomposition_impact_profiles.json}"
ENFORCE="${WSTONE_NATIVE_IMPACT_COVERAGE_ENFORCE:-1}"

args=(
  --run-dir "$RUN_DIR"
  --profiles "$PROFILES"
  --out "$OUT_JSON"
)
if [[ "$ENFORCE" == "1" ]]; then
  args+=(--enforce)
fi

if python3 "$ROOT_DIR/tools/mcp/check_native_decomposition_impact_coverage.py" "${args[@]}" >/dev/null; then
  jq -n --arg run_dir "$RUN_DIR" --argjson report "$(cat "$OUT_JSON")" '{status:"pass", run_dir:$run_dir, report:$report}'
else
  jq -n --arg run_dir "$RUN_DIR" --argjson report "$(cat "$OUT_JSON")" '{status:"fail", run_dir:$run_dir, report:$report}'
  exit 12
fi
