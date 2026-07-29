---
name: 'mla-c-array-lists'
description: 'Patterns for using mla_array_list_t in the MLA framework'
---

# Array Lists

The array list module (`core/system/mla_array_list.h`) provides a generic, dynamically-resizing array. It is the standard ordered collection in the MLA framework — **never** use `std::vector`, raw arrays with `new[]`, or other standard containers.

## Template Parameters

```cpp
mla_array_list_t<T, TInit>
```

| Parameter | Purpose | Default |
|---|---|---|
| `T` | Element type | (required) |
| `TInit` | Initializer struct (provides `static T init()`) | `mla_default_init(T)` |

The initializer is called when elements are removed or the list is cleared, to reset slots to a clean state.

## Creating an Array List

```cpp
#include "../system/mla_array_list.h"

// Default capacity (8)
mla_array_list_t<mla_int32_t> numbers = mla_array_list<mla_int32_t>();

// Custom initial capacity
mla_array_list_t<mla_string_t, mla_string_initializer> names =
    mla_array_list<mla_string_t, mla_string_initializer>(64);

// Empty / sentinel value
auto empty = mla_array_list_empty<mla_int32_t>();
```

## Adding Elements

```cpp
mla_array_list_add(numbers, 42);
mla_array_list_add(numbers, 17);
mla_array_list_add(numbers, 99);
// numbers: [42, 17, 99], size = 3
```

The list automatically doubles its capacity when full. **In test code, you must always verify that `mla_array_list_add` returns `true`**:

```cpp
assert_true(mla_array_list_add(numbers, 42), "Failed to add to list");
```

### Adding Multiple Elements

```cpp
mla_array_list_t<mla_int32_t> more = mla_array_list<mla_int32_t>(4);
mla_array_list_add(more, 1);
mla_array_list_add(more, 2);

mla_array_list_add_all(numbers, more);
// numbers: [42, 17, 99, 1, 2]
```

## Accessing Elements

### Safe Access (Bounds-Checked)

```cpp
mla_int32_t value = 0;
if (mla_array_list_get(numbers, 0, value)) {
    // value == 42
} else {
    // index out of bounds
}
```

### Reference Access (Bounds-Checked)

```cpp
mla_int32_t* ref = mla_array_list_get_ref(numbers, 0);
if (ref != nullptr) {
    *ref = 100; // modify in-place
}
```

### Unsafe Access (No Bounds Check)

Use only when the index is already validated:

```cpp
mla_int32_t& val = mla_array_list_get_unsafe(numbers, 0);
mla_int32_t* ptr = mla_array_list_get_ref_unsafe(numbers, 0);
```

## Removing Elements

```cpp
mla_array_list_remove(numbers, 1); // removes element at index 1
// numbers: [42, 99, 1, 2] — remaining elements shift left
```

## Size and Capacity

```cpp
mla_size_t count = mla_array_list_size(numbers);
mla_size_t cap   = mla_array_list_capacity(numbers);
```

### Shrink to Fit

Reduce capacity to match the current size:

```cpp
mla_array_list_shrink_to_fit(numbers);
```

### Manual Resize

```cpp
mla_array_list_resize(numbers, 128); // ensure capacity for 128 elements
```

## Searching

### Contains

```cpp
if (mla_array_list_contains(numbers, 42)) {
    // found
}
```

### Index Of

```cpp
mla_int32_t idx = mla_array_list_index_of(numbers, 42);
if (idx >= 0) {
    // found at position idx
} else {
    // not found (returns -1)
}
```

Both use `operator==` on the element type.

## Sorting (Quicksort)

```cpp
mla_int32_t compare_ints(const mla_int32_t& a, const mla_int32_t& b) {
    return a - b; // negative if a < b, 0 if equal, positive if a > b
}

mla_array_list_sort(numbers, compare_ints);
```

The compare function must return:
- Negative if `a < b`
- Zero if `a == b`
- Positive if `a > b`

## Reversing

```cpp
mla_array_list_reverse(numbers);
```

## Clearing and Destroying

```cpp
mla_array_list_clear(numbers);   // resets size to 0, calls TInit::init() on each slot
mla_array_list_destroy(numbers); // releases the underlying buffer
```

## Iteration Patterns

### Forward Iteration

```cpp
for (mla_size_t i = 0; i < mla_array_list_size(list); i++) {
    auto& item = mla_array_list_get_unsafe(list, i);
    // … process item …
}
```

### With Strings

```cpp
mla_array_list_t<mla_init_struct(mla_string_t)> tags =
    mla_array_list<mla_init_struct(mla_string_t)>(4);

mla_array_list_add(tags, mla_string_const("alpha"));
mla_array_list_add(tags, mla_string_const("beta"));

for (mla_size_t i = 0; i < mla_array_list_size(tags); i++) {
    mla_string_t& tag = mla_array_list_get_unsafe(tags, i);
    mla_info(tag);
}
```

## Using as a Stack

```cpp
// Push
mla_array_list_add(stack, item);

// Pop (remove last)
mla_size_t last = mla_array_list_size(stack) - 1;
auto& top = mla_array_list_get_unsafe(stack, last);
// … use top …
mla_array_list_remove(stack, last);
```

## Initializer for Containers

Static `init()` is defined directly inside target structs:

```cpp
struct my_struct_t {
    mla_string_t name;
    static my_struct_t init() {
        return { mla_string_empty() };
    }
};
```

## Rules

- **Never** use `std::vector`, `new[]`, or C arrays for dynamic collections.
- Use `mla_init_struct(T)` as container template parameter for structs with static `init()` method.
- Use `mla_init_data(T)` for primitive data types and POD structs without static `init()`.
- Static `init()` function is called when elements are removed or the list is cleared — it acts as a reset/cleanup for the element slot.
- Do **not** modify the list (add/remove) while iterating by index — indices shift on removal.
- Pointers from `mla_array_list_get_ref` are invalidated when the list resizes (add triggers realloc if capacity is exceeded).
- Use `mla_array_list_get_unsafe` only when the index has been validated.

## Incorrect Usage

```cpp
// ❌ Using std::vector
std::vector<int> items; // use mla_array_list_t<mla_int32_t>

// ❌ Accessing out of bounds without checking
auto& item = mla_array_list_get_unsafe(list, 999); // undefined if size < 1000

// ❌ Removing while iterating forward
for (mla_size_t i = 0; i < mla_array_list_size(list); i++) {
    if (shouldRemove(mla_array_list_get_unsafe(list, i))) {
        mla_array_list_remove(list, i); // shifts elements — next iteration skips one
    }
}

// ❌ Storing get_ref pointers across add operations
mla_int32_t* ref = mla_array_list_get_ref(list, 0);
mla_array_list_add(list, newItem); // may realloc — ref is dangling
*ref = 42; // undefined behaviour
```

