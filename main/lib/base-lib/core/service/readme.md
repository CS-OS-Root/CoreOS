# Service Management Platform Abstraction

The Service module (`core/service/`) provides a cross-platform interface to install and uninstall the current running binary as an operating system service / daemon.

## Architecture

- **`mla_service_platform_t`**: Platform abstraction table struct containing function pointers:
  - `install`: Automatically resolves the currently running binary path, appends startup arguments if provided, and registers the service in the host OS init system (e.g., systemd unit files on Linux, Service Control Manager on Windows).
  - `uninstall`: Deregisters, removes, and cleans up the configured service entry for the application from the host system.
  - `get_install_summary`: Formats and returns platform-specific service management instructions and installation summary.
- **`g_mla_service_platform`**: Global struct instance configured per target platform.
- **`mla_service_install(const mla_string_t &service_name, const mla_string_t &service_args = mla_string_empty())`**: Helper API function.
- **`mla_service_uninstall(const mla_string_t &service_name)`**: Helper API function.
- **`mla_service_get_install_summary(const mla_string_t &service_name, const mla_string_t &service_args = mla_string_empty())`**: Helper API function.

## Platform Implementations

- **Linux** (`platform/linux/mla_global_service_linux.h`): Resolves `/proc/self/exe`, writes systemd service units to `/etc/systemd/system/` (root) or `~/.config/systemd/user/` (user), and manages `systemctl daemon-reload`.
- **Windows** (`platform/windows/mla_global_service_windows.h`): Resolves `GetModuleFileNameA`, interacts with Win32 SCM (`OpenSCManagerA`, `CreateServiceA`, `DeleteService`).
- **Generic / Disabled** (`platform/generic/mla_global_service_disabled.h`): Returns `MLA_SERVICE_ERROR_NOT_SUPPORTED` for unsupported targets (WASM, ESP-IDF, etc.).

## Build System Integration

Service files are encapsulated in the `SOURCE_SERVICE_FILES` source group in `sources.cmake` and can be toggled per build target. Tests are isolated behind `#if defined(MLA_SERVICE_SUPPORTED) && MLA_SERVICE_SUPPORTED == 1`.
