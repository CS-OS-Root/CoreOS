//
// Created by chris on 10/23/2025.
//

#ifndef MLA_HTTP_SERVER_H
#define MLA_HTTP_SERVER_H

#include "mla_http_request.h"
#include "mla_http_response.h"
#include "../network/mla_network.h"
#include "../task/mla_mutx.h"
#include "../task/mla_rw_lock.h"
#include "mla_websocket_utils.h"

struct mla_http_server_t;

enum mla_http_server_status_t: mla_uint8_t {
    MLA_HTTP_SERVER_STATUS_STOPPED,
    MLA_HTTP_SERVER_STATUS_RUNNING,
    MLA_HTTP_SERVER_STATUS_ERROR
};

enum mla_http_request_handler_checker_compare_mode_t: mla_uint8_t {
    MLA_HTTP_REQUEST_HANDLER_CHECKER_COMPARE_MODE_PERFECT_MATCH, // Only will return ture if the handle will serve the URL
    MLA_HTTP_REQUEST_HANDLER_CHECKER_COMPARE_MODE_PREFIX, // Will return true if the handler can potentially serve the URL. For example, a handler with path prefix "/api/" can potentially serve the URL "/api/users/1" but not "/static/index.html"
};

typedef mla_bool_t (*mla_http_request_handler_checker_t)(const mla_user_data_t &userdata, const mla_http_request_t& request, mla_http_request_handler_checker_compare_mode_t compare_mode);

///////////////////////////////////
/// HTTP Server Handler Item
//////////////////////////////////

typedef mla_bool_t (*mla_http_request_handler_t)(mla_http_server_t& http_server, const mla_user_data_t &userdata, const mla_http_request_t &request, mla_http_response_t &response);

struct mla_http_server_handler_item_t {
    mla_user_data_t userdata;
    mla_string_t method;
    mla_http_request_handler_checker_t checker;
    mla_http_request_handler_t executor;

    static mla_http_server_handler_item_t init() {
        return {
            mla_user_data_empty(),
            mla_string_empty(),
            nullptr,
            nullptr
        };
    }
};

mla_http_server_handler_item_t mla_http_server_handler_invalid();
mla_http_server_handler_item_t mla_http_server_handler(const mla_string_t &http_method, mla_user_data_t& userdata, const mla_http_request_handler_checker_t& checker, const mla_http_request_handler_t& executor);
mla_http_server_handler_item_t mla_http_server_handler_starts_with(const mla_string_t &http_method, mla_user_data_t& userdata, const mla_string_t& pathPrefix, const mla_http_request_handler_t& executor);
mla_http_server_handler_item_t mla_http_server_handler_all(const mla_string_t &http_method, mla_user_data_t& userdata, const mla_http_request_handler_t& executor);

mla_user_data_id_init(mla_http_server_handler_struct_user_data_id)

template<typename T>
mla_bool_t mla_http_request_handler_checker_execute(const mla_user_data_t &userdata, const mla_http_request_t& request, mla_http_request_handler_checker_compare_mode_t compare_mode) {

    T* handler_struct = mla_user_data_get_struct_data<T>(userdata, mla_http_server_handler_struct_user_data_id);

    if (handler_struct == nullptr) {
        return false;
    }

    return T::http_request_check(*handler_struct, userdata, request.url, compare_mode);
}

template<typename T>
mla_bool_t mla_http_server_handler_struct_handle_execute(mla_http_server_t& http_server, const mla_user_data_t &userdata, const mla_http_request_t &request, mla_http_response_t &response) {

    T* handler_struct = mla_user_data_get_struct_data<T>(userdata, mla_http_server_handler_struct_user_data_id);

    if (handler_struct == nullptr) {
        return false;
    }

    return T::http_request_handle(*handler_struct, http_server, userdata, request, response);
}

template<typename T>
mla_http_server_handler_item_t mla_http_server_handler_struct(const mla_string_t &http_method, mla_user_data_t& userdata, T& handler_struct) {

    if (!mla_user_data_set_struct(userdata, mla_http_server_handler_struct_user_data_id, handler_struct)) {
        return mla_http_server_handler_invalid();
    }

    return mla_http_server_handler(http_method, userdata, mla_http_request_handler_checker_execute<T>, mla_http_server_handler_struct_handle_execute<T>);
}

