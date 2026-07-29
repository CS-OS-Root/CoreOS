//
// Created by chris on 10/13/2025.
//

#ifndef MLA_FILE_SYSTEM_H
#define MLA_FILE_SYSTEM_H

#include "../mla_data_types.h"
#include "../system/mla_string.h"
#include "../system/mla_array_list.h"
#include "../system/mla_stream.h"

enum mla_file_system_file_open_mode: mla_uint8_t {
    MLA_FILE_SYSTEM_FILE_OPEN_MODE_READ,
    MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE,
    MLA_FILE_SYSTEM_FILE_OPEN_MODE_READ_AND_WRITE
};

struct mla_file_system_stream_t {
    mla_string_t path;

    mla_bool_t (*seek)(const mla_file_system_stream_t& stream, mla_size_t offset);
    mla_size_t (*position)(const mla_file_system_stream_t& stream);
    mla_size_t (*length)(const mla_file_system_stream_t& stream);
    mla_bool_t (*set_length)(const mla_file_system_stream_t& stream, mla_size_t length); // Optional, can be nullptr if not supported

    mla_size_t (*read)(const mla_file_system_stream_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer); // Optional, can be nullptr if not supported
    mla_size_t (*write)(const mla_file_system_stream_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer); // Optional, can be nullptr if not supported

    mla_pointer_t resource;

    static mla_file_system_stream_t init();
};

/**
* @brief Converts a file system stream to a generic stream input interface.
*
* This function creates an `mla_stream_input_t` that wraps the provided
* `mla_file_system_stream_t`. The resulting input stream will use the read
* callback of the file system stream for reading data. If the file system
* stream does not support reading (i.e., its read callback is `nullptr`),
* the resulting input stream will be a no-op stream that always returns 0 bytes read.
*
* @param stream The file system stream to convert.
* @return An `mla_stream_input_t` that can be used for reading from the file system stream.
*/
mla_stream_input_t mla_file_system_stream_as_input(const mla_file_system_stream_t& stream);

/**
* @brief Converts a file system stream to a generic stream output interface.
* This function creates an `mla_stream_output_t` that wraps the provided
* `mla_file_system_stream_t`. The resulting output stream will use the write
* callback of the file system stream for writing data. If the file system stream does not support writing (i.e., its write callback is `nullptr`), the resulting output stream will be a no-op stream that always returns the length of the data as written without actually writing anything.
*
* @param stream The file system stream to convert.
* @return An `mla_stream_output_t` that can be used for writing to the file system stream.
 */
mla_stream_output_t mla_file_system_stream_as_output(const mla_file_system_stream_t& stream);

/**
 * @brief Virtual file system interface for mounting custom file system implementations.
 *
 * This structure defines a set of function pointers that provide file and directory
 * operations for a mounted file system. Each file system implementation must provide
 * these callbacks to integrate with the file system manager.
 *
 * All paths passed to these callbacks are relative to the mount point and have been
 * normalized (lowercase). The file system manager handles mount point resolution
 * before invoking these callbacks.
 *
 * @note All callback functions should return `true` on success, `false` on failure.
 * @note Callbacks can be set to `nullptr` if the operation is not supported.
 */
struct mla_file_system_t {

    mla_bool_t (*file_exists)(mla_file_system_t& file_system, const mla_string_t& path);
    mla_bool_t (*open_file)(mla_file_system_t& file_system, const mla_string_t& path, mla_file_system_file_open_mode mode, mla_file_system_stream_t& out_stream);
    mla_bool_t (*delete_file)(mla_file_system_t& file_system, const mla_string_t& path);
    mla_bool_t (*list_files)(mla_file_system_t& file_system, const mla_string_t& path, mla_array_list_t<mla_init_struct(mla_string_t)>& out_entries);

    mla_bool_t (*create_directory)(mla_file_system_t& file_system, const mla_string_t& path);
    mla_bool_t (*directory_exists)(mla_file_system_t& file_system, const mla_string_t& path);
    mla_bool_t (*delete_directory)(mla_file_system_t& file_system, const mla_string_t& path);
    mla_bool_t (*list_directory)(mla_file_system_t& file_system, const mla_string_t& path, mla_array_list_t<mla_init_struct(mla_string_t)>& out_entries);

