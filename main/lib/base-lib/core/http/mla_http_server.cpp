//
// Created by chris on 10/23/2025.
//

#include "mla_http_server.h"

#include "../log/mla_logging.h"
#include "../task/mla_task_manager.h"
#include "mla_http_utils.h"
#include "../system/mla_id.h"
#include "../system/mla_number.h"
#include "mla_websocket_utils.h"
#include "../hash/mla_sha1.h"
#include "../http/mla_http_chunked_stream.h"
#include "mla_http_client.h"
#include "mla_websocket_client.h"
#include "../url/mla_url.h"

#define mla_handler_item_array_param mla_init_struct(mla_http_server_handler_item_t)
#define mla_websocket_handler_item_array_param mla_init_struct(mla_http_server_websocket_handler_item_t)
#define mla_websocket_connection_array_param mla_init_struct(mla_http_server_websocket_connection_t)

mla_user_data_id_init(mla_http_task_user_data_server_name)

#define mla_http_server_connection_input_buffer_size mla_global_config_stream_fast_read_buffer_size
#define mla_http_server_connection_output_buffer_size mla_global_config_stream_fast_read_buffer_size

mla_http_server_handler_item_t mla_http_server_handler(const mla_string_t &http_method,
                                                       mla_user_data_t& userdata,
                                                       const mla_http_request_handler_checker_t &checker,
                                                       const mla_http_request_handler_t &executor) {
    return {
        userdata,
        mla_string_to_upper(http_method),
        checker,
        executor
    };
}

struct mla_http_server_path_checker_userdata_t {
    mla_string_t path_data;

    static mla_http_server_path_checker_userdata_t init() {
        return {
            mla_string_empty(),
        };
    }
};

mla_user_data_id_init(mla_http_server_handler_path_data_user_data_name)

mla_bool_t mla_private_http_server_handler_starts_with_checker(const mla_user_data_t& userdata,
                                                         const mla_http_request_t &request, mla_http_request_handler_checker_compare_mode_t compare_mode) {

    mla_pointer_t checker_userdata = mla_user_data_get_pointer(userdata, mla_http_server_handler_path_data_user_data_name);

    mla_http_server_path_checker_userdata_t *checker_userdata_ptr = mla_pointer_get_data<mla_http_server_path_checker_userdata_t>(checker_userdata);

    if (checker_userdata_ptr == nullptr) {
        return false;
    }

    if (compare_mode == MLA_HTTP_REQUEST_HANDLER_CHECKER_COMPARE_MODE_PERFECT_MATCH) {
        return mla_string_starts_with(request.url, checker_userdata_ptr->path_data);
    }

    if (compare_mode == MLA_HTTP_REQUEST_HANDLER_CHECKER_COMPARE_MODE_PREFIX) {
        return mla_string_starts_with(checker_userdata_ptr->path_data, request.url);
    }

    return false;

}

mla_bool_t mla_private_http_server_handler_equals_checker(const mla_user_data_t &userdata,
                                                    const mla_http_request_t &request, mla_http_request_handler_checker_compare_mode_t compare_mode) {

    mla_pointer_t checker_userdata = mla_user_data_get_pointer(userdata, mla_http_server_handler_path_data_user_data_name);

    mla_http_server_path_checker_userdata_t *checker_userdata_ptr = mla_pointer_get_data<mla_http_server_path_checker_userdata_t>(checker_userdata);

    if (checker_userdata_ptr == nullptr) {
        return false;
    }

    if (compare_mode == MLA_HTTP_REQUEST_HANDLER_CHECKER_COMPARE_MODE_PERFECT_MATCH) {
        return mla_string_equals(request.url, checker_userdata_ptr->path_data);
    }

    if (compare_mode == MLA_HTTP_REQUEST_HANDLER_CHECKER_COMPARE_MODE_PREFIX) {
        return mla_string_starts_with(request.url, checker_userdata_ptr->path_data);
    }

    return false;
}

mla_http_server_handler_item_t mla_http_server_handler_starts_with(const mla_string_t &http_method, mla_user_data_t& userdata,
                                                                   const mla_string_t& pathPrefix,
                                                                   const mla_http_request_handler_t &executor) {

    mla_pointer_t checker_userdata = mla_malloc_struct(mla_http_server_path_checker_userdata_t);

    mla_http_server_path_checker_userdata_t *checker_userdata_ptr = mla_pointer_get_data<mla_http_server_path_checker_userdata_t>(checker_userdata);

    if (checker_userdata_ptr == nullptr) {
        return mla_http_server_handler_invalid();
    }

    checker_userdata_ptr->path_data = pathPrefix;

    mla_user_data_set_pointer(userdata, mla_http_server_handler_path_data_user_data_name, checker_userdata);

    return {
        userdata,
        mla_string_to_upper(http_method),
        mla_private_http_server_handler_starts_with_checker,
        executor
    };
}

mla_bool_t mla_private_http_server_handler_all_checker(const mla_user_data_t& userdata, const mla_http_request_t &request, mla_http_request_handler_checker_compare_mode_t compare_mode) {
    (void) userdata;
    (void) request;
    (void) compare_mode;
    return true;
}

mla_http_server_handler_item_t mla_http_server_handler_all(const mla_string_t &http_method, mla_user_data_t& userdata,
                                                           const mla_http_request_handler_t &executor) {
    return {
        userdata,
        mla_string_to_upper(http_method),
        mla_private_http_server_handler_all_checker,
        executor
    };
}

mla_http_server_handler_item_t mla_http_server_handler_invalid() {
    return {
        mla_user_data_empty(),
        mla_string_empty(),
        nullptr,
        nullptr
    };
}

mla_http_server_websocket_connection_t mla_http_server_websocket_connection_invalid() {
    return {
        nullptr,
        mla_network_connection_disconnected(),
        mla_string_empty(),
        mla_string_empty(),
        mla_mutex_invalid(),
        mla_user_data_empty(),
        false,
        nullptr,
        nullptr,
        nullptr,
        nullptr
    };
}

mla_http_server_websocket_connection_t mla_private_http_server_websocket_connection(mla_http_server_t& http_server, mla_network_connection_t& connection, const mla_http_server_websocket_handler_item_t& handlerItem, const mla_string_t& endpoint, mla_bool_t supports_deflate_compression) {

    const mla_string_t id = mla_generate_runtime_id();

#if mla_global_feature_flag_http_server_use_deflate_compression == 1
    mla_bool_t final_supports_deflate_compression = supports_deflate_compression;
#else
    (void)supports_deflate_compression;
    mla_bool_t final_supports_deflate_compression = false;
#endif


    return {
        &http_server,
        connection,
        id,
        endpoint,
        mla_mutex(mla_string_concat("WebSocketConn_", id), true),
        handlerItem.userdata,
        final_supports_deflate_compression,
        handlerItem.open_executor,
        handlerItem.text_executor,
        handlerItem.binary_executor,
        handlerItem.close_executor,
    };
}

mla_http_server_websocket_handler_item_t mla_http_server_websocket_handler(
    mla_user_data_t &userdata, const mla_http_request_handler_checker_t &checker,
    const mla_http_websocket_text_message_handler_t &text_message_handler,
    const mla_http_websocket_binary_message_handler_t &binary_message_handler) {
    return {
        userdata,
        checker,
        nullptr,
        text_message_handler,
        binary_message_handler,
        nullptr
    };
}

mla_http_server_websocket_handler_item_t mla_http_server_websocket_handler_path_equals(
    const mla_string_t& path, const mla_http_websocket_text_message_handler_t &text_message_handler,
    const mla_http_websocket_binary_message_handler_t &binary_message_handler) {

    mla_pointer_t checker_userdata = mla_malloc_struct(mla_http_server_path_checker_userdata_t);

    mla_http_server_path_checker_userdata_t *checker_userdata_ptr = mla_pointer_get_data<mla_http_server_path_checker_userdata_t>(checker_userdata);

    if (checker_userdata_ptr == nullptr) {
        return mla_http_server_websocket_handler_invalid();
    }

    checker_userdata_ptr->path_data = path;

    mla_user_data_t userData = mla_user_data_empty();
    mla_user_data_set_pointer(userData, mla_http_server_handler_path_data_user_data_name, checker_userdata);


    return {
        userData,
        mla_private_http_server_handler_equals_checker,
        nullptr,
        text_message_handler,
        binary_message_handler,
        nullptr
    };
}

mla_http_server_websocket_handler_item_t mla_http_server_websocket_handler_invalid() {
    return {
        mla_user_data_empty(),
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr
    };
}


struct mla_http_server_internal_resource_cleanup_userdata {
    mla_array_list_t<mla_init_struct(mla_string_t)> active_tasks;

    static mla_http_server_internal_resource_cleanup_userdata init() {
        return {
            mla_array_list_empty<mla_init_struct(mla_string_t)>()
        };
    }
};

mla_http_server_t mla_http_server_invalid() {
    return {
        mla_array_list_empty<mla_handler_item_array_param>(),
        mla_array_list_empty<mla_websocket_handler_item_array_param>(),
        mla_array_list_empty<mla_websocket_connection_array_param>(),
        mla_rw_lock_invalid(),
        mla_network_host_invalid(),
        mla_network_listener_invalid(),
        mla_mutex_invalid(),
        mla_pointer_null(),
        MLA_HTTP_SERVER_STATUS_STOPPED,
        0,
        mla_network_security_config_none()
    };
}

mla_http_server_t mla_http_server(const mla_network_host_t &host) {
    return {
        mla_array_list_empty<mla_handler_item_array_param>(),
        mla_array_list_empty<mla_websocket_handler_item_array_param>(),
        mla_array_list_empty<mla_websocket_connection_array_param>(),
        mla_rw_lock(mla_string_concat("HttpServerWebsocketConnections_", host.address.address, ":",
                                    mla_string_from_uint16(host.port)), true),
        host,
        mla_network_listener_invalid(),
        mla_mutex(mla_string_concat("HttpServerListenerLock_", host.address.address, ":",
                                    mla_string_from_uint16(host.port)), true),
        mla_pointer_null(),
        MLA_HTTP_SERVER_STATUS_STOPPED,
        mla_global_config_default_http_timeout_ms,
        mla_network_security_config_none()
    };
}

