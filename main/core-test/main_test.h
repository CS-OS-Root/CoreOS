//
// Created by christian on 9/10/25.
//

#ifndef MAIN_TEST_H
#define MAIN_TEST_H


// Include Global Storage
#include "../lib/base-lib/test-support/mla_test_runner.h"

#include "tests/mla_data_types_test.h"
#include "tests/mla_pointer_test.h"
#include "tests/mla_string_test.h"
#include "tests/mla_string_builder_test.h"
#include "tests/mla_number_test.h"
#include "tests/mla_memory_hook_test.h"
#include "tests/mla_array_list_test.h"
#include "tests/mla_link_list_test.h"
#include "tests/mla_hash_map_test.h"
#include "tests/mla_logger_console_test.h"
#include "tests/mla_task_manager_test.h"
#include "tests/mla_rw_lock_test.h"
#include "tests/mla_mutex_test.h"
#include "tests/mla_cli_parser_test.h"
#include "tests/mla_cli_app_test.h"
#include "tests/mla_task_cli_module_test.h"
#include "tests/mla_task_local_test.h"
#include "tests/mla_serializer_test.h"
#include "tests/mla_config_test.h"
#include "tests/mla_id_test.h"
#include "tests/mla_bytes_test.h"
#include "tests/mla_atomic_test.h"
#include "tests/mla_sha1_test.h"
#include "tests/mla_hash_test.h"
#include "tests/mla_rpc_test.h"
#include "tests/mla_stream_test.h"
#include "tests/mla_stream_deflate_test.h"
#include "tests/mla_logger_rpc_test.h"
#include "tests/mla_reflection_test.h"
#include "tests/mla_reflection_rpc_test.h"
#include "tests/mla_memory_test.h"
#include "tests/mla_math_utils_test.h"
#include "tests/mla_user_data_test.h"
#include "tests/mla_area_memory_test.h"

#if !defined mla_test_disable_update || mla_test_disable_update != 1
#include "tests/mla_update_test.h"
#include "tests/mla_update_cli_module_test.h"
#endif

#if !defined mla_test_disable_external_task || mla_test_disable_external_task != 1
#include "tests/mla_external_task_test.h"
#endif

#if !defined mla_test_disable_network || mla_test_disable_network != 1
// Network
#include "tests/mla_http_chunked_stream_test.h"
#include "tests/mla_http_header_test.h"
#include "tests/mla_url_test.h"
#include "tests/mla_http_client_test.h"
#include "tests/mla_http_server_test.h"
#include "tests/mla_websocket_client_test.h"
#include "tests/mla_http_rpc_test.h"
#include "tests/mla_network_test.h"
#include "tests/mla_ui_html_template_compile_test.h"

#endif

#if !defined mla_test_disable_file_system|| mla_test_disable_file_system != 1
// File System
#include "tests/mla_file_system_test.h"

#endif

#include "tests/native_string_test.h"
#include "tests/native_list_test.h"

