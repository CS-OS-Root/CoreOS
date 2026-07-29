//
// Created by chris on 10/14/2025.
//

#include "mla_file_system_inmemory.h"
#include "../../mla_data_types.h"
#include "../../system/mla_string_concat.h"

#define mla_file_system_inmemory_buffer_item_size 512 // 512 bytes per item

enum mla_file_system_inmemory_buffer_item_type: mla_uint8_t {
    /**
     * NO Layout all zero
     */
    MLA_FILE_SYSTEM_INMEMORY_BUFFER_ITEM_TYPE_FREE = 0,
    /**
     * File item
     * Followed by the file meta data (mla_file_system_inmemory_buffer_file_meta_data_t)
     * Followed by the file name as string data
     * If the file name is larger than the item size it will be continued in the next item(s)
     * next == mla_size_max means no more items
     */
    MLA_FILE_SYSTEM_INMEMORY_BUFFER_ITEM_TYPE_FILE = 1,
    /**
     * Directory item
     * Followed by the directory name as string data
     * If the directory name is larger than the item size it will be continued in the next item(s)
     * next == mla_size_max means no more items
     */
    MLA_FILE_SYSTEM_INMEMORY_BUFFER_ITEM_TYPE_DIRECTORY = 2,
    /**
     * Data item
     * Followed by the file data
     * If the file data is larger than the item size it will be continued in the next item(s)
     * next == mla_size_max means no more items
     */
    MLA_FILE_SYSTEM_INMEMORY_BUFFER_ITEM_TYPE_DATA = 3
};

struct mla_file_system_inmemory_buffer_item_header_t {
    mla_file_system_inmemory_buffer_item_type type;
    mla_size_t next; // Offset to the next item in the buffer
    mla_size_t size; // Size of the data in this item
};

struct mla_file_system_inmemory_buffer_file_meta_data_t {
    mla_size_t size; // Size of the file
    mla_size_t firstDataOffset; // Offset to the first data item
};

struct mla_file_system_inmemory_t {
    mla_size_t capacity;
    mla_byte_t *buffer;
};

mla_size_t mla_private_file_system_inmemory_get_buffer_position(mla_size_t offset) {
    return offset * (sizeof(mla_file_system_inmemory_buffer_item_header_t) + mla_file_system_inmemory_buffer_item_size);
}

mla_bool_t mla_private_file_system_inmemory_find_next_buffer_of_type(mla_file_system_inmemory_t &fs, mla_size_t startOffset,
                                                        mla_file_system_inmemory_buffer_item_type type,
                                                        mla_size_t &out_offset) {

    mla_size_t currentOffset = startOffset;

    while (mla_private_file_system_inmemory_get_buffer_position(currentOffset) < fs.capacity) {

        mla_file_system_inmemory_buffer_item_header_t *header = mla_r_cast<mla_file_system_inmemory_buffer_item_header_t *>(fs.buffer + mla_private_file_system_inmemory_get_buffer_position(currentOffset));

        if (header->type == type) {
            out_offset = currentOffset;
            return true;
        }

        currentOffset++;
    }

    return false;
}

mla_bool_t mla_private_file_system_inmemory_find_next_free_header(mla_file_system_inmemory_t &fs, mla_file_system_inmemory_buffer_item_header_t* &header) {

    mla_size_t currentOffset = 0;

    while (mla_private_file_system_inmemory_get_buffer_position(currentOffset) < fs.capacity) {

        mla_file_system_inmemory_buffer_item_header_t *currentHeader = mla_r_cast<mla_file_system_inmemory_buffer_item_header_t *>(fs.buffer + mla_private_file_system_inmemory_get_buffer_position(currentOffset));

        if (currentHeader->type == MLA_FILE_SYSTEM_INMEMORY_BUFFER_ITEM_TYPE_FREE) {
            header = currentHeader;
            return true;
        }

        currentOffset++;
    }

    return false;
}

