//
// Created by chris on 10/13/2025.
//

#include "mla_file_system.h"

#include "../task/mla_mutx.h"
#include "../lifecycle/mla_lifecycle_events.h"

mla_file_system_t mla_file_system_empty() {
    return { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, mla_pointer_null() };
}

mla_file_system_stream_t mla_file_system_stream_empty() {
    return { mla_string_empty(), nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, mla_pointer_null() };
}

mla_file_system_stream_t mla_file_system_stream_t::init() {
    return mla_file_system_stream_empty();
}

struct mla_file_system_mount_t {
    mla_string_t mount_path;
    mla_file_system_t file_system;

    static mla_file_system_mount_t init() {
        return { mla_string_empty(), mla_file_system_empty() };
    }
};

mla_file_system_mount_t mla_file_system_mount_empty() {
    return { mla_string_empty(), mla_file_system_empty() };
}


struct mla_file_system_manager_t {
    mla_bool_t locked;
    mla_mutex_t mounded_file_systems_mutex;
    mla_array_list_t<mla_file_system_mount_t, mla_file_system_mount_t> mounted_file_systems;
};

static mla_file_system_manager_t file_system_manager = {
    false,
    mla_mutex_create("file_system_manager"),
    mla_array_list_empty<mla_file_system_mount_t, mla_file_system_mount_t>()
};

mla_int32_t mla_private_file_system_sort_compare(const mla_file_system_mount_t &a, const mla_file_system_mount_t &b) {
    mla_size_t mountPathLengthA = mla_string_length(a.mount_path);
    mla_size_t mountPathLengthB = mla_string_length(b.mount_path);

    if (mountPathLengthA != mountPathLengthB) {
        return (mountPathLengthA > mountPathLengthB) ? -1 : 1;
    }
    return mla_string_compare(a.mount_path, b.mount_path);
}

void mla_file_system_lock() {
    if (file_system_manager.locked) {
        return;
    }

    if (!mla_mutex_lock(file_system_manager.mounded_file_systems_mutex)) {
        return;
    }

    // We sort the list by the length desc and alphabetical order of the mount paths
    // This ensures that the most specific mount paths are checked first
    mla_array_list_sort(file_system_manager.mounted_file_systems, mla_private_file_system_sort_compare);

    file_system_manager.locked = true;

    mla_mutex_unlock(file_system_manager.mounded_file_systems_mutex);
}

void mla_file_system_unlock() {

    if (!file_system_manager.locked) {
        return;
    }

    if (!mla_mutex_lock(file_system_manager.mounded_file_systems_mutex)) {
        return;
    }

    file_system_manager.locked = false;

    mla_mutex_unlock(file_system_manager.mounded_file_systems_mutex);
}

mla_bool_t mla_file_system_is_locked() {

    return file_system_manager.locked;
}

mla_bool_t mla_private_file_system_isvalid_directory_path(const mla_string_t& path) {

    mla_size_t path_length = mla_string_length(path);

    if (path_length == 1 && mla_string_equals(path, mla_fs_directory_seperator)) {
        return true;
    }

    // A valid directory path must start and end with a slash
    if (path_length < 2) {
        return false;
    }

    if (!mla_string_starts_with(path, mla_fs_directory_seperator)) {
        return false;
    }

    if (!mla_string_ends_with(path, mla_fs_directory_seperator)) {
        return false;
    }

    return true;

}

mla_bool_t mla_file_system_initialize(const mla_string_t& mount_path, const mla_file_system_t& file_system) {

    if (!mla_private_file_system_isvalid_directory_path(mount_path)) {
        mla_error(mla_string_concat("Mount point '", mount_path, "' is not a valid directory path. It must start and end with a slash."));
        return false;
    }


    if (file_system_manager.locked) {
        return false;
    }

    if (!mla_mutex_lock(file_system_manager.mounded_file_systems_mutex)) {
        return false;
    }

    mla_string_t normalized_mount_path = mla_string_to_lower(mount_path);

    mla_bool_t result = mla_array_list_add(file_system_manager.mounted_file_systems, {
                                               normalized_mount_path,
                                               file_system
                                           });

    mla_mutex_unlock(file_system_manager.mounded_file_systems_mutex);
    return result;
}

