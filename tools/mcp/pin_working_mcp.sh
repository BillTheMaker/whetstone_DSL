#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="$ROOT_DIR/editor/build-native"
SRC_BIN="$BUILD_DIR/whetstone_mcp"
RELEASE_DIR="$BUILD_DIR/releases"
STAMP="$(date +%Y%m%d_%H%M%S)"
DEST_BIN="$RELEASE_DIR/whetstone_mcp_${STAMP}"
STABLE_LINK="$BUILD_DIR/whetstone_mcp_stable"

if [[ ! -x "$SRC_BIN" ]]; then
  echo "error: missing executable $SRC_BIN" >&2
  exit 1
fi

mkdir -p "$RELEASE_DIR"
cp "$SRC_BIN" "$DEST_BIN"
chmod +x "$DEST_BIN"
ln -sfn "$DEST_BIN" "$STABLE_LINK"

echo "Pinned working MCP binary:"
echo "  release: $DEST_BIN"
echo "  stable : $STABLE_LINK"
