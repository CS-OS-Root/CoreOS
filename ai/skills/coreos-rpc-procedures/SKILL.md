---
name: coreos-rpc-procedures
description: Patterns for creating and registering RPC procedures in the MLA framework. Use when exposing functions over RPC, creating remote endpoints, or calling procedures locally or remotely.
metadata:
  author: coreos
  version: "1.0"
  converted-from: ai/instructions/rpc_procedures.instructions.md
---

# RPC Procedures

The RPC module (`core-os/rpc/mla_rpc.h`) provides local and remote procedure call infrastructure. Procedures are identified by a string name, carry typed input/output structs, and are automatically discovered over HTTP when using `mla_rpc_http_server`.

## Core Concepts

| Term | Meaning |
|---|---|
| **Procedure** | A named function with typed input and output |
| **Local procedure** | Registered on this node; called directly in-process |
| **Remote endpoint** | An HTTP (or other transport) destination that handles calls for a set of procedure names |
| **Safe procedure** | Template wrapper with concrete input/output types |
| **Unsafe procedure** | Raw function-pointer variant using `void*` — used internally |

## Quickstart — Auto-Register a Procedure (Recommended)

Use one of the four `mla_rpc_auto_register_procedure_*` macros. They register the procedure at boot (priority `rpc_preSetup`) and also register the struct reflection metadata.

### With both input and output

```cpp
#include "../rpc/mla_rpc.h"

// Input / output structs must have a static metadata() function (see reflection guide)
struct mla_add_input_t {
    mla_int32_t a;
    mla_int32_t b;

    static mla_reflection_struct_metadata_t metadata() {
        auto data = mla_reflection_struct_metadata(mla_add_input_t);
        mla_reflection_struct_field_int32(data, mla_add_input_t, a);
        mla_reflection_struct_field_int32(data, mla_add_input_t, b);
        mla_reflection_struct_metadata_freeze(data);
        return data;
    }
};

struct mla_add_output_t {
    mla_int32_t result;

    static mla_reflection_struct_metadata_t metadata() {
        auto data = mla_reflection_struct_metadata(mla_add_output_t);
        mla_reflection_struct_field_int32(data, mla_add_output_t, result);
        mla_reflection_struct_metadata_freeze(data);
        return data;
    }
};

// Handler function — must match signature bool(*)(const TInput*, TOutput*)
static mla_bool_t add_handler(const mla_add_input_t* p_Input, mla_add_output_t* p_Output) {
    p_Output->result = p_Input->a + p_Input->b;
    return true;
}

// Registers "math.add" automatically at startup
mla_rpc_auto_register_procedure("math.add", mla_add_input_t, mla_add_output_t, add_handler)
```

### With void output (fire-and-forget)

```cpp
static mla_bool_t log_handler(const mla_log_input_t* p_Input, void*) {
    mla_info(mla_string_data(p_Input->message));
    return true;
}

mla_rpc_auto_register_procedure_void_output("log.write", mla_log_input_t, log_handler)
```

### With void input (query with no parameters)

```cpp
static mla_bool_t get_status_handler(const void*, mla_status_output_t* p_Output) {
    p_Output->running = true;
    return true;
}

mla_rpc_auto_register_procedure_void_input("system.getStatus", mla_status_output_t, get_status_handler)
```

### With both void input and output

```cpp
static mla_bool_t reboot_handler(const void*, void*) {
    // trigger reboot
    return true;
}

mla_rpc_auto_register_procedure_void("system.reboot", reboot_handler)
```

## Calling a Procedure Locally

```cpp
mla_add_input_t  input  = { 3, 4 };
mla_add_output_t output = {};

mla_bool_t ok = mla_rpc_execute_procedure(mla_string_const("math.add"), &input, &output);
// output.result == 7
```

For void variants:

```cpp
// void output
mla_rpc_execute_procedure_void_output(mla_string_const("log.write"), &logInput);

// void input
mla_rpc_execute_procedure_void_input(mla_string_const("system.getStatus"), &statusOutput);

// both void
mla_rpc_execute_procedure(mla_string_const("system.reboot"));
```

## Calling a Procedure on a Remote Endpoint

```cpp
mla_add_input_t  input  = { 10, 20 };
mla_add_output_t output = {};

mla_bool_t ok = mla_rpc_execute_procedure_remote(mla_string_const("math.add"), &input, &output);
```

The framework routes the call to the registered remote endpoint that handles the procedure name.

## Registering a Remote Endpoint

```cpp
#include "../rpc/mla_rpc_http_client.h"

// Forward all unknown procedure calls to an HTTP server
mla_user_data_t userData = mla_user_data_empty();
mla_user_data_set_cstring(userData, "url", "http://192.168.1.10:8080");

mla_rpc_remote_endpoint_t endpoint = mla_rpc_remote_endpoint_all(
    mla_rpc_http_client_execute_procedure, // transport handler
    userData
);
mla_rpc_register_remote_endpoint(endpoint);
```

Use `mla_rpc_remote_endpoint_start_with` to limit routing to procedures with a specific prefix:

```cpp
mla_rpc_remote_endpoint_t endpoint = mla_rpc_remote_endpoint_start_with(
    mla_string_const("sensor."),
    mla_rpc_http_client_execute_procedure,
    userData
);
```

## Manual Procedure Registration (Advanced)

When the auto-register macros are not applicable:

```cpp
// 1. Build reflect definitions
mla_reflection_register_struct<mla_add_input_t>();
mla_reflection_register_struct<mla_add_output_t>();

auto inputDef  = mla_serialize_definition<mla_add_input_t>();
auto outputDef = mla_serialize_definition<mla_add_output_t>();

// 2. Create the safe procedure descriptor
auto procedure = mla_rpc_procedure_safe<mla_add_input_t, mla_add_output_t>(
    mla_string_const("math.add"),
    inputDef,
    outputDef,
    add_handler
);

// 3. Register
mla_rpc_register_procedure<mla_add_input_t, mla_add_output_t>(procedure);
```

## Querying Available Procedures

```cpp
auto procedures = mla_rpc_list_procedures();
for (mla_size_t i = 0; i < mla_array_list_size(procedures); i++) {
    auto& proc = mla_array_list_get(procedures, i);
    mla_info(mla_string_data(proc.procedureName));
}
```

## Rules

- Procedure names are **dot-separated strings** (e.g., `"module.action"`).
- Input/output structs must provide a `static metadata()` function for automatic serialisation.
- Handler functions **must not** allocate memory that outlives the call unless they take ownership via `mla_malloc`.
- Return `true` on success, `false` on failure. The framework propagates the return value to the caller.
- Unregister procedures when a module is torn down: `mla_rpc_unregister_procedure(name)`.

## Incorrect Usage

```cpp
// ❌ Using standard types in input/output structs
struct bad_input_t { int value; };

// ❌ Calling mla_rpc_execute_procedure before boot events have fired
mla_rpc_execute_procedure(mla_string_const("math.add"), &in, &out); // too early

// ❌ Same handler name in two mla_rpc_auto_register_procedure macros
mla_rpc_auto_register_procedure("a.fn", InputA, OutputA, my_handler)
mla_rpc_auto_register_procedure("b.fn", InputB, OutputB, my_handler) // name collision
```
