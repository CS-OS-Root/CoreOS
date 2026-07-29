//
// Created by chris on 10/10/2025.
//

#include "mla_network.h"

mla_global mla_network_low_level_operations_t g_network_low_level_operations;

mla_network_tls_config_t mla_network_tls_config_default() {
    return {
        mla_string_empty(),
        mla_string_empty(),
        mla_string_empty(),
        mla_string_empty(),
        true,
        true
    };
}

void mla_network_tls_config_set_certificate(mla_network_tls_config_t &tls_config, const mla_string_t &certificate) {
    tls_config.certificate = certificate;
}

mla_string_t mla_network_tls_config_get_certificate(const mla_network_tls_config_t &tls_config) {
    return tls_config.certificate;
}

void mla_network_tls_config_set_private_key(mla_network_tls_config_t &tls_config, const mla_string_t &private_key) {
    tls_config.private_key = private_key;
}

mla_string_t mla_network_tls_config_get_private_key(const mla_network_tls_config_t &tls_config) {
    return tls_config.private_key;
}

void mla_network_tls_config_set_ca_certificate(mla_network_tls_config_t &tls_config, const mla_string_t &ca_certificate) {
    tls_config.ca_certificate = ca_certificate;
}

mla_string_t mla_network_tls_config_get_ca_certificate(const mla_network_tls_config_t &tls_config) {
    return tls_config.ca_certificate;
}

void mla_network_tls_config_set_server_name(mla_network_tls_config_t &tls_config, const mla_string_t &server_name) {
    tls_config.server_name = server_name;
}

mla_string_t mla_network_tls_config_get_server_name(const mla_network_tls_config_t &tls_config) {
    return tls_config.server_name;
}

void mla_network_tls_config_set_verify_peer(mla_network_tls_config_t &tls_config, mla_bool_t verify_peer) {
    tls_config.verify_peer = verify_peer;
}

mla_bool_t mla_network_tls_config_get_verify_peer(const mla_network_tls_config_t &tls_config) {
    return tls_config.verify_peer;
}

void mla_network_tls_config_set_verify_host_name(mla_network_tls_config_t &tls_config, mla_bool_t verify_host_name) {
    tls_config.verify_host_name = verify_host_name;
}

mla_bool_t mla_network_tls_config_get_verify_host_name(const mla_network_tls_config_t &tls_config) {
    return tls_config.verify_host_name;
}

mla_network_security_config_t mla_network_security_config_none() {
    return {
        mla_network_security_mode_insecure,
        mla_network_tls_config_default()
    };
}

mla_network_security_config_t mla_network_security_config_tls(const mla_network_tls_config_t &tls_config) {
    return {
        mla_network_security_mode_tls,
        tls_config
    };
}

void mla_network_security_config_set_mode(mla_network_security_config_t &security_config, mla_network_security_mode_t mode) {
    security_config.mode = mode;
}

mla_network_security_mode_t mla_network_security_config_get_mode(const mla_network_security_config_t &security_config) {
    return security_config.mode;
}

void mla_network_security_config_set_tls_config(mla_network_security_config_t &security_config, const mla_network_tls_config_t &tls_config) {
    security_config.tls = tls_config;
}

mla_network_tls_config_t mla_network_security_config_get_tls_config(const mla_network_security_config_t &security_config) {
    return security_config.tls;
}

mla_network_ip_address_t mla_network_ip_address_invalid() {
    return {
        mla_string_empty(),
        false
    };
}

mla_network_ip_address_t mla_network_ip_address_ip4(const mla_string_t &address) {
    return {
                address,
                false
            };
}

mla_network_ip_address_t mla_network_ip_address_ip6(const mla_string_t &address) {
    return {
            address,
            true
        };
}

mla_network_host_t mla_network_host_ip4(const mla_string_t &address, mla_uint16_t port) {

    return {
            mla_network_ip_address_ip4(address), port
            };
}

mla_network_host_t mla_network_host_ip6(const mla_string_t &address, mla_uint16_t port) {

    return {
            mla_network_ip_address_ip6(address), port
        };
}

mla_network_host_t mla_network_host_invalid() {

    return {
        {
            mla_string_empty(),
        false,
        }, 0
    };
}

