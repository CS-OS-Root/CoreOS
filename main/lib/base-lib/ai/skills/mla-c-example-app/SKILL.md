---
name: 'mla-c-example-app'
description: 'Patterns and structure for building complete applications using the MLA framework'
---

# Core Example Application Module

The `core-example-app/` component showcases reference patterns for assembling full-featured applications using `mla-c` framework modules.

## Architecture & Structure

| Header | Description |
|---|---|
| `main_app.h` | Core application entry point, logging setup, and module orchestrator |
| `main_app_cli.h` | CLI app initialization and stdin/stdout updating loop |
| `main_app_web_ui.h` | HTTP web server setup on port 8081 with RPC endpoints |
| `main_app_main_window.h` | Native UI window showcasing labels, buttons, and inputs |
| `main_app_background_task.h` | Periodic background task running every 500ms |
| `main_app_window_ui.h` | Single-threaded display surface rendering setup |

## Bootstrap Pattern

To build an application using the framework:

```cpp
#include "platform/linux/mla_global_platform_linux.h"
#include "main_app.h"

int main() {
    return run();
}
```

## Integrated Framework Features

- **CLI Integration**: Standard input command parsing & sub-module management.
- **Web UI & Dashboard**: HTTP server running on `0.0.0.0:8081` with WebSocket remote canvas rendering.
- **UI Control Gallery**: Demonstration of primary/secondary/error labels, styled buttons, and password inputs.
- **Scheduled Tasks**: Repeating task manager routines with thread-safe user data.
