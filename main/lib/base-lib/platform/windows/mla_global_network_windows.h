//
// Created by chris on 10/10/2025.
//

#ifndef MLA_GLOBAL_NETWORK_WINDOWS_H
#define MLA_GLOBAL_NETWORK_WINDOWS_H

#include "../../core/network/mla_network.h"
#include "winsock2.h"
#include "ws2tcpip.h"
#include "iphlpapi.h"
#include <cstdio>

mla_user_data_id_init(mla_network_connection_user_data_name)

mla_bool_t mla_private_windows_resolve_host(mla_network_host_t &host, const mla_string_t &hostname, mla_uint16_t port) {
    WSADATA wsaData;
    mla_memset(&wsaData, 0, sizeof(WSADATA));
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return false;
    }

    addrinfo hints = {
        0,
        AF_UNSPEC, // IPv4 or IPv6
        SOCK_STREAM,
        IPPROTO_TCP,
        0,
        nullptr,
        nullptr,
        nullptr
    };
    mla_c_string_t cHostName = mla_string_to_cString(hostname);

    mla_char_t* cHostNamePtr = mla_pointer_get_data<mla_char_t>(cHostName.c_heap_str);

    if (cHostNamePtr == nullptr) {
        return false;
    }

    addrinfo *result = nullptr;
    if (getaddrinfo(cHostNamePtr, nullptr, &hints, &result) != 0) {
        return false;
    }

    // Extract IP address from first result
    if (result->ai_family == AF_INET) {
        sockaddr_in *addr = mla_r_cast<sockaddr_in *>(result->ai_addr);
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(addr->sin_addr), ip, INET_ADDRSTRLEN);

        host.address.address = mla_string_copy(ip, mla_strlen(ip));
        host.address.is_ipv6 = false;
        host.port = port;
    } else if (result->ai_family == AF_INET6) {
        sockaddr_in6 *addr = mla_r_cast<sockaddr_in6 *>(result->ai_addr);
        char ip[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &(addr->sin6_addr), ip, INET6_ADDRSTRLEN);

        host.address.address = mla_string_copy(ip, mla_strlen(ip));
        host.address.is_ipv6 = true;
        host.port = port;
    }

    freeaddrinfo(result);
    return true;
}

void mla_private_windows_socket_cleanup(const mla_dynamic_data_t& userData) {
    SOCKET sock = mla_s_cast<SOCKET>(userData.asUint64);
    if (sock != INVALID_SOCKET) {

        // Flush before closing
        shutdown(sock, SD_SEND);
        closesocket(sock);
    }
}