mla_string_t mla_private_file_system_inmemory_read_string_data(mla_file_system_inmemory_t &fs, mla_file_system_inmemory_buffer_item_header_t *header) {

    mla_string_t result = mla_string_empty();

    if (header->size == 0 || header->size > mla_file_system_inmemory_buffer_item_size) {
        return result;
    }

    mla_bool_t first = true;

    while (header != nullptr) {

        mla_size_t dataSize = header->size;

        if (dataSize > mla_file_system_inmemory_buffer_item_size) {
            dataSize = mla_file_system_inmemory_buffer_item_size;
        }

        mla_char_t * dataPosition = mla_r_cast<mla_char_t *>(header) + sizeof(mla_file_system_inmemory_buffer_item_header_t);

        if (first) {
            first = false;

            // Add also the file meta data to the offset
            dataPosition = dataPosition + sizeof(mla_file_system_inmemory_buffer_file_meta_data_t);
        }

        result = mla_string_concat(result, mla_string_from_buffer_without_ownership(dataPosition, dataSize));

        if (header->next == mla_size_max) {
            break; // No more data
        }

        header = mla_r_cast<mla_file_system_inmemory_buffer_item_header_t *>(fs.buffer + mla_private_file_system_inmemory_get_buffer_position(header->next));
    }

    return result;
}

mla_bool_t mla_private_file_system_inmemory_find_file_header(mla_file_system_inmemory_t &fs, const mla_string_t &path,
                                               mla_file_system_inmemory_buffer_item_header_t* &out_header) {

    mla_size_t lastOffset = 0;
    mla_size_t currentOffset = 0;

    while (mla_private_file_system_inmemory_find_next_buffer_of_type(fs, lastOffset, MLA_FILE_SYSTEM_INMEMORY_BUFFER_ITEM_TYPE_FILE, currentOffset)) {

        mla_byte_t* position = fs.buffer + mla_private_file_system_inmemory_get_buffer_position(currentOffset);

        // Read the file header
        mla_file_system_inmemory_buffer_item_header_t *header = mla_r_cast<mla_file_system_inmemory_buffer_item_header_t *>(position);

        // Read the file name
        mla_string_t fileName = mla_private_file_system_inmemory_read_string_data(fs, header);

        if (mla_string_equals(path, fileName)) {
            out_header = header;
            return true;
        }

        lastOffset = currentOffset + 1;
    }

    return false;
}

mla_bool_t mla_private_file_system_inmemory_file_exists(mla_file_system_t &file_system, const mla_string_t &path) {

    mla_file_system_inmemory_t fs = *mla_r_cast<mla_file_system_inmemory_t *>(file_system.user_data.asPointer);
    mla_file_system_inmemory_buffer_item_header_t* ignored = nullptr;
    return mla_private_file_system_inmemory_find_file_header(fs, path, ignored);

}

mla_bool_t mla_private_file_system_inmemory_open_file(mla_file_system_t& file_system, const mla_string_t& path, mla_file_system_file_open_mode mode, mla_file_system_stream_t& out_stream) {
    return false;
}

mla_bool_t mla_private_file_system_inmemory_find_file_header_read_meta_data(mla_file_system_inmemory_t &fs,
                                                               mla_file_system_inmemory_buffer_item_header_t* header,
                                                               mla_file_system_inmemory_buffer_file_meta_data_t &out_meta_data) {

    if (header == nullptr || header->type != MLA_FILE_SYSTEM_INMEMORY_BUFFER_ITEM_TYPE_FILE) {
        return false;
    }

    if (header->size < sizeof(mla_file_system_inmemory_buffer_file_meta_data_t)) {
        return false; // Invalid size
    }

    mla_byte_t* position = mla_r_cast<mla_byte_t *>(header) + sizeof(mla_file_system_inmemory_buffer_item_header_t);
    mla_file_system_inmemory_buffer_file_meta_data_t* metaData = mla_r_cast<mla_file_system_inmemory_buffer_file_meta_data_t *>(position);

    out_meta_data.size = metaData->size;
    out_meta_data.firstDataOffset = metaData->firstDataOffset;

    return true;
}

