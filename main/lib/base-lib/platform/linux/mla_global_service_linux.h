//
// Created for service platform abstraction module.
//

#ifndef MLA_GLOBAL_SERVICE_LINUX_H
#define MLA_GLOBAL_SERVICE_LINUX_H

#include "../../core/service/mla_service.h"
#include "../../core/system/mla_string_builder.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>

#ifndef MLA_SERVICE_SUPPORTED
#define MLA_SERVICE_SUPPORTED 1
#endif

inline mla_bool_t mla_private_linux_create_dir_recursive(const mla_char_t* p_Path) {
    if (p_Path == nullptr || p_Path[0] == '\0') {
        return false;
    }

    char temp[1024];
    size_t len = strlen(p_Path);
    if (len >= sizeof(temp)) {
        return false;
    }

    memcpy(temp, p_Path, len + 1);

    for (size_t i = 1; i < len; ++i) {
        if (temp[i] == '/') {
            temp[i] = '\0';
            struct stat st;
            if (stat(temp, &st) != 0) {
                if (mkdir(temp, 0755) != 0 && errno != EEXIST) {
                    return false;
                }
            }
            temp[i] = '/';
        }
    }

    struct stat st;
    if (stat(temp, &st) != 0) {
        if (mkdir(temp, 0755) != 0 && errno != EEXIST) {
            return false;
        }
    }

    return true;
}

inline mla_bool_t mla_private_linux_get_service_unit_path(const mla_char_t* p_ServiceName, mla_char_t* p_OutPath, mla_size_t p_OutSize, mla_bool_t& p_OutIsUserMode) {
    if (p_ServiceName == nullptr || p_ServiceName[0] == '\0' || p_OutPath == nullptr || p_OutSize == 0) {
        return false;
    }

    if (geteuid() == 0) {
        p_OutIsUserMode = false;
        int written = snprintf(p_OutPath, p_OutSize, "/etc/systemd/system/%s.service", p_ServiceName);
        return written > 0 && mla_s_cast<mla_size_t>(written) < p_OutSize;
    }

    p_OutIsUserMode = true;

    // 1. Check $XDG_CONFIG_HOME
    const char* xdg_config = getenv("XDG_CONFIG_HOME");
    if (xdg_config != nullptr && xdg_config[0] != '\0') {
        char dir[1024];
        snprintf(dir, sizeof(dir), "%s/systemd/user", xdg_config);
        mla_private_linux_create_dir_recursive(dir);
        int written = snprintf(p_OutPath, p_OutSize, "%s/%s.service", dir, p_ServiceName);
        return written > 0 && mla_s_cast<mla_size_t>(written) < p_OutSize;
    }

    // 2. Check $HOME
    const char* home = getenv("HOME");
    if (home != nullptr && home[0] != '\0') {
        char dir[1024];
        snprintf(dir, sizeof(dir), "%s/.config/systemd/user", home);
        mla_private_linux_create_dir_recursive(dir);
        int written = snprintf(p_OutPath, p_OutSize, "%s/%s.service", dir, p_ServiceName);
        return written > 0 && mla_s_cast<mla_size_t>(written) < p_OutSize;
    }

    // 3. Query user database from /etc/passwd via getpwuid
    struct passwd* pw = getpwuid(geteuid());
    if (pw != nullptr && pw->pw_dir != nullptr && pw->pw_dir[0] != '\0') {
        char dir[1024];
        snprintf(dir, sizeof(dir), "%s/.config/systemd/user", pw->pw_dir);
        mla_private_linux_create_dir_recursive(dir);
        int written = snprintf(p_OutPath, p_OutSize, "%s/%s.service", dir, p_ServiceName);
        return written > 0 && mla_s_cast<mla_size_t>(written) < p_OutSize;
    }

    char dir[1024];
    snprintf(dir, sizeof(dir), "/tmp/systemd/user");
    mla_private_linux_create_dir_recursive(dir);
    int written = snprintf(p_OutPath, p_OutSize, "%s/%s.service", dir, p_ServiceName);
    return written > 0 && mla_s_cast<mla_size_t>(written) < p_OutSize;
}

