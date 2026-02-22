#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="$ROOT_DIR/editor/build-native"
SRC_BIN="$BUILD_DIR/whetstone_mcp"
RELEASE_DIR="$BUILD_DIR/releases"
STABLE_LINK="$BUILD_DIR/whetstone_mcp_stable"
STAMP="$(date +%Y%m%d_%H%M%S)"
DEST_BIN="$RELEASE_DIR/whetstone_mcp_${STAMP}"

if [[ ! -x "$SRC_BIN" ]]; then
  echo "error: missing executable $SRC_BIN" >&2
  exit 1
fi

mkdir -p "$RELEASE_DIR"

# Basic liveness check: initialize handshake should include protocolVersion.
REQ='{"jsonrpc":"2.0","id":1,"method":"initialize","params":{"protocolVersion":"2025-11-25","capabilities":{},"clientInfo":{"name":"promote-script","version":"1.0"}}}'
OUT="$(printf '%s\n' "$REQ" | timeout 5s "$SRC_BIN" 2>/dev/null || true)"
if [[ "$OUT" != *"protocolVersion"* ]]; then
  echo "error: whetstone_mcp initialize health check failed" >&2
  exit 2
fi

cp "$SRC_BIN" "$DEST_BIN"
chmod +x "$DEST_BIN"
ln -sfn "$DEST_BIN" "$STABLE_LINK"

echo "Promoted healthy MCP binary:"
echo "  release: $DEST_BIN"
echo "  stable : $STABLE_LINK"
