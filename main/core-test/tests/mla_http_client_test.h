//
// Created by chris on 10/10/2025.
//

#ifndef MLA_HTTP_CLIENT_TEST_H
#define MLA_HTTP_CLIENT_TEST_H

#include "../../lib/base-lib/core/http/mla_http_client.h"
#include "../../lib/base-lib/test-support/mla_test_executor.h"
#include "../../lib/base-lib/test-support/mla_benchmark_executor.h"

void SimpleGetRequestWithoutDeflateTest() {

    // Create a simple GET request to a test site
    mla_http_request_t request = mla_http_request(mla_string_const("http://www.testingmcafeesites.com"), mla_http_method_get);

    // Send the request
    mla_http_client_t client = mla_http_client();
    mla_http_client_set_support_deflate_compression(client, false);
    mla_http_client_set_support_gzip_compression(client, false);
    mla_http_client_response_t response = mla_http_client_send_request(client, request);

    // Verify the response status
    assert_equal(response.status, MLA_HTTP_CLIENT_RESPONSE_STATUS_OK, "HTTP request should succeed");

    // Verify we got a valid HTTP response
    assert_equal(response.response.statusCode, mla_http_status_ok,
                "Should receive a valid HTTP status code");

    // Verify content type header
    mla_string_t contentType = mla_http_headers_get_value(response.response.headers, mla_string_const("Content-Type"));
    assert_struct_equal(mla_string_t, contentType, mla_string_const("text/html"),
                "Content-Type should be text/html");

    // Verify encoding
    mla_string_t encoding = mla_http_headers_get_value(response.response.headers, mla_string_const("Content-Encoding"));
    assert_true(mla_string_is_empty(encoding),
                "Content-Encoding should be empty");

    // Check that we received some content
    mla_byte_t buffer[mla_global_config_stream_fast_read_buffer_size] = {0};

    if (response.response.content.read != nullptr) {
        mla_size_t readed = response.response.content.read(response.response.content, 0, sizeof(buffer), buffer);
        assert_true(readed > 0, "Should read some bytes from response content");

        mla_string_t content = mla_string(mla_platform_pointer_to_managed_pointer (buffer), sizeof(buffer));
        assert_true(mla_string_contains(content, mla_string_const("<html")),"Response content should contain HTML");

        mla_size_t totalRead = readed;
        // Read all content (for cleanup)
        while (readed > 0) {
            readed = response.response.content.read(response.response.content, 0, sizeof(buffer), buffer);
            totalRead += readed;
        }

        // Get total content length from headers
        mla_string_t contentLengthStr = mla_http_headers_get_value(response.response.headers, mla_string_const("Content-Length"));

        mla_size_t contentLength = 0;

        mla_parse_uint32(contentLengthStr, contentLength);
        if (mla_string_is_empty(encoding)) {
            assert_equal(totalRead, contentLength, "Total read bytes should match Content-Length header");
        } else {
            assert_true(totalRead > 0, "Should read some bytes from response content");
        }

    } else {
        assert_fail("Response content read function is null");
    }

    // Clean up
    response = mla_http_client_response_invalid();
}

