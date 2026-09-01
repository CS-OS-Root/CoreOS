---
name: coreos-data-types
description: Data type and coding style guidelines for the MLA framework. Use when writing any C++ code in the CoreOS project to ensure correct types, memory functions, and naming conventions are followed.
metadata:
  author: coreos
  version: "1.0"
  converted-from: ai/instructions/project_and_data_type_rules.instructions.md
---

# Data Type and Coding Style Rules

## Core Principles

This project follows strict data type and coding conventions to ensure cross-platform compatibility and consistent behavior across different compilers and architectures.

## Data Type Rules

### 1. Use Only MLA Framework Types

**NEVER** use standard C/C++ types directly. Always use the MLA framework types defined in `mla_data_types.h`:

- ❌ `int`, `long`, `short`, `char`, `unsigned int`, etc.
- ✅ `mla_int32_t`, `mla_int64_t`, `mla_uint32_t`, `mla_char_t`, etc.

### 2. Available MLA Types

**Integer Types:**
- `mla_int8_t`, `mla_uint8_t` (8-bit)
- `mla_int16_t`, `mla_uint16_t` (16-bit)
- `mla_int32_t`, `mla_uint32_t` (32-bit)
- `mla_int64_t`, `mla_uint64_t` (64-bit)

**Floating Point Types:**
- `mla_float_t` (32-bit float)
- `mla_double_t` (64-bit double)

**Character Types:**
- `mla_char_t` (standard char)
- `mla_utf_16_char_t` (UTF-16 character)
- `mla_utf_32_char_t` (UTF-32 character)

**Special Types:**
- `mla_bool_t` (boolean)
- `mla_size_t` (size type, aliased to `mla_uint32_t`)
- `mla_byte_t` (byte type, aliased to `mla_uint8_t`)
- `mla_hash_t` (hash type)
- `mla_pointer_t` (void pointer)
- `mla_void_t` (void)

### 3. Type Limits

Use MLA-defined limits instead of standard library constants:

- ❌ `INT_MAX`, `UINT_MAX`, `SIZE_MAX`
- ✅ `mla_int32_max`, `mla_uint32_max`, `mla_size_max`

Available for all integer types (e.g., `mla_int8_min`, `mla_int8_max`, `mla_uint64_max`)

## Memory Operations

### Use MLA Memory Functions

**NEVER** use standard C library functions directly:

- ❌ `memcpy`, `memset`, `memcmp`, `memmove`
- ✅ `mla_memcpy`, `mla_memset`, `mla_memcmp`, `mla_memmove`

- ❌ `malloc`, `free`
- ✅ `mla_malloc`, `mla_free`

- ❌ `strcpy`, `strlen`, `strstr`
- ✅ `mla_strcpy`, `mla_strlen`, `mla_strstr`

## Coding Style (C-Style with C++ Features)

### 1. Naming Conventions

- **Structs:** `mla_<name>_t` (e.g., `mla_string_t`, `mla_buffer_t`)
- **Functions:** `mla_<module>_<action>` (e.g., `mla_string_copy`, `mla_string_equals`)
- **Variables:** `p_<Name>` for parameters, `camelCase` for local variables
- **Constants/Macros:** `MLA_<NAME>_<CONSTANT>` or `mla_<name>_constant`

### 2. Function Signatures

Functions should follow this pattern:

```cpp
mla_return_type mla_module_function_name(const mla_type_t &p_Parameter1, mla_type_t &p_Output);
```

Key points:
- Use references (&) for struct parameters (C++ feature)
- Use const for read-only parameters
- Prefix parameters with p_
- Return types use MLA types

### 3. Struct Definitions

```cpp
struct mla_example_t {
    mla_size_t length;
    const mla_char_t *data;
    mla_bool_t isValid;
};
```

Key points:
- Use struct keyword explicitly
- Suffix with _t
- Use MLA data types for all fields
- Operator overloading is allowed (C++ feature)

### 4. Initializers

Use initializer structs for default initialization in templates:
```cpp
struct mla_string_initializer {
    static mla_string_t init() {
        return mla_string_empty();
    }
};
```

### 5. Helper Macros and Templates

Template functions are allowed for compile-time operations:
```cpp
template<mla_size_t N>
mla_string_t mla_string_const(const mla_char_t (&literal)[N]) {
    return mla_string(literal, N-1);
}
```

### Avoid Standard Library
   ❌ #include <string>, #include <vector>, #include <stdio.h>
   ✅ Use MLA framework equivalents (mla_string_t, mla_array_list_t)

### Boolean Values
   - Use mla_bool_t type
   - Return values: true or false (C++ keywords are acceptable)

### Examples
   ✅ Correct

```cpp
mla_string_t mla_string_copy(const mla_string_t &p_String) {
    mla_size_t length = mla_string_length(p_String);
    mla_char_t* buffer = mla_create_char_array(length);
    mla_memcpy(buffer, mla_string_data(p_String), length);
    return mla_string_from_buffer_with_ownership(buffer, length);
}
```

❌ Incorrect

```cpp
string copy_string(const string &str) {
    size_t len = str.length();
    char* buf = new char[len];
    memcpy(buf, str.c_str(), len);
    return string(buf);
}
```

### Summary
- Always use MLA data types from mla_data_types.h
- Never use standard C/C++ types or library functions directly
- Follow C-style naming with C++ references and templates
- Use struct keyword and _t suffix for types
- Prefix functions with mla_<module>_
- Use MLA memory operations and low-level from the framework like mla_malloc, mla_memcpy, etc.
- Never include standard library headers; use MLA equivalents instead.
- Never use new or delete; use mla_malloc and mla_free instead.