mla_network_connection_t mla_network_connection_disconnected() {

    return {
        mla_network_host_invalid(),
        mla_stream_noop_input(),
        mla_stream_noop_output()
    };
}


mla_array_list_t<mla_init_struct(mla_network_ip_address_t)> mla_network_get_local_ip_addresses() {

    if (g_network_low_level_operations.get_local_ip_addresses == nullptr) {
        return mla_array_list_empty<mla_init_struct(mla_network_ip_address_t)>();
    }

    return g_network_low_level_operations.get_local_ip_addresses();
}


mla_bool_t mla_network_host_resolve(mla_network_host_t &host, const mla_string_t &hostname, mla_uint16_t port) {

    if (mla_string_length(hostname) == 0) {
        host = mla_network_host_invalid();
        return false;
    }

    return g_network_low_level_operations.resolve_host(host, hostname, port);

}

mla_bool_t mla_network_connection_connect(mla_network_connection_t &connection, const mla_network_host_t &host, mla_connection_type_t type, mla_size_t timeout_ms) {

    return mla_network_connection_connect_secure(connection, host, type, timeout_ms, mla_network_security_config_none());
}

mla_bool_t mla_network_connection_connect_secure(
        mla_network_connection_t &connection,
        const mla_network_host_t &host,
        mla_connection_type_t type,
        mla_size_t timeout_ms,
        const mla_network_security_config_t &security_config) {

    if (host.port == 0 || mla_string_length(host.address.address) == 0) {
        return false;
    }

    if (mla_network_security_config_get_mode(security_config) == mla_network_security_mode_tls && type != mla_connection_type_tcp) {
        return false;
    }

    if (g_network_low_level_operations.connect_secure != nullptr) {
        return g_network_low_level_operations.connect_secure(connection, host, type, timeout_ms, security_config);
    }

    if (mla_network_security_config_get_mode(security_config) != mla_network_security_mode_insecure || g_network_low_level_operations.connect == nullptr) {
        return false;
    }

    return g_network_low_level_operations.connect(connection, host, type, timeout_ms);
}

mla_bool_t mla_network_connection_disconnect(mla_network_connection_t &connection) {

    // By assigning noop streams the connection gets effectively disconnected
    connection.inputStream = mla_stream_noop_input();
    connection.outputStream = mla_stream_noop_output();
    connection.host = mla_network_host_invalid();
    return true;
}

mla_bool_t mla_network_connection_is_connected(const mla_network_connection_t &connection) {
    return connection.host.port != 0 && !mla_string_is_empty(connection.host.address.address);
}

mla_network_listener_t mla_network_listener_invalid() {

    return {
        mla_network_host_invalid(),
        nullptr,
        mla_user_data_empty()
    };
}

mla_bool_t mla_network_listener_bind_and_listen(mla_network_listener_t& listener, const mla_network_host_t &host, mla_connection_type_t type) {

    return mla_network_listener_bind_and_listen_secure(listener, host, type, mla_network_security_config_none());
}

mla_bool_t mla_network_listener_bind_and_listen_secure(
        mla_network_listener_t& listener,
        const mla_network_host_t &host,
        mla_connection_type_t type,
        const mla_network_security_config_t &security_config) {

    if (host.port == 0 || mla_string_length(host.address.address) == 0) {
        return false;
    }

    if (mla_network_security_config_get_mode(security_config) == mla_network_security_mode_tls && type != mla_connection_type_tcp) {
        return false;
    }

    if (g_network_low_level_operations.bind_and_listen_secure != nullptr) {
        return g_network_low_level_operations.bind_and_listen_secure(listener, host, type, security_config);
    }

    if (mla_network_security_config_get_mode(security_config) != mla_network_security_mode_insecure || g_network_low_level_operations.bind_and_listen == nullptr) {
        return false;
    }

    return g_network_low_level_operations.bind_and_listen(listener, host , type);

}

mla_bool_t mla_network_listener_close(mla_network_listener_t& listener) {
    listener = mla_network_listener_invalid();
    return true;
}

mla_bool_t mla_network_listener_accept_connection(const mla_network_listener_t& listener, mla_network_connection_t &connection) {

    if (listener.accept_connection == nullptr) {
        return false;
    }

    return listener.accept_connection(listener, connection);
}