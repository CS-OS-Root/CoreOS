//
// Created by copilot on 12/28/2024.
//

#ifndef MLA_GLOBAL_FILE_SYSTEM_ESP32_H
#define MLA_GLOBAL_FILE_SYSTEM_ESP32_H

#include "../../core/filesystem/mla_file_system.h"
#include "../../core/mla_data_types.h"
#include "../../core/system/mla_string_concat.h"

#include <esp_spiffs.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

struct mla_file_system_native_t {
    mla_string_t basePath;
};

static const mla_string_t mla_esp32_fs_directory_seperator = mla_string_const("/");

static inline mla_string_t mla_private_file_system_native_file_path_to_full_path(mla_file_system_native_t* fs, const mla_string_t& path) {
    // Replace backslashes with forward slashes for ESP32
    mla_string_t correctedPath = mla_string_replace(path, mla_string_const("\\"), mla_esp32_fs_directory_seperator);

    if (fs->basePath.length == 0)
        return correctedPath;

    mla_string_t p1 = fs->basePath;
    mla_string_t p2 = correctedPath;
    mla_bool_t p1_allocated = false;
    mla_bool_t p2_allocated = false;

    if (mla_string_ends_with(p1, mla_esp32_fs_directory_seperator)) {
        p1 = mla_string_substr(p1, 0, p1.length - 1);
        p1_allocated = true;
    }

    if (mla_string_starts_with(p2, mla_esp32_fs_directory_seperator)) {
        p2 = mla_string_substr(p2, 1);
        p2_allocated = true;
    }

    mla_string_t result = mla_string_concat(p1, mla_esp32_fs_directory_seperator, p2);
    
    // Clean up intermediate allocations
    if (p1_allocated) mla_string_destroy(p1);
    if (p2_allocated) mla_string_destroy(p2);
    mla_string_destroy(correctedPath);
    
    return result;
}

static inline mla_bool_t mla_private_file_system_native_file_exists(mla_file_system_t& file_system, const mla_string_t& path) {
    mla_file_system_native_t* fs = mla_r_cast<mla_file_system_native_t*>(file_system.user_data);
    mla_string_t fullPath = mla_private_file_system_native_file_path_to_full_path(fs, path);

    struct stat statbuf;
    int result = stat(fullPath.data, &statbuf);
    mla_bool_t exists = (result == 0 && S_ISREG(statbuf.st_mode));

    mla_string_destroy(fullPath);
    return exists;
}

static inline mla_bool_t mla_private_file_system_native_delete_file(mla_file_system_t& file_system, const mla_string_t& path) {
    mla_file_system_native_t* fs = mla_r_cast<mla_file_system_native_t*>(file_system.user_data);
    mla_string_t fullPath = mla_private_file_system_native_file_path_to_full_path(fs, path);

    int result = unlink(fullPath.data);
    mla_string_destroy(fullPath);

    return result == 0;
}

static inline mla_bool_t mla_private_file_system_native_create_directory(mla_file_system_t& file_system, const mla_string_t& path) {
    mla_file_system_native_t* fs = mla_r_cast<mla_file_system_native_t*>(file_system.user_data);
    mla_string_t fullPath = mla_private_file_system_native_file_path_to_full_path(fs, path);

    int result = mkdir(fullPath.data, 0755); // rwxr-xr-x permissions
    mla_string_destroy(fullPath);

    return result == 0 || errno == EEXIST;
}

static inline mla_bool_t mla_private_file_system_native_directory_exists(mla_file_system_t& file_system, const mla_string_t& path) {
    mla_file_system_native_t* fs = mla_r_cast<mla_file_system_native_t*>(file_system.user_data);
    mla_string_t fullPath = mla_private_file_system_native_file_path_to_full_path(fs, path);

    struct stat statbuf;
    int result = stat(fullPath.data, &statbuf);
    mla_bool_t exists = (result == 0 && S_ISDIR(statbuf.st_mode));

    mla_string_destroy(fullPath);
    return exists;
}

static inline mla_bool_t mla_private_file_system_native_delete_directory(mla_file_system_t& file_system, const mla_string_t& path) {
    mla_file_system_native_t* fs = mla_r_cast<mla_file_system_native_t*>(file_system.user_data);
    mla_string_t fullPath = mla_private_file_system_native_file_path_to_full_path(fs, path);

    int result = rmdir(fullPath.data);
    mla_string_destroy(fullPath);

    return result == 0;
}

