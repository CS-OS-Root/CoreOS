//
// Created for auto-update package.
//

#ifndef MLA_UPDATE_TEST_H
#define MLA_UPDATE_TEST_H

#include "../../lib/base-lib/core/update/mla_update.h"
#include "../../lib/base-lib/test-support/mla_test_executor.h"

#if !defined mla_test_disable_network || mla_test_disable_network != 1
#include "../../lib/base-lib/core/http/mla_http_server.h"
#endif

inline void UpdateGetCurrentVersionTest() {
    mla_string_t version = mla_update_get_current_version();
    assert_true(mla_string_length(version) > 0, "Current version should not be empty");
    assert_true(mla_string_equals(version, mla_string_const(MLA_APP_VERSION)), "Current version should match MLA_APP_VERSION define");
}

inline mla_bool_t mock_get_last_version(const mla_update_provider_t& p_Provider, mla_string_t& p_OutVersion) {
    (void)p_Provider;
    p_OutVersion = mla_string_const("2.0.0");
    return true;
}

inline mla_bool_t mock_get_binary_content(const mla_update_provider_t& p_Provider, const mla_string_t& p_Version, mla_stream_input_t& p_OutStream) {
    (void)p_Provider;
    (void)p_Version;
    p_OutStream = mla_stream_input_from_string(mla_string_const("mock_binary_data"));
    return true;
}

inline void UpdateGetLastVersionMockTest() {
    mla_update_provider_t provider = { mla_user_data_empty(), mock_get_last_version, mock_get_binary_content };

    mla_string_t version = mla_string_empty();
    mla_bool_t success = mla_update_get_last_version(provider, version);
    assert_true(success, "Mock get_last_version should succeed");
    assert_true(mla_string_equals(version, mla_string_const("2.0.0")), "Should return mock version 2.0.0");

    mla_stream_input_t stream = mla_stream_noop_input();
    success = provider.get_binary_content(provider, version, stream);
    assert_true(success, "Mock get_binary_content should succeed");

    mla_string_t content = mla_string_from_stream(stream, 100);
    assert_true(mla_string_equals(content, mla_string_const("mock_binary_data")), "Binary content should match mock binary data");

    mla_update_set_provider(provider);
    version = mla_string_empty();
    success = mla_update_get_last_version(version);
    assert_true(success, "Global provider get_last_version should succeed");
    assert_true(mla_string_equals(version, mla_string_const("2.0.0")), "Global provider version should match 2.0.0");
}

inline void UpdateUpgradeInvalidStreamTest() {
    mla_stream_input_t invalid_stream = mla_stream_noop_input();
    invalid_stream.read = nullptr;

    mla_update_error_t err = mla_update_upgrade_to_version(invalid_stream);
    assert_true(err == MLA_UPDATE_ERROR_INVALID_STREAM || err == MLA_UPDATE_ERROR_NOT_SUPPORTED, "Should return invalid stream or not supported error for null stream");
}

inline void UpdateCheckAndApplyNoFlagsTest() {
    char arg0[] = "app_binary";
    char* mock_argv[] = { arg0 };
    mla_bool_t res = mla_update_check_and_apply_pending_update(1, mock_argv);
    assert_false(res, "Check and apply should return false when update flag is absent");
}

inline void UpdateGetPlatformCompilerTest() {
    mla_string_t platform = mla_update_get_current_platform();
    assert_true(mla_string_length(platform) > 0, "Current platform should not be empty");

    mla_string_t compiler = mla_update_get_current_compiler();
    assert_true(mla_string_length(compiler) > 0, "Current compiler should not be empty");

    mla_update_provider_t provider = mla_update_provider_http_create(mla_string_const("test_module"));
    assert_true(provider.get_last_version != nullptr, "Provider get_last_version function pointer should be set");
    assert_true(provider.get_binary_content != nullptr, "Provider get_binary_content function pointer should be set");
}

#if !defined mla_test_disable_network || mla_test_disable_network != 1
inline mla_bool_t update_test_http_version_handler(mla_http_server_t& http_server, const mla_user_data_t &userdata, const mla_http_request_t &request, mla_http_response_t &response) {
    (void)http_server;
    (void)userdata;
    (void)request;
    response.statusCode = 200;
    response.content = mla_stream_input_from_string(mla_string_const("3.1.4"));
    return true;
}

inline mla_bool_t update_test_http_binary_handler(mla_http_server_t& http_server, const mla_user_data_t &userdata, const mla_http_request_t &request, mla_http_response_t &response) {
    (void)http_server;
    (void)userdata;
    (void)request;
    response.statusCode = 200;
    response.content = mla_stream_input_from_string(mla_string_const("http_binary_payload"));
    return true;
}

