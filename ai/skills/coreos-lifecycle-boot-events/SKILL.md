---
name: coreos-lifecycle-boot-events
description: Lifecycle boot event registration patterns for the MLA framework. Use when initializing modules at startup, controlling boot order, or registering callbacks that fire during the application boot sequence.
metadata:
  author: coreos
  version: "1.0"
  converted-from: ai/instructions/lifecycle_and_boot_events.instructions.md
---

# Lifecycle and Boot Events

The lifecycle module (`core-os/lifecycle/mla_lifecycle_events.h`) provides a **priority-ordered boot system**. Callbacks are registered at compile time via a static-initialiser macro and are executed in ascending priority order when `mla_lifecycle_fire_boot_events()` is called at startup.

## Priority Levels

Use the predefined priority constants to control the order of execution:

| Constant | Value | When to Use |
|---|---|---|
| `mla_lifecycle_boot_event_priority_low_level_preSetup` | -1010 | Before any platform setup |
| `mla_lifecycle_boot_event_priority_low_level_setup` | -1000 | Platform / low-level initialisation |
| `mla_lifecycle_boot_event_priority_low_level_postSetup` | -990 | After low-level setup |
| `mla_lifecycle_boot_event_priority_file_system_preSetup` | -10 | Before file system init |
| `mla_lifecycle_boot_event_priority_file_system_setup` | 0 | File system initialisation |
| `mla_lifecycle_boot_event_priority_file_system_postSetup` | 10 | After file system init |
| `mla_lifecycle_boot_event_priority_rpc_preSetup` | 490 | Before RPC init |
| `mla_lifecycle_boot_event_priority_rpc_setup` | 500 | RPC / reflection registration |
| `mla_lifecycle_boot_event_priority_rpc_postSetup` | 510 | After RPC init |
| `mla_lifecycle_boot_event_priority_network_preSetup` | 990 | Before network init |
| `mla_lifecycle_boot_event_priority_network_setup` | 1000 | Network initialisation |
| `mla_lifecycle_boot_event_priority_network_postSetup` | 1100 | After network init |
| `mla_lifecycle_boot_event_priority_application_preSetup` | 1900 | Before application init |
| `mla_lifecycle_boot_event_priority_application_setup` | 2000 | Application initialisation |
| `mla_lifecycle_boot_event_priority_application_postSetup` | 2100 | After application init |

> **Rule of thumb:** Use `mla_lifecycle_boot_event_priority_application_setup` for most feature modules.
> Use lower priorities only for infrastructure that other modules depend on.

## Registering a Boot Event (Static Macro — Preferred)

The macro `mla_lifecycle_boot_event_static_register` uses a C++ static initialiser to register the callback **without any manual wiring**. Place it in the `.cpp` file of your module, at file scope:

```cpp
#include "../lifecycle/mla_lifecycle_events.h"
#include "../log/mla_logging.h"

static void my_module_boot() {
    mla_info("my_module: initialising");
    // one-time setup …
}

// Registration happens automatically before main() is called
mla_lifecycle_boot_event_static_register(
    mla_lifecycle_boot_event_priority_application_setup,
    my_module_boot
)
```

### Rules
- The callback name passed to the macro must be a valid C identifier (no spaces, no `()`).
- Each callback name must be **unique** across the entire project.
- Do **not** call `mla_lifecycle_fire_boot_events()` yourself; the platform entry point does it.

## Registering a Boot Event at Runtime (Advanced)

Call `mla_lifecycle_boot_event_register` directly when the priority or callback must be determined dynamically. This must be called **before** `mla_lifecycle_fire_boot_events()` runs.

```cpp
void my_runtime_boot_callback() {
    mla_info("runtime boot event fired");
}

// Called early in main() or from another static initialiser
mla_lifecycle_boot_event_register(
    mla_lifecycle_boot_event_priority_application_setup,
    my_runtime_boot_callback
);
```

## Firing All Boot Events

The platform entry point is responsible for firing boot events exactly once:

```cpp
mla_lifecycle_fire_boot_events();
```

All callbacks registered via `mla_lifecycle_boot_event_static_register` or `mla_lifecycle_boot_event_register` are invoked in priority order (lowest value first).

## Common Patterns

### Module That Depends on Another Module

If Module B needs Module A to be initialised first, register Module A at a lower priority:

```cpp
// Module A — registered at 2000
mla_lifecycle_boot_event_static_register(
    mla_lifecycle_boot_event_priority_application_setup,
    module_a_boot
)

// Module B — registered at 2100 (after A)
mla_lifecycle_boot_event_static_register(
    mla_lifecycle_boot_event_priority_application_postSetup,
    module_b_boot
)
```

### Registering Reflection at Boot

The reflection macro uses the `low_level_postSetup` priority internally. Register custom structs at this same level if they must be available before RPC is set up:

```cpp
mla_reflection_auto_register_struct(mla_my_data_t)
```

(This expands to a `mla_lifecycle_boot_event_static_register` call at priority -990.)

### Registering RPC at Boot

The `mla_rpc_auto_register_procedure` macro registers procedures at `rpc_preSetup` (490). No explicit lifecycle call is needed.

## Incorrect Usage

```cpp
// ❌ Wrong — never call the callback directly; let the lifecycle fire it
my_module_boot();

// ❌ Wrong — registering after fire_boot_events() has no effect
mla_lifecycle_fire_boot_events();
mla_lifecycle_boot_event_register(2000, late_callback); // too late!

// ❌ Wrong — duplicate callback name causes a compile error
mla_lifecycle_boot_event_static_register(2000, my_module_boot)
mla_lifecycle_boot_event_static_register(2000, my_module_boot) // name collision
```