mla_bool_t mla_file_system_deinitialize(const mla_string_t &mount_path) {

    if (file_system_manager.locked) {
        return false;
    }

    if (!mla_mutex_lock(file_system_manager.mounded_file_systems_mutex)) {
        return false;
    }

    // Find the matching entity
    mla_int32_t index = -1;
    for (mla_size_t i = 0; i < mla_array_list_size(file_system_manager.mounted_file_systems); i++) {
        const mla_file_system_mount_t *mount = mla_array_list_get_ref(file_system_manager.mounted_file_systems, i);
        if (mla_string_equals_ignore_case(mount->mount_path, mount_path)) {
            index = mla_s_cast<mla_int32_t>(i);
            break;
        }
    }

    mla_bool_t result = false;
    if (index != -1) {
        result = mla_array_list_remove(file_system_manager.mounted_file_systems, index);
    }

    mla_mutex_unlock(file_system_manager.mounded_file_systems_mutex);
    return result;
}

mla_bool_t mla_private_file_system_find_file_system_for_path(const mla_string_t& path, mla_file_system_mount_t& out_file_system) {

    if (!file_system_manager.locked) {
        return false;
    }

    mla_string_t normalized_path = mla_string_to_lower(path);

    for (mla_size_t i = 0; i < mla_array_list_size(file_system_manager.mounted_file_systems); i++) {

        const mla_file_system_mount_t *mount = mla_array_list_get_ref(file_system_manager.mounted_file_systems, i);
        if (mla_string_starts_with(normalized_path, mount->mount_path)) {
            out_file_system = *mount;
            return true;
        }
    }

    return false;
}

mla_bool_t mla_private_file_system_find_file_system_for_file(const mla_string_t& file_path, mla_file_system_mount_t& out_file_system) {

    // Split the in the file path into directory and file name
    mla_size_t last_slash_index = mla_string_last_index_of(file_path, mla_fs_directory_seperator);
    if (last_slash_index == mla_s_cast<mla_size_t>(-1)) {
        return false;
    }

    mla_string_t dicrectory_path = mla_string_substr(file_path, 0, last_slash_index + 1);
    return mla_private_file_system_find_file_system_for_path(dicrectory_path, out_file_system);
}

mla_string_t mla_private_file_system_get_relative_path(const mla_string_t& full_path, const mla_string_t& mount_path) {

    mla_string_t path = mla_string_substr(full_path, mla_string_length(mount_path));
    return path;
}

mla_bool_t mla_fs_file_exists(const mla_string_t& path) {

    mla_file_system_mount_t file_system_mount = mla_file_system_mount_empty();

    if (!mla_private_file_system_find_file_system_for_file(path, file_system_mount)) {
        return false;
    }

    mla_string_t relative_path = mla_private_file_system_get_relative_path(path, file_system_mount.mount_path);

    if (file_system_mount.file_system.file_exists == nullptr) {
        return false;
    }

    return file_system_mount.file_system.file_exists(file_system_mount.file_system, relative_path);

}


mla_bool_t mla_fs_delete_file(const mla_string_t& path) {

    mla_file_system_mount_t file_system_mount = mla_file_system_mount_empty();

    if (!mla_private_file_system_find_file_system_for_file(path, file_system_mount)) {
        return false;
    }

    mla_string_t relative_path = mla_private_file_system_get_relative_path(path, file_system_mount.mount_path);

    if (file_system_mount.file_system.delete_file == nullptr) {
        return false;
    }

    return file_system_mount.file_system.delete_file(file_system_mount.file_system, relative_path);

}

mla_bool_t mla_fs_list_files(const mla_string_t& path, mla_array_list_t<mla_init_struct(mla_string_t)>& out_entries) {

    mla_file_system_mount_t file_system_mount = mla_file_system_mount_empty();

    if (!mla_private_file_system_find_file_system_for_path(path, file_system_mount)) {
        return false;
    }

    mla_string_t relative_path = mla_private_file_system_get_relative_path(path, file_system_mount.mount_path);

    if (file_system_mount.file_system.list_files == nullptr) {
        return false;
    }

    return file_system_mount.file_system.list_files(file_system_mount.file_system, relative_path, out_entries);

}

