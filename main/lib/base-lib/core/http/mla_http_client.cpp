//
// Created by christian on 10/6/25.
//

#include "mla_http_client.h"
#include "../log/mla_logging.h"
#include "../system/mla_string_concat.h"
#include "../system/mla_number.h"
#include "mla_http_utils.h"
#include "../http/mla_http_chunked_stream.h"

mla_bool_t mla_private_http_client_default_resolve_host(const mla_http_client_t &client, mla_http_client_response_t& response, const mla_url_t& url, mla_network_host_t & host) {

    (void)client;

    if (!mla_network_host_resolve(host, url.host, url.port)) {
        response.status = MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_UNKNOWN_HOST;
        response.errorMessage = mla_string_concat("Failed to resolve host: ", url.host);
        return false;
    }

    return true;
}

mla_bool_t mla_private_http_client_default_connect(const mla_http_client_t &client, mla_http_client_response_t& response, const mla_network_host_t & host, mla_network_connection_t & connection) {

    if (!mla_network_connection_connect(connection, host, mla_connection_type_tcp, client.timeout_ms)) {
        response.status = MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_CONNECTION_FAILED;
        response.errorMessage = mla_string_concat("Failed to connect to host: ", host.address.address);
        return false;
    }

    return true;
}

mla_http_client_t mla_http_client() {
    return {
#if mla_global_feature_flag_http_client_use_deflate_compression == 1
        true,
#else
        false,
#endif

#if mla_global_feature_flag_http_client_use_gzip_compression == 1
        true,
#else
        false,
#endif

        mla_global_config_default_http_timeout_ms, // Default timeout 30 seconds
        mla_private_http_client_default_resolve_host,
        mla_private_http_client_default_connect
    };
}

mla_bool_t mla_http_client_support_deflate_compression(const mla_http_client_t &client) {
    return client.support_deflate_compression;
}

void mla_http_client_set_support_deflate_compression(mla_http_client_t &client, mla_bool_t support_deflate_compression) {
    client.support_deflate_compression = support_deflate_compression;
}

mla_bool_t mla_http_client_support_gzip_compression(const mla_http_client_t &client) {
    return client.support_gzip_compression;
}

void mla_http_client_set_support_gzip_compression(mla_http_client_t &client, mla_bool_t support_gzip_compression) {
    client.support_gzip_compression = support_gzip_compression;
}

mla_int32_t mla_http_client_get_timeout(const mla_http_client_t &client) {
    return client.timeout_ms;
}

void mla_http_client_set_timeout(mla_http_client_t &client, mla_int32_t timeout_ms) {
    client.timeout_ms = timeout_ms;
}

mla_bool_t mla_private_http_client_is_https_url(const mla_url_t &url) {
    return mla_string_equals_const(url.scheme, "https");
}

mla_bool_t mla_private_http_client_is_supported_url_scheme(const mla_url_t &url) {
    return mla_string_equals_const(url.scheme, "http") || mla_private_http_client_is_https_url(url);
}


mla_bool_t mla_private_http_client_resolve_host(const mla_http_client_t &client, mla_http_client_response_t& response, const mla_url_t& url, mla_network_host_t & host) {

    if (client.resolve_host == nullptr) {
        response.status = MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_UNKNOWN;
        response.errorMessage = mla_string_const("HTTP client has no resolve_host function");
        return false;
    }

    return client.resolve_host(client, response, url, host);
}

mla_bool_t mla_private_http_client_connect(const mla_http_client_t &client, mla_http_client_response_t& response,
                                           const mla_url_t &url, const mla_network_host_t & host,
                                           mla_network_connection_t & connection) {

    if (client.connect == nullptr) {
        response.status = MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_UNKNOWN;
        response.errorMessage = mla_string_const("HTTP client has no connect function");
        return false;
    }

    if (client.connect != mla_private_http_client_default_connect) {
        return client.connect(client, response, host, connection);
    }

    mla_network_security_config_t security_config = mla_network_security_config_none();

    if (mla_private_http_client_is_https_url(url)) {
        mla_network_tls_config_t tls_config = mla_network_tls_config_default();
        mla_network_tls_config_set_server_name(tls_config, url.host);
        security_config = mla_network_security_config_tls(tls_config);
    }

    if (!mla_network_connection_connect_secure(connection, host, mla_connection_type_tcp, client.timeout_ms,
                                               security_config)) {
        response.status = MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_CONNECTION_FAILED;
        response.errorMessage = mla_string_concat("Failed to connect to host: ", host.address.address);
        return false;
    }

    return true;
}