static inline mla_bool_t mla_private_file_system_native_list_files(mla_file_system_t& file_system, const mla_string_t& path, mla_array_list_t<mla_init_struct(mla_string_t)>& out_entries) {
    mla_file_system_native_t* fs = mla_r_cast<mla_file_system_native_t*>(file_system.user_data);
    mla_string_t fullPath = mla_private_file_system_native_file_path_to_full_path(fs, path);

    DIR* dir = opendir(fullPath.data);
    mla_string_destroy(fullPath);

    if (dir == NULL) {
        return false;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip directories and special entries (. and ..)
        if (entry->d_type != DT_REG) {
            continue;
        }

        mla_string_t entryName = mla_string_copy(entry->d_name, mla_strlen(entry->d_name));
        mla_array_list_add(out_entries, entryName);
    }

    closedir(dir);
    return true;
}

static inline mla_bool_t mla_private_file_system_native_list_directory(mla_file_system_t& file_system, const mla_string_t& path, mla_array_list_t<mla_init_struct(mla_string_t)>& out_entries) {
    mla_file_system_native_t* fs = mla_r_cast<mla_file_system_native_t*>(file_system.user_data);
    mla_string_t fullPath = mla_private_file_system_native_file_path_to_full_path(fs, path);

    DIR* dir = opendir(fullPath.data);
    mla_string_destroy(fullPath);

    if (dir == NULL) {
        return false;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip special entries (. and ..)
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Only include directories
        if (entry->d_type == DT_DIR) {
            mla_string_t entryName = mla_string_copy(entry->d_name, mla_strlen(entry->d_name));
            mla_array_list_add(out_entries, entryName);
        }
    }

    closedir(dir);
    return true;
}

static inline mla_buffer_cleanup_mode mla_private_file_system_native_close_file(mla_platform_pointer_t data, mla_callback_userdata userData) {
    (void)userData;
    int fd = *mla_r_cast<int*>(data);

    close(fd);
    mla_platform_free(data);

    return CLEAN_UP_SKIP;
}

static inline mla_bool_t mla_private_file_system_native_open_file_seek(const mla_file_system_stream_t& stream, mla_size_t offset) {
    int* fd = mla_r_cast<int*>(stream.user_data);

    return lseek(*fd, offset, SEEK_SET) != -1;
}

static inline mla_size_t mla_private_file_system_native_open_file_position(const mla_file_system_stream_t& stream) {
    int* fd = mla_r_cast<int*>(stream.user_data);

    off_t pos = lseek(*fd, 0, SEEK_CUR);
    return pos != -1 ? mla_s_cast<mla_size_t>(pos) : 0;
}

static inline mla_size_t mla_private_file_system_native_open_file_length(const mla_file_system_stream_t& stream) {
    int* fd = mla_r_cast<int*>(stream.user_data);

    // Save current position
    off_t currentPos = lseek(*fd, 0, SEEK_CUR);
    if (currentPos == -1) return 0;

    // Seek to end and get position
    off_t size = lseek(*fd, 0, SEEK_END);

    // Restore original position
    lseek(*fd, currentPos, SEEK_SET);

    return size != -1 ? mla_s_cast<mla_size_t>(size) : 0;
}

static inline mla_bool_t mla_private_file_system_native_open_file_set_length(const mla_file_system_stream_t& stream, mla_size_t length) {
    int* fd = mla_r_cast<int*>(stream.user_data);

    return ftruncate(*fd, length) == 0;
}

