---
name: coreos-task-management
description: Patterns for creating and scheduling tasks in the MLA framework. Use when implementing repeating tasks, one-time tasks, native threads, mutexes, or read-write locks in CoreOS.
metadata:
  author: coreos
  version: "1.0"
  converted-from: ai/instructions/task_management.instructions.md
---

# Task Management

The task module (`core-os/task/`) provides co-operative and native multi-tasking. Tasks are registered with the `mla_task_manager` and dispatched either in a single-threaded loop or via native OS threads, depending on the platform.

## Task Types

| Factory | Worker Signature | Behaviour |
|---|---|---|
| `mla_task_repeating` | `mla_task_process_result_state (*)(mla_user_data_t&)` | Called repeatedly; stops when it returns `TASK_PROCESS_RESULT_DONE` |
| `mla_task_one_time` | `void (*)(mla_user_data_t&)` | Called once, then removed |
| `mla_task_native` | `mla_task_process_result_state (*)(mla_user_data_t&)` | Maps directly to an OS thread (use when blocking I/O is needed) |

## Priority Levels

| Constant | When to Use |
|---|---|
| `TASK_PRIO_LOW` | Background work, logging, cleanup |
| `TASK_PRIO_NORMAL` | Default — most tasks |
| `TASK_PRIO_HIGH` | Time-sensitive operations (networking I/O, sensor polling) |

## Stack Size Hints

| Constant | When to Use |
|---|---|
| `TASK_STACK_SIZE_DEFAULT` | Standard use (recommended default) |
| `TASK_STACK_SIZE_TINY` | Minimal footprint, embedded targets |
| `TASK_STACK_SIZE_SMALL` | Light I/O, simple state machines |
| `TASK_STACK_SIZE_MEDIUM` | JSON parsing, string manipulation |
| `TASK_STACK_SIZE_LARGE` | Complex RPC handlers, file I/O |
| `TASK_STACK_SIZE_XLARGE` | Heavy computation, nested serialization |
| `TASK_STACK_SIZE_XXLARGE` | TLS handshake, large buffer operations |

## Creating a Repeating Task

A repeating task runs continuously until it signals completion.

```cpp
#include "../task/mla_task.h"
#include "../task/mla_task_manager.h"

static mla_task_process_result_state sensor_poll_task(mla_user_data_t& p_UserData) {
    // Retrieve context stored in user data
    SensorContext* ctx = mla_user_data_get_pointer<SensorContext>(p_UserData, "ctx");

    // Do work …
    mla_float_t reading = ctx->readTemperature();
    mla_info("Temperature polled");

    // Return CONTINUE to be called again, or DONE to self-remove
    return TASK_PROCESS_RESULT_CONTINUE;
}

void sensor_module_init() {
    SensorContext* ctx = static_cast<SensorContext*>(mla_malloc(sizeof(SensorContext)));
    // … initialise ctx …

    mla_user_data_t userData = mla_user_data_empty();
    mla_user_data_set_pointer_with_ownership(userData, "ctx", ctx);

    mla_task_t task = mla_task_repeating(
        mla_string_const("sensor.poll"),
        sensor_poll_task,
        userData
    );

    mla_task_manager_register_task(task);
}
```

## Creating a One-Time Task

Use for deferred or asynchronous initialisation that runs once after the main loop starts.

```cpp
static void config_load_task(mla_user_data_t& p_UserData) {
    mla_info("Loading configuration …");
    // … load config from file …
}

void schedule_config_load() {
    mla_user_data_t userData = mla_user_data_empty();
    mla_task_t task = mla_task_one_time(
        mla_string_const("config.load"),
        config_load_task,
        userData
    );
    mla_task_manager_register_task(task);
}
```

## Creating a Native (Blocking) Task

Use `mla_task_native` for tasks that perform blocking calls (e.g., socket `recv`, file reads). On embedded platforms without native threads this falls back to the simulated mode.

