//
// Created for auto-update package.
//

#ifndef MLA_GLOBAL_UPDATE_WINDOWS_H
#define MLA_GLOBAL_UPDATE_WINDOWS_H

#include "../../core/update/mla_update.h"
#include "../../core/filesystem/mla_file_system.h"
#include "../../core/external_task/mla_external_task.h"
#include "mla_global_file_system_windows.h"
#include <windows.h>

inline mla_update_error_t mla_private_update_windows_upgrade_to_version(mla_stream_input_t& p_BinaryStream) {
    if (p_BinaryStream.read == nullptr) {
        return MLA_UPDATE_ERROR_INVALID_STREAM;
    }

    char exe_path_buf[MAX_PATH];
    DWORD len = GetModuleFileNameA(nullptr, exe_path_buf, MAX_PATH);
    if (len == 0) {
        return MLA_UPDATE_ERROR_NOT_SUPPORTED;
    }
    mla_string_t current_exe_path = mla_string(exe_path_buf);

    char temp_dir[MAX_PATH];
    DWORD temp_len = GetTempPathA(MAX_PATH, temp_dir);
    if (temp_len == 0) {
        return MLA_UPDATE_ERROR_WRITE_FAILED;
    }
    mla_string_t temp_binary_path = mla_fs_combine_paths(mla_string(temp_dir), mla_string_const("mla_update_app_tmp.exe"));

    mla_file_system_t fs = mla_file_system_native_create_global();
    if (fs.file_exists != nullptr && fs.file_exists(fs, temp_binary_path)) {
        if (fs.delete_file != nullptr) {
            fs.delete_file(fs, temp_binary_path);
        }
    }

    mla_file_system_stream_t out_fs_stream = mla_file_system_stream_empty();
    if (fs.open_file == nullptr || !fs.open_file(fs, temp_binary_path, MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE, out_fs_stream)) {
        return MLA_UPDATE_ERROR_WRITE_FAILED;
    }

    mla_stream_output_t out = mla_file_system_stream_as_output(out_fs_stream);
    mla_byte_t buffer[8192];
    mla_size_t read_bytes = 0;
    while ((read_bytes = p_BinaryStream.read(p_BinaryStream, 0, sizeof(buffer), buffer)) > 0) {
        out.write(out, 0, read_bytes, buffer);
    }

    // Close the Steam
    out_fs_stream = mla_file_system_stream_empty();

    mla_string_t cmdline = mla_string_concat(temp_binary_path, mla_string_const(" --mla-apply-update \""));
    cmdline = mla_string_concat(cmdline, current_exe_path);
    cmdline = mla_string_concat(cmdline, mla_string_const("\""));

    mla_external_task_t task = mla_external_task_create(cmdline);
    if (mla_pointer_is_null(task.native_resource)) {
        return MLA_UPDATE_ERROR_SPAWN_FAILED;
    }

    return MLA_UPDATE_SUCCESS;
}

inline mla_bool_t mla_update_check_and_apply_pending_update(int argc, char** argv) {
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

    char own_exe_buf[MAX_PATH];
    DWORD len = GetModuleFileNameA(nullptr, own_exe_buf, MAX_PATH);
    if (len == 0) {
        return false;
    }
    mla_string_t own_temp_path = mla_string(own_exe_buf);

    mla_file_system_t fs = mla_file_system_native_create_global();
    mla_file_system_stream_t in_fs_stream = mla_file_system_stream_empty();
    mla_file_system_stream_t out_fs_stream = mla_file_system_stream_empty();

    if (fs.open_file == nullptr ||
        !fs.open_file(fs, own_temp_path, MLA_FILE_SYSTEM_FILE_OPEN_MODE_READ, in_fs_stream) ||
        !fs.open_file(fs, target_path, MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE, out_fs_stream)) {
        return false;
    }

    mla_stream_input_t in = mla_file_system_stream_as_input(in_fs_stream);
    mla_stream_output_t out = mla_file_system_stream_as_output(out_fs_stream);
    mla_byte_t buffer[8192];
    mla_size_t read_bytes = 0;
    while ((read_bytes = in.read(in, 0, sizeof(buffer), buffer)) > 0) {
        out.write(out, 0, read_bytes, buffer);
    }

    // Close the files
    in_fs_stream = mla_file_system_stream_empty();
    out_fs_stream = mla_file_system_stream_empty();

    mla_string_t restart_cmdline = target_path;
    mla_external_task_t task = mla_external_task_create(restart_cmdline);
    if (!mla_pointer_is_null(task.native_resource)) {
        ExitProcess(0);
    }

    return true;
}

mla_update_management_t g_update_management = {
    mla_private_update_windows_upgrade_to_version
};

#endif // MLA_GLOBAL_UPDATE_WINDOWS_H