static inline mla_size_t mla_private_file_system_native_open_file_read(const mla_file_system_stream_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {
    int* fd = mla_r_cast<int*>(input.user_data);

    ssize_t bytesRead = read(*fd, buffer + offset, length);
    return bytesRead > 0 ? mla_s_cast<mla_size_t>(bytesRead) : 0;
}

static inline mla_size_t mla_private_file_system_native_open_file_write(const mla_file_system_stream_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {
    int* fd = mla_r_cast<int*>(output.user_data);

    ssize_t bytesWritten = write(*fd, buffer + offset, length);
    return bytesWritten > 0 ? mla_s_cast<mla_size_t>(bytesWritten) : 0;
}

static inline mla_bool_t mla_private_file_system_native_open_file(mla_file_system_t& file_system, const mla_string_t& path, mla_file_system_file_open_mode mode, mla_file_system_stream_t& out_stream) {
    mla_file_system_native_t* fs = mla_r_cast<mla_file_system_native_t*>(file_system.user_data);
    mla_string_t fullPath = mla_private_file_system_native_file_path_to_full_path(fs, path);

    int flags = 0;
    mla_bool_t canRead = false;
    mla_bool_t canWrite = false;

    switch (mode) {
        case MLA_FILE_SYSTEM_FILE_OPEN_MODE_READ:
            flags = O_RDONLY;
            canRead = true;
            break;
        case MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE:
            flags = O_WRONLY | O_CREAT | O_TRUNC;
            canWrite = true;
            break;
        case MLA_FILE_SYSTEM_FILE_OPEN_MODE_READ_AND_WRITE:
            flags = O_RDWR | O_CREAT;
            canRead = true;
            canWrite = true;
            break;
        default:
            mla_string_destroy(fullPath);
            return false;
    }

    int fd = open(fullPath.data, flags, 0644); // rw-r--r-- permissions

    if (fd == -1) {
        mla_string_destroy(fullPath);
        return false;
    }

    // Allocate memory to store the file descriptor (needs to be freed in cleanup)
    int* fdPtr = mla_s_cast<int*>(mla_platform_malloc(sizeof(int)));
    if (fdPtr == nullptr) {
        close(fd);
        mla_string_destroy(fullPath);
        return false;
    }

    *fdPtr = fd;

    // Make a copy of the path for the stream
    mla_string_t pathCopy = mla_string_copy(path.data, path.length);

    if (canRead && canWrite) {
        out_stream = {
            pathCopy,
            mla_private_file_system_native_open_file_seek,
            mla_private_file_system_native_open_file_position,
            mla_private_file_system_native_open_file_length,
            mla_private_file_system_native_open_file_set_length,
            mla_private_file_system_native_open_file_read,
            mla_private_file_system_native_open_file_write,
            mla_r_cast<mla_callback_userdata>(fdPtr),
            mla_buffer_reference(fdPtr, true, mla_private_file_system_native_close_file)
        };
    } else if (canRead) {
        out_stream = {
            pathCopy,
            mla_private_file_system_native_open_file_seek,
            mla_private_file_system_native_open_file_position,
            mla_private_file_system_native_open_file_length,
            nullptr,
            mla_private_file_system_native_open_file_read,
            nullptr,
            mla_r_cast<mla_callback_userdata>(fdPtr),
            mla_buffer_reference(fdPtr, true, mla_private_file_system_native_close_file)
        };
    } else if (canWrite) {
        out_stream = {
            pathCopy,
            mla_private_file_system_native_open_file_seek,
            mla_private_file_system_native_open_file_position,
            mla_private_file_system_native_open_file_length,
            mla_private_file_system_native_open_file_set_length,
            nullptr,
            mla_private_file_system_native_open_file_write,
            mla_r_cast<mla_callback_userdata>(fdPtr),
            mla_buffer_reference(fdPtr, true, mla_private_file_system_native_close_file)
        };
    } else {
        close(fd);
        mla_platform_free(fdPtr);
        mla_string_destroy(fullPath);
        return false;
    }

    mla_string_destroy(fullPath);
    return true;
}

static inline mla_buffer_cleanup_mode mla_private_file_system_native_cleanup(mla_platform_pointer_t data, mla_callback_userdata userData) {

    (void)userData;

    mla_file_system_native_t* fs = mla_s_cast<mla_file_system_native_t *>(data);

    if (fs == nullptr)
        return CLEAN_UP_SKIP;

    mla_string_destroy(fs->basePath);
    return CLEAN_UP_NEEDED;
}

static inline mla_file_system_t mla_private_file_system_native_create_with_base(const mla_string_t& basePath) {

    mla_file_system_native_t* fs = mla_s_cast<mla_file_system_native_t *>(mla_platform_malloc(sizeof(mla_file_system_native_t)));

    if (fs == nullptr)
        return mla_file_system_empty();

    mla_memset(fs, 0, sizeof(mla_file_system_native_t));
    fs->basePath = basePath;

    return {
        mla_private_file_system_native_file_exists,
        mla_private_file_system_native_open_file,
        mla_private_file_system_native_delete_file,
        mla_private_file_system_native_list_files,
        mla_private_file_system_native_create_directory,
        mla_private_file_system_native_directory_exists,
        mla_private_file_system_native_delete_directory,
        mla_private_file_system_native_list_directory,
        mla_r_cast<mla_callback_userdata>(fs),
        mla_buffer_reference(fs, true, mla_private_file_system_native_cleanup)
    };
}

mla_file_system_t mla_file_system_native_create_data_restricted(mla_string_t basePath) {
    // On ESP32, we use SPIFFS which is typically mounted at /spiffs
    // Create a base path for data storage
    mla_string_t spiffsPath = mla_string_const("/spiffs/data/");

    // Create the data directory if it doesn't exist
    mkdir("/spiffs/data", 0755);

    // Append the basePath
    mla_string_t fullPath = mla_string_concat(spiffsPath, basePath);

    // Create the final directory if it doesn't exist
    mkdir(fullPath.data, 0755);

    return mla_private_file_system_native_create_with_base(fullPath);
}

mla_file_system_t mla_file_system_native_create_global() {

    return mla_private_file_system_native_create_with_base(mla_string_const("/spiffs"));

}

#endif
