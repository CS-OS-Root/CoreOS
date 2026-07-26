//
// Created by christian on 9/13/25.
//

#include "mla_cli_history.h"
#include "../serializer/mla_json_serializer.h"

mla_cli_history_store_t mla_cli_history_store_init(mla_size_t max_per_key) {
    return mla_cli_history_store_t::init(max_per_key);
}

mla_cli_history_store_t &mla_cli_history_global_store() {
    static mla_cli_history_store_t global_store = mla_cli_history_store_t::init(10);
    return global_store;
}

void mla_cli_history_record_value(mla_cli_history_store_t &store, const mla_string_t &key, const mla_string_t &value) {
    if (mla_string_is_empty(key) || mla_string_is_empty(value)) {
        return;
    }

    // Remove existing entry with matching key and value if present
    for (mla_size_t i = 0; i < mla_array_list_size(store.entries); ++i) {
        const mla_cli_history_entry_t &entry = mla_array_list_get_unsafe(store.entries, i);
        if (mla_string_equals(entry.key, key) && mla_string_equals(entry.value, value)) {
            mla_array_list_remove(store.entries, i);
            break;
        }
    }

    // Count existing entries for this key
    mla_size_t count_for_key = 0;
    for (mla_size_t i = 0; i < mla_array_list_size(store.entries); ++i) {
        const mla_cli_history_entry_t &entry = mla_array_list_get_unsafe(store.entries, i);
        if (mla_string_equals(entry.key, key)) {
            count_for_key++;
        }
    }

    // Evict oldest entry for this key if count exceeds limit
    if (count_for_key >= store.max_entries_per_key && store.max_entries_per_key > 0) {
        for (mla_size_t i = 0; i < mla_array_list_size(store.entries); ++i) {
            const mla_cli_history_entry_t &entry = mla_array_list_get_unsafe(store.entries, i);
            if (mla_string_equals(entry.key, key)) {
                mla_array_list_remove(store.entries, i);
                break;
            }
        }
    }

    mla_cli_history_entry_t new_entry = { key, value };
    mla_array_list_add(store.entries, new_entry);
}

mla_array_list_t<mla_string_t, mla_string_initializer> mla_cli_history_get_candidates(const mla_cli_history_store_t &store,
                                                                                      const mla_string_t &key,
                                                                                      const mla_string_t &prefix) {
    mla_array_list_t<mla_string_t, mla_string_initializer> result =
        mla_array_list_empty<mla_string_t, mla_string_initializer>();

    for (mla_size_t i = 0; i < mla_array_list_size(store.entries); ++i) {
        const mla_cli_history_entry_t &entry = mla_array_list_get_unsafe(store.entries, i);
        if (mla_string_equals(entry.key, key)) {
            if (mla_string_starts_with(entry.value, prefix)) {
                // Check if already in result list to prevent duplicates
                mla_bool_t exists = false;
                for (mla_size_t j = 0; j < mla_array_list_size(result); ++j) {
                    if (mla_string_equals(mla_array_list_get_unsafe(result, j), entry.value)) {
                        exists = true;
                        break;
                    }
                }
                if (!exists) {
                    mla_array_list_add(result, entry.value);
                }
            }
        }
    }
    return result;
}

mla_bool_t mla_cli_history_save_to_file(const mla_cli_history_store_t &store, const mla_string_t &vfs_file_path) {
    mla_file_system_stream_t fileStream = mla_file_system_stream_empty();
    if (!mla_fs_open_file(vfs_file_path, MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE, fileStream)) {
        return false;
    }

    mla_stream_output_t output = mla_file_system_stream_as_output(fileStream);
    mla_serializer_t serializer = mla_json_serializer(output);
    return mla_serializer_write_data_struct(serializer, store);
}

mla_bool_t mla_cli_history_load_from_file(mla_cli_history_store_t &store, const mla_string_t &vfs_file_path) {
    mla_file_system_stream_t fileStream = mla_file_system_stream_empty();
    if (!mla_fs_open_file(vfs_file_path, MLA_FILE_SYSTEM_FILE_OPEN_MODE_READ, fileStream)) {
        return false;
    }

    mla_stream_input_t input = mla_file_system_stream_as_input(fileStream);
    mla_deserializer_t deserializer = mla_json_deserializer(input);
    return mla_serializer_read_data_struct(deserializer, store);
}
