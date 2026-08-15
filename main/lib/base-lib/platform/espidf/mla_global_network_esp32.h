//
// Created by copilot on 12/28/2024.
//

#ifndef MLA_GLOBAL_NETWORK_ESP32_H
#define MLA_GLOBAL_NETWORK_ESP32_H

#include "../../core/network/mla_network.h"
#include "../../core/lifecycle/mla_lifecycle_events.h"

#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <lwip/tcp.h>
#include <lwip/inet.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <net/if.h>
#include <fcntl.h>

#include <esp_netif.h>
#include <esp_event.h>

inline mla_user_data_id mla_private_network_connection_user_data_id() {
    static const mla_user_data_id id = mla_get_next_user_data_id();
    return id;
}

//////////////////////////////////////////////////////////////////
/// ESP32 lwIP TCP/IP Stack Initialization
/// Must be called before any socket operations.
/// Initializes esp_netif and the default event loop which in turn
//////////////////////////////////////////////////////////////////
void mla_private_esp32_network_stack_init() {

    static bool initialized = false;
    if (initialized) {
        return;
    }

    esp_err_t err = esp_netif_init();
    if (err != ESP_OK) {
        ESP_ERROR_CHECK(err);
    }

    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_ERROR_CHECK(err);
    }

    initialized = true;
}

mla_lifecycle_boot_event_static_register(mla_lifecycle_boot_event_priority_network_preSetup, mla_private_esp32_network_stack_init)

mla_bool_t mla_private_esp32_resolve_host(mla_network_host_t &host, const mla_string_t &hostname, mla_uint16_t port) {
    struct addrinfo hints;
    mla_memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    mla_c_string_t cHostName = mla_string_to_cString(hostname);
    if (cHostName.c_str == nullptr) {
        return false;
    }

    struct addrinfo *result = nullptr;
    if (getaddrinfo(cHostName.c_str, nullptr, &hints, &result) != 0) {
        if (cHostName.isOwner) {
            mla_platform_free(mla_c_cast<mla_char_t *>(cHostName.c_str));
        }
        return false;
    }

    if (cHostName.isOwner) {
        mla_platform_free(mla_c_cast<mla_char_t *>(cHostName.c_str));
    }

    // Extract IP address from first result
    if (result->ai_family == AF_INET) {
        struct sockaddr_in *addr = (struct sockaddr_in *) result->ai_addr;
        mla_char_t ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(addr->sin_addr), ip, INET_ADDRSTRLEN);

        host.address.address = mla_string_copy(ip, mla_strlen(ip));
        host.address.is_ipv6 = false;
        host.port = port;
    } else if (result->ai_family == AF_INET6) {
        struct sockaddr_in6 *addr = (struct sockaddr_in6 *) result->ai_addr;
        mla_char_t ip[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &(addr->sin6_addr), ip, INET6_ADDRSTRLEN);

        host.address.address = mla_string_copy(ip, mla_strlen(ip));
        host.address.is_ipv6 = true;
        host.port = port;
    }

    freeaddrinfo(result);
    return true;
}

void mla_private_esp32_socket_cleanup(const mla_dynamic_data_t& userData) {

    mla_int32_t sock = userData.asInt32;
    if (sock >= 0) {
        lwip_close(sock);
    }
}