mla_bool_t mla_fs_count_files(const mla_string_t& path, mla_size_t& out_count) {

    mla_array_list_t<mla_init_struct(mla_string_t)> entries = mla_array_list_empty<mla_init_struct(mla_string_t)>();

    if (!mla_fs_list_files(path, entries)) {
        return false;
    }

    out_count = mla_array_list_size(entries);
    return true;
}

mla_bool_t mla_private_fs_create_directory_recursive(mla_file_system_t& file_system, const mla_string_t& path) {

    if (mla_string_length(path) == 0) {
        return true;
    }

    if (file_system.directory_exists != nullptr && file_system.directory_exists(file_system, path)) {
        return true;
    }

    mla_string_t parentPath = mla_fs_get_parent_directory(path);

    if (!mla_string_equals(parentPath, mla_fs_root_directory) &&
        !mla_string_equals(parentPath, path) &&
        !mla_private_fs_create_directory_recursive(file_system, parentPath)) {
        return false;
    }

    return file_system.create_directory(file_system, path);
}

mla_bool_t mla_fs_create_directory(const mla_string_t& path) {

    mla_file_system_mount_t file_system_mount = mla_file_system_mount_empty();

    if (!mla_private_file_system_find_file_system_for_path(path, file_system_mount)) {
        return false;
    }

    mla_string_t relative_path = mla_private_file_system_get_relative_path(path, file_system_mount.mount_path);

    if (file_system_mount.file_system.create_directory == nullptr) {
        return false;
    }

    return mla_private_fs_create_directory_recursive(file_system_mount.file_system, relative_path);

}

mla_bool_t mla_fs_directory_exists(const mla_string_t& path) {

    mla_file_system_mount_t file_system_mount = mla_file_system_mount_empty();

    if (!mla_private_file_system_find_file_system_for_path(path, file_system_mount)) {
        return false;
    }

    mla_string_t relative_path = mla_private_file_system_get_relative_path(path, file_system_mount.mount_path);

    if (file_system_mount.file_system.directory_exists == nullptr) {
        return false;
    }

    return file_system_mount.file_system.directory_exists(file_system_mount.file_system, relative_path);

}

mla_bool_t mla_private_fs_delete_directory_recursive(mla_file_system_t& file_system, const mla_string_t& path) {

    mla_array_list_t<mla_init_struct(mla_string_t)> files =
        mla_array_list_empty<mla_init_struct(mla_string_t)>();

    if (!file_system.list_files(file_system, path, files)) {
        return false;
    }

    for (mla_size_t i = 0; i < mla_array_list_size(files); i++) {
        const mla_string_t& fileName = mla_array_list_get_unsafe(files, i);
        mla_string_t filePath = mla_fs_combine_paths(path, fileName);

        if (!file_system.delete_file(file_system, filePath)) {
            return false;
        }
    }

    mla_array_list_t<mla_init_struct(mla_string_t)> directories =
        mla_array_list_empty<mla_init_struct(mla_string_t)>();

    if (!file_system.list_directory(file_system, path, directories)) {
        return false;
    }

    for (mla_size_t i = 0; i < mla_array_list_size(directories); i++) {
        const mla_string_t& directoryName = mla_array_list_get_unsafe(directories, i);
        mla_string_t directoryPath = mla_fs_combine_paths(path, directoryName);

        if (!mla_private_fs_delete_directory_recursive(file_system, directoryPath)) {
            return false;
        }
    }

    return file_system.delete_directory(file_system, path);
}

