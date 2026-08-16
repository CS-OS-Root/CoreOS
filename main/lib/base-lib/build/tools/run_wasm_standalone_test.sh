#!/usr/bin/env bash
#
# run_wasm_standalone_test.sh
#
# Run a standalone WASM binary using the MLA Node.js WASM runner.
# Provides the mla and mla_test import modules automatically.
#
# Usage:
#   run_wasm_standalone_test.sh <path-to-wasm-file> [--test|--benchmark]
#

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORKSPACE_DIR="$(cd "$SCRIPT_DIR/../../../.." && pwd)"

WASM_RUNNER="$WORKSPACE_DIR/lib/base-lib/platform/wasm/javascript/mla_wasm_runner.js"
WASM_TEST_MODULE="$WORKSPACE_DIR/lib/base-lib/test-support/platform/mla_wasm_test_module.js"

WASM_FILE="$1"
RUN_FLAG="${2:---test}"

if [ -z "$WASM_FILE" ]; then
    echo "Usage: $0 <path-to-wasm-file> [--test|--benchmark]"
    exit 1
fi

# Find Node.js (honour NODE_BIN if set, then emsdk node, then system node)
if [ -z "$NODE_BIN" ]; then
    emsdk_node=$(ls -d /opt/emsdk/node/*/bin/node 2>/dev/null | head -n 1)
    if [ -n "$emsdk_node" ] && [ -f "$emsdk_node" ]; then
        NODE_BIN="$emsdk_node"
    elif command -v node >/dev/null 2>&1; then
        NODE_BIN="node"
    else
        echo "Error: Node.js not found — cannot run WASM standalone binary."
        exit 1
    fi
fi

exec "$NODE_BIN" "$WASM_RUNNER" "$WASM_FILE" --custom "$WASM_TEST_MODULE" "$RUN_FLAG"
