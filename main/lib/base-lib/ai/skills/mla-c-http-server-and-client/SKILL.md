---
name: 'mla-c-http-server-and-client'
description: 'Patterns for creating HTTP servers and making HTTP client requests in the MLA framework'
---

# HTTP Server and Client

The HTTP module (`core/http/`) provides a lightweight HTTP server and client. Requests and responses use `mla_stream_input_t` / `mla_stream_output_t` for body content and `mla_array_list_t` of `mla_http_header_t` for headers.

## Key Types

| Type | Header | Purpose |
|---|---|---|
| `mla_http_server_t` | `mla_http_server.h` | HTTP server with request handler registry |
| `mla_http_server_handler_item_t` | `mla_http_server.h` | A registered request handler (method + URL matcher + executor) |
| `mla_http_client_t` | `mla_http_client.h` | HTTP client with timeout settings |
| `mla_http_client_response_t` | `mla_http_client.h` | Client response wrapper with status/error info |
| `mla_http_request_t` | `mla_http_request.h` | HTTP request (method, URL, headers, body) |
| `mla_http_response_t` | `mla_http_response.h` | HTTP response (status code, headers, body) |
| `mla_http_header_t` | `mla_http_header.h` | A single HTTP header (name + values) |

## HTTP Method Constants

```cpp
mla_http_method_get     // "GET"
mla_http_method_post    // "POST"
mla_http_method_put     // "PUT"
mla_http_method_delete  // "DELETE"
mla_http_method_options // "OPTIONS"
```

## Status Code Constants

```cpp
mla_http_status_ok                    // 200
mla_http_status_created               // 201
mla_http_status_no_content            // 204
mla_http_status_moved_permanently     // 301
mla_http_status_bad_request           // 400
mla_http_status_unauthorized          // 401
mla_http_status_forbidden             // 403
mla_http_status_not_found             // 404
mla_http_status_method_not_allowed    // 405
mla_http_status_internal_server_error // 500
```

---

## HTTP Server

### Step 1 — Create the Server

```cpp
#include "../http/mla_http_server.h"
#include "../network/mla_network.h"

mla_network_host_t host = mla_network_host_ip4(mla_string_const("0.0.0.0"), 8080);
mla_http_server_t server = mla_http_server(host);
```

### Step 2 — Write a Request Handler

A handler receives a request and populates a response. It returns `true` on success:

```cpp
static mla_bool_t api_status_handler(
    mla_http_server_t& http_server,
    const mla_user_data_t &userdata,
    const mla_http_request_t &request,
    mla_http_response_t &response)
{
    (void)userdata;
    response.statusCode = mla_http_status_ok;
    response.statusMessage = mla_string_const("OK");
    mla_http_headers_add(response.headers,
        mla_string_const("Content-Type"),
        mla_string_const("application/json"));

    // Provide the body via a content stream or a content writer
    mla_string_t body = mla_string_const("{\"status\":\"running\"}");
    response.content = mla_stream_input_from_string(body);
    return true;
}
```

### Step 3 — Register Handlers

```cpp
mla_user_data_t userdata = mla_user_data_empty();

// Match GET requests whose path starts with "/api/"
mla_http_server_handler_item_t handler =
    mla_http_server_handler_starts_with(
        mla_http_method_get,
        userdata,
        mla_string_const("/api/"),
        api_status_handler);
mla_http_server_register_handler(server, handler);

// Match all POST requests (any path)
mla_http_server_handler_item_t postAll =
    mla_http_server_handler_all(mla_http_method_post, userdata, post_handler);
mla_http_server_register_handler(server, postAll);
```

### Using Struct-Based Handlers (Static Dispatch)

For more complex handlers, you can define a struct with `http_request_check` and `http_request_handle` static methods, enabling static dispatch via class templates for better performance and type safety.