void mla_private_file_system_inmemory_free_item_chain(mla_file_system_inmemory_t &fs, mla_file_system_inmemory_buffer_item_header_t* header) {

    while (header != nullptr) {

        mla_size_t nextOffset = header->next;

        // Mark the item as free
        header->type = MLA_FILE_SYSTEM_INMEMORY_BUFFER_ITEM_TYPE_FREE;
        header->next = mla_size_max;
        header->size = 0;

        if (nextOffset == mla_size_max) {
            break; // No more items
        }

        header = mla_r_cast<mla_file_system_inmemory_buffer_item_header_t *>(fs.buffer + mla_private_file_system_inmemory_get_buffer_position(nextOffset));
    }


}

mla_bool_t mla_private_file_system_inmemory_delete_file(mla_file_system_t &file_system, const mla_string_t &path) {

    mla_file_system_inmemory_t fs = *mla_r_cast<mla_file_system_inmemory_t *>(file_system.user_data.asPointer);
    mla_file_system_inmemory_buffer_item_header_t* header = nullptr;

    if (!mla_private_file_system_inmemory_find_file_header(fs, path, header)) {
        return false; // File not found
    }

    mla_file_system_inmemory_buffer_file_meta_data_t meta_data;

    if (mla_private_file_system_inmemory_find_file_header_read_meta_data(fs, header, meta_data)) {

        if (meta_data.firstDataOffset != mla_size_max) {

            mla_file_system_inmemory_buffer_item_header_t* data_header = mla_r_cast<mla_file_system_inmemory_buffer_item_header_t *>(
                fs.buffer + mla_private_file_system_inmemory_get_buffer_position(meta_data.firstDataOffset));

            mla_private_file_system_inmemory_free_item_chain(fs, data_header);
        }

    }

    mla_private_file_system_inmemory_free_item_chain(fs, header);
    return true;
}

mla_bool_t mla_private_file_system_inmemory_list_files(mla_file_system_t &file_system, const mla_string_t &path,
                                                 mla_array_list_t<mla_init_struct(mla_string_t)> &out_entries) {
    return false;
}

mla_bool_t mla_private_file_system_inmemory_find_directory_header(mla_file_system_inmemory_t &fs, const mla_string_t &path,
                                                    mla_file_system_inmemory_buffer_item_header_t* &out_header) {

    mla_size_t lastOffset = 0;
    mla_size_t currentOffset = 0;

    while (mla_private_file_system_inmemory_find_next_buffer_of_type(fs, lastOffset, MLA_FILE_SYSTEM_INMEMORY_BUFFER_ITEM_TYPE_DIRECTORY, currentOffset)) {

        mla_byte_t* position = fs.buffer + mla_private_file_system_inmemory_get_buffer_position(currentOffset);

        // Read the directory header
        mla_file_system_inmemory_buffer_item_header_t *header = mla_r_cast<mla_file_system_inmemory_buffer_item_header_t *>(position);

        // Read the directory name
        mla_string_t dirName = mla_private_file_system_inmemory_read_string_data(fs, header);

        if (mla_string_equals(path, dirName)) {
            out_header = header;
            return true;
        }

        lastOffset = currentOffset + 1;
    }

    return false;
}

mla_bool_t mla_private_file_system_inmemory_create_directory(mla_file_system_t &file_system, const mla_string_t &path) {

    if (mla_fs_is_directory_path(path)) {
        return false;
    }

    if (mla_string_equals(path, mla_fs_root_directory)) {
        return true; // Root directory always exists
    }

    mla_file_system_inmemory_t fs = *mla_r_cast<mla_file_system_inmemory_t *>(file_system.user_data.asPointer);

    mla_string_t parent_path = mla_fs_get_parent_directory(path);

    if (!mla_string_equals(path, mla_fs_root_directory)) {

        mla_file_system_inmemory_buffer_item_header_t* ignored = nullptr;

        // Check if parent directory exists
        if (!mla_private_file_system_inmemory_find_directory_header(fs, parent_path, ignored)) {
            return false; // Parent directory does not exist
        }

    }

    mla_file_system_inmemory_buffer_item_header_t* freeBlock = nullptr;

    if (!mla_private_file_system_inmemory_find_next_free_header(fs, freeBlock)) {
        return false;
    }

    freeBlock->type = MLA_FILE_SYSTEM_INMEMORY_BUFFER_ITEM_TYPE_DIRECTORY;
    freeBlock->next = mla_size_max;

    //mla_private_file_system_inmemory_read_string_data()


    return true;
}

