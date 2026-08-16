//
// Created by christian on 10/6/25.
//

#ifndef MLA_HTTP_CLIENT_H
#define MLA_HTTP_CLIENT_H

#include "mla_http_request.h"
#include "mla_http_response.h"
#include "../network/mla_network.h"
#include "../url/mla_url.h"

enum mla_http_client_response_status: mla_uint8_t {
    MLA_HTTP_CLIENT_RESPONSE_STATUS_OK,
    MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_WRONG_PROTOCOL,
    MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_UNKNOWN_HOST,
    MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_CONNECTION_FAILED,
    MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_TIMEOUT,
    MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_INVALID_RESPONSE,
    MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_UNKNOWN
};

struct mla_http_client_response_t {
    mla_http_client_response_status status;
    mla_string_t errorMessage; // Present if status is not OK
    mla_http_response_t response;

};

struct mla_http_client_t {
    mla_bool_t support_deflate_compression;
    mla_bool_t support_gzip_compression;
    mla_int32_t timeout_ms;
    mla_bool_t (*resolve_host)(const mla_http_client_t &client, mla_http_client_response_t& response, const mla_url_t& url, mla_network_host_t & host);
    mla_bool_t (*connect)(const mla_http_client_t &client, mla_http_client_response_t& response, const mla_network_host_t & host, mla_network_connection_t & connection);
    mla_network_tls_config_t tls_config;
};

mla_http_client_t mla_http_client();

mla_bool_t mla_http_client_support_deflate_compression(const mla_http_client_t &client);
void mla_http_client_set_support_deflate_compression(mla_http_client_t &client, mla_bool_t support_deflate_compression);

mla_bool_t mla_http_client_support_gzip_compression(const mla_http_client_t &client);
void mla_http_client_set_support_gzip_compression(mla_http_client_t &client, mla_bool_t support_gzip_compression);

mla_int32_t mla_http_client_get_timeout(const mla_http_client_t &client);
void mla_http_client_set_timeout(mla_http_client_t &client, mla_int32_t timeout_ms);

mla_network_tls_config_t mla_http_client_get_tls_config(const mla_http_client_t &client);
void mla_http_client_set_tls_config(mla_http_client_t &client, const mla_network_tls_config_t &tls_config);
void mla_http_client_set_verify_peer(mla_http_client_t &client, mla_bool_t verify_peer);

mla_http_client_response_t mla_http_client_send_request(const mla_http_client_t &client, mla_http_request_t &p_Request);


////////////////////////////////////////////////////////////////
/// Helpers
////////////////////////////////////////////////////////////////

inline mla_http_client_response_t mla_http_client_send_request(mla_http_request_t &p_Request) {

    mla_http_client_t client = mla_http_client();
    return mla_http_client_send_request(client, p_Request);
}

inline mla_http_client_response_t mla_http_client_response_invalid() {
    return  {
        MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_UNKNOWN,
        mla_string_empty(),
        mla_http_response_empty()
    };
}


#endif