inline void mla_private_linux_exec_systemctl(const mla_char_t* const* p_Argv) {
    pid_t pid = fork();
    if (pid == 0) {
        int dev_null = open("/dev/null", O_WRONLY);
        if (dev_null >= 0) {
            dup2(dev_null, STDOUT_FILENO);
            dup2(dev_null, STDERR_FILENO);
            close(dev_null);
        }

        // If XDG_RUNTIME_DIR is not set for user-level systemctl, check standard /run/user/<uid>
        if (getenv("XDG_RUNTIME_DIR") == nullptr) {
            char runtime_dir[64];
            snprintf(runtime_dir, sizeof(runtime_dir), "/run/user/%u", mla_s_cast<unsigned int>(getuid()));
            struct stat st;
            if (stat(runtime_dir, &st) == 0 && S_ISDIR(st.st_mode)) {
                setenv("XDG_RUNTIME_DIR", runtime_dir, 0);
            }
        }

        execvp(p_Argv[0], mla_c_cast<char* const*>(p_Argv));
        _exit(1);
    } else if (pid > 0) {
        int status = 0;
        waitpid(pid, &status, 0);
    }
}

inline mla_int32_t mla_private_linux_service_install(const mla_string_t &p_ServiceName, const mla_string_t &p_ServiceArgs) {
    if (mla_string_is_empty(p_ServiceName)) {
        return MLA_SERVICE_ERROR_INVALID_ARGUMENT;
    }

    mla_c_string_t serviceNameCStr = mla_string_to_cString(p_ServiceName);
    const mla_char_t* serviceName = mla_c_string_data(serviceNameCStr);
    if (serviceName == nullptr || serviceName[0] == '\0') {
        return MLA_SERVICE_ERROR_INVALID_ARGUMENT;
    }

    char exe_path[1024];
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (len <= 0) {
        return MLA_SERVICE_ERROR_SYSTEM;
    }
    exe_path[len] = '\0';

    char unit_path[2048];
    mla_bool_t is_user_mode = false;
    if (!mla_private_linux_get_service_unit_path(serviceName, unit_path, sizeof(unit_path), is_user_mode)) {
        return MLA_SERVICE_ERROR_SYSTEM;
    }

    char exec_start[2048];
    if (!mla_string_is_empty(p_ServiceArgs)) {
        mla_c_string_t argsCStr = mla_string_to_cString(p_ServiceArgs);
        const mla_char_t* args = mla_c_string_data(argsCStr);
        snprintf(exec_start, sizeof(exec_start), "%s %s", exe_path, args != nullptr ? args : "");
    } else {
        snprintf(exec_start, sizeof(exec_start), "%s", exe_path);
    }

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == nullptr || cwd[0] == '\0') {
        snprintf(cwd, sizeof(cwd), "/");
    }

    char unit_content[4096];
    int content_len = snprintf(
        unit_content,
        sizeof(unit_content),
        "[Unit]\n"
        "Description=%s Service\n"
        "After=network.target\n\n"
        "[Service]\n"
        "Type=simple\n"
        "WorkingDirectory=%s\n"
        "ExecStart=%s\n"
        "Restart=on-failure\n\n"
        "[Install]\n"
        "WantedBy=default.target\n",
        serviceName,
        cwd,
        exec_start
    );

    if (content_len <= 0 || mla_s_cast<size_t>(content_len) >= sizeof(unit_content)) {
        return MLA_SERVICE_ERROR_SYSTEM;
    }

    int fd = open(unit_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        if (errno == EACCES || errno == EPERM) {
            return MLA_SERVICE_ERROR_PERMISSION_DENIED;
        }
        return MLA_SERVICE_ERROR_SYSTEM;
    }

    ssize_t written = write(fd, unit_content, mla_s_cast<size_t>(content_len));
    close(fd);

    if (written < 0 || written != content_len) {
        unlink(unit_path);
        return MLA_SERVICE_ERROR_SYSTEM;
    }

    if (is_user_mode) {
        const char* const reload_args[] = {"systemctl", "--user", "daemon-reload", nullptr};
        mla_private_linux_exec_systemctl(reload_args);
    } else {
        const char* const reload_args[] = {"systemctl", "daemon-reload", nullptr};
        mla_private_linux_exec_systemctl(reload_args);
    }

    return MLA_SERVICE_SUCCESS;
}

