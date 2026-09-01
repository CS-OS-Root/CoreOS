---
name: coreos-serialization
description: Patterns for using serialization in the MLA framework. Use when implementing JSON, binary, or XML serialization and deserialization for structs, including nested types and lists.
metadata:
  author: coreos
  version: "1.0"
  converted-from: ai/instructions/serialization.instructions.md
---

# Serialization

The serializer module (`core-os/serializer/mla_serializer.h`) serializes and deserializes structured data to JSON, binary, or XML formats. Every struct that participates in serialization must implement two static member functions — `serialize` and `deserialize`.

## The Static Serialize / Deserialize Convention

Every serializable struct must define both static functions inside the struct body. These are the functions that `mla_serialize_definition<T>()` binds to automatically.

```cpp
#include "../serializer/mla_serializer.h"
#include "../reflection/mla_reflection.h"

struct mla_sensor_reading_t {
    mla_string_t sensorId;
    mla_float_t  temperature;
    mla_int32_t  humidity;
    mla_bool_t   isValid;

    // --- Serialize: write all fields to the serializer ---
    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t obj) {
        const mla_sensor_reading_t* self = static_cast<const mla_sensor_reading_t*>(obj);
        mla_serializer_write_string(serializer, mla_string_const("sensorId"),    self->sensorId);
        mla_serializer_write_float (serializer, mla_string_const("temperature"), self->temperature);
        mla_serializer_write_int32 (serializer, mla_string_const("humidity"),    self->humidity);
        mla_serializer_write_bool  (serializer, mla_string_const("isValid"),     self->isValid);
        return true;
    }

    // --- Deserialize: handle one property at a time ---
    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t obj, const mla_string_t& property_name) {
        mla_sensor_reading_t* self = static_cast<mla_sensor_reading_t*>(obj);
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
static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t obj);
```

- Cast `obj` to `const MyStruct*` at the top of the function.
- Write every field using the appropriate `mla_serializer_write_*` macro.
- Return `true` on success, `false` on failure.

### Deserialize Function Signature

```cpp
static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t obj, const mla_string_t& property_name);
```

- Cast `obj` to `MyStruct*` at the top of the function.
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

## Nested Struct and List Example (Full)

```cpp
struct mla_gps_position_t {
    mla_double_t latitude;
    mla_double_t longitude;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t obj) {
        const mla_gps_position_t* self = static_cast<const mla_gps_position_t*>(obj);
        mla_serializer_write_double(serializer, mla_string_const("latitude"),  self->latitude);
        mla_serializer_write_double(serializer, mla_string_const("longitude"), self->longitude);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t obj, const mla_string_t& property_name) {
        mla_gps_position_t* self = static_cast<mla_gps_position_t*>(obj);
        if (mla_string_equals_const(property_name, "latitude")) {
            mla_deserializer_read_double(deserializer, self->latitude);
        } else if (mla_string_equals_const(property_name, "longitude")) {
            mla_deserializer_read_double(deserializer, self->longitude);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }

};

struct mla_device_report_t {
    mla_string_t       deviceId;
    mla_gps_position_t position;
    mla_array_list_t<mla_sensor_reading_t, mla_sensor_reading_initializer> readings;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t obj) {
        const mla_device_report_t* self = static_cast<const mla_device_report_t*>(obj);
        mla_serializer_write_string     (serializer, mla_string_const("deviceId"), self->deviceId);
        mla_serializer_write_struct     (serializer, mla_string_const("position"), self->position, mla_gps_position_t);
        mla_serializer_write_list_struct(serializer, mla_string_const("readings"), self->readings, mla_sensor_reading_t);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t obj, const mla_string_t& property_name) {
        mla_device_report_t* self = static_cast<mla_device_report_t*>(obj);
        if (mla_string_equals_const(property_name, "deviceId")) {
            mla_deserializer_read_string(deserializer, self->deviceId);
        } else if (mla_string_equals_const(property_name, "position")) {
            mla_deserializer_read_struct(deserializer, self->position, mla_gps_position_t);
        } else if (mla_string_equals_const(property_name, "readings")) {
            mla_deserializer_read_list_struct(deserializer, self->readings, mla_sensor_reading_t);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }

};
```

## Serialize to JSON

```cpp
#include "../serializer/mla_json_serializer.h"

mla_sensor_reading_t reading = { mla_string_const("sensor-01"), 23.5f, 65, true };

mla_stream_output_t stream = mla_stream_output_string_builder();
mla_serializer_t serializer = mla_json_serializer(stream);

auto definition = mla_serialize_definition<mla_sensor_reading_t>();
mla_serialize(serializer, definition, &reading);

// Retrieve the JSON string from the builder stream
mla_string_t json = mla_stream_output_string_builder_get(stream);
```

## Deserialize from JSON

```cpp
#include "../serializer/mla_json_serializer.h"

mla_string_t json = mla_string_const("{\"sensorId\":\"sensor-01\",\"temperature\":23.5}");
mla_sensor_reading_t reading = {};

mla_stream_input_t stream = mla_stream_input_from_string(json);
mla_deserializer_t deserializer = mla_json_deserializer(stream);

auto definition = mla_serialize_definition<mla_sensor_reading_t>();
mla_deserialize(deserializer, definition, &reading);
```

## Rules

- Every serializable struct **must** provide both `serialize` and `deserialize` static member functions.
- The `serialize` function must cast `obj` to `const MyStruct*`; the `deserialize` function must cast `obj` to `MyStruct*`.
- All `mla_deserializer_read_*` macros return internally — do **not** add an explicit `return` statement after them.
- Always return `MLA_DESERIALIZER_READ_SKIPPED` in the `else` branch for unknown properties.
- Nested struct types used via `mla_serializer_write_struct` / `mla_deserializer_read_struct` must also implement `serialize` and `deserialize`.
- List element types used via `mla_serializer_write_list_struct` / `mla_deserializer_read_list_struct` must also implement `serialize` and `deserialize`.
- Only MLA types may appear in serializable structs — never standard C/C++ types.

## Incorrect Usage

```cpp
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

// ❌ Using standard types in serializable structs
struct bad_t {
    int value; // use mla_int32_t instead
    static mla_bool_t serialize(...) { ... }
};

// ❌ Forgetting to implement deserialize
struct incomplete_t {
    mla_int32_t value;
    static mla_bool_t serialize(...) { ... }
    // missing: static mla_deserializer_read_result_t deserialize(...)
};
```