template<typename T>
mla_http_server_handler_item_t mla_http_server_handler_struct_starts_with(const mla_string_t &http_method, mla_user_data_t& userdata, const mla_string_t& pathPrefix, T& handler_struct) {

    if (!mla_user_data_set_struct(userdata, mla_http_server_handler_struct_user_data_id, handler_struct)) {
        return mla_http_server_handler_invalid();
    }

    return mla_http_server_handler_starts_with(http_method, userdata, pathPrefix, mla_http_server_handler_struct_handle_execute<T>);
}

template<typename T>
mla_http_server_handler_item_t mla_http_server_handler_struct_all(const mla_string_t &http_method, mla_user_data_t& userdata, T& handler_struct) {

    if (!mla_user_data_set_struct(userdata, mla_http_server_handler_struct_user_data_id, handler_struct)) {
        return mla_http_server_handler_invalid();
    }

    return mla_http_server_handler_all(http_method, userdata, mla_http_server_handler_struct_handle_execute<T>);
}




///////////////////////////////////
/// HTTP Server WebSocket Handler Item
//////////////////////////////////

struct mla_http_server_t;
struct mla_http_server_websocket_connection_t;

typedef mla_bool_t (*mla_http_websocket_open_handler_t)(mla_http_server_websocket_connection_t& connection);
typedef mla_bool_t (*mla_http_websocket_text_message_handler_t)(mla_http_server_websocket_connection_t& connection, const mla_string_t& message);
typedef mla_bool_t (*mla_http_websocket_binary_message_handler_t)(mla_http_server_websocket_connection_t& connection, const mla_bytes_t& message);
typedef void (*mla_http_websocket_closed_handler_t)(const mla_http_server_websocket_connection_t& connection);

struct mla_http_server_websocket_connection_t {
    mla_http_server_t* server;
    mla_network_connection_t connection;
    mla_string_t id;
    mla_string_t endpoint;
    mla_mutex_t lock;
    mla_user_data_t userdata;
    mla_bool_t supports_deflate_compression;
    mla_http_websocket_open_handler_t open_executor;
    mla_http_websocket_text_message_handler_t text_executor;
    mla_http_websocket_binary_message_handler_t binary_executor;
    mla_http_websocket_closed_handler_t closed_executor;

    static mla_http_server_websocket_connection_t init() {
        return { nullptr, mla_network_connection_t::init(), mla_string_empty(), mla_string_empty(), mla_mutex_t::init(), mla_user_data_empty(), false, nullptr, nullptr, nullptr, nullptr };
    }
};

mla_http_server_websocket_connection_t mla_http_server_websocket_connection_invalid();



struct mla_http_server_websocket_handler_item_t {
    mla_user_data_t userdata;
    mla_http_request_handler_checker_t checker;
    mla_http_websocket_open_handler_t open_executor;
    mla_http_websocket_text_message_handler_t text_executor;
    mla_http_websocket_binary_message_handler_t binary_executor;
    mla_http_websocket_closed_handler_t close_executor;

    static mla_http_server_websocket_handler_item_t init() {
        return {
            mla_user_data_empty(),
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        };
    }
};

mla_http_server_websocket_handler_item_t mla_http_server_websocket_handler(mla_user_data_t& userdata, const mla_http_request_handler_checker_t& checker, const mla_http_websocket_text_message_handler_t& text_message_handler, const mla_http_websocket_binary_message_handler_t& binary_message_handler);
mla_http_server_websocket_handler_item_t mla_http_server_websocket_handler_path_equals(const mla_string_t& path, const mla_http_websocket_text_message_handler_t& text_message_handler, const mla_http_websocket_binary_message_handler_t& binary_message_handler);
mla_http_server_websocket_handler_item_t mla_http_server_websocket_handler_invalid();



////////////////////////////////////
/// HTTP Server
///////////////////////////////////

struct mla_http_server_t {
    mla_array_list_t<mla_init_struct(mla_http_server_handler_item_t)> httpHandlers;
    mla_array_list_t<mla_init_struct(mla_http_server_websocket_handler_item_t)> websocketHandlers;
    mla_array_list_t<mla_init_struct(mla_http_server_websocket_connection_t)> websocketConnections;
    mla_rw_lock_t websocketConnectionsLock;
    mla_network_host_t host;
    mla_network_listener_t listener;
    mla_mutex_t listenerLock;
    mla_pointer_t serverOwner;
    mla_http_server_status_t status;
    mla_int32_t timeout_ms;
    mla_network_security_config_t security_config;
};

