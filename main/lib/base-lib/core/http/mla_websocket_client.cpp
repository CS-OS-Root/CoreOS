//
// Created by chris on 11/13/2025.
//

#include "mla_websocket_client.h"

#include "../log/mla_logging.h"
#include "../system/mla_string_concat.h"
#include "../system/mla_id.h"
#include "mla_http_utils.h"
#include "../hash/mla_sha1.h"
#include "../url/mla_url.h"
#include "mla_websocket_utils.h"

// Timing constants
#define mla_websocket_max_messages_per_call 10

mla_websocket_client_t mla_websocket_client_invalid() {
    return {
        mla_network_connection_disconnected(),
        mla_user_data_empty(),
         false
    };
}

mla_websocket_binary_message_t mla_websocket_binary_message_empty() {
    return {
        mla_bytes_empty()
    };
}

mla_websocket_text_message_t mla_websocket_text_message_empty() {
    return {
        mla_string_empty()
    };
}

mla_string_t mla_private_websocket_client_websocket_key() {

    // Generate random 16-byte WebSocket key
    mla_bytes_t random_bytes = mla_bytes(16);
    mla_byte_t *key_data = mla_bytes_get_data_for_writing(random_bytes);
    for (mla_size_t i = 0; i < 16; i++) {
        key_data[i] = mla_s_cast<mla_byte_t>(mla_random_uint32() & 0xFF);
    }
    return mla_bytes_to_base64(random_bytes);

}

mla_bool_t mla_websocket_client_is_connected(const mla_websocket_client_t &client) {
    return mla_network_connection_is_connected(client.connection);
}

mla_bool_t mla_websocket_client_is_deflate_compression_supported(const mla_websocket_client_t &client) {
    return client.supports_deflate_compression;
}

mla_bool_t mla_private_websocket_client_is_secure_scheme(const mla_url_t &url) {
    return mla_string_equals_const(url.scheme, "wss");
}

mla_bool_t mla_private_websocket_client_is_supported_scheme(const mla_url_t &url) {
    return mla_string_equals_const(url.scheme, "ws") || mla_private_websocket_client_is_secure_scheme(url);
}

mla_bool_t mla_websocket_client_connect(mla_websocket_client_t &client, const mla_string_t& url,
                                        mla_size_t timeout_ms, mla_bool_t supports_deflate_compression) {

    mla_url_t parsed_url = mla_url_empty();

    if (!mla_url_parse(url, parsed_url)) {
        return false;
    }

    if (!mla_private_websocket_client_is_supported_scheme(parsed_url)) {
        return false;
    }

    mla_network_host_t host = mla_network_host_invalid();

    if (!mla_network_host_resolve(host, parsed_url.host, parsed_url.port)) {
        return false;
    }

    // Create TCP connection
    mla_network_connection_t connection = mla_network_connection_disconnected();

    mla_network_security_config_t security_config = mla_network_security_config_none();

    if (mla_private_websocket_client_is_secure_scheme(parsed_url)) {
        mla_network_tls_config_t tls_config = mla_network_tls_config_default();
        mla_network_tls_config_set_server_name(tls_config, parsed_url.host);
        security_config = mla_network_security_config_tls(tls_config);
    }

    if (!mla_network_connection_connect_secure(connection, host, mla_connection_type_tcp, timeout_ms,
                                               security_config)) {
        return false;
    }

    mla_stream_output_t &output = connection.outputStream;
    mla_stream_input_t &input = connection.inputStream;

    // Generate random WebSocket key (16 bytes, base64 encoded)
    mla_string_t ws_key = mla_private_websocket_client_websocket_key();

    // Build HTTP upgrade request
    if (!mla_stream_output_write_string(output, mla_string_const("GET "))) {
        return false;
    }

    mla_string_t pathAndQuery = mla_url_to_string_pathAndQuery(parsed_url);

    if (!mla_stream_output_write_string(output, pathAndQuery)) {
        return false;
    }

    if (!mla_stream_output_write_string(output, mla_string_const(" HTTP/1.1\r\n"))) {
        return false;
    }
    if (!mla_stream_output_write_string(output, mla_string_const("Host: "))) {
        return false;
    }
    if (!mla_stream_output_write_string(output, parsed_url.host)) {
        return false;
    }
    if (!mla_stream_output_write_string(output, mla_string_const("\r\n"))) {
        return false;
    }
    if (!mla_stream_output_write_string(output, mla_string_const("Upgrade: websocket\r\n"))) {
        return false;
    }
    if (!mla_stream_output_write_string(output, mla_string_const("Connection: Upgrade\r\n"))) {
        return false;
    }
    if (!mla_stream_output_write_string(output, mla_string_const("Sec-WebSocket-Key: "))) {
        return false;
    }
    if (!mla_stream_output_write_string(output, ws_key)) {
        return false;
    }
    if (!mla_stream_output_write_string(output, mla_string_const("\r\n"))) {
        return false;
    }
    if (!mla_stream_output_write_string(output, mla_string_const("Sec-WebSocket-Version: 13\r\n"))) {
        return false;
    }
    if (supports_deflate_compression) {
        // Send Sec-WebSocket-Extensions header to indicate support for permessage-deflate compression
        if (!mla_stream_output_write_string(output, mla_string_const("Sec-WebSocket-Extensions: permessage-deflate\r\n"))) {
            return false;
        }
    }
    if (!mla_stream_output_write_string(output, mla_string_const("\r\n"))) {
        return false;
    }

    // Read status line
    mla_string_t status_line = mla_string_empty();
    if (!mla_http_utils_read_line(input, status_line, timeout_ms)) {
        return false;
    }

    // Verify "HTTP/1.1 101"
    if (!mla_string_starts_with(status_line, mla_string_const("HTTP/1.1 101"))) {
        return false;
    }

    // Read and verify headers
    mla_array_list_t<mla_init_struct(mla_http_header_t)> headers = mla_array_list<mla_init_struct(mla_http_header_t)>();
    if (!mla_http_utils_read_headers(headers, input, timeout_ms)) {
        return false;
    }

    // Verify Upgrade: websocket header
    mla_string_t upgrade = mla_http_headers_get_value(headers, mla_string_const("Upgrade"));
    if (!mla_string_equals_const(upgrade, "websocket")) {
        return false;
    }

    client.supports_deflate_compression = mla_http_headers_has_header_value(headers, mla_string_const("Sec-WebSocket-Extensions"), mla_string_const("permessage-deflate"), mla_string_const(","));


    // Verify Sec-WebSocket-Accept header
    mla_string_t accept_header = mla_http_headers_get_value(headers, mla_string_const("Sec-WebSocket-Accept"));
    if (mla_string_is_empty(accept_header)) {
        return false;
    }

    // Compute expected accept value: base64(sha1(key + GUID))
    mla_string_t key_with_guid = mla_string_concat(ws_key, mla_string_const("258EAFA5-E914-47DA-95CA-C5AB0DC85B11"));
    mla_bytes_t hash = mla_sha1(mla_bytes_from_string(key_with_guid));
    mla_string_t expected_accept = mla_bytes_to_base64(hash);

    // Verify accept header matches
    if (!mla_string_equals(accept_header, expected_accept)) {
        return false;
    }

    // Connection successful, store it
    client.connection = connection;
    return true;
}


