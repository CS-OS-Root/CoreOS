---
name: 'mla-c-config'
description: 'Patterns for using the configuration management system in the MLA framework'
---

# Configuration Management System

The Config module (`core/config/`) provides configuration storage and management for key-value settings. It supports loading, retrieving, and serializing application settings in memory or backed by file storage.

## Key Concepts and Headers

| Concept | Header | Description |
|---|---|---|
| Config Instance | `mla_config.h` | Core configuration getter/setter functions |
| Global Config | `mla_global_config.h` | Global configuration options and limit macros |
| Platform Storage | `mla_global_config_linux.h`, `mla_global_config_inmemory.h`, etc. | Platform-specific storage backends |

## Retrieving Configuration Values

Use getters with default values to read configuration options safely:

```cpp
#include "core/config/mla_config.h"

// Read configuration setting with fallback default
mla_string_t key = mla_string_const("network.port");
mla_string_t default_val = mla_string_const("8080");

mla_string_t port_str = mla_config_get_string(key, default_val);
```

## Updating Configuration Values

```cpp
#include "core/config/mla_config.h"

mla_string_t key = mla_string_const("network.port");
mla_string_t new_val = mla_string_const("9090");

mla_bool_t success = mla_config_set_string(key, new_val);
```

## Rules & Best Practices

1. **No Standard Types**: Always use `mla_string_t` for configuration keys and string values.
2. **Encapsulation**: Use API functions (`mla_config_get_string`, `mla_config_set_string`) rather than directly accessing struct members.
3. **Platform Initialization**: On startup, ensure the relevant platform config backend (`mla_global_config_*`) is initialized before accessing global config values.