mla_bool_t mla_fs_delete_directory(const mla_string_t& path) {

    mla_file_system_mount_t file_system_mount = mla_file_system_mount_empty();

    if (!mla_private_file_system_find_file_system_for_path(path, file_system_mount)) {
        return false;
    }

    mla_string_t relative_path = mla_private_file_system_get_relative_path(path, file_system_mount.mount_path);

    if (file_system_mount.file_system.delete_file == nullptr ||
        file_system_mount.file_system.list_files == nullptr ||
        file_system_mount.file_system.delete_directory == nullptr ||
        file_system_mount.file_system.list_directory == nullptr) {
        return false;
    }

    return mla_private_fs_delete_directory_recursive(file_system_mount.file_system, relative_path);
}

mla_bool_t mla_fs_list_directory(const mla_string_t& path, mla_array_list_t<mla_init_struct(mla_string_t)>& out_entries) {

    mla_file_system_mount_t file_system_mount = mla_file_system_mount_empty();

    if (!mla_private_file_system_find_file_system_for_path(path, file_system_mount)) {
        return false;
    }

    mla_string_t relative_path = mla_private_file_system_get_relative_path(path, file_system_mount.mount_path);

    if (file_system_mount.file_system.list_directory == nullptr) {
        return false;
    }

    return file_system_mount.file_system.list_directory(file_system_mount.file_system, relative_path, out_entries);
}

mla_bool_t mla_fs_copy_directory(const mla_string_t& source_path, const mla_string_t& destination_path) {

    if (!mla_fs_is_directory_path(source_path) ||
        !mla_fs_is_directory_path(destination_path) ||
        !mla_fs_directory_exists(source_path)) {
        return false;
    }

    mla_string_t normalizedSourcePath = mla_string_to_lower(source_path);
    mla_string_t normalizedDestinationPath = mla_string_to_lower(destination_path);

    if (mla_string_equals(normalizedSourcePath, normalizedDestinationPath) ||
        mla_string_starts_with(normalizedDestinationPath, normalizedSourcePath)) {
        return false;
    }

    if (!mla_fs_create_directory(destination_path)) {
        return false;
    }

    mla_array_list_t<mla_init_struct(mla_string_t)> files =
        mla_array_list_empty<mla_init_struct(mla_string_t)>();

    if (!mla_fs_list_files(source_path, files)) {
        return false;
    }

    for (mla_size_t i = 0; i < mla_array_list_size(files); i++) {
        const mla_string_t& fileName = mla_array_list_get_unsafe(files, i);
        mla_string_t sourceFilePath = mla_string_concat(source_path, fileName);
        mla_string_t destinationFilePath = mla_string_concat(destination_path, fileName);

        if (!mla_fs_copy_file_to(sourceFilePath, destinationFilePath)) {
            return false;
        }
    }

    mla_array_list_t<mla_init_struct(mla_string_t)> directories =
        mla_array_list_empty<mla_init_struct(mla_string_t)>();

    if (!mla_fs_list_directory(source_path, directories)) {
        return false;
    }

    for (mla_size_t i = 0; i < mla_array_list_size(directories); i++) {
        const mla_string_t& directoryName = mla_array_list_get_unsafe(directories, i);
        mla_string_t sourceDirectoryPath =
            mla_string_concat(source_path, directoryName, mla_fs_directory_seperator);
        mla_string_t destinationDirectoryPath =
            mla_string_concat(destination_path, directoryName, mla_fs_directory_seperator);

        if (!mla_fs_copy_directory(sourceDirectoryPath, destinationDirectoryPath)) {
            return false;
        }
    }

    return true;
}

mla_bool_t mla_fs_count_directory(const mla_string_t& path, mla_size_t& out_count) {

    mla_array_list_t<mla_init_struct(mla_string_t)> entries = mla_array_list_empty<mla_init_struct(mla_string_t)>();

    if (!mla_fs_list_directory(path, entries)) {
        return false;
    }

    out_count = mla_array_list_size(entries);
    return true;
}

