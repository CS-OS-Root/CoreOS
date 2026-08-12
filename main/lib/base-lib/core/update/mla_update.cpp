//
// Created for auto-update package.
//

#include "mla_update.h"

mla_update_provider_t g_private_global_update_provider = {
    mla_user_data_empty(),
    nullptr,
    nullptr
};

mla_string_t mla_update_get_current_version() {
    return mla_string_const(MLA_APP_VERSION);
}

mla_bool_t mla_update_get_last_version(const mla_update_provider_t& p_Provider, mla_string_t& p_OutVersion) {
    if (p_Provider.get_last_version == nullptr) {
        return false;
    }
    return p_Provider.get_last_version(p_Provider, p_OutVersion);
}

mla_bool_t mla_update_get_last_version(mla_string_t& p_OutVersion) {
    return mla_update_get_last_version(g_private_global_update_provider, p_OutVersion);
}

void mla_update_set_provider(const mla_update_provider_t& p_Provider) {
    g_private_global_update_provider = p_Provider;
}

mla_update_provider_t mla_update_get_provider() {
    return g_private_global_update_provider;
}

mla_update_error_t mla_update_upgrade_to_version(mla_stream_input_t& p_BinaryStream) {
    if (g_update_management.upgrade_to_version == nullptr) {
        return MLA_UPDATE_ERROR_NOT_SUPPORTED;
    }
    return g_update_management.upgrade_to_version(p_BinaryStream);
}

