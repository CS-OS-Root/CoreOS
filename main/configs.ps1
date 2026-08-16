# Common configuration definitions for building and running MLA tests/benchmarks (PowerShell)

if (-not $global:WORKSPACE_DIR) {
    $global:WORKSPACE_DIR = $PSScriptRoot
}

# Find Node.js
$global:NODE_BIN = ""
if (Get-Command "node" -ErrorAction SilentlyContinue) {
    $global:NODE_BIN = "node"
}

# Benchmark execution mode: 'regular' (default, fast CI mode) or 'deep' (slow profiling mode)
if (-not $global:BENCHMARK_MODE) {
    if ($env:BENCHMARK_MODE) {
        $global:BENCHMARK_MODE = $env:BENCHMARK_MODE
    } else {
        $global:BENCHMARK_MODE = "regular"
    }
}

$benchmarkIters = 5000
if ($global:BENCHMARK_MODE.ToLower() -eq "deep") {
    $benchmarkIters = 10000
}
if ($env:BENCHMARK_ITERATIONS) {
    $benchmarkIters = [int]$env:BENCHMARK_ITERATIONS
}

# Build configurations definitions
# Format: "config_name;C_COMPILER;CXX_COMPILER;EXTRA_CMAKE_FLAGS"
$global:BUILD_CONFIGS = @(
    "msvc;cl;cl;-DCMAKE_C_FLAGS=-Dmla_test_global_config_benchmark_iterations=$benchmarkIters -DCMAKE_CXX_FLAGS=-Dmla_test_global_config_benchmark_iterations=$benchmarkIters",
    "clang;clang;clang++;-DCMAKE_C_FLAGS=-Dmla_test_global_config_benchmark_iterations=$benchmarkIters -DCMAKE_CXX_FLAGS=-Dmla_test_global_config_benchmark_iterations=$benchmarkIters",
    "gcc;gcc;g++;-DCMAKE_C_FLAGS=-Dmla_test_global_config_benchmark_iterations=$benchmarkIters -DCMAKE_CXX_FLAGS=-Dmla_test_global_config_benchmark_iterations=$benchmarkIters",
    "zig_native;$global:WORKSPACE_DIR\lib\base-lib\build\tools\zig\zig-cc.bat;$global:WORKSPACE_DIR\lib\base-lib\build\tools\zig\zig-cxx.bat;-DCMAKE_C_FLAGS=-Dmla_test_global_config_benchmark_iterations=$benchmarkIters -DCMAKE_CXX_FLAGS=-Dmla_test_global_config_benchmark_iterations=$benchmarkIters"
)

# Test/Benchmark suite executable definitions
# Format: "config_name;runner_type;binary_relative_path"
# runner_type can be: native, node, node_wasm_dir, wasm, none
$global:RUN_SUITES = @(
    "msvc;native;build/msvc/mla-build-test",
    "clang;native;build/clang/mla-build-test",
    "gcc;native;build/gcc/mla-build-test",
    "zig_native;native;build/zig_native/mla-build-test"
)