// Lifecycle Methods
mla_http_server_t mla_http_server_invalid();
mla_http_server_t mla_http_server(const mla_network_host_t &host);
mla_bool_t mla_http_server_running(const mla_http_server_t &server);
mla_bool_t mla_http_server_start(mla_http_server_t &server, mla_uint8_t number_of_tasks, mla_task_stack_size task_stack_size = TASK_STACK_SIZE_DEFAULT);
mla_bool_t mla_http_server_stop(mla_http_server_t &server);

void mla_http_server_set_timeout(mla_http_server_t &server, mla_int32_t timeout_ms);
void mla_http_server_set_security_config(mla_http_server_t &server, const mla_network_security_config_t &security_config);
void mla_http_server_enable_tls(mla_http_server_t &server, const mla_network_tls_config_t &tls_config);
void mla_http_server_disable_security(mla_http_server_t &server);

// Registration Methods
mla_bool_t mla_http_server_register_handler(mla_http_server_t &server, const mla_http_server_handler_item_t& handlerItem);
mla_bool_t mla_http_server_register_websocket_handler(mla_http_server_t &server, const mla_http_server_websocket_handler_item_t& handlerItem);

// WebSocket Methods
mla_bool_t mla_http_server_find_websocket_connection(mla_http_server_t &server, const mla_string_t& connectionId, mla_http_server_websocket_connection_t& outConnection);
mla_array_list_t<mla_init_struct(mla_http_server_websocket_connection_t)> mla_http_server_find_websocket_connections(mla_http_server_t &server, const mla_string_t &path_prefix);
mla_size_t mla_http_server_get_websocket_connection_count(mla_http_server_t &server, const mla_string_t &path_prefix);
mla_bool_t mla_http_server_is_websocket_connection_open(const mla_http_server_websocket_connection_t& connection);
mla_array_list_t<mla_init_struct(mla_http_server_websocket_handler_item_t)> mla_http_server_get_websocket_handler_for_path(mla_http_server_t &server, const mla_string_t& path);

mla_bool_t mla_http_server_close_websocket_connection(mla_http_server_websocket_connection_t& connection, mla_uint16_t closeCode, const mla_string_t& reason);
mla_bool_t mla_http_server_close_websocket_connection(mla_http_server_t &server, const mla_string_t& connectionId, mla_uint16_t closeCode, const mla_string_t& reason);

// Sends a text message over a websocket connection using a generator callback.
//
// This function blocks until the connection lock can be acquired.
//
// NOTE: The message_generator callback may be invoked multiple times for a single message send
// (e.g., once to calculate the message length and again to write the actual content).
// The generator MUST produce deterministic output for a given call to ensure data consistency.
//
// connection: The websocket connection to send to.
// userData: User data passed to the generator callback.
// message_generator: Callback function that generates the message content.
// Returns true if the message was sent successfully, false otherwise.
mla_bool_t mla_http_server_send_websocket_text_message(mla_http_server_websocket_connection_t& connection, mla_user_data_t &userData, mla_websocket_transport_message_generator_t message_generator, mla_bool_t use_deflate_compression_if_supported = true);

// Sends a text message over a websocket connection.
//
// This function blocks until the connection lock can be acquired.
//
// connection: The websocket connection to send to.
// message: The text message to send.
// is_final: Whether this is the final fragment of the message.
// Returns true if the message was sent successfully, false otherwise.
mla_bool_t mla_http_server_send_websocket_text_message(mla_http_server_websocket_connection_t& connection, const mla_string_t& message, mla_bool_t use_deflate_compression_if_supported = true);

// Sends a text message to a specific connection by ID.
//
// This function looks up the connection by ID and then sends the message.
// It blocks until the connection lock can be acquired.
//
// server: The HTTP server instance.
// connectionId: The ID of the target connection.
// message: The text message to send.
// is_final: Whether this is the final fragment of the message.
// Returns true if the message was sent successfully, false otherwise.
mla_bool_t mla_http_server_send_websocket_text_message(mla_http_server_t &server, const mla_string_t& connectionId, const mla_string_t& message, mla_bool_t use_deflate_compression_if_supported = true);