mla_size_t mla_private_windows_socket_read(mla_stream_input_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {

    (void)offset;
    mla_dynamic_data_t socket_data = mla_user_data_get_native_resource(input.userdata, mla_network_connection_user_data_name);
    SOCKET sock =mla_s_cast<SOCKET>(socket_data.asUint64);
    if (sock == INVALID_SOCKET) {
        return 0;
    }

    int bytesRead = recv(sock, mla_r_cast<char *>(buffer) + offset, mla_s_cast<int>(length), 0);
    if (bytesRead == SOCKET_ERROR || bytesRead == 0) {
        return 0;
    }

    return mla_s_cast<mla_size_t>(bytesRead);
}

mla_size_t mla_private_windows_socket_remaining_bytes(mla_stream_input_t& input) {

    mla_dynamic_data_t socket_data = mla_user_data_get_native_resource(input.userdata, mla_network_connection_user_data_name);

    SOCKET sock = mla_s_cast<SOCKET>(socket_data.asUint64);
    if (sock == INVALID_SOCKET) {
        return 0;
    }

    u_long pending = 0;
    if (ioctlsocket(sock, FIONREAD, &pending) == 0) {
        // Exact bytes currently queued for TCP; 0 means closed or none
        if (pending > 0) {
            return mla_size_max;
        }
    }

    return 0;
}

mla_size_t mla_private_windows_socket_write(mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {
    (void)offset;
    mla_dynamic_data_t socket_data = mla_user_data_get_native_resource(output.userdata, mla_network_connection_user_data_name);

    SOCKET sock = mla_s_cast<SOCKET>(socket_data.asUint64);
    if (sock == INVALID_SOCKET) {
        return 0;
    }

    mla_size_t total_sent = 0;
    mla_size_t bytes_remaining = length;
    const char* ptr = mla_r_cast<const char *>(buffer) + offset;

    while (bytes_remaining > 0) {
        int bytesSent = send(sock, ptr + total_sent, mla_s_cast<int>(bytes_remaining), 0);

        if (bytesSent != SOCKET_ERROR && bytesSent > 0) {
            total_sent += mla_s_cast<mla_size_t>(bytesSent);
            bytes_remaining -= mla_s_cast<mla_size_t>(bytesSent);
        } else {
            if (bytesSent == SOCKET_ERROR) {
                int error = WSAGetLastError();
                // WSAEWOULDBLOCK: Resource temporarily unavailable
                // WSAENOBUFS: No buffer space available (less common on TCP send, but possible)
                if (error == WSAEWOULDBLOCK || error == WSAENOBUFS) {
                    fd_set write_set;
                    FD_ZERO(&write_set);
                    FD_SET(sock, &write_set);

                    // On Windows, the first argument to select is ignored but included for compatibility
                    int result = select(0, nullptr, &write_set, nullptr, nullptr);

                    if (result == SOCKET_ERROR) {
                        break; // Select failed
                    }
                    continue; // Socket is writable again, retry send
                }

                // Real error happened
                break;
            } else {
                // bytesSent == 0 means graceful shutdown from other side (unlikely during send, but possible)
                break;
            }
        }
    }

    return total_sent;
}


mla_bool_t mla_private_windows_connect(mla_network_connection_t &connection, const mla_network_host_t &host,
                             mla_connection_type_t type, mla_size_t timeout_ms) {
    connection.host = host;

    WSADATA wsaData;
    mla_memset(&wsaData, 0, sizeof(WSADATA));
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return false;
    }

    // Create socket
    int sockType = (type == mla_connection_type_tcp) ? SOCK_STREAM : SOCK_DGRAM;
    int protocol = (type == mla_connection_type_tcp) ? IPPROTO_TCP : IPPROTO_UDP;
    int family = host.address.is_ipv6 ? AF_INET6 : AF_INET;

    SOCKET sock = socket(family, sockType, protocol);
    if (sock == INVALID_SOCKET) {
        return false;
    }

    // Set socket to non-blocking mode for timeout support
    u_long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);

    // Prepare address structure
    sockaddr_storage addr = {
        0,
        {0},
        0,
        {0}
    };
    int addrLen;

    mla_c_string_t cAddress = mla_string_to_cString(host.address.address);

    mla_char_t* cAddressPtr = mla_pointer_get_data<mla_char_t>(cAddress.c_heap_str);

    if (cAddressPtr == nullptr) {
        closesocket(sock);
        return false;
    }

    if (host.address.is_ipv6) {
        sockaddr_in6 *addr6 = mla_r_cast<sockaddr_in6*>(&addr);
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = htons(host.port);
        inet_pton(AF_INET6, cAddressPtr, &addr6->sin6_addr);
        addrLen = sizeof(sockaddr_in6);
    } else {
        sockaddr_in *addr4 = mla_r_cast<sockaddr_in*>(&addr);
        addr4->sin_family = AF_INET;
        addr4->sin_port = htons(host.port);
        inet_pton(AF_INET, cAddressPtr, &addr4->sin_addr);
        addrLen = sizeof(sockaddr_in);
    }

    // Attempt connection
    int result = connect(sock, mla_r_cast<sockaddr*>(&addr), addrLen);

    if (result == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error == WSAEWOULDBLOCK) {
            // Wait for connection with timeout
            fd_set writeSet;
            FD_ZERO(&writeSet);
            FD_SET(sock, &writeSet);

            timeval timeout = {0, 0};
            timeout.tv_sec = mla_s_cast<long>(timeout_ms) / 1000;
            timeout.tv_usec = mla_s_cast<long>(timeout_ms % 1000) * 1000;

            result = select(0, nullptr, &writeSet, nullptr, &timeout);
            if (result <= 0) {
                closesocket(sock);
                return false;
            }
        } else {
            closesocket(sock);
            return false;
        }
    }

    // Disable Nagle's algorithm (TCP_NODELAY) by default for better responsiveness
    if (type == mla_connection_type_tcp) {
        BOOL nodelay = TRUE;
        setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, mla_r_cast<const char*>(&nodelay), sizeof(nodelay));
    }

    mla_user_data_t userData = mla_user_data_empty();
    mla_user_data_set_native_resource(userData, mla_network_connection_user_data_name, mla_dynamic_data_from_uint64(sock), mla_private_windows_socket_cleanup);

    connection.inputStream = {
        userData,
        mla_private_windows_socket_read,
        mla_private_windows_socket_remaining_bytes
    };

    connection.outputStream = {
        userData,
        mla_private_windows_socket_write,
        nullptr
    };

    return true;
}