    mla_bool_t (*os_absolute_path)(mla_file_system_t& file_system, const mla_string_t& path, mla_bool_t check_if_exists, mla_string_t& out);

    mla_pointer_t resource;
};

mla_file_system_stream_t mla_file_system_stream_empty();
mla_file_system_t mla_file_system_empty();

mla_bool_t mla_file_system_initialize(const mla_string_t& mount_path, const mla_file_system_t& file_system);
mla_bool_t mla_file_system_deinitialize(const mla_string_t& mount_path);

void mla_file_system_lock();
void mla_file_system_unlock();
mla_bool_t mla_file_system_is_locked();

//// File operations

mla_bool_t mla_fs_file_exists(const mla_string_t& path);
mla_bool_t mla_fs_delete_file(const mla_string_t& path);
mla_bool_t mla_fs_list_files(const mla_string_t& path, mla_array_list_t<mla_init_struct(mla_string_t)>& out_entries);
mla_bool_t mla_fs_count_files(const mla_string_t& path, mla_size_t& out_count);

mla_bool_t mla_fs_open_file(const mla_string_t& path, mla_file_system_file_open_mode mode, mla_file_system_stream_t& out_stream);

mla_bool_t mla_fs_copy_file_to(const mla_string_t& source_path, const mla_string_t& destination_path);
mla_bool_t mla_fs_copy_file_to_stream(const mla_string_t& source_path, mla_stream_output_t& destination_stream);
mla_bool_t mla_fs_copy_stream_to_file(mla_stream_input_t& source_stream, const mla_string_t& destination_path);

//// Directory operations
mla_bool_t mla_fs_directory_exists(const mla_string_t& path);

/**
 * @brief Creates a directory and any missing parent directories.
 *
 * @param path The absolute virtual file-system directory path to create.
 * @return `true` if the complete directory path exists after the operation, otherwise `false`.
 */
mla_bool_t mla_fs_create_directory(const mla_string_t& path);

/**
 * @brief Deletes a directory recursively, including all contained files and subdirectories.
 *
 * @param path The absolute virtual file-system directory path to delete.
 * @return `true` if the directory and all of its contents were deleted, otherwise `false`.
 */
mla_bool_t mla_fs_delete_directory(const mla_string_t& path);
mla_bool_t mla_fs_list_directory(const mla_string_t& path, mla_array_list_t<mla_init_struct(mla_string_t)>& out_entries);
mla_bool_t mla_fs_count_directory(const mla_string_t& path, mla_size_t& out_count);

/**
 * @brief Recursively copies a directory and all of its contents.
 *
 * Existing destination directories are merged and destination files with matching names are overwritten.
 * Copying a directory onto itself or into one of its descendants is not supported.
 *
 * @param source_path The existing absolute virtual file-system directory path to copy.
 * @param destination_path The absolute virtual file-system directory path to create or merge into.
 * @return `true` if the complete directory tree was copied, otherwise `false`.
 */
mla_bool_t mla_fs_copy_directory(const mla_string_t& source_path, const mla_string_t& destination_path);

// Helpers
mla_bool_t mla_fs_is_directory_path(const mla_string_t& path);
mla_string_t mla_fs_get_parent_directory(const mla_string_t& path);
mla_string_t mla_fs_get_file_name(const mla_string_t& path);
mla_string_t mla_fs_get_file_extension(const mla_string_t& path);
mla_string_t mla_fs_change_file_extension(const mla_string_t& path, const mla_string_t& new_extension);

mla_string_t mla_fs_combine_paths(const mla_string_t& path1, const mla_string_t& path2);
mla_string_t mla_fs_combine_paths(const mla_string_t& path1, const mla_string_t& path2, const mla_string_t& path3);

mla_string_t mla_fs_get_relative_path(const mla_string_t& base_path, const mla_string_t& target_path, mla_bool_t navigate_up_if_needed);


mla_string_t mla_fs_get_complete_os_absolute_path(const mla_string_t& path);
mla_string_t mla_fs_get_complete_os_absolute_path(const mla_string_t& path, mla_bool_t check_if_exists);

/// Const
static const mla_string_t mla_fs_directory_seperator = mla_string_const("/");
static const mla_string_t mla_fs_root_directory = mla_fs_directory_seperator;

#endif
