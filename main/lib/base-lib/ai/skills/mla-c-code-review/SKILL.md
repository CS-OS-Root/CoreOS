---
name: 'mla-c-code-review'
description: 'Guidelines and step-by-step procedure for performing code reviews against MLA-C framework coding rules, GEMINI.md/AGENTS.md guidelines, and module skill conventions'
---

# Code Review Protocol for MLA-C Framework

This skill provides instructions and a comprehensive checklist for performing thorough code reviews in the `mla-c` codebase.

## Code Review Workflow

When conducting a code review of modified or newly added code:

1. **Identify Changed Files**: Use `git diff` or inspect target files to identify all modifications.
2. **Execute Checklist**: Evaluate changes against every section of the compliance checklist below.
3. **Verify Compliance**: Ensure that all rule violations are flagged with specific line numbers and exact remediation instructions.
4. **Validation**: Verify that unit tests (`./run_all_tests.sh`) and benchmarks (`./run_all_benchmarks.sh`) build and pass cleanly across toolchains.

---

## Code Review Compliance Checklist

### 1. Architecture & C-Style Design Rules
- [ ] **No C++ Classes**: Code MUST use `struct` with static methods or standalone functions. No `class` keyword.
- [ ] **No Standard Memory Operators**: No `new` or `delete` keywords anywhere.
  - Heap data owned by reference count: use `mla_pointer_t` with `mla_malloc()` or `mla_malloc_struct()`.
  - Non-reference-counted raw allocation: use `mla_platform_malloc()` and `mla_platform_free()`.
- [ ] **No Standard C/C++ Libraries**:
  - NO `#include <string>`, `<vector>`, `<stdlib.h>`, `<stdio.h>`, `<iostream>`, `<memory>`, `<map>`, `<set>`, etc.
  - Standard types (`int`, `char`, `float`, `size_t`, `bool`) are forbidden in core framework code — use MLA types (`mla_int32_t`, `mla_char_t`, `mla_size_t`, `mla_bool_t`). Platform modules (code under `platform/` or OS abstraction headers) are permitted to use standard platform C primitives (`int`, `char`, `const char*`) as required by native OS APIs.
  - Use MLA containers (`mla_string_t`, `mla_array_list_t`, `mla_hash_map_t`).
- [ ] **Data Ownership**:
  - Use `mla_pointer_t` for owning heap-allocated data.
  - Use `mla_platform_pointer_t` (raw void pointer) strictly for short-lived, non-owning data access.

### 2. File Organization & Syntax Conventions
- [ ] **Include Directive Placement**: All `#include` directives MUST be placed strictly at the top of source (`.cpp`) and header (`.h`) files, immediately following file headers/comments and header guards. NEVER place `#include` inside functions, between functions, or in the middle of a file.
- [ ] **Brace Compliance (`clang-tidy`)**: Always wrap single-line control statements (`if`, `for`, `while`, `do`) in explicit braces `{ ... }`. No single-line unbraced statements (e.g. `if (cond) return;` is invalid).
- [ ] **No Static Functions in `.cpp` Files**: Helper functions in `.cpp` files MUST NOT use the `static` keyword. Use the `mla_private_` prefix naming convention instead (e.g., `mla_private_helper_func`).
- [ ] **Avoid Deeply Nested Control Flow**: Use guard clauses and early `return`/`continue`/`break` to keep functions flat and maintainable.
- [ ] **User Data ID Initialization**: Initialize internal `mla_user_data_id` keys inside `.cpp` implementation files using `mla_user_data_id_init(key_name)`. NEVER declare implementation-private `mla_user_data_id` keys as `extern` in public `.h` header files.

### 3. Encapsulation & Safety
- [ ] **Struct Encapsulation**: Do not access struct fields directly in consumer code when framework getters/setters exist. Always use API functions (e.g., `mla_network_tls_config_set_server_name`).
- [ ] **Fallible Operation Checks**: Always check return values of fallible operations (allocations, socket ops, list additions, file system calls).
- [ ] **Casts**: Use explicit MLA cast wrappers (`mla_r_cast`, `mla_s_cast`, `mla_c_cast`) rather than raw C/C++ casts.

### 4. Skill & AI Documentation Synchronization
- [ ] **Skill Mirroring**: Any skill file created, modified, or deleted under `.agents/skills/` MUST also be mirrored to `lib/<module-name>/ai/skills/` (and vice-versa).
- [ ] **Rules Mirroring**: Any changes to rules or intro documentation MUST be made in `lib/<module-name>/ai/` and synced to `AGENTS.md` and `GEMINI.md`.

### 5. Definition of Done & Verification
- [ ] **Automated Tests**: Execute `./run_all_tests.sh` across toolchains and confirm 0 failed tests.
- [ ] **Benchmarks**: Execute `./run_all_benchmarks.sh` across toolchains and confirm all benchmark suites complete cleanly.