mla_bool_t mla_fs_open_file(const mla_string_t& path, mla_file_system_file_open_mode mode, mla_file_system_stream_t& out_stream) {

    mla_file_system_mount_t file_system_mount = mla_file_system_mount_empty();

    if (!mla_private_file_system_find_file_system_for_file(path, file_system_mount)) {
        return false;
    }

    mla_string_t relative_path = mla_private_file_system_get_relative_path(path, file_system_mount.mount_path);

    if (file_system_mount.file_system.open_file == nullptr) {
        return false;
    }

    if (file_system_mount.file_system.open_file(file_system_mount.file_system, relative_path, mode, out_stream)) {
        out_stream.path = path; // Set the full path
        return true;
    }

    return false;
}

mla_bool_t mla_fs_copy_file_to(const mla_string_t& source_path, const mla_string_t& destination_path) {

    // Open the source file for reading
    mla_file_system_stream_t source_stream = mla_file_system_stream_empty();

    if (!mla_fs_open_file(source_path, MLA_FILE_SYSTEM_FILE_OPEN_MODE_READ, source_stream)) {
        return false;
    }

    mla_stream_input_t source_input_stream = mla_file_system_stream_as_input(source_stream);

    // Copy the contents from the source stream to the destination stream
    if (!mla_fs_copy_stream_to_file(source_input_stream, destination_path)) {
        return false;
    }

    return true;
}

mla_bool_t mla_fs_copy_file_to_stream(const mla_string_t& source_path, mla_stream_output_t& destination_stream) {

    mla_file_system_stream_t source_stream = mla_file_system_stream_empty();

    if (!mla_fs_open_file(source_path, MLA_FILE_SYSTEM_FILE_OPEN_MODE_READ, source_stream)) {
        return false;
    }

    mla_stream_input_t source_input_stream = mla_file_system_stream_as_input(source_stream);

    // Copy the contents from the source stream to the destination stream
    if (!mla_stream_copy(source_input_stream, destination_stream)) {
        return false;
    }

    return true;
}

mla_bool_t mla_fs_copy_stream_to_file(mla_stream_input_t& source_stream, const mla_string_t& destination_path) {

    // Open the destination file for writing
    mla_file_system_stream_t destination_stream = mla_file_system_stream_empty();

    if (!mla_fs_open_file(destination_path, MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE, destination_stream)) {
        return false;
    }

    mla_stream_output_t destination_output_stream = mla_file_system_stream_as_output(destination_stream);

    // Copy the contents from the source stream to the destination stream
    if (!mla_stream_copy(source_stream, destination_output_stream)) {
        return false;
    }

    destination_stream.set_length(destination_stream, destination_stream.position(destination_stream)); // Set the length of the destination file to the current position after writing

    return true;
}

mla_string_t mla_fs_get_complete_os_absolute_path(const mla_string_t& path, mla_bool_t check_if_exists) {

    mla_file_system_mount_t file_system_mount = mla_file_system_mount_empty();

    if (!mla_private_file_system_find_file_system_for_file(path, file_system_mount)) {
        return mla_string_empty();
    }

    mla_string_t relative_path = mla_private_file_system_get_relative_path(path, file_system_mount.mount_path);

    if (file_system_mount.file_system.os_absolute_path == nullptr) {
        return mla_string_empty();
    }

    mla_string_t out = mla_string_empty();

    if (!file_system_mount.file_system.os_absolute_path(file_system_mount.file_system, relative_path, check_if_exists, out)) {
        return mla_string_empty();
    }

    return out;

}

mla_string_t mla_fs_get_complete_os_absolute_path(const mla_string_t& path) {
    return mla_fs_get_complete_os_absolute_path(path, true);
}

mla_bool_t mla_fs_is_directory_path(const mla_string_t& path) {
    // Must start and end with a slash
    return mla_private_file_system_isvalid_directory_path(path);
}

