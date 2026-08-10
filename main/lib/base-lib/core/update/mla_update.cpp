//
// Created for auto-update package.
//

#include "mla_update.h"
#include "../http/mla_http_client.h"
#include "../http/mla_http_request.h"
#include "../http/mla_http_response.h"
#include "../system/mla_string_concat.h"

mla_user_data_id_init(mla_update_provider_url_id)

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

mla_bool_t mla_update_check_and_apply_pending_update(int argc, char** argv) {
    if (g_update_management.check_and_apply_pending_update == nullptr) {
        return false;
    }
    return g_update_management.check_and_apply_pending_update(argc, argv);
}

mla_bool_t mla_private_update_http_get_last_version(const mla_update_provider_t& p_Provider, mla_string_t& p_OutVersion) {
    mla_string_t base_url = mla_user_data_get_string(p_Provider.user_data, mla_update_provider_url_id);
    if (mla_string_length(base_url) == 0) {
        return false;
    }

    mla_string_t url_string = mla_string_concat(base_url, mla_string_const("/version"));
    mla_http_request_t request = mla_http_get_request(url_string);
    mla_http_client_t client = mla_http_client();
    mla_http_client_set_timeout(client, 5000);
    mla_http_client_response_t client_res = mla_http_client_send_request(client, request);
    if (client_res.status != MLA_HTTP_CLIENT_RESPONSE_STATUS_OK || client_res.response.statusCode != 200) {
        return false;
    }

    mla_stream_input_t content_stream = client_res.response.content;
    p_OutVersion = mla_string_from_stream(content_stream, 10000);
    return true;
}

mla_bool_t mla_private_update_http_get_binary_content(const mla_update_provider_t& p_Provider, const mla_string_t& p_Version, mla_stream_input_t& p_OutStream) {
    (void)p_Version;
    mla_string_t base_url = mla_user_data_get_string(p_Provider.user_data, mla_update_provider_url_id);
    if (mla_string_length(base_url) == 0) {
        return false;
    }

    mla_string_t url_string = mla_string_concat(base_url, mla_string_const("/binary"));
    mla_http_request_t request = mla_http_get_request(url_string);
    mla_http_client_t client = mla_http_client();
    mla_http_client_set_timeout(client, 5000);
    mla_http_client_response_t client_res = mla_http_client_send_request(client, request);
    if (client_res.status != MLA_HTTP_CLIENT_RESPONSE_STATUS_OK || client_res.response.statusCode != 200) {
        return false;
    }

    p_OutStream = client_res.response.content;
    return true;
}

mla_update_provider_t mla_update_provider_http_create(const mla_string_t& p_BaseUrl) {
    mla_update_provider_t provider = { mla_user_data_empty(), mla_private_update_http_get_last_version, mla_private_update_http_get_binary_content };
    mla_string_t url_copy = p_BaseUrl;
    mla_user_data_set_string(provider.user_data, mla_update_provider_url_id, url_copy);
    return provider;
}
