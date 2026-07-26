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

## Detailed Instructions / Skills
Detailed instructions about framework modules (array lists, strings, memory, networking, tests, etc.) are available as skills.