mla_string_t mla_fs_get_parent_directory(const mla_string_t& path) {

    mla_int32_t last_slash_index;

    if (mla_string_length(path) == 0) {
        return mla_string_empty();
    }

    // check if the path end with a slash
    if (mla_string_ends_with(path, mla_fs_directory_seperator)) {
        // If it does, we need to find the second last slash
        // Remove the trailing slash and find the last slash in the remaining path
        mla_string_t path_without_trailing_slash = mla_string_substr(path, 0, mla_string_length(path) - 1);
        last_slash_index = mla_string_last_index_of(path_without_trailing_slash, mla_fs_directory_seperator);
    } else {
        // Find the last slash in the path
        last_slash_index = mla_string_last_index_of(path, mla_fs_directory_seperator);
    }


    if (last_slash_index < 0) {
        return mla_fs_directory_seperator; // No slashes, return root
    }

    return mla_string_substr(path, 0, last_slash_index + 1); // Include the slash

}
mla_string_t mla_fs_get_file_name(const mla_string_t& path) {

    // Find the last slash in the path
    mla_size_t last_slash_index = mla_string_last_index_of(path, mla_fs_directory_seperator);

    if (last_slash_index == mla_s_cast<mla_size_t>(-1)) {
        return path; // No slashes, return the whole path
    }

    return mla_string_substr(path, last_slash_index + 1);

}

mla_string_t mla_fs_get_file_extension(const mla_string_t& path) {

    mla_string_t file_name = mla_fs_get_file_name(path);

    mla_size_t last_dot_index = mla_string_last_index_of(file_name, mla_string_const("."));
    if (last_dot_index == mla_s_cast<mla_size_t>(-1)) {
        return mla_string_empty(); // No extension
    }

    return mla_string_substr(file_name, last_dot_index + 1);
}

mla_string_t mla_fs_change_file_extension(const mla_string_t& path, const mla_string_t& new_extension) {

    mla_string_t file_name = mla_fs_get_file_name(path);
    mla_int32_t last_dot_index = mla_string_last_index_of(file_name, mla_string_const("."));

    mla_string_t base_name = mla_string_empty();
    if (last_dot_index < 0) {
        base_name = file_name; // No extension
    } else {
        base_name = mla_string_substr(file_name, 0, last_dot_index);
    }

    mla_string_t new_file_name = mla_string_empty();
    if (mla_string_length(new_extension) == 0) {
        new_file_name = base_name; // Remove extension
    } else {
        new_file_name = mla_string_concat(base_name, mla_string_const("."), new_extension);
    }

    mla_string_t parent_directory = mla_fs_get_parent_directory(path);
    if (mla_string_length(parent_directory) == 0) {
        return new_file_name; // No parent directory
    }

    return mla_string_concat(parent_directory, new_file_name);
}

mla_string_t mla_fs_combine_paths(const mla_string_t& path1, const mla_string_t& path2) {
    mla_size_t path1_length = mla_string_length(path1);
    mla_size_t path2_length = mla_string_length(path2);

    if (path1_length == 0 && path2_length == 0) {
        return mla_fs_directory_seperator;
    }

    if (path1_length == 0) {

        if (!mla_string_starts_with(path2, mla_fs_directory_seperator)) {
            return mla_string_concat(mla_fs_directory_seperator, path2);
        }

        return path2;
    }

    if (path2_length == 0) {

        if (!mla_string_starts_with(path1, mla_fs_directory_seperator)) {
            return mla_string_concat(mla_fs_directory_seperator, path1);
        }

        return path1;
    }

    mla_string_t p1 = path1;
    mla_string_t p2 = path2;

    if (mla_string_starts_with(p1, mla_fs_directory_seperator)) {
        p1 = mla_string_substr(p1, 1);
    }

    if (mla_string_ends_with(p1, mla_fs_directory_seperator)) {
        p1 = mla_string_substr(p1, 0, mla_string_length(p1) - 1);
    }

    if (mla_string_starts_with(p2, mla_fs_directory_seperator)) {
        p2 = mla_string_substr(p2, 1);
    }

    if (mla_string_is_empty(p1)) {
        return mla_string_concat(mla_fs_directory_seperator, p2);
    } else {
        return mla_string_concat(mla_fs_directory_seperator, p1, mla_fs_directory_seperator, p2);
    }
}

