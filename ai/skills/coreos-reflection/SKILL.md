---
name: coreos-reflection
description: Patterns for adding reflection metadata in the MLA framework. Use when enabling runtime type information, struct discovery, RPC integration, or serialization for CoreOS types.
metadata:
  author: coreos
  version: "1.0"
  converted-from: ai/instructions/reflection.instructions.md
---

# Reflection

The reflection module (`core-os/reflection/mla_reflection.h`) provides **runtime type metadata** for structs. This metadata enables the type system to discover struct fields at runtime and is required whenever a struct is used over RPC or with the serializer.

## When to Add Reflection

Add a `metadata()` function to a struct whenever:
- The struct is used as an RPC input or output.
- The struct must be saved/loaded via a serializer.
- The struct needs to be discoverable by the type system at runtime.

## Step 1 — Add a `metadata()` Function to Your Struct

The `metadata()` function must be a `static` member function that returns `mla_reflection_struct_metadata_t`. Declare each field using the appropriate macro, then **freeze** the metadata.

```cpp
#include "../reflection/mla_reflection.h"

struct mla_sensor_reading_t {
    mla_string_t  sensorId;
    mla_float_t   temperature;
    mla_int32_t   humidity;
    mla_bool_t    isValid;

    static mla_reflection_struct_metadata_t metadata() {
        auto data = mla_reflection_struct_metadata(mla_sensor_reading_t);
        mla_reflection_struct_field_string(data, mla_sensor_reading_t, sensorId);
        mla_reflection_struct_field_float (data, mla_sensor_reading_t, temperature);
        mla_reflection_struct_field_int32 (data, mla_sensor_reading_t, humidity);
        mla_reflection_struct_field_bool  (data, mla_sensor_reading_t, isValid);
        mla_reflection_struct_metadata_freeze(data);
        return data;
    }
};
```

## Field Macros Reference

Use the macro that matches the **MLA type** of the field — never a standard C/C++ type.

### Primitive fields

| Macro | MLA Field Type |
|---|---|
| `mla_reflection_struct_field_bool` | `mla_bool_t` |
| `mla_reflection_struct_field_int8` | `mla_int8_t` |
| `mla_reflection_struct_field_int16` | `mla_int16_t` |
| `mla_reflection_struct_field_int32` | `mla_int32_t` |
| `mla_reflection_struct_field_int64` | `mla_int64_t` |
| `mla_reflection_struct_field_uint8` | `mla_uint8_t` |
| `mla_reflection_struct_field_uint16` | `mla_uint16_t` |
| `mla_reflection_struct_field_uint32` | `mla_uint32_t` |
| `mla_reflection_struct_field_uint64` | `mla_uint64_t` |
| `mla_reflection_struct_field_float` | `mla_float_t` |
| `mla_reflection_struct_field_double` | `mla_double_t` |
| `mla_reflection_struct_field_string` | `mla_string_t` |
| `mla_reflection_struct_field_bytes` | `mla_bytes_t` |
| `mla_reflection_struct_field_enum` | Any `enum` stored as `mla_uint8_t` |

### Nested struct field

```cpp
mla_reflection_struct_field_struct(data, mla_parent_t, childField, mla_child_t);
// mla_child_t must also have a static metadata() function
```

### List fields

```cpp
// List of primitives
mla_reflection_struct_field_int32_list (data, mla_my_t, counts);
mla_reflection_struct_field_string_list(data, mla_my_t, tags);
mla_reflection_struct_field_float_list (data, mla_my_t, values);

// List of nested structs (element type must have metadata())
mla_reflection_struct_field_struct_list(data, mla_parent_t, children, mla_child_t);
```

## Step 2 — Register the Struct

### Option A: Auto-registration at boot (preferred for global types)

Place this macro at file scope in your `.cpp` file:

```cpp
#include "../reflection/mla_reflection.h"

mla_reflection_auto_register_struct(mla_sensor_reading_t)
```

This fires at `mla_lifecycle_boot_event_priority_low_level_postSetup` (-990).

### Option B: Register via RPC macro (for RPC input/output types)

When using `mla_rpc_auto_register_procedure`, the macro automatically calls `mla_reflection_register_struct<T>()` for both input and output types. No separate step is needed.

### Option C: Manual registration

```cpp
mla_reflection_register_struct<mla_sensor_reading_t>();
// Also recursively registers all nested struct types:
mla_reflection_register_inner_structs(mla_sensor_reading_t::metadata());
```

## Nested Struct Example (Full)

```cpp
struct mla_gps_position_t {
    mla_double_t latitude;
    mla_double_t longitude;

    static mla_reflection_struct_metadata_t metadata() {
        auto data = mla_reflection_struct_metadata(mla_gps_position_t);
        mla_reflection_struct_field_double(data, mla_gps_position_t, latitude);
        mla_reflection_struct_field_double(data, mla_gps_position_t, longitude);
        mla_reflection_struct_metadata_freeze(data);
        return data;
    }
};

struct mla_device_report_t {
    mla_string_t       deviceId;
    mla_gps_position_t position;
    mla_array_list_t<mla_sensor_reading_t, mla_sensor_reading_initializer> readings;

    static mla_reflection_struct_metadata_t metadata() {
        auto data = mla_reflection_struct_metadata(mla_device_report_t);
        mla_reflection_struct_field_string      (data, mla_device_report_t, deviceId);
        mla_reflection_struct_field_struct      (data, mla_device_report_t, position, mla_gps_position_t);
        mla_reflection_struct_field_struct_list (data, mla_device_report_t, readings, mla_sensor_reading_t);
        mla_reflection_struct_metadata_freeze(data);
        return data;
    }
};
```

## Runtime Metadata Lookup

```cpp
mla_reflection_struct_metadata_t meta = {};
if (mla_reflection_get_struct_metadata(mla_string_const("mla_sensor_reading_t"), meta)) {
    // iterate fields …
    for (mla_size_t i = 0; i < mla_array_list_size(meta.fields); i++) {
        auto& field = mla_array_list_get(meta.fields, i);
        mla_info(mla_string_data(field.name));
    }
}
```

## Rules

- Always call `mla_reflection_struct_metadata_freeze(data)` at the end of `metadata()`.
- Use `mla_offsetof(struct_type, field_name)` — never compute offsets manually.
- Nested struct types used in `mla_reflection_struct_field_struct*` macros must also have a `metadata()` function.
- Register nested structs before the parent if registering manually; `mla_reflection_register_inner_structs` handles this automatically.
- The `metadata()` function must be **pure** (no side-effects, no global state mutation).

## Incorrect Usage

```cpp
// ❌ Missing freeze — metadata is considered invalid
auto data = mla_reflection_struct_metadata(mla_my_t);
mla_reflection_struct_field_int32(data, mla_my_t, value);
return data; // forgot mla_reflection_struct_metadata_freeze(data)!

// ❌ Using standard field type in macro — offset calculation will be wrong
mla_reflection_struct_field_int32(data, mla_my_t, myIntField); // ok only if myIntField is mla_int32_t

// ❌ Registering the same struct twice manually
mla_reflection_register_struct<mla_my_t>();
mla_reflection_register_struct<mla_my_t>(); // second call is silently ignored but wasteful
```
