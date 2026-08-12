---
name: 'mla-c-update'
description: 'Patterns, architecture, and rules for auto-update functionality in the MLA framework'
---

# Auto-Update Module (`mla_update`)

The Auto-Update package (`core/update/`) provides cross-platform application update checking, binary fetching, self-replacement, and restart capabilities for applications built with the `mla-c` framework.

## Core Concepts

1. **Version Identifier**: The application version is defined by the `MLA_APP_VERSION` preprocessor macro (defaults to `"snapshot"` in `mla_update_version.h`).
2. **Provider Abstraction**: Update providers query remote endpoints (e.g. HTTP servers) to check for newer versions and download update streams.
3. **Platform Self-Replacement**: Upgrade logic is separated into core provider/query abstractions and platform-specific process execution routines (`mla_global_update_linux.h`, `mla_global_update_windows.h`, `mla_global_update_disabled.h`).

---

## Architecture & File Structure

| Layer | File Path | Purpose |
|---|---|---|
| Core Version | `core/update/mla_update_version.h` | Defines default `MLA_APP_VERSION` macro |
| Core API Header | `core/update/mla_update.h` | Public API declarations, structs, error codes |
| Core API Implementation | `core/update/mla_update.cpp` | Global provider registry and HTTP provider |
| Linux Platform | `platform/linux/mla_global_update_linux.h` | Linux self-replacement process & restart |
| Windows Platform | `platform/windows/mla_global_update_windows.h` | Windows self-replacement process & restart |
| Disabled Platform | `platform/generic/mla_global_update_disabled.h` | Stub fallback for WASM / unsupported targets |
| Release Script | `create_release.sh` | Shell script for packaging release binaries |

---

## Data Types and Structures

### Result Codes (`mla_update_error_t`)

```cpp
#define MLA_UPDATE_SUCCESS 0
#define MLA_UPDATE_ERROR_INVALID_STREAM 1
#define MLA_UPDATE_ERROR_WRITE_FAILED 2
#define MLA_UPDATE_ERROR_SPAWN_FAILED 3
#define MLA_UPDATE_ERROR_NOT_SUPPORTED 4
```

### Update Provider (`mla_update_provider_t`)

Abstract provider interface for fetching version info and executable binary content:

```cpp
struct mla_update_provider_t {
    mla_user_data_t user_data;
    mla_bool_t (*get_last_version)(const mla_update_provider_t& p_Provider, mla_string_t& p_OutVersion);
    mla_bool_t (*get_binary_content)(const mla_update_provider_t& p_Provider, const mla_string_t& p_Version, mla_stream_input_t& p_OutStream);
};
```

### Platform Management (`mla_update_management_t`)

Platform-specific executable upgrade delegate:

```cpp
struct mla_update_management_t {
    mla_update_error_t (*upgrade_to_version)(mla_stream_input_t& p_BinaryStream);
};

extern mla_update_management_t g_update_management;
```

---

## Core API Usage

### 1. Get Current Version

```cpp
#include "../../core/update/mla_update.h"

mla_string_t version = mla_update_get_current_version();
```

### 2. Check for Remote Updates (HTTP Provider)

```cpp
mla_update_provider_t http_provider = mla_update_provider_http_create(mla_string_const("mla-core"), mla_string_const("https://releases.home.schlegel.ovh"));
mla_update_set_provider(http_provider);

mla_string_t latest_version = mla_string_empty();
if (mla_update_get_last_version(latest_version)) {
    if (!mla_string_equals(latest_version, mla_update_get_current_version())) {
        // New version available
    }
}
```

### 3. Apply Update Stream

```cpp
mla_stream_input_t binary_stream = mla_stream_noop_input();
if (http_provider.get_binary_content(http_provider, latest_version, binary_stream)) {
    mla_update_error_t err = mla_update_upgrade_to_version(binary_stream);
    if (err == MLA_UPDATE_SUCCESS) {
        // Upgrade process spawned; app will be replaced
    }
}
```

---

## Platform Replacement Flow (`check_and_apply_pending_update`)

The startup check function `mla_update_check_and_apply_pending_update(int argc, char** argv)` is a **platform package API** defined directly in platform headers:

```cpp
mla_bool_t mla_update_check_and_apply_pending_update(int argc, char** argv);
```

### Sequence Diagram

```
[ App Startup ] ---> Call mla_update_check_and_apply_pending_update(argc, argv)
                          |
             Is --mla-apply-update flag present?
             /                                 \
          (Yes)                               (No)
           /                                     \
 1. Copy own temp binary to target path    Return false (Normal App Boot)
 2. Chmod +x target binary (Linux/POSIX)
 3. Spawn target executable process
 4. Exit current temp process (code 0)
```

---

## Critical Rules & Guidelines

1. **Platform Scoping**: `mla_update_check_and_apply_pending_update` is platform-specific and MUST NOT be declared in `mla_update.h` or `mla_update_management_t`. It is provided directly by platform headers (`mla_global_update_linux.h`, `mla_global_update_windows.h`, `mla_global_update_disabled.h`).
2. **No Platform Header Guards for Globals**: Inside specific platform headers (`mla_global_update_linux.h` and `mla_global_update_windows.h`), do NOT wrap `g_update_management` or platform functions in extra `#if defined(...)` guards. Inclusion of the platform header implies that platform's symbols are active.
3. **Git Ignore Rules**: Release build outputs in `release/` must remain ignored by git (`.gitignore`).
