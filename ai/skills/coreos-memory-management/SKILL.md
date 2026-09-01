---
name: coreos-memory-management
description: Memory management patterns for the MLA framework. Use when allocating memory, installing memory hooks, or managing buffer ownership and reference counting in CoreOS.
metadata:
  author: coreos
  version: "1.0"
---

# Memory Management

CoreOS provides its own memory management layer that wraps platform allocators and supports hook-based interception. All memory operations must go through MLA functions — never use standard C/C++ allocation.

## Core Memory Functions

### Allocation and Deallocation

```cpp
// Allocate memory (replacement for malloc)
mla_pointer_t ptr = mla_malloc(sizeof(MyStruct));

// Free memory (replacement for free)
mla_free(ptr);

// Create typed arrays
mla_char_t* chars = mla_create_char_array(256);
mla_byte_t* bytes = mla_create_byte_array(1024);
```

### Memory Operations

```cpp
// Copy memory (replacement for memcpy)
mla_memcpy(dest, src, byteCount);

// Set memory (replacement for memset)
mla_memset(ptr, 0, byteCount);

// Compare memory (replacement for memcmp)
mla_int32_t result = mla_memcmp(ptr1, ptr2, byteCount);

// Move memory (replacement for memmove — handles overlapping regions)
mla_memmove(dest, src, byteCount);
```

### String Memory Operations

```cpp
// Copy a C string (replacement for strcpy)
mla_strcpy(dest, src);

// Get string length (replacement for strlen)
mla_size_t len = mla_strlen(cstr);

// Find substring (replacement for strstr)
const mla_char_t* found = mla_strstr(haystack, needle);
```

## Reference Counting with `mla_buffer_reference_t`

Many MLA types (strings, buffers) use reference counting for automatic memory management:

```cpp
// The mla_buffer_reference_t tracks ownership of heap-allocated data.
// When the reference count reaches zero, the buffer is freed automatically.

// Strings use this internally:
mla_string_t str = mla_string_copy(mla_string_const("Hello"));
// str.dataOwner holds the reference count

// When str goes out of scope or is destroyed, the buffer is freed
mla_string_destroy(str);
```

## User Data for Attaching Arbitrary State

`mla_user_data_t` provides a generic key-value container for attaching state to framework objects:

```cpp
#include "../system/mla_user_data.h"

mla_user_data_t userData = mla_user_data_empty();

// Store a C string (non-owning)
mla_user_data_set_cstring(userData, "url", "http://example.com");

// Store an MLA string
mla_user_data_set_string(userData, "name", mla_string_const("MyModule"));

// Store a pointer with ownership (will be freed when user_data is destroyed)
MyContext* ctx = static_cast<MyContext*>(mla_malloc(sizeof(MyContext)));
mla_user_data_set_pointer_with_ownership(userData, "ctx", ctx);

// Retrieve values
const mla_char_t* url = mla_user_data_get_cstring(userData, "url");
MyContext* retrieved = mla_user_data_get_pointer<MyContext>(userData, "ctx");
```

## Memory Hooks

The memory hook system lets you intercept all `mla_malloc` / `mla_free` calls for debugging, profiling, or custom allocation:

```cpp
#include "../memory/mla_memory_hook.h"

static mla_bool_t my_malloc_hook(mla_size_t p_Size, mla_pointer_t* p_OutPtr) {
    // Custom allocation logic
    *p_OutPtr = malloc(p_Size);
    // Track allocation for debugging
    return *p_OutPtr != nullptr;
}

static mla_bool_t my_free_hook(mla_pointer_t p_Ptr) {
    // Custom free logic
    free(p_Ptr);
    return true;
}

// Install hook
mla_memory_hook_t hook = mla_memory_hook_install(
    my_malloc_hook,
    my_free_hook
);

// All subsequent mla_malloc/mla_free calls go through hooks

// Uninstall when done
mla_memory_hook_uninstall(hook);
```

### Hook with GCC Pointer Detection

For platforms that differentiate between allocation sources:

```cpp
static MLA_MEMORY_POINTER_TYPE my_is_gcc_pointer(const mla_pointer_t p_Ptr) {
    // Return the type of pointer
    return MLA_MEMORY_POINTER_TYPE_REGULAR;
}

mla_memory_hook_t hook = mla_memory_hook_install(
    my_malloc_hook,
    my_free_hook,
    my_is_gcc_pointer  // optional third parameter
);
```

### Hook Limits

- Maximum of `CONST_MLA_MAX_MEMORY_HOOKS` (10) concurrent hooks.
- Hooks are checked in reverse installation order (last installed, first checked).
- If a hook's malloc returns `false`, the next hook (or original allocator) is tried.

## Rules

- **Never** use `new`, `delete`, `malloc`, `free`, `memcpy`, `memset`, `memmove`, or `memcmp` directly.
- **Always** use `mla_malloc`, `mla_free`, `mla_memcpy`, `mla_memset`, `mla_memmove`, `mla_memcmp`.
- **Never** include `<cstdlib>`, `<cstring>`, or `<memory>`.
- Use `mla_user_data_set_pointer_with_ownership` when the framework should free the pointer.
- Check pointer validity before dereferencing — `mla_malloc` may return `nullptr` on failure.
- Memory hooks should be installed early (e.g., during low-level boot events).

## Incorrect Usage

```cpp
// ❌ Using standard allocation
char* buf = new char[256];        // use mla_create_char_array(256)
delete[] buf;                      // use mla_free(buf)

// ❌ Using standard memory functions
memcpy(dst, src, len);            // use mla_memcpy(dst, src, len)
memset(buf, 0, len);              // use mla_memset(buf, 0, len)

// ❌ Including standard headers
#include <cstring>                 // never include this
#include <memory>                  // never include this

// ❌ Not checking allocation result
mla_pointer_t ptr = mla_malloc(huge_size);
// should check: if (ptr == nullptr) { mla_error("allocation failed"); }
```
