---
name: 'mla-c-hash-maps'
description: 'Patterns for using mla_hash_map_t in the MLA framework'
---

# Hash Maps

The hash map module (`core/system/mla_hash_map.h`) provides a template-based key-value store with automatic rehashing. It is the standard associative container in the MLA framework — **never** use `std::map`, `std::unordered_map`, or similar standard containers.

## Template Parameters

```cpp
#define mla_hash_map_template_full class TKey, class TKeyInit, class Hasher, class TValue, class TValueInit = mla_default_init(TValue)
#define mla_hash_map_template class TKey, class TKeyInit, class TValue, class TValueInit = mla_default_init(TValue)

#define mla_hash_map_t_param TKey, TKeyInit, TValue, TValueInit
#define mla_hash_map_t_param_full TKey, TKeyInit, Hasher, TValue, TValueInit
```

| Parameter | Purpose | Default |
|---|---|---|
| `TKey` | Key type | (required) |
| `TKeyInit` | Key initializer parameter | `mla_init_struct(TKey)` / `mla_init_data(TKey)` |
| `Hasher` | Struct with `static mla_size_t hash(const TKey&)` | (required in full template) |
| `TValue` | Value type | (required) |
| `TValueInit` | Value initializer parameter | `mla_init_struct(TValue)` / `mla_init_data(TValue)` |

## Built-In Hashers

| Hasher | Key Type |
|---|---|
| `mla_string_hash_t` | `mla_string_t` |
| `mla_int32_hash_t` | `mla_int32_t` / `short` (32-bit) |
| `mla_int16_hash_t` | `mla_int16_t` / `short` (16-bit) |

## Creating a Hash Map

```cpp
#include "../system/mla_hash_map.h"

// String → int32 map with 16 initial buckets
mla_hash_map_t<mla_init_struct(mla_string_t),
               mla_string_hash_t,
               mla_init_data(mla_int32_t)> map =
    mla_hash_map<mla_init_struct(mla_string_t),
                 mla_string_hash_t,
                 mla_init_data(mla_int32_t)>(16);

// Empty (sentinel) map
auto emptyMap = mla_hash_map_empty<mla_init_struct(mla_string_t),
                                    mla_string_hash_t,
                                    mla_init_data(mla_int32_t)>();
```

### Defaults

| Constant | Value | Purpose |
|---|---|---|
| `mla_global_config_hash_map_default_bucket_size` | 8 | Initial number of buckets |
| `mla_global_config_hash_map_default_load_factor` | 0.75 | Rehash threshold |

## Inserting (Push)

```cpp
mla_hash_map_push_result result = mla_hash_map_push(map, mla_string_const("key"), 42);
// Returns MLA_HASH_MAP_PUSH_ADDED on first insert

result = mla_hash_map_push(map, mla_string_const("key"), 100);
// Returns MLA_HASH_MAP_PUSH_REPLACED (key existed, value updated)

// In test code, always verify that the operation did not result in an error:
assert_not_equal(mla_hash_map_push(map, key, value), MLA_HASH_MAP_PUSH_ERROR, "Failed to push to map");
```

Push returns one of:

| Result | Meaning |
|---|---|
| `MLA_HASH_MAP_PUSH_ADDED` | New key was inserted |
| `MLA_HASH_MAP_PUSH_REPLACED` | Existing key's value was updated |
| `MLA_HASH_MAP_PUSH_ERROR` | Operation failed |

To prevent replacement:

```cpp
mla_hash_map_push(map, key, value, false); // allowReplace = false
```

## Retrieving Values

### By Copy

```cpp
mla_int32_t value = 0;
if (mla_hash_map_get(map, mla_string_const("key"), value)) {
    // value is now 42
} else {
    // key not found — value is unchanged
}
```

### By Reference (Zero-Copy)

```cpp
mla_int32_t* ref = mla_hash_map_get_ref(map, mla_string_const("key"));
if (ref != nullptr) {
    *ref = 99; // modify in-place
}
```

## Checking for a Key

```cpp
if (mla_hash_map_contains(map, mla_string_const("key"))) {
    // key exists
}
```

## Removing a Key

```cpp
mla_bool_t removed = mla_hash_map_remove(map, mla_string_const("key"));
```

## Iteration

```cpp
auto keys = mla_hash_map_keys(map);
for (mla_size_t i = 0; i < mla_array_list_size(keys); i++) {
    mla_string_t& key = mla_array_list_get_unsafe(keys, i);
    mla_int32_t value = 0;
    mla_hash_map_get(map, key, value);
    // … process key / value …
}
```

Or iterate values directly:

```cpp
auto values = mla_hash_map_values(map);
for (mla_size_t i = 0; i < mla_array_list_size(values); i++) {
    mla_int32_t& val = mla_array_list_get_unsafe(values, i);
    // …
}
```

## Size and Clear

```cpp
mla_size_t count = mla_hash_map_size(map);

mla_hash_map_clear(map); // removes all entries
```

## Common Configurations

### String Keys, String Values

```cpp
mla_hash_map_t<mla_string_t, mla_string_t,
               mla_string_hash_t,
               mla_string_initializer,
               mla_string_initializer> headers =
    mla_hash_map<mla_string_t, mla_string_t,
                 mla_string_hash_t,
                 mla_string_initializer,
                 mla_string_initializer>(8);
```

### Integer Keys

```cpp
mla_hash_map_t<mla_int32_t, mla_string_t,
               mla_int32_hash_t> idToName =
    mla_hash_map<mla_int32_t, mla_string_t,
                 mla_int32_hash_t>(32);
```

## Rules

- **Never** use `std::map`, `std::unordered_map`, or any standard library container.
- The `Hasher` struct must provide a `static mla_size_t hash(const TKey&)` function.
- Keys are compared using `operator==`; ensure the key type has a correct equality operator.
- The map **rehashes automatically** when `size / bucketCount > loadFactor`.
- Do not store pointers returned by `mla_hash_map_get_ref` across insert/remove operations — rehashing invalidates them.
- Always use `mla_string_initializer` for `TKeyInit` / `TValueInit` when the key or value type is `mla_string_t`.

## Incorrect Usage

```cpp
// ❌ Using std::unordered_map
std::unordered_map<std::string, int> m; // use mla_hash_map_t instead

// ❌ Dereferencing get_ref without null check
mla_int32_t* ref = mla_hash_map_get_ref(map, key);
*ref = 10; // crash if key does not exist

// ❌ Storing a get_ref pointer across mutations
mla_int32_t* ref = mla_hash_map_get_ref(map, key1);
mla_hash_map_push(map, key2, value2); // may trigger rehash
*ref = 42; // dangling pointer — undefined behaviour

// ❌ Forgetting the hasher template parameter
mla_hash_map_t<mla_string_t, mla_int32_t> map; // won't compile — missing Hasher
```

