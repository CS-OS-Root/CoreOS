//
// Created by chris on 9/11/2025.
//

#include "../lib/base-lib/platform/windows/mla_global_network_windows.h"
#include "../lib/base-lib/platform/windows/mla_global_platform_windows.h"
#include "../lib/base-lib/platform/windows/mla_global_file_system_windows.h"
#include "../lib/base-lib/platform/windows/mla_global_config_windows.h"
#include "../lib/base-lib/platform/windows/mla_global_platform_task_manager_windows.h"
#include "../lib/base-lib/platform/windows/mla_global_external_task_windows.h"
#include "../lib/base-lib/platform/windows/mla_global_service_windows.h"


#include "../lib/base-lib/test-support/mla_test_data_types.h"
#include "../lib/base-lib/test-support/platform/mla_test_memory_malloc.h"
#include "../lib/base-lib/test-support/platform/mla_benchmark_timer_std.h"
#include "../lib/base-lib/test-support/platform/mla_test_print_std.h"
#include "../lib/base-lib/test-support/platform/mla_test_mutex_std.h"
#include "../lib/base-lib/test-support/mla_test_cli_utils.h"

#include "main_test.h"

int main(int argc, char** argv) {

    mla_test_bool_t runTest;
    mla_test_bool_t runBenchmark;
    mla_test_output_format_t benchmarkOutputFormat;

    mla_test_parse_cmd(argc, argv, runTest, runBenchmark, benchmarkOutputFormat);

    mla_test_uint32_t allocationFailureSeed = 0;
    mla_test_uint32_t allocationFailureSeedCount = 0;
    mla_test_parse_allocation_failure_cmd(argc, argv, allocationFailureSeed, allocationFailureSeedCount);

    // Setup the file system
    mla_file_system_t fs = mla_file_system_native_create_data_restricted(mla_string_const("Test"));
    mla_file_system_initialize(mla_string_const("/"), fs);

    mla_boot_os_application();
    return run(runTest, runBenchmark, benchmarkOutputFormat, allocationFailureSeed, allocationFailureSeedCount);
}