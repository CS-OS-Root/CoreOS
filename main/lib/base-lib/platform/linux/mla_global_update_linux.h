//
// Created for auto-update package.
//

#ifndef MLA_GLOBAL_UPDATE_LINUX_H
#define MLA_GLOBAL_UPDATE_LINUX_H

#include "../../core/update/mla_update.h"
#include "../../core/filesystem/mla_file_system.h"
#include "../../core/external_task/mla_external_task.h"
#include "../../core/system/mla_string_concat.h"
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>

inline mla_update_error_t mla_private_update_linux_upgrade_to_version(mla_stream_input_t& p_BinaryStream) {
    if (p_BinaryStream.read == nullptr) {
        return MLA_UPDATE_ERROR_INVALID_STREAM;
    }

    char exe_path_buf[1024];
    ssize_t len = readlink("/proc/self/exe", exe_path_buf, sizeof(exe_path_buf) - 1);
    if (len <= 0) {
        return MLA_UPDATE_ERROR_NOT_SUPPORTED;
    }
    exe_path_buf[len] = '\0';
    mla_string_t current_exe_path = mla_string(exe_path_buf);

    mla_string_t temp_binary_path = mla_string_const("/tmp/mla_update_app_tmp");
    if (mla_fs_file_exists(temp_binary_path)) {
        mla_fs_delete_file(temp_binary_path);
    }

    if (!mla_fs_copy_stream_to_file(p_BinaryStream, temp_binary_path)) {
        return MLA_UPDATE_ERROR_WRITE_FAILED;
    }

    mla_c_string_t temp_c_path = mla_string_to_cString(temp_binary_path);
    const mla_char_t* temp_str = mla_c_string_data(temp_c_path);
    if (temp_str != nullptr) {
        chmod(temp_str, 0755);
    }

    mla_string_t cmdline = mla_string_concat(temp_binary_path, mla_string_const(" --mla-apply-update \""));
    cmdline = mla_string_concat(cmdline, current_exe_path);
    cmdline = mla_string_concat(cmdline, mla_string_const("\""));

    mla_external_task_t task = mla_external_task_create(cmdline);
    if (mla_pointer_is_null(task.native_resource)) {
        return MLA_UPDATE_ERROR_SPAWN_FAILED;
    }

    return MLA_UPDATE_SUCCESS;
}

inline mla_bool_t mla_private_update_linux_check_and_apply_pending_update(int argc, char** argv) {
    if (argc < 3 || argv == nullptr) {
        return false;
    }

    mla_string_t target_path = mla_string_empty();
    mla_bool_t is_update_flag_found = false;

    for (int i = 1; i < argc - 1; ++i) {
        if (argv[i] != nullptr && mla_string_equals(mla_string(argv[i]), mla_string_const("--mla-apply-update"))) {
            if (argv[i + 1] != nullptr) {
                target_path = mla_string(argv[i + 1]);
                is_update_flag_found = true;
                break;
            }
        }
    }

    if (!is_update_flag_found || mla_string_length(target_path) == 0) {
        return false;
    }

    char own_exe_buf[1024];
    ssize_t len = readlink("/proc/self/exe", own_exe_buf, sizeof(own_exe_buf) - 1);
    if (len <= 0) {
        return false;
    }
    own_exe_buf[len] = '\0';
    mla_string_t own_temp_path = mla_string(own_exe_buf);

    if (!mla_fs_copy_file_to(own_temp_path, target_path)) {
        return false;
    }

    mla_c_string_t target_c_path = mla_string_to_cString(target_path);
    const mla_char_t* target_str = mla_c_string_data(target_c_path);
    if (target_str != nullptr) {
        chmod(target_str, 0755);
    }

    mla_string_t restart_cmdline = target_path;
    mla_external_task_t task = mla_external_task_create(restart_cmdline);
    if (!mla_pointer_is_null(task.native_resource)) {
        exit(0);
    }

    return true;
}

#if defined(UNIX) && !defined(EMSCRIPTEN) && !defined(APPLE)
mla_update_management_t g_update_management = {
    mla_private_update_linux_upgrade_to_version,
    mla_private_update_linux_check_and_apply_pending_update
};
#endif

#endif // MLA_GLOBAL_UPDATE_LINUX_H
