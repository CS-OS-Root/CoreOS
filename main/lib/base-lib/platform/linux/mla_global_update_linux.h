//
// Created for auto-update package.
//

#ifndef MLA_GLOBAL_UPDATE_LINUX_H
#define MLA_GLOBAL_UPDATE_LINUX_H

#include "../../core/update/mla_update.h"
#include "../../core/filesystem/mla_file_system.h"
#include "../../core/external_task/mla_external_task.h"
#include "../../core/system/mla_string_concat.h"
#include "mla_global_file_system_linux.h"
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

    out = mla_stream_noop_output();
    out_fs_stream = mla_file_system_stream_empty();

    mla_c_string_t temp_c_path = mla_string_to_cString(temp_binary_path);
    const mla_char_t* temp_str = mla_c_string_data(temp_c_path);
    if (temp_str != nullptr) {
        chmod(temp_str, 0755);
    }

    mla_string_t cmdline = mla_string_concat(temp_binary_path, mla_string_const(" --mla-apply-update \""));
    cmdline = mla_string_concat(cmdline, current_exe_path);
    cmdline = mla_string_concat(cmdline, mla_string_const("\""));

    pid_t pid = fork();
    if (pid < 0) {
        return MLA_UPDATE_ERROR_SPAWN_FAILED;
    }

    if (pid == 0) {
        setsid();
        mla_c_string_t cmdlineCStr = mla_string_to_cString(cmdline);
        const mla_char_t* cmdline_str = mla_c_string_data(cmdlineCStr);
        if (cmdline_str != nullptr) {
            execl("/bin/sh", "sh", "-c", cmdline_str, nullptr);
        }
        _exit(1);
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

    char own_exe_buf[1024];
    ssize_t len = readlink("/proc/self/exe", own_exe_buf, sizeof(own_exe_buf) - 1);
    if (len <= 0) {
        return false;
    }
    own_exe_buf[len] = '\0';
    mla_string_t own_temp_path = mla_string(own_exe_buf);

    mla_file_system_t fs = mla_file_system_native_create_global();
    mla_file_system_stream_t in_fs_stream = mla_file_system_stream_empty();
    mla_file_system_stream_t out_fs_stream = mla_file_system_stream_empty();

    mla_c_string_t target_c_path = mla_string_to_cString(target_path);
    const mla_char_t* target_str = mla_c_string_data(target_c_path);

    if (target_str != nullptr) {
        unlink(target_str);
    }

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

    in = mla_stream_noop_input();
    out = mla_stream_noop_output();
    in_fs_stream = mla_file_system_stream_empty();
    out_fs_stream = mla_file_system_stream_empty();

    if (target_str != nullptr) {
        chmod(target_str, 0755);
    }

    pid_t restart_pid = fork();
    if (restart_pid == 0) {
        setsid();
        if (target_str != nullptr) {
            execl(target_str, target_str, nullptr);
        }
        _exit(1);
    }

    exit(0);
}

mla_update_management_t g_update_management = {
    mla_private_update_linux_upgrade_to_version
};

#endif // MLA_GLOBAL_UPDATE_LINUX_H