// Attempts to send a text message over a websocket connection using a generator callback, with a timeout for acquiring the lock.
//
// NOTE: The message_generator callback may be invoked multiple times for a single message send
// (e.g., once to calculate the message length and again to write the actual content).
// The generator MUST produce deterministic output for a given call to ensure data consistency.
//
// connection: The websocket connection to send to.
// userData: User data passed to the generator callback.
// message_generator: Callback function that generates the message content.
// connection_lock_timeout: Maximum time in milliseconds to wait for the connection lock.
// Returns true if the message was sent successfully, false if the lock timed out or sending failed.
mla_bool_t mla_http_server_try_send_websocket_text_message(mla_http_server_websocket_connection_t& connection, mla_user_data_t &userData, mla_websocket_transport_message_generator_t message_generator, mla_int32_t connection_lock_timeout, mla_bool_t use_deflate_compression_if_supported = true);

// Attempts to send a text message over a websocket connection, with a timeout for acquiring the lock.
//
// connection: The websocket connection to send to.
// message: The text message to send.
// connection_lock_timeout: Maximum time in milliseconds to wait for the connection lock.
// Returns true if the message was sent successfully, false if the lock timed out or sending failed.
mla_bool_t mla_http_server_try_send_websocket_text_message(mla_http_server_websocket_connection_t& connection, const mla_string_t& message, mla_int32_t connection_lock_timeout, mla_bool_t use_deflate_compression_if_supported = true);

// Attempts to send a text message to a specific connection by ID, with a timeout for acquiring the lock.
//
// server: The HTTP server instance.
// connectionId: The ID of the target connection.
// message: The text message to send.
// connection_lock_timeout: Maximum time in milliseconds to wait for the connection lock.
// Returns true if the message was sent successfully, false if the lock timed out or sending failed.
mla_bool_t mla_http_server_try_send_websocket_text_message(mla_http_server_t &server, const mla_string_t& connectionId, const mla_string_t& message, mla_int32_t connection_lock_timeout, mla_bool_t use_deflate_compression_if_supported = true);

// Sends a binary message over a websocket connection using a generator callback.
//
// This function blocks until the connection lock can be acquired.
//
// connection: The websocket connection to send to.
// userData: User data passed to the generator callback.
// message_generator: Callback function that generates the message content.
// Returns true if the message was sent successfully, false otherwise.
mla_bool_t mla_http_server_send_websocket_binary_message(mla_http_server_websocket_connection_t& connection, mla_user_data_t &userData, mla_websocket_transport_message_generator_t message_generator, mla_bool_t use_deflate_compression_if_supported = true);

// Sends a binary message over a websocket connection.
//
// This function blocks until the connection lock can be acquired.
//
// connection: The websocket connection to send to.
// message: The binary message to send.
// Returns true if the message was sent successfully, false otherwise.
mla_bool_t mla_http_server_send_websocket_binary_message(mla_http_server_websocket_connection_t& connection, const mla_bytes_t& message, mla_bool_t use_deflate_compression_if_supported = true);

// Sends a binary message to a specific connection by ID.
//
// This function looks up the connection by ID and then sends the message.
// It blocks until the connection lock can be acquired.
//
// server: The HTTP server instance.
// connectionId: The ID of the target connection.
// message: The binary message to send.
// Returns true if the message was sent successfully, false otherwise.
mla_bool_t mla_http_server_send_websocket_binary_message(mla_http_server_t &server, const mla_string_t& connectionId, const mla_bytes_t& message, mla_bool_t use_deflate_compression_if_supported = true);

// Attempts to send a binary message over a websocket connection using a generator callback, with a timeout for acquiring the lock.
//
// connection: The websocket connection to send to.
// userData: User data passed to the generator callback.
// message_generator: Callback function that generates the message content.
// connection_lock_timeout: Maximum time in milliseconds to wait for the connection lock.
// Returns true if the message was sent successfully, false if the lock timed out or sending failed.
mla_bool_t mla_http_server_try_send_websocket_binary_message(mla_http_server_websocket_connection_t& connection, mla_user_data_t &userData, mla_websocket_transport_message_generator_t message_generator, mla_int32_t connection_lock_timeout, mla_bool_t use_deflate_compression_if_supported = true);

// Attempts to send a binary message over a websocket connection, with a timeout for acquiring the lock.
//
// connection: The websocket connection to send to.
// message: The binary message to send.
// is_final: Whether this is the final fragment of the message.
// connection_lock_timeout: Maximum time in milliseconds to wait for the connection lock.
// Returns true if the message was sent successfully, false if the lock timed out or sending failed.
mla_bool_t mla_http_server_try_send_websocket_binary_message(mla_http_server_websocket_connection_t& connection, const mla_bytes_t& message, mla_int32_t connection_lock_timeout, mla_bool_t use_deflate_compression_if_supported = true);

