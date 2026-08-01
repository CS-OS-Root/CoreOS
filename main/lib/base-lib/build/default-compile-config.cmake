if (MLA_WASM_STANDALONE)

    if (CMAKE_CXX_COMPILER MATCHES "zig")

        set(CMAKE_EXECUTABLE_SUFFIX ".wasm")

        # Basic flags for Zig WASM
        add_compile_options(-Wall -Wextra -Wpedantic -Werror -fno-exceptions -target wasm32-freestanding)

        # 1. -target wasm32-freestanding: Set linker target
        # 2. -Wl,--no-entry: Don't look for _start (because main returns int, which is invalid for WASM start)
        # 3. -Wl,--export=main: Make main available to be called by JavaScript
        add_link_options(
                -target wasm32-freestanding
                -Wl,--no-entry
                -Wl,--export=main
        )

        # Fix: Even on Generic systems, CMake might try to link standard C libs if checks fail.
        # Explicitly empty them for freestanding WASM.
        set(CMAKE_C_STANDARD_LIBRARIES "" CACHE INTERNAL "")
        set(CMAKE_CXX_STANDARD_LIBRARIES "" CACHE INTERNAL "")

        message(STATUS "Configured Zig Compiler for Standalone WASM Build  ${CMAKE_CXX_COMPILER}")
    else()
        # Zig compiler does not support standalone wasm builds yet
        message(FATAL_ERROR "Standalone WASM builds are only supported with the Zig compiler at this time.")
    endif()

elseif (MLA_EMSDK_PATH)

    # Set Option for smallest possible output

    if (MLA_WASM_STANDALONE)

        # Create standalone WASM binary

        set(CMAKE_EXECUTABLE_SUFFIX ".wasm")

        # Enable all warnings and treat them as errors
        # Disable exceptions to reduce binary size
        add_compile_options(-Wall -Wextra -Wpedantic -Werror -fno-exceptions -ffreestanding)

        add_link_options(
                -nostdlib
                -ffreestanding
                -Wl,--no-entry
                -Wl,--export=main
        )

        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sMALLOC=none -sSTANDALONE_WASM=1 -sNO_FILESYSTEM=1 -sASSERTIONS=0")
        message(STATUS "Configured Emscripten Compiler for Standalone WASM  ${CMAKE_CXX_COMPILER}")
    elseif (MLA_JS_STANDALONE)

        # Create standalone JS

        set(CMAKE_EXECUTABLE_SUFFIX ".js")

        # Enable all warnings and treat them as errors
        # Disable exceptions to reduce binary size
        add_compile_options(-Wall -Wextra -Wpedantic -Werror -fno-exceptions)

        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sMALLOC=emmalloc -sALLOW_MEMORY_GROWTH=1 -sWASM=0 -sNO_FILESYSTEM=1 -sASSERTIONS=0 -sERROR_ON_UNDEFINED_SYMBOLS=0")
        message(STATUS "Configured Emscripten Compiler for Standalone JS ${CMAKE_CXX_COMPILER}")
    else()

        # Create WASM binary that depends on Emscripten runtime

        # Enable all warnings and treat them as errors
        # Disable exceptions to reduce binary size
        add_compile_options(-Wall -Wextra -Wpedantic -Werror -fno-exceptions)

        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sMALLOC=emmalloc -sALLOW_MEMORY_GROWTH=1 -sSTANDALONE_WASM=1 -sNO_FILESYSTEM=1 -sASSERTIONS=0")
        message(STATUS "Configured Emscripten Compiler ${CMAKE_CXX_COMPILER}")
    endif()

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

    # Enable all warnings and treat them as errors
    # Disable exceptions to reduce binary size
    add_compile_options(/W4 /wd4996 /wd4127 /EHsc)

    # Set Options for static linking libc++.dll and libstdc++.dll and libwinpthread-1.dll Even if we dont need it
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")
    message(STATUS "Configured MSVC Compiler ${CMAKE_CXX_COMPILER}")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER MATCHES "fil" OR CMAKE_CXX_COMPILER MATCHES "filc")

    # Enable all warnings and treat them as errors
    # Disable exceptions to reduce binary size
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -fno-exceptions)

    message(STATUS "Configured Clang Compiler ${CMAKE_CXX_COMPILER}")

    # Zig/Filc compiler uses clang but does not support address sanitizer
    if(NOT (CMAKE_CXX_COMPILER MATCHES "zig" OR CMAKE_CXX_COMPILER MATCHES "fil" OR CMAKE_CXX_COMPILER MATCHES "filc")  AND CMAKE_BUILD_TYPE MATCHES Debug)
        message("Enabling Address Sanitizer for Debug Build")
        add_compile_options(-fsanitize=address)
        add_link_options(-fsanitize=address)
    endif()

    if(CMAKE_CXX_COMPILER MATCHES "fil" OR CMAKE_CXX_COMPILER MATCHES "filc")
        add_compile_options(-Qunused-arguments -Dmla_use_stacktraces=0)
    endif()

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

    # Enable all warnings and treat them as errors
    # Disable exceptions to reduce binary size
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -fno-exceptions)

    message(STATUS "Configured GNU Compiler ${CMAKE_CXX_COMPILER}")

    if(LINUX AND CMAKE_BUILD_TYPE MATCHES Debug)
        message("Enabling Address Sanitizer for Debug Build")
        add_compile_options(-fsanitize=address)
        add_link_options(-fsanitize=address)
    endif()

else()
    message(FATAL_ERROR "Unsupported Compiler. ${CMAKE_CXX_COMPILER_ID} is not supported.")
endif()