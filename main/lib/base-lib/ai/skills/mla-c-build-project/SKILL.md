---
name: 'mla-c-build-project'
description: 'Instructions on how to build the project and run tests/benchmarks'
---

# Build and Run Instructions

This project uses **CMake** as its primary build system. Native platform builds are supported on Linux, Windows, and WebAssembly.

## Prerequisites

- CMake (version 3.25 or higher)
- A C++11 compatible compiler (GCC, Clang, or MSVC)
- A build generator (e.g., Ninja or Make)

## Compiler Compatibility

To maintain the portability of the MLA framework, the codebase must remain strictly compatible with all supported compiler toolchains across different platforms:

- **Linux:**
  - **GCC** (GNU Compiler Collection)
  - **Clang** (LLVM Clang compiler)
  - **Fil-C** (Memory-safe C/C++ compiler, `filcc`/`filcpp`)
  - **Zig** (Zig compiler in C/C++ mode)
- **Windows:**
  - **MSVC** (Microsoft Visual C++ compiler)
  - **Clang** (LLVM on Windows)
  - **Zig** (Zig compiler on Windows)
- **WebAssembly (WASM):**
  - **Emscripten** (`emcc`/`em++`)

Code changes must not use compiler-specific or non-standard features unless they are properly guarded behind platform/compiler macros, ensuring clean compilation across all of the above tools.

## Building the Project Natively

To build the test suites and example applications natively:

1. **Configure the build directory:**
   Using Ninja (recommended):
   ```bash
   cmake -G Ninja -B build
   ```
   Or using standard Makefiles:
   ```bash
   cmake -B build
   ```

2. **Compile all targets:**
   ```bash
   cmake --build build -j$(nproc)
   ```

### Major Build Targets

> [!NOTE]
> The actual build target names might differ depending on the parent repository or project configuration.

Depending on your platform, CMake generates the following targets:
- `MLA_C_Test_Linux_Single_Thread`: Linux unit/integration tests running in single-threaded mode.
- `MLA_C_Test_Linux_Multi_Thread`: Linux unit/integration tests running in multi-threaded mode.
- `MLA_C_App_Linux`: Example application demonstrating framework features on Linux.
- `MLA_C_Test_Windows_Single_Thread` / `MLA_C_Test_Windows_Multi_Thread`: Windows test suites.
- `MLA_C_App_Windows`: Windows example application (D2D/OpenGL).
- `MLA_C_Test_WASM_Single_Thread`: WebAssembly test suite via Emscripten.

---

## Running Tests

To run individual test suites, invoke the corresponding test binary with the `--test` flag:

```bash
./build/gcc/MLA_C_Test_Linux_Single_Thread --test
```

To run all tests across all compiled compiler configurations automatically, use the provided helper script:

```bash
./run_all_tests.sh
```

You can also run tests for a specific compiler configuration by passing its name:

```bash
./run_all_tests.sh gcc
```

> [!NOTE]
> `run_all_tests.sh` (via `run_tests_impl.sh`) automatically executes `build_all_impl.sh` to compile the target configuration before running test suites.

---

## Running Benchmarks

To run individual micro-benchmarks, invoke the test binary with the `--benchmark` flag:

```bash
./build/gcc/MLA_C_Test_Linux_Single_Thread --benchmark
```

By default, this will run all registered benchmarks and print the timing results to the console.

> [!WARNING]
> Benchmarks are slow and can take several minutes to run, especially for all configurations.

To run all benchmarks across all compiled compiler configurations automatically in regular (fast) mode:

```bash
./run_all_benchmarks.sh
```

To run in deep (comprehensive profiling) mode:

```bash
./run_all_benchmarks.sh --deep
```

You can also run benchmarks for a specific compiler configuration by passing its name:

```bash
./run_all_benchmarks.sh gcc
./run_all_benchmarks.sh gcc --deep
```

> [!NOTE]
> `run_all_benchmarks.sh` (via `run_benchmarks_impl.sh`) automatically executes `build_all_impl.sh` to compile the target configuration before running benchmarks. You can also select the mode via the `BENCHMARK_MODE=deep` environment variable.

---

## Common Configurations

All build and runner scripts share a common configuration file:

- `configs.sh` (defines `BUILD_CONFIGS` and `RUN_SUITES`)

The core execution logic is separated and stored within the base library tools folder:

- [build_all_impl.sh](file:///workspace/lib/base-lib/build/tools/build_all_impl.sh)
- [run_tests_impl.sh](file:///workspace/lib/base-lib/build/tools/run_tests_impl.sh) (automatically executes `build_all_impl.sh` before running tests)
- [run_benchmarks_impl.sh](file:///workspace/lib/base-lib/build/tools/run_benchmarks_impl.sh) (automatically executes `build_all_impl.sh` before running benchmarks)

The root scripts (`build_all.sh`, `run_all_tests.sh`, and `run_all_benchmarks.sh`) act as clean entry points that configure the environment by sourcing `configs.sh` and then delegate execution to the implementation scripts.

---

## Static Analysis (Clang-Tidy)

Clang-tidy is integrated into the build system and enabled by default (`MLA_ENABLE_CLANG_TIDY=ON`). It automatically analyzes code during compilation using the config in `lib/base-lib/build/.clang-tidy`.

To disable clang-tidy for faster builds:
```bash
cmake -DMLA_ENABLE_CLANG_TIDY=OFF -B build
```

## Critical Rules

1. **Never commit build artifacts** (always use `build/` directory which is gitignored).
2. **Never check in custom binary files or main templates.**
3. Verify that all tests pass locally before proposing changes.
4. **Definition of Done**: At the end of every task, all unit tests (`./run_all_tests.sh`) and all benchmarks in regular mode (`./run_all_benchmarks.sh`) MUST be executed and pass successfully across all supported compiler toolchains before declaring success (running in deep mode is optional and not required for DoD). The benchmark results must be summarized and checked for performance regressions related to the changes.
