---
name: 'mla-c-create-a-test'
description: 'Conventions and patterns for writing unit tests in the MLA framework using mla_test_executor_t'
---
Provide project context and coding guidelines that AI should follow when generating code, answering questions, or reviewing changes.

# Copilot Instructions for MLA Project

## Test File Structure

When creating test files for this project, follow these conventions:

### File Naming
- Test files should end with `_test.h`
- Place test files in `core-test/tests/` directory

### Required Includes
```cpp
#include "../../lib/base-lib/core/<module_dir>/<module_name>.h"  // e.g. core/system/mla_string.h
#include "../../lib/base-lib/test-support/mla_test_executor.h"
```

### Test Function Format

- Use inline void functions for each test
- Name format: [Module][Function]Test()
- Use assert_true() for assertions with descriptive messages
- Test known values with tolerance ranges (e.g., > 0.99999 && < 1.00001)

### Test Registration Function

- Create a `Register[Module]Tests(mla_test_executor_t &p_TestExecutor)` function
- Use `mla_test("TestName", test_category, TestFunction)` to create test objects
- **IMPORTANT**: Always use the framework's pre-defined `test_category` macro (which automatically expands to `mla_test_filename_only`). **NEVER** declare a custom `static const mla_char_t*` or string variable for test category names.
- Register each test with `mla_test_executor_register_test(p_TestExecutor, test)`

Example:
```cpp
inline void RegisterMyModuleTests(mla_test_executor_t &p_TestExecutor) {
    mla_test_t test = mla_test("MyFeature", test_category, MyFeatureTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}
```

### Available Assertions

All assertions take a message parameter as the last argument:
- assert_fail(message) - Force test failure with message
- assert_true(condition, message) - Assert condition is true
- assert_false(condition, message) - Assert condition is false
- assert_equal(actual, expected, message) - Assert values are equal
- assert_struct_equal(Type, actual, expected, message) - Assert structs are equal (requires operator!=)
- assert_not_equal(actual, expected, message) - Assert values are not equal
- assert_null(pointer, message) - Assert pointer is null
- assert_not_null(pointer, message) - Assert pointer is not null

Assertion Examples
```cpp
assert_true(result > 0, "Result should be positive");
assert_equal(value, 42, "Value should be 42");
assert_struct_equal(my_struct_t, actual, expected, "Structs should match");
assert_not_null(pointer, "Pointer should be allocated");
```

## Mandatory Safety Checks

To ensure robust tests, you **must** check the return values of all fallible operations.

### 1. Memory Allocation
Always check pointers after `mla_platform_malloc`:
```cpp
mla_byte_t* buffer = (mla_byte_t*)mla_platform_malloc(size);
assert_not_null(buffer, "Memory allocation failed");
```

### 2. Collection Operations
Always check if adding to a list or map succeeded:
```cpp
assert_true(mla_array_list_add(list, item), "Failed to add item to list");

auto result = mla_hash_map_push(map, key, value);
assert_not_equal(result, MLA_HASH_MAP_PUSH_ERROR, "Failed to push to hash map");
```

### 3. File System Operations
Always check if file or directory operations succeeded:
```cpp
mla_file_system_stream_t stream;
assert_true(mla_fs_open_file(path, mode, stream), "Failed to open file");
```

### Running Created Tests
To execute all test suites (including newly created ones), run:
```bash
./run_all_tests.sh
```
Or for a specific compiler configuration:
```bash
./run_all_tests.sh gcc
```
> [!NOTE]
> `run_all_tests.sh` (via `run_tests_impl.sh`) automatically executes `build_all_impl.sh` to compile the project before running tests.

### Header Guards
Use #ifndef [FILENAME]_H format matching the filename

## Two-Pass Test Execution & Allocation Failure Injection

In the MLA-C framework, every registered unit test runs **twice** by default:
1. **Pass 1 (Normal Run)**: Executes with standard memory allocations.
2. **Pass 2 (Forced Allocation Failure Run)**: Executes with `block_memory_allocations = true`, where all heap allocations (`mla_malloc`, `mla_platform_malloc`) return `NULL` to test error handling and guard against memory leaks/crashes under out-of-memory (OOM) conditions.

### Guidelines for Two-Pass Execution:
- **Empty Path Operations**: String and path construction functions (e.g. `mla_fs_get_complete_os_absolute_path`, `mla_fs_combine_paths`, `mla_string_concat`) return an empty string (`""`) when heap allocations fail in Pass 2.
- **External Task Execution**: Always verify that working directory paths are non-empty (`!mla_string_is_empty(path)`) before invoking external tasks via `mla_external_task_create(cmd, path)`. In test helper setup code, guard against empty paths with an early return:
  ```cpp
  mla_string_t os_path = mla_fs_get_complete_os_absolute_path(dir);
  if (mla_string_is_empty(os_path)) { return; }
  ```

