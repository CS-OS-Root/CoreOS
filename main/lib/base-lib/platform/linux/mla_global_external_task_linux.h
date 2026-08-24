//
// Created by Copilot on 5/15/2026.
//

#ifndef MLA_GLOBAL_EXTERNAL_TASK_LINUX_H
#define MLA_GLOBAL_EXTERNAL_TASK_LINUX_H

#include "../../core/external_task/mla_external_task.h"
#include "../../core/system/mla_string.h"
#include "../../core/mla_native_resource.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/prctl.h>

struct mla_private_linux_external_task_native_resource_t {
    pid_t pid;
    mla_int32_t stdin_write_fd;
    mla_int32_t stdout_read_fd;
    mla_int32_t exit_code;
    mla_bool_t is_stopped;

    static mla_private_linux_external_task_native_resource_t init() {
        return {
            -1, // pid
            -1, // stdin_write_fd
            -1, // stdout_read_fd
            -1, // exit_code
            false // is_stopped
        };
    }

    static void clean_up_resource(mla_private_linux_external_task_native_resource_t& self) {

        if (self.stdin_write_fd >= 0) {
            close(self.stdin_write_fd);
            self.stdin_write_fd = -1;
        }

        if (self.stdout_read_fd >= 0) {
            close(self.stdout_read_fd);
            self.stdout_read_fd = -1;
        }

        if (self.pid > 0) {
            kill(self.pid, SIGTERM);
            waitpid(self.pid, nullptr, 0);
            self.pid = -1;
        }
    }
};

void mla_private_linux_external_task_child_fail(mla_int32_t p_StatusPipeFd) {
    mla_uint8_t childError = 1;
    ssize_t bytes_written = write(p_StatusPipeFd, &childError, sizeof(childError));
    (void)bytes_written;
    kill(getpid(), SIGKILL);
    while (true) {
        pause();
    }
}

void mla_private_linux_external_task_cleanup_process_data(mla_private_linux_external_task_native_resource_t* p_ProcessData) {

    if (p_ProcessData == nullptr) {
        return;
    }

    if (p_ProcessData->stdin_write_fd >= 0) {
        close(p_ProcessData->stdin_write_fd);
        p_ProcessData->stdin_write_fd = -1;
    }

    if (p_ProcessData->stdout_read_fd >= 0) {
        close(p_ProcessData->stdout_read_fd);
        p_ProcessData->stdout_read_fd = -1;
    }
}

mla_private_linux_external_task_native_resource_t* mla_linux_external_task_get_process_data(const mla_pointer_t& p_TaskResource) {

    return mla_pointer_get_data<mla_private_linux_external_task_native_resource_t>(p_TaskResource);
}

