---
name: 'mla-c-serialization'
description: 'Patterns for using serialization in the MLA framework'
---

# Serialization

The serializer module (`core/serializer/mla_serializer.h`) serializes and deserializes structured data to JSON, binary, or XML formats. Every struct that participates in serialization must implement two static member functions — `serialize` and `deserialize`.

## The Static Serialize / Deserialize Convention

Every serializable struct must define both static functions inside the struct body. These are the functions that `mla_serialize_definition<T>()` and `mla_serializer_write_data_struct` / `mla_serializer_read_data_struct` bind to automatically.

```cpp
#include "../serializer/mla_serializer.h"

struct mla_sensor_reading_t {
    mla_string_t sensorId;
    mla_float_t  temperature;
    mla_int32_t  humidity;
    mla_bool_t   isValid;

    static mla_sensor_reading_t init() {
        return {
            mla_string_empty(),
            0.0f,
            0,
            false
        };
    }

    // --- Serialize: write all fields to the serializer ---
    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_sensor_reading_t* self = mla_pointer_get_data<const mla_sensor_reading_t>(obj);
        if (self == nullptr) {
            return false;
        }
        mla_serializer_write_string(serializer, mla_string_const("sensorId"),    self->sensorId);
        mla_serializer_write_float (serializer, mla_string_const("temperature"), self->temperature);
        mla_serializer_write_int32 (serializer, mla_string_const("humidity"),    self->humidity);
        mla_serializer_write_bool  (serializer, mla_string_const("isValid"),     self->isValid);
        return true;
    }

    // --- Deserialize: handle one property at a time ---
    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_sensor_reading_t* self = mla_pointer_get_data<mla_sensor_reading_t>(obj);
        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }
        if (mla_string_equals_const(property_name, "sensorId")) {
            mla_deserializer_read_string(deserializer, self->sensorId);
        } else if (mla_string_equals_const(property_name, "temperature")) {
            mla_deserializer_read_float(deserializer, self->temperature);
        } else if (mla_string_equals_const(property_name, "humidity")) {
            mla_deserializer_read_int32(deserializer, self->humidity);
        } else if (mla_string_equals_const(property_name, "isValid")) {
            mla_deserializer_read_bool(deserializer, self->isValid);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }
};
```

### Serialize Function Signature

```cpp
static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj);
```

- Extract data pointer via `mla_pointer_get_data<const MyStruct>(obj)` at the top of the function and check for `nullptr`.
- Write every field using the appropriate `mla_serializer_write_*` macro.
- Return `true` on success, `false` on failure.

### Deserialize Function Signature

```cpp
static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name);
```

- Extract data pointer via `mla_pointer_get_data<MyStruct>(obj)` at the top of the function and check for `nullptr`.
- Match `property_name` against each field name using `mla_string_equals_const`.
- Call the corresponding `mla_deserializer_read_*` macro for matched fields — these macros return internally.
- Return `MLA_DESERIALIZER_READ_SKIPPED` in the `else` branch for unknown properties.

## Serializer Write Macros

| Macro | MLA Type |
|---|---|
| `mla_serializer_write_bool(instance, name, value)` | `mla_bool_t` |
| `mla_serializer_write_int8(instance, name, value)` | `mla_int8_t` |
| `mla_serializer_write_int16(instance, name, value)` | `mla_int16_t` |
| `mla_serializer_write_int32(instance, name, value)` | `mla_int32_t` |
| `mla_serializer_write_int64(instance, name, value)` | `mla_int64_t` |
| `mla_serializer_write_uint8(instance, name, value)` | `mla_uint8_t` |
| `mla_serializer_write_uint16(instance, name, value)` | `mla_uint16_t` |
| `mla_serializer_write_uint32(instance, name, value)` | `mla_uint32_t` |
| `mla_serializer_write_uint64(instance, name, value)` | `mla_uint64_t` |
| `mla_serializer_write_float(instance, name, value)` | `mla_float_t` |
| `mla_serializer_write_double(instance, name, value)` | `mla_double_t` |
| `mla_serializer_write_string(instance, name, value)` | `mla_string_t` |
| `mla_serializer_write_bytes(instance, name, bytes)` | `mla_bytes_t` |
| `mla_serializer_write_enum(instance, name, value)` | any `enum` (stored as `mla_uint8_t`) |
| `mla_serializer_write_list(instance, name, list)` | primitive `mla_array_list_t` |
| `mla_serializer_write_list_struct(instance, name, list, StructType)` | struct `mla_array_list_t` |
| `mla_serializer_write_struct(instance, name, value, StructType)` | nested struct |

## Deserializer Read Macros