mla_string_t mla_fs_combine_paths(const mla_string_t& path1, const mla_string_t& path2, const mla_string_t& path3) {

    mla_size_t len1 = mla_string_length(path1);
    mla_size_t len2 = mla_string_length(path2);
    mla_size_t len3 = mla_string_length(path3);

    // All empty → return a single separator
    if (len1 == 0 && len2 == 0 && len3 == 0) {
        return mla_fs_directory_seperator;
    }

    // Normalize paths
    mla_string_t p1 = path1;
    mla_string_t p2 = path2;
    mla_string_t p3 = path3;

    // Handle when any path is empty, ensuring separators are correct
    // (similar to your 2‑path version but covering 3 inputs)

    if (mla_string_starts_with(p1, mla_fs_directory_seperator)) {
        p1 = mla_string_substr(p1, 1);
    }

    // Trim trailing slash from p1 if present
    if (mla_string_ends_with(p1, mla_fs_directory_seperator)) {
        p1 = mla_string_substr(p1, 0, mla_string_length(p1) - 1);
    }

    // Trim leading and trailing slashes for middle path (p2)
    if (len1 != 1 && mla_string_starts_with(p2, mla_fs_directory_seperator)) {
        p2 = mla_string_substr(p2, 1);
    }
    if (len2 != 1 && mla_string_ends_with(p2, mla_fs_directory_seperator)) {
        p2 = mla_string_substr(p2, 0, mla_string_length(p2) - 1);
    }

    // Trim leading slash from last path
    if (len3 != 1 && mla_string_starts_with(p3, mla_fs_directory_seperator)) {
        p3 = mla_string_substr(p3, 1);
    }

    // Handle any empty parts gracefully
    if (mla_string_length(p1) == 0 && mla_string_length(p2) == 0) {
        return mla_fs_combine_paths(p3, mla_string_empty()); // → just normalized p3
    }

    if (mla_string_length(p1) == 0) {
        return mla_fs_combine_paths(p2, p3);
    }

    if (mla_string_length(p2) == 0) {
        return mla_fs_combine_paths(p1, p3);
    }

    if (mla_string_length(p3) == 0) {
        return mla_fs_combine_paths(p1, p2);
    }

    // Combine efficiently in one shot
    return mla_string_concat(mla_fs_directory_seperator, p1, mla_fs_directory_seperator, p2, mla_fs_directory_seperator, p3);

}

mla_string_t mla_fs_get_relative_path(const mla_string_t& base_path, const mla_string_t& target_path, mla_bool_t navigate_up_if_needed) {

    mla_size_t base_length = mla_string_length(base_path);
    mla_size_t target_length = mla_string_length(target_path);

    if (base_length == 0 || target_length == 0) {
        return mla_string_empty();
    }

    if (!mla_string_starts_with(base_path, mla_fs_directory_seperator) || !mla_string_starts_with(target_path, mla_fs_directory_seperator)) {
        return mla_string_empty();
    }

    if (navigate_up_if_needed) {

        mla_array_list_t<mla_init_struct(mla_string_t)> base_parts = mla_string_split(base_path, mla_fs_directory_seperator);
        mla_array_list_t<mla_init_struct(mla_string_t)> target_parts = mla_string_split(target_path, mla_fs_directory_seperator);

        mla_size_t base_count = mla_array_list_size(base_parts);
        mla_size_t target_count = mla_array_list_size(target_parts);

        mla_size_t i = 0;
        while (i < base_count && i < target_count) {
            mla_string_t base_part = mla_array_list_get_unsafe(base_parts, i);
            mla_string_t target_part = mla_array_list_get_unsafe(target_parts, i);

            if (!mla_string_equals(base_part, target_part)) {
                break;
            }
            i++;
        }

        mla_string_t result = mla_string_empty();

        mla_size_t up_count = 0;
        for (mla_size_t j = i; j < base_count; j++) {
            mla_string_t base_part = mla_array_list_get_unsafe(base_parts, j);
            if (!mla_string_is_empty(base_part)) {
                up_count++;
            }
        }

        for (mla_size_t j = 0; j < up_count; j++) {
            if (mla_string_is_empty(result)) {
                result = mla_string_concat(mla_fs_directory_seperator, mla_string_const(".."));
            } else {
                result = mla_string_concat(result, mla_fs_directory_seperator, mla_string_const(".."));
            }
        }

        for (mla_size_t j = i; j < target_count; j++) {
            mla_string_t target_part = mla_array_list_get_unsafe(target_parts, j);
            if (!mla_string_is_empty(target_part)) {
                if (mla_string_is_empty(result)) {
                    result = mla_string_concat(mla_fs_directory_seperator, target_part);
                } else {
                    result = mla_string_concat(result, mla_fs_directory_seperator, target_part);
                }
            }
        }

        if (mla_string_is_empty(result)) {
             result = mla_fs_directory_seperator;
        } else if (mla_string_ends_with(target_path, mla_fs_directory_seperator)) {
             result = mla_string_concat(result, mla_fs_directory_seperator);
        }

        return result;
    }

    if (base_length > target_length) {
        return mla_string_empty();
    }

    if (!mla_string_starts_with(target_path, base_path)) {
        return mla_string_empty();
    }

    return mla_string_substr(target_path, base_length - 1); // -1 to include the slash at the end of base_path
}

