#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT_DIR"

python3 tools/mcp/generate_tool_grammars.py \
  --schemas tools/mcp/whetstone_tool_schemas.json \
  --out-dir tools/mcp/grammars \
  --strict-report tools/mcp/grammars/strictness_report.json

python3 tools/mcp/audit_grammar_strictness.py \
  --schemas tools/mcp/whetstone_tool_schemas.json \
  --grammars-dir tools/mcp/grammars \
  --out tools/mcp/grammars/strictness_report.json

python3 tools/mcp/check_strictness_policy.py \
  --policy tools/mcp/grammars/strictness_policy.json \
  --report tools/mcp/grammars/strictness_report.json

python3 tools/mcp/verify_grammar_manifest.py \
  --schemas tools/mcp/whetstone_tool_schemas.json \
  --grammars-dir tools/mcp/grammars

echo "Grammar CI checks passed."