void SimpleGetRequestTest() {

    // Create a simple GET request to a test site
    mla_http_request_t request = mla_http_request(mla_string_const("http://www.testingmcafeesites.com"), mla_http_method_get);

    // Send the request
    mla_http_client_t client = mla_http_client();
    mla_http_client_set_support_deflate_compression(client, true);
    mla_http_client_response_t response = mla_http_client_send_request(client, request);

    // Verify the response status
    assert_equal(response.status, MLA_HTTP_CLIENT_RESPONSE_STATUS_OK, "HTTP request should succeed");

    // Verify we got a valid HTTP response
    assert_equal(response.response.statusCode, mla_http_status_ok,
                "Should receive a valid HTTP status code");

    // Verify content type header
    mla_string_t contentType = mla_http_headers_get_value(response.response.headers, mla_string_const("Content-Type"));
    assert_struct_equal(mla_string_t, contentType, mla_string_const("text/html"),
                "Content-Type should be text/html");

    // Verify encoding
    mla_string_t encoding = mla_http_headers_get_value(response.response.headers, mla_string_const("Content-Encoding"));
    assert_true(mla_string_equals_ignore_case(encoding, mla_string_const("deflate")) || mla_string_equals_ignore_case(encoding, mla_string_const("gzip")),
                "Content-Encoding should be deflate or gzip");

    // Check that we received some content
    mla_byte_t buffer[mla_global_config_stream_fast_read_buffer_size] = {0};

    if (response.response.content.read != nullptr) {
        mla_size_t readed = response.response.content.read(response.response.content, 0, sizeof(buffer), buffer);
        assert_true(readed > 0, "Should read some bytes from response content");

        mla_string_t content = mla_string(mla_platform_pointer_to_managed_pointer (buffer), sizeof(buffer));
        assert_true(mla_string_contains(content, mla_string_const("<html")),"Response content should contain HTML");

        mla_size_t totalRead = readed;
        // Read all content (for cleanup)
        while (readed > 0) {
            readed = response.response.content.read(response.response.content, 0, sizeof(buffer), buffer);
            totalRead += readed;
        }

        // Get total content length from headers
        mla_string_t contentLengthStr = mla_http_headers_get_value(response.response.headers, mla_string_const("Content-Length"));

        mla_size_t contentLength = 0;

        mla_parse_uint32(contentLengthStr, contentLength);
        if (mla_string_is_empty(encoding)) {
            assert_equal(totalRead, contentLength, "Total read bytes should match Content-Length header");
        } else {
            assert_true(totalRead > 0, "Should read some bytes from response content");
        }

    } else {
        assert_fail("Response content read function is null");
    }

    // Clean up
    response = mla_http_client_response_invalid();
}

void GetRequestWithHeadersTest() {

    // Create a simple GET request to a test site
    mla_http_request_t request = mla_http_request(mla_string_const("http://www.testingmcafeesites.com"), mla_http_method_get);

    // Add custom headers
    mla_http_headers_add(request.headers, mla_string("User-Agent"), mla_string("MLA-HTTP-Client/1.0"));
    mla_http_headers_add(request.headers, mla_string("Accept"), mla_string("text/html"));

    // Send the request
    mla_http_client_response_t response = mla_http_client_send_request(request);

    // Verify the response status
    assert_equal(response.status, MLA_HTTP_CLIENT_RESPONSE_STATUS_OK, "HTTP request should succeed");

    // Verify we got a valid HTTP response
    assert_equal(response.response.statusCode, mla_http_status_ok,
                "Should receive a valid HTTP status code");

    // Clean up
    response = mla_http_client_response_invalid();
}

void RequestTimeoutTest() {

    // Create a simple GET request to a test site
    mla_http_request_t request = mla_http_request(mla_string_const("http://www.testingmcafeesites.com"), mla_http_method_get);

    // Create client with very short timeout
    mla_http_client_t client = mla_http_client();
    client.timeout_ms = 1; // 1ms timeout - should fail

    mla_http_client_response_t response = mla_http_client_send_request(client, request);

    // Should timeout or fail
    assert_true(response.status != MLA_HTTP_CLIENT_RESPONSE_STATUS_OK,
                "Request with 1ms timeout should fail");

    response = mla_http_client_response_invalid();
}


// Mock data structures
struct mock_connection_data_t {
    mla_bool_t should_connect_fail;
    mla_bool_t should_resolve_fail;
    mla_string_t response_data;
    mla_size_t bytes_sent;
    mla_size_t bytes_read;
    mla_bool_t simulate_partial_read;
    mla_bool_t simulate_connection_drop;
};

// Mock resolve host function
mla_bool_t mock_resolve_host_success(const mla_http_client_t &client, mla_http_client_response_t& response, const mla_url_t& url, mla_network_host_t & host) {
    (void)client;
    (void)url;
    (void)response;
    host.address.address = mla_string_const("127.0.0.1");
    host.port = 80;
    return true;
}

mla_bool_t mock_resolve_host_failure(const mla_http_client_t &client, mla_http_client_response_t& response, const mla_url_t& url, mla_network_host_t & host) {
    (void)client;
    (void)host;
    response.status = MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_UNKNOWN_HOST;
    response.errorMessage = mla_string_concat("Mock: Failed to resolve host: ", url.host);
    return false;
}

