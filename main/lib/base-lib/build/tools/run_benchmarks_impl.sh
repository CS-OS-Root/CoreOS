#!/usr/bin/env bash

# Resolve workspace directory
if [ -z "$WORKSPACE_DIR" ]; then
    WORKSPACE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../.." && pwd)"
fi

target_config=""
for arg in "$@"; do
    case "$arg" in
        --deep|deep)
            BENCHMARK_MODE="deep"
            ;;
        --regular|regular)
            BENCHMARK_MODE="regular"
            ;;
        --mode=*)
            BENCHMARK_MODE="${arg#*=}"
            ;;
        *)
            if [ -z "$target_config" ]; then
                target_config="$arg"
            fi
            ;;
    esac
done

export BENCHMARK_MODE

# Execute build phase before running benchmarks
source "${WORKSPACE_DIR}/lib/base-lib/build/tools/build_all_impl.sh" "$target_config"
build_exit_code=$?
if [ $build_exit_code -ne 0 ]; then
    echo "Error: Build phase failed with exit code $build_exit_code — aborting benchmarks."
    return $build_exit_code 2>/dev/null || exit $build_exit_code
fi

passed_count=0
failed_count=0
skipped_count=0
failed_suites=()

for suite in "${RUN_SUITES[@]}"; do
    IFS=';' read -r config_name runner_type binary_path <<< "$suite"

    # If a specific configuration was requested, skip others
    if [ -n "$target_config" ] && [ "$target_config" != "$config_name" ]; then
        continue
    fi

    full_binary_path="$WORKSPACE_DIR/$binary_path"

    if [ ! -f "$full_binary_path" ]; then
        echo "Warning: Binary not found: $full_binary_path — skipping $config_name ($binary_path)"
        skipped_count=$((skipped_count + 1))
        continue
    fi

    echo "========================================================================"
    echo "Running Benchmarks: $config_name (Mode: ${BENCHMARK_MODE:-regular})"
    echo "Runner:             $runner_type"
    echo "Binary:             $full_binary_path"
    echo "========================================================================"

    case "$runner_type" in
        native)
            "$full_binary_path" --benchmark
            exit_code=$?
            ;;
        node)
            if [ -n "$NODE_BIN" ]; then
                "$NODE_BIN" "$full_binary_path" --benchmark
                exit_code=$?
            else
                echo "Warning: Node.js not found, cannot run $binary_path — skipping."
                skipped_count=$((skipped_count + 1))
                continue
            fi
            ;;
        node_wasm_dir)
            if [ -n "$NODE_BIN" ]; then
                binary_dir="$(dirname "$full_binary_path")"
                binary_name="$(basename "$full_binary_path")"
                (cd "$binary_dir" && "$NODE_BIN" "$binary_name" --benchmark)
                exit_code=$?
            else
                echo "Warning: Node.js not found, cannot run $binary_path — skipping."
                skipped_count=$((skipped_count + 1))
                continue
            fi
            ;;
        wasm)
            "$WORKSPACE_DIR/lib/base-lib/build/tools/run_wasm_standalone_test.sh" "$full_binary_path" --benchmark
            exit_code=$?
            ;;
        none)
            echo "Skipping $binary_path (runner type 'none' — no standalone runner available)."
            skipped_count=$((skipped_count + 1))
            continue
            ;;
        *)
            echo "Error: Unknown runner type '$runner_type' for $config_name"
            failed_suites+=("$config_name ($binary_path)")
            failed_count=$((failed_count + 1))
            continue
            ;;
    esac

    if [ $exit_code -eq 0 ]; then
        passed_count=$((passed_count + 1))
    else
        echo "Error: Benchmarks FAILED for $config_name ($binary_path) — exit code $exit_code"
        failed_suites+=("$config_name ($binary_path)")
        failed_count=$((failed_count + 1))
    fi
done

echo "========================================================================"
echo "Benchmark Run Summary:"
echo "  Passed:  $passed_count suites"
echo "  Failed:  $failed_count suites"
echo "  Skipped: $skipped_count suites"
if [ ${#failed_suites[@]} -ne 0 ]; then
    echo "  Failed suites:"
    for s in "${failed_suites[@]}"; do
        echo "    - $s"
    done
    return 1 2>/dev/null || exit 1
else
    echo "All benchmark suites completed successfully!"
    return 0 2>/dev/null || exit 0
fi