mla_bool_t mla_http_server_register_handler(mla_http_server_t &server,
                                            const mla_http_server_handler_item_t &handlerItem) {
    if (!mla_mutex_lock(server.listenerLock)) {
        return false;
    }

    if (server.status == MLA_HTTP_SERVER_STATUS_RUNNING) {
        mla_mutex_unlock(server.listenerLock);
        mla_error("Cannot register handler while server is running");
        return false;
    }

    mla_bool_t result = mla_array_list_add(server.httpHandlers, handlerItem);

    mla_mutex_unlock(server.listenerLock);
    return result;
}

mla_bool_t mla_http_server_register_websocket_handler(mla_http_server_t &server,
                                                      const mla_http_server_websocket_handler_item_t &handlerItem) {
    if (!mla_mutex_lock(server.listenerLock)) {
        return false;
    }

    if (server.status == MLA_HTTP_SERVER_STATUS_RUNNING) {
        mla_mutex_unlock(server.listenerLock);
        mla_error("Cannot register websocket handler while server is running");
        return false;
    }

    mla_bool_t result = mla_array_list_add(server.websocketHandlers, handlerItem);

    mla_mutex_unlock(server.listenerLock);
    return result;
}

mla_bool_t mla_private_http_server_request_read(mla_network_connection_t &connection, mla_http_request_t &request,
                                          mla_int32_t timeout_ms) {

    // Read request line
    mla_string_t requestLine = mla_string_empty();
    if (!mla_http_utils_read_line(connection.inputStream, requestLine, timeout_ms)) {
        return false;
    }

    // Parse request line
    mla_array_list_t<mla_init_struct(mla_string_t)> parts = mla_string_split(requestLine, mla_string_const(" "));

    if (mla_array_list_size(parts) < 3) {
        return false;
    }

    // Method
    request.method = mla_string_to_upper(*mla_array_list_get_ref(parts, 0));
    // URL
    request.url = *mla_array_list_get_ref(parts, 1);
    // HTTP Version
    mla_string_t versionStr = *mla_array_list_get_ref(parts, 2);

    if (!mla_http_utils_parse_http_version(versionStr, request.version)) {
        return false; // Unsupported HTTP version
    }

    // Read headers
    if (!mla_http_utils_read_headers(request.headers, connection.inputStream, timeout_ms)) {
        return false;
    }

    // Body (if any)
    // Check for Content-Length header
    mla_string_t contentLengthStr = mla_http_headers_get_value(request.headers, mla_string_const("Content-Length"));

    if (mla_string_equals_const(contentLengthStr, "0")) {
        request.content = mla_stream_noop_input();
        return true;
    }

    mla_size_t content_size = 0;

    if (mla_parse_uint32(contentLengthStr, content_size)) {
        request.content = mla_http_content_fixed_size_input_stream(connection.inputStream, timeout_ms, content_size);
        return true;
    }

    // No Content-Length, use chunked or until close
    mla_string_t transferEncoding = mla_http_headers_get_value(request.headers, mla_string_const("Transfer-Encoding"));

    if (mla_string_equals_const(transferEncoding, "chunked")) {

        // Check if we need switch to deflate
        if (mla_http_headers_has_header_value(request.headers, mla_string_const("Content-Encoding"), mla_string_const("deflate"))) {
            request.content = mla_http_chunked_stream_input_deflate(connection.inputStream, timeout_ms);
        } else {
            request.content = mla_http_chunked_stream_input(connection.inputStream, timeout_ms);
        }

        return true;
    }

    request.content = mla_stream_noop_input();
    return true;
}

mla_bool_t mla_private_http_server_response_send(mla_network_connection_t &connection,
                                           mla_http_response_t &response) {


    // Send status line
    mla_string_t httpVersion = (response.version == MLA_HTTP_VERSION_1_1)
                                   ? mla_string_const("HTTP/1.1 ")
                                   : mla_string_const("HTTP/1.0 ");

    if (!mla_stream_output_write_string(connection.outputStream, httpVersion)) {
        mla_stream_output_flush_buffered_wrapper(connection.outputStream);
        return false;
    }

    if (!mla_stream_output_write_string(connection.outputStream, mla_string_from_uint16(response.statusCode))) {
        mla_stream_output_flush_buffered_wrapper(connection.outputStream);
        return false;
    }

    if (!mla_stream_output_write_string(connection.outputStream, mla_string_const(" "))) {
        mla_stream_output_flush_buffered_wrapper(connection.outputStream);
        return false;
    }

    if (!mla_stream_output_write_string(connection.outputStream, response.statusMessage)) {
        mla_stream_output_flush_buffered_wrapper(connection.outputStream);
        return false;
    }

    if (!mla_stream_output_write_string(connection.outputStream, mla_string_const("\r\n"))) {
        mla_stream_output_flush_buffered_wrapper(connection.outputStream);
        return false;
    }

    // Send headers
    if (!mla_http_utils_write_headers(response.headers, connection.outputStream)) {
        mla_stream_output_flush_buffered_wrapper(connection.outputStream);
        return false;
    }

    // Write Content Length if body is present
    if (!mla_http_utils_write_content_headers(response.headers, response.content, connection.outputStream)) {
        mla_stream_output_flush_buffered_wrapper(connection.outputStream);
        return false;
    }

    if (!mla_stream_output_write_string(connection.outputStream, mla_string_const("\r\n"))) {
        mla_stream_output_flush_buffered_wrapper(connection.outputStream);
        return false;
    }

    // Send body
    if (mla_http_response_content_writer_is_valid(response.contentWriter)) {

        if (!response.contentWriter.writeTo(response.contentWriter, connection.outputStream)) {
            mla_stream_output_flush_buffered_wrapper(connection.outputStream);
            return false;
        }

    } else {
        // No content writer, send content stream
        if (!mla_stream_copy(response.content, connection.outputStream)) {
            mla_stream_output_flush_buffered_wrapper(connection.outputStream);
            return false;
        }
    }

    if (!mla_stream_output_flush_buffered_wrapper(connection.outputStream)) {
        return false;
    }

    return true;
}

