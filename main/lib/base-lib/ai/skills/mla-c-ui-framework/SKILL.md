---
name: 'mla-c-ui-framework'
description: 'Patterns for creating UI controls, native/web drawing surfaces, and Web UI integration in the MLA framework'
---

# UI Framework Module

The UI module (`core/ui/`, `core/ui/web/`, and `core-web-ui/`) provides user interface controls, layout trees, drawing surfaces, serializable graphics commands, and web-based remote rendering via HTTP and WebSocket.

## UI Components Architecture

- **Controls** (`core/ui/controls/`): Buttons, labels, text inputs, windows, layout containers.
- **Surfaces** (`core/ui/surfaces/`): Drawing command queues (rectangles, lines, text, paths) and input event dispatchers.
- **Web UI** (`core/ui/web/` & `core-web-ui/`): Preact/TypeScript frontend served as compressed C byte arrays via HTTP server, displaying surfaces on HTML5 canvas via WebSockets.

## Defining UI Controls

```cpp
#include "core/ui/controls/mla_ui_button.h"
#include "core/ui/controls/mla_ui_label.h"

// Create a label
mla_ui_label_t label = mla_ui_label_init(
    mla_string_const("Welcome to MLA-C UI"),
    MLA_UI_TEXT_KIND_PRIMARY
);

// Create a button with click listener
static void on_button_click(const mla_ui_button_t& btn, void* user_data) {
    // Handle click
}

mla_ui_button_t button = mla_ui_button_init(
    mla_string_const("Submit"),
    MLA_UI_BUTTON_STYLE_PRIMARY
);
button.on_click = on_button_click;
```

## Initializing Web UI HTTP Server

```cpp
#include "core/ui/web/mla_ui_http_server.h"
#include "core/http/mla_http_server.h"

mla_network_host_t host = mla_network_host_ip4(mla_string_const("0.0.0.0"), 8081);
mla_http_server_t server = mla_http_server(host);

if (mla_ui_http_server_initialize(server)) {
    mla_http_server_start(server, 4);
}
```

## Best Practices

1. **Serializability**: UI draw commands must be serializable for remote WebSocket rendering.
2. **Style Constants**: Use predefined styling constants from `mla_ui_style.h` (e.g. font sizes and theme color palettes).
3. **No Direct DOM Mutation**: In Web UI frontend code, rely on state hooks (`useLogService`, `useBootstrap`) and canvas renderers (`RemoteUIDrawer`).