```cpp
struct my_custom_handler_t {
    mla_int32_t counter;

    static my_custom_handler_t init() {
        return { 0 };
    }

    static mla_bool_t http_request_check(
        my_custom_handler_t** self,
        const mla_user_data_t &userdata,
        const mla_string_t& url,
        mla_http_request_handler_checker_compare_mode_t compare_mode)
    {
        (void)self; (void)userdata; (void)compare_mode;
        return mla_string_starts_with(url, mla_string_const("/custom/"));
    }

    static mla_bool_t http_request_handle(
        my_custom_handler_t& self,
        mla_http_server_t& http_server,
        const mla_user_data_t &userdata,
        const mla_http_request_t &request,
        mla_http_response_t &response)
    {
        (void)http_server; (void)userdata; (void)request;
        self.counter++;
        response.statusCode = mla_http_status_ok;
        response.statusMessage = mla_string_const("OK");
        response.content = mla_stream_input_from_string(mla_string_const("Custom response"));
        return true;
    }
};

// Registration using template helper:
mla_user_data_t userdata = mla_user_data_empty();
my_custom_handler_t handler_struct = my_custom_handler_t::init();

mla_http_server_handler_item_t item =
    mla_http_server_handler_struct_starts_with<my_custom_handler_t>(
        mla_http_method_get,
        userdata,
        mla_string_const("/custom/"),
        handler_struct);
mla_http_server_register_handler(server, item);
```

### Step 4 — Start and Stop

```cpp
// Start with 4 worker tasks
mla_http_server_start(server, 4);

// Later …
mla_http_server_stop(server);
mla_http_server_destroy(server);
```

### Enable HTTPS / WSS on the Same Server

Use TLS server configuration before `mla_http_server_start`. The same handler and WebSocket pipeline is reused; only the transport becomes secure.

```cpp
mla_network_tls_config_t tls = mla_network_tls_config_default();
mla_network_tls_config_set_certificate(tls, server_certificate_pem);
mla_network_tls_config_set_private_key(tls, server_private_key_pem);

// Optional: provide custom CA chain if needed by your TLS backend
mla_network_tls_config_set_ca_certificate(tls, mla_string_empty());

mla_http_server_enable_tls(server, tls);

// HTTPS and WSS now run through the same HTTP server instance
mla_http_server_start(server, 4);
```

To go back to plain `http` / `ws` transport:

```cpp
mla_http_server_disable_security(server);
```

### Example — Consume Let’s Encrypt Certificates

Let’s Encrypt usually provides:
- `fullchain.pem` (server cert + intermediates)
- `privkey.pem` (private key)

Load PEM text from the mounted file system and pass it into `mla_network_tls_config_t`:

```cpp
// App-level helper: read full file content as UTF-8 text.
// Implement this helper using your platform/file-system module.
mla_string_t fullchain_pem = app_read_text_file(
    mla_string_const("/etc/letsencrypt/live/example.com/fullchain.pem"));
mla_string_t privkey_pem = app_read_text_file(
    mla_string_const("/etc/letsencrypt/live/example.com/privkey.pem"));

mla_network_tls_config_t tls = mla_network_tls_config_default();
mla_network_tls_config_set_certificate(tls, fullchain_pem);
mla_network_tls_config_set_private_key(tls, privkey_pem);

mla_http_server_enable_tls(server, tls);
mla_http_server_start(server, 4);
```

If secure transport is not supported by the active platform backend, secure start/connect fails deterministically.

### WebSocket Support

Register a WebSocket handler for a specific path:

```cpp
static mla_bool_t ws_text_handler(
    mla_http_server_websocket_connection_t& connection,
    const mla_string_t& message,
    mla_bool_t isFinalFragment)
{
    // Echo the message back
    mla_http_server_send_websocket_text_message(connection, message, true);
    return true;
}

mla_http_server_websocket_handler_item_t wsHandler =
    mla_http_server_websocket_handler_path_equals(
        mla_string_const("/ws"),
        ws_text_handler,   // text message handler
        nullptr            // binary message handler (optional)
    );
mla_http_server_register_websocket_handler(server, wsHandler);
```

WebSocket connection management:

```cpp
// Find connections by path prefix
auto conns = mla_http_server_find_websocket_connections(server, mla_string_const("/ws"));

// Send to a specific connection by ID
mla_http_server_send_websocket_text_message(server, connectionId, msg, true);

// Close a connection
mla_http_server_close_websocket_connection(connection, 1000, mla_string_const("Normal closure"));
```

---

## HTTP Client

### Sending a GET Request

```cpp
#include "../http/mla_http_client.h"

mla_http_request_t request = mla_http_get_request(
    mla_string_const("http://example.com/api/status"));
mla_http_headers_add(request.headers,
    mla_string_const("Accept"),
    mla_string_const("application/json"));

mla_http_client_response_t result = mla_http_client_send_request(request);

if (result.status == MLA_HTTP_CLIENT_RESPONSE_STATUS_OK) {
    mla_http_response_t& resp = result.response;
    // resp.statusCode, resp.headers, resp.content are available
} else {
    mla_error("HTTP request failed");
}

mla_http_client_response_destroy(result);
```

### Sending a POST Request with Body

