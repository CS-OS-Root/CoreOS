---
name: coreos-cli-commands
description: Patterns for creating CLI commands and modules in the MLA framework. Use when building command-line interfaces, adding commands with parameters, or integrating CLI functionality into a CoreOS application.
metadata:
  author: coreos
  version: "1.0"
  converted-from: ai/instructions/cli_commands.instructions.md
---

# CLI Commands

The CLI module (`core-os/cli/`) provides a hierarchical command-line interface. Commands are grouped into **modules** (namespaces), each containing one or more **commands** with named parameters.

## Key Types

| Type | Header | Purpose |
|---|---|---|
| `mla_cli_command_t` | `mla_cli_command.h` | A single command with parameters and an execute callback |
| `mla_cli_module_t` | `mla_cli_module.h` | A named group of commands |
| `mla_cli_app_t` | `mla_cli_app.h` | The top-level CLI application holding all modules |

## Step 1 — Define a Command

```cpp
#include "../cli/mla_cli_command.h"

// Execute callback signature
static mla_bool_t my_command_execute(
    const mla_cli_command_t& p_Command,
    const mla_hash_map_t<mla_string_t, mla_string_t,
                         mla_string_hash_t,
                         mla_string_initializer,
                         mla_string_initializer>& p_Parameters,
    const mla_cli_command_execute_outstream_t& p_Out)
{
    // Read a named parameter
    mla_string_t value = mla_string_empty();
    mla_hash_map_get(p_Parameters, mla_string_const("name"), value);

    // Write output
    p_Out.writeCString(p_Out.userdata, "Hello, ");
    p_Out.write(p_Out.userdata, value);
    p_Out.writeCString(p_Out.userdata, "\n");
    return true;
}
```

### Reading Parameters Safely

```cpp
mla_string_t host = mla_string_empty();
if (!mla_hash_map_get(p_Parameters, mla_string_const("host"), host)) {
    p_Out.writeCString(p_Out.userdata, "Error: --host is required\n");
    return false;
}
```

## Step 2 — Build a CLI Module

```cpp
#include "../cli/mla_cli_module.h"
#include "../cli/mla_cli_command.h"

static mla_cli_module_t build_network_module() {
    mla_cli_module_t module = mla_cli_module(mla_string_const("network"));

    // --- "connect" command ---
    mla_cli_command_t connectCmd = mla_cli_command(
        mla_string_const("connect"),
        mla_string_const("Connect to a remote host")
    );
    mla_cli_command_add_parameter(connectCmd,
        mla_string_const("host"),
        mla_string_const("Hostname or IP address"),
        true  // mandatory
    );
    mla_cli_command_add_parameter(connectCmd,
        mla_string_const("port"),
        mla_string_const("Port number"),
        false // optional
    );
    connectCmd.execute = connect_execute;
    mla_cli_module_add_command(module, connectCmd);

    // --- "status" command (no parameters) ---
    mla_cli_command_t statusCmd = mla_cli_command(mla_string_const("status"));
    statusCmd.execute = status_execute;
    mla_cli_module_add_command(module, statusCmd);

    return module;
}
```

## Step 3 — Register the Module with the CLI App

```cpp
#include "../cli/mla_cli_app.h"

// Called once at startup (e.g., from your boot event or init function)
void network_cli_init(mla_cli_app_t& p_App) {
    mla_cli_module_t networkModule = build_network_module();
    mla_cli_app_register_module(p_App, networkModule);
}
```

## Step 4 — Create and Run the CLI App

The CLI app reads input from a stream and writes output to another stream. The typical pattern is to create the app once and poll it in a repeating task:

```cpp
#include "../cli/mla_cli_app.h"
#include "../system/mla_stream.h"
#include "../task/mla_task.h"
#include "../task/mla_task_manager.h"

static mla_cli_app_t g_cli_app;

static mla_task_process_result_state cli_task(mla_user_data_t& p_UserData) {
    mla_cli_app_update_and_process_input(
        g_cli_app,
        mla_stream_input_stdin(),
        mla_stream_output_stdout()
    );
    return TASK_PROCESS_RESULT_CONTINUE;
}

void cli_init() {
    mla_cli_module_t rootModule = build_root_module();
    g_cli_app = mla_cli_app_init(rootModule, mla_stream_output_stdout());

    mla_user_data_t userData = mla_user_data_empty();
    mla_task_t task = mla_task_repeating(mla_string_const("cli"), cli_task, userData);
    mla_task_manager_register_task(task);
}
```

## Step 5 — Attach User Data to a Command (Optional)

Use `mla_user_data_t` to pass state to the execute callback without globals:

```cpp
// In your module builder:
MyContext* ctx = static_cast<MyContext*>(mla_malloc(sizeof(MyContext)));
// … initialise ctx …

mla_user_data_t userData = mla_user_data_empty();
mla_user_data_set_pointer_with_ownership(userData, "ctx", ctx);

connectCmd.user_data = userData;
connectCmd.execute   = connect_execute;

// In the execute callback:
static mla_bool_t connect_execute(
    const mla_cli_command_t& p_Command,
    const mla_hash_map_t<...>& p_Parameters,
    const mla_cli_command_execute_outstream_t& p_Out)
{
    MyContext* ctx = mla_user_data_get_pointer<MyContext>(p_Command.user_data, "ctx");
    // use ctx …
    return true;
}
```

## Output Helpers

The `mla_cli_command_execute_outstream_t` struct provides two write functions:

```cpp
// Write an mla_string_t
p_Out.write(p_Out.userdata, mla_string_const("value: "));

// Write a C string literal (no mla_string_t needed)
p_Out.writeCString(p_Out.userdata, "Done.\n");
```

Always end output lines with `\n` for proper terminal display.

## Naming Conventions

- Module names: lowercase, short, hyphen-separated (e.g., `"network"`, `"file-system"`)
- Command names: lowercase verb or verb-noun (e.g., `"connect"`, `"list-files"`)
- Parameter names: lowercase with hyphens (e.g., `"host"`, `"output-path"`)
- Mandatory parameters should be listed before optional ones when calling `mla_cli_command_add_parameter`.

## Rules

- Execute callbacks must return `true` on success and `false` on failure.
- Do not store a pointer to the `p_Parameters` map after the callback returns — it is stack-allocated.
- Each module name must be unique within the CLI app.
- Each command name must be unique within its module.
- Parameters are always passed as `mla_string_t` values; convert to numbers using `mla_string_to_int32` or similar utilities.

## Incorrect Usage

```cpp
// ❌ Using standard types for output
printf("result: %d\n", value);   // use p_Out.writeCString instead

// ❌ Not null-checking user data
MyContext* ctx = mla_user_data_get_pointer<MyContext>(p_Command.user_data, "ctx");
ctx->doSomething(); // crash if ctx is null

// ❌ Forgetting to set the execute callback
mla_cli_command_t cmd = mla_cli_command(mla_string_const("status"));
mla_cli_module_add_command(module, cmd); // cmd.execute is nullptr — will crash on invocation
```
