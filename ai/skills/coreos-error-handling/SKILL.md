---
name: coreos-error-handling
description: Error handling patterns and conventions for the MLA framework. Use when implementing error returns, status enums, validity checks, or logging errors in CoreOS modules.
metadata:
  author: coreos
  version: "1.0"
---

# Error Handling

CoreOS is an **exception-free** C++ framework. All error handling uses return values, status enums, and structured logging. This skill describes the standard patterns.

## Pattern 1: Boolean Return (Most Common)

Functions return `mla_bool_t` — `true` on success, `false` on failure:

```cpp
mla_bool_t mla_http_server_start(mla_http_server_t& p_Server, mla_uint8_t p_TaskCount, mla_task_stack_size p_StackSize);
mla_bool_t mla_http_server_register_handler(mla_http_server_t& p_Server, const mla_http_server_handler_item_t& p_Handler);
mla_bool_t mla_mutex_lock(mla_mutex_t& p_Mutex);
```

### Usage

```cpp
if (!mla_http_server_start(server, 2, TASK_STACK_SIZE_LARGE)) {
    mla_error("Failed to start HTTP server");
    return false;  // propagate failure
}
```

### Chaining Boolean Checks

```cpp
mla_bool_t init_system() {
    if (!mla_config_load(config)) {
        mla_error("Config load failed");
        return false;
    }
    if (!mla_http_server_start(server, 2, TASK_STACK_SIZE_LARGE)) {
        mla_error("Server start failed");
        return false;
    }
    if (!mla_rpc_register_procedure(procedure)) {
        mla_error("RPC registration failed");
        return false;
    }
    mla_info("System initialised successfully");
    return true;
}
```

## Pattern 2: Status Enum

For operations with multiple failure modes, use a status enum:

```cpp
enum mla_http_client_response_status: mla_uint8_t {
    MLA_HTTP_CLIENT_RESPONSE_STATUS_OK,
    MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_WRONG_PROTOCOL,
    MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_UNKNOWN_HOST,
    MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_CONNECTION_FAILED,
    MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_TIMEOUT,
    MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_INVALID_RESPONSE,
    MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_UNKNOWN
};

struct mla_http_client_response_t {
    mla_http_client_response_status status;
    mla_string_t errorMessage;      // populated on failure
    mla_http_response_t response;   // populated on success
};
```

### Usage

```cpp
auto response = mla_http_client_send_request(request);
switch (response.status) {
    case MLA_HTTP_CLIENT_RESPONSE_STATUS_OK:
        process_response(response.response);
        break;
    case MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_TIMEOUT:
        mla_warning("Request timed out, retrying...");
        break;
    default:
        mla_error("HTTP request failed");
        break;
}
```

### Defining Your Own Status Enum

```cpp
enum mla_my_result_t: mla_uint8_t {
    MLA_MY_RESULT_OK,
    MLA_MY_RESULT_ERROR_INVALID_INPUT,
    MLA_MY_RESULT_ERROR_NOT_FOUND,
    MLA_MY_RESULT_ERROR_TIMEOUT
};
```

Naming convention: `MLA_<MODULE>_<RESULT>_ERROR_<REASON>`

## Pattern 3: Output Parameters

Return the success flag and write results to a reference parameter:

```cpp
mla_bool_t mla_hash_map_get(
    const mla_hash_map_t<...>& p_Map,
    const TKey& p_Key,
    TValue& p_OutValue          // written on success
);
```

### Usage

```cpp
mla_string_t value = mla_string_empty();
if (mla_hash_map_get(map, mla_string_const("key"), value)) {
    // value is valid
} else {
    mla_warning("Key not found");
}
```

## Pattern 4: Nullable Pointers

Functions returning a pointer return `nullptr` on failure:

```cpp
TValue* mla_hash_map_get_ref(mla_hash_map_t<...>& p_Map, const TKey& p_Key);
```

### Usage

```cpp
auto* value = mla_hash_map_get_ref(map, key);
if (value != nullptr) {
    *value = newValue;  // modify in-place
} else {
    mla_warning("Key not found in map");
}
```

## Pattern 5: Invalid State Objects

Many types provide `_invalid()` factory functions and validity predicates:

```cpp
mla_my_feature_t mla_my_feature_invalid();
mla_bool_t mla_my_feature_is_valid(const mla_my_feature_t& p_Feature);
```

### Usage

```cpp
mla_my_feature_t feature = mla_my_feature_invalid();

// ... try to initialise feature ...

if (!mla_my_feature_is_valid(feature)) {
    mla_error("Feature initialisation failed");
    return;
}
```

## Pattern 6: Task Return States

Tasks signal completion or continuation via return values:

```cpp
static mla_task_process_result_state my_task(mla_user_data_t& p_UserData) {
    if (error_occurred) {
        mla_error("Task encountered an error");
        return TASK_PROCESS_RESULT_DONE;  // stop the task
    }
    return TASK_PROCESS_RESULT_CONTINUE;  // keep running
}
```

## Logging Errors

Always log errors at the point of failure with context:

```cpp
// Use the appropriate log level
mla_error("Failed to connect to server");
mla_warning("Connection retry — attempt 3 of 5");
mla_debug("Parsing failed at offset 42");

// With context
mla_log_error(
    mla_string_const("Connection failed"),
    mla_string_const("192.168.1.10:8080")
);
```

## Rules

- **Never** use C++ exceptions (`throw`, `try`, `catch`).
- **Never** include `<stdexcept>`, `<exception>`, or any exception header.
- Always check return values — do not ignore `mla_bool_t` results from functions that can fail.
- Log errors at the point of failure, not at the caller.
- Propagate failures up the call stack by returning `false` or an error status.
- Clean up resources on the failure path before returning.
- Use `_invalid()` / `_is_valid()` pairs for types that have a meaningful invalid state.
- Status enums should use `mla_uint8_t` as the underlying type.

## Incorrect Usage

```cpp
// ❌ Using C++ exceptions
throw std::runtime_error("error");  // never use exceptions

// ❌ Ignoring return values
mla_http_server_start(server, 2, TASK_STACK_SIZE_LARGE);  // not checking result!

// ❌ Not logging before returning failure
if (!success) {
    return false;  // should log: mla_error("reason for failure");
}

// ❌ Not cleaning up on failure
mla_pointer_t buf = mla_malloc(1024);
if (!mla_some_operation(buf)) {
    return false;  // memory leak! should: mla_free(buf); return false;
}
```
