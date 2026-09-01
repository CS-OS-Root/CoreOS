---
name: coreos-string-operations
description: String creation, manipulation, and conversion patterns for the MLA framework. Use when working with mla_string_t, performing string comparisons, conversions, splitting, or UTF-8/UTF-16 encoding in CoreOS.
metadata:
  author: coreos
  version: "1.0"
---

# String Operations

The string module (`core-os/system/mla_string.h`) provides a feature-rich, UTF-8-native string type with small string optimization (SSO), reference counting, and comprehensive manipulation functions.

## String Type Overview

`mla_string_t` uses a union-based layout:
- **Embedded (SSO):** Strings ≤ 14 bytes are stored inline with zero heap allocation.
- **Heap:** Longer strings use reference-counted heap buffers.
- **Sub-string:** Zero-copy views into other strings.

## Creating Strings

### From Compile-Time Literals (Preferred)

```cpp
// Compile-time constant — no allocation, no copy
mla_string_t greeting = mla_string_const("Hello, World!");
```

### From C Strings

```cpp
// Non-owning wrapper around C string data
mla_string_t str1 = mla_string("Hello", 5);
mla_string_t str2 = mla_string("Hello");  // auto-detect length

// Owning copy of C string data
mla_string_t str3 = mla_string_copy("Hello", 5);
```

### From Another mla_string_t

```cpp
mla_string_t original = mla_string_const("Hello");
mla_string_t copy = mla_string_copy(original);  // deep copy
```

### Empty and Invalid

```cpp
mla_string_t empty = mla_string_empty();
mla_bool_t isEmpty = mla_string_is_empty(empty);  // true
```

### From Numeric Types

```cpp
mla_string_t intStr    = mla_string_from_int32(42);
mla_string_t int64Str  = mla_string_from_int64(123456789LL);
mla_string_t uintStr   = mla_string_from_uint32(255);
mla_string_t floatStr  = mla_string_from_float(3.14f, 2);    // "3.14"
mla_string_t doubleStr = mla_string_from_double(2.71828, 4);  // "2.7183"
mla_string_t boolStr   = mla_string_from_bool(true);          // "true"
mla_string_t hexStr    = mla_string_from_uint8_hex(0xFF);     // "0xFF"
```

### From Buffer with Ownership

```cpp
mla_char_t* buf = mla_create_char_array(64);
mla_memcpy(buf, "dynamic", 7);

// Transfer ownership — buffer will be freed when string is destroyed
mla_string_t owned = mla_string_from_buffer_with_ownership(buf, 7);
```

## String Properties

```cpp
mla_string_t str = mla_string_const("Hello");

mla_size_t length = mla_string_length(str);           // 5 (byte length)
const mla_char_t* data = mla_string_data(str);        // raw pointer
mla_bool_t empty = mla_string_is_empty(str);           // false
mla_string_memory_layout_t layout = mla_string_get_memory_layout(str);
```

## Comparison

```cpp
mla_string_t a = mla_string_const("hello");
mla_string_t b = mla_string_const("Hello");

// Exact comparison
mla_bool_t equal = mla_string_equals(a, a);                    // true
mla_int32_t cmp = mla_string_compare(a, b);                    // != 0

// Case-insensitive
mla_bool_t equalCI = mla_string_equals_ignore_case(a, b);      // true
mla_int32_t cmpCI = mla_string_compare_ignore_case(a, b);      // 0

// Operator overloads
if (a == b) { /* exact match */ }
if (a != b) { /* different */ }
```

## Search and Contains

```cpp
mla_string_t text = mla_string_const("Hello, World!");

mla_bool_t has = mla_string_contains(text, mla_string_const("World"));           // true
mla_bool_t hasCI = mla_string_contains_ignore_case(text, mla_string_const("world")); // true

mla_int32_t idx = mla_string_index_of(text, mla_string_const("World"));          // 7
mla_int32_t last = mla_string_last_index_of(text, mla_string_const("l"));        // 10

mla_bool_t starts = mla_string_starts_with(text, mla_string_const("Hello"));     // true
mla_bool_t ends = mla_string_ends_with(text, mla_string_const("!"));             // true
```

## Manipulation

### Substring

```cpp
mla_string_t str = mla_string_const("Hello, World!");
mla_string_t sub1 = mla_string_substr(str, 7, 5);   // "World"
mla_string_t sub2 = mla_string_substr(str, 7);       // "World!" (to end)
```

