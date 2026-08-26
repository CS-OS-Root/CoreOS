---
name: 'mla-c-service'
description: 'Patterns, architecture, and rules for daemon and system service management in the MLA framework'
---

# System Service & Daemon Management Module (`mla_service`)

The Service module (`core/service/`) provides a cross-platform abstraction to install and uninstall the currently running application binary as an operating system background service / daemon.

## Core Concepts

1. **Self-Targeting Binary Registration**: The module automatically resolves the filesystem path of the currently executing application binary (`/proc/self/exe` on Linux, `GetModuleFileNameA` on Windows) and configures it as the service payload executable.
2. **Platform Init System Integration**:
   - **Linux (`systemd`)**: Generates systemd `.service` unit files in `/etc/systemd/system/` (when executed as root/privileged) or `~/.config/systemd/user/` (for user-level non-root daemons), and notifies systemd via `systemctl daemon-reload`.
   - **Windows (`Service Control Manager`)**: Registers and deletes Win32 services via `OpenSCManagerA`, `CreateServiceA`, and `DeleteService`.
   - **Generic / Disabled Targets**: Provides clean fallback returning `MLA_SERVICE_ERROR_NOT_SUPPORTED` for targets without daemon support (WASM, embedded).
3. **Encapsulated Platform Abstraction**: All platform implementations hook into the `mla_service_platform_t` interface table via `g_mla_service_platform`. Consumer code accesses functionality strictly through public APIs (`mla_service_install`, `mla_service_uninstall`).

---

## Architecture & File Structure

| Layer | File Path | Purpose |
|---|---|---|
| Core API Header | `core/service/mla_service.h` | Public API declarations, error code constants, `mla_service_platform_t` interface |
| Core Implementation | `core/service/mla_service.cpp` | Public API functions with argument validation and platform dispatch |
| Documentation | `core/service/readme.md` | Architecture and platform overview |
| Linux Platform | `platform/linux/mla_global_service_linux.h` | Linux systemd unit generation, reload, and service lifecycle management |
| Windows Platform | `platform/windows/mla_global_service_windows.h` | Windows Win32 Service Control Manager (SCM) integration |
| Generic / Disabled | `platform/generic/mla_global_service_disabled.h` | Disabled stubs returning `MLA_SERVICE_ERROR_NOT_SUPPORTED` |
| Unit Tests | `core-test/tests/mla_service_test.h` | Comprehensive test cases covering arguments, installation, and uninstallation |

---

## Data Types and Constants

### Result Codes

```cpp
#define MLA_SERVICE_SUCCESS 0
#define MLA_SERVICE_ERROR_INVALID_ARGUMENT 1
#define MLA_SERVICE_ERROR_NOT_SUPPORTED 2
#define MLA_SERVICE_ERROR_SYSTEM 3
#define MLA_SERVICE_ERROR_PERMISSION_DENIED 4
#define MLA_SERVICE_ERROR_NOT_FOUND 5
```

### Platform Interface Table (`mla_service_platform_t`)

```cpp
typedef struct mla_service_platform_t {
    mla_int32_t (*install)(const mla_string_t &service_name, const mla_string_t &service_args);
    mla_int32_t (*uninstall)(const mla_string_t &service_name);
    mla_string_t (*get_install_summary)(const mla_string_t &service_name, const mla_string_t &service_args);
} mla_service_platform_t;

extern const mla_service_platform_t g_mla_service_platform;
```

---

## Public API Reference

```cpp
#include "core/service/mla_service.h"

// Installs the currently executing binary as a system service with optional startup arguments
mla_int32_t mla_service_install(const mla_string_t &p_ServiceName, const mla_string_t &p_ServiceArgs = mla_string_empty());

// Uninstalls and removes the specified service entry
mla_int32_t mla_service_uninstall(const mla_string_t &p_ServiceName);

// Returns platform-specific service management instructions / installation summary
mla_string_t mla_service_get_install_summary(const mla_string_t &p_ServiceName, const mla_string_t &p_ServiceArgs = mla_string_empty());
```

---

## Code Examples

### 1. Installing a Service with Startup Arguments