mla_bool_t mla_websocket_client_disconnect(mla_websocket_client_t &client) {
    return mla_websocket_client_disconnect(client, mla_websocket_close_normal, mla_string_empty());
}

mla_bool_t mla_websocket_client_disconnect(mla_websocket_client_t &client, mla_uint16_t status_code,
                                           const mla_string_t &reason) {
    if (!mla_network_connection_is_connected(client.connection)) {
        return true;
    }

    mla_stream_output_t &output = client.connection.outputStream;
    // We dont cate about the result of sending the close frame, we are disconnecting anyway
    mla_websocket_transport_send_close_frame(output, status_code, reason, true);
    client = mla_websocket_client_invalid();
    return true;
}



mla_bool_t mla_websocket_client_send_text_message(mla_websocket_client_t &client, const mla_string_t &message) {
    if (!mla_network_connection_is_connected(client.connection)) {
        return false;
    }

    mla_stream_output_t &output = client.connection.outputStream;
    if (!mla_websocket_transport_send_text_frame(output, message, true, client.supports_deflate_compression)) {
        return false;
    }
    return mla_stream_output_flush_buffered_wrapper(output);
}

mla_bool_t mla_websocket_client_send_binary_message(mla_websocket_client_t &client, const mla_bytes_t &message) {
    if (!mla_network_connection_is_connected(client.connection)) {
        return false;
    }

    mla_stream_output_t &output = client.connection.outputStream;

    if (!mla_websocket_transport_send_binary_frame(output, message, true, client.supports_deflate_compression)) {
        return false;
    }
    return mla_stream_output_flush_buffered_wrapper(output);
}

mla_websocket_client_message_receive_type_t mla_websocket_client_receive_message(mla_websocket_client_t &client, mla_size_t timeout_ms, mla_websocket_text_message_t &textMessage, mla_websocket_binary_message_t &binaryMessage) {

    // Max process 10 messages per call to avoid blocking too long
    for (mla_size_t repeatCount = 0; repeatCount < mla_websocket_max_messages_per_call; repeatCount++) {

        if (!mla_network_connection_is_connected(client.connection)) {
            return MLA_WEBSOCKET_CLIENT_MESSAGE_RECEIVE_TYPE_CLOSED;
        }

        mla_websocket_transport_message_receive_type_t result = mla_websocket_transport_receive_message(client.connection, timeout_ms, textMessage.message, binaryMessage.message, true);

        if (result == MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TEXT) {
            return MLA_WEBSOCKET_CLIENT_MESSAGE_RECEIVE_TYPE_TEXT;

        } else if (result == MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_BINARY) {
            return MLA_WEBSOCKET_CLIENT_MESSAGE_RECEIVE_TYPE_BINARY;

        } else if (result == MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_CLOSED) {
            client = mla_websocket_client_invalid();
            return MLA_WEBSOCKET_CLIENT_MESSAGE_RECEIVE_TYPE_CLOSED;

        } else if (result == MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TIMEOUT) {
            // Timeout occurred - no message received within window, return TIMEOUT without disconnecting
            return MLA_WEBSOCKET_CLIENT_MESSAGE_RECEIVE_TYPE_TIMEOUT;
        } else {
            // continue processing
            continue;
        }
    }

    return MLA_WEBSOCKET_CLIENT_MESSAGE_RECEIVE_TYPE_NO_MESSAGE;
}
