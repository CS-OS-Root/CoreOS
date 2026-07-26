---
name: 'mla-c-external-task'
description: 'Patterns for launching and controlling external system processes in the MLA framework'
---

# External Task Module

The External Task module (`core/external_task/`) provides functionality for launching, managing, and interacting with external processes and system executables.

## Key Types & Functions

| Type / Function | Header | Description |
|---|---|---|
| `mla_external_task_t` | `mla_external_task.h` | Handle representing a running or spawned external process |
| `mla_external_task_run` | `mla_external_task.h` | Launches an external executable with parameters |
| `mla_external_task_kill` | `mla_external_task.h` | Terminates a running process handle |
| `mla_external_task_wait` | `mla_external_task.h` | Waits for process execution to complete |

## Launching an External Process

```cpp
#include "core/external_task/mla_external_task.h"

mla_string_t program = mla_string_const("/usr/bin/git");
mla_array_list_t<mla_string_t, mla_string_initializer> args = mla_array_list_t<mla_string_t, mla_string_initializer>::create();

mla_array_list_add(args, mla_string_const("status"));

mla_external_task_t task;
if (mla_external_task_run(program, args, task)) {
    // Process launched successfully
    mla_int32_t exit_code = 0;
    mla_external_task_wait(task, exit_code);
}
```

## Rules & Best Practices

1. **Memory Management**: Always release arguments and task handles when no longer needed using framework utilities.
2. **Resource Cleanup**: Ensure processes are waited for or killed to prevent orphan/zombie processes.
3. **No Exceptions**: All external task operations return `mla_bool_t` status flags indicating success or failure.