mla_string_t mla_private_http_server_compute_websocket_accept(const mla_string_t& secWebSocketKey) {
    const mla_string_t guid = mla_string_const("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    mla_string_t input = mla_string_concat(secWebSocketKey, guid);

    mla_bytes_t inputBytes = mla_bytes_from_string(input);
    mla_bytes_t digest = mla_sha1(inputBytes);
    mla_string_t accept = mla_bytes_to_base64(digest);
    return accept;
}


mla_task_process_result_state mla_private_http_server_handler_new_request(mla_user_data_t& userData) {

    mla_pointer_t http_server = mla_user_data_get_pointer(userData, mla_http_task_user_data_server_name);

    mla_http_server_t* http_server_ptr = mla_pointer_get_data<mla_http_server_t>(http_server);

    if (http_server_ptr == nullptr) {
        return TASK_PROCESS_RESULT_DONE; // Server reference lost, exit task
    }

    mla_http_server_t& server = *http_server_ptr;

    if (server.status != MLA_HTTP_SERVER_STATUS_RUNNING) {
        return TASK_PROCESS_RESULT_DONE; // Server is not running, exit task
    }

    // Accept a new connection

    if (!mla_mutex_trylock(server.listenerLock, 100)) {
        return TASK_PROCESS_RESULT_CONTINUE; // No connection, yield and try again
    }

    if (server.status != MLA_HTTP_SERVER_STATUS_RUNNING) {
        return TASK_PROCESS_RESULT_DONE; // Server stopped while accepting, exit task
    }

    mla_network_connection_t clientConnection = mla_network_connection_disconnected();

    if (!mla_network_listener_accept_connection(server.listener, clientConnection)) {
        mla_mutex_unlock(server.listenerLock);
        return TASK_PROCESS_RESULT_CONTINUE; // No connection, yield and try again
    }

    mla_mutex_unlock(server.listenerLock);

    if (server.status != MLA_HTTP_SERVER_STATUS_RUNNING) {
        clientConnection = mla_network_connection_disconnected();
        return TASK_PROCESS_RESULT_DONE; // Server stopped while accepting, exit task
    }

    // apply buffers to the connection
    clientConnection.inputStream = mla_stream_input_buffered_wrapper(clientConnection.inputStream, mla_http_server_connection_input_buffer_size);
    clientConnection.outputStream = mla_stream_output_buffered_wrapper(clientConnection.outputStream, mla_http_server_connection_output_buffer_size);

    mla_http_request_t request = mla_http_request_empty();

    if (!mla_private_http_server_request_read(clientConnection, request, server.timeout_ms)) {
        mla_warning(
            mla_string_concat("Invalid http request from client ", clientConnection.host.address.address, ":",
                mla_string_from_uint16(clientConnection.host.port)));
        clientConnection = mla_network_connection_disconnected();
        return TASK_PROCESS_RESULT_CONTINUE;
    }

    if (server.status != MLA_HTTP_SERVER_STATUS_RUNNING) {
        clientConnection = mla_network_connection_disconnected();
        return TASK_PROCESS_RESULT_DONE; // Server stopped while accepting, exit task
    }

    mla_array_list_t<mla_handler_item_array_param> copyHandlers = server.
            httpHandlers;

    mla_http_response_t response = mla_http_response_empty();
    response.version = request.version;
    response.statusCode = 404; // Default to Not Found
    response.statusMessage = mla_string_const("Not Found");

    // Detect WebSocket upgrade requests BEFORE HTTP handler matching.
    // A WebSocket upgrade MUST be routed to the WS handler path, not the HTTP handler loop,
    // even if an HTTP handler (e.g. a proxy with empty method) would otherwise match.
    mla_string_t upgrade_header = mla_http_headers_get_value(request.headers, mla_string_const("Upgrade"));
    mla_bool_t is_websocket_upgrade = mla_string_equals_ignore_case(upgrade_header, mla_string_const("websocket"));

    mla_bool_t processed = false;

    // Find a handler for the request (skip for WebSocket upgrades)
    if (!is_websocket_upgrade) {
        for (mla_size_t i = 0; i < mla_array_list_size(copyHandlers); i++) {
            mla_http_server_handler_item_t &handlerItem = mla_array_list_get_unsafe(copyHandlers, i);

            if (handlerItem.checker == nullptr) {
                continue; // No checker function, skip
            }

            // Empty method means "match any HTTP method" (used by proxy and catch-all handlers)
            if (!mla_string_is_empty(handlerItem.method) && !mla_string_equals(request.method, handlerItem.method)) {
                continue;
            }

            if (!handlerItem.checker(handlerItem.userdata, request, MLA_HTTP_REQUEST_HANDLER_CHECKER_COMPARE_MODE_PERFECT_MATCH)) {
                continue; // Checker returned false, skip
            }

            if (handlerItem.executor != nullptr) {
                if (!handlerItem.executor(server, handlerItem.userdata, request, response)) {
                    mla_warning(mla_string_concat("Handler executor failed for URL: ", request.url));
                    response.statusCode = 500; // Internal Server Error
                    response.statusMessage = mla_string_const("Internal Server Error");
                }
            }
            processed = true;

            break;
        }
    }

    if (server.status != MLA_HTTP_SERVER_STATUS_RUNNING) {
        clientConnection = mla_network_connection_disconnected();
        return TASK_PROCESS_RESULT_DONE; // Server stopped while accepting, exit task
    }

    if (!processed) {

        // Check for websocket handlers
        mla_array_list_t<mla_websocket_handler_item_array_param> copyWebsocketHandlers = server.
            websocketHandlers;

        // Find a handler for the request
        for (mla_size_t i = 0; i < mla_array_list_size(copyWebsocketHandlers); i++) {

            mla_http_server_websocket_handler_item_t &handlerItem = mla_array_list_get_unsafe(copyWebsocketHandlers, i);

            if (handlerItem.checker == nullptr) {
                continue; // No checker function, skip
            }

            if (!handlerItem.checker(handlerItem.userdata, request, MLA_HTTP_REQUEST_HANDLER_CHECKER_COMPARE_MODE_PERFECT_MATCH)) {
                continue; // Checker returned false, skip
            }

            // ... inside the websocket handler match block ...
            // 1) Validate and compute accept
            mla_string_t wsKey = mla_http_headers_get_value(request.headers, mla_string_const("Sec-WebSocket-Key"));
            if (mla_string_is_empty(wsKey)) {
                // Reject bad handshake
                response.statusCode = 400;
                response.statusMessage = mla_string_const("Bad Request");
                break;
            }

            mla_string_t accept = mla_private_http_server_compute_websocket_accept(wsKey);

            mla_bool_t supports_deflate_compression = mla_http_headers_has_header_value(request.headers, mla_string_const("Sec-WebSocket-Extensions"), mla_string_const("permessage-deflate"), mla_string_const(","));

            // 2) Build 101 response (no body)
            response.statusCode = mla_http_status_switching_protocols; // 101
            response.statusMessage = mla_string_const("Switching Protocols");
            mla_http_headers_add(response.headers, mla_string_const("Connection"), mla_string_const("Upgrade"));
            mla_http_headers_add(response.headers, mla_string_const("Upgrade"), mla_string_const("websocket"));
            mla_http_headers_add(response.headers, mla_string_const("Sec-WebSocket-Accept"), accept);

#if mla_global_feature_flag_http_server_use_deflate_compression == 1

            if (supports_deflate_compression) {
                // Advertise client_no_context_takeover and server_no_context_takeover because
                // the server creates a fresh DEFLATE compressor/decompressor for every message.
                // Without these parameters third-party clients may reuse their compression
                // window across messages, causing the server's per-message decompressor to
                // produce corrupt output.
                mla_http_headers_add(response.headers, mla_string_const("Sec-WebSocket-Extensions"), mla_string_const("permessage-deflate; client_no_context_takeover; server_no_context_takeover"));
            }
#endif
            response.content = mla_stream_noop_input(); // ensure no body

            // 3) Register the WebSocket connection and keep it open
            mla_http_server_websocket_connection_t ws_connection = mla_private_http_server_websocket_connection(server, clientConnection, handlerItem, request.url, supports_deflate_compression);


            if (ws_connection.open_executor != nullptr) {
                if (!ws_connection.open_executor(ws_connection)) {
                    ws_connection = mla_http_server_websocket_connection_invalid();
                    break;
                }
            }

            if (mla_rw_lock_write(server.websocketConnectionsLock)) {
                if (mla_array_list_add(server.websocketConnections, ws_connection)) {
                    mla_info(mla_string_concat("Registered new WebSocket connection with id ", ws_connection.id, " for endpoint ", request.url));
                } else {
                    ws_connection = mla_http_server_websocket_connection_invalid(); // Mark as invalid to avoid processing
                }
                mla_rw_unlock_write(server.websocketConnectionsLock);
            } else {
                ws_connection = mla_http_server_websocket_connection_invalid(); // Mark as invalid to avoid processing
            }

            break;

        }
    }

    // Regular Http response
    if (!mla_private_http_server_response_send(clientConnection, response)) {

        mla_warning(
            mla_string_concat("Failed to send HTTP response to client ", clientConnection.host.address.address, ":",
                mla_string_from_uint16(clientConnection.host.port), " for URL: ", request.url));
    }

    clientConnection = mla_network_connection_disconnected();
    return TASK_PROCESS_RESULT_CONTINUE;

}

mla_bool_t mla_private_http_server_remove_websocket_connection(mla_http_server_t &server,
                                                     const mla_http_server_websocket_connection_t &connection) {

    if (!mla_rw_lock_read(server.websocketConnectionsLock)) {
        return false;
    }

    mla_int32_t index = -1;

    for (mla_size_t i = 0; i < mla_array_list_size(server.websocketConnections); ++i) {

        mla_http_server_websocket_connection_t& current_connection = mla_array_list_get_unsafe(server.websocketConnections, i);

        if (mla_string_equals(current_connection.id, connection.id)) {
            index = mla_s_cast<mla_int32_t>(i);
            break;
        }

    }

    mla_rw_unlock_read(server.websocketConnectionsLock);

    if (connection.closed_executor != nullptr) {
        connection.closed_executor(connection);
    }

    if (index < 0) {
        return false;
    }

    if (!mla_rw_lock_write(server.websocketConnectionsLock)) {
        return false;
    }

    // Hot Path
    if (mla_s_cast<mla_size_t>(index) < mla_array_list_size(server.websocketConnections)) {

        mla_http_server_websocket_connection_t& current_connection = mla_array_list_get_unsafe(server.websocketConnections, index);

        if (mla_string_equals(current_connection.id, connection.id)) {

            if (mla_array_list_remove(server.websocketConnections, index)) {
                mla_rw_unlock_write(server.websocketConnectionsLock);
                mla_info(mla_string_concat("Removed WebSocket connection with id ", connection.id, " from server "));
                return true;
            }
        }

    }

    // Fallback to linear search remove if the index is out of bounds or the id doesn't match (concurrent modification case)
    index = -1;

    for (mla_size_t i = 0; i < mla_array_list_size(server.websocketConnections); ++i) {

        mla_http_server_websocket_connection_t& current_connection = mla_array_list_get_unsafe(server.websocketConnections, i);

        if (mla_string_equals(current_connection.id, connection.id)) {
            index = mla_s_cast<mla_int32_t>(i);
            break;
        }

    }

    if (index < 0) {
        mla_rw_unlock_write(server.websocketConnectionsLock);
        return false;
    }

    if (mla_array_list_remove(server.websocketConnections, index)) {
        mla_rw_unlock_write(server.websocketConnectionsLock);
        mla_info(mla_string_concat("Removed WebSocket connection with id ", connection.id, " from server "));
        return true;
    }
    mla_rw_unlock_write(server.websocketConnectionsLock);
    mla_error(mla_string_concat("Failed to remove WebSocket connection with id ", connection.id, " from server "));
    return false;
}

mla_task_process_result_state mla_private_http_server_handler_websocket_messages(mla_user_data_t& userData) {

    mla_pointer_t http_server = mla_user_data_get_pointer(userData, mla_http_task_user_data_server_name);

    mla_http_server_t* http_server_ptr = mla_pointer_get_data<mla_http_server_t>(http_server);

    if (http_server_ptr == nullptr) {
        return TASK_PROCESS_RESULT_DONE; // Server reference lost, exit task
    }

    mla_http_server_t& server = *http_server_ptr;

    if (server.status != MLA_HTTP_SERVER_STATUS_RUNNING) {
        return TASK_PROCESS_RESULT_DONE; // Server is not running, exit task
    }

    if (!mla_rw_lock_read(server.websocketConnectionsLock)) {
        return TASK_PROCESS_RESULT_CONTINUE;
    }

    mla_size_t connectionCount = mla_array_list_size(server.websocketConnections);

    if (mla_array_list_size(server.websocketConnections) == 0) {
        mla_rw_unlock_read(server.websocketConnectionsLock);
        return TASK_PROCESS_RESULT_CONTINUE; // No connections, yield and try again
    }

    mla_array_list_t<mla_websocket_connection_array_param> copyConnections = mla_array_list<mla_websocket_connection_array_param>(connectionCount);
    mla_array_list_add_all(copyConnections, server.websocketConnections);

    mla_rw_unlock_read(server.websocketConnectionsLock);

    for (mla_size_t i = 0; i < mla_array_list_size(copyConnections); ++i) {

        if (server.status != MLA_HTTP_SERVER_STATUS_RUNNING) {
            return TASK_PROCESS_RESULT_DONE; // Server is not running, exit task
        }

        mla_http_server_websocket_connection_t connection = mla_array_list_get_unsafe(copyConnections, i);

        if (!mla_http_server_is_websocket_connection_open(connection)) {
            continue;
        }

        // We lock the connection for the whole processing time because
        // if multiple task are processing messages for the same connection at the same time,
        // it can cause issues with the order of messages and the state of the connection.
        // For example, if one task is processing a text message and another task receives a close message for the same connection
        if (!mla_mutex_trylock(connection.lock, 25)) {
            continue;
        }

        mla_string_t textMessage = mla_string_empty();
        mla_bytes_t binaryMessage = mla_bytes_empty();

        mla_websocket_transport_message_receive_type_t receive_type = mla_websocket_transport_receive_message(connection.connection, 20, textMessage, binaryMessage, false);

        enum mla_websocket_connection_action_t: mla_uint8_t {
            MLA_WEBSOCKET_CONNECTION_NONE,
            MLA_WEBSOCKET_CONNECTION_CLOSE,
            MLA_WEBSOCKET_CONNECTION_REMOVE,
        };

        mla_websocket_connection_action_t action = MLA_WEBSOCKET_CONNECTION_NONE;

        switch (receive_type) {
            case MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TEXT:
                if (connection.text_executor != nullptr) {
                    if (!connection.text_executor(connection, textMessage)) {
                        mla_warning(mla_string_concat("WebSocket text message handler failed for connection ", connection.id, ". Closing connection."));
                        action = MLA_WEBSOCKET_CONNECTION_CLOSE;
                    }
                } else {
                    mla_warning(mla_string_concat("No WebSocket text message handler registered for connection ", connection.id, ". Closing connection."));
                    action = MLA_WEBSOCKET_CONNECTION_CLOSE;
                }
                break;
            case MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_BINARY:
                if (connection.binary_executor != nullptr) {
                    if (!connection.binary_executor(connection, binaryMessage)) {
                        mla_warning(mla_string_concat("WebSocket binary message handler failed for connection ", connection.id, ". Closing connection."));
                        action = MLA_WEBSOCKET_CONNECTION_CLOSE;
                    }
                } else {
                    mla_warning(mla_string_concat("No WebSocket binary message handler registered for connection ", connection.id, ". Closing connection."));
                    action = MLA_WEBSOCKET_CONNECTION_CLOSE;
                }
                break;
            case MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_CLOSED:
                action = MLA_WEBSOCKET_CONNECTION_REMOVE;
                break;
            case MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_PONG:
                // Ignore pong messages
                break;
            case MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TIMEOUT:
                action = MLA_WEBSOCKET_CONNECTION_NONE;
                break;
            case MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_NO_MESSAGE:
                // No message received -> Nothing to do
                break;
        }

        if (action == MLA_WEBSOCKET_CONNECTION_NONE) {

            if (!mla_stream_output_flush_buffered_wrapper(connection.connection.outputStream)) {
                mla_warning(mla_string_concat("Failed to flush WebSocket connection ", connection.id, " output stream. Closing connection."));
                action = MLA_WEBSOCKET_CONNECTION_CLOSE;
            }
        }
        mla_mutex_unlock(connection.lock);

        if (action == MLA_WEBSOCKET_CONNECTION_CLOSE) {
            mla_http_server_close_websocket_connection(connection, mla_websocket_close_abnormal, mla_string_const("Closing connection"));
        } else if (action == MLA_WEBSOCKET_CONNECTION_REMOVE) {
            mla_private_http_server_remove_websocket_connection(server, connection);
        }

    }


    return TASK_PROCESS_RESULT_CONTINUE;

}

mla_task_process_result_state mla_private_http_server_handler_task(mla_user_data_t& userData) {

    // Process new http request
    mla_task_process_result_state state = mla_private_http_server_handler_new_request(userData);

    if (state == TASK_PROCESS_RESULT_DONE) {
        return TASK_PROCESS_RESULT_DONE;
    }

    // Process websocket messages
    return mla_private_http_server_handler_websocket_messages(userData);
}

void mla_private_http_server_cleanup_hook(mla_http_server_internal_resource_cleanup_userdata& cleanup_userdata) {

    for (mla_size_t i = 0; i < mla_array_list_size(cleanup_userdata.active_tasks); ++i) {
        mla_string_t &task_name = mla_array_list_get_unsafe(cleanup_userdata.active_tasks, i);
        if (!mla_task_manager_abort_task(task_name)) {
            mla_warning(mla_string_concat("Failed to abort HTTP server task ", task_name));
        }
    }

    mla_sleep(100); // Give some time for tasks to abort


}

mla_bool_t mla_http_server_start(mla_http_server_t &server, mla_uint8_t number_of_tasks, mla_task_stack_size task_stack_size) {
    if (!mla_mutex_lock(server.listenerLock)) {
        return false;
    }

    if (server.status == MLA_HTTP_SERVER_STATUS_RUNNING) {
        mla_mutex_unlock(server.listenerLock);
        return true;
    }

    if (number_of_tasks == 0) {
        server.status = MLA_HTTP_SERVER_STATUS_ERROR;
        mla_mutex_unlock(server.listenerLock);
        mla_error("Http server must have at least one task to run");
        return false;
    }

    if (mla_array_list_size(server.httpHandlers) == 0 && mla_array_list_size(server.websocketHandlers) == 0) {
        server.status = MLA_HTTP_SERVER_STATUS_ERROR;
        mla_mutex_unlock(server.listenerLock);
        mla_error("No handlers registered for HTTP server");
        return false;
    }

    if (!mla_network_listener_bind_and_listen_secure(server.listener, server.host, mla_connection_type_tcp,
                                                     server.security_config)) {
        server.status = MLA_HTTP_SERVER_STATUS_ERROR;
        mla_mutex_unlock(server.listenerLock);
        mla_error(
            mla_string_concat("Failed to bind and listen on ", server.host.address.address, ":", mla_string_from_uint16(
                server.host.port)));
        return false;
    }

    server.status = MLA_HTTP_SERVER_STATUS_RUNNING;

    server.serverOwner = mla_malloc_struct_cleanup_hook(mla_http_server_internal_resource_cleanup_userdata, mla_private_http_server_cleanup_hook);

    mla_http_server_internal_resource_cleanup_userdata *cleanup_userdata = mla_pointer_get_data<mla_http_server_internal_resource_cleanup_userdata>(server.serverOwner);

    if (cleanup_userdata == nullptr) {
        server.status = MLA_HTTP_SERVER_STATUS_ERROR;
        mla_mutex_unlock(server.listenerLock);
        mla_error("Failed to allocate memory for HTTP server cleanup userdata");
        return false;
    }

    mla_memset(cleanup_userdata, 0, sizeof(mla_http_server_internal_resource_cleanup_userdata));
    cleanup_userdata->active_tasks = mla_array_list_empty<mla_init_struct(mla_string_t)>();

    mla_string_t runtime_id = mla_generate_runtime_id();

    mla_user_data_t taskUserData = mla_user_data_empty();
    mla_pointer_t http_server_ptr = mla_platform_pointer_to_managed_pointer(&server);

    mla_user_data_set_pointer(taskUserData, mla_http_task_user_data_server_name, http_server_ptr);

    for (mla_uint8_t i = 0; i < number_of_tasks; i++) {
        mla_string_t task_name = mla_string_concat("HttpServerTask_", server.host.address.address, ":",
                                                   mla_string_from_uint16(server.host.port), "_",
                                                   mla_string_from_uint8(i), "_", runtime_id);

        mla_task_t http_task = mla_task_repeating(
            task_name,
            mla_private_http_server_handler_task,
            taskUserData
        );

        mla_task_update_stack_size(http_task, task_stack_size);

        if (!mla_task_manager_register_task(http_task)) {
            server.status = MLA_HTTP_SERVER_STATUS_ERROR;
            mla_mutex_unlock(server.listenerLock);
            server.serverOwner = mla_pointer_null();
            mla_error(mla_string_concat("Failed to register HTTP server task ", http_task.name));
            return false;
        }

        mla_array_list_add(cleanup_userdata->active_tasks, task_name);
    }

    mla_mutex_unlock(server.listenerLock);
    mla_info(
        mla_string_concat("Started HTTP server on ", server.host.address.address, ":", mla_string_from_uint16(server.
            host.port), " with " , mla_string_from_uint8(number_of_tasks), " tasks"));

    return true;
}

mla_bool_t mla_http_server_running(const mla_http_server_t &server) {
    return (server.status == MLA_HTTP_SERVER_STATUS_RUNNING);
}

void mla_http_server_set_timeout(mla_http_server_t &server, mla_int32_t timeout_ms) {
    server.timeout_ms = timeout_ms;
}

void mla_http_server_set_security_config(mla_http_server_t &server,
                                         const mla_network_security_config_t &security_config) {
    server.security_config = security_config;
}

void mla_http_server_enable_tls(mla_http_server_t &server, const mla_network_tls_config_t &tls_config) {
    server.security_config = mla_network_security_config_tls(tls_config);
}

void mla_http_server_disable_security(mla_http_server_t &server) {
    server.security_config = mla_network_security_config_none();
}

mla_bool_t mla_http_server_stop(mla_http_server_t &server) {

    if (!mla_mutex_lock(server.listenerLock)) {
        return false;
    }

    if (!mla_rw_lock_write(server.websocketConnectionsLock)) {
        mla_mutex_unlock(server.listenerLock);
        return false;
    }

    if (server.status != MLA_HTTP_SERVER_STATUS_RUNNING) {
        mla_mutex_unlock(server.listenerLock);
        mla_rw_unlock_write(server.websocketConnectionsLock);
        return true;
    }

    mla_bool_t all_closed = true;

    if (!mla_network_listener_close(server.listener)) {
        mla_error("Failed to close HTTP server listener");
        all_closed = false;
    }

    // Close all active websocket connections
    for (mla_size_t i = 0; i < mla_array_list_size(server.websocketConnections); ++i) {
        mla_http_server_websocket_connection_t &connection = mla_array_list_get_unsafe(server.websocketConnections, i);

        if (mla_http_server_is_websocket_connection_open(connection)) {
            mla_websocket_transport_send_close_frame(connection.connection.outputStream, mla_websocket_close_normal, mla_string_const("Server shutting down"), false);
            mla_stream_output_flush_buffered_wrapper(connection.connection.outputStream);
        }

        if (connection.closed_executor != nullptr) {
            connection.closed_executor(connection);
        }
    }

    mla_array_list_clear(server.websocketConnections);

    if (!all_closed) {
        mla_mutex_unlock(server.listenerLock);
        mla_rw_unlock_write(server.websocketConnectionsLock);
        // Some resources failed to close
        // Keep the server in error state
        server.status = MLA_HTTP_SERVER_STATUS_ERROR;
        return false;
    }

    server.status = MLA_HTTP_SERVER_STATUS_STOPPED;
    mla_mutex_unlock(server.listenerLock);
    mla_rw_unlock_write(server.websocketConnectionsLock);
    server.serverOwner = mla_pointer_null();

    mla_info(
        mla_string_concat("HTTP server stopped on ", server.host.address.address, ":", mla_string_from_uint16(server.
            host.port)));

    return true;
}


mla_bool_t mla_http_server_find_websocket_connection(mla_http_server_t &server, const mla_string_t &connectionId,
                                                     mla_http_server_websocket_connection_t &outConnection) {
    if (!mla_rw_lock_read(server.websocketConnectionsLock)) {
        return false;
    }


    for (mla_size_t i = 0; i < mla_array_list_size(server.websocketConnections); ++i) {
        mla_http_server_websocket_connection_t &current_connection = mla_array_list_get_unsafe(
            server.websocketConnections, i);

        if (mla_string_equals(current_connection.id, connectionId)) {
            outConnection = current_connection;
            mla_rw_unlock_read(server.websocketConnectionsLock);
            return true;
        }
    }

    mla_rw_unlock_read(server.websocketConnectionsLock);
    return false;
}

mla_array_list_t<mla_init_struct(mla_http_server_websocket_connection_t)> mla_http_server_find_websocket_connections(mla_http_server_t &server, const mla_string_t &path_prefix) {
    mla_array_list_t<mla_init_struct(mla_http_server_websocket_connection_t)> result = mla_array_list_empty<mla_init_struct(mla_http_server_websocket_connection_t)>();

    if (!mla_rw_lock_read(server.websocketConnectionsLock)) {
        return result;
    }

    for (mla_size_t i = 0; i < mla_array_list_size(server.websocketConnections); ++i) {

        mla_http_server_websocket_connection_t &current_connection = mla_array_list_get_unsafe(server.websocketConnections, i);

        if (mla_string_starts_with(current_connection.endpoint, path_prefix)) {
            mla_array_list_add(result, current_connection);
        }
    }

    mla_rw_unlock_read(server.websocketConnectionsLock);
    return result;
}

mla_size_t mla_http_server_get_websocket_connection_count(mla_http_server_t &server, const mla_string_t &path_prefix) {
    mla_size_t count = 0;

    if (!mla_rw_lock_read(server.websocketConnectionsLock)) {
        return count;
    }

    for (mla_size_t i = 0; i < mla_array_list_size(server.websocketConnections); ++i) {

        mla_http_server_websocket_connection_t &current_connection = mla_array_list_get_unsafe(server.websocketConnections, i);

        if (mla_string_starts_with(current_connection.endpoint, path_prefix)) {
            count++;
        }
    }

    mla_rw_unlock_read(server.websocketConnectionsLock);
    return count;
}

mla_bool_t mla_http_server_is_websocket_connection_open(const mla_http_server_websocket_connection_t &connection) {
    if (mla_string_is_empty(connection.id)) {
        return false;
    }

    return mla_network_connection_is_connected(connection.connection);
}

mla_array_list_t<mla_init_struct(mla_http_server_websocket_handler_item_t)> mla_http_server_get_websocket_handler_for_path(mla_http_server_t &server, const mla_string_t& path) {

    mla_array_list_t<mla_init_struct(mla_http_server_websocket_handler_item_t)> result = mla_array_list_empty<mla_init_struct(mla_http_server_websocket_handler_item_t)>();

    mla_http_request_t path_request = mla_http_request(path, mla_http_method_get);

    for (mla_size_t i = 0; i < mla_array_list_size(server.websocketHandlers); ++i) {

        mla_http_server_websocket_handler_item_t &handlerItem = mla_array_list_get_unsafe(server.websocketHandlers, i);

        if (handlerItem.checker != nullptr && handlerItem.checker(handlerItem.userdata, path_request, MLA_HTTP_REQUEST_HANDLER_CHECKER_COMPARE_MODE_PREFIX)) {
            mla_array_list_add(result, handlerItem);
        }
    }

    return result;
}

mla_bool_t mla_http_server_close_websocket_connection(mla_http_server_websocket_connection_t &connection,
                                                      mla_uint16_t closeCode, const mla_string_t &reason) {

    if (mla_http_server_is_websocket_connection_open(connection)) {

        if (mla_mutex_lock(connection.lock)) {

            mla_websocket_transport_send_close_frame(connection.connection.outputStream, closeCode, reason, false);
            mla_stream_output_flush_buffered_wrapper(connection.connection.outputStream);
            mla_mutex_unlock(connection.lock);
        }

    }

    if (connection.server == nullptr) {
        return true;
    }

    mla_private_http_server_remove_websocket_connection(*connection.server, connection);
    connection = mla_http_server_websocket_connection_invalid();

    return true;
}

mla_bool_t mla_http_server_close_websocket_connection(mla_http_server_t &server, const mla_string_t &connectionId,
                                                      mla_uint16_t closeCode, const mla_string_t &reason) {
    mla_http_server_websocket_connection_t connection = mla_http_server_websocket_connection_invalid();

    if (!mla_http_server_find_websocket_connection(server, connectionId, connection)) {
        return false;
    }

    return mla_http_server_close_websocket_connection(connection, closeCode, reason);
}

mla_bool_t mla_http_server_send_websocket_text_message(mla_http_server_websocket_connection_t& connection, mla_user_data_t &userData, mla_websocket_transport_message_generator_t message_generator, mla_bool_t use_deflate_compression_if_supported) {
    return mla_http_server_try_send_websocket_text_message(connection, userData, message_generator, -1, use_deflate_compression_if_supported);
}

mla_bool_t mla_http_server_send_websocket_text_message(mla_http_server_websocket_connection_t &connection, const mla_string_t &message, mla_bool_t use_deflate_compression_if_supported) {
    return mla_http_server_try_send_websocket_text_message(connection, message, -1, use_deflate_compression_if_supported);
}

mla_bool_t mla_http_server_send_websocket_text_message(mla_http_server_t &server, const mla_string_t &connectionId, const mla_string_t &message, mla_bool_t use_deflate_compression_if_supported) {
    mla_http_server_websocket_connection_t connection = mla_http_server_websocket_connection_invalid();

    if (!mla_http_server_find_websocket_connection(server, connectionId, connection)) {
        return false;
    }

    return mla_http_server_send_websocket_text_message(connection, message, use_deflate_compression_if_supported);
}

mla_bool_t mla_http_server_try_send_websocket_text_message(mla_http_server_websocket_connection_t& connection, mla_user_data_t &userData, mla_websocket_transport_message_generator_t message_generator, mla_int32_t connection_lock_timeout, mla_bool_t use_deflate_compression_if_supported) {

    if (!mla_http_server_is_websocket_connection_open(connection)) {
        return false;
    }

    if (connection_lock_timeout <= 0) {
        if (!mla_mutex_lock(connection.lock)) {
            return false;
        }
    } else {
        if (!mla_mutex_trylock(connection.lock, ((mla_int32_t)connection_lock_timeout))) {
            return false;
        }
    }

    mla_bool_t use_deflate_compression = connection.supports_deflate_compression && use_deflate_compression_if_supported;

    if (mla_websocket_transport_send_text_with_generator(connection.connection.outputStream, userData, message_generator, false, use_deflate_compression)) {

        mla_bool_t flush_successful = mla_stream_output_flush_buffered_wrapper(connection.connection.outputStream);
        mla_mutex_unlock(connection.lock);
        return flush_successful;
    }
    mla_mutex_unlock(connection.lock);

    if (connection.server == nullptr) {
        return false;
    }

    // Something is wrong with the connection, close it to avoid further issues
    mla_http_server_close_websocket_connection(connection, mla_websocket_close_abnormal, mla_string_const("Failed to send text message"));
    return false;

}

mla_bool_t mla_http_server_try_send_websocket_text_message(mla_http_server_websocket_connection_t& connection, const mla_string_t& message, mla_int32_t connection_lock_timeout, mla_bool_t use_deflate_compression_if_supported) {

    if (!mla_http_server_is_websocket_connection_open(connection)) {
        return false;
    }

    if (connection_lock_timeout <= 0) {
        if (!mla_mutex_lock(connection.lock)) {
            return false;
        }
    } else {
        if (!mla_mutex_trylock(connection.lock, connection_lock_timeout)) {
            return false;
        }
    }

    mla_bool_t use_deflate_compression = connection.supports_deflate_compression && use_deflate_compression_if_supported;

    if (mla_websocket_transport_send_text_frame(connection.connection.outputStream, message, false, use_deflate_compression)) {

        mla_bool_t flush_successful = mla_stream_output_flush_buffered_wrapper(connection.connection.outputStream);
        mla_mutex_unlock(connection.lock);
        return flush_successful;
    }
    mla_mutex_unlock(connection.lock);

    if (connection.server == nullptr) {
        return false;
    }

    mla_http_server_close_websocket_connection(connection, mla_websocket_close_abnormal, mla_string_const("Failed to send text message"));
    return false;

}

mla_bool_t mla_http_server_try_send_websocket_text_message(mla_http_server_t &server, const mla_string_t& connectionId, const mla_string_t& message, mla_int32_t connection_lock_timeout, mla_bool_t use_deflate_compression_if_supported) {

    mla_http_server_websocket_connection_t connection = mla_http_server_websocket_connection_invalid();

    if (!mla_http_server_find_websocket_connection(server, connectionId, connection)) {
        return false;
    }

    return mla_http_server_try_send_websocket_text_message(connection, message, connection_lock_timeout, use_deflate_compression_if_supported);

}

mla_bool_t mla_http_server_send_websocket_binary_message(mla_http_server_websocket_connection_t& connection, mla_user_data_t &userData, mla_websocket_transport_message_generator_t message_generator, mla_bool_t use_deflate_compression_if_supported) {

    return mla_http_server_try_send_websocket_binary_message(connection, userData, message_generator, -1, use_deflate_compression_if_supported);
}

mla_bool_t mla_http_server_send_websocket_binary_message(mla_http_server_websocket_connection_t &connection, const mla_bytes_t &message, mla_bool_t use_deflate_compression_if_supported) {

    return mla_http_server_try_send_websocket_binary_message(connection, message, -1, use_deflate_compression_if_supported);
}

mla_bool_t mla_http_server_send_websocket_binary_message(mla_http_server_t &server, const mla_string_t &connectionId,
                                                         const mla_bytes_t &message, mla_bool_t use_deflate_compression_if_supported) {
    mla_http_server_websocket_connection_t connection = mla_http_server_websocket_connection_invalid();

    if (!mla_http_server_find_websocket_connection(server, connectionId, connection)) {
        return false;
    }

    return mla_http_server_send_websocket_binary_message(connection, message, use_deflate_compression_if_supported);
}

mla_bool_t mla_http_server_try_send_websocket_binary_message(mla_http_server_websocket_connection_t& connection, mla_user_data_t &userData, mla_websocket_transport_message_generator_t message_generator, mla_int32_t connection_lock_timeout, mla_bool_t use_deflate_compression_if_supported) {

    if (!mla_http_server_is_websocket_connection_open(connection)) {
        return false;
    }

    if (connection_lock_timeout <= 0) {
        if (!mla_mutex_lock(connection.lock)) {
            return false;
        }
    } else {
        if (!mla_mutex_trylock(connection.lock, connection_lock_timeout)) {
            return false;
        }
    }

    mla_bool_t use_deflate_compression = connection.supports_deflate_compression && use_deflate_compression_if_supported;

    if (mla_websocket_transport_send_binary_with_generator(connection.connection.outputStream, userData, message_generator, false, use_deflate_compression)) {
        mla_bool_t flush_successful = mla_stream_output_flush_buffered_wrapper(connection.connection.outputStream);
        mla_mutex_unlock(connection.lock);
        return flush_successful;
    }
    mla_mutex_unlock(connection.lock);

    if (connection.server == nullptr) {
        return false;
    }

    mla_http_server_close_websocket_connection(connection, mla_websocket_close_abnormal, mla_string_const("Failed to send binary message"));
    return false;

}

mla_bool_t mla_http_server_try_send_websocket_binary_message(mla_http_server_websocket_connection_t& connection, const mla_bytes_t& message, mla_int32_t connection_lock_timeout, mla_bool_t use_deflate_compression_if_supported) {

    if (!mla_http_server_is_websocket_connection_open(connection)) {
        return false;
    }

    if (connection_lock_timeout <= 0) {
        if (!mla_mutex_lock(connection.lock)) {
            return false;
        }
    } else {
        if (!mla_mutex_trylock(connection.lock, connection_lock_timeout)) {
            return false;
        }
    }

    mla_bool_t use_deflate_compression = connection.supports_deflate_compression && use_deflate_compression_if_supported;

    if (mla_websocket_transport_send_binary_frame(connection.connection.outputStream, message, false, use_deflate_compression)) {
        mla_bool_t flush_successful = mla_stream_output_flush_buffered_wrapper(connection.connection.outputStream);
        mla_mutex_unlock(connection.lock);
        return flush_successful;
    }

    mla_mutex_unlock(connection.lock);

    if (connection.server == nullptr) {
        return false;
    }

    mla_http_server_close_websocket_connection(connection, mla_websocket_close_abnormal, mla_string_const("Failed to send binary message"));
    return false;

}

mla_bool_t mla_http_server_try_send_websocket_binary_message(mla_http_server_t &server, const mla_string_t& connectionId, const mla_bytes_t& message, mla_int32_t connection_lock_timeout, mla_bool_t use_deflate_compression_if_supported) {

    mla_http_server_websocket_connection_t connection = mla_http_server_websocket_connection_invalid();

    if (!mla_http_server_find_websocket_connection(server, connectionId, connection)) {
        return false;
    }

    return mla_http_server_try_send_websocket_binary_message(connection, message, connection_lock_timeout, use_deflate_compression_if_supported);

}

////////////////////////////////////////////////////////////////
/// Proxy Handler & Virtual Host Implementation
////////////////////////////////////////////////////////////////

mla_http_server_proxy_target_t mla_http_server_proxy_target(
    const mla_string_t &virtual_host_pattern,
    const mla_string_t &incoming_path_prefix,
    const mla_string_t &upstream_target_url,
    mla_int32_t timeout_ms,
    mla_bool_t preserve_host_header)
{
    mla_url_t url = mla_url_empty();
    mla_network_host_t host = mla_network_host_invalid();
    if (mla_url_parse(upstream_target_url, url) && !mla_string_is_empty(url.host)) {
        mla_network_host_resolve(host, url.host, url.port > 0 ? url.port : 80);
    }

    return {
        virtual_host_pattern,
        incoming_path_prefix,
        upstream_target_url,
        host,
        timeout_ms,
        preserve_host_header
    };
}

mla_http_server_proxy_target_t mla_http_server_proxy_target(
    const mla_string_t &incoming_path_prefix,
    const mla_string_t &upstream_target_url,
    mla_int32_t timeout_ms,
    mla_bool_t preserve_host_header)
{
    return mla_http_server_proxy_target(mla_string_empty(), incoming_path_prefix, upstream_target_url, timeout_ms, preserve_host_header);
}

mla_bool_t mla_private_http_server_match_host_pattern(
    const mla_string_t &host_header_value,
    const mla_string_t &host_pattern)
{
    if (mla_string_is_empty(host_pattern) || mla_string_equals(host_pattern, mla_string_const("*"))) {
        return true; // Match any host
    }

    if (mla_string_is_empty(host_header_value)) {
        return false;
    }

    // Strip port from Host header if present (e.g. "api.example.com:8080" -> "api.example.com")
    mla_string_t domain = host_header_value;
    mla_int32_t colon_idx = mla_string_index_of(host_header_value, mla_string_const(":"));
    if (colon_idx >= 0) {
        domain = mla_string_substr(host_header_value, 0, mla_s_cast<mla_size_t>(colon_idx));
    }

    // Wildcard subdomain matching (e.g. "*.example.com")
    if (mla_string_starts_with(host_pattern, mla_string_const("*."))) {
        mla_string_t suffix = mla_string_substr(host_pattern, 1); // ".example.com"
        return mla_string_ends_with_ignore_case(domain, suffix);
    }

    // Exact domain matching (case-insensitive)
    return mla_string_equals_ignore_case(domain, host_pattern);
}

struct mla_http_server_proxy_context_t {
    mla_http_server_proxy_target_t target;

    static mla_http_server_proxy_context_t init() {
        return {
            mla_http_server_proxy_target(mla_string_empty(), mla_string_empty(), mla_string_empty())
        };
    }
};

mla_user_data_id_init(mla_http_server_proxy_context_user_data_name)

static mla_bool_t mla_private_http_server_proxy_checker(
    const mla_user_data_t &userdata,
    const mla_http_request_t &request,
    mla_http_request_handler_checker_compare_mode_t compare_mode)
{
    mla_pointer_t ctx_ptr = mla_user_data_get_pointer(userdata, mla_http_server_proxy_context_user_data_name);
    mla_http_server_proxy_context_t *ctx = mla_pointer_get_data<mla_http_server_proxy_context_t>(ctx_ptr);

    if (ctx == nullptr) {
        return false;
    }

    // 1. Virtual Host pattern matching against Host header
    mla_string_t host_hdr = mla_http_headers_get_value(request.headers, mla_string_const("Host"));
    if (!mla_private_http_server_match_host_pattern(host_hdr, ctx->target.virtual_host_pattern)) {
        return false;
    }

    // 2. Incoming path prefix matching
    if (mla_string_is_empty(ctx->target.incoming_path_prefix)) {
        return true;
    }

    if (compare_mode == MLA_HTTP_REQUEST_HANDLER_CHECKER_COMPARE_MODE_PERFECT_MATCH) {
        return mla_string_starts_with(request.url, ctx->target.incoming_path_prefix);
    }

    if (compare_mode == MLA_HTTP_REQUEST_HANDLER_CHECKER_COMPARE_MODE_PREFIX) {
        return mla_string_starts_with(ctx->target.incoming_path_prefix, request.url);
    }

    return false;
}

static mla_bool_t mla_private_http_server_proxy_executor(
    mla_http_server_t &http_server,
    const mla_user_data_t &userdata,
    const mla_http_request_t &request,
    mla_http_response_t &response)
{
    mla_pointer_t ctx_ptr = mla_user_data_get_pointer(userdata, mla_http_server_proxy_context_user_data_name);
    mla_http_server_proxy_context_t *ctx = mla_pointer_get_data<mla_http_server_proxy_context_t>(ctx_ptr);

    if (ctx == nullptr) {
        response.statusCode = mla_http_status_internal_server_error;
        response.statusMessage = mla_string_const("Internal Server Error");
        return false;
    }

    // Path rewriting
    mla_string_t subpath = request.url;
    if (!mla_string_is_empty(ctx->target.incoming_path_prefix) && mla_string_starts_with(request.url, ctx->target.incoming_path_prefix)) {
        subpath = mla_string_substr(request.url, mla_string_length(ctx->target.incoming_path_prefix));
    }

    mla_string_t base_url = ctx->target.upstream_target_url;
    mla_string_t target_url = mla_string_empty();

    if (!mla_string_is_empty(base_url)) {
        if (mla_string_ends_with(base_url, mla_string_const("/")) && mla_string_starts_with(subpath, mla_string_const("/"))) {
            target_url = mla_string_concat(base_url, mla_string_substr(subpath, 1));
        } else if (!mla_string_ends_with(base_url, mla_string_const("/")) && !mla_string_starts_with(subpath, mla_string_const("/")) && !mla_string_is_empty(subpath)) {
            target_url = mla_string_concat(base_url, mla_string_const("/"), subpath);
        } else {
            target_url = mla_string_concat(base_url, subpath);
        }
    } else {
        mla_string_t host_str = mla_string_concat(
            mla_string_const("http://"),
            ctx->target.upstream_network_host.address.address,
            mla_string_const(":"),
            mla_string_from_uint16(ctx->target.upstream_network_host.port)
        );
        if (mla_string_starts_with(subpath, mla_string_const("/"))) {
            target_url = mla_string_concat(host_str, subpath);
        } else {
            target_url = mla_string_concat(host_str, mla_string_const("/"), subpath);
        }
    }

    // Build target HTTP client request
    mla_http_request_t target_req = mla_http_request(target_url, request.method);
    target_req.version = request.version;
    target_req.content = request.content; // Stream payload pipe

    // Filter hop-by-hop headers and copy rest
    for (mla_size_t i = 0; i < mla_array_list_size(request.headers); ++i) {
        const mla_http_header_t &hdr = mla_array_list_get_unsafe(request.headers, i);
        if (mla_string_equals_ignore_case(hdr.name, mla_string_const("Connection")) ||
            mla_string_equals_ignore_case(hdr.name, mla_string_const("Keep-Alive")) ||
            mla_string_equals_ignore_case(hdr.name, mla_string_const("Transfer-Encoding")) ||
            mla_string_equals_ignore_case(hdr.name, mla_string_const("Upgrade")) ||
            mla_string_equals_ignore_case(hdr.name, mla_string_const("Proxy-Authorization"))) {
            continue;
        }
        if (mla_string_equals_ignore_case(hdr.name, mla_string_const("Host")) && !ctx->target.preserve_host_header) {
            continue;
        }
        mla_http_headers_add(target_req.headers, hdr.name, mla_http_headers_get_value(request.headers, hdr.name));
    }

    if (!ctx->target.preserve_host_header) {
        mla_url_t parsed_target_url = mla_url_empty();
        mla_url_parse(target_url, parsed_target_url);
        mla_string_t host_val = parsed_target_url.host;
        if (parsed_target_url.port != 80 && parsed_target_url.port != 443 && parsed_target_url.port > 0) {
            host_val = mla_string_concat(host_val, mla_string_const(":"), mla_string_from_uint16(parsed_target_url.port));
        }
        mla_http_headers_add(target_req.headers, mla_string_const("Host"), host_val);
    }

    mla_http_headers_add(target_req.headers, mla_string_const("X-Forwarded-Proto"), mla_string_const("http"));

    mla_http_client_t client = mla_http_client();
    if (ctx->target.timeout_ms > 0) {
        mla_http_client_set_timeout(client, ctx->target.timeout_ms);
    } else {
        mla_http_client_set_timeout(client, http_server.timeout_ms);
    }

    mla_http_client_response_t client_resp = mla_http_client_send_request(client, target_req);
    if (client_resp.status != MLA_HTTP_CLIENT_RESPONSE_STATUS_OK) {
        response.statusCode = mla_http_status_bad_gateway;
        response.statusMessage = mla_string_const("Bad Gateway");
        return true;
    }

    response.statusCode = client_resp.response.statusCode;
    response.statusMessage = client_resp.response.statusMessage;

    for (mla_size_t i = 0; i < mla_array_list_size(client_resp.response.headers); ++i) {
        const mla_http_header_t &hdr = mla_array_list_get_unsafe(client_resp.response.headers, i);
        if (mla_string_equals_ignore_case(hdr.name, mla_string_const("Connection")) ||
            mla_string_equals_ignore_case(hdr.name, mla_string_const("Transfer-Encoding"))) {
            continue;
        }
        mla_http_headers_add(response.headers, hdr.name, mla_http_headers_get_value(client_resp.response.headers, hdr.name));
    }

    response.content = client_resp.response.content;
    return true;
}

mla_http_server_handler_item_t mla_http_server_proxy_handler_starts_with(
    const mla_string_t &virtual_host_pattern,
    const mla_string_t &incoming_path_prefix,
    const mla_string_t &upstream_target_url,
    const mla_string_t &http_method,
    const mla_user_data_t &userdata)
{
    mla_http_server_proxy_target_t target = mla_http_server_proxy_target(virtual_host_pattern, incoming_path_prefix, upstream_target_url);
    return mla_http_server_proxy_handler_starts_with(target, http_method, userdata);
}

mla_http_server_handler_item_t mla_http_server_proxy_handler_starts_with(
    const mla_string_t &incoming_path_prefix,
    const mla_string_t &upstream_target_url,
    const mla_string_t &http_method,
    const mla_user_data_t &userdata)
{
    return mla_http_server_proxy_handler_starts_with(mla_string_empty(), incoming_path_prefix, upstream_target_url, http_method, userdata);
}

mla_http_server_handler_item_t mla_http_server_proxy_handler_starts_with(
    const mla_string_t &virtual_host_pattern,
    const mla_string_t &incoming_path_prefix,
    const mla_network_host_t &upstream_network_host,
    const mla_string_t &target_base_path,
    const mla_string_t &http_method,
    const mla_user_data_t &userdata)
{
    mla_string_t target_url = mla_string_concat(
        mla_string_const("http://"),
        upstream_network_host.address.address,
        mla_string_const(":"),
        mla_string_from_uint16(upstream_network_host.port),
        target_base_path
    );
    return mla_http_server_proxy_handler_starts_with(virtual_host_pattern, incoming_path_prefix, target_url, http_method, userdata);
}

mla_http_server_handler_item_t mla_http_server_proxy_handler_starts_with(
    const mla_http_server_proxy_target_t &proxy_target,
    const mla_string_t &http_method,
    const mla_user_data_t &userdata)
{
    mla_pointer_t ctx_ptr = mla_malloc_struct(mla_http_server_proxy_context_t);
    mla_http_server_proxy_context_t *ctx = mla_pointer_get_data<mla_http_server_proxy_context_t>(ctx_ptr);
    if (ctx == nullptr) {
        return mla_http_server_handler_invalid();
    }
    ctx->target = proxy_target;
    mla_user_data_t ud_copy = userdata;
    mla_user_data_set_pointer(ud_copy, mla_http_server_proxy_context_user_data_name, ctx_ptr);

    return {
        ud_copy,
        mla_string_to_upper(http_method),
        mla_private_http_server_proxy_checker,
        mla_private_http_server_proxy_executor
    };
}

struct mla_websocket_proxy_context_t {
    mla_websocket_client_t client;
    mla_string_t receive_task_name; // Name for aborting task via mla_task_manager_abort_task

    static mla_websocket_proxy_context_t init() {
        return {
            mla_websocket_client_invalid(),
            mla_string_empty()
        };
    }
};

mla_user_data_id_init(mla_websocket_proxy_context_user_data_name)

struct mla_websocket_proxy_task_userdata_t {
    mla_http_server_websocket_connection_t connection;
    mla_websocket_client_t upstream_client;

    static mla_websocket_proxy_task_userdata_t init() {
        return {
            mla_http_server_websocket_connection_invalid(),
            mla_websocket_client_invalid()
        };
    }
};

static mla_task_process_result_state mla_private_websocket_proxy_forward_upstream(mla_user_data_t &userData) {
    mla_pointer_t task_ud_ptr = mla_user_data_get_pointer(userData, mla_http_task_user_data_server_name);
    mla_websocket_proxy_task_userdata_t *task_ud = mla_pointer_get_data<mla_websocket_proxy_task_userdata_t>(task_ud_ptr);
    if (task_ud == nullptr) {
        return TASK_PROCESS_RESULT_DONE;
    }

    if (!mla_http_server_is_websocket_connection_open(task_ud->connection) || !mla_websocket_client_is_connected(task_ud->upstream_client)) {
        return TASK_PROCESS_RESULT_DONE;
    }

    mla_websocket_text_message_t text_msg = mla_websocket_text_message_empty();
    mla_websocket_binary_message_t bin_msg = mla_websocket_binary_message_empty();

    mla_websocket_client_message_receive_type_t rx_type = mla_websocket_client_receive_message(task_ud->upstream_client, 20, text_msg, bin_msg);
    if (rx_type == MLA_WEBSOCKET_CLIENT_MESSAGE_RECEIVE_TYPE_TEXT) {
        mla_http_server_send_websocket_text_message(task_ud->connection, text_msg.message, false);
    } else if (rx_type == MLA_WEBSOCKET_CLIENT_MESSAGE_RECEIVE_TYPE_BINARY) {
        mla_http_server_send_websocket_binary_message(task_ud->connection, bin_msg.message, false);
    } else if (rx_type == MLA_WEBSOCKET_CLIENT_MESSAGE_RECEIVE_TYPE_CLOSED) {
        mla_http_server_close_websocket_connection(task_ud->connection, 1000, mla_string_const("Upstream closed"));
        return TASK_PROCESS_RESULT_DONE;
    }

    return TASK_PROCESS_RESULT_CONTINUE;
}

static mla_bool_t mla_private_websocket_proxy_open(mla_http_server_websocket_connection_t &connection) {
    mla_pointer_t ctx_ptr = mla_user_data_get_pointer(connection.userdata, mla_http_server_proxy_context_user_data_name);
    mla_http_server_proxy_context_t *proxy_ctx = mla_pointer_get_data<mla_http_server_proxy_context_t>(ctx_ptr);
    if (proxy_ctx == nullptr) {
        return false;
    }

    mla_string_t base_target_url = proxy_ctx->target.upstream_target_url;
    mla_string_t subpath = connection.endpoint;
    if (!mla_string_is_empty(proxy_ctx->target.incoming_path_prefix) && mla_string_starts_with(subpath, proxy_ctx->target.incoming_path_prefix)) {
        subpath = mla_string_substr(subpath, mla_string_length(proxy_ctx->target.incoming_path_prefix));
    }

    mla_string_t target_url = mla_string_empty();
    if (mla_string_is_empty(subpath)) {
        target_url = base_target_url;
    } else if (mla_string_ends_with(base_target_url, mla_string_const("/")) && mla_string_starts_with(subpath, mla_string_const("/"))) {
        target_url = mla_string_concat(base_target_url, mla_string_substr(subpath, 1));
    } else if (mla_string_ends_with(base_target_url, mla_string_const("/")) || mla_string_starts_with(subpath, mla_string_const("/"))) {
        target_url = mla_string_concat(base_target_url, subpath);
    } else {
        target_url = mla_string_concat(base_target_url, mla_string_const("/"), subpath);
    }

    mla_string_t ws_url = target_url;
    if (mla_string_starts_with(ws_url, mla_string_const("http://"))) {
        ws_url = mla_string_concat(mla_string_const("ws://"), mla_string_substr(ws_url, 7));
    } else if (mla_string_starts_with(ws_url, mla_string_const("https://"))) {
        ws_url = mla_string_concat(mla_string_const("wss://"), mla_string_substr(ws_url, 8));
    }

    mla_pointer_t ws_ctx_ptr = mla_malloc_struct(mla_websocket_proxy_context_t);
    mla_websocket_proxy_context_t *ws_ctx = mla_pointer_get_data<mla_websocket_proxy_context_t>(ws_ctx_ptr);
    if (ws_ctx == nullptr) {
        return false;
    }

    ws_ctx->client = mla_websocket_client_invalid();
    mla_int32_t connect_timeout_ms = proxy_ctx->target.timeout_ms > 0
        ? proxy_ctx->target.timeout_ms
        : connection.server->timeout_ms;
    if (!mla_websocket_client_connect(ws_ctx->client, ws_url, mla_s_cast<mla_size_t>(connect_timeout_ms), false)) {
        return false;
    }

    mla_pointer_t task_ud_ptr = mla_malloc_struct(mla_websocket_proxy_task_userdata_t);
    mla_websocket_proxy_task_userdata_t *task_ud = mla_pointer_get_data<mla_websocket_proxy_task_userdata_t>(task_ud_ptr);
    if (task_ud == nullptr) {
        mla_websocket_client_disconnect(ws_ctx->client);
        return false;
    }

    task_ud->connection = connection;
    task_ud->upstream_client = ws_ctx->client;

    mla_user_data_t task_ud_container = mla_user_data_empty();
    if (!mla_user_data_set_pointer(task_ud_container, mla_http_task_user_data_server_name, task_ud_ptr)) {
        mla_websocket_client_disconnect(ws_ctx->client);
        return false;
    }

    mla_user_data_set_pointer(connection.userdata, mla_websocket_proxy_context_user_data_name, ws_ctx_ptr);
    ws_ctx->receive_task_name = mla_string_concat(mla_string_const("WSProxyTask_"), connection.id);
    mla_task_t receive_task = mla_task_repeating(ws_ctx->receive_task_name, mla_private_websocket_proxy_forward_upstream, task_ud_container);
    if (!mla_task_manager_register_task(receive_task)) {
        mla_websocket_client_disconnect(ws_ctx->client);
        return false;
    }

    return true;
}

static mla_bool_t mla_private_websocket_proxy_text(mla_http_server_websocket_connection_t &connection, const mla_string_t &message) {
    mla_pointer_t ws_ctx_ptr = mla_user_data_get_pointer(connection.userdata, mla_websocket_proxy_context_user_data_name);
    mla_websocket_proxy_context_t *ws_ctx = mla_pointer_get_data<mla_websocket_proxy_context_t>(ws_ctx_ptr);
    if (ws_ctx == nullptr || !mla_websocket_client_is_connected(ws_ctx->client)) {
        return false;
    }
    return mla_websocket_client_send_text_message(ws_ctx->client, message);
}

static mla_bool_t mla_private_websocket_proxy_binary(mla_http_server_websocket_connection_t &connection, const mla_bytes_t &message) {
    mla_pointer_t ws_ctx_ptr = mla_user_data_get_pointer(connection.userdata, mla_websocket_proxy_context_user_data_name);
    mla_websocket_proxy_context_t *ws_ctx = mla_pointer_get_data<mla_websocket_proxy_context_t>(ws_ctx_ptr);
    if (ws_ctx == nullptr || !mla_websocket_client_is_connected(ws_ctx->client)) {
        return false;
    }
    return mla_websocket_client_send_binary_message(ws_ctx->client, message);
}

static void mla_private_websocket_proxy_closed(const mla_http_server_websocket_connection_t &connection) {
    mla_pointer_t ws_ctx_ptr = mla_user_data_get_pointer(connection.userdata, mla_websocket_proxy_context_user_data_name);
    mla_websocket_proxy_context_t *ws_ctx = mla_pointer_get_data<mla_websocket_proxy_context_t>(ws_ctx_ptr);
    if (ws_ctx != nullptr) {
        if (mla_websocket_client_is_connected(ws_ctx->client)) {
            mla_websocket_client_disconnect(ws_ctx->client);
        }
        if (!mla_string_is_empty(ws_ctx->receive_task_name)) {
            mla_task_manager_abort_task(ws_ctx->receive_task_name);
        }
    }
}

mla_http_server_websocket_handler_item_t mla_http_server_websocket_proxy_handler_starts_with(
    const mla_string_t &virtual_host_pattern,
    const mla_string_t &incoming_path_prefix,
    const mla_string_t &upstream_target_url,
    const mla_user_data_t &userdata)
{
    mla_http_server_proxy_target_t target = mla_http_server_proxy_target(virtual_host_pattern, incoming_path_prefix, upstream_target_url);
    mla_pointer_t ctx_ptr = mla_malloc_struct(mla_http_server_proxy_context_t);
    mla_http_server_proxy_context_t *ctx = mla_pointer_get_data<mla_http_server_proxy_context_t>(ctx_ptr);
    if (ctx == nullptr) {
        return mla_http_server_websocket_handler_invalid();
    }
    ctx->target = target;
    mla_user_data_t ud_copy = userdata;
    mla_user_data_set_pointer(ud_copy, mla_http_server_proxy_context_user_data_name, ctx_ptr);

    mla_http_server_websocket_handler_item_t item = {
        ud_copy,
        mla_private_http_server_proxy_checker,
        mla_private_websocket_proxy_open,
        mla_private_websocket_proxy_text,
        mla_private_websocket_proxy_binary,
        mla_private_websocket_proxy_closed
    };
    return item;
}

mla_http_server_websocket_handler_item_t mla_http_server_websocket_proxy_handler_starts_with(
    const mla_string_t &incoming_path_prefix,
    const mla_string_t &upstream_target_url,
    const mla_user_data_t &userdata)
{
    return mla_http_server_websocket_proxy_handler_starts_with(mla_string_empty(), incoming_path_prefix, upstream_target_url, userdata);
}

mla_bool_t mla_http_server_register_proxy(
    mla_http_server_t &server,
    const mla_string_t &virtual_host_pattern,
    const mla_string_t &incoming_path_prefix,
    const mla_string_t &upstream_target_url)
{
    mla_http_server_proxy_target_t target = mla_http_server_proxy_target(virtual_host_pattern, incoming_path_prefix, upstream_target_url);
    return mla_http_server_register_proxy(server, target);
}

mla_bool_t mla_http_server_register_proxy(
    mla_http_server_t &server,
    const mla_string_t &incoming_path_prefix,
    const mla_string_t &upstream_target_url)
{
    return mla_http_server_register_proxy(server, mla_string_empty(), incoming_path_prefix, upstream_target_url);
}

mla_bool_t mla_http_server_register_proxy(
    mla_http_server_t &server,
    const mla_http_server_proxy_target_t &proxy_target)
{
    mla_user_data_t http_ud = mla_user_data_empty();
    mla_http_server_handler_item_t http_item = mla_http_server_proxy_handler_starts_with(proxy_target, mla_string_empty(), http_ud);
    if (!mla_http_server_register_handler(server, http_item)) {
        return false;
    }

    mla_user_data_t ws_ud = mla_user_data_empty();
    mla_http_server_websocket_handler_item_t ws_item = mla_http_server_websocket_proxy_handler_starts_with(proxy_target.virtual_host_pattern, proxy_target.incoming_path_prefix, proxy_target.upstream_target_url, ws_ud);
    return mla_http_server_register_websocket_handler(server, ws_item);
}