inline mla_int32_t mla_private_linux_service_uninstall(const mla_string_t &p_ServiceName) {
    if (mla_string_is_empty(p_ServiceName)) {
        return MLA_SERVICE_ERROR_INVALID_ARGUMENT;
    }

    mla_c_string_t serviceNameCStr = mla_string_to_cString(p_ServiceName);
    const mla_char_t* serviceName = mla_c_string_data(serviceNameCStr);
    if (serviceName == nullptr || serviceName[0] == '\0') {
        return MLA_SERVICE_ERROR_INVALID_ARGUMENT;
    }

    char unit_path[2048];
    mla_bool_t is_user_mode = false;
    if (!mla_private_linux_get_service_unit_path(serviceName, unit_path, sizeof(unit_path), is_user_mode)) {
        return MLA_SERVICE_ERROR_SYSTEM;
    }

    if (is_user_mode) {
        const char* const stop_args[] = {"systemctl", "--user", "stop", serviceName, nullptr};
        mla_private_linux_exec_systemctl(stop_args);
    } else {
        const char* const stop_args[] = {"systemctl", "stop", serviceName, nullptr};
        mla_private_linux_exec_systemctl(stop_args);
    }

    if (unlink(unit_path) != 0 && errno != ENOENT) {
        if (errno == EACCES || errno == EPERM) {
            return MLA_SERVICE_ERROR_PERMISSION_DENIED;
        }
        return MLA_SERVICE_ERROR_SYSTEM;
    }

    if (is_user_mode) {
        const char* const reload_args[] = {"systemctl", "--user", "daemon-reload", nullptr};
        mla_private_linux_exec_systemctl(reload_args);
    } else {
        const char* const reload_args[] = {"systemctl", "daemon-reload", nullptr};
        mla_private_linux_exec_systemctl(reload_args);
    }

    return MLA_SERVICE_SUCCESS;
}

inline mla_string_t mla_private_linux_service_get_install_summary(const mla_string_t &p_ServiceName, const mla_string_t &p_ServiceArgs) {
    mla_bool_t is_user_mode = (geteuid() != 0);

    mla_string_builder_t sb = mla_string_builder_empty();
    if (is_user_mode) {
        mla_string_builder_append(sb, mla_string_const("Successfully installed user service '"));
        mla_string_builder_append(sb, p_ServiceName);
        if (!mla_string_is_empty(p_ServiceArgs)) {
            mla_string_builder_append(sb, mla_string_const("' with arguments '"));
            mla_string_builder_append(sb, p_ServiceArgs);
            mla_string_builder_append(sb, mla_string_const("'.\n\n"));
        } else {
            mla_string_builder_append(sb, mla_string_const("'.\n\n"));
        }
        mla_string_builder_append(sb, mla_string_const("Service management commands:\n"));
        mla_string_builder_append(sb, mla_string_const("  Start:   systemctl --user start "));
        mla_string_builder_append(sb, p_ServiceName);
        mla_string_builder_append(sb, mla_string_const("\n  Stop:    systemctl --user stop "));
        mla_string_builder_append(sb, p_ServiceName);
        mla_string_builder_append(sb, mla_string_const("\n  Status:  systemctl --user status "));
        mla_string_builder_append(sb, p_ServiceName);
        mla_string_builder_append(sb, mla_string_const("\n  Enable:  systemctl --user enable "));
        mla_string_builder_append(sb, p_ServiceName);
        mla_string_builder_append(sb, mla_string_const("\n\nNote: Installed in user scope (~/.config/systemd/user/). Use 'systemctl --user' instead of 'sudo systemctl'.\n"));
    } else {
        mla_string_builder_append(sb, mla_string_const("Successfully installed system service '"));
        mla_string_builder_append(sb, p_ServiceName);
        if (!mla_string_is_empty(p_ServiceArgs)) {
            mla_string_builder_append(sb, mla_string_const("' with arguments '"));
            mla_string_builder_append(sb, p_ServiceArgs);
            mla_string_builder_append(sb, mla_string_const("'.\n\n"));
        } else {
            mla_string_builder_append(sb, mla_string_const("'.\n\n"));
        }
        mla_string_builder_append(sb, mla_string_const("Service management commands:\n"));
        mla_string_builder_append(sb, mla_string_const("  Start:   sudo systemctl start "));
        mla_string_builder_append(sb, p_ServiceName);
        mla_string_builder_append(sb, mla_string_const("\n  Stop:    sudo systemctl stop "));
        mla_string_builder_append(sb, p_ServiceName);
        mla_string_builder_append(sb, mla_string_const("\n  Status:  sudo systemctl status "));
        mla_string_builder_append(sb, p_ServiceName);
        mla_string_builder_append(sb, mla_string_const("\n  Enable:  sudo systemctl enable "));
        mla_string_builder_append(sb, p_ServiceName);
        mla_string_builder_append(sb, mla_string_const("\n"));
    }

    return mla_string_builder_to_string(sb);
}

const mla_service_platform_t g_mla_service_platform = {
    mla_private_linux_service_install,
    mla_private_linux_service_uninstall,
    mla_private_linux_service_get_install_summary
};

#endif // MLA_GLOBAL_SERVICE_LINUX_H
