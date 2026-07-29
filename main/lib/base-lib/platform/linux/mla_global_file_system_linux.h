//
// Created by christian on 10/17/25.
//

#ifndef MLA_GLOBAL_FILE_SYSTEM_LINUX_H
#define MLA_GLOBAL_FILE_SYSTEM_LINUX_H

#include "../../core/filesystem/mla_file_system.h"
#include "../../core/mla_data_types.h"
#include "../../core/mla_native_resource.h"
#include "../../core/system/mla_string_concat.h"

#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <errno.h>
#include <cstring>

struct mla_file_system_native_t {
    mla_string_t basePath;

    static mla_file_system_native_t init() {
        return {
            mla_string_empty()
        };
    }
};


static const mla_string_t mla_linux_fs_directory_seperator = mla_string_const("/");

mla_file_system_native_t* mla_private_file_system_native_get_native_data(mla_file_system_t& file_system) {

    return mla_pointer_get_data<mla_file_system_native_t>(file_system.resource);
}

mla_string_t mla_private_file_system_native_file_path_to_full_path(mla_file_system_native_t* fs, const mla_string_t& path) {
    // Replace backslashes with forward slashes for Linux
    mla_string_t correctedPath = mla_string_replace(path, mla_string_const("\\"), mla_linux_fs_directory_seperator);

    if (mla_string_length(fs->basePath) == 0) {
        if (!mla_string_starts_with(correctedPath, mla_linux_fs_directory_seperator)) {
            return mla_string_concat(mla_linux_fs_directory_seperator, correctedPath);
        }
        return correctedPath;
    }

    mla_string_t p1 = fs->basePath;
    mla_string_t p2 = correctedPath;

    if (mla_string_ends_with(p1, mla_linux_fs_directory_seperator)) {
        p1 = mla_string_substr(p1, 0, mla_string_length(p1) - 1);
    }

    if (mla_string_starts_with(p2, mla_linux_fs_directory_seperator)) {
        p2 = mla_string_substr(p2, 1);
    }

    return mla_string_concat(p1, mla_linux_fs_directory_seperator, p2);
}

mla_bool_t mla_private_file_system_native_file_exists(mla_file_system_t& file_system, const mla_string_t& path) {

    mla_file_system_native_t* fs = mla_private_file_system_native_get_native_data(file_system);

    if (fs == nullptr) {
        return false;
    }

    mla_string_t fullPath = mla_private_file_system_native_file_path_to_full_path(fs, path);

    struct stat statbuf;
    int result = stat(mla_string_data(fullPath), &statbuf);
    mla_bool_t exists = (result == 0 && S_ISREG(statbuf.st_mode));

    return exists;
}

mla_bool_t mla_private_file_system_native_delete_file(mla_file_system_t& file_system, const mla_string_t& path) {

    mla_file_system_native_t* fs = mla_private_file_system_native_get_native_data(file_system);

    if (fs == nullptr) {
        return false;
    }

    mla_string_t fullPath = mla_private_file_system_native_file_path_to_full_path(fs, path);

    int result = unlink(mla_string_data(fullPath));

    return result == 0;
}

mla_bool_t mla_private_file_system_native_create_directory(mla_file_system_t& file_system, const mla_string_t& path) {

    mla_file_system_native_t* fs = mla_private_file_system_native_get_native_data(file_system);

    if (fs == nullptr) {
        return false;
    }

    mla_string_t fullPath = mla_private_file_system_native_file_path_to_full_path(fs, path);

    int result = mkdir(mla_string_data(fullPath), 0755); // rwxr-xr-x permissions
    int err = errno;

    return result == 0 || err == EEXIST;
}

mla_bool_t mla_private_file_system_native_directory_exists(mla_file_system_t& file_system, const mla_string_t& path) {

    mla_file_system_native_t* fs = mla_private_file_system_native_get_native_data(file_system);

    if (fs == nullptr) {
        return false;
    }

    mla_string_t fullPath = mla_private_file_system_native_file_path_to_full_path(fs, path);

    struct stat statbuf;
    int result = stat(mla_string_data(fullPath), &statbuf);
    mla_bool_t exists = (result == 0 && S_ISDIR(statbuf.st_mode));

    return exists;
}

mla_bool_t mla_private_file_system_native_delete_directory(mla_file_system_t& file_system, const mla_string_t& path) {

    mla_file_system_native_t* fs = mla_private_file_system_native_get_native_data(file_system);

    if (fs == nullptr) {
        return false;
    }

    mla_string_t fullPath = mla_private_file_system_native_file_path_to_full_path(fs, path);

    int result = rmdir(mla_string_data(fullPath));

    return result == 0 || errno == ENOENT;
}

