//
// Created by christian on 10/6/25.
//

#ifndef MLA_NETWORK_H
#define MLA_NETWORK_H

#include "../mla_data_types.h"
#include "../system/mla_string.h"
#include "../system/mla_stream.h"

enum mla_connection_type_t: mla_uint8_t {
    mla_connection_type_tcp,
    mla_connection_type_udp
};

enum mla_network_security_mode_t: mla_uint8_t {
    mla_network_security_mode_insecure,
    mla_network_security_mode_tls
};

struct mla_network_tls_config_t {
    mla_string_t certificate;
    mla_string_t private_key;
    mla_string_t ca_certificate;
    mla_string_t server_name;
    mla_bool_t verify_peer;
    mla_bool_t verify_host_name;
};

struct mla_network_security_config_t {
    mla_network_security_mode_t mode;
    mla_network_tls_config_t tls;
};

mla_network_tls_config_t mla_network_tls_config_default();
void mla_network_tls_config_set_certificate(mla_network_tls_config_t &tls_config, const mla_string_t &certificate);
mla_string_t mla_network_tls_config_get_certificate(const mla_network_tls_config_t &tls_config);
void mla_network_tls_config_set_private_key(mla_network_tls_config_t &tls_config, const mla_string_t &private_key);
mla_string_t mla_network_tls_config_get_private_key(const mla_network_tls_config_t &tls_config);
void mla_network_tls_config_set_ca_certificate(mla_network_tls_config_t &tls_config, const mla_string_t &ca_certificate);
mla_string_t mla_network_tls_config_get_ca_certificate(const mla_network_tls_config_t &tls_config);
void mla_network_tls_config_set_server_name(mla_network_tls_config_t &tls_config, const mla_string_t &server_name);
mla_string_t mla_network_tls_config_get_server_name(const mla_network_tls_config_t &tls_config);
void mla_network_tls_config_set_verify_peer(mla_network_tls_config_t &tls_config, mla_bool_t verify_peer);
mla_bool_t mla_network_tls_config_get_verify_peer(const mla_network_tls_config_t &tls_config);
void mla_network_tls_config_set_verify_host_name(mla_network_tls_config_t &tls_config, mla_bool_t verify_host_name);
mla_bool_t mla_network_tls_config_get_verify_host_name(const mla_network_tls_config_t &tls_config);

mla_network_security_config_t mla_network_security_config_none();
mla_network_security_config_t mla_network_security_config_tls(const mla_network_tls_config_t &tls_config);
void mla_network_security_config_set_mode(mla_network_security_config_t &security_config, mla_network_security_mode_t mode);
mla_network_security_mode_t mla_network_security_config_get_mode(const mla_network_security_config_t &security_config);
void mla_network_security_config_set_tls_config(mla_network_security_config_t &security_config, const mla_network_tls_config_t &tls_config);
mla_network_tls_config_t mla_network_security_config_get_tls_config(const mla_network_security_config_t &security_config);


struct mla_network_ip_address_t {
    mla_string_t address; // IP address in string format
    mla_bool_t is_ipv6;   // True if IPv6, false if IPv4

    static mla_network_ip_address_t init() {
        return { mla_string_empty(), false };
    }
};

mla_network_ip_address_t mla_network_ip_address_invalid();
mla_network_ip_address_t mla_network_ip_address_ip4(const mla_string_t &address);
mla_network_ip_address_t mla_network_ip_address_ip6(const mla_string_t &address);

struct mla_network_host_t {
    mla_network_ip_address_t address;
    mla_uint16_t port;
};

mla_network_host_t mla_network_host_ip4(const mla_string_t &address, mla_uint16_t port);
mla_network_host_t mla_network_host_ip6(const mla_string_t &address, mla_uint16_t port);
mla_network_host_t mla_network_host_invalid();

//////////////////////////////////////////////////////////////////
//// Lookup Operations
//////////////////////////////////////////////////////////////////

mla_array_list_t<mla_init_struct(mla_network_ip_address_t)> mla_network_get_local_ip_addresses();
mla_bool_t mla_network_host_resolve(mla_network_host_t &host, const mla_string_t &hostname, mla_uint16_t port);

//////////////////////////////////////////////////////////////////
/// Client Connection
//////////////////////////////////////////////////////////////////

struct mla_network_connection_t {
    mla_network_host_t host;
    mla_stream_input_t inputStream;
    mla_stream_output_t outputStream;

    static mla_network_connection_t init() {
        return { { { mla_string_empty(), false }, 0 }, mla_stream_input_t::init(), mla_stream_output_t::init() };
    }
};

mla_network_connection_t mla_network_connection_disconnected();

mla_bool_t mla_network_connection_connect(mla_network_connection_t &connection, const mla_network_host_t &host, mla_connection_type_t type, mla_size_t timeout_ms);
mla_bool_t mla_network_connection_connect_secure(
    mla_network_connection_t &connection,
    const mla_network_host_t &host,
    mla_connection_type_t type,
    mla_size_t timeout_ms,
    const mla_network_security_config_t &security_config);
mla_bool_t mla_network_connection_disconnect(mla_network_connection_t &connection);
mla_bool_t mla_network_connection_is_connected(const mla_network_connection_t &connection);

//////////////////////////////////////////////////////////////////
/// Server Operations
//////////////////////////////////////////////////////////////////

struct mla_network_listener_t {
    mla_network_host_t host;
    mla_bool_t (*accept_connection)(const mla_network_listener_t& listener, mla_network_connection_t &connection);
    mla_user_data_t userdata;
};

mla_network_listener_t mla_network_listener_invalid();

mla_bool_t mla_network_listener_bind_and_listen(mla_network_listener_t& listener, const mla_network_host_t &host, mla_connection_type_t type);
mla_bool_t mla_network_listener_bind_and_listen_secure(
    mla_network_listener_t& listener,
    const mla_network_host_t &host,
    mla_connection_type_t type,
    const mla_network_security_config_t &security_config);
mla_bool_t mla_network_listener_close(mla_network_listener_t& listener);
mla_bool_t mla_network_listener_accept_connection(const mla_network_listener_t& listener, mla_network_connection_t &connection);

/////////////////////////////////////////////////////////////////
/// Low Level Operations
/////////////////////////////////////////////////////////////////

struct mla_network_low_level_operations_t {
    mla_bool_t (*resolve_host)(mla_network_host_t &host, const mla_string_t &hostname, mla_uint16_t port);
    mla_bool_t (*connect)(mla_network_connection_t &connection, const mla_network_host_t &host, mla_connection_type_t type, mla_size_t timeout_ms);
    mla_bool_t (*connect_secure)(
        mla_network_connection_t &connection,
        const mla_network_host_t &host,
        mla_connection_type_t type,
        mla_size_t timeout_ms,
        const mla_network_security_config_t &security_config);
    mla_bool_t (*bind_and_listen)(mla_network_listener_t &listener, const mla_network_host_t &host, mla_connection_type_t type);
    mla_bool_t (*bind_and_listen_secure)(
        mla_network_listener_t &listener,
        const mla_network_host_t &host,
        mla_connection_type_t type,
        const mla_network_security_config_t &security_config);
    mla_array_list_t<mla_init_struct(mla_network_ip_address_t)> (*get_local_ip_addresses)();
};



#endif
