# Common configuration definitions for building and running MLA tests/benchmarks

# Workspace root
WORKSPACE_DIR="${WORKSPACE_DIR:-/workspace}"

# Find Node.js
emsdk_node=$(ls -d /opt/emsdk/node/*/bin/node 2>/dev/null | head -n 1)
if [ -n "$emsdk_node" ] && [ -f "$emsdk_node" ]; then
    NODE_BIN="$emsdk_node"
elif command -v node >/dev/null 2>&1; then
    NODE_BIN="node"
else
    NODE_BIN=""
fi

# Benchmark execution mode: 'regular' (default, fast CI mode) or 'deep' (slow profiling mode)
BENCHMARK_MODE="${BENCHMARK_MODE:-regular}"
BENCHMARK_MODE="$(echo "$BENCHMARK_MODE" | tr '[:upper:]' '[:lower:]')"

if [ "$BENCHMARK_MODE" = "deep" ]; then
    BENCHMARK_ITERS="${BENCHMARK_ITERATIONS:-10000}"
    FILC_BENCHMARK_ITERS="${FILC_BENCHMARK_ITERATIONS:-1000}"
else
    BENCHMARK_MODE="regular"
    BENCHMARK_ITERS="${BENCHMARK_ITERATIONS:-5000}"
    FILC_BENCHMARK_ITERS="${FILC_BENCHMARK_ITERATIONS:-500}"
fi

# Build configurations definitions
# Format: "config_name;C_COMPILER;CXX_COMPILER;EXTRA_CMAKE_FLAGS"
BUILD_CONFIGS=(
    "gcc;gcc;g++;-DCMAKE_C_FLAGS=-Dmla_test_global_config_benchmark_iterations=${BENCHMARK_ITERS} -DCMAKE_CXX_FLAGS=-Dmla_test_global_config_benchmark_iterations=${BENCHMARK_ITERS}"
    "clang;clang;clang++;-DCMAKE_C_FLAGS=-Dmla_test_global_config_benchmark_iterations=${BENCHMARK_ITERS} -DCMAKE_CXX_FLAGS=-Dmla_test_global_config_benchmark_iterations=${BENCHMARK_ITERS}"
    "filc;filcc;fil++;-DCMAKE_C_FLAGS=-Dmla_test_global_config_benchmark_iterations=${FILC_BENCHMARK_ITERS} -DCMAKE_CXX_FLAGS=-Dmla_test_global_config_benchmark_iterations=${FILC_BENCHMARK_ITERS}"
    "zig_native;${WORKSPACE_DIR}/lib/base-lib/build/tools/zig/zig-cc.sh;${WORKSPACE_DIR}/lib/base-lib/build/tools/zig/zig-cxx.sh;-DCMAKE_C_FLAGS=-Dmla_test_global_config_benchmark_iterations=${BENCHMARK_ITERS} -DCMAKE_CXX_FLAGS=-Dmla_test_global_config_benchmark_iterations=${BENCHMARK_ITERS}"
    "emscripten_std;emcc;em++;-DMLA_EMSDK_PATH=/opt/emsdk -DCMAKE_C_FLAGS=-Dmla_test_global_config_benchmark_iterations=${BENCHMARK_ITERS} -DCMAKE_CXX_FLAGS=-Dmla_test_global_config_benchmark_iterations=${BENCHMARK_ITERS}"
    "emscripten_js;emcc;em++;-DMLA_EMSDK_PATH=/opt/emsdk -DMLA_JS_STANDALONE=ON -DCMAKE_C_FLAGS=-Dmla_test_global_config_benchmark_iterations=${BENCHMARK_ITERS} -DCMAKE_CXX_FLAGS=-Dmla_test_global_config_benchmark_iterations=${BENCHMARK_ITERS}"
    "zig_wasm;${WORKSPACE_DIR}/lib/base-lib/build/tools/zig/zig-cc.sh;${WORKSPACE_DIR}/lib/base-lib/build/tools/zig/zig-cxx.sh;-DMLA_WASM_STANDALONE=ON -DCMAKE_C_FLAGS=-Dmla_test_global_config_benchmark_iterations=${BENCHMARK_ITERS} -DCMAKE_CXX_FLAGS=-Dmla_test_global_config_benchmark_iterations=${BENCHMARK_ITERS}"
)

# Test/Benchmark suite executable definitions
# Format: "config_name;runner_type;binary_relative_path"
# runner_type can be: native, node, node_wasm_dir, wasm, none
RUN_SUITES=(
    "gcc;native;build/gcc/MLA_C_Test_Linux_Single_Thread"
    "gcc;native;build/gcc/MLA_C_Test_Linux_Multi_Thread"
    "clang;native;build/clang/MLA_C_Test_Linux_Single_Thread"
    "clang;native;build/clang/MLA_C_Test_Linux_Multi_Thread"
    "zig_native;native;build/zig_native/MLA_C_Test_Linux_Single_Thread"
    "zig_native;native;build/zig_native/MLA_C_Test_Linux_Multi_Thread"
    "filc;native;build/filc/MLA_C_Test_Linux_Single_Thread"
    "filc;native;build/filc/MLA_C_Test_Linux_Multi_Thread"
    "emscripten_std;node;build/emscripten_std/MLA_C_Test_WASM_Single_Thread.js"
    "emscripten_js;node;build/emscripten_js/MLA_C_Test_WASM_Single_Thread.js"
    "zig_wasm;wasm;build/zig_wasm/MLA_C_Test_WASM_Single_Thread_Standalone.wasm"
)