mla_size_t mla_private_esp32_socket_read(mla_stream_input_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {
    (void)offset;
    mla_dynamic_data_t socket_data = mla_user_data_get_native_resource(input.userdata, mla_private_network_connection_user_data_id());
    mla_int32_t sock = socket_data.asInt32;
    if (sock < 0) {
        return 0;
    }

    mla_int32_t bytesRead = lwip_recv(sock, (mla_char_t*)buffer + offset, length, 0);
    if (bytesRead <= 0) {
        return 0;
    }

    return (mla_size_t)bytesRead;
}

mla_size_t mla_private_esp32_socket_remaining_bytes(mla_stream_input_t& input) {
    mla_dynamic_data_t socket_data = mla_user_data_get_native_resource(input.userdata, mla_private_network_connection_user_data_id());
    mla_int32_t sock = socket_data.asInt32;
    if (sock < 0) {
        return 0;
    }

    mla_int32_t pending = 0;
    if (lwip_ioctl(sock, FIONREAD, &pending) == 0) {
        if (pending > 0) {
            return mla_size_max;
        }
    }

    return 0;
}

mla_size_t mla_private_esp32_socket_write(mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {
    mla_dynamic_data_t socket_data = mla_user_data_get_native_resource(output.userdata, mla_private_network_connection_user_data_id());
    mla_int32_t sock = socket_data.asInt32;
    if (sock < 0) {
        return 0;
    }

    mla_size_t total_sent = 0;
    mla_size_t bytes_remaining = length;
    const mla_char_t* ptr = (const mla_char_t*)buffer + offset;

    while (bytes_remaining > 0) {
        mla_int32_t sent = lwip_send(sock, ptr + total_sent, bytes_remaining, 0);

        if (sent > 0) {
            total_sent += sent;
            bytes_remaining -= sent;
        } else {
            if (sent < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                // Socket buffer is full; wait for it to become writable
                fd_set write_set;
                FD_ZERO(&write_set);
                FD_SET(sock, &write_set);

                // Select treats the socket as non-blocking just for this wait
                if (select(sock + 1, nullptr, &write_set, nullptr, nullptr) < 0) {
                    break; // Select error
                }
                continue;
            } else if (sent < 0 && errno == EINTR) {
                continue; // Interrupted by signal, retry
            }

            // Fatal error (e.g., connection reset)
            break;
        }
    }

    return total_sent;
}

mla_bool_t mla_private_esp32_connect(mla_network_connection_t &connection, const mla_network_host_t &host,
                           mla_connection_type_t type, mla_size_t timeout_ms) {
    connection.host = host;

    // Create socket
    mla_int32_t sockType = (type == mla_connection_type_tcp) ? SOCK_STREAM : SOCK_DGRAM;
    mla_int32_t protocol = (type == mla_connection_type_tcp) ? IPPROTO_TCP : IPPROTO_UDP;
    mla_int32_t family = host.address.is_ipv6 ? AF_INET6 : AF_INET;

    mla_int32_t sock = lwip_socket(family, sockType, protocol);
    if (sock < 0) {
        return false;
    }

    // Set socket to non-blocking mode for timeout support
    mla_int32_t flags = lwip_fcntl(sock, F_GETFL, 0);
    lwip_fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    // Prepare address structure
    struct sockaddr_storage addr;
    mla_memset(&addr, 0, sizeof(addr));
    socklen_t addrLen;

    mla_c_string_t cAddress = mla_string_to_cString(host.address.address);
    if (cAddress.c_str == nullptr) {
        lwip_close(sock);
        return false;
    }

    if (host.address.is_ipv6) {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6*)&addr;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = htons(host.port);
        inet_pton(AF_INET6, cAddress.c_str, &addr6->sin6_addr);
        addrLen = sizeof(struct sockaddr_in6);
    } else {
        struct sockaddr_in *addr4 = (struct sockaddr_in*)&addr;
        addr4->sin_family = AF_INET;
        addr4->sin_port = htons(host.port);
        inet_pton(AF_INET, cAddress.c_str, &addr4->sin_addr);
        addrLen = sizeof(struct sockaddr_in);
    }

    if (cAddress.isOwner) {
        mla_platform_free(mla_c_cast<mla_char_t*>(cAddress.c_str));
    }

    // Attempt connection
    mla_int32_t result = lwip_connect(sock, (struct sockaddr*)&addr, addrLen);

    if (result < 0) {
        if (errno == EINPROGRESS) {
            // Wait for connection with timeout
            fd_set writeSet;
            FD_ZERO(&writeSet);
            FD_SET(sock, &writeSet);

            struct timeval timeout = {0, 0};
            timeout.tv_sec = timeout_ms / 1000;
            timeout.tv_usec = (timeout_ms % 1000) * 1000;

            result = lwip_select(sock + 1, nullptr, &writeSet, nullptr, &timeout);
            if (result <= 0) {
                lwip_close(sock);
                return false;
            }

            // Check if connection was successful
            mla_int32_t error = 0;
            socklen_t len = sizeof(error);
            if (lwip_getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error != 0) {
                lwip_close(sock);
                return false;
            }
        } else {
            lwip_close(sock);
            return false;
        }
    }

    // Disable Nagle's algorithm (TCP_NODELAY) by default for better responsiveness
    if (type == mla_connection_type_tcp) {
        mla_int32_t nodelay = 1;
        lwip_setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));
    }

    mla_user_data_t userData = mla_user_data_empty();
    mla_user_data_set_native_resource(userData, mla_private_network_connection_user_data_id(), mla_dynamic_data_from_int32(sock), mla_private_esp32_socket_cleanup);

    connection.inputStream = {
        userData,
        mla_private_esp32_socket_read,
        mla_private_esp32_socket_remaining_bytes
    };

    connection.outputStream = {
        userData,
        mla_private_esp32_socket_write,
        nullptr
    };

    return true;
}

mla_bool_t mla_private_esp32_connect_secure(
        mla_network_connection_t &connection,
        const mla_network_host_t &host,
        mla_connection_type_t type,
        mla_size_t timeout_ms,
        const mla_network_security_config_t &security_config) {
    if (mla_network_security_config_get_mode(security_config) == mla_network_security_mode_insecure) {
        return mla_private_esp32_connect(connection, host, type, timeout_ms);
    }

    return false;
}