mla_bool_t mla_private_linux_external_task_create_process(mla_pointer_t& p_OutTaskResource, const mla_string_t& p_CmdLine, const mla_string_t& p_WorkingDirectory) {

    p_OutTaskResource = mla_pointer_null();

    if (mla_string_is_empty(p_CmdLine)) {
        return false;
    }

    int stdinPipe[2] = {-1, -1};
    int stdoutPipe[2] = {-1, -1};
    int statusPipe[2] = {-1, -1};

    if (pipe(stdinPipe) != 0) {
        return false;
    }

    if (pipe(stdoutPipe) != 0) {
        close(stdinPipe[0]);
        close(stdinPipe[1]);
        return false;
    }

    if (pipe(statusPipe) != 0) {
        close(stdinPipe[0]);
        close(stdinPipe[1]);
        close(stdoutPipe[0]);
        close(stdoutPipe[1]);
        return false;
    }

    if (fcntl(statusPipe[1], F_SETFD, FD_CLOEXEC) != 0) {
        close(stdinPipe[0]);
        close(stdinPipe[1]);
        close(stdoutPipe[0]);
        close(stdoutPipe[1]);
        close(statusPipe[0]);
        close(statusPipe[1]);
        return false;
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(stdinPipe[0]);
        close(stdinPipe[1]);
        close(stdoutPipe[0]);
        close(stdoutPipe[1]);
        close(statusPipe[0]);
        close(statusPipe[1]);
        return false;
    }

    if (pid == 0) {

        close(statusPipe[0]);
        close(stdinPipe[1]);
        close(stdoutPipe[0]);

        if (prctl(PR_SET_PDEATHSIG, SIGTERM) != 0) {
            mla_private_linux_external_task_child_fail(statusPipe[1]);
        }

        if (dup2(stdinPipe[0], STDIN_FILENO) < 0 || dup2(stdoutPipe[1], STDOUT_FILENO) < 0) {
            mla_private_linux_external_task_child_fail(statusPipe[1]);
        }

        close(stdinPipe[0]);
        close(stdoutPipe[1]);

        if (!mla_string_is_empty(p_WorkingDirectory)) {
            mla_c_string_t workingDirCStr = mla_string_to_cString(p_WorkingDirectory);
            const mla_char_t* workingDir = mla_c_string_data(workingDirCStr);
            if (workingDir == nullptr || chdir(workingDir) != 0) {
                mla_private_linux_external_task_child_fail(statusPipe[1]);
            }
        }

        mla_c_string_t cmdlineCStr = mla_string_to_cString(p_CmdLine);
        const mla_char_t* cmdline = mla_c_string_data(cmdlineCStr);

        if (cmdline == nullptr) {
            mla_private_linux_external_task_child_fail(statusPipe[1]);
        }

        execl("/bin/sh", "sh", "-c", cmdline, nullptr);
        mla_private_linux_external_task_child_fail(statusPipe[1]);
    }

    close(statusPipe[1]);
    close(stdinPipe[0]);
    close(stdoutPipe[1]);

    mla_uint8_t childError = 0;
    ssize_t childState = 0;
    do {
        childState = read(statusPipe[0], &childError, sizeof(childError));
    } while (childState < 0 && errno == EINTR);
    close(statusPipe[0]);

    if (childState != 0) {
        close(stdinPipe[1]);
        close(stdoutPipe[0]);
        waitpid(pid, nullptr, 0);
        return false;
    }

    p_OutTaskResource = mla_malloc_struct_cleanup_extension(mla_private_linux_external_task_native_resource_t);
    mla_private_linux_external_task_native_resource_t* processData = mla_pointer_get_data<mla_private_linux_external_task_native_resource_t>(p_OutTaskResource);

    if (processData == nullptr) {
        close(stdinPipe[1]);
        close(stdoutPipe[0]);
        kill(pid, SIGTERM);
        waitpid(pid, nullptr, 0);
        return false;
    }

    processData->pid = pid;
    processData->stdin_write_fd = stdinPipe[1];
    processData->stdout_read_fd = stdoutPipe[0];

    // Make both parent-side fds non-blocking so reads/writes return
    // immediately instead of blocking when no data / buffer is full.
    fcntl(processData->stdout_read_fd, F_SETFL, O_NONBLOCK);
    fcntl(processData->stdin_write_fd,  F_SETFL, O_NONBLOCK);


    return true;
}

mla_external_task_state mla_private_linux_external_task_get_state(const mla_pointer_t& p_TaskResource) {

    mla_private_linux_external_task_native_resource_t* processData = mla_linux_external_task_get_process_data(p_TaskResource);

    if (processData == nullptr) {
        return MLA_EXTERNAL_TASK_STATE_STOPPED;
    }

    if (processData->is_stopped) {
        return MLA_EXTERNAL_TASK_STATE_STOPPED;
    }

    if (processData->pid <= 0) {
        return MLA_EXTERNAL_TASK_STATE_STOPPED;
    }

    int status = 0;
    pid_t waitResult = -1;
    do {
        waitResult = waitpid(processData->pid, &status, WNOHANG);
    } while (waitResult < 0 && errno == EINTR);

    if (waitResult == 0) {
        return MLA_EXTERNAL_TASK_STATE_RUNNING;
    }

    processData->exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    processData->is_stopped = true;
    processData->pid = -1;
    if (processData->stdin_write_fd >= 0) {
        close(processData->stdin_write_fd);
        processData->stdin_write_fd = -1;
    }
    return MLA_EXTERNAL_TASK_STATE_STOPPED;
}

