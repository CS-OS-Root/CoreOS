---
name: coreos-hash-map-usage
description: Hash map creation, lookup, iteration, and deletion patterns for the MLA framework. Use when storing key-value data, implementing caches, or working with mla_hash_map_t in CoreOS.
metadata:
  author: coreos
  version: "1.0"
---

# Hash Map Usage

The hash map module (`core-os/system/mla_hash_map.h`) provides a template-based, open-addressing hash table with automatic rehashing. It requires a custom hasher and initializer structs for type-safe generic usage.

## Template Signature

```cpp
template <
    class TKey,
    class TValue,
    class Hasher,
    class TKeyInit = mla_default_init(TKey),
    class TValueInit = mla_default_init(TValue)
>
struct mla_hash_map_t;
```

## Common Type Aliases

```cpp
// String → String map
mla_hash_map_t<mla_string_t, mla_string_t, mla_string_hash_t, mla_string_initializer, mla_string_initializer>

// String → Int32 map
mla_hash_map_t<mla_string_t, mla_int32_t, mla_string_hash_t, mla_string_initializer>

// Int16 → Int16 map
mla_hash_map_t<mla_int16_t, mla_int16_t, mla_int16_hash_t>
```

## Creating a Hash Map

```cpp
// Create with default bucket count (8) and load factor (0.75)
auto map = mla_hash_map<mla_string_t, mla_int32_t, mla_string_hash_t>();

// Create with custom bucket count
auto map = mla_hash_map<mla_string_t, mla_int32_t, mla_string_hash_t>(64);

// Create with custom bucket count and load factor
auto map = mla_hash_map<mla_string_t, mla_int32_t, mla_string_hash_t>(64, 0.5f);

// Empty sentinel
auto empty = mla_hash_map_empty<mla_string_t, mla_int32_t, mla_string_hash_t>();
```

## Inserting Values

```cpp
auto map = mla_hash_map<mla_string_t, mla_int32_t, mla_string_hash_t>(16);

// Insert or replace (default: allowReplace = true)
mla_hash_map_push_result result = mla_hash_map_push(
    map,
    mla_string_const("age"),
    42
);

switch (result) {
    case MLA_HASH_MAP_PUSH_ADDED:
        mla_info("New key added");
        break;
    case MLA_HASH_MAP_PUSH_REPLACED:
        mla_info("Existing key updated");
        break;
    case MLA_HASH_MAP_PUSH_ERROR:
        mla_error("Push failed");
        break;
}

// Insert without replacing existing
mla_hash_map_push(map, mla_string_const("age"), 99, false);
// age stays 42 because allowReplace is false
```

## Looking Up Values

### Copy Lookup

Returns a copy of the value. Safe but involves copying:

```cpp
mla_int32_t age = 0;
if (mla_hash_map_get(map, mla_string_const("age"), age)) {
    // age == 42
} else {
    mla_warning("Key not found");
}
```

### Reference Lookup

Returns a pointer to the value in the map. Allows in-place modification:

```cpp
mla_int32_t* agePtr = mla_hash_map_get_ref(map, mla_string_const("age"));
if (agePtr != nullptr) {
    *agePtr = 43;  // modify value in-place
}
```

### Containment Check

```cpp
if (mla_hash_map_contains(map, mla_string_const("age"))) {
    // key exists
}
```

## Removing Values

```cpp
if (mla_hash_map_remove(map, mla_string_const("age"))) {
    mla_info("Key removed");
} else {
    mla_warning("Key not found");
}
```

## Size and Clear

```cpp
mla_size_t count = mla_hash_map_size(map);

// Remove all entries
mla_hash_map_clear(map);
```

## Iteration

### Iterate Keys

```cpp
auto keys = mla_hash_map_keys(map);
for (mla_size_t i = 0; i < mla_array_list_size(keys); i++) {
    auto& key = mla_array_list_get_unsafe(keys, i);
    mla_info(mla_string_data(key));
}
```

### Iterate Values

