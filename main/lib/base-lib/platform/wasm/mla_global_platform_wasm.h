//
// Created by christian on 8/12/25.
//

#ifndef MLA_GLOBAL_PLATFORM_WASM_H
#define MLA_GLOBAL_PLATFORM_WASM_H

#include "../../core/mla_data_types.h"
#include <unistd.h>
#include <time.h>
#include "../generic/mla_global_platform_generic.h"
#include "../generic/mla_global_update_disabled.h"
#include "../../core/lifecycle/mla_lifecycle_events.h"


void mla_private_wasm_sleep(mla_uint32_t milliseconds) {

    usleep(milliseconds * 1000); // Convert milliseconds to microseconds
}

mla_uint64_t mla_private_wasm_system_time_ms() {
    struct timespec ts;
    // Uses a faster, less precise clock source usually sufficient for millisecond-level timing
    clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
    return (mla_s_cast<mla_uint64_t>(ts.tv_sec) * 1000) + (mla_s_cast<mla_uint64_t>(ts.tv_nsec) / 1000000);
}

// Initialize low-level memory operations with default implementations
mla_low_level_operations_t g_low_level_access ={
    mla_private_generic_memcpy,
    mla_private_generic_memset,
    mla_private_generic_memcmp,
    mla_private_generic_memmove,
    mla_private_generic_strlen,
    mla_private_generic_strstr,
    mla_private_generic_malloc,
    mla_private_generic_free,
    mla_private_generic_on_malloc_failure,
    mla_private_generic_print,
    mla_private_generic_std_read,
    mla_private_generic_strtod,
    mla_private_generic_strtoll,
    mla_private_generic_strtoull,
    mla_private_wasm_sleep,
    mla_private_wasm_system_time_ms,
    nullptr
};


void mla_boot_os_application() {
    // This function can be used to perform any additional bootstrapping
    // required for the OS application, such as initializing logging or other subsystems.

    // Finish boot
    mla_lifecycle_fire_boot_events();
}


#endif
