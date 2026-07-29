//
// Created by chris on 12/29/2025.
//

#ifndef MLA_GLOBAL_NETWORK_RASPBERRY_PICO_H
#define MLA_GLOBAL_NETWORK_RASPBERRY_PICO_H

#include "../../core/network/mla_network.h"

// Raspberry Pi Pico minimal network implementation
// This is a stub implementation as basic Pico doesn't have native networking
// For actual network operations, users would need to integrate with WiFi/Ethernet modules
// like the Pico W (with WiFi) or external Ethernet shields

mla_bool_t mla_private_pico_resolve_host(mla_network_host_t& host, const mla_string_t& hostname, mla_uint16_t port) {
    (void)host;
    (void)hostname;
    (void)port;
    // Stub implementation - cannot resolve hosts without network hardware
    return false;
}

mla_bool_t mla_private_pico_connect(mla_network_connection_t& connection, const mla_network_host_t& host,
                          mla_connection_type_t type, mla_size_t timeout_ms) {
    (void)connection;
    (void)host;
    (void)type;
    (void)timeout_ms;
    // Stub implementation - cannot connect without network hardware
    return false;
}

mla_bool_t mla_private_pico_connect_secure(
        mla_network_connection_t &connection,
        const mla_network_host_t &host,
        mla_connection_type_t type,
        mla_size_t timeout_ms,
        const mla_network_security_config_t &security_config) {
    if (mla_network_security_config_get_mode(security_config) == mla_network_security_mode_insecure) {
        return mla_private_pico_connect(connection, host, type, timeout_ms);
    }

    return false;
}

mla_bool_t mla_private_pico_bind_and_listen(mla_network_listener_t& listener, const mla_network_host_t& host, mla_connection_type_t type) {
    (void)listener;
    (void)host;
    (void)type;
    // Stub implementation - cannot bind/listen without network hardware
    return false;
}

mla_bool_t mla_private_pico_bind_and_listen_secure(
        mla_network_listener_t &listener,
        const mla_network_host_t &host,
        mla_connection_type_t type,
        const mla_network_security_config_t &security_config) {
    if (mla_network_security_config_get_mode(security_config) == mla_network_security_mode_insecure) {
        return mla_private_pico_bind_and_listen(listener, host, type);
    }

    return false;
}

mla_array_list_t<mla_init_struct(mla_network_ip_address_t)> mla_private_pico_get_local_ip_addresses() {
    return mla_array_list_empty<mla_init_struct(mla_network_ip_address_t)>();
}

mla_network_low_level_operations_t g_network_low_level_operations = {
    mla_private_pico_resolve_host,
    mla_private_pico_connect,
    mla_private_pico_connect_secure,
    mla_private_pico_bind_and_listen,
    mla_private_pico_bind_and_listen_secure,
    mla_private_pico_get_local_ip_addresses
};

#endif
