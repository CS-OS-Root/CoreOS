---
name: 'mla-c-memory-management'
description: 'Patterns for memory allocation, reference counting, and custom memory hooks in the MLA framework'
---

# Memory Management Module

The Memory module (`core/memory/`) provides reference-counted smart pointers, custom memory allocation hooks, memory pool management, and platform-level allocator abstractions.

## Strict Rules

1. **NO `new` or `delete`**: Never use C++ `new` or `delete` operators.
2. **Owning Data**: Use `mla_pointer_t` along with `mla_malloc()` or `mla_malloc_struct()` for reference-counted heap memory.
3. **Basic Allocation**: Use `mla_platform_malloc()` and `mla_platform_free()` for raw allocations where `mla_pointer_t` overhead is unnecessary.

## Key Types & Functions

| Function / Type | Header | Description |
|---|---|---|
| `mla_pointer_t` | `mla_pointer.h` | Reference-counted smart pointer wrapper |
| `mla_malloc` / `mla_malloc_struct` | `mla_memory.h` | Heap allocation producing an `mla_pointer_t` |
| `mla_platform_malloc` / `free` | `mla_global_platform.h` | Platform raw memory allocation |
| `mla_memory_hook_install` | `mla_memory_hook.h` | Install custom memory allocation tracking hooks |

## Using Reference Counted Memory

```cpp
#include "core/memory/mla_memory.h"

struct MyData {
    mla_int32_t id;
    mla_string_t name;
};

// Allocation
mla_pointer_t ptr = mla_malloc_struct<MyData>();
if (mla_pointer_is_valid(ptr)) {
    MyData* data = mla_pointer_get<MyData>(ptr);
    data->id = 42;
    data->name = mla_string_const("Test");
}

// Automatic cleanup when reference count drops to 0
```

## Installing Custom Memory Hooks

```cpp
#include "core/memory/mla_memory_hook.h"

static mla_bool_t custom_malloc_hook(mla_size_t size, mla_platform_pointer_t* out_ptr) {
    // Return false to pass down to default platform malloc
    return false;
}

static mla_bool_t custom_free_hook(mla_platform_pointer_t ptr) {
    return false;
}

// Installation
mla_memory_hook_t hook = mla_memory_hook_install(custom_malloc_hook, custom_free_hook, nullptr);

// Uninstallation
mla_memory_hook_uninstall(hook);
```