// Attempts to send a binary message to a specific connection by ID, with a timeout for acquiring the lock.
//
// server: The HTTP server instance.
// connectionId: The ID of the target connection.
// message: The binary message to send.
// is_final: Whether this is the final fragment of the message.
// connection_lock_timeout: Maximum time in milliseconds to wait for the connection lock.
// Returns true if the message was sent successfully, false if the lock timed out or sending failed.
mla_bool_t mla_http_server_try_send_websocket_binary_message(mla_http_server_t &server, const mla_string_t& connectionId, const mla_bytes_t& message, mla_int32_t connection_lock_timeout, mla_bool_t use_deflate_compression_if_supported = true);

////////////////////////////////////////////////////////////////
/// Proxy Handlers & Virtual Host Support
////////////////////////////////////////////////////////////////

struct mla_http_server_proxy_target_t {
    mla_string_t virtual_host_pattern;
    mla_string_t incoming_path_prefix;
    mla_string_t upstream_target_url;
    mla_network_host_t upstream_network_host;
    mla_int32_t timeout_ms;
    mla_bool_t preserve_host_header;
};

mla_http_server_proxy_target_t mla_http_server_proxy_target(
    const mla_string_t &virtual_host_pattern,
    const mla_string_t &incoming_path_prefix,
    const mla_string_t &upstream_target_url,
    mla_int32_t timeout_ms = 0,
    mla_bool_t preserve_host_header = false
);

mla_http_server_proxy_target_t mla_http_server_proxy_target(
    const mla_string_t &incoming_path_prefix,
    const mla_string_t &upstream_target_url,
    mla_int32_t timeout_ms = 0,
    mla_bool_t preserve_host_header = false
);

mla_http_server_handler_item_t mla_http_server_proxy_handler_starts_with(
    const mla_string_t &virtual_host_pattern,
    const mla_string_t &incoming_path_prefix,
    const mla_string_t &upstream_target_url,
    const mla_string_t &http_method = mla_string_empty(),
    const mla_user_data_t &userdata = mla_user_data_empty()
);

mla_http_server_handler_item_t mla_http_server_proxy_handler_starts_with(
    const mla_string_t &incoming_path_prefix,
    const mla_string_t &upstream_target_url,
    const mla_string_t &http_method = mla_string_empty(),
    const mla_user_data_t &userdata = mla_user_data_empty()
);

mla_http_server_handler_item_t mla_http_server_proxy_handler_starts_with(
    const mla_string_t &virtual_host_pattern,
    const mla_string_t &incoming_path_prefix,
    const mla_network_host_t &upstream_network_host,
    const mla_string_t &target_base_path = mla_string_empty(),
    const mla_string_t &http_method = mla_string_empty(),
    const mla_user_data_t &userdata = mla_user_data_empty()
);

mla_http_server_handler_item_t mla_http_server_proxy_handler_starts_with(
    const mla_http_server_proxy_target_t &proxy_target,
    const mla_string_t &http_method = mla_string_empty(),
    const mla_user_data_t &userdata = mla_user_data_empty()
);

mla_http_server_websocket_handler_item_t mla_http_server_websocket_proxy_handler_starts_with(
    const mla_string_t &virtual_host_pattern,
    const mla_string_t &incoming_path_prefix,
    const mla_string_t &upstream_target_url,
    const mla_user_data_t &userdata = mla_user_data_empty()
);

mla_http_server_websocket_handler_item_t mla_http_server_websocket_proxy_handler_starts_with(
    const mla_string_t &incoming_path_prefix,
    const mla_string_t &upstream_target_url,
    const mla_user_data_t &userdata = mla_user_data_empty()
);

mla_bool_t mla_http_server_register_proxy(
    mla_http_server_t &server,
    const mla_string_t &virtual_host_pattern,
    const mla_string_t &incoming_path_prefix,
    const mla_string_t &upstream_target_url
);

mla_bool_t mla_http_server_register_proxy(
    mla_http_server_t &server,
    const mla_string_t &incoming_path_prefix,
    const mla_string_t &upstream_target_url
);

mla_bool_t mla_http_server_register_proxy(
    mla_http_server_t &server,
    const mla_http_server_proxy_target_t &proxy_target
);

#endif