// Mock connect function
mla_bool_t mock_connect_success(const mla_http_client_t &client, mla_http_client_response_t& response, const mla_network_host_t & host, mla_network_connection_t & connection) {
    (void)client;
    (void)response;
    (void)host;

    // Create mock input/output streams
    connection.inputStream = mla_stream_noop_input();
    connection.outputStream = mla_stream_noop_output();
    return true;
}

mla_bool_t mock_connect_failure(const mla_http_client_t &client, mla_http_client_response_t& response, const mla_network_host_t & host, mla_network_connection_t & connection) {
    (void)client;
    (void)connection;
    response.status = MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_CONNECTION_FAILED;
    response.errorMessage = mla_string_concat("Mock: Failed to connect to host: ", host.address.address);
    return false;
}

// Test: Mocked successful connection
void MockSuccessfulConnectionTest() {

    mla_http_client_t client = mla_http_client();
    client.timeout_ms = 25;
    client.resolve_host = mock_resolve_host_success;
    client.connect = mock_connect_success;

    mla_http_request_t request = mla_http_get_request(mla_string_const("http://example.com"));
    mla_http_client_response_t response = mla_http_client_send_request(client, request);

    // Note: Will fail at header parsing since we're using noop streams, but connection should succeed
    assert_true(response.status != MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_CONNECTION_FAILED,
                "Connection should not fail with mock success");

    response = mla_http_client_response_invalid();
}

// Test: Mocked resolve host failure
void MockResolveHostFailureTest() {

    mla_http_client_t client = mla_http_client();
    client.timeout_ms = 25;
    client.resolve_host = mock_resolve_host_failure;
    client.connect = mock_connect_success;

    mla_http_request_t request = mla_http_get_request(mla_string_const("http://invalid.example.com"));
    mla_http_client_response_t response = mla_http_client_send_request(client, request);

    assert_equal(response.status, MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_UNKNOWN_HOST,
                "Should fail with unknown host error");
    assert_true(!mla_string_is_empty(response.errorMessage),
                "Should have error message");
    assert_true(mla_string_contains(response.errorMessage, mla_string_const("resolve")),
                "Error message should mention resolve");

    response = mla_http_client_response_invalid();
}

// Test: Mocked connection failure
void MockConnectFailureTest() {

    mla_http_client_t client = mla_http_client();
    client.timeout_ms = 25;
    client.resolve_host = mock_resolve_host_success;
    client.connect = mock_connect_failure;

    mla_http_request_t request = mla_http_get_request(mla_string_const("http://example.com"));
    mla_http_client_response_t response = mla_http_client_send_request(client, request);

    assert_equal(response.status, MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_CONNECTION_FAILED,
                "Should fail with connection failed error");
    assert_true(!mla_string_is_empty(response.errorMessage),
                "Should have error message");

    response = mla_http_client_response_invalid();
}

// Test: HTTPS uses secure transport and fails deterministically when TLS backend is unavailable
void HttpsSecureTransportUnavailableTest() {

    mla_http_client_t client = mla_http_client();
    client.timeout_ms = 25;
    client.resolve_host = mock_resolve_host_success;

    mla_http_request_t request = mla_http_get_request(mla_string_const("https://example.com"));
    mla_http_client_response_t response = mla_http_client_send_request(client, request);

    assert_equal(response.status, MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_CONNECTION_FAILED,
                "HTTPS should fail with connection failed when secure transport is unavailable");
    assert_true(!mla_string_is_empty(response.errorMessage),
                "Should have error message for HTTPS secure transport failure");

    response = mla_http_client_response_invalid();
}

// Test: Unsupported URL scheme
void UnsupportedUrlSchemeTest() {

    mla_http_client_t client = mla_http_client();

    mla_http_request_t request = mla_http_get_request(mla_string_const("ftp://example.com"));
    mla_http_client_response_t response = mla_http_client_send_request(client, request);

    assert_equal(response.status, MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_WRONG_PROTOCOL,
                "Unsupported scheme should fail with wrong protocol error");

    response = mla_http_client_response_invalid();
}

// Test: Invalid URL parsing
void InvalidUrlTest() {

    mla_http_client_t client = mla_http_client();

    // Test with malformed URL
    mla_http_request_t request = mla_http_get_request(mla_string_const("not-a-valid-url"));
    mla_http_client_response_t response = mla_http_client_send_request(client, request);

    assert_equal(response.status, MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_WRONG_PROTOCOL,
                "Should fail with wrong protocol error");

    response = mla_http_client_response_invalid();
}

