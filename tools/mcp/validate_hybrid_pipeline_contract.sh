#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'USAGE'
Usage:
  validate_hybrid_pipeline_contract.sh [--start N] [--end N] [--strict] [--enforce-non-cpp] [--json-out FILE]

Checks sprint integration summaries for hybrid pipeline signals:
  1) canonical signal (AST/IR/schema/semantic/model)
  2) non-C++ projection signal (Rust/Go/Java/Python/etc.)

Options:
  --start N            Start sprint number (default: 50)
  --end N              End sprint number (default: 90)
  --strict             Exit non-zero if any sprint fails active checks
  --enforce-non-cpp    Require non-C++ projection signal (otherwise warning-only)
  --json-out FILE      Write JSON summary to FILE
USAGE
}

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SRC_DIR="$ROOT_DIR/editor/src"

START=50
END=90
STRICT=0
ENFORCE_NON_CPP=0
JSON_OUT=""

while [[ $# -gt 0 ]]; do
  case "$1" in
    --start)
      START="${2:-}"
      shift 2
      ;;
    --end)
      END="${2:-}"
      shift 2
      ;;
    --strict)
      STRICT=1
      shift
      ;;
    --enforce-non-cpp)
      ENFORCE_NON_CPP=1
      shift
      ;;
    --json-out)
      JSON_OUT="${2:-}"
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "error: unknown argument: $1" >&2
      usage >&2
      exit 2
      ;;
  esac
done

if ! [[ "$START" =~ ^[0-9]+$ && "$END" =~ ^[0-9]+$ ]]; then
  echo "error: --start/--end must be integers" >&2
  exit 2
fi
if (( START > END )); then
  echo "error: --start cannot be greater than --end" >&2
  exit 2
fi

canonical_re='(AST|IR|Schema|Canonical|Semantic|Model)'
non_cpp_re='(Rust|Go|Java|Python|TypeScript|JavaScript|Kotlin|CSharp|FSharp|Ruby|Lua|Erlang|Elixir|Prolog|PostgreSql|MySql|TSql|Sql|Wasm|ARM|x86|Swift|Scala|CAdapter|CRaising|VbNet)'
cpp_re='(Cpp|C\+\+)'

missing=0
failed=0
warn_non_cpp=0
checked=0
json_rows=""

printf '%-8s %-9s %-9s %-9s %-9s %-s\n' "Sprint" "Summary" "Canonical" "NonCpp" "Result" "Theme"

for sprint in $(seq "$START" "$END"); do
  file="$SRC_DIR/Sprint${sprint}IntegrationSummary.h"
  if [[ ! -f "$file" ]]; then
    ((missing+=1))
    ((failed+=1))
    printf '%-8s %-9s %-9s %-9s %-9s %-s\n' "$sprint" "missing" "-" "-" "FAIL" "-"
    json_rows+="{\"sprint\":$sprint,\"summary\":\"missing\",\"canonical\":false,\"non_cpp\":false,\"result\":\"fail\",\"theme\":\"\"},"
    continue
  fi

  ((checked+=1))
  theme="$(sed -nE 's/.*theme\s*=\s*"([^"]+)".*/\1/p' "$file" | head -n1)"
  if [[ -z "$theme" ]]; then
    theme="-"
  fi

  canonical="no"
  non_cpp="no"
  result="PASS"

  if rg -q "$canonical_re" "$file"; then
    canonical="yes"
  fi

  if rg -q "$non_cpp_re" "$file"; then
    non_cpp="yes"
  fi

  if [[ "$canonical" != "yes" ]]; then
    result="FAIL"
  fi

  if (( ENFORCE_NON_CPP == 1 )) && [[ "$non_cpp" != "yes" ]]; then
    result="FAIL"
  fi

  if [[ "$result" == "FAIL" ]]; then
    ((failed+=1))
  elif [[ "$non_cpp" != "yes" ]]; then
    ((warn_non_cpp+=1))
  fi

  printf '%-8s %-9s %-9s %-9s %-9s %-s\n' "$sprint" "present" "$canonical" "$non_cpp" "$result" "$theme"
  json_rows+="{\"sprint\":$sprint,\"summary\":\"present\",\"canonical\":$([[ "$canonical" == "yes" ]] && echo true || echo false),\"non_cpp\":$([[ "$non_cpp" == "yes" ]] && echo true || echo false),\"result\":\"$(echo "$result" | tr '[:upper:]' '[:lower:]')\",\"theme\":\"${theme//\"/\\\"}\"},"
done

summary_json="$(cat <<JSON
{
  "range": {"start": $START, "end": $END},
  "enforce_non_cpp": $([[ "$ENFORCE_NON_CPP" -eq 1 ]] && echo true || echo false),
  "checked": $checked,
  "missing": $missing,
  "failed": $failed,
  "warn_non_cpp": $warn_non_cpp,
  "rows": [${json_rows%,}]
}
JSON
)"

echo
echo "Summary: checked=$checked missing=$missing failed=$failed warn_non_cpp=$warn_non_cpp enforce_non_cpp=$ENFORCE_NON_CPP"

if [[ -n "$JSON_OUT" ]]; then
  mkdir -p "$(dirname "$JSON_OUT")"
  printf '%s\n' "$summary_json" > "$JSON_OUT"
  echo "JSON written: $JSON_OUT"
fi

if (( STRICT == 1 )) && (( failed > 0 )); then
  exit 1
fi