void mla_private_linux_external_task_stop_process(const mla_pointer_t& p_TaskResource) {

    mla_private_linux_external_task_native_resource_t* processData = mla_linux_external_task_get_process_data(p_TaskResource);

    if (processData == nullptr) {
        return;
    }

    mla_private_linux_external_task_cleanup_process_data(processData);

    if (processData->pid > 0) {
        kill(processData->pid, SIGTERM);
        waitpid(processData->pid, nullptr, 0);
        processData->pid = -1;
        processData->is_stopped = true;
        processData->exit_code = -1;
    }
}

mla_size_t mla_private_linux_external_task_read_stdout(const mla_pointer_t& p_TaskResource, mla_size_t p_Offset, mla_size_t p_Length, mla_byte_t* p_Buffer) {

    mla_private_linux_external_task_native_resource_t* processData = mla_linux_external_task_get_process_data(p_TaskResource);

    if (processData == nullptr || processData->stdout_read_fd < 0 || p_Buffer == nullptr) {
        return 0;
    }

    // Non-blocking: fd has O_NONBLOCK set; EAGAIN/EWOULDBLOCK means no data yet.
    ssize_t bytesRead = read(processData->stdout_read_fd, p_Buffer + p_Offset, p_Length);

    if (bytesRead <= 0) {
        return 0;
    }

    return mla_s_cast<mla_size_t>(bytesRead);
}

mla_size_t mla_private_linux_external_task_write_stdin(const mla_pointer_t& p_TaskResource, mla_size_t p_Offset, mla_size_t p_Length, const mla_byte_t* p_Buffer) {

    mla_private_linux_external_task_native_resource_t* processData = mla_linux_external_task_get_process_data(p_TaskResource);

    if (processData == nullptr || processData->stdin_write_fd < 0 || p_Buffer == nullptr) {
        return 0;
    }

    // Non-blocking: fd has O_NONBLOCK set; EAGAIN/EWOULDBLOCK means pipe buffer full.
    ssize_t bytesWritten = write(processData->stdin_write_fd, p_Buffer + p_Offset, p_Length);

    if (bytesWritten <= 0) {
        return 0;
    }

    return mla_s_cast<mla_size_t>(bytesWritten);
}

void mla_private_linux_external_task_close_stdin(const mla_pointer_t& p_TaskResource) {

    mla_private_linux_external_task_native_resource_t* processData = mla_linux_external_task_get_process_data(p_TaskResource);

    if (processData == nullptr) {
        return;
    }

    if (processData->stdin_write_fd >= 0) {
        close(processData->stdin_write_fd);
        processData->stdin_write_fd = -1;
    }
}

mla_int32_t mla_private_linux_external_task_read_result_code(const mla_pointer_t& p_TaskResource) {

    mla_private_linux_external_task_native_resource_t* processData = mla_linux_external_task_get_process_data(p_TaskResource);

    if (processData == nullptr) {
        return -1;
    }

    if (processData->is_stopped) {
        return processData->exit_code;
    }

    if (processData->pid <= 0) {
        return -1;
    }

    int status = 0;
    pid_t waitResult = -1;
    do {
        waitResult = waitpid(processData->pid, &status, WNOHANG);
    } while (waitResult < 0 && errno == EINTR);

    if (waitResult == 0) {
        return -1; // Still running
    }

    processData->exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    processData->is_stopped = true;
    processData->pid = -1;
    if (processData->stdin_write_fd >= 0) {
        close(processData->stdin_write_fd);
        processData->stdin_write_fd = -1;
    }

    return processData->exit_code;
}

mla_external_task_management_t g_external_task_management = {
    mla_private_linux_external_task_create_process,
    mla_private_linux_external_task_stop_process,
    mla_private_linux_external_task_get_state,
    mla_private_linux_external_task_read_stdout,
    mla_private_linux_external_task_write_stdin,
    mla_private_linux_external_task_close_stdin,
    mla_private_linux_external_task_read_result_code
};

#endif
