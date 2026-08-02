//
// Created by christian on 9/13/25.
//

#ifndef MLA_CLI_HISTORY_H
#define MLA_CLI_HISTORY_H

#include "../system/mla_string.h"
#include "../system/mla_array_list.h"
#include "../serializer/mla_serializer.h"
#include "../filesystem/mla_file_system.h"

/**
 * @struct mla_cli_history_entry_t
 * @brief Represents a single parameter history entry.
 */
struct mla_cli_history_entry_t {
    mla_string_t key;   /**< History lookup key (e.g., "sandbox:create:image") */
    mla_string_t value; /**< Recorded parameter value */

    static mla_cli_history_entry_t init() {
        return {
            mla_string_empty(),
            mla_string_empty()
        };
    }

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_cli_history_entry_t* self = mla_pointer_get_data<const mla_cli_history_entry_t>(obj);
        if (self == nullptr) {
            return false;
        }
        mla_serializer_write_string(serializer, mla_string_const("key"), self->key);
        mla_serializer_write_string(serializer, mla_string_const("value"), self->value);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_cli_history_entry_t* self = mla_pointer_get_data<mla_cli_history_entry_t>(obj);
        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }
        if (mla_string_equals_const(property_name, "key")) {
            mla_deserializer_read_string(deserializer, self->key);
        } else if (mla_string_equals_const(property_name, "value")) {
            mla_deserializer_read_string(deserializer, self->value);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }
};

/**
 * @struct mla_cli_history_store_t
 * @brief Store holding recorded CLI parameter values per key.
 */
struct mla_cli_history_store_t {
    mla_array_list_t<mla_cli_history_entry_t, mla_cli_history_entry_t> entries;
    mla_size_t max_entries_per_key;

    static mla_cli_history_store_t init(mla_size_t max_per_key = 10) {
        return {
            mla_array_list_empty<mla_cli_history_entry_t, mla_cli_history_entry_t>(),
            max_per_key
        };
    }

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_cli_history_store_t* self = mla_pointer_get_data<const mla_cli_history_store_t>(obj);
        if (self == nullptr) {
            return false;
        }
        mla_serializer_write_list_struct(serializer, mla_string_const("entries"), self->entries, mla_cli_history_entry_t);
        mla_serializer_write_uint32(serializer, mla_string_const("max_entries_per_key"), self->max_entries_per_key);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_cli_history_store_t* self = mla_pointer_get_data<mla_cli_history_store_t>(obj);
        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }
        if (mla_string_equals_const(property_name, "entries")) {
            mla_deserializer_read_list_struct(deserializer, self->entries, mla_cli_history_entry_t);
        } else if (mla_string_equals_const(property_name, "max_entries_per_key")) {
            mla_deserializer_read_uint32(deserializer, self->max_entries_per_key);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }
};

/**
 * @brief Constructs an empty CLI history store.
 * @param max_per_key Maximum history entries saved per key.
 * @return Initialized history store structure.
 */
mla_cli_history_store_t mla_cli_history_store_init(mla_size_t max_per_key = 10);

/**
 * @brief Records a parameter value for a key in the history store.
 * @param store History store to record into.
 * @param key Identification key.
 * @param value Parameter value string to record.
 */
void mla_cli_history_record_value(mla_cli_history_store_t &store, const mla_string_t &key, const mla_string_t &value);

/**
 * @brief Retrieves history candidates matching a prefix.
 * @param store History store.
 * @param key Identification key.
 * @param prefix Prefix to filter against.
 * @return Array list of matching candidate value strings.
 */
mla_array_list_t<mla_init_struct(mla_string_t)> mla_cli_history_get_candidates(const mla_cli_history_store_t &store,
                                                                                      const mla_string_t &key,
                                                                                      const mla_string_t &prefix);

/**
 * @brief Saves history store to a VFS file path using framework serialization.
 * @param store History store.
 * @param vfs_file_path VFS path (e.g. "/.cli_history.json").
 * @return true on success, false on failure.
 */
mla_bool_t mla_cli_history_save_to_file(const mla_cli_history_store_t &store, const mla_string_t &vfs_file_path);

/**
 * @brief Loads history store from a VFS file path using framework deserialization.
 * @param store History store to populate.
 * @param vfs_file_path VFS path.
 * @param max_per_key Maximum history entries saved per key.
 * @return true on success, false on failure.
 */
mla_bool_t mla_cli_history_load_from_file(mla_cli_history_store_t &store, const mla_string_t &vfs_file_path, mla_size_t max_per_key = 10);

#endif // MLA_CLI_HISTORY_H
