#include "mla_logger_rpc.h"
#include "../task/mla_mutx.h"

#ifndef mla_rpc_logger_level
#define mla_rpc_logger_level MLA_LOG_LEVEL_INFO
#endif

struct mla_rpc_cache_logger {
    mla_logger_rpc_log_entry_t* entries;
    mla_size_t entry_count;
    mla_uint32_t last_log_id;
    mla_size_t current_index;
    mla_mutex_t lock;
};

static mla_rpc_cache_logger g_rpc_log_cache =  {
    nullptr,
    0,
    0,
    0,
    mla_mutex_create("RpcCache")
};

void mla_private_log_rpc_writer(const mla_log_level level, mla_string_t &message, mla_string_t &context1,
                              mla_user_data_t& userData) {

    (void)userData;

    if (!mla_mutex_trylock(g_rpc_log_cache.lock, 250)) {
        return;
    }

    g_rpc_log_cache.current_index++;
    if (g_rpc_log_cache.current_index >= mla_global_config_rpc_log_cache_size) {
        g_rpc_log_cache.current_index = 0;
    }

    g_rpc_log_cache.last_log_id++;
    g_rpc_log_cache.entries[g_rpc_log_cache.current_index] = {
        g_rpc_log_cache.last_log_id,
        level,
        message,
        context1
    };
    if (g_rpc_log_cache.entry_count < mla_global_config_rpc_log_cache_size) {
        g_rpc_log_cache.entry_count++;
    }


    mla_mutex_unlock(g_rpc_log_cache.lock);

}


mla_bool_t mla_logger_rpc_activate() {

    if (!mla_mutex_lock(g_rpc_log_cache.lock)) {
        return false;
    }

    if (g_rpc_log_cache.entries != nullptr) {
        mla_mutex_unlock(g_rpc_log_cache.lock);
        return false;
    }

    g_rpc_log_cache.entries = mla_r_cast<mla_logger_rpc_log_entry_t*>(mla_platform_malloc(sizeof(mla_logger_rpc_log_entry_t) * mla_global_config_rpc_log_cache_size));

    if (g_rpc_log_cache.entries == nullptr) {
        mla_mutex_unlock(g_rpc_log_cache.lock);
        return false;
    }

    mla_memset(g_rpc_log_cache.entries, 0, sizeof(mla_logger_rpc_log_entry_t) * mla_global_config_rpc_log_cache_size);
    g_rpc_log_cache.last_log_id = 0;
    g_rpc_log_cache.current_index = 0;
    g_rpc_log_cache.entry_count = 0;

    mla_mutex_unlock(g_rpc_log_cache.lock);

    const mla_logger_t logger = {
        RPC_CACHE_LOGGER_NAME,
        mla_rpc_logger_level, // Default log level
        true, // Need full managed strings
        mla_private_log_rpc_writer, // Function to write log messages
        mla_user_data_empty()
    };

    return mla_log_register_logger(logger);

}

mla_bool_t mla_logger_rpc_deactivate() {

    if (!mla_mutex_lock(g_rpc_log_cache.lock)) {
        return false;
    }

    if (g_rpc_log_cache.entries != nullptr) {
        mla_platform_free(g_rpc_log_cache.entries);
        g_rpc_log_cache.entries = nullptr;
    }

    mla_mutex_unlock(g_rpc_log_cache.lock);

    return mla_log_unregister_logger(RPC_CACHE_LOGGER_NAME);
}

mla_bool_t mla_logger_rpc_active() {
    return mla_log_indexOf_logger(RPC_CACHE_LOGGER_NAME) >= 0;
}

mla_logger_rpc_log_entry_t mla_logger_rpc_log_entry_empty() {
    return {
        0,
        MLA_LOG_LEVEL_INFO,
        mla_string_empty(),
        mla_string_empty()
    };
}

mla_bool_t mla_logger_rpc_set_loglevel_handler(const mla_logger_rpc_log_level_t* input, mla_rpc_procedure_void_t output) {

    (void)output;
    return mla_log_set_logger_level(RPC_CACHE_LOGGER_NAME, input->level);
}


mla_bool_t mla_logger_rpc_get_loglevel_handler(mla_rpc_procedure_const_void_t input, mla_logger_rpc_log_level_t* output) {

    (void)input;
    output->level = mla_log_get_logger_level(RPC_CACHE_LOGGER_NAME);
    return true;
}


mla_bool_t mla_logger_rpc_log_get_messages_handler(mla_rpc_procedure_const_void_t input, mla_logger_rpc_log_messages_t* output) {

    (void)input;

    if (!mla_mutex_lock(g_rpc_log_cache.lock)) {
        return false;
    }

    output->entries = mla_array_list<mla_init_struct(mla_logger_rpc_log_entry_t)>(g_rpc_log_cache.entry_count);
    mla_size_t index = g_rpc_log_cache.current_index;

    for (mla_size_t i = 0; i < g_rpc_log_cache.entry_count; ++i) {
        mla_logger_rpc_log_entry_t& entry = g_rpc_log_cache.entries[index];
        mla_array_list_add(output->entries, entry);
        g_rpc_log_cache.entries[index] = mla_logger_rpc_log_entry_empty();
        if (index == 0) {
            index = mla_global_config_rpc_log_cache_size - 1;
        } else {
            index--;
        }
    }
    g_rpc_log_cache.entry_count = 0;
    g_rpc_log_cache.current_index = 0;

    mla_mutex_unlock(g_rpc_log_cache.lock);

    mla_array_list_reverse(output->entries);

    return true;
}


mla_rpc_auto_register_procedure_void_output(mla_rpc_procedure_set_loglevel_name, mla_logger_rpc_log_level_t, mla_logger_rpc_set_loglevel_handler)
mla_rpc_auto_register_procedure_void_input(mla_rpc_procedure_get_loglevel_name, mla_logger_rpc_log_level_t, mla_logger_rpc_get_loglevel_handler)
mla_rpc_auto_register_procedure_void_input(mla_rpc_procedure_log_message_name, mla_logger_rpc_log_messages_t, mla_logger_rpc_log_get_messages_handler)

void mla_private_logger_rpc_init() {

    if (!mla_logger_rpc_activate()) {
        mla_error("Failed to activate RPC logger");
    }

}

mla_lifecycle_boot_event_static_register(mla_lifecycle_boot_event_priority_low_level_postSetup, mla_private_logger_rpc_init)