| Macro | MLA Type |
|---|---|
| `mla_deserializer_read_bool(instance, setter)` | `mla_bool_t` |
| `mla_deserializer_read_int8(instance, setter)` | `mla_int8_t` |
| `mla_deserializer_read_int16(instance, setter)` | `mla_int16_t` |
| `mla_deserializer_read_int32(instance, setter)` | `mla_int32_t` |
| `mla_deserializer_read_int64(instance, setter)` | `mla_int64_t` |
| `mla_deserializer_read_uint8(instance, setter)` | `mla_uint8_t` |
| `mla_deserializer_read_uint16(instance, setter)` | `mla_uint16_t` |
| `mla_deserializer_read_uint32(instance, setter)` | `mla_uint32_t` |
| `mla_deserializer_read_uint64(instance, setter)` | `mla_uint64_t` |
| `mla_deserializer_read_float(instance, setter)` | `mla_float_t` |
| `mla_deserializer_read_double(instance, setter)` | `mla_double_t` |
| `mla_deserializer_read_string(instance, setter)` | `mla_string_t` |
| `mla_deserializer_read_bytes(instance, setter)` | `mla_bytes_t` |
| `mla_deserializer_read_enum(EnumType, instance, setter)` | any `enum` |
| `mla_deserializer_read_list_struct(instance, setter, StructType)` | struct `mla_array_list_t` |
| `mla_deserializer_read_struct(instance, setter, StructType)` | nested struct |

> **Note:** All `mla_deserializer_read_*` macros contain an implicit `return` — do **not** write an explicit return after them.

## High-Level Struct Serialization & Deserialization Helpers

Use `mla_serializer_write_data_struct` and `mla_serializer_read_data_struct` to serialize or deserialize an entire root struct instance to/from a serializer/deserializer:

```cpp
// Serialize a struct
mla_sensor_reading_t reading = mla_sensor_reading_t::init();
reading.sensorId = mla_string_const("sensor-01");
reading.temperature = 23.5f;

mla_stream_output_t stream = mla_stream_output_string_builder();
mla_serializer_t serializer = mla_json_serializer(stream);

mla_serializer_write_data_struct(serializer, reading);

// Deserialize a struct
mla_stream_input_t input = mla_stream_input_from_string(json_string);
mla_deserializer_t deserializer = mla_json_deserializer(input);

mla_sensor_reading_t loaded = mla_sensor_reading_t::init();
mla_serializer_read_data_struct(deserializer, loaded);
```

## Rules

- Every serializable struct **must** provide both `serialize` and `deserialize` static member functions.
- The `serialize` function must use `const mla_pointer_t& obj` and cast using `mla_pointer_get_data<const MyStruct>(obj)`; the `deserialize` function must use `mla_pointer_t& obj` and cast using `mla_pointer_get_data<MyStruct>(obj)`.
- **NEVER** write manual token reading loops (e.g. `while (deserializer.read_next(...))`) to parse struct properties manually. Always use `mla_serializer_write_data_struct` and `mla_serializer_read_data_struct` with static `serialize`/`deserialize` struct methods.
- All `mla_deserializer_read_*` macros return internally — do **not** add an explicit `return` statement after them.
- Always return `MLA_DESERIALIZER_READ_SKIPPED` in the `else` branch for unknown properties.
- Nested struct types used via `mla_serializer_write_struct` / `mla_deserializer_read_struct` must also implement `serialize` and `deserialize`.
- List element types used via `mla_serializer_write_list_struct` / `mla_deserializer_read_list_struct` must also implement `serialize` and `deserialize`.
- Only MLA types may appear in serializable structs — never standard C/C++ types.

## Incorrect Usage

```cpp
// ❌ Manual token parsing loop instead of framework helper
while (deserializer.read_next(deserializer)) { ... } // ❌ Use static deserialize method & mla_serializer_read_data_struct

// ❌ Missing SKIPPED return for unknown properties
static mla_deserializer_read_result_t deserialize(...) {
    if (mla_string_equals_const(property_name, "value")) {
        mla_deserializer_read_int32(deserializer, self->value);
    }
    // forgot: return MLA_DESERIALIZER_READ_SKIPPED;
}

// ❌ Explicit return after mla_deserializer_read_* macro (macro already returns)
if (mla_string_equals_const(property_name, "value")) {
    mla_deserializer_read_int32(deserializer, self->value);
    return MLA_DESERIALIZER_READ_HANDLED; // ❌ unreachable / duplicate return
}

// ❌ Using raw void pointers or C-style casts in serialize
static mla_bool_t serialize(mla_serializer_t& serializer, mla_platform_const_pointer_t obj) { // ❌ Use const mla_pointer_t&
    const MyStruct* self = (const MyStruct*)obj; // ❌ Use mla_pointer_get_data<const MyStruct>(obj)
}
```
