//
// Created by Jules on 10/24/2025.
//

#ifndef MLA_NETWORK_TEST_H
#define MLA_NETWORK_TEST_H

#include "../../lib/base-lib/core/network/mla_network.h"
#include "../../lib/base-lib/test-support/mla_test_executor.h"
#include "../../lib/base-lib/test-support/mla_test_utils.h"
#include "../../lib/base-lib/test-support/Test/mla_test.h"

static mla_bool_t mock_resolve_host_called = false;
static mla_bool_t mock_resolve_host_result = true;
static mla_network_host_t mock_resolved_host = mla_network_host_invalid();

static mla_bool_t mock_resolve_host(mla_network_host_t &host, const mla_string_t &hostname, mla_uint16_t port) {
    (void)hostname;
    (void)port;
    mock_resolve_host_called = true;
    host = mock_resolved_host;
    return mock_resolve_host_result;
}

static mla_bool_t mock_connect_called = false;
static mla_bool_t mock_connect_result = true;

static mla_bool_t mock_connect(mla_network_connection_t &connection, const mla_network_host_t &host, mla_connection_type_t type, mla_size_t timeout_ms) {
    (void)type;
    (void)timeout_ms;
    mock_connect_called = true;
    connection.host = host;
    connection.inputStream = mla_stream_noop_input();
    connection.outputStream = mla_stream_noop_output();
    return mock_connect_result;
}

static mla_bool_t mock_bind_and_listen_called = false;
static mla_bool_t mock_bind_and_listen_result = true;

static mla_bool_t mock_bind_and_listen(mla_network_listener_t &listener, const mla_network_host_t &host, mla_connection_type_t type) {
    (void)type;
    mock_bind_and_listen_called = true;
    listener.host = host;
    listener.accept_connection = nullptr;
    listener.userdata = mla_user_data_empty();
    return mock_bind_and_listen_result;
}

static mla_bool_t mock_get_local_ip_addresses_called = false;
static mla_array_list_t<mla_init_struct(mla_network_ip_address_t)> mock_local_ips = mla_array_list_empty<mla_init_struct(mla_network_ip_address_t)>();

static mla_array_list_t<mla_init_struct(mla_network_ip_address_t)> mock_get_local_ip_addresses() {
    mock_get_local_ip_addresses_called = true;
    return mock_local_ips;
}

inline void NetworkIpAddressInvalidTest() {
    mla_network_ip_address_t addr = mla_network_ip_address_invalid();
    assert_true(mla_string_is_empty(addr.address), "Invalid IP address should be empty");
    assert_false(addr.is_ipv6, "Invalid IP address should not be IPv6");
}

inline void NetworkIpAddressIp4Test() {
    mla_string_t ip = mla_string_const("127.0.0.1");
    mla_network_ip_address_t addr = mla_network_ip_address_ip4(ip);
    assert_true(mla_string_equals(addr.address, ip), "IPv4 address should match");
    assert_false(addr.is_ipv6, "IPv4 address should not be IPv6");
}

inline void NetworkIpAddressIp6Test() {
    mla_string_t ip = mla_string_const("::1");
    mla_network_ip_address_t addr = mla_network_ip_address_ip6(ip);
    assert_true(mla_string_equals(addr.address, ip), "IPv6 address should match");
    assert_true(addr.is_ipv6, "IPv6 address should be IPv6");
}

inline void NetworkHostIp4Test() {
    mla_string_t ip = mla_string_const("192.168.1.1");
    mla_uint16_t port = 8080;
    mla_network_host_t host = mla_network_host_ip4(ip, port);
    assert_true(mla_string_equals(host.address.address, ip), "Host IPv4 address should match");
    assert_false(host.address.is_ipv6, "Host IPv4 address should not be IPv6");
    assert_equal(host.port, port, "Host port should match");
}

inline void NetworkHostIp6Test() {
    mla_string_t ip = mla_string_const("fe80::1");
    mla_uint16_t port = 443;
    mla_network_host_t host = mla_network_host_ip6(ip, port);
    assert_true(mla_string_equals(host.address.address, ip), "Host IPv6 address should match");
    assert_true(host.address.is_ipv6, "Host IPv6 address should be IPv6");
    assert_equal(host.port, port, "Host port should match");
}

inline void NetworkHostInvalidTest() {
    mla_network_host_t host = mla_network_host_invalid();
    assert_true(mla_string_is_empty(host.address.address), "Invalid host address should be empty");
    assert_equal(host.port, (mla_uint16_t)0, "Invalid host port should be 0");
}

inline void NetworkGetLocalIpAddressesTest() {
    mla_network_low_level_operations_t original = g_network_low_level_operations;
    g_network_low_level_operations.get_local_ip_addresses = mock_get_local_ip_addresses;

    mock_get_local_ip_addresses_called = false;
    mock_local_ips = mla_array_list<mla_init_struct(mla_network_ip_address_t)>();
    mla_array_list_add(mock_local_ips, mla_network_ip_address_ip4(mla_string_copy("127.0.0.1", 9)));

    auto result = mla_network_get_local_ip_addresses();
    assert_true(mock_get_local_ip_addresses_called, "Mock get_local_ip_addresses should be called");
    assert_equal((mla_test_uint32_t)mla_array_list_size(result), (mla_test_uint32_t)1, "Result size should be 1");

    if (mla_array_list_size(result) > 0) {
        mla_network_ip_address_t ip = mla_array_list_get_unsafe(result, 0);
        assert_true(mla_string_equals(ip.address, mla_string_const("127.0.0.1")), "IP address should match");
    }

    mock_local_ips = mla_array_list_empty<mla_init_struct(mla_network_ip_address_t)>();
    g_network_low_level_operations = original;
}