inline void UpdateGetLastVersionHttpTest() {
    mla_network_host_t host = mla_network_host_ip4(mla_string_const("127.0.0.1"), 41259);
    mla_http_server_t server = mla_http_server(host);

    mla_user_data_t userdata1 = mla_user_data_empty();
    mla_http_server_handler_item_t versionHandler = mla_http_server_handler_starts_with(
        mla_http_method_get, userdata1, mla_string_const("/version"), update_test_http_version_handler);
    assert_true(mla_http_server_register_handler(server, versionHandler), "Should register version handler");

    mla_user_data_t userdata2 = mla_user_data_empty();
    mla_http_server_handler_item_t binaryHandler = mla_http_server_handler_starts_with(
        mla_http_method_get, userdata2, mla_string_const("/binary"), update_test_http_binary_handler);
    assert_true(mla_http_server_register_handler(server, binaryHandler), "Should register binary handler");

    mla_http_server_set_timeout(server, 2000);

    if (mla_http_server_start(server, 2)) {
        mla_update_provider_t provider = mla_update_provider_http_create(mla_string_const("test_app"), mla_string_const("http://127.0.0.1:41259"));
        mla_string_t version = mla_string_empty();
        mla_bool_t success = mla_update_get_last_version(provider, version);
        assert_true(success, "HTTP get_last_version should succeed");
        assert_true(mla_string_equals(version, mla_string_const("3.1.4")), "Version from HTTP server should match 3.1.4");

        mla_stream_input_t binary_stream = mla_stream_noop_input();
        success = provider.get_binary_content(provider, version, binary_stream);
        assert_true(success, "HTTP get_binary_content should succeed");

        mla_string_t payload = mla_string_from_stream(binary_stream, 100);
        assert_true(mla_string_equals(payload, mla_string_const("http_binary_payload")), "Payload from HTTP server should match http_binary_payload");

        mla_http_server_stop(server);
    } else {
        assert_fail("Should start HTTP update server");
    }
}
#endif

inline void UpdateSemanticVersionComparisonTest() {
    mla_string_t v_0_0_9 = mla_string_const("0.0.9");
    mla_string_t v_0_0_10 = mla_string_const("0.0.10");
    mla_string_t v_0_0_11 = mla_string_const("0.0.11");

    assert_equal(mla_private_update_compare_versions(v_0_0_11, v_0_0_9), (mla_int32_t)1, "0.0.11 should be greater than 0.0.9");
    assert_equal(mla_private_update_compare_versions(v_0_0_9, v_0_0_10), (mla_int32_t)-1, "0.0.9 should be less than 0.0.10");
    assert_equal(mla_private_update_compare_versions(v_0_0_11, v_0_0_10), (mla_int32_t)1, "0.0.11 should be greater than 0.0.10");
    assert_equal(mla_private_update_compare_versions(v_0_0_11, v_0_0_11), (mla_int32_t)0, "0.0.11 should equal 0.0.11");

    mla_string_t html_dir = mla_string_const(
        "<html><body>"
        "<a href=\"/mla-build/0.0.1/\">0.0.1/</a>"
        "<a href=\"/mla-build/0.0.9/\">0.0.9/</a>"
        "<a href=\"/mla-build/0.0.10/\">0.0.10/</a>"
        "<a href=\"/mla-build/0.0.11/\">0.0.11/</a>"
        "</body></html>"
    );

    mla_string_t extracted = mla_private_update_extract_latest_version(html_dir);
    assert_struct_equal(mla_string_t, extracted, mla_string_const("0.0.11"), "Latest extracted version from HTML should be 0.0.11");
}

#if !defined mla_test_disable_network || mla_test_disable_network != 1
inline void UpdateRealReleaseServerIntegrationTest() {
    mla_update_provider_t provider = mla_update_provider_http_create(
        mla_string_const("mla-build"),
        mla_string_const("https://releases.home.schlegel.ovh")
    );

    mla_string_t version = mla_string_empty();
    mla_bool_t success = mla_update_get_last_version(provider, version);
    if (success) {
        assert_true(mla_string_length(version) > 0, "Fetched live version should not be empty");
    }
}
#endif

inline void RegisterUpdateTests(mla_test_executor_t& p_TestExecutor) {
    mla_test_t test = mla_test("UpdateGetCurrentVersion", test_category, UpdateGetCurrentVersionTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UpdateSemanticVersionComparison", test_category, UpdateSemanticVersionComparisonTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UpdateGetPlatformCompiler", test_category, UpdateGetPlatformCompilerTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UpdateGetLastVersionMock", test_category, UpdateGetLastVersionMockTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UpdateUpgradeInvalidStream", test_category, UpdateUpgradeInvalidStreamTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UpdateCheckAndApplyNoFlags", test_category, UpdateCheckAndApplyNoFlagsTest);
    mla_test_executor_register_test(p_TestExecutor, test);

#if !defined mla_test_disable_network || mla_test_disable_network != 1
    if (mla_is_native_multi_tasking) {
        test = mla_test("UpdateGetLastVersionHttp", test_category, UpdateGetLastVersionHttpTest);
        mla_test_executor_register_test(p_TestExecutor, test);

        test = mla_test("UpdateRealReleaseServerIntegration", test_category, UpdateRealReleaseServerIntegrationTest);
        mla_test_executor_register_test(p_TestExecutor, test);
    }
#endif
}

#endif // MLA_UPDATE_TEST_H