```cpp
#include "core/service/mla_service.h"

mla_string_t service_name = mla_string_const("my_app_daemon");
mla_string_t service_args = mla_string_const("--daemon --port 8080");

mla_int32_t res = mla_service_install(service_name, service_args);
if (res == MLA_SERVICE_SUCCESS) {
    // Service registered successfully
} else if (res == MLA_SERVICE_ERROR_PERMISSION_DENIED) {
    // Requires elevated / root privileges
}
```

### 2. Installing a Service without Arguments

```cpp
mla_string_t service_name = mla_string_const("my_app_worker");

mla_int32_t res = mla_service_install(service_name);
if (res == MLA_SERVICE_SUCCESS) {
    // Installed
}
```

### 3. Uninstalling a Service

```cpp
mla_string_t service_name = mla_string_const("my_app_daemon");

mla_int32_t res = mla_service_uninstall(service_name);
if (res == MLA_SERVICE_SUCCESS) {
    // Service stopped and removed
} else if (res == MLA_SERVICE_ERROR_NOT_FOUND) {
    // Service was not installed
}
```

---

## Platform-Specific Details

### Linux (`systemd`)
- Generates a unit file:
  ```ini
  [Unit]
  Description=<ServiceName> Service
  After=network.target

  [Service]
  Type=simple
  WorkingDirectory=<CurrentWorkingDirectory>
  ExecStart=<ResolvedExecutablePath> <OptionalArgs>
  Restart=on-failure

  [Install]
  WantedBy=default.target
  ```
- If running as `root` (`geteuid() == 0`): writes to `/etc/systemd/system/<ServiceName>.service` and runs `systemctl daemon-reload`.
- If running as regular user: writes to `~/.config/systemd/user/<ServiceName>.service` (resolving `$XDG_CONFIG_HOME`, `$HOME`, or user home via `getpwuid`) and runs `systemctl --user daemon-reload` (with `XDG_RUNTIME_DIR` fallback).
- Uninstallation stops the unit, unlinks the unit file, and executes daemon reload.

### Windows (`SCM`)
- Connects to Service Control Manager via `OpenSCManagerA`.
- Creates service with `SERVICE_WIN32_OWN_PROCESS`, `SERVICE_AUTO_START`, and the quoted executable command line (automatically appending `--working-dir "<CurrentWorkingDirectory>"` if not explicitly set).
- Uninstallation opens the service, sends `SERVICE_CONTROL_STOP`, and calls `DeleteService`.

---

## Build System Integration

1. In `lib/base-lib/core/sources.cmake`, service source files are declared in `SOURCE_SERVICE_FILES`:
   ```cmake
   set(SOURCE_SERVICE_FILES
           ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/service/mla_service.cpp
   )
   ```
2. In `CMakeLists.txt`, include `${SOURCE_SERVICE_FILES}` in all executable target source lists (Linux, Windows, WASM).
3. In entry points (`main_app_*.cpp` and `main_test_*.cpp`), include the corresponding platform header:
   - Linux: `#include "../lib/base-lib/platform/linux/mla_global_service_linux.h"`
   - Windows: `#include "../lib/base-lib/platform/windows/mla_global_service_windows.h"`
   - Generic / WASM: `#include "../lib/base-lib/platform/generic/mla_global_service_disabled.h"`
4. Test files are gated with:
   ```cpp
   #if defined(MLA_SERVICE_SUPPORTED) && MLA_SERVICE_SUPPORTED == 1
   #include "tests/mla_service_test.h"
   #endif
   ```

---

## Rules & Best Practices

1. **Framework String Types**: Always use `const mla_string_t &` for parameters; never use raw platform `const char*` in core framework APIs.
2. **Encapsulation**: Never invoke `g_mla_service_platform.install` or `.uninstall` directly from consumer application or test code; always use `mla_service_install()` and `mla_service_uninstall()`.
3. **No Exceptions**: All service operations return `mla_int32_t` error codes; check return values against `MLA_SERVICE_SUCCESS`.
4. **Skill Synchronization**: Any modification to skills in `lib/base-lib/ai/skills/` must be mirrored to `.agents/skills/` (and vice-versa).