mla_bool_t mla_private_esp32_accept_connection(const mla_network_listener_t& listener, mla_network_connection_t &connection) {
    mla_dynamic_data_t socket_data = mla_user_data_get_native_resource(listener.userdata, mla_private_network_connection_user_data_id());
    mla_int32_t listenSock = socket_data.asInt32;
    if (listenSock < 0) {
        return false;
    }

    mla_int32_t sockType = 0;
    socklen_t optLen = sizeof(sockType);
    if (lwip_getsockopt(listenSock, SOL_SOCKET, SO_TYPE, &sockType, &optLen) != 0 || sockType != SOCK_STREAM) {
        return false;
    }

    struct sockaddr_storage clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    mla_int32_t clientSock = lwip_accept(listenSock, (struct sockaddr*)&clientAddr, &clientLen);
    if (clientSock < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No pending connection; non-blocking accept
            return false;
        }
        return false;
    }

    // Set accepted socket to non blocking mode
    mla_int32_t flags = lwip_fcntl(clientSock, F_GETFL, 0);
    lwip_fcntl(clientSock, F_SETFL, flags | O_NONBLOCK);

    // Disable Nagle's algorithm (TCP_NODELAY) by default for better responsiveness
    mla_int32_t nodelay = 1;
    lwip_setsockopt(clientSock, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));

    // Fill connection.host from peer address
    mla_network_host_t peer = mla_network_host_invalid();
    mla_char_t ip4[INET_ADDRSTRLEN] = {0};
    mla_char_t ip6[INET6_ADDRSTRLEN] = {0};

    if (clientAddr.ss_family == AF_INET) {
        struct sockaddr_in* a4 = (struct sockaddr_in*)&clientAddr;
        inet_ntop(AF_INET, &a4->sin_addr, ip4, sizeof(ip4));
        peer.address.address = mla_string_copy(ip4, mla_strlen(ip4));
        peer.address.is_ipv6 = false;
        peer.port = ntohs(a4->sin_port);
    } else if (clientAddr.ss_family == AF_INET6) {
        struct sockaddr_in6* a6 = (struct sockaddr_in6*)&clientAddr;
        inet_ntop(AF_INET6, &a6->sin6_addr, ip6, sizeof(ip6));
        peer.address.address = mla_string_copy(ip6, mla_strlen(ip6));
        peer.address.is_ipv6 = true;
        peer.port = ntohs(a6->sin6_port);
    } else {
        lwip_close(clientSock);
        return false;
    }

    connection.host = peer;

    mla_user_data_t userData = mla_user_data_empty();
    mla_user_data_set_native_resource(userData, mla_private_network_connection_user_data_id(), mla_dynamic_data_from_int32(clientSock), mla_private_esp32_socket_cleanup);

    connection.inputStream = {
        userData,
        mla_private_esp32_socket_read,
        mla_private_esp32_socket_remaining_bytes
    };

    connection.outputStream = {
        userData,
        mla_private_esp32_socket_write,
        nullptr
    };

    return true;
}

mla_bool_t mla_private_esp32_bind_and_listen(mla_network_listener_t &listener, const mla_network_host_t &host, mla_connection_type_t type) {
    listener.host = host;

    mla_int32_t family = host.address.is_ipv6 ? AF_INET6 : AF_INET;
    mla_int32_t sockType = (type == mla_connection_type_tcp) ? SOCK_STREAM : SOCK_DGRAM;
    mla_int32_t protocol = (type == mla_connection_type_tcp) ? IPPROTO_TCP : IPPROTO_UDP;

    mla_int32_t sock = lwip_socket(family, sockType, protocol);
    if (sock < 0) {
        return false;
    }

    // Enable address reuse
    mla_int32_t reuseAddr = 1;
    lwip_setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr));

    // Allow dual-stack for IPv6
    if (family == AF_INET6) {
        mla_int32_t v6only = 0;
        lwip_setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &v6only, sizeof(v6only));
    }

    struct sockaddr_storage ss;
    mla_memset(&ss, 0, sizeof(ss));
    socklen_t addrLen = 0;

    mla_c_string_t cAddress = mla_string_to_cString(host.address.address);
    if (cAddress.c_str == nullptr) {
        lwip_close(sock);
        return false;
    }

    if (host.address.is_ipv6) {
        struct sockaddr_in6* addr6 = (struct sockaddr_in6*)&ss;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = htons(host.port);
        mla_bool_t ok = (inet_pton(AF_INET6, cAddress.c_str, &addr6->sin6_addr) == 1);
        if (!ok) {
            addr6->sin6_addr = in6addr_any;
        }
        addrLen = sizeof(struct sockaddr_in6);
    } else {
        struct sockaddr_in* addr4 = (struct sockaddr_in*)&ss;
        addr4->sin_family = AF_INET;
        addr4->sin_port = htons(host.port);
        mla_bool_t ok = (inet_pton(AF_INET, cAddress.c_str, &addr4->sin_addr) == 1);
        if (!ok) {
            addr4->sin_addr.s_addr = htonl(INADDR_ANY);
        }
        addrLen = sizeof(struct sockaddr_in);
    }

    if (cAddress.isOwner) {
        mla_platform_free(mla_c_cast<mla_char_t*>(cAddress.c_str));
    }

    if (lwip_bind(sock, (struct sockaddr*)&ss, addrLen) < 0) {
        lwip_close(sock);
        return false;
    }

    if (type == mla_connection_type_tcp) {
        if (lwip_listen(sock, SOMAXCONN) < 0) {
            lwip_close(sock);
            return false;
        }

        // Make the listening socket non-blocking so accept() will not block
        mla_int32_t flags = lwip_fcntl(sock, F_GETFL, 0);
        lwip_fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    }

    mla_user_data_t userData = mla_user_data_empty();
    mla_user_data_set_native_resource(userData, mla_private_network_connection_user_data_id(), mla_dynamic_data_from_int32(sock), mla_private_esp32_socket_cleanup);

    listener.accept_connection = mla_private_esp32_accept_connection;
    listener.userdata = userData;

    return true;
}