### Replace

```cpp
mla_string_t original = mla_string_const("Hello World");
mla_string_t result = mla_string_replace(
    original,
    mla_string_const("World"),
    mla_string_const("CoreOS")
);
// result: "Hello CoreOS"
```

### Split

```cpp
mla_string_t csv = mla_string_const("a,b,c,d");
auto parts = mla_string_split(csv, mla_string_const(","));
// parts[0] = "a", parts[1] = "b", parts[2] = "c", parts[3] = "d"
```

### Trim

```cpp
mla_string_t padded = mla_string_const("  hello  ");
mla_string_t trimmed = mla_string_trim(padded);  // "hello"
```

### Repeat

```cpp
mla_string_t dash = mla_string_const("-");
mla_string_t line = mla_string_repeat(dash, 40);  // "----------------------------------------"
```

### Case Conversion

```cpp
mla_string_t lower = mla_string_to_lower(mla_string_const("HELLO"));  // "hello"
mla_string_t upper = mla_string_to_upper(mla_string_const("hello"));  // "HELLO"
```

## C String Conversion

```cpp
mla_string_t str = mla_string_const("Hello");

// Get a null-terminated C string (may need allocation)
mla_c_string_t cstr = mla_string_to_cString(str);
const mla_char_t* raw = cstr.c_str;

// Clean up if it was allocated
mla_destroy_c_string(cstr);
```

## UTF-8 / UTF-16 / UTF-32

```cpp
mla_string_t emoji = mla_string_const("Hello 🌍");

// Multi-byte character count (different from byte length for non-ASCII)
mla_size_t charCount = mla_string_multi_byte_char_count(emoji);

// Get character at index (multi-byte aware)
mla_multi_byte_char_t ch = mla_string_multi_byte_char_at(emoji, 6);

// Convert to UTF-16
mla_string_utf16_buffer_t utf16 = mla_string_to_utf16_buffer(emoji);
// ... use utf16.data and utf16.charCount ...
mla_string_utf16_buffer_destroy(utf16);

// Convert to UTF-32
mla_string_utf32_buffer_t utf32 = mla_string_to_utf32_buffer(emoji);
// ... use utf32.data and utf32.charCount ...
mla_string_utf32_buffer_destroy(utf32);

// Convert back from UTF-16
mla_string_t fromUtf16 = mla_string_from_utf16_buffer(utf16);
```

## Memory Layout Control

```cpp
// Change the internal memory layout
mla_string_t str = mla_string_const("Hello");
mla_string_change_memory_layout(str, MLA_STRING_MEMORY_LAYOUT_C_STRING);
```

Available layouts:
- `MLA_STRING_MEMORY_LAYOUT_EMBEDDED` — SSO (≤ 14 bytes)
- `MLA_STRING_MEMORY_LAYOUT_C_STRING` — null-terminated heap string
- `MLA_STRING_MEMORY_LAYOUT_BUFFER` — heap buffer without null terminator
- `MLA_STRING_MEMORY_LAYOUT_SUB_STRING` — view into another string

## Cleanup

```cpp
// Destroy a string (releases reference count / frees buffer if owned)
mla_string_t str = mla_string_copy(mla_string_const("Hello"));
mla_string_destroy(str);
```

> **Note:** `mla_string_const()` strings do not need destruction as they reference static data.

## Rules

- Always use `mla_string_const()` for string literals — it avoids allocation.
- Use `mla_string_copy()` when you need a string to outlive the source data.
- Call `mla_string_destroy()` on strings created via `mla_string_copy` or `mla_string_from_buffer_with_ownership`.
- Never use `std::string`, `char*` arithmetic, `strlen`, `strcmp`, etc. — use MLA equivalents.
- String byte length and character count differ for multi-byte UTF-8 strings.

## Incorrect Usage

```cpp
// ❌ Using std::string
std::string name = "hello";  // use mla_string_t

// ❌ Using strlen
size_t len = strlen(cstr);   // use mla_strlen(cstr) or mla_string_length(str)

// ❌ Forgetting to destroy copied strings
mla_string_t copy = mla_string_copy(original);
// missing: mla_string_destroy(copy);

// ❌ Accessing data pointer after string is destroyed
mla_string_destroy(str);
const mla_char_t* data = mla_string_data(str);  // dangling pointer!
```
