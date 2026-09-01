---
name: coreos-benchmark
description: Patterns for creating and registering benchmarks in the MLA framework. Use when measuring performance of MLA operations, comparing implementations, or tracking performance regressions.
metadata:
  author: coreos
  version: "1.0"
  converted-from: ai/instructions/benchmark.instructions.md
---

# Benchmarks

The benchmark module (`core-os-test-support/Benchmark/`) provides a micro-benchmarking framework for measuring the performance of MLA operations. Benchmarks live alongside tests in `core-os-test/` and are registered via `mla_benchmark_executor_t`.

## When to Add a Benchmark

Add benchmarks whenever:
- A new data structure or algorithm is introduced and you need to track its throughput.
- You want to compare MLA framework operations against native/standard-library equivalents.
- A performance-critical code path is modified and regression tracking is needed.

## Benchmark Infrastructure

### Key Types

| Type | Purpose |
|---|---|
| `mla_benchmark_t` | Describes a single benchmark (name, category, run/setUp/tearDown functions) |
| `mla_benchmark_executor_t` | Collects and executes benchmarks |

### Constants

| Constant | Default | Purpose |
|---|---|---|
| `CONST_CPU_WARMUP_ITERATIONS` | 1 000 000 | Iterations used to warm up the CPU before measuring |
| `CONST_BENCHMARK_ITERATIONS` | 1 000 000 | Iterations used for the actual measurement |

Override both at compile time with `-Dmla_benchmark_iternations=<N>` to speed up CI runs.

### Category Macro

Every benchmark file uses `benchmark_category` which expands to the current filename via:

```cpp
#define benchmark_category mla_test__FILENAME_ONLY__
```

This groups benchmarks automatically by source file in the output.

## Creating a Benchmark

### Step 1 — Write the Benchmark Function

A benchmark function has the signature `void (*)(void)`. It performs the operation to be measured **once per call**; the executor invokes it `CONST_BENCHMARK_ITERATIONS` times.

```cpp
void MyOperationBenchmark() {
    // Set up minimal state
    mla_string_t str = mla_string_const("Hello");
    mla_string_t result = mla_string_concat(str, mla_string_const(" World"));

    // Prevent dead-code elimination
    mla_test_int32_t length = mla_string_length(result);
    (void)length;

    // Clean up if the operation allocates
    mla_string_destroy(result);
}
```

> **Tip:** Keep benchmark functions as lean as possible — only include the code you want to measure plus the minimum setup/teardown needed to avoid side-effects between iterations.

### Step 2 — (Optional) Add setUp / tearDown Functions

When a benchmark requires expensive one-time initialisation or cleanup, provide separate `setUp` and `tearDown` callbacks. These run **once** — before all iterations start and after all iterations finish, respectively.

```cpp
static mla_hash_map_t<short, short, mla_int16_hash_t> benchMap = mla_hash_map_empty<short, short, mla_int16_hash_t>();

void SetupHashMapBenchmark() {
    benchMap = mla_hash_map<short, short, mla_int16_hash_t>(16);
    for (short i = 0; i < 100; ++i) {
        mla_hash_map_push(benchMap, i, i);
    }
}

void TearDownHashMapBenchmark() {
    benchMap = mla_hash_map_empty<short, short, mla_int16_hash_t>();
}

void HashMapLookupBenchmark() {
    mla_bool_t found = mla_hash_map_contains(benchMap, (short)50);
    (void)found;
}
```

### Step 3 — Register the Benchmark

Create a `Register<Module>Benchmarks(mla_benchmark_executor_t&)` function and register each benchmark with `mla_benchmark_executor_register`.

```cpp
void RegisterMyModuleBenchmarks(mla_benchmark_executor_t &p_BenchmarkExecutor) {
    mla_benchmark_t benchmark = mla_benchmark("MyOperation", benchmark_category, MyOperationBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    // Benchmark with setUp / tearDown
    benchmark = mla_benchmark("HashMapLookup", benchmark_category, HashMapLookupBenchmark,
                              SetupHashMapBenchmark, TearDownHashMapBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
}
```

### Step 4 — Wire into `main_test.h`

Add the registration call in `main_test.h` alongside the existing benchmark registrations:

```cpp
mla_benchmark_executor_t l_BenchmarkExecutor = mla_benchmark_executor(100);
// …existing registrations…
RegisterMyModuleBenchmarks(l_BenchmarkExecutor);
```

## Iteration Division

When a single benchmark iteration already loops internally (e.g., searching 100 items), use `mla_benchmark_set_iteration_division` so the executor divides total iterations accordingly:

```cpp
mla_benchmark_t benchmark = mla_benchmark("Contains", benchmark_category, ContainsBenchmark,
                                          SetupContains, TearDownContains);
mla_benchmark_set_iteration_division(benchmark, 100);
mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
```

Common division values used in the project: `10`, `100`, `1000`.

## Preventing Dead-Code Elimination

Compilers may optimise away unused results. Use one of these patterns:

```cpp
// Cast to void
mla_size_t hash = mla_string_hash(str, len);
(void)hash;

// Touch through a helper
static inline void bench_touch(mla_int32_t v) { (void)v; }
bench_touch(result.value);

// Use volatile read
volatile mla_byte_t temp = buffer[0];
(void)temp;
```

## Rules

- Benchmark functions must be **deterministic** — no random input or wall-clock dependencies.
- Keep benchmark bodies **minimal**: only the operation under test plus guard against dead-code elimination.
- Use `setUp` / `tearDown` for any allocation that is not part of the measured operation.
- Use `mla_benchmark_set_iteration_division` when the benchmark body loops internally to keep reported per-iteration times accurate.
- Follow all data type rules — use MLA types, MLA memory functions, and no standard library headers in MLA benchmarks. Native comparison benchmarks are the only exception.
- Register new benchmarks in `main_test.h` inside the benchmark executor section.
- Benchmarks for a module must live in the **same file** as the module's tests (e.g., `mla_my_module_test.h`). Place all benchmark functions and the `Register<Module>Benchmarks` function at the **end of the file**, after all test functions and the `Register<Module>Tests` function.