```cpp
static mla_task_process_result_state http_server_task(mla_user_data_t& p_UserData) {
    HttpServerContext* ctx = mla_user_data_get_pointer<HttpServerContext>(p_UserData, "ctx");
    ctx->acceptNextConnection(); // blocking call
    return TASK_PROCESS_RESULT_CONTINUE;
}

void http_server_start() {
    mla_user_data_t userData = mla_user_data_empty();
    // … fill userData …

    mla_task_t task = mla_task_native(
        mla_string_const("http.server"),
        http_server_task,
        userData
    );
    task.priority   = TASK_PRIO_HIGH;
    task.stack_size = TASK_STACK_SIZE_LARGE;
    mla_task_manager_register_task(task);
}
```

## Task Lifecycle Operations

```cpp
// Check if a task is registered
mla_bool_t exists = mla_task_manager_task_exists(mla_string_const("sensor.poll"));

// Abort a running task
mla_task_manager_abort_task(mla_string_const("sensor.poll"));

// Get info for a single task
mla_task_info_t info = mla_task_manager_get_task_info(mla_string_const("sensor.poll"));
mla_info(mla_task_state_to_string(info.state));

// Get all task infos
auto infos = mla_task_manager_get_task_infos();
for (mla_size_t i = 0; i < mla_array_list_size(infos); i++) {
    auto& t = mla_array_list_get(infos, i);
    mla_info(mla_string_data(t.name));
}
```

## Running the Task Loop

The platform entry point calls `mla_task_manager_process_all_tasks()`, which blocks until all tasks have completed or been aborted:

```cpp
// Typically called from main() after lifecycle boot events
mla_task_manager_process_all_tasks();
```

Do **not** call this from within a task or a boot event callback.

## Using Mutexes

```cpp
#include "../task/mla_mutx.h"

mla_mutex_t mutex = mla_mutex_create("my.mutex");

// Lock with default timeout (1000 ms)
if (mla_mutex_lock(mutex)) {
    // … critical section …
    mla_mutex_unlock(mutex);
} else {
    mla_error("Mutex timeout");
}

// Lock with a custom timeout (milliseconds)
if (mla_mutex_lock_timeout(mutex, 500)) {
    // … critical section …
    mla_mutex_unlock(mutex);
}
```

## Using Read-Write Locks

```cpp
#include "../task/mla_rw_lock.h"

mla_rw_lock_t lock = mla_rw_lock_create("my.lock");

// Multiple readers can hold the lock simultaneously
if (mla_rw_lock_read(lock)) {
    // … read shared data …
    mla_rw_unlock_read(lock);
}

// Only one writer at a time; excludes all readers
if (mla_rw_lock_write(lock)) {
    // … modify shared data …
    mla_rw_unlock_write(lock);
}
```

## Rules

- Task names must be **unique** across the task manager.
- Never call blocking OS functions from a `mla_task_repeating` worker on single-threaded platforms — use `mla_task_native`.
- Always return `TASK_PROCESS_RESULT_DONE` when the task has finished its work to allow cleanup.
- Do **not** register the same task twice (check with `mla_task_manager_task_exists` first if needed).
- Free all resources in the worker before returning `TASK_PROCESS_RESULT_DONE`; do not rely on `mla_task_manager_cleanup` for per-task cleanup.

## Incorrect Usage

```cpp
// ❌ Blocking inside a simulated repeating task
static mla_task_process_result_state bad_task(mla_user_data_t& p_UserData) {
    sleep(1);  // blocks the entire task loop on single-threaded platforms
    return TASK_PROCESS_RESULT_CONTINUE;
}

// ❌ Registering a task before the task manager is initialised
// (Register tasks inside boot events or after mla_lifecycle_fire_boot_events)
mla_task_manager_register_task(task); // too early if called before boot events

// ❌ Duplicate task name
mla_task_manager_register_task(mla_task_repeating(mla_string_const("worker"), fn, ud));
mla_task_manager_register_task(mla_task_repeating(mla_string_const("worker"), fn, ud)); // duplicate!
```
