//
// Created by christian on 12/5/25.
//

#ifndef MLA_LOGGER_RPC_H
#define MLA_LOGGER_RPC_H

#include "mla_logger.h"
#include "../rpc/mla_rpc.h"
#include "../serializer/mla_serializer.h"
#include "../lifecycle/mla_lifecycle_events.h"

static mla_string_t RPC_CACHE_LOGGER_NAME = mla_string_const("RpcConsole");

struct mla_logger_rpc_log_level_t {
    mla_log_level level;

    static mla_bool_t serialize(mla_serializer_t &serializer, const mla_pointer_t &obj) {
        const mla_logger_rpc_log_level_t *logLevel = mla_pointer_get_data<const mla_logger_rpc_log_level_t>(obj);

        if (logLevel == nullptr) {
            return false;
        }

        mla_serializer_write_enum(serializer, mla_string_const("level"), logLevel->level);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t &deserializer, mla_pointer_t &obj, const mla_string_t &property_name) {
        mla_logger_rpc_log_level_t *logLevel = mla_pointer_get_data<mla_logger_rpc_log_level_t>(obj);

        if (logLevel == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "level")) {
            mla_deserializer_read_enum(mla_log_level, deserializer, logLevel->level)
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }

    static mla_reflection_struct_metadata_t metadata() {
        mla_reflection_struct_metadata_t meta = mla_reflection_struct_metadata(mla_logger_rpc_log_level_t);
        mla_reflection_struct_field_enum(meta, mla_logger_rpc_log_level_t, level);
        mla_reflection_struct_metadata_freeze(meta);
        return meta;
    }
};

struct mla_logger_rpc_log_entry_t {
    mla_uint32_t logid;
    mla_log_level level;
    mla_string_t message;
    mla_string_t context1;

    static mla_logger_rpc_log_entry_t init() {
        return { 0, MLA_LOG_LEVEL_INFO, mla_string_empty(), mla_string_empty() };
    }

    static mla_bool_t serialize(mla_serializer_t &serializer, const mla_pointer_t &obj) {
        const mla_logger_rpc_log_entry_t *logEntry = mla_pointer_get_data<const mla_logger_rpc_log_entry_t>(obj);

        if (logEntry == nullptr) {
            return false;
        }

        mla_serializer_write_uint32(serializer, mla_string_const("logid"), logEntry->logid);
        mla_serializer_write_uint8(serializer, mla_string_const("level"), logEntry->level);
        mla_serializer_write_string(serializer, mla_string_const("message"), logEntry->message);
        mla_serializer_write_string(serializer, mla_string_const("context1"), logEntry->context1);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t &deserializer, mla_pointer_t &obj,
                                                      const mla_string_t &property_name) {
        mla_logger_rpc_log_entry_t *logEntry = mla_pointer_get_data<mla_logger_rpc_log_entry_t>(obj);

        if (logEntry == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "logid")) {
            mla_deserializer_read_uint32(deserializer, logEntry->logid)
        } else if (mla_string_equals_const(property_name, "level")) {
            mla_deserializer_read_enum(mla_log_level, deserializer, logEntry->level)
        } else if (mla_string_equals_const(property_name, "message")) {
            mla_deserializer_read_string(deserializer, logEntry->message)
        } else if (mla_string_equals_const(property_name, "context1")) {
            mla_deserializer_read_string(deserializer, logEntry->context1)
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }

    static mla_reflection_struct_metadata_t metadata() {
        mla_reflection_struct_metadata_t meta = mla_reflection_struct_metadata(mla_logger_rpc_log_level_t);
        mla_reflection_struct_field_uint32(meta, mla_logger_rpc_log_entry_t, logid);
        mla_reflection_struct_field_enum(meta, mla_logger_rpc_log_entry_t, level);
        mla_reflection_struct_field_string(meta, mla_logger_rpc_log_entry_t, message);
        mla_reflection_struct_field_string(meta, mla_logger_rpc_log_entry_t, context1);
        mla_reflection_struct_metadata_freeze(meta);
        return meta;
    }
};


mla_logger_rpc_log_entry_t mla_logger_rpc_log_entry_empty();


struct mla_logger_rpc_log_messages_t {
    mla_array_list_t<mla_init_struct(mla_logger_rpc_log_entry_t)> entries;

    static mla_bool_t serialize(mla_serializer_t &serializer, const mla_pointer_t &obj) {
        const mla_logger_rpc_log_messages_t *logMessages = mla_pointer_get_data<const
            mla_logger_rpc_log_messages_t>(obj);

        if (logMessages == nullptr) {
            return false;
        }

        mla_serializer_write_list<mla_logger_rpc_log_entry_t>(serializer, mla_string_const("entries"),
                                                              logMessages->entries,
                                                              mla_logger_rpc_log_entry_t::serialize);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t &deserializer, mla_pointer_t &obj,
                                                      const mla_string_t &property_name) {
        mla_logger_rpc_log_messages_t *logMessages = mla_pointer_get_data<mla_logger_rpc_log_messages_t>(obj);

        if (logMessages == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "entries")) {
            mla_deserializer_read_list_struct(deserializer, logMessages->entries, mla_logger_rpc_log_entry_t)
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }

    static mla_reflection_struct_metadata_t metadata() {
        mla_reflection_struct_metadata_t meta = mla_reflection_struct_metadata(mla_logger_rpc_log_messages_t);
        mla_reflection_struct_field_struct_list(meta, mla_logger_rpc_log_messages_t, entries,
                                                mla_logger_rpc_log_entry_t);
        mla_reflection_struct_metadata_freeze(meta);
        return meta;
    }
};

mla_bool_t mla_logger_rpc_activate();

mla_bool_t mla_logger_rpc_deactivate();

mla_bool_t mla_logger_rpc_active();


#define mla_rpc_procedure_set_loglevel_name "log/setLoglevel"
#define mla_rpc_procedure_set_loglevel_signature mla_logger_rpc_log_level_t, mla_void_t
mla_bool_t mla_logger_rpc_set_loglevel_handler(const mla_logger_rpc_log_level_t *input,
                                               mla_rpc_procedure_void_t output);

#define mla_rpc_procedure_get_loglevel_name "log/getLoglevel"
#define mla_rpc_procedure_get_loglevel_signature void, mla_logger_rpc_log_level_t
mla_bool_t mla_logger_rpc_get_loglevel_handler(mla_rpc_procedure_const_void_t input,
                                               mla_logger_rpc_log_level_t *output);

#define mla_rpc_procedure_log_message_name "log/getMessages"
#define mla_rpc_procedure_log_message_signature void, mla_logger_rpc_log_messages_t
mla_bool_t mla_logger_rpc_log_get_messages_handler(mla_rpc_procedure_const_void_t input,
                                                   mla_logger_rpc_log_messages_t *output);


#endif
