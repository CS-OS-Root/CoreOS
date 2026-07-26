---
name: 'mla-c-platform-abstractions'
description: 'Patterns for platform implementations, hardware abstractions, and OS porting in the MLA framework'
---

# Platform Abstractions & HAL Module

The Platform module (`platform/`) provides OS and hardware abstraction layers (HAL) allowing `mla-c` applications to run across multiple target environments.

## Supported Platform Implementations

| Platform | Location | Description |
|---|---|---|
| Linux | `platform/linux/` | POSIX sockets, pthreads, POSIX filesystem |
| Windows | `platform/windows/` | Winsock2, Win32 threads, Win32 filesystem, GDI+/Direct2D UI |
| WebAssembly | `platform/wasm/` | Browser & Node.js WASM targets (standard & zero-dependency standalone mode) |
| ESP-IDF | `platform/espidf/` | ESP32 & ESP8266 FreeRTOS, SPIFFS/LittleFS, ESP Wi-Fi |
| Raspberry Pi Pico | `platform/raspberry/` | RP2040 Pico SDK, dual-core ARM Cortex-M0+ support |
| Generic | `platform/generic/` | Fallback platform & fast floating point parsing (`fast_float`) |

## Fast Float Parsing

The generic and platform modules provide optimized, header-only floating point parsing:

```cpp
#include "platform/generic/mla_global_platform_fast_float.h"

const char* str = "3.1415926535";
double val = __fast_float_strtod(str, nullptr);
```

## WASM Standalone Mode

In standalone WASM mode (`mla_global_platform_wasm_standalone.h`), standard C libraries are not linked; memory operations (`memcpy`, `memset`) and system calls are imported directly from the JavaScript runtime via Web Workers.

## Rules & Guidelines

1. **Include Order**: Include the target platform header (`mla_global_platform_<target>.h`) before core headers when compiling platform-specific applications.
2. **Abstracted Interfaces**: Consumer code must use abstract platform functions (`mla_platform_malloc`, `mla_platform_sleep_ms`) instead of OS-specific calls (`sleep`, `Sleep`, `vTaskDelay`).