mla_bool_t mla_private_file_system_inmemory_directory_exists(mla_file_system_t &file_system, const mla_string_t &path) {

    mla_file_system_inmemory_t fs = *mla_r_cast<mla_file_system_inmemory_t *>(file_system.user_data.asPointer);
    mla_file_system_inmemory_buffer_item_header_t* ignored = nullptr;
    return mla_private_file_system_inmemory_find_directory_header(fs, path, ignored);
}

mla_bool_t mla_private_file_system_inmemory_delete_directory(mla_file_system_t &file_system, const mla_string_t &path) {

    mla_file_system_inmemory_t fs = *mla_r_cast<mla_file_system_inmemory_t *>(file_system.user_data.asPointer);
    mla_file_system_inmemory_buffer_item_header_t* header = nullptr;

    if (!mla_private_file_system_inmemory_find_directory_header(fs, path, header)) {
        return false; // Directory not found
    }


    // Delete all files in the directory
    // Delete all sub directories in the directory
    // Delete the directory itself



    mla_private_file_system_inmemory_free_item_chain(fs, header);
    return true;
}

mla_bool_t mla_private_file_system_inmemory_list_directories(mla_file_system_t &file_system, const mla_string_t &path,
                                                       mla_array_list_t<mla_init_struct(mla_string_t)> &
                                                       out_entries) {
    return false;
}

mla_buffer_cleanup_mode mla_private_file_system_inmemory_cleanup(mla_platform_pointer_t data, const mla_dynamic_data_t& userData) {

    mla_file_system_inmemory_t *fs = mla_s_cast<mla_file_system_inmemory_t *>(data);

    if (fs == nullptr) {
        return CLEAN_UP_SKIP;
    }

    mla_platform_free(fs->buffer);
    fs->buffer = nullptr;
    fs->capacity = 0;

    return CLEAN_UP_NEEDED;
}

mla_file_system_t mla_file_system_inmemory_create_from_buffer(mla_byte_t *buffer, mla_size_t capacity) {

    mla_file_system_inmemory_t *fs = mla_s_cast<mla_file_system_inmemory_t *>(mla_platform_malloc(
        sizeof(mla_file_system_inmemory_t)));

    if (fs == nullptr)
        return mla_file_system_empty();

    mla_memset(buffer, 0 ,capacity);
    mla_memcpy(fs, 0, sizeof(mla_file_system_inmemory_t));

    fs->buffer = buffer;
    fs->capacity = capacity;

    return {
        mla_private_file_system_inmemory_file_exists,
        mla_private_file_system_inmemory_open_file,
        mla_private_file_system_inmemory_delete_file,
        mla_private_file_system_inmemory_list_files,
        mla_private_file_system_inmemory_create_directory,
        mla_private_file_system_inmemory_directory_exists,
        mla_private_file_system_inmemory_delete_directory,
        mla_private_file_system_inmemory_list_directories,
        mla_dynamic_data_from_pointer(fs),
        mla_buffer_reference_create(fs, true, mla_private_file_system_inmemory_cleanup, mla_dynamic_data_empty())
    };
}


mla_file_system_t mla_file_system_inmemory_create(mla_size_t capacity) {

    mla_byte_t *buffer = mla_s_cast<mla_byte_t *>(mla_platform_malloc(capacity * sizeof(mla_byte_t)));

    if (buffer == nullptr) {
        return mla_file_system_empty();
    }

    return mla_file_system_inmemory_create_from_buffer(buffer, capacity);
}
