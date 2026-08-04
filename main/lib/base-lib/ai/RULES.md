# MLA-C Framework Project Rules & Information

You are working in the `mla-core/main` repository, which contains the `mla-c` framework's `base-lib`. This is a specialized C++ framework designed with strict coding conventions.

## Essential Coding Guidelines

1. **No Classes**: This framework uses a C-style architecture. Do not create or use C++ `class` types. Use `struct` combined with static methods or standalone functions.
2. **No Standard Memory Operators**: **NEVER** use the `new` or `delete` keywords. Memory must be managed using the framework's internal tools:
   - For owning heap-allocated data, use `mla_pointer_t` along with `mla_malloc()` or `mla_malloc_struct()`.
   - For basic allocation where `mla_pointer_t` is not needed, use `mla_platform_malloc()` and `mla_platform_free()`.
3. **No Standard Libraries**: Do not use standard C/C++ types or containers (e.g., `std::vector`, `std::string`, `int`, `char`).
   - Use MLA data types defined in `mla_data_types.h` (e.g., `mla_int32_t`, `mla_char_t`).
   - Use MLA framework containers like `mla_array_list_t` and `mla_string_t`.
   - Do not include standard headers like `<string>`, `<vector>`, or `<stdio.h>`.
4. **Data Ownership**: Use `mla_pointer_t` for owning heap-allocated data. It provides automatic reference-counted cleanup. Use `mla_platform_pointer_t` (raw void pointer) strictly for short-lived, non-owning data access.
6. **DOCUMENTATION**: 
    - Write clear and concise documentation for all public APIs and data structures.
    - Use Doxygen-style comments for code documentation.
    - Ensure documentation is up-to-date and accurate.
    - Document the dataflow methodically.
7. **Struct Encapsulation**: Never access struct fields directly in consumer code when access methods exist. Always use the framework API getters/setters (for example, TLS config access methods instead of direct `tls.certificate = ...` writes).
8. **Testing & Definition of Done**: Any behavior change or feature needs tests. As an explicit **Definition of Done**, at the end of every task, ALL unit tests (`./run_all_tests.sh`) and ALL benchmarks (`./run_all_benchmarks.sh`) MUST be executed and pass successfully across all supported compiler toolchains before declaring completion.
9. **Skill & AI Documentation Synchronization**:
   - The canonical location for AI skills is within the respective library repository under `lib/<module-name>/ai/skills/` (e.g. `lib/base-lib/ai/skills/`). The `.agents/skills/` directory is a working copy. Any modification, creation, or deletion of skills in `.agents/skills/` MUST also be mirrored to the corresponding library's `lib/<module-name>/ai/skills/` directory.
   - The root instruction files `AGENTS.md` and `GEMINI.md` are generated copies of `lib/<module-name>/ai/INTRO.md` and `lib/<module-name>/ai/RULES.md`. Any changes to rules or intro documentation MUST be made in `lib/<module-name>/ai/` and synced to `AGENTS.md` and `GEMINI.md`.
10. **Include Placement**: All `#include` directives MUST be placed strictly at the top of source (`.cpp`) and header (`.h`) files, immediately following file headers/comments and header guards. **NEVER** place `#include` directives in the middle of a file, between functions, or inside functions/blocks.
11. **Clean & Maintainable Code**: This project has a very long lifetime. Clean, readable, and maintainable code is of the utmost importance. Every piece of code written should be easy to understand, modify, and extend by future contributors. Avoid clever tricks or overly complex constructs in favor of clarity. Prefer explicit over implicit, and always consider the long-term maintainability of the code over short-term convenience.

## Code Style

1. **Brace Compliance**: Always wrap single-line control statements (`if`, `for`, `while`, `do`) in explicit braces `{ ... }` to comply with `clang-tidy` rules (`readability-braces-around-statements`). Do not write single-line unbraced statements (e.g., `if (cond) return;`).
2. **No Static Functions in CPP Files**: Do not use `static` functions or helper methods in `.cpp` files as they are unnecessary. Use `mla_private_` prefix naming instead:
   - **Incorrect**: `static mla_external_task_t mla_private_external_task_create_internal(...)`
   - **Correct**: `mla_external_task_t mla_private_external_task_create_internal(...)`
3. **Avoid Deeply Nested Control Flow**: Do not write multi-level deeply nested `if` blocks. Use guard clauses, early `continue` or `break` statements in loops, or extract complex block logic into separate helper functions to keep code flat, clean, and maintainable.

## Detailed Instructions / Skills
Detailed instructions about framework modules (array lists, strings, memory, networking, tests, etc.) are available as skills.