```cpp
mla_http_request_t request = mla_http_post_request(
    mla_string_const("http://example.com/api/data"));
mla_http_headers_add(request.headers,
    mla_string_const("Content-Type"),
    mla_string_const("application/json"));

mla_string_t body = mla_string_const("{\"key\":\"value\"}");
request.content = mla_stream_input_from_string(body);

mla_http_client_response_t result = mla_http_client_send_request(request);
// … check result.status …
mla_http_client_response_destroy(result);
```

### Custom Client (Timeout)

```cpp
mla_http_client_t client = mla_http_client();
client.timeout_ms = 10000; // 10 seconds

mla_http_client_response_t result = mla_http_client_send_request(client, request);
```

### HTTPS Requests

`mla_http_client_send_request` selects secure transport automatically when the URL scheme is `https://`.

```cpp
mla_http_request_t request = mla_http_get_request(
    mla_string_const("https://example.com/api/status"));

mla_http_client_response_t result = mla_http_client_send_request(request);
```

#### Custom TLS Configuration on Client

You can configure TLS options such as peer verification, custom CA certificates, or client certificates:

```cpp
mla_http_client_t client = mla_http_client();

// Disable peer verification (e.g. for self-signed development certificates)
mla_http_client_set_verify_peer(client, false);

// Or configure full TLS settings
mla_network_tls_config_t tls = mla_network_tls_config_default();
mla_network_tls_config_set_ca_certificate(tls, custom_ca_pem);
mla_network_tls_config_set_verify_peer(tls, true);
mla_http_client_set_tls_config(client, tls);

mla_http_client_response_t result = mla_http_client_send_request(client, request);
```

### WSS Client Connections

`mla_websocket_client_connect` selects secure transport automatically for `wss://` URLs.

```cpp
#include "../http/mla_websocket_client.h"

mla_websocket_client_t ws = mla_websocket_client_invalid();
mla_bool_t connected = mla_websocket_client_connect(
    ws,
    mla_string_const("wss://example.com/ws"),
    5000,
    false);
```

### Client Response Status Codes

| Enum | Meaning |
|---|---|
| `MLA_HTTP_CLIENT_RESPONSE_STATUS_OK` | Request succeeded |
| `MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_WRONG_PROTOCOL` | Unsupported protocol |
| `MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_UNKNOWN_HOST` | DNS resolution failed |
| `MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_CONNECTION_FAILED` | TCP connection failed |
| `MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_TIMEOUT` | Request timed out |
| `MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_INVALID_RESPONSE` | Could not parse response |
| `MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_UNKNOWN` | Other error |

---

## Working with Headers

```cpp
// Add a header
mla_http_headers_add(request.headers,
    mla_string_const("Authorization"),
    mla_string_const("Bearer abc123"));

// Read a header value
mla_string_t contentType = mla_http_headers_get_value(
    response.headers,
    mla_string_const("Content-Type"));

// Check for a specific header value
mla_bool_t hasJson = mla_http_headers_has_header_value(
    response.headers,
    mla_string_const("Content-Type"),
    mla_string_const("application/json"));
```

## Rules

- Always call `mla_http_client_response_destroy` after processing a client response.
- Always call `mla_http_server_destroy` when the server is no longer needed.
- Request handlers must **not** store pointers to the request or response after returning — they are stack-allocated.
- Use `mla_http_server_handler_starts_with` for prefix-based URL matching and `mla_http_server_handler_all` for catch-all handlers.
- WebSocket message handlers must return `true` to keep the connection open and `false` to close it.
- Use `mla_stream_input_from_string` to set a string body on a request or response.
- Use `mla_http_server_enable_tls` to enable HTTPS/WSS transport on the server and `mla_http_server_disable_security` to revert to plaintext.
- Use `https://` and `wss://` URL schemes in clients to trigger secure transport selection.
- Handle deterministic secure-transport failures (`MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_CONNECTION_FAILED`) on platforms without TLS backend support.

## Incorrect Usage

```cpp
// ❌ Using standard library HTTP utilities
#include <curl/curl.h> // never — use mla_http_client

// ❌ Forgetting to destroy the client response
mla_http_client_response_t result = mla_http_client_send_request(request);
// leaked — always call mla_http_client_response_destroy(result)

// ❌ Not checking the response status
mla_http_client_response_t result = mla_http_client_send_request(request);
auto body = result.response.content; // may be invalid if status != OK

// ❌ Using printf for response output
printf("status: %d\n", response.statusCode); // use mla_info or stream writes
```

