set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/build/default-toolchain.cmake")

cmake_minimum_required(VERSION 3.25)
project(mla-base-lib)

include("${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/build/default-compile-config.cmake")


include("${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/sources.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/test-support/sources.cmake")


##########################################################
## ZLIB Detection
##########################################################
find_package(ZLIB)
if(ZLIB_FOUND)
    message(STATUS "zlib found: ${ZLIB_VERSION_STRING} - zlib-dependent tests will be enabled")
else()
    message(STATUS "zlib not found - zlib-dependent tests will be disabled")
endif()

# Helper: link zlib and set the feature flag compile definition for a test target.
# When zlib is found, links ZLIB::ZLIB and defines mla_test_featureflag_zlib=1.
# When zlib is not found, the feature flag defaults to 0 (see mla_test_global_config.h).
function(configure_zlib_for_target target_name)
    if(ZLIB_FOUND)
        target_link_libraries(${target_name} PRIVATE ZLIB::ZLIB)
        target_compile_definitions(${target_name} PRIVATE mla_test_featureflag_zlib=1)
    endif()
endfunction()

# Helper: link OpenSSL and set feature flag compile definition for target
function(configure_openssl_for_target target_name)
    if(OPENSSL_SSL_LIB AND OPENSSL_CRYPTO_LIB AND OPENSSL_INCLUDE_DIR AND NOT (CMAKE_CXX_COMPILER MATCHES "fil\\+\\+"))
        target_link_libraries(${target_name} PRIVATE ${OPENSSL_SSL_LIB} ${OPENSSL_CRYPTO_LIB})
        target_include_directories(${target_name} PRIVATE ${OPENSSL_INCLUDE_DIR})
        target_compile_definitions(${target_name} PRIVATE MLA_HAS_OPENSSL=1)
    endif()
endfunction()


##########################################################
## LINUX
##########################################################

if(UNIX AND NOT APPLE AND NOT EMSCRIPTEN)

    find_path(OPENSSL_INCLUDE_DIR NAMES openssl/ssl.h PATHS /usr/include /usr/local/include /opt/emsdk/node/24.19.0_64bit/include/node /opt/emsdk/node/22.16.0_64bit/include/node)
    find_library(OPENSSL_SSL_LIB NAMES ssl libssl libssl.so.3 libssl.so PATHS /usr/lib/x86_64-linux-gnu /usr/lib)
    find_library(OPENSSL_CRYPTO_LIB NAMES crypto libcrypto libcrypto.so.3 libcrypto.so PATHS /usr/lib/x86_64-linux-gnu /usr/lib)

    ## TEST SINGLE THREAD
    ############################
    set(SOURCE_FILES_LINUX_SINGLE_THREAD
            core-test/main_test_linux_single_thread.cpp
            ${SOURCE_FILES}
            ${SOURCE_EXTERNAL_TASK_FILES}
            ${SOURCE_NETWORK_FILES}
            ${SOURCE_FILE_SYSTEM_FILES}
            ${SOURCE_UPDATE_FILES}
            ${SOURCE_TEST_FILES}
    )
    add_executable(MLA_C_Test_Linux_Single_Thread ${SOURCE_FILES_LINUX_SINGLE_THREAD})
    target_link_libraries(MLA_C_Test_Linux_Single_Thread PRIVATE ${CMAKE_DL_LIBS})
    configure_zlib_for_target(MLA_C_Test_Linux_Single_Thread)
    configure_openssl_for_target(MLA_C_Test_Linux_Single_Thread)

    ## LINUX TEST MULTI THREAD
    ############################
    set(SOURCE_FILES_LINUX_MULTI_THREAD
            core-test/main_test_linux_multi_thread.cpp
            ${SOURCE_FILES}
            ${SOURCE_EXTERNAL_TASK_FILES}
            ${SOURCE_NETWORK_FILES}
            ${SOURCE_FILE_SYSTEM_FILES}
            ${SOURCE_UPDATE_FILES}
            ${SOURCE_TEST_FILES}
    )
    add_executable(MLA_C_Test_Linux_Multi_Thread ${SOURCE_FILES_LINUX_MULTI_THREAD})
    mla_enable_clang_tidy(MLA_C_Test_Linux_Multi_Thread)
    target_link_libraries(MLA_C_Test_Linux_Multi_Thread PRIVATE ${CMAKE_DL_LIBS})
    configure_zlib_for_target(MLA_C_Test_Linux_Multi_Thread)
    configure_openssl_for_target(MLA_C_Test_Linux_Multi_Thread)

    ## APP
    ############################
    set(SOURCE_FILES_LINUX_APP
            ${CMAKE_CURRENT_SOURCE_DIR}/core-example-app/main_app_linux.cpp
            ${SOURCE_FILES}
            ${SOURCE_NETWORK_FILES}
            ${SOURCE_UI_FILES}
            ${SOURCE_FILE_SYSTEM_FILES}
            ${SOURCE_EXTERNAL_TASK_FILES}
            ${SOURCE_UPDATE_FILES}
    )
    add_executable(MLA_C_App_Linux ${SOURCE_FILES_LINUX_APP})
    target_link_libraries(MLA_C_App_Linux PRIVATE ${CMAKE_DL_LIBS})
    configure_openssl_for_target(MLA_C_App_Linux)

endif()

##########################################################
## WASM
##########################################################

if (MLA_EMSDK_PATH AND NOT MLA_WASM_STANDALONE)
    ## TEST SINGLE THREAD
    ############################
    set(SOURCE_FILES_WASM_SINGLE_THREAD
            core-test/main_test_wasm.cpp
            ${SOURCE_FILES}
            ${SOURCE_FILE_SYSTEM_FILES}
            ${SOURCE_UPDATE_FILES}
            ${SOURCE_TEST_FILES}
    )
    add_executable(MLA_C_Test_WASM_Single_Thread ${SOURCE_FILES_WASM_SINGLE_THREAD})
    mla_enable_clang_tidy(MLA_C_Test_WASM_Single_Thread)
    target_link_libraries(MLA_C_Test_WASM_Single_Thread)

    # Add target-specific compile definition to disable network for WASM test only
    target_compile_definitions(MLA_C_Test_WASM_Single_Thread PRIVATE mla_test_disable_network=1 mla_test_disable_file_system=1 mla_test_disable_external_task=1)

    ## APP
    ############################
    set(SOURCE_FILES_WASM_APP
            core-example-app/main_app_wasm.cpp
            ${SOURCE_FILES}
            ${SOURCE_NETWORK_FILES}
            ${SOURCE_UI_FILES}
            ${SOURCE_FILE_SYSTEM_FILES}
            ${SOURCE_UPDATE_FILES}
    )
    add_executable(MLA_C_App_WASM ${SOURCE_FILES_WASM_APP})
    target_link_libraries(MLA_C_App_WASM)
endif()

##########################################################
## WASM STANDALONE
##########################################################

## TEST SINGLE THREAD STANDALONE
############################
if (MLA_WASM_STANDALONE OR MLA_EMSDK_PATH)
    set(SOURCE_FILES_WASM_SINGLE_THREAD_STANDALONE
            core-test/main_test_wasm_standalone.cpp
            ${SOURCE_FILES}
            ${SOURCE_FILE_SYSTEM_FILES}
            ${SOURCE_UPDATE_FILES}
            ${SOURCE_TEST_FILES}
    )
    add_executable(MLA_C_Test_WASM_Single_Thread_Standalone ${SOURCE_FILES_WASM_SINGLE_THREAD_STANDALONE})
    mla_enable_clang_tidy(MLA_C_Test_WASM_Single_Thread_Standalone)
    target_link_libraries(MLA_C_Test_WASM_Single_Thread_Standalone)
    if (EMSCRIPTEN)
        target_link_options(MLA_C_Test_WASM_Single_Thread_Standalone PRIVATE "-sERROR_ON_UNDEFINED_SYMBOLS=0")
    endif()

    # Add target-specific compile definition to disable network for WASM test only
    target_compile_definitions(MLA_C_Test_WASM_Single_Thread_Standalone PRIVATE mla_test_disable_network=1 mla_test_disable_file_system=1 mla_benchmark_std=0 mla_benchmark_cstring=0 mla_test_disable_external_task=1)
endif()

##########################################################
## WINDOWS
##########################################################

if(WIN32)
    ## TEST SINGLE THREAD
    ############################
    set(SOURCE_FILES_WINDOWS_SINGLE_THREAD
            core-test/main_test_windows_single_thread.cpp
            ${SOURCE_FILES}
            ${SOURCE_NETWORK_FILES}
            ${SOURCE_FILE_SYSTEM_FILES}
            ${SOURCE_EXTERNAL_TASK_FILES}
            ${SOURCE_UPDATE_FILES}
            ${SOURCE_TEST_FILES}
    )
    add_executable(MLA_C_Test_Windows_Single_Thread ${SOURCE_FILES_WINDOWS_SINGLE_THREAD})
    mla_enable_clang_tidy(MLA_C_Test_Windows_Single_Thread)
    target_link_libraries(MLA_C_Test_Windows_Single_Thread PRIVATE
            ws2_32
            Iphlpapi
            dbghelp
            # ... other libraries
    )
    configure_zlib_for_target(MLA_C_Test_Windows_Single_Thread)

    ## TEST MULTI THREAD
    ############################
    set(SOURCE_FILES_WINDOWS_MULTI_THREAD
            core-test/main_test_windows_multi_thread.cpp
            ${SOURCE_FILES}
            ${SOURCE_NETWORK_FILES}
            ${SOURCE_FILE_SYSTEM_FILES}
            ${SOURCE_EXTERNAL_TASK_FILES}
            ${SOURCE_UPDATE_FILES}
            ${SOURCE_TEST_FILES}
    )
    add_executable(MLA_C_Test_Windows_Multi_Thread ${SOURCE_FILES_WINDOWS_MULTI_THREAD})
    mla_enable_clang_tidy(MLA_C_Test_Windows_Multi_Thread)
    target_link_libraries(MLA_C_Test_Windows_Multi_Thread PRIVATE
            ws2_32
            Iphlpapi
            dbghelp
            # ... other libraries
    )
    configure_zlib_for_target(MLA_C_Test_Windows_Multi_Thread)

    ## APP
    ############################
    set(SOURCE_FILES_WINDOWS_APP
            ${CMAKE_CURRENT_SOURCE_DIR}/core-example-app/main_app_windows.cpp
            ${SOURCE_FILES}
            ${SOURCE_NETWORK_FILES}
            ${SOURCE_UI_FILES}
            ${SOURCE_FILE_SYSTEM_FILES}
            ${SOURCE_EXTERNAL_TASK_FILES}
            ${SOURCE_UPDATE_FILES}
    )
    add_executable(MLA_C_App_Windows ${SOURCE_FILES_WINDOWS_APP})
    mla_enable_clang_tidy(MLA_C_App_Windows)
    target_link_libraries(MLA_C_App_Windows PRIVATE
            ws2_32
            Iphlpapi
            dbghelp

            # For openGL
            #opengl32

            # For Direct2D
            d2d1.lib
            dwrite.lib
            windowscodecs.lib

            #For Gdi+
            #gdiplus.lib

            # ... other libraries
    )
endif()
