---
name: coreos-logging
description: Logging conventions and patterns for the MLA framework. Use when adding log output, registering custom loggers, controlling log levels, or implementing structured logging in CoreOS modules.
metadata:
  author: coreos
  version: "1.0"
---

# Logging

The logging module (`core-os/log/`) provides structured, level-based logging with support for multiple log targets. All logging goes through macros that check the active level before formatting, ensuring minimal overhead when a level is disabled.

## Log Levels

| Level | Macro | When to Use |
|---|---|---|
| VERBOSE | `mla_verbose(msg)` | Detailed tracing, inner-loop diagnostics |
| DEBUG | `mla_debug(msg)` | Development-time diagnostics |
| INFO | `mla_info(msg)` | Normal operational events (startup, shutdown, connections) |
| WARNING | `mla_warning(msg)` | Recoverable issues, degraded operation |
| ERROR | `mla_error(msg)` | Failures that prevent an operation from completing |

Log levels are ordered: VERBOSE < DEBUG < INFO < WARNING < ERROR. Setting a level filters out all messages below it.

## Basic Usage

```cpp
#include "../log/mla_logging.h"

void my_function() {
    mla_info("Module initialised");
    mla_debug("Processing started");
    mla_warning("Connection retrying");
    mla_error("Failed to open file");
}
```

The macros automatically include the source file name and function context in the log output.

## Logging with Context

Use the function-based API for messages that include additional context:

```cpp
#include "../log/mla_logger.h"

// With mla_string_t context
mla_log_info(mla_string_const("Connection established"), mla_string_const("192.168.1.10:8080"));

// With C string context
mla_log_warning(mla_string_const("Retry attempt"), "3 of 5");

// With a custom level
mla_log_message(MLA_LOG_LEVEL_DEBUG, mla_string_const("Value updated"), mla_string_const("42"));
```

## Checking Log Level

Before performing expensive formatting, check if the level is active:

```cpp
if (mla_log_is_active(MLA_LOG_LEVEL_DEBUG)) {
    mla_string_t details = build_expensive_debug_string();
    mla_debug(mla_string_data(details));
    mla_string_destroy(details);
}
```

## Activating the Console Logger

The console logger writes to standard output. Activate it during startup:

```cpp
#include "../log/mla_logger_console.h"

// Typically called in a boot event
mla_log_to_console_activate();
```

## Registering a Custom Logger

Create a logger with a custom write function to route logs to any destination (file, network, buffer):

```cpp
#include "../log/mla_logger.h"

static void my_log_writer(
    const mla_log_level p_Level,
    mla_string_t& p_Message,
    mla_string_t& p_Context,
    mla_user_data_t& p_UserData)
{
    // Write to your custom destination
    // p_Level: the log level
    // p_Message: the log message
    // p_Context: additional context (file, function)
    // p_UserData: custom data passed during registration
}

void register_my_logger() {
    mla_logger_t logger;
    logger.name = mla_string_const("MyLogger");
    logger.level = MLA_LOG_LEVEL_INFO;
    logger.need_full_managed_strings = false;
    logger.write = my_log_writer;
    logger.userData = mla_user_data_empty();

    mla_log_register_logger(logger);
}
```

## Controlling Log Levels at Runtime

```cpp
// Set level for a specific logger
mla_log_set_logger_level(mla_string_const("console"), MLA_LOG_LEVEL_WARNING);

// Get current level
mla_log_level level = mla_log_get_logger_level(mla_string_const("console"));

// Unregister a logger
mla_log_unregister_logger(mla_string_const("MyLogger"));
```

## Log Level String Conversion

```cpp
const mla_char_t* levelStr = mla_log_level_to_string(MLA_LOG_LEVEL_INFO);
// Returns "INFO"

mla_size_t len = mla_log_level_to_string_length(MLA_LOG_LEVEL_WARNING);
// Returns length of "WARNING"
```

## Rules

- Use the macro form (`mla_info`, `mla_error`, etc.) for simple messages — they include automatic context.
- Use `mla_log_is_active()` before expensive string construction to avoid wasted work.
- Register the console logger early (e.g., at `mla_lifecycle_boot_event_priority_low_level_setup`).
- Custom loggers must handle thread-safety internally if accessed from multiple tasks.
- Never use `printf`, `std::cout`, or other standard I/O — always use the MLA logging macros.
- Logger names must be unique when registering multiple loggers.

## Incorrect Usage

```cpp
// ❌ Using printf instead of MLA logging
printf("Error: %s\n", errorMsg);  // use mla_error() instead

// ❌ Expensive formatting without level check
mla_string_t msg = build_huge_debug_string();  // wasteful if DEBUG is disabled
mla_debug(mla_string_data(msg));

// ❌ Including <iostream> or <stdio.h>
#include <iostream>  // never include standard I/O headers
```
