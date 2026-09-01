---
name: coreos-create-test
description: Guide for creating test files and registering tests in the MLA framework. Use when adding unit tests, writing assertions, or setting up test infrastructure for CoreOS modules.
metadata:
  author: coreos
  version: "1.0"
  converted-from: ai/instructions/create_a_test.instructions.md
---

# Creating Tests

Provide project context and coding guidelines that AI should follow when generating code, answering questions, or reviewing changes.

## Test File Structure

When creating test files for this project, follow these conventions:

### File Naming
- Test files should end with `_test.h`
- Place test files in `core-os-test/` directory

### Required Includes
```cpp
#include "../core-os/utils/[module_name].h"
#include "../core-os-test-support/mla_test_executor.h"
```

### Test Function Format

- Use inline void functions for each test
- Name format: [Module][Function]Test()
- Use assert_true() for assertions with descriptive messages
- Test known values with tolerance ranges (e.g., > 0.99999 && < 1.00001)

### Test Registration Function

- Create a Register[Module]Tests(mla_test_executor_t &p_TestExecutor) function
- Use mla_test() to create test objects with: name, test_category, and test function
- Register each test with mla_test_executor_register_test(p_TestExecutor, test)

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

### Assertion Examples
```cpp
assert_true(result > 0, "Result should be positive");
assert_equal(value, 42, "Value should be 42");
assert_struct_equal(my_struct_t, actual, expected, "Structs should match");
assert_not_null(pointer, "Pointer should be allocated");
```

### Header Guards
Use #ifndef [FILENAME]_H format matching the filename

### Complete Test File Example

```cpp
#ifndef MLA_MY_FEATURE_TEST_H
#define MLA_MY_FEATURE_TEST_H

#include "../core-os/my_feature/mla_my_feature.h"
#include "../core-os-test-support/mla_test_executor.h"

inline void MyFeatureCreateTest() {
    mla_my_feature_t feature = mla_my_feature(mla_string_const("test"), 42);
    assert_true(mla_my_feature_is_valid(feature), "Feature should be valid");
    assert_equal(feature.value, 42, "Value should be 42");
}

inline void MyFeatureInvalidTest() {
    mla_my_feature_t feature = mla_my_feature_invalid();
    assert_false(mla_my_feature_is_valid(feature), "Invalid feature should not be valid");
}

inline void RegisterMyFeatureTests(mla_test_executor_t &p_TestExecutor) {
    mla_test_t test = mla_test("MyFeatureCreate", test_category, MyFeatureCreateTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("MyFeatureInvalid", test_category, MyFeatureInvalidTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}

#endif
```