inline void NetworkHostResolveTest() {
    mla_network_low_level_operations_t original = g_network_low_level_operations;
    g_network_low_level_operations.resolve_host = mock_resolve_host;

    mock_resolve_host_called = false;
    mock_resolved_host = mla_network_host_ip4(mla_string_copy("1.2.3.4", 7), 80);
    mock_resolve_host_result = true;

    mla_network_host_t host = mla_network_host_invalid();
    mla_bool_t success = mla_network_host_resolve(host, mla_string_const("example.com"), 80);

    assert_true(mock_resolve_host_called, "Mock resolve_host should be called");
    assert_true(success, "Resolve should succeed");
    assert_true(mla_string_equals(host.address.address, mla_string_const("1.2.3.4")), "Resolved IP should match");
    assert_equal(host.port, (mla_uint16_t)80, "Resolved port should match");

    // Test empty hostname
    mock_resolve_host_called = false;
    success = mla_network_host_resolve(host, mla_string_empty(), 80);
    assert_false(mock_resolve_host_called, "Mock resolve_host should NOT be called for empty hostname");
    assert_false(success, "Resolve should fail for empty hostname");

    mock_resolved_host.address.address = mla_string_empty();
    mock_resolved_host = mla_network_host_invalid();
    g_network_low_level_operations = original;
}

inline void NetworkConnectionConnectTest() {
    mla_network_low_level_operations_t original = g_network_low_level_operations;
    g_network_low_level_operations.connect = mock_connect;
    g_network_low_level_operations.connect_secure = nullptr;

    mock_connect_called = false;
    mock_connect_result = true;

    mla_network_connection_t conn = mla_network_connection_disconnected();
    mla_network_host_t host = mla_network_host_ip4(mla_string_const("127.0.0.1"), 80);

    mla_bool_t success = mla_network_connection_connect(conn, host, mla_connection_type_tcp, 1000);
    assert_true(mock_connect_called, "Mock connect should be called");
    assert_true(success, "Connect should succeed");
    assert_true(mla_network_connection_is_connected(conn), "Connection should be connected");

    // Test connect with invalid host (port 0)
    mock_connect_called = false;
    mla_network_host_t invalid_host = mla_network_host_ip4(mla_string_const("127.0.0.1"), 0);
    success = mla_network_connection_connect(conn, invalid_host, mla_connection_type_tcp, 1000);
    assert_false(mock_connect_called, "Mock connect should NOT be called for invalid host");
    assert_false(success, "Connect should fail for invalid host");

    g_network_low_level_operations = original;
}

inline void NetworkConnectionDisconnectTest() {
    mla_network_connection_t conn = mla_network_connection_disconnected();
    conn.host = mla_network_host_ip4(mla_string_const("127.0.0.1"), 80);
    conn.inputStream = mla_stream_noop_input();
    conn.outputStream = mla_stream_noop_output();

    assert_true(mla_network_connection_is_connected(conn), "Should be connected initially");

    mla_bool_t success = mla_network_connection_disconnect(conn);
    assert_true(success, "Disconnect should succeed");
    assert_false(mla_network_connection_is_connected(conn), "Should be disconnected");
}

static mla_bool_t mock_accept_called = false;
static mla_bool_t mock_accept_connection(const mla_network_listener_t& listener, mla_network_connection_t& connection) {
    (void)listener;
    (void)connection;
    mock_accept_called = true;
    return true;
}

inline void NetworkListenerTest() {
    mla_network_low_level_operations_t original = g_network_low_level_operations;
    g_network_low_level_operations.bind_and_listen = mock_bind_and_listen;
    g_network_low_level_operations.bind_and_listen_secure = nullptr;

    mock_bind_and_listen_called = false;
    mock_bind_and_listen_result = true;

    mla_network_listener_t listener = mla_network_listener_invalid();
    mla_network_host_t host = mla_network_host_ip4(mla_string_const("0.0.0.0"), 8080);

    mla_bool_t success = mla_network_listener_bind_and_listen(listener, host, mla_connection_type_tcp);
    assert_true(mock_bind_and_listen_called, "Mock bind_and_listen should be called");
    assert_true(success, "Bind and listen should succeed");

    // Test accept_connection delegation
    mock_accept_called = false;
    listener.accept_connection = mock_accept_connection;

    mla_network_connection_t conn = mla_network_connection_disconnected();
    success = mla_network_listener_accept_connection(listener, conn);
    assert_true(mock_accept_called, "Custom accept_connection should be called");
    assert_true(success, "Accept should succeed");

    // Test close
    success = mla_network_listener_close(listener);
    assert_true(success, "Close should succeed");
    assert_null((void*)listener.accept_connection, "Accept function should be null after close");

    g_network_low_level_operations = original;
}

inline void RegisterNetworkTests(mla_test_executor_t &p_TestExecutor) {
    mla_test_t test = mla_test("IpAddressInvalid", test_category, NetworkIpAddressInvalidTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("IpAddressIp4", test_category, NetworkIpAddressIp4Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("IpAddressIp6", test_category, NetworkIpAddressIp6Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("HostIp4", test_category, NetworkHostIp4Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("HostIp6", test_category, NetworkHostIp6Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("HostInvalid", test_category, NetworkHostInvalidTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("GetLocalIpAddresses", test_category, NetworkGetLocalIpAddressesTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("HostResolve", test_category, NetworkHostResolveTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ConnectionConnect", test_category, NetworkConnectionConnectTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ConnectionDisconnect", test_category, NetworkConnectionDisconnectTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Listener", test_category, NetworkListenerTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}

#endif
