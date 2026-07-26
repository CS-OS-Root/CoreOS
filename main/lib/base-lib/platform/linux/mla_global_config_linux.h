//
// Created by christian on 10/2/25.
//

#ifndef MLA_GLOBAL_CONFIG_LINUX_H
#define MLA_GLOBAL_CONFIG_LINUX_H

#include "../../core/config/mla_config.h"

#ifndef mla_max_config_size
#define mla_max_config_size (16L * 1024) // 16KB is default
#endif

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <limits.h>

void get_config_filename(char* buffer, ssize_t bufferSize) {

    // Read the symlink /proc/self/exe to get executable path
    ssize_t len = readlink("/proc/self/exe", buffer, bufferSize - 1);
    if (len == -1) {
        buffer[0] = '\0';
        return;
    }
    buffer[len] = '\0';

    // Find last dot for extension
    char* lastDot = nullptr;
    for (char* p = buffer + len - 1; p >= buffer; p--) {
        if (*p == '.') {
            lastDot = p;
            break;
        }
        if (*p == '/') { break; }
    }

    if (lastDot != nullptr) {
        *lastDot = '\0';
        len = lastDot - buffer;
    }

    // Append .config
    const char* suffix = ".config";
    for (const char* s = suffix; *s != '\0' && len < bufferSize - 1; s++, len++) {
        buffer[len] = *s;
    }
    buffer[len] = '\0';
}

// On Linux the configuration is just an file in the application directory
// The file can be read and written using standard file operations

mla_bytes_t mla_linux_read_config_input() {

    char configFilename[PATH_MAX];
    get_config_filename(configFilename, PATH_MAX);

    int fd = open(configFilename, O_RDONLY);
    if (fd == -1) {
        return mla_bytes_empty();
    }

    struct stat st;
    if (fstat(fd, &st) == -1 || st.st_size > mla_max_config_size) {
        close(fd);
        return mla_bytes_empty();
    }

    mla_bytes_t config_data = mla_bytes(st.st_size);
    if (config_data.size != mla_s_cast<mla_size_t>(st.st_size)) {
        close(fd);
        return config_data;
    }

    ssize_t bytesRead = read(fd, mla_bytes_get_data_for_writing(config_data), st.st_size);
    close(fd);

    if (bytesRead != st.st_size) {
        config_data = mla_bytes_empty();
        return mla_bytes(0);
    }

    return config_data;
}


mla_bytes_t mla_linux_create_config_output_buffer() {

    return mla_bytes(mla_max_config_size);
}

mla_bool_t mla_linux_commit_config_output(mla_bytes_t& output, mla_size_t unused_bytes) {

    char configPath[PATH_MAX];
    char tempPath[PATH_MAX];
    char backupPath[PATH_MAX];

    get_config_filename(configPath, PATH_MAX);

    // Build temp and backup paths
    size_t len = 0;
    while (configPath[len] != '\0' && len < PATH_MAX - 5) {
        tempPath[len] = configPath[len];
        backupPath[len] = configPath[len];
        len++;
    }
    tempPath[len] = '\0';
    backupPath[len] = '\0';

    const char* tmpSuffix = ".tmp";
    const char* bakSuffix = ".bak";
    for (const char* s = tmpSuffix; *s != '\0' && len < PATH_MAX - 1; s++) {
        tempPath[len++] = *s;
    }
    tempPath[len] = '\0';

    len = 0;
    while (configPath[len] != '\0') { len++; }
    for (const char* s = bakSuffix; *s != '\0' && len < PATH_MAX - 1; s++, len++) {
        backupPath[len] = *s;
    }
    backupPath[len] = '\0';

    unlink(tempPath);

    int fd = open(tempPath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        output = mla_bytes_empty();
        return false;
    }

    mla_size_t bytesToWrite = mla_min(output.size, output.size - unused_bytes);

    const mla_byte_t* output_ptr = mla_bytes_get_data_readonly(output);

    ssize_t bytesWritten = write(fd, output_ptr, bytesToWrite);
    close(fd);

    if (bytesWritten != mla_s_cast<ssize_t>(bytesToWrite)) {
        unlink(tempPath);
        output = mla_bytes_empty();
        return false;
    }

    unlink(backupPath);

    if (access(configPath, F_OK) == 0) {
        if (rename(configPath, backupPath) != 0) {
            unlink(tempPath);
            output = mla_bytes_empty();
            return false;
        }
    }

    if (rename(tempPath, configPath) != 0) {
        rename(backupPath, configPath);
        unlink(tempPath);
        output = mla_bytes_empty();
        return false;
    }

    unlink(backupPath);
    output = mla_bytes_empty();
    return true;
}

mla_bool_t mla_linux_reset() {

    char configFilename[PATH_MAX];
    get_config_filename(configFilename, PATH_MAX);
    return unlink(configFilename) == 0;
}


mla_config_low_level_operations_t g_config_low_level_operations = {
    mla_linux_read_config_input,
    mla_linux_create_config_output_buffer,
    mla_linux_commit_config_output,
    mla_linux_reset
};

#endif