mla_bool_t mla_private_esp32_bind_and_listen_secure(
        mla_network_listener_t &listener,
        const mla_network_host_t &host,
        mla_connection_type_t type,
        const mla_network_security_config_t &security_config) {
    if (mla_network_security_config_get_mode(security_config) == mla_network_security_mode_insecure) {
        return mla_private_esp32_bind_and_listen(listener, host, type);
    }

    return false;
}

mla_array_list_t<mla_init_struct(mla_network_ip_address_t)> mla_private_esp32_get_local_ip_addresses() {
    mla_array_list_t<mla_init_struct(mla_network_ip_address_t)> ipAddresses = mla_array_list_empty<mla_init_struct(mla_network_ip_address_t)>();

    esp_netif_t *netif = nullptr;
    // Use the unsafe variant as esp_netif_next is deprecated
    while ((netif = esp_netif_next_unsafe(netif)) != nullptr) {
        if (!esp_netif_is_netif_up(netif)) {
            continue;
        }

        // Handle IPv4
        esp_netif_ip_info_t ip_info;
        if (esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
            if (ip_info.ip.addr != 0) {

                mla_char_t ipStr[INET_ADDRSTRLEN] = {0};
                struct in_addr ip4_addr;
                ip4_addr.s_addr = ip_info.ip.addr;

                if (inet_ntop(AF_INET, &ip4_addr, ipStr, sizeof(ipStr))) {
                    mla_network_ip_address_t addr = mla_network_ip_address_invalid();
                    addr.address = mla_string_copy(ipStr, mla_strlen(ipStr));
                    addr.is_ipv6 = false;
                    mla_array_list_add(ipAddresses, addr);
                }
            }
        }

        // Handle IPv6 (Link Local and Global)
        esp_ip6_addr_t ip6_addr;

        if (esp_netif_get_ip6_linklocal(netif, &ip6_addr) == ESP_OK) {
             mla_char_t ipStr[INET6_ADDRSTRLEN] = {0};
             if (inet_ntop(AF_INET6, &ip6_addr.addr, ipStr, sizeof(ipStr))) {
                 mla_network_ip_address_t addr = mla_network_ip_address_invalid();
                 addr.address = mla_string_copy(ipStr, mla_strlen(ipStr));
                 addr.is_ipv6 = true;
                 mla_array_list_add(ipAddresses, addr);
             }
        }

        if (esp_netif_get_ip6_global(netif, &ip6_addr) == ESP_OK) {
             mla_char_t ipStr[INET6_ADDRSTRLEN] = {0};
             if (inet_ntop(AF_INET6, &ip6_addr.addr, ipStr, sizeof(ipStr))) {
                 mla_network_ip_address_t addr = mla_network_ip_address_invalid();
                 addr.address = mla_string_copy(ipStr, mla_strlen(ipStr));
                 addr.is_ipv6 = true;
                 mla_array_list_add(ipAddresses, addr);
             }
        }
    }

    return ipAddresses;
}

mla_network_low_level_operations_t g_network_low_level_operations = {
    mla_private_esp32_resolve_host,
    mla_private_esp32_connect,
    mla_private_esp32_connect_secure,
    mla_private_esp32_bind_and_listen,
    mla_private_esp32_bind_and_listen_secure,
    mla_private_esp32_get_local_ip_addresses
};

#endif