mla_user_data_id_init(mla_file_system_stream_userdata_id);

mla_size_t mla_private_file_system_stream_as_input_read(mla_stream_input_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {

    mla_file_system_stream_t* fs_stream = mla_user_data_get_struct_data<mla_file_system_stream_t>(input.userdata, mla_file_system_stream_userdata_id);

    if (fs_stream == nullptr || fs_stream->read == nullptr) {
        return 0;
    }

    return fs_stream->read(*fs_stream, offset, length, buffer);
}

mla_size_t mla_private_file_system_stream_as_input_remaining_bytes(mla_stream_input_t& input) {

    mla_file_system_stream_t* fs_stream = mla_user_data_get_struct_data<mla_file_system_stream_t>(input.userdata, mla_file_system_stream_userdata_id);

    if (fs_stream == nullptr || fs_stream->position == nullptr || fs_stream->length == nullptr) {
        return 0;
    }

    mla_file_system_stream_t stream = *fs_stream;

    return stream.length(stream) - stream.position(stream);
}


mla_stream_input_t mla_file_system_stream_as_input(const mla_file_system_stream_t& stream) {

    if (stream.read == nullptr) {
        return mla_stream_noop_input();
    }

    mla_user_data_t userdata = mla_user_data_empty();
    mla_user_data_set_struct(userdata, mla_file_system_stream_userdata_id, stream);

    if (stream.position != nullptr && stream.length != nullptr) {
        // We can calculate the remaining bytes
        return {
            userdata,
            mla_private_file_system_stream_as_input_read,
            mla_private_file_system_stream_as_input_remaining_bytes
        };
    } else {
        // We don't know the remaining bytes, return nullptr for the function pointer
        return {
            userdata,
            mla_private_file_system_stream_as_input_read,
            nullptr
        };
    }
}


mla_size_t mla_private_file_system_stream_as_output_write(mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {

    mla_file_system_stream_t* fs_stream = mla_user_data_get_struct_data<mla_file_system_stream_t>(output.userdata, mla_file_system_stream_userdata_id);

    if (fs_stream == nullptr || fs_stream->write == nullptr || buffer == nullptr) {
        return 0;
    }

    return fs_stream->write(*fs_stream, offset, length, buffer);
}

mla_size_t mla_private_file_system_stream_as_output_available_bytes(mla_stream_output_t& output) {
    (void)output;
    return mla_size_max; // We don't know the available bytes, return max size_t
}


mla_stream_output_t mla_file_system_stream_as_output(const mla_file_system_stream_t& stream) {

    if (stream.write == nullptr) {
        return mla_stream_noop_output();
    }

    mla_user_data_t userdata = mla_user_data_empty();
    mla_user_data_set_struct(userdata, mla_file_system_stream_userdata_id, stream);

    return {
        userdata,
        mla_private_file_system_stream_as_output_write,
        mla_private_file_system_stream_as_output_available_bytes
    };
}

mla_lifecycle_boot_event_static_register(mla_lifecycle_boot_event_priority_file_system_setup, mla_file_system_lock)
