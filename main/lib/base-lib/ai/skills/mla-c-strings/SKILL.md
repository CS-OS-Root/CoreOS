---
name: 'mla-c-strings'
description: 'Patterns for using mla_string_t in the MLA framework'
---

# Strings

The string module (`core/system/mla_string.h`) is the primary text type in the MLA framework. `mla_string_t` supports UTF-8/16/32, small-string optimization (SSO up to 14 bytes), and reference-counted heap storage. **Never** use `std::string`, `char*` arithmetic, or standard library string functions directly.

## Key Types

| Type | Purpose |
|---|---|
| `mla_string_t` | Primary string type (SSO or heap-backed) |
| `mla_c_string_t` | Null-terminated C string wrapper (may or may not own its buffer) |
| `mla_multi_byte_char_t` | A single UTF-8 character (1–4 bytes, null-terminated) |
| `mla_string_utf16_buffer_t` | Owned UTF-16 buffer for interop |
| `mla_string_utf32_buffer_t` | Owned UTF-32 buffer for interop |
| `mla_string_initializer` | Initializer for use in `mla_array_list_t` / `mla_hash_map_t` |
| `mla_string_hash_t` | Hash functor for use as a `mla_hash_map_t` key hasher |

## Memory Layouts

`mla_string_t` uses a tagged union internally. The layout is chosen automatically:

| Layout | When Used |
|---|---|
| `MLA_STRING_MEMORY_LAYOUT_EMBEDDED` | String ≤ 14 bytes (SSO — no allocation) |
| `MLA_STRING_MEMORY_LAYOUT_C_STRING` | Null-terminated heap buffer |
| `MLA_STRING_MEMORY_LAYOUT_BUFFER` | Heap buffer without null terminator |
| `MLA_STRING_MEMORY_LAYOUT_SUB_STRING` | View into another string (no copy) |

## Creating Strings

### Compile-Time Constant (Preferred for Literals)

```cpp
mla_string_t greeting = mla_string_const("Hello, World!");
```

`mla_string_const` is a template that captures the literal length at compile time and avoids runtime `strlen`.

### From a Buffer

```cpp
const mla_char_t* data = getExternalData();
mla_size_t len = getExternalDataLength();

// Non-owning — caller must keep data alive
mla_string_t view = mla_string(data, len);

// Owning copy — safe after data is freed
mla_string_t owned = mla_string_copy(data, len);
```

### From Another String

```cpp
mla_string_t copy = mla_string_copy(original);
```

### Copy from a Raw Buffer

```cpp
mla_char_t* buf = (mla_char_t*)mla_platform_malloc(64);
// … fill buf …
mla_string_t str = mla_string_copy(buf, actualLength); // makes an owned copy
mla_platform_free(buf); // caller is still responsible for freeing the raw buffer
```

### Wrap an Existing Managed Buffer (No Copy)

```cpp
// Allocate a managed buffer and fill it
mla_pointer_t ptr = mla_malloc_buffer(64);
mla_char_t* raw = mla_pointer_get_data<mla_char_t>(ptr);
// … fill raw …
// Wrap the managed pointer directly — ownership is shared with str
mla_string_t str = mla_string(ptr, actualLength);
```

### Empty / Sentinel

```cpp
mla_string_t empty = mla_string_empty();
mla_bool_t isEmpty = mla_string_is_empty(empty); // true
```

## Destroying Strings

Strings are getting destroyed when they go out of scope, but you can also assign an empty string to release resources immediately:

```cpp
str = mla_string_empty();
```

## Basic Operations

### Length and Data Access

```cpp
mla_size_t len       = mla_string_length(str);
const mla_char_t* p  = mla_string_data(str);
```

### Comparison and Equality

```cpp
mla_bool_t same = mla_string_equals(a, b);
mla_bool_t sameIgnoreCase = mla_string_equals_ignore_case(a, b);

// Compare with a compile-time literal (macro, avoids temporary)
if (mla_string_equals_const(method, "GET")) { /* … */ }

// Lexicographic compare (returns negative / zero / positive)
mla_int32_t cmp = mla_string_compare(a, b);
mla_int32_t cmpI = mla_string_compare_ignore_case(a, b);
```

## Search and Matching

```cpp
mla_bool_t found = mla_string_contains(haystack, needle);
mla_bool_t foundI = mla_string_contains_ignore_case(haystack, needle);

mla_int32_t idx  = mla_string_index_of(str, sub);       // -1 if not found
mla_int32_t last = mla_string_last_index_of(str, sub);   // -1 if not found

mla_bool_t prefix = mla_string_starts_with(str, mla_string_const("http"));
mla_bool_t suffix = mla_string_ends_with(str, mla_string_const(".json"));
// Case-insensitive variants also available
```

## String Manipulation

All manipulation functions return a **new** `mla_string_t`. Destroy the result when done.

```cpp
mla_string_t upper   = mla_string_to_upper(str);
mla_string_t lower   = mla_string_to_lower(str);
mla_string_t trimmed = mla_string_trim(str);
mla_string_t sub     = mla_string_substr(str, startIndex, length);
mla_string_t sub2    = mla_string_substr(str, startIndex); // to end
mla_string_t rep     = mla_string_repeat(mla_string_const("-"), 40);
mla_string_t replaced = mla_string_replace(str,
    mla_string_const("old"),
    mla_string_const("new"));
```

### Splitting

```cpp
mla_array_list_t<mla_init_struct(mla_string_t)> parts =
    mla_string_split(csv, mla_string_const(","));

for (mla_size_t i = 0; i < mla_array_list_size(parts); i++) {
    mla_string_t& part = mla_array_list_get_unsafe(parts, i);
    // … process part …
}
```

## Numeric Conversions

### From Primitives to String

```cpp
mla_string_t s1 = mla_string_from_int32(42);
mla_string_t s2 = mla_string_from_uint64(100000ULL);
mla_string_t s3 = mla_string_from_float(3.14f, 2);   // "3.14" — 2 decimal places
mla_string_t s4 = mla_string_from_double(2.718, 3);   // "2.718"
mla_string_t s5 = mla_string_from_bool(true);          // "true"
```

### Hexadecimal

```cpp
mla_string_t hex  = mla_string_from_uint32_hex(0xDEAD);       // "0x0000DEAD"
mla_string_t hexS = mla_string_from_uint32_hex_short(0xDEAD); // "DEAD"
```

## UTF Conversion

```cpp
// UTF-8 → UTF-16
mla_string_utf16_buffer_t utf16 = mla_string_to_utf16_buffer(str);
const mla_utf_16_char_t* wchars = mla_string_utf16_data(utf16); // length: utf16.charCount
// … use wchars …
// utf16 is automatically cleaned up when it goes out of scope (holds mla_pointer_t internally)

// UTF-16 → UTF-8
mla_string_t back = mla_string_from_utf16_buffer(utf16);

// Individual multibyte character access
mla_size_t charCount = mla_string_multi_byte_char_count(str);
mla_multi_byte_char_t ch = mla_string_multi_byte_char_at(str, 0);
```

## C String Interop

```cpp
mla_c_string_t cstr = mla_string_to_cString(str);
const mla_char_t* raw = mla_c_string_data(cstr); // null-terminated
printf_native("%s", raw); // platform logging only
// cstr is automatically cleaned up when it goes out of scope — no explicit destroy needed
```

## Using Strings as Hash Map Keys

```cpp
mla_hash_map_t<mla_string_t, mla_int32_t,
               mla_string_hash_t,
               mla_string_initializer> map =
    mla_hash_map<mla_string_t, mla_int32_t,
                 mla_string_hash_t,
                 mla_string_initializer>(16);

mla_hash_map_push(map, mla_string_const("key"), 42);
```

## Rules

- **Never** use `std::string`, `char*` arithmetic, `strlen`, `strcmp`, `strcpy`, etc.
- Always use `mla_string_const("…")` for compile-time string literals.
- Use `mla_string_equals_const(value, "literal")` for quick comparisons against literals.
- String well be destroyed automatically when going out of scope
- Do not access `mla_string_data(str)` beyond `mla_string_length(str)` — the buffer may not be null-terminated.
- When passing strings to C APIs, convert with `mla_string_to_cString` and obtain the raw data via `mla_c_string_data`.

## Incorrect Usage

```cpp
// ❌ Using std::string
std::string name = "hello";

// ❌ Using strlen / strcmp
size_t len = strlen(mla_string_data(str));
int cmp = strcmp(mla_string_data(a), mla_string_data(b));

// ❌ Assuming null-termination
const char* raw = mla_string_data(str);
printf("%s", raw); // may crash — use mla_string_to_cString instead
```

