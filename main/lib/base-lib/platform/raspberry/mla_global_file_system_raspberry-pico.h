//
// Created by chris on 12/29/2025.
//

#ifndef MLA_GLOBAL_FILE_SYSTEM_RASPBERRY_PICO_H
#define MLA_GLOBAL_FILE_SYSTEM_RASPBERRY_PICO_H

#include "../../core/filesystem/mla_file_system.h"
#include "../../core/mla_data_types.h"

// Raspberry Pi Pico minimal filesystem implementation
// This is a stub implementation as the Pico doesn't have a native filesystem
// For actual file operations, users would need to integrate with LittleFS or SD card libraries

struct mla_file_system_pico_native_t {
    mla_bool_t initialized;
};

mla_bool_t mla_private_file_system_pico_native_file_exists(mla_file_system_t& file_system, const mla_string_t& path) {
    (void)file_system;
    (void)path;
    // Stub implementation - always returns false
    return false;
}

mla_bool_t mla_private_file_system_pico_native_delete_file(mla_file_system_t& file_system, const mla_string_t& path) {
    (void)file_system;
    (void)path;
    // Stub implementation - always returns false
    return false;
}

mla_bool_t mla_private_file_system_pico_native_create_directory(mla_file_system_t& file_system, const mla_string_t& path) {
    (void)file_system;
    (void)path;
    // Stub implementation - always returns false
    return false;
}

mla_bool_t mla_private_file_system_pico_native_directory_exists(mla_file_system_t& file_system, const mla_string_t& path) {
    (void)file_system;
    (void)path;
    // Stub implementation - always returns false
    return false;
}

mla_bool_t mla_private_file_system_pico_native_delete_directory(mla_file_system_t& file_system, const mla_string_t& path) {
    (void)file_system;
    (void)path;
    // Stub implementation - always returns false
    return false;
}

mla_bool_t mla_private_file_system_pico_native_list_files(mla_file_system_t& file_system, const mla_string_t& path, mla_array_list_t<mla_init_struct(mla_string_t)>& out_entries) {
    (void)file_system;
    (void)path;
    (void)out_entries;
    // Stub implementation - returns empty list
    return true;
}

mla_bool_t mla_private_file_system_pico_native_list_directory(mla_file_system_t& file_system, const mla_string_t& path, mla_array_list_t<mla_init_struct(mla_string_t)>& out_entries) {
    (void)file_system;
    (void)path;
    (void)out_entries;
    // Stub implementation - returns empty list
    return true;
}

mla_bool_t mla_private_file_system_pico_native_open_file(mla_file_system_t& file_system, const mla_string_t& path, mla_file_system_file_open_mode mode, mla_file_system_stream_t& out_stream) {
    (void)file_system;
    (void)path;
    (void)mode;
    (void)out_stream;
    // Stub implementation - always fails to open files
    return false;
}

mla_buffer_cleanup_mode mla_private_file_system_pico_native_cleanup(mla_platform_pointer_t data, mla_callback_userdata userData) {
    (void)userData;
    
    mla_file_system_pico_native_t* fs = mla_s_cast<mla_file_system_pico_native_t*>(data);
    
    if (fs == nullptr)
        return CLEAN_UP_SKIP;
    
    return CLEAN_UP_NEEDED;
}

mla_file_system_t mla_private_file_system_pico_native_create() {
    
    mla_file_system_pico_native_t* fs = mla_s_cast<mla_file_system_pico_native_t*>(mla_platform_malloc(sizeof(mla_file_system_pico_native_t)));
    
    if (fs == nullptr)
        return mla_file_system_empty();
    
    mla_memset(fs, 0, sizeof(mla_file_system_pico_native_t));
    fs->initialized = true;
    
    return {
        mla_private_file_system_pico_native_file_exists,
        mla_private_file_system_pico_native_open_file,
        mla_private_file_system_pico_native_delete_file,
        mla_private_file_system_pico_native_list_files,
        mla_private_file_system_pico_native_create_directory,
        mla_private_file_system_pico_native_directory_exists,
        mla_private_file_system_pico_native_delete_directory,
        mla_private_file_system_pico_native_list_directory,
        mla_r_cast<mla_callback_userdata>(fs),
        mla_buffer_reference(fs, true, mla_private_file_system_pico_native_cleanup)
    };
}

mla_file_system_t mla_file_system_native_create_data_restricted(mla_string_t basePath) {
    (void)basePath;
    // Stub implementation for Raspberry Pi Pico
    // Returns a minimal filesystem instance
    return mla_private_file_system_pico_native_create();
}

mla_file_system_t mla_file_system_native_create_global() {
    // Stub implementation for Raspberry Pi Pico
    // Returns a minimal filesystem instance
    return mla_private_file_system_pico_native_create();
}

#endif