mla_bool_t mla_private_http_client_send_header(mla_http_client_response_t& response, const mla_url_t& url, mla_http_request_t & request, mla_stream_output_t & connection) {

    if (mla_string_is_empty(request.method)) {
        response.status = MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_UNKNOWN;
        response.errorMessage = mla_string_const("HTTP request has no method");
        return false;
    }

    if (!mla_stream_output_write_string(connection, request.method)) {
        return false;
    }

    if (!mla_stream_output_write_string(connection, mla_string_const(" "))) {
        return false;
    }

    mla_string_t pathAndQuery = mla_url_to_string_pathAndQuery(url);

    if (!mla_stream_output_write_string(connection, pathAndQuery)) {
        return false;
    }

    if (!mla_stream_output_write_string(connection, mla_string_const(" "))) {
        return false;
    }

    switch (request.version) {
        case MLA_HTTP_VERSION_1_0:
            if (!mla_stream_output_write_string(connection, mla_string_const("HTTP/1.0\r\n"))) {
                return false;
            }
            break;
        case MLA_HTTP_VERSION_1_1:
            if (!mla_stream_output_write_string(connection, mla_string_const("HTTP/1.1\r\n"))) {
                return false;
            }
            break;
        default:
            response.status = MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_WRONG_PROTOCOL;
            response.errorMessage = mla_string_concat("Unsupported HTTP version: ", mla_string_from_int32(request.version));
            return false;
    }

    if (!mla_stream_output_write_string(connection, mla_string_const("Host: "))) {
        return false;
    }

    if (!mla_stream_output_write_string(connection, url.host)) {
        return false;
    }

    if (!mla_stream_output_write_string(connection, mla_string_const("\r\n"))) {
        return false;
    }

    // Write Headers
    if (!mla_http_utils_write_headers(request.headers, connection)) {
        return false;
    }

    // Write Content Length if body is present
    if (!mla_http_utils_write_content_headers(request.headers, request.content, connection)) {
        return false;
    }

    if (!mla_stream_output_write_string(connection, mla_string_const("\r\n"))) {
        return false;
    }

    return true;

}

mla_bool_t mla_private_http_client_send_body(mla_http_client_response_t& response, mla_http_request_t & request, mla_stream_output_t & connection) {

    if (mla_http_request_content_writer_is_valid(request.contentWriter)) {

        if (!request.contentWriter.writeTo(request.contentWriter, connection)) {
            response.status = MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_UNKNOWN;
            response.errorMessage = mla_string_const("Failed to write request body using content writer");
            return false;
        }

    } else if (!mla_stream_copy(request.content, connection)) {
        response.status = MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_UNKNOWN;
        response.errorMessage = mla_string_const("Failed to send request body");
        return false;
    }

    mla_stream_output_flush_buffered_wrapper(connection);
    return true;
}

mla_bool_t mla_private_http_client_parse_response_header(mla_stream_input_t & inputStream, mla_http_response_t & response, mla_int32_t timeout_ms) {

    mla_string_t statusLine = mla_string_empty();

    if (!mla_http_utils_read_line(inputStream, statusLine, timeout_ms)) {
        response.statusCode = 400;
        response.statusMessage = mla_string_const("Failed to read status line");
        return false;
    }

    // Parse status line
    mla_array_list_t<mla_init_struct(mla_string_t)> parts = mla_string_split(statusLine, mla_string_const(" "));

    if (mla_array_list_size(parts) < 2) {
        response.statusCode = 400;
        response.statusMessage = mla_string_const("Invalid status line");
        return false;
    }
    // HTTP version is parts[0]
    mla_string_t versionStr = *mla_array_list_get_ref(parts, 0);

    if (!mla_http_utils_parse_http_version(versionStr, response.version)) {
        response.statusCode = 400;
        response.statusMessage = mla_string_concat("Unsupported HTTP version: ", versionStr);
        return false;
    }

    // HTTP status code is parts[1]
    mla_string_t statusCodeStr = *mla_array_list_get_ref(parts, 1);

    if (!mla_parse_uint16(statusCodeStr, response.statusCode)) {
        response.statusCode = 400;
        response.statusMessage = mla_string_const("Invalid status code");
        return false;
    }

    if (!mla_http_utils_read_headers(response.headers, inputStream, timeout_ms)) {
        response.statusCode = 400;
        response.statusMessage = mla_string_const("Failed to read header line");
        return false;
    }

    return true;

}

mla_bool_t mla_private_http_client_handle_response_body(mla_http_response_t& response, mla_network_connection_t & connection, mla_int32_t timeout_ms) {

    // Check for Content-Length header
    mla_string_t contentLengthStr = mla_http_headers_get_value(response.headers, mla_string_const("Content-Length"));

    if (mla_string_equals_const(contentLengthStr, "0")) {
        response.content = mla_stream_noop_input();
        return false;
    }

    mla_bool_t use_deflate = mla_http_headers_has_header_value(response.headers, mla_string_const("Content-Encoding"), mla_string_const("deflate"), mla_string_const(","));
    mla_bool_t use_gzip = mla_http_headers_has_header_value(response.headers, mla_string_const("Content-Encoding"), mla_string_const("gzip"), mla_string_const(","));

    mla_size_t content_size = 0;

    if (mla_parse_uint32(contentLengthStr, content_size)) {

        mla_stream_input_t response_stream = mla_stream_input_buffered_wrapper(connection.inputStream, mla_global_config_stream_fast_read_buffer_size);
        response_stream = mla_http_content_fixed_size_input_stream(response_stream, timeout_ms, content_size);

        if (use_deflate || use_gzip) {
            response_stream = mla_stream_input_deflate_decompress_wrapper(response_stream);
        }


        response.content = response_stream;
        return true;
    }

    // No Content-Length, use chunked or until close
    mla_string_t transferEncoding = mla_http_headers_get_value(response.headers, mla_string_const("Transfer-Encoding"));

    if (mla_string_equals_const(transferEncoding, "chunked")) {
        mla_stream_input_t buffered_stream = mla_stream_input_buffered_wrapper(connection.inputStream, mla_global_config_stream_fast_read_buffer_size);

        if (use_deflate || use_gzip) {
            response.content = mla_http_chunked_stream_input_deflate(buffered_stream, timeout_ms);
        } else {
            response.content = mla_http_chunked_stream_input(buffered_stream, timeout_ms);
        }

        return true;
    }

    response.content = mla_stream_noop_input();
    return true;
}

mla_bool_t mla_private_http_client_parse_url(mla_http_client_response_t& response, const mla_string_t & urlString, mla_url_t & url) {

    if (!mla_url_parse(urlString, url)) {
        response.status = MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_WRONG_PROTOCOL;
        response.errorMessage = mla_string_concat("Failed to parse URL: ", urlString);
        return false;
    }

    if (!mla_private_http_client_is_supported_url_scheme(url)) {
        response.status = MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_WRONG_PROTOCOL;
        response.errorMessage = mla_string_concat("Unsupported URL scheme: ", url.scheme);
        return false;
    }

    return true;
}

void mla_private_http_client_close_connection(mla_network_connection_t & connection) {
    mla_network_connection_disconnect(connection);
}

mla_http_client_response_t mla_http_client_send_request(const mla_http_client_t &client, mla_http_request_t &p_Request) {

    mla_http_client_response_t response = { MLA_HTTP_CLIENT_RESPONSE_STATUS_OK, mla_string_empty(), mla_http_response_empty() };

    /////////////
    /// CONNECT
    /////////////

    // Resolve host
    mla_network_host_t host = mla_network_host_invalid();

    mla_url_t url = mla_url_empty();

    if (!mla_private_http_client_parse_url(response, p_Request.url, url)) {
        return response;
    }

    if (!mla_private_http_client_resolve_host(client, response, url, host)) {
        return response;
    }

    // Open Connection
    mla_network_connection_t connection = mla_network_connection_disconnected();

    if (!mla_private_http_client_connect(client, response, url, host, connection)) {
        return response;
    }

    ////////////
    /// REQUEST
    ////////////

    mla_stream_output_t bufferedOutputStream = mla_stream_output_buffered_wrapper(connection.outputStream, mla_global_config_stream_fast_read_buffer_size);

    if (client.support_deflate_compression) {

        mla_string_t accepted_encoding = mla_http_headers_get_value(p_Request.headers, mla_string_const("Accept-Encoding"));

        if (mla_string_is_empty(accepted_encoding)) {
            mla_http_headers_add(p_Request.headers, mla_string_const("Accept-Encoding"), mla_string_const("deflate"));
        } else if (!mla_string_contains(accepted_encoding, mla_string_const("deflate"))) {
            mla_http_headers_add(p_Request.headers, mla_string_const("Accept-Encoding"), mla_string_const("deflate"), mla_string_const(","));
        }
    }

    if (client.support_gzip_compression) {

        mla_string_t accepted_encoding = mla_http_headers_get_value(p_Request.headers, mla_string_const("Accept-Encoding"));

        if (mla_string_is_empty(accepted_encoding)) {
            mla_http_headers_add(p_Request.headers, mla_string_const("Accept-Encoding"), mla_string_const("gzip"));
        } else if (!mla_string_contains(accepted_encoding, mla_string_const("gzip"))) {
            mla_http_headers_add(p_Request.headers, mla_string_const("Accept-Encoding"), mla_string_const("gzip"), mla_string_const(","));
        }
    }


    // Send Header
    if (!mla_private_http_client_send_header(response, url, p_Request, bufferedOutputStream)) {
        mla_stream_output_flush_buffered_wrapper(bufferedOutputStream);
        mla_private_http_client_close_connection(connection);
        return response;
    }

    // Send Body
    if (!mla_private_http_client_send_body(response, p_Request, bufferedOutputStream)) {
        mla_stream_output_flush_buffered_wrapper(bufferedOutputStream);
        mla_private_http_client_close_connection(connection);
        return response;
    }

    // Close and Flush
    if (!mla_stream_output_flush_buffered_wrapper(bufferedOutputStream)) {
        mla_private_http_client_close_connection(connection);
        return response;
    }
    bufferedOutputStream = mla_stream_noop_output();

    ///////////
    /// RESPONSE
    ////////////

    // Parse Response Header
    if (!mla_private_http_client_parse_response_header(connection.inputStream, response.response, client.timeout_ms)) {
        mla_private_http_client_close_connection(connection);
        return response;
    }

    // Handle Response Body
    if (!mla_private_http_client_handle_response_body(response.response, connection, client.timeout_ms)) {
        mla_private_http_client_close_connection(connection);
        return response;
    }

    return response;
}