mla_bool_t mla_private_windows_connect_secure(
        mla_network_connection_t &connection,
        const mla_network_host_t &host,
        mla_connection_type_t type,
        mla_size_t timeout_ms,
        const mla_network_security_config_t &security_config) {
    if (mla_network_security_config_get_mode(security_config) == mla_network_security_mode_insecure) {
        return mla_private_windows_connect(connection, host, type, timeout_ms);
    }

    return false;
}

mla_bool_t mla_private_windows_accept_connection(const mla_network_listener_t& listener, mla_network_connection_t &connection) {

    mla_dynamic_data_t socket_data = mla_user_data_get_native_resource(listener.userdata, mla_network_connection_user_data_name);

    SOCKET listenSock = mla_s_cast<SOCKET>(socket_data.asUint64);

    if (listenSock == INVALID_SOCKET) {
        return false;
    }

    int sockType = 0;
    int optLen = mla_s_cast<int>(sizeof(sockType));
    if (getsockopt(listenSock, SOL_SOCKET, SO_TYPE, mla_r_cast<char*>(&sockType), &optLen) != 0 || sockType != SOCK_STREAM) {
        return false;
    }

    sockaddr_storage clientAddr{};
    int clientLen = mla_s_cast<int>(sizeof(clientAddr));

    SOCKET clientSock = accept(listenSock, mla_r_cast<sockaddr *>(&clientAddr), &clientLen);
    if (clientSock == INVALID_SOCKET) {
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK) {
            // No pending connection; non-blocking accept
            return false;
        }
        return false;
    }

    // Set socket to non-blocking mode for timeout support
    u_long blocking = 1;
    ioctlsocket(clientSock, FIONBIO, &blocking);

    // Disable Nagle's algorithm (TCP_NODELAY) by default for better responsiveness
    {
        BOOL nodelay = TRUE;
        setsockopt(clientSock, IPPROTO_TCP, TCP_NODELAY, mla_r_cast<const char*>(&nodelay), sizeof(nodelay));
    }

    // Fill connection.host from peer address
    mla_network_host_t peer = mla_network_host_invalid();
    char ip4[INET_ADDRSTRLEN] = {0};
    char ip6[INET6_ADDRSTRLEN] = {0};

    if (clientAddr.ss_family == AF_INET) {
        sockaddr_in* a4 = mla_r_cast<sockaddr_in*>(&clientAddr);
        inet_ntop(AF_INET, &a4->sin_addr, ip4, sizeof(ip4));
        peer.address.address = mla_string_copy(ip4, mla_strlen(ip4));
        peer.address.is_ipv6 = false;
        peer.port = ntohs(a4->sin_port);
    } else if (clientAddr.ss_family == AF_INET6) {
        sockaddr_in6* a6 = mla_r_cast<sockaddr_in6*>(&clientAddr);
        inet_ntop(AF_INET6, &a6->sin6_addr, ip6, sizeof(ip6));
        peer.address.address = mla_string_copy(ip6, mla_strlen(ip6));
        peer.address.is_ipv6 = true;
        peer.port = ntohs(a6->sin6_port);
    } else {
        closesocket(clientSock);
        return false;
    }

    connection.host = peer;

    mla_user_data_t userData = mla_user_data_empty();
    mla_user_data_set_native_resource(userData, mla_network_connection_user_data_name, mla_dynamic_data_from_uint64(clientSock), mla_private_windows_socket_cleanup);


    connection.inputStream = {
        userData,
        mla_private_windows_socket_read,
        mla_private_windows_socket_remaining_bytes
    };

    connection.outputStream = {
        userData,
        mla_private_windows_socket_write,
        nullptr
    };

    return true;
}