mla_bool_t mla_private_file_system_native_os_absolute_path(mla_file_system_t& file_system, const mla_string_t& path, mla_bool_t check_if_exists, mla_string_t& out) {

    if (check_if_exists && !mla_private_file_system_native_file_exists(file_system, path) && !mla_private_file_system_native_directory_exists(file_system, path)) {
        return false;
    }

    mla_file_system_native_t* fs = mla_private_file_system_native_get_native_data(file_system);

    if (fs == nullptr) {
        return false;
    }

    mla_string_t fullPath = mla_private_file_system_native_file_path_to_full_path(fs, path);

    out = fullPath;

    return true;
}

mla_bool_t mla_private_file_system_native_list_files(mla_file_system_t& file_system, const mla_string_t& path, mla_array_list_t<mla_init_struct(mla_string_t)>& out_entries) {

    mla_file_system_native_t* fs = mla_private_file_system_native_get_native_data(file_system);

    if (fs == nullptr) {
        return false;
    }

    mla_string_t fullPath = mla_private_file_system_native_file_path_to_full_path(fs, path);

    DIR* dir = opendir(mla_string_data(fullPath));

    if (dir == nullptr) {
        return false;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        // Skip special entries (. and ..)
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        mla_bool_t is_dir = false;
        if (entry->d_type == DT_DIR) {
            is_dir = true;
        } else if (entry->d_type == DT_UNKNOWN || entry->d_type == DT_LNK) {
            mla_string_t entryPath = mla_fs_combine_paths(fullPath, mla_string_const(entry->d_name));
            struct stat statbuf;
            if (stat(mla_string_data(entryPath), &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
                is_dir = true;
            }
        }

        if (!is_dir) {
            mla_string_t entryName = mla_string_copy(entry->d_name, mla_strlen(entry->d_name));
            mla_array_list_add(out_entries, entryName);
        }
    }

    closedir(dir);
    return true;
}

mla_bool_t mla_private_file_system_native_list_directory(mla_file_system_t& file_system, const mla_string_t& path, mla_array_list_t<mla_init_struct(mla_string_t)>& out_entries) {

    mla_file_system_native_t* fs = mla_private_file_system_native_get_native_data(file_system);

    if (fs == nullptr) {
        return false;
    }

    mla_string_t fullPath = mla_private_file_system_native_file_path_to_full_path(fs, path);

    DIR* dir = opendir(mla_string_data(fullPath));

    if (dir == nullptr) {
        return false;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        // Skip special entries (. and ..)
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        mla_bool_t is_dir = false;
        if (entry->d_type == DT_DIR) {
            is_dir = true;
        } else if (entry->d_type == DT_UNKNOWN || entry->d_type == DT_LNK) {
            mla_string_t entryPath = mla_fs_combine_paths(fullPath, mla_string_const(entry->d_name));
            struct stat statbuf;
            if (stat(mla_string_data(entryPath), &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
                is_dir = true;
            }
        }

        if (is_dir) {
            mla_string_t entryName = mla_string_copy(entry->d_name, mla_strlen(entry->d_name));
            mla_array_list_add(out_entries, entryName);
        }
    }

    closedir(dir);
    return true;
}

void mla_private_file_system_native_close_file(const mla_native_resource_t& userData) {

    int* fdPtr = mla_r_cast<int*>(userData.asPointer);

    if (fdPtr == nullptr) {
        return;
    }

    close(*fdPtr);
    mla_platform_free(fdPtr);
}

int mla_private_file_system_native_get_fd_from_stream(const mla_file_system_stream_t& stream) {

    mla_native_resource_t* native_resource = mla_native_resource_from_managed_pointer(stream.resource);

    if (native_resource == nullptr) {
        return -1;
    }

    return *mla_r_cast<int*>(native_resource->asPointer);
}

mla_bool_t mla_private_file_system_native_open_file_seek(const mla_file_system_stream_t& stream, mla_size_t offset) {

    int fd = mla_private_file_system_native_get_fd_from_stream(stream);

    if (fd == -1) {
        return false;
    }

    return lseek(fd, offset, SEEK_SET) != -1;
}

mla_size_t mla_private_file_system_native_open_file_position(const mla_file_system_stream_t& stream) {

    int fd = mla_private_file_system_native_get_fd_from_stream(stream);

    if (fd == -1) {
        return 0;
    }

    off_t pos = lseek(fd, 0, SEEK_CUR);
    return pos != -1 ? mla_s_cast<mla_size_t>(pos) : 0;
}

mla_size_t mla_private_file_system_native_open_file_length(const mla_file_system_stream_t& stream) {

    int fd = mla_private_file_system_native_get_fd_from_stream(stream);

    if (fd == -1) {
        return 0;
    }

    // Save current position
    off_t currentPos = lseek(fd, 0, SEEK_CUR);
    if (currentPos == -1) { return 0; }

    // Seek to end and get position
    off_t size = lseek(fd, 0, SEEK_END);

    // Restore original position
    lseek(fd, currentPos, SEEK_SET);

    return size != -1 ? mla_s_cast<mla_size_t>(size) : 0;
}

mla_bool_t mla_private_file_system_native_open_file_set_length(const mla_file_system_stream_t& stream, mla_size_t length) {

    int fd = mla_private_file_system_native_get_fd_from_stream(stream);

    if (fd == -1) {
        return false;
    }

    return ftruncate(fd, length) == 0;
}

mla_size_t mla_private_file_system_native_open_file_read(const mla_file_system_stream_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {

    int fd = mla_private_file_system_native_get_fd_from_stream(input);

    if (fd == -1) {
        return 0;
    }

    ssize_t bytesRead = read(fd, buffer + offset, length);
    return bytesRead > 0 ? mla_s_cast<mla_size_t>(bytesRead) : 0;
}

mla_size_t mla_private_file_system_native_open_file_write(const mla_file_system_stream_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {

    int fd = mla_private_file_system_native_get_fd_from_stream(output);

    if (fd == -1) {
        return 0;
    }

    ssize_t bytesWritten = write(fd, buffer + offset, length);
    return bytesWritten > 0 ? mla_s_cast<mla_size_t>(bytesWritten) : 0;
}

mla_bool_t mla_private_file_system_native_open_file(mla_file_system_t& file_system, const mla_string_t& path, mla_file_system_file_open_mode mode, mla_file_system_stream_t& out_stream) {

    mla_file_system_native_t* fs = mla_private_file_system_native_get_native_data(file_system);

    if (fs == nullptr) {
        return false;
    }

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
            return false;
    }

    int fd = open(mla_string_data(fullPath), flags, 0644); // rw-r--r-- permissions

    if (fd == -1) {
        return false;
    }

    // Allocate memory to store the file descriptor (freed in cleanup hook)
    int* fdPtr = mla_s_cast<int*>(mla_platform_malloc(sizeof(int)));
    if (fdPtr == nullptr) {
        close(fd);
        return false;
    }

    *fdPtr = fd;

    mla_native_resource_t resource = mla_dynamic_data_from_pointer(fdPtr);
    mla_pointer_t resource_ptr = mla_native_resource_to_managed_pointer(resource, mla_private_file_system_native_close_file);

    if (canRead && canWrite) {
        out_stream = {
            path,
            mla_private_file_system_native_open_file_seek, // seek
            mla_private_file_system_native_open_file_position, // position
            mla_private_file_system_native_open_file_length, // length
            mla_private_file_system_native_open_file_set_length, // set_length
            mla_private_file_system_native_open_file_read, // read
            mla_private_file_system_native_open_file_write, // write
            resource_ptr
        };
    } else if (canRead) {
        out_stream = {
            path,
            mla_private_file_system_native_open_file_seek, // seek
            mla_private_file_system_native_open_file_position, // position
            mla_private_file_system_native_open_file_length, // length
            nullptr, // no set_length
            mla_private_file_system_native_open_file_read, // read
            nullptr, // no write
            resource_ptr
        };
    } else if (canWrite) {
        out_stream = {
            path,
            mla_private_file_system_native_open_file_seek, // seek
            mla_private_file_system_native_open_file_position, // position
            mla_private_file_system_native_open_file_length, // length
            mla_private_file_system_native_open_file_set_length, // set_length
            nullptr, // no read
            mla_private_file_system_native_open_file_write, // write
            resource_ptr
        };
    } else {
        close(fd);
        mla_platform_free(fdPtr);
        return false;
    }

    return true;
}

mla_file_system_t mla_private_file_system_native_create_with_base(const mla_string_t& basePath) {

    mla_pointer_t fs_ptr = mla_malloc_struct(mla_file_system_native_t);

    mla_file_system_native_t* fs = mla_pointer_get_data<mla_file_system_native_t>(fs_ptr);

    if (fs == nullptr) {
        return mla_file_system_empty();
    }

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
        mla_private_file_system_native_os_absolute_path,
        fs_ptr
    };
}

mla_file_system_t mla_file_system_native_create_restricted(const mla_string_t& full_basePath) {
    return mla_private_file_system_native_create_with_base(full_basePath);
}

mla_file_system_t mla_file_system_native_create_data_restricted(const mla_string_t& basePath) {
    // Get the executable path
    char exePath[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);

    if (len == -1) {
        return mla_file_system_empty();
    }

    exePath[len] = '\0';

    // Find last slash to get directory
    char* lastSlash = strrchr(exePath, '/');
    if (lastSlash != nullptr) {
        *(lastSlash + 1) = '\0';
    }

    // Create path string
    mla_string_t modulePath = mla_string(exePath);

    // Append "data/" to the module path
    mla_string_t dataPath = mla_string_concat(modulePath, mla_string_const("data/"));

    // Create the data directory if it doesn't exist
    mkdir(mla_string_data(dataPath), 0755);

    // Append the basePath
    mla_string_t fullPath = mla_string_concat(dataPath, basePath);

    // Create the final directory if it doesn't exist
    mkdir(mla_string_data(fullPath), 0755);

    return mla_private_file_system_native_create_with_base(fullPath);
}



mla_file_system_t mla_file_system_native_create_global() {

    return mla_private_file_system_native_create_with_base(mla_string_empty());

}


#endif