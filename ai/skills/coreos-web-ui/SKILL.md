---
name: coreos-web-ui
description: Web UI development patterns for the MLA framework. Use when creating UI controls, rendering surfaces, integrating the web UI server, or building interactive web-based interfaces for CoreOS applications.
metadata:
  author: coreos
  version: "1.0"
---

# Web UI

The UI module (`core-os/ui/`) provides a control-based UI system that renders to draw commands and can be served over HTTP via WebSockets. Controls are hierarchical, support input events, and are styled with typed value properties.

## Architecture Overview

```
┌─────────────┐    ┌──────────────┐    ┌──────────────┐
│  UI Controls │───▶│ Draw Commands │───▶│   Surface    │
│  (logical)   │    │  (rendering) │    │  (display)   │
└─────────────┘    └──────────────┘    └──────────────┘
       │                                       │
       │                                       ▼
       │                               ┌──────────────┐
       └──────── Input Events ◀────────│  Web Server  │
                                       │  (WebSocket) │
                                       └──────────────┘
```

## Key Types

| Type | Header | Purpose |
|---|---|---|
| `mla_ui_control_t` | `controls/mla_ui_control.h` | A UI element with layout, children, and event handlers |
| `mla_ui_control_layout_t` | `controls/mla_ui_control.h` | Position and size (x, y, width, height) |
| `mla_ui_control_value_t` | `controls/mla_ui_control.h` | Named property value (string, int, bool, etc.) |
| `mla_ui_control_context_t` | `controls/mla_ui_control.h` | Rendering context with timing and input state |
| `mla_ui_surface_draw_command_t` | `surfaces/mla_ui_surface_draw.h` | Low-level drawing primitives |

## Built-In Controls

| Control | Header | Purpose |
|---|---|---|
| Window | `controls/mla_ui_window.h` | Container panel with title bar |
| Label | `controls/mla_ui_label.h` | Text display |
| Button | `controls/mla_ui_button.h` | Clickable button |
| Text Edit | `controls/mla_ui_text_edit.h` | Text input field |
| Loading Indicator | `controls/mla_ui_loading_indicator.h` | Activity spinner |

## Creating Controls

### Basic Control

```cpp
#include "../ui/controls/mla_ui_control.h"

mla_ui_control_t myControl = mla_ui_control();
myControl.id = mla_string_const("my-panel");
myControl.layout = { 10.0, 10.0, 400.0, 300.0 };  // x, y, width, height
```

### Setting Control Values

Controls store typed properties via the value system:

```cpp
// Set string value
mla_ui_control_set_value_as_string(control, mla_string_const("text"), mla_string_const("Hello"));

// Set numeric values
mla_ui_control_set_value_as_int32(control, mla_string_const("count"), 42);
mla_ui_control_set_value_as_double(control, mla_string_const("progress"), 0.75);

// Set boolean value
mla_ui_control_set_value_as_bool(control, mla_string_const("visible"), true);
```

### Reading Control Values

```cpp
mla_string_t text = mla_ui_control_get_value_as_string(control, mla_string_const("text"));
mla_int32_t count = mla_ui_control_get_value_as_int32(control, mla_string_const("count"), 0);
mla_bool_t visible = mla_ui_control_get_value_as_bool(control, mla_string_const("visible"), true);
```

## Building a Control Hierarchy

```cpp
mla_ui_control_t window = mla_ui_control();
window.id = mla_string_const("main-window");
window.layout = { 0.0, 0.0, 800.0, 600.0 };

mla_ui_control_t label = mla_ui_control();
label.id = mla_string_const("title-label");
label.layout = { 20.0, 20.0, 200.0, 30.0 };
mla_ui_control_set_value_as_string(label, mla_string_const("text"), mla_string_const("My App"));

mla_ui_control_t button = mla_ui_control();
button.id = mla_string_const("submit-btn");
button.layout = { 20.0, 60.0, 120.0, 40.0 };
mla_ui_control_set_value_as_string(button, mla_string_const("text"), mla_string_const("Submit"));

// Add children to window
mla_ui_control_add_child(window, label);
mla_ui_control_add_child(window, button);
```

## Implementing Custom Render Functions

Each control can define how it renders by setting `renderToDrawCommands`:

```cpp
static mla_bool_t my_render(
    const mla_ui_control_context_t& p_Context,
    const mla_ui_control_t& p_Control,
    mla_array_list_t<mla_ui_surface_draw_command_t, ...>& p_DrawCommands,
    mla_array_list_t<mla_ui_control_input_area_t, ...>& p_InputAreas)
{
    // Emit draw commands for this control
    // ...

    // Render children
    mla_ui_control_render_children_draw_commands(p_Context, p_Control, p_DrawCommands, p_InputAreas);

    return true;
}

myControl.renderToDrawCommands = my_render;
```

## Handling Input Events

### Click Events

```cpp
static mla_bool_t my_click_handler(
    mla_ui_control_t& p_Control,
    const mla_ui_surface_input_event_click_t& p_ClickEvent,
    const mla_ui_control_input_area_t& p_InputArea,
    mla_array_list_t<mla_ui_control_t, ...>& p_UIControls,
    mla_user_data_t& p_UserData)
{
    mla_info("Button clicked!");
    return true;
}

button.processClickEvent = my_click_handler;
```

### Character Input Events

```cpp
static mla_bool_t my_char_handler(
    mla_ui_control_t& p_Control,
    const mla_ui_surface_input_event_char_input_t& p_CharEvent,
    mla_array_list_t<mla_ui_control_t, ...>& p_UIControls,
    mla_user_data_t& p_UserData)
{
    // Handle text input
    return true;
}

textEdit.processCharInputEvent = my_char_handler;
```

## Web UI HTTP Server Integration

### Initialize the Web UI Server

```cpp
#include "../ui/web/mla_ui_http_server.h"
#include "../http/mla_http_server.h"

mla_http_server_t server = mla_http_server(
    mla_network_host_ip4(mla_string_const("0.0.0.0"), 8081)
);

// Initialize UI support on the HTTP server
mla_ui_http_server_initialize(server);
```

### Add a Web Surface

```cpp
static mla_task_process_result_state my_ui_task(mla_user_data_t& p_UserData) {
    // Update UI controls and render
    return TASK_PROCESS_RESULT_CONTINUE;
}

mla_ui_http_server_add_web_surface(
    server,
    mla_string_const("Main Display"),    // display name
    mla_string_const("main"),            // surface name (URL path)
    my_ui_task                           // processing task
);
```

### Start the Server

```cpp
mla_http_server_start(server, 2, TASK_STACK_SIZE_LARGE);
```

The Web UI is then accessible via a browser at the server address.

## Finding and Updating Controls

```cpp
mla_ui_control_t* foundControl = nullptr;

// Find by ID
if (mla_ui_control_find_by_id(uiControls, mla_string_const("my-label"), foundControl)) {
    mla_ui_control_set_value_as_string(
        *foundControl,
        mla_string_const("text"),
        mla_string_const("Updated text")
    );
}

// Check focus state
if (mla_ui_control_has_focus(control)) {
    // control is focused
}

// Check hover state
if (mla_ui_control_is_hovered(context, control)) {
    // mouse is over control
}
```

## Rules

- Control IDs must be unique within the control hierarchy.
- Always set the layout (position and size) for each control.
- Use the value system (`set_value_as_*` / `get_value_as_*`) for control properties.
- Render functions should call `mla_ui_control_render_children_draw_commands` to render child controls.
- The web surface name becomes part of the URL path.
- Use `TASK_STACK_SIZE_LARGE` or higher for UI server tasks.
- Input processing is done through the `processClickEvent` and `processCharInputEvent` callbacks.

## Incorrect Usage

```cpp
// ❌ Forgetting to set layout
mla_ui_control_t ctrl = mla_ui_control();
ctrl.id = mla_string_const("my-ctrl");
// missing: ctrl.layout = { ... };

// ❌ Duplicate control IDs
label1.id = mla_string_const("label");
label2.id = mla_string_const("label");  // ID collision

// ❌ Not rendering children in custom render function
static mla_bool_t my_render(...) {
    // draws own content but forgets children
    return true;
    // missing: mla_ui_control_render_children_draw_commands(...)
}
```