int run(mla_test_bool_t runTest, mla_test_bool_t runBenchmark, mla_test_output_format_t benchmarkOutputFormat,
        mla_test_uint32_t p_AllocationFailureSeed = 0, mla_test_uint32_t p_AllocationFailureSeedCount = 0) {

    mla_test_executor_t l_TestExecutor = mla_test_executor();
    RegisterDataTypesTests(l_TestExecutor);
    RegisterPointerTests(l_TestExecutor);
    RegisterAreaMemoryTests(l_TestExecutor);
    RegisterStringTests(l_TestExecutor);
    RegisterStringBuilderTests(l_TestExecutor);
    RegisterNumberTests(l_TestExecutor);
    RegisterMemoryHookTests(l_TestExecutor);
    RegisterArrayListTests(l_TestExecutor);
    RegisterLinkListTests(l_TestExecutor);
    RegisterHashMapTests(l_TestExecutor);
    RegisterLoggerConsoleTests(l_TestExecutor);
    RegisterTaskManagerTests(l_TestExecutor);
    RegisterReadWriteLockTests(l_TestExecutor);
    RegisterMutexTests(l_TestExecutor);
    RegisterCliParserTests(l_TestExecutor);
    RegisterCliAppTests(l_TestExecutor);
    RegisterTaskCliModuleTests(l_TestExecutor);
    RegisterTaskLocalTests(l_TestExecutor);
    RegisterSerializerTests(l_TestExecutor);
    RegisterConfigTests(l_TestExecutor);
    RegisterIdTests(l_TestExecutor);
    RegisterBytesTests(l_TestExecutor);
    RegisterAtomicTests(l_TestExecutor);
    RegisterSha1Tests(l_TestExecutor);
    RegisterHashTests(l_TestExecutor);
    RegisterRpcTests(l_TestExecutor);
    RegisterStreamTests(l_TestExecutor);
    RegisterStreamDeflateTests(l_TestExecutor);
    RegisterLoggerRpcTests(l_TestExecutor);
    RegisterReflectionTests(l_TestExecutor);
    RegisterReflectionRpcTests(l_TestExecutor);
    RegisterMathUtilsTests(l_TestExecutor);
    RegisterUserDataTests(l_TestExecutor);

#if !defined mla_test_disable_update || mla_test_disable_update != 1
    RegisterUpdateTests(l_TestExecutor);
    RegisterUpdateCliModuleTests(l_TestExecutor);
#endif

#if !defined mla_test_disable_external_task || mla_test_disable_external_task != 1
    RegisterExternalTaskTests(l_TestExecutor);
#endif

#if !defined mla_test_disable_network || mla_test_disable_network != 1
    // Network Tests
    RegisterHttpChunkedStreamTests(l_TestExecutor);
    RegisterHttpHeaderTests(l_TestExecutor);
    RegisterUiHtmlTemplateCompileTests(l_TestExecutor);
    RegisterUrlTests(l_TestExecutor);
    RegisterHttpClientTests(l_TestExecutor);
    RegisterHttpServerTests(l_TestExecutor);
    RegisterWebsocketClientTests(l_TestExecutor);
    RegisterHttpRpcTests(l_TestExecutor);
    RegisterNetworkTests(l_TestExecutor);

#endif

#if !defined mla_test_disable_file_system|| mla_test_disable_file_system != 1
    // File System Tests
    RegisterFileSystemPathTests(l_TestExecutor);
#endif

    mla_benchmark_executor_t l_BenchmarkExecutor = mla_benchmark_executor();
    RegisterPointerBenchmarks(l_BenchmarkExecutor);
    RegisterAreaMemoryBenchmarks(l_BenchmarkExecutor);
    RegisterStringBenchmarks(l_BenchmarkExecutor);
    RegisterNumberBenchmarks(l_BenchmarkExecutor);
    RegisterMemoryHookBenchmarks(l_BenchmarkExecutor);
    RegisterArrayListBenchmarks(l_BenchmarkExecutor);
    RegisterLinkListBenchmarks(l_BenchmarkExecutor);
    RegisterHashMapBenchmarks(l_BenchmarkExecutor);
    RegisterMutexBenchmarks(l_BenchmarkExecutor);
    RegisterReadWriteLockBenchmarks(l_BenchmarkExecutor);
    RegisterSerializerBenchmarks(l_BenchmarkExecutor);
    RegisterAtomicBenchmarks(l_BenchmarkExecutor);
    RegisterSha1Benchmarks(l_BenchmarkExecutor);
    RegisterHashBenchmarks(l_BenchmarkExecutor);
    RegisterTaskLocalBenchmarks(l_BenchmarkExecutor);
    RegisterUserDataBenchmarks(l_BenchmarkExecutor);
    RegisterStreamDeflateBenchmarks(l_BenchmarkExecutor);

#if !defined mla_test_disable_network || mla_test_disable_network != 1
    // Network Benchmarks

    RegisterHttpServerBenchmarks(l_BenchmarkExecutor);

#endif

    // Native Test for comparison with mla_memory functions
    RegisterMemoryBenchmarks(l_BenchmarkExecutor);
    // Native Test for comparison with C++ std::string
    RegisterNativeStringBenchmarks(l_BenchmarkExecutor);
    RegisterNativeListBenchmark(l_BenchmarkExecutor);

    return mla_test_runner_run(l_TestExecutor, l_BenchmarkExecutor, runTest, runBenchmark, benchmarkOutputFormat, p_AllocationFailureSeed, p_AllocationFailureSeedCount);
}

#endif //MAIN_TEST_H