mla_bool_t mla_private_windows_bind_and_listen(mla_network_listener_t &listener, const mla_network_host_t &host, mla_connection_type_t type) {

    listener.host = host;


    int family   = host.address.is_ipv6 ? AF_INET6 : AF_INET;
    int sockType = (type == mla_connection_type_tcp) ? SOCK_STREAM : SOCK_DGRAM;
    int protocol = (type == mla_connection_type_tcp) ? IPPROTO_TCP : IPPROTO_UDP;

    SOCKET sock = socket(family, sockType, protocol);
    if (sock == INVALID_SOCKET) {
        return false;
    }

    // Exclusive address use
    {
        BOOL exclusive = TRUE;
        setsockopt(sock, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, mla_r_cast<const char*>(&exclusive), sizeof(exclusive));
    }

    // Allow dual-stack for IPv6
    if (family == AF_INET6) {
        DWORD v6only = 0;
        setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, mla_r_cast<const char*>(&v6only), sizeof(v6only));
    }

    sockaddr_storage ss{};
    int addrLen = 0;

    mla_c_string_t cAddress = mla_string_to_cString(host.address.address);

    mla_char_t* cAddressPtr = mla_pointer_get_data<mla_char_t>(cAddress.c_heap_str);

    if (cAddressPtr == nullptr) {
        closesocket(sock);
        return false;
    }

    if (host.address.is_ipv6) {
        sockaddr_in6* addr6 = mla_r_cast<sockaddr_in6*>(&ss);
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = htons(host.port);
        mla_bool_t ok = (inet_pton(AF_INET6, cAddressPtr, &addr6->sin6_addr) == 1);
        if (!ok) {
            addr6->sin6_addr = in6addr_any;
        }
        addrLen = sizeof(sockaddr_in6);
    } else {
        sockaddr_in* addr4 = mla_r_cast<sockaddr_in*>(&ss);
        addr4->sin_family = AF_INET;
        addr4->sin_port = htons(host.port);
        mla_bool_t ok = (inet_pton(AF_INET, cAddressPtr, &addr4->sin_addr) == 1);
        if (!ok) {
            addr4->sin_addr.s_addr = htonl(INADDR_ANY);
        }
        addrLen = sizeof(sockaddr_in);
    }

    if (bind(sock, mla_r_cast<sockaddr*>(&ss), addrLen) == SOCKET_ERROR) {
        closesocket(sock);
        return false;
    }

    if (type == mla_connection_type_tcp) {
        if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
            closesocket(sock);
            return false;
        }

        // Make the listening socket non-blocking so accept() will not block
        u_long nb = 1;
        if (ioctlsocket(sock, FIONBIO, &nb) == SOCKET_ERROR) {
            closesocket(sock);
            return false;
        }
    }

    mla_user_data_t userData = mla_user_data_empty();
    mla_user_data_set_native_resource(userData, mla_network_connection_user_data_name, mla_dynamic_data_from_uint64(sock), mla_private_windows_socket_cleanup);
    listener.accept_connection = mla_private_windows_accept_connection;
    listener.userdata = userData;

    return true;
}

mla_bool_t mla_private_windows_bind_and_listen_secure(
        mla_network_listener_t &listener,
        const mla_network_host_t &host,
        mla_connection_type_t type,
        const mla_network_security_config_t &security_config) {
    if (mla_network_security_config_get_mode(security_config) == mla_network_security_mode_insecure) {
        return mla_private_windows_bind_and_listen(listener, host, type);
    }

    return false;
}


mla_array_list_t<mla_init_struct(mla_network_ip_address_t)> mla_private_windows_get_local_ip_addresses() {
    mla_array_list_t<mla_init_struct(mla_network_ip_address_t)> ipAddresses = mla_array_list_empty<mla_init_struct(mla_network_ip_address_t)>();

    ULONG bufferSize = 15000;
    IP_ADAPTER_ADDRESSES* adapterAddresses = nullptr;
    ULONG result = ERROR_BUFFER_OVERFLOW;

    // Retry with increasing buffer size (Microsoft recommended pattern)
    for (mla_int32_t attempt = 0; attempt < 3 && result == ERROR_BUFFER_OVERFLOW; ++attempt) {
        adapterAddresses = mla_s_cast<IP_ADAPTER_ADDRESSES *>(mla_platform_malloc(bufferSize));
        if (adapterAddresses == nullptr) {
            return ipAddresses;
        }

        result = GetAdaptersAddresses(
            AF_UNSPEC,
            GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER,
            nullptr,
            adapterAddresses,
            &bufferSize
        );

        if (result == ERROR_BUFFER_OVERFLOW) {
            mla_platform_free(adapterAddresses);
            adapterAddresses = nullptr;
        }
    }

    if (result != NO_ERROR || adapterAddresses == nullptr) {
        if (adapterAddresses != nullptr) {
            mla_platform_free(adapterAddresses);
        }
        return ipAddresses;
    }

    for (IP_ADAPTER_ADDRESSES* adapter = adapterAddresses; adapter != nullptr; adapter = adapter->Next) {
        // Skip loopback and non-operational adapters
        if (adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK) {
            continue;
        }
        if (adapter->OperStatus != IfOperStatusUp) {
            continue;
        }

        for (IP_ADAPTER_UNICAST_ADDRESS* unicast = adapter->FirstUnicastAddress; unicast != nullptr; unicast = unicast->Next) {
            mla_network_ip_address_t ipAddress = mla_network_ip_address_invalid();

            if (unicast->Address.lpSockaddr->sa_family == AF_INET) {
                sockaddr_in* addr4 = mla_r_cast<sockaddr_in *>(unicast->Address.lpSockaddr);
                char ip[INET_ADDRSTRLEN] = {0};
                inet_ntop(AF_INET, &addr4->sin_addr, ip, INET_ADDRSTRLEN);
                ipAddress.address = mla_string_copy(ip, mla_strlen(ip));
                ipAddress.is_ipv6 = false;
                mla_array_list_add(ipAddresses, ipAddress);

            } else if (unicast->Address.lpSockaddr->sa_family == AF_INET6) {
                sockaddr_in6* addr6 = mla_r_cast<sockaddr_in6 *>(unicast->Address.lpSockaddr);
                char ip[INET6_ADDRSTRLEN] = {0};
                inet_ntop(AF_INET6, &addr6->sin6_addr, ip, INET6_ADDRSTRLEN);
                ipAddress.address = mla_string_copy(ip, mla_strlen(ip));
                ipAddress.is_ipv6 = true;
                mla_array_list_add(ipAddresses, ipAddress);
            }
        }
    }

    mla_platform_free(adapterAddresses);
    return ipAddresses;
}

mla_network_low_level_operations_t g_network_low_level_operations = {
    mla_private_windows_resolve_host,
    mla_private_windows_connect,
    mla_private_windows_connect_secure,
    mla_private_windows_bind_and_listen,
    mla_private_windows_bind_and_listen_secure,
    mla_private_windows_get_local_ip_addresses
};




void mla_private_network_teardown_platform_windows() {
    WSACleanup();
}

void mla_private_network_setup_platform_windows() {

    WSADATA wsaData;
    memset(&wsaData, 0, sizeof(WSADATA));
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        // Get more infos about the error
        auto last_error = WSAGetLastError();
        printf("WSAStartup failed: %d\n", last_error);

    }

}

// RAII auto init
struct MlaWinsockAutoInit {
    MlaWinsockAutoInit()  { mla_private_network_setup_platform_windows(); }
    ~MlaWinsockAutoInit() { mla_private_network_teardown_platform_windows(); }
};

// Single global instance
static MlaWinsockAutoInit g_mlaWinsockAutoInit;

#endif
