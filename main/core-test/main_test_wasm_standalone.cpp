//
// Created by christian on 1/8/26.
//

#include "../lib/base-lib/platform/wasm/mla_global_platform_wasm_standalone.h"
#include "../lib/base-lib/platform/generic/mla_global_config_inmemory.h"
#include "../lib/base-lib/platform/generic/mla_global_service_disabled.h"
#include "../lib/base-lib/platform/wasm/mla_global_platform_task_manager_wasm.h"


#include "../lib/base-lib/test-support/mla_test_data_types.h"
#include "../lib/base-lib/test-support/platform/mla_test_memory_external.h"
#include "../lib/base-lib/test-support/platform/mla_benchmark_timer_external.h"
#include "../lib/base-lib/test-support/platform/mla_test_print_external.h"
#include "../lib/base-lib/test-support/platform/mla_test_mutex_single_thread.h"

#include "../lib/base-lib/test-support/mla_test_cli_utils.h"
#include "main_test.h"

#ifdef __EMSCRIPTEN__
int main(int argc, char** argv) {
#else
extern "C" int main(int argc, char** argv) { // NOLINT
#endif

    mla_test_bool_t runTest;
    mla_test_bool_t runBenchmark;
    mla_test_output_format_t benchmarkOutputFormat;

    mla_test_parse_cmd(argc, argv, runTest, runBenchmark, benchmarkOutputFormat);

    mla_test_uint32_t allocationFailureSeed = 0;
    mla_test_uint32_t allocationFailureSeedCount = 0;
    mla_test_parse_allocation_failure_cmd(argc, argv, allocationFailureSeed, allocationFailureSeedCount);

    // Setup the file system
    //mla_file_system_t fs = mla_file_system_native_create_data_restricted(mla_string_const("Test"));
    //mla_file_system_initialize(mla_string_const("/"), fs);

    mla_boot_os_application();
    return run(runTest, runBenchmark, benchmarkOutputFormat, allocationFailureSeed, allocationFailureSeedCount);
}