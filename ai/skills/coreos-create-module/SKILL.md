---
name: coreos-create-module
description: Step-by-step guide for creating a new CoreOS module. Use when adding a new module or feature to the MLA framework, including header, implementation, lifecycle, and test setup.
metadata:
  author: coreos
  version: "1.0"
  converted-from: ai/instructions/create_a_module.instructions.md
---

# Creating a New CoreOS Module

Follow these steps whenever you need to create a new module in the CoreOS framework. Modules are self-contained units that expose a header file and an optional `.cpp` implementation.

## Directory Layout

Place all module files under `core-os/<module_name>/`:

```
core-os/
  my_feature/
    mla_my_feature.h       ← Public API declarations
    mla_my_feature.cpp     ← Implementation
    readme.md              ← Module documentation
```

## Step 1 — Define the Core Struct

Every module exposes one primary struct with an associated initializer and factory functions.

```cpp
// core-os/my_feature/mla_my_feature.h
#ifndef COREOS_MLA_MY_FEATURE_H
#define COREOS_MLA_MY_FEATURE_H

#include "../mla_data_types.h"
#include "../system/mla_string.h"
#include "../system/mla_user_data.h"

struct mla_my_feature_t {
    mla_string_t name;
    mla_int32_t  value;
    mla_user_data_t userData;
};

// Initializer required when the struct is used inside generic containers
struct mla_my_feature_initializer {
    static mla_my_feature_t init() {
        return { mla_string_empty(), 0, mla_user_data_empty() };
    }
};

// Factory function — preferred construction method
mla_my_feature_t mla_my_feature(const mla_string_t& p_Name, mla_int32_t p_Value);

// Sentinel / invalid value
mla_my_feature_t mla_my_feature_invalid();

// Validity check
mla_bool_t mla_my_feature_is_valid(const mla_my_feature_t& p_Feature);

#endif
```

### Key points
- Struct name ends with `_t`.
- Include `mla_data_types.h` for primitive types; never use `int`, `char`, etc.
- Always provide an `_invalid()` factory and a validity predicate.
- Provide an `_initializer` struct for use in `mla_array_list_t` / `mla_hash_map_t`.

## Step 2 — Implement the Module

```cpp
// core-os/my_feature/mla_my_feature.cpp
#include "mla_my_feature.h"

mla_my_feature_t mla_my_feature(const mla_string_t& p_Name, mla_int32_t p_Value) {
    return { p_Name, p_Value, mla_user_data_empty() };
}

mla_my_feature_t mla_my_feature_invalid() {
    return { mla_string_empty(), -1, mla_user_data_empty() };
}

mla_bool_t mla_my_feature_is_valid(const mla_my_feature_t& p_Feature) {
    return p_Feature.value >= 0;
}
```

### Key points
- Never use `new`/`delete` — use `mla_malloc` / `mla_free`.
- Never use `memcpy`, `strlen`, etc. — use `mla_memcpy`, `mla_strlen`, etc.
- Never include `<string>`, `<vector>`, or any standard library header.

## Step 3 — Register the Source File

Add the new `.cpp` file to the relevant source list in `sources.cmake`:

```cmake
set(SOURCE_FILES
    # … existing files …
    core-os/my_feature/mla_my_feature.cpp
)
```

## Step 4 — Add Logging

Use the logging macros from `core-os/log/mla_logging.h` to emit structured messages:

```cpp
#include "../log/mla_logging.h"

void some_function() {
    mla_info("My feature initialised");
    mla_debug("Processing value");
    mla_warning("Unexpected state encountered");
    mla_error("Operation failed");
}
```

Log levels (lowest → highest): `mla_verbose` → `mla_debug` → `mla_info` → `mla_warning` → `mla_error`.

## Step 5 — Hook into the Lifecycle (Optional)

If the module needs one-time initialisation at startup, register a boot event in the `.cpp` file:

```cpp
#include "../lifecycle/mla_lifecycle_events.h"

static void my_feature_boot() {
    // One-time setup here
    mla_info("my_feature ready");
}

mla_lifecycle_boot_event_static_register(
    mla_lifecycle_boot_event_priority_application_setup,
    my_feature_boot
)
```

See the `coreos-lifecycle-boot-events` skill for available priority levels.

## Step 6 — Expose RPC Procedures (Optional)

Use `mla_rpc_auto_register_procedure` to automatically register and expose a function over RPC.

See the `coreos-rpc-procedures` skill for full details.

## Step 7 — Add Reflection Metadata (Optional)

If the struct must be serialised/deserialised or used over RPC, add a `metadata()` function:

```cpp
struct mla_my_feature_t {
    mla_string_t name;
    mla_int32_t  value;

    static mla_reflection_struct_metadata_t metadata() {
        auto data = mla_reflection_struct_metadata(mla_my_feature_t);
        mla_reflection_struct_field_string(data, mla_my_feature_t, name);
        mla_reflection_struct_field_int32(data, mla_my_feature_t, value);
        mla_reflection_struct_metadata_freeze(data);
        return data;
    }
};
```

See the `coreos-reflection` skill for full details.

## Step 8 — Write Tests

Create a test file `core-os-test/mla_my_feature_test.h` and register it in `main_test.h`.

See the `coreos-create-test` skill for full details.

## Complete Checklist

- [ ] Header file created under `core-os/<module>/mla_<module>.h`
- [ ] Implementation file created under `core-os/<module>/mla_<module>.cpp`
- [ ] Struct uses `_t` suffix and `mla_` prefix
- [ ] Initializer struct provided for container compatibility
- [ ] Factory functions: constructor, `_invalid()`, validity check
- [ ] Only MLA types used (no `int`, `char`, `size_t`, etc.)
- [ ] Only MLA memory/string macros used (no `memcpy`, `strlen`, etc.)
- [ ] No standard library headers included
- [ ] `.cpp` added to `sources.cmake`
- [ ] Lifecycle boot event registered if needed
- [ ] Reflection metadata added if serialisation is needed
- [ ] Tests written
