---
name: coreos-http-server
description: Patterns for creating HTTP servers and clients in the MLA framework. Use when setting up web servers, registering request handlers, working with WebSockets, or making HTTP client requests.
metadata:
  author: coreos
  version: "1.0"
---

# HTTP Server and Client

The HTTP module (`core-os/http/`) provides a full HTTP server and client implementation with WebSocket support. Use it to serve APIs, host web applications, handle WebSocket connections, and make outgoing HTTP requests.

## Key Types

| Type | Header | Purpose |
|---|---|---|
| `mla_http_server_t` | `mla_http_server.h` | HTTP server with handler registration |
| `mla_http_request_t` | `mla_http_request.h` | Incoming or outgoing HTTP request |
| `mla_http_response_t` | `mla_http_response.h` | HTTP response with status and body |
| `mla_http_header_t` | `mla_http_header.h` | HTTP header name-value pair |
| `mla_http_client_t` | `mla_http_client.h` | HTTP client for outgoing requests |
| `mla_http_server_websocket_connection_t` | `mla_http_server.h` | WebSocket connection state |
| `mla_websocket_client_t` | `mla_websocket_client.h` | WebSocket client |

## Creating an HTTP Server

```cpp
#include "../http/mla_http_server.h"
#include "../network/mla_network.h"

// Create a server on port 8081
mla_http_server_t server = mla_http_server(
    mla_network_host_ip4(mla_string_const("0.0.0.0"), 8081)
);

// Start with 2 worker tasks
mla_http_server_start(server, 2, TASK_STACK_SIZE_LARGE);
```

## Registering Request Handlers

### Handler by Path Prefix

```cpp
static mla_bool_t api_handler(
    mla_http_server_t& p_Server,
    const mla_http_request_t& p_Request,
    mla_http_response_t& p_Response)
{
    p_Response.statusCode = 200;
    p_Response.statusMessage = mla_string_const("OK");

    // Set response body from a string
    mla_string_t body = mla_string_const("{\"status\":\"ok\"}");
    p_Response.content = mla_stream_input_from_string(body);

    // Add content type header
    mla_http_header_t header = mla_http_header(
        mla_string_const("Content-Type"),
        mla_string_const("application/json")
    );
    mla_array_list_add(p_Response.headers, header);

    return true;
}

// Register: matches GET /api/...
auto handler = mla_http_server_handler_starts_with(
    mla_string_const("GET"),
    mla_string_const("/api/"),
    api_handler
);
mla_http_server_register_handler(server, handler);
```

### Handler That Matches All Requests

```cpp
auto handler = mla_http_server_handler_all(
    mla_string_const("GET"),
    fallback_handler
);
mla_http_server_register_handler(server, handler);
```

## Reading Request Data

```cpp
static mla_bool_t my_handler(
    mla_http_server_t& p_Server,
    const mla_http_request_t& p_Request,
    mla_http_response_t& p_Response)
{
    // Access the URL
    mla_string_t url = p_Request.url;

    // Access the HTTP method
    mla_string_t method = p_Request.method;

    // Read a header value
    mla_string_t contentType = mla_http_header_get_value(
        p_Request.headers,
        mla_string_const("Content-Type")
    );

    // Read request body
    mla_byte_t buffer[4096];
    mla_size_t bytesRead = p_Request.content.read(
        p_Request.content, 0, 4096, buffer
    );

    return true;
}
```

## WebSocket Server

### Registering a WebSocket Handler

```cpp
static mla_bool_t ws_open(mla_http_server_websocket_connection_t& p_Connection) {
    mla_info("WebSocket connected");
    return true;
}

static mla_bool_t ws_text_message(
    mla_http_server_websocket_connection_t& p_Connection,
    const mla_string_t& p_Message,
    mla_bool_t p_IsFinalFragment)
{
    // Echo message back
    mla_http_server_send_websocket_text_message(
        p_Connection, p_Message, true
    );
    return true;
}

static mla_bool_t ws_closed(mla_http_server_websocket_connection_t& p_Connection) {
    mla_info("WebSocket disconnected");
    return true;
}

// Register WebSocket endpoint at /ws
mla_http_server_websocket_handler_item_t wsHandler = {};
wsHandler.endpoint = mla_string_const("/ws");
wsHandler.open_executor = ws_open;
wsHandler.text_executor = ws_text_message;
wsHandler.closed_executor = ws_closed;

mla_http_server_register_websocket_handler(server, wsHandler);
```

### Broadcasting to WebSocket Connections

```cpp
// Find all connections on a path prefix
auto connections = mla_http_server_find_websocket_connections(
    server, mla_string_const("/ws")
);

for (mla_size_t i = 0; i < mla_array_list_size(connections); i++) {
    auto& conn = mla_array_list_get(connections, i);
    mla_http_server_send_websocket_text_message(
        conn, mla_string_const("broadcast message"), true
    );
}
```

## HTTP Client

### Sending a GET Request

```cpp
#include "../http/mla_http_client.h"
#include "../http/mla_http_request.h"

mla_http_request_t request = mla_http_get_request(
    mla_string_const("http://example.com/api/data")
);

mla_http_client_response_t response = mla_http_client_send_request(request);

if (response.status == MLA_HTTP_CLIENT_RESPONSE_STATUS_OK) {
    // Process response.response.content
    mla_uint16_t statusCode = response.response.statusCode;
} else {
    mla_error("HTTP request failed");
}
```

### Handling Client Errors

```cpp
mla_http_client_response_t response = mla_http_client_send_request(request);

switch (response.status) {
    case MLA_HTTP_CLIENT_RESPONSE_STATUS_OK:
        // success
        break;
    case MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_UNKNOWN_HOST:
        mla_error("DNS resolution failed");
        break;
    case MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_CONNECTION_FAILED:
        mla_error("Connection refused");
        break;
    case MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_TIMEOUT:
        mla_error("Request timed out");
        break;
    default:
        mla_error("Unknown error");
        break;
}
```

## WebSocket Client

```cpp
#include "../http/mla_websocket_client.h"

mla_websocket_client_t client = mla_websocket_client_invalid();

// Connect
if (mla_websocket_client_connect(client, mla_string_const("ws://localhost:8081/ws"), 5000)) {
    // Send a text message
    mla_websocket_client_send_text_message(client, mla_string_const("hello"), true);

    // Receive messages
    mla_websocket_text_message_t textMsg;
    mla_websocket_binary_message_t binMsg;
    auto type = mla_websocket_client_receive_message(client, 5000, textMsg, binMsg);

    if (type == MLA_WEBSOCKET_CLIENT_MESSAGE_RECEIVE_TYPE_TEXT) {
        // Process textMsg.message
    }

    // Disconnect
    mla_websocket_client_disconnect(client);
}
```

## Server Lifecycle

```cpp
// Check server status
if (mla_http_server_running(server)) {
    mla_info("Server is running");
}

// Stop server
mla_http_server_stop(server);
```

## Rules

- Handler callbacks must return `true` on success, `false` on failure.
- Always set `statusCode` and `statusMessage` on the response.
- WebSocket message sending is thread-safe (uses internal mutex).
- Use `mla_http_server_try_send_websocket_text_message` with a timeout for non-blocking sends.
- Close WebSocket connections with `mla_http_server_close_websocket_connection` and a close code.
- The server must be started with `mla_http_server_start` before it accepts connections.
- Use `TASK_STACK_SIZE_LARGE` or higher for HTTP server tasks due to buffer requirements.