```cpp
auto values = mla_hash_map_values(map);
for (mla_size_t i = 0; i < mla_array_list_size(values); i++) {
    auto& value = mla_array_list_get_unsafe(values, i);
    // process value
}
```

## Custom Hasher

Every key type needs a hasher struct with a static `hash` function:

```cpp
struct mla_my_key_hash_t {
    static mla_hash_t hash(const mla_my_key_t& p_Key) {
        // Compute hash from key fields
        return some_hash_value;
    }
};
```

### Built-In Hashers

| Hasher | Key Type |
|---|---|
| `mla_string_hash_t` | `mla_string_t` |
| `mla_int8_hash_t` | `mla_int8_t` |
| `mla_int16_hash_t` | `mla_int16_t` |
| `mla_int32_hash_t` | `mla_int32_t` |
| `mla_uint8_hash_t` | `mla_uint8_t` |
| `mla_uint16_hash_t` | `mla_uint16_t` |
| `mla_uint32_hash_t` | `mla_uint32_t` |

## Initializer Structs

Both key and value types need initializer structs for the template:

```cpp
struct mla_string_initializer {
    static mla_string_t init() {
        return mla_string_empty();
    }
};
```

For primitive types, the default initializer (`mla_default_init`) is used automatically.

## Complete Example

```cpp
#include "../system/mla_hash_map.h"
#include "../system/mla_string.h"

void example() {
    // Create a string → string map
    auto config = mla_hash_map<
        mla_string_t, mla_string_t,
        mla_string_hash_t,
        mla_string_initializer,
        mla_string_initializer
    >(16);

    // Populate
    mla_hash_map_push(config, mla_string_const("host"), mla_string_const("localhost"));
    mla_hash_map_push(config, mla_string_const("port"), mla_string_const("8080"));
    mla_hash_map_push(config, mla_string_const("debug"), mla_string_const("true"));

    // Lookup
    mla_string_t host = mla_string_empty();
    if (mla_hash_map_get(config, mla_string_const("host"), host)) {
        mla_info(mla_string_data(host));
    }

    // Iterate all entries
    auto keys = mla_hash_map_keys(config);
    for (mla_size_t i = 0; i < mla_array_list_size(keys); i++) {
        auto& key = mla_array_list_get_unsafe(keys, i);
        mla_string_t val = mla_string_empty();
        mla_hash_map_get(config, key, val);
    }

    // Update
    mla_hash_map_push(config, mla_string_const("port"), mla_string_const("9090"));

    // Remove
    mla_hash_map_remove(config, mla_string_const("debug"));

    // Clear all
    mla_hash_map_clear(config);
}
```

## Rules

- Every key type must have a corresponding hasher struct.
- Both key and value types need initializer structs (or use `mla_default_init` for primitives).
- `mla_hash_map_get` returns a **copy** — modifications to the returned value do not affect the map.
- `mla_hash_map_get_ref` returns a **pointer** — modifications affect the map directly.
- The map automatically rehashes when the load factor threshold is exceeded.
- Default load factor is 0.75 (rehash when 75% full).
- Default bucket count is 8; use a larger initial count if you know the approximate size.
- Never use `std::map`, `std::unordered_map`, or any standard library container.

## Incorrect Usage

```cpp
// ❌ Using std::unordered_map
std::unordered_map<std::string, int> map;  // use mla_hash_map_t

// ❌ Modifying a copy from mla_hash_map_get
mla_int32_t value;
mla_hash_map_get(map, key, value);
value = 99;  // does NOT update the map! use mla_hash_map_get_ref instead

// ❌ Using get_ref pointer after map modification
auto* ptr = mla_hash_map_get_ref(map, key);
mla_hash_map_push(map, otherKey, otherValue);  // may trigger rehash
*ptr = 42;  // DANGEROUS — ptr may be invalidated by rehash!

// ❌ Missing hasher struct
mla_hash_map_t<MyKey, MyValue> map;  // won't compile — needs Hasher template parameter
```