// Test: Empty URL
void HttpEmptyUrlTest() {
    mla_http_client_t client = mla_http_client();

    mla_http_request_t request = mla_http_get_request(mla_string_empty());
    mla_http_client_response_t response = mla_http_client_send_request(client, request);

    assert_true(response.status != MLA_HTTP_CLIENT_RESPONSE_STATUS_OK,
                "Should fail with empty URL");

    response = mla_http_client_response_invalid();
}

// Test: Missing HTTP method
void MissingHttpMethodTest() {
    mla_http_request_t request = mla_http_request_empty();
    request.url = mla_string_const("http://example.com");
    request.method = mla_string_empty(); // Empty method

    mla_http_client_t client = mla_http_client();
    client.resolve_host = mock_resolve_host_success;
    client.connect = mock_connect_success;

    mla_http_client_response_t response = mla_http_client_send_request(client, request);

    // Should fail when trying to send request
    assert_true(response.status != MLA_HTTP_CLIENT_RESPONSE_STATUS_OK,
                "Should fail with empty method");

    response = mla_http_client_response_invalid();
}


// Test: Null client functions
void NullClientFunctionsTest() {

    mla_http_client_t client = mla_http_client();
    client.resolve_host = nullptr;
    client.connect = nullptr;

    mla_http_request_t request = mla_http_get_request(mla_string_const("http://example.com"));
    mla_http_client_response_t response = mla_http_client_send_request(client, request);

    assert_equal(response.status, MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_UNKNOWN,
                "Should fail with unknown error when functions are null");
    assert_true(!mla_string_is_empty(response.errorMessage),
                "Should have error message about missing function");

    response = mla_http_client_response_invalid();
}



// Test: Zero timeout
void ZeroTimeoutTest() {
    mla_http_client_t client = mla_http_client();
    client.timeout_ms = 0;

    mla_http_request_t request = mla_http_get_request(mla_string_const("http://example.com"));
    mla_http_client_response_t response = mla_http_client_send_request(client, request);

    // Should handle zero timeout gracefully
    assert_true(response.status != MLA_HTTP_CLIENT_RESPONSE_STATUS_OK,
                "Should fail with zero timeout");

    response = mla_http_client_response_invalid();
}


void HttpsClientConnectFlowTest() {
    mla_http_client_t client = mla_http_client();
    client.timeout_ms = 25;
    client.resolve_host = mock_resolve_host_success;

    mla_http_request_t request = mla_http_get_request(mla_string_const("https://127.0.0.1:443"));
    mla_http_client_response_t response = mla_http_client_send_request(client, request);

    assert_equal(response.status, MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_CONNECTION_FAILED,
                "HTTPS connect should attempt TLS connection and fail gracefully when host is unreachable");
    assert_true(!mla_string_is_empty(response.errorMessage),
                "HTTPS failure should populate error message");

    response = mla_http_client_response_invalid();
}

void RegisterHttpClientTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("SimpleGetRequestWithoutDeflate", test_category, SimpleGetRequestWithoutDeflateTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("SimpleGetRequest", test_category, SimpleGetRequestTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("GetRequestWithHeaders", test_category, GetRequestWithHeadersTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("RequestTimeout", test_category, RequestTimeoutTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("MockSuccessfulConnection", test_category, MockSuccessfulConnectionTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("MockResolveHostFailure", test_category, MockResolveHostFailureTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("MockConnectFailure", test_category, MockConnectFailureTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("HttpsSecureTransportUnavailable", test_category, HttpsSecureTransportUnavailableTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("HttpsClientConnectFlow", test_category, HttpsClientConnectFlowTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UnsupportedUrlScheme", test_category, UnsupportedUrlSchemeTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("InvalidUrl", test_category, InvalidUrlTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("EmptyUrl", test_category, HttpEmptyUrlTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("MissingHttpMethod", test_category, MissingHttpMethodTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("NullClientFunctions", test_category, NullClientFunctionsTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ZeroTimeout", test_category, ZeroTimeoutTest);
    mla_test_executor_register_test(p_TestExecutor, test);

}


#endif