#!/usr/bin/env bash

# Resolve workspace directory
WORKSPACE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Parse benchmark mode and target configuration from arguments
BENCHMARK_MODE="${BENCHMARK_MODE:-regular}"
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

# Source definitions and implementation
source "${WORKSPACE_DIR}/configs.sh"
source "${WORKSPACE_DIR}/lib/base-lib/build/tools/run_benchmarks_impl.sh" "$target_config"
