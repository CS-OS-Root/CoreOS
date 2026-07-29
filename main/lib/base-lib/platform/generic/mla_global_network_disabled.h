// src/lib/base-lib/platform/generic/mla_global_network_disabled.h
//
// Created by chris on 1/30/2026.
//
// This header provides stubbed (disabled) implementations of the low-level
// network operations used by the platform abstraction. These implementations
// intentionally perform no network activity and always indicate failure.
// They are useful for builds or environments where networking is not available
// or must be explicitly disabled.

#ifndef MLA_GLOBAL_NETWORK_DISABLED_H
#define MLA_GLOBAL_NETWORK_DISABLED_H

#include "../../core/network/mla_network.h"

/**
 * Disabled resolver implementation.
 *
 * This function is a stub that does not perform any DNS/host resolution.
 * It intentionally ignores the provided host/hostname/port and returns false
 * to indicate resolution failure.
 *
 * @param host   Output host structure (ignored).
 * @param hostname  Hostname to resolve (ignored).
 * @param port   Port number (ignored).
 * @return false always, indicating resolution failed / not supported.
 */
mla_bool_t mla_private_disabled_resolve_host(mla_network_host_t &host, const mla_string_t &hostname, mla_uint16_t port) {
    (void) host;
    (void) hostname;
    (void) port;
    return false;
}

/**
 * Disabled connect implementation.
 *
 * This function is a stub that does not establish any network connections.
 * It ignores all parameters and returns false to indicate that a connection
 * could not be established.
 *
 * @param connection  Connection object to initialize (ignored).
 * @param host        Remote host to connect to (ignored).
 * @param type        Connection type/protocol (ignored).
 * @param timeout_ms  Connection timeout in milliseconds (ignored).
 * @return false always, indicating connect failed / not supported.
 */
mla_bool_t mla_private_disabled_connect(mla_network_connection_t &connection, const mla_network_host_t &host,
                           mla_connection_type_t type, mla_size_t timeout_ms) {
    (void) connection;
    (void) host;
    (void) type;
    (void) timeout_ms;
    return false;
}

mla_bool_t mla_private_disabled_connect_secure(
        mla_network_connection_t &connection,
        const mla_network_host_t &host,
        mla_connection_type_t type,
        mla_size_t timeout_ms,
        const mla_network_security_config_t &security_config) {
    if (mla_network_security_config_get_mode(security_config) == mla_network_security_mode_insecure) {
        return mla_private_disabled_connect(connection, host, type, timeout_ms);
    }

    return false;
}

/**
 * Disabled bind-and-listen implementation.
 *
 * This stub does not bind or listen on any sockets. It ignores the listener
 * and host parameters and returns false to indicate that listening is not
 * available in this configuration.
 *
 * @param listener  Listener object to initialize (ignored).
 * @param host      Local host/address to bind to (ignored).
 * @param type      Connection type/protocol (ignored).
 * @return false always, indicating bind/listen failed / not supported.
 */
mla_bool_t mla_private_disabled_bind_and_listen(mla_network_listener_t &listener, const mla_network_host_t &host, mla_connection_type_t type) {
    (void) listener;
    (void) host;
    (void) type;
    return false;
}

mla_bool_t mla_private_disabled_bind_and_listen_secure(
        mla_network_listener_t &listener,
        const mla_network_host_t &host,
        mla_connection_type_t type,
        const mla_network_security_config_t &security_config) {
    if (mla_network_security_config_get_mode(security_config) == mla_network_security_mode_insecure) {
        return mla_private_disabled_bind_and_listen(listener, host, type);
    }

    return false;
}

/**
 * Disabled local IP address retrieval.
 *
 * This function is a stub that does not query any network interfaces or
 * return any IP addresses. It always returns an empty list, indicating that
 * no local IP addresses are available.
 *
 * @return An empty array list of local IP addresses.
 */
mla_array_list_t<mla_init_struct(mla_network_ip_address_t)> mla_private_disabled_get_local_ip_addresses() {
    return mla_array_list_empty<mla_init_struct(mla_network_ip_address_t)>();
}

/**
 * Global table of low-level network operations.
 *
 * When networking is globally disabled this table is populated with the
 * stub implementations above. Consumers of the low-level network API should
 * call through this table; substituting a different table enables actual
 * network functionality.
 */
mla_network_low_level_operations_t g_network_low_level_operations = {
    mla_private_disabled_resolve_host,
    mla_private_disabled_connect,
    mla_private_disabled_connect_secure,
    mla_private_disabled_bind_and_listen,
    mla_private_disabled_bind_and_listen_secure,
    mla_private_disabled_get_local_ip_addresses
};

#endif