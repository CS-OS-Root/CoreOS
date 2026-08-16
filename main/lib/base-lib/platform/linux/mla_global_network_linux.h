//
// Created by christian on 10/17/25.
//

#ifndef MLA_GLOBAL_NETWORK_LINUX_H
#define MLA_GLOBAL_NETWORK_LINUX_H

#include "../../core/network/mla_network.h"

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <netinet/tcp.h>
#include <ifaddrs.h>
#include <net/if.h>

#if defined(MLA_HAS_OPENSSL) && MLA_HAS_OPENSSL == 1
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#endif

inline mla_user_data_id mla_private_network_connection_user_data_id() {
    static const mla_user_data_id id = mla_get_next_user_data_id();
    return id;
}

#if defined(MLA_HAS_OPENSSL) && MLA_HAS_OPENSSL == 1
inline mla_user_data_id mla_private_network_ssl_connection_user_data_id() {
    static const mla_user_data_id id = mla_get_next_user_data_id();
    return id;
}

struct mla_linux_ssl_context_t {
    int sock;
    SSL* ssl;
};

inline SSL_CTX* mla_linux_get_client_ssl_ctx() {
    static SSL_CTX* g_client_ssl_ctx = nullptr;
    if (g_client_ssl_ctx == nullptr) {
        OPENSSL_init_ssl(0, nullptr);
        g_client_ssl_ctx = SSL_CTX_new(TLS_client_method());
        if (g_client_ssl_ctx != nullptr) {
            SSL_CTX_set_default_verify_paths(g_client_ssl_ctx);
        }
    }
    return g_client_ssl_ctx;
}

inline void mla_linux_ssl_socket_cleanup(const mla_dynamic_data_t& userData) {
    mla_linux_ssl_context_t* ctx = mla_r_cast<mla_linux_ssl_context_t*>(userData.asPointer);
    if (ctx != nullptr) {
        if (ctx->ssl != nullptr) {
            SSL_shutdown(ctx->ssl);
            SSL_free(ctx->ssl);
            ctx->ssl = nullptr;
        }
        if (ctx->sock >= 0) {
            close(ctx->sock);
            ctx->sock = -1;
        }
        mla_platform_free(ctx);
    }
}

inline mla_size_t mla_linux_ssl_socket_read(mla_stream_input_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {
    (void)offset;
    mla_dynamic_data_t socket_data = mla_user_data_get_native_resource(input.userdata, mla_private_network_ssl_connection_user_data_id());
    mla_linux_ssl_context_t* ssl_ctx = mla_r_cast<mla_linux_ssl_context_t*>(socket_data.asPointer);
    if (ssl_ctx == nullptr || ssl_ctx->ssl == nullptr || ssl_ctx->sock < 0) {
        return 0;
    }

    int bytesRead = SSL_read(ssl_ctx->ssl, mla_r_cast<char*>(buffer) + offset, mla_s_cast<int>(length));
    if (bytesRead > 0) {
        return mla_s_cast<mla_size_t>(bytesRead);
    }

    int err = SSL_get_error(ssl_ctx->ssl, bytesRead);
    if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(ssl_ctx->sock, &fds);
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 50000;
        if (err == SSL_ERROR_WANT_READ) {
            select(ssl_ctx->sock + 1, &fds, nullptr, nullptr, &tv);
        } else {
            select(ssl_ctx->sock + 1, nullptr, &fds, nullptr, &tv);
        }
        int retryBytes = SSL_read(ssl_ctx->ssl, mla_r_cast<char*>(buffer) + offset, mla_s_cast<int>(length));
        if (retryBytes > 0) {
            return mla_s_cast<mla_size_t>(retryBytes);
        }
    }

    return 0;
}

inline mla_size_t mla_linux_ssl_socket_remaining_bytes(mla_stream_input_t& input) {
    mla_dynamic_data_t socket_data = mla_user_data_get_native_resource(input.userdata, mla_private_network_ssl_connection_user_data_id());
    mla_linux_ssl_context_t* ssl_ctx = mla_r_cast<mla_linux_ssl_context_t*>(socket_data.asPointer);
    if (ssl_ctx == nullptr || ssl_ctx->ssl == nullptr) {
        return 0;
    }

    int pending = SSL_pending(ssl_ctx->ssl);
    if (pending > 0) {
        return mla_s_cast<mla_size_t>(pending);
    }

    int socket_pending = 0;
    if (ioctl(ssl_ctx->sock, FIONREAD, &socket_pending) == 0 && socket_pending > 0) {
        return mla_size_max;
    }

    return 0;
}

inline mla_size_t mla_linux_ssl_socket_write(mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {
    (void)offset;
    mla_dynamic_data_t socket_data = mla_user_data_get_native_resource(output.userdata, mla_private_network_ssl_connection_user_data_id());
    mla_linux_ssl_context_t* ssl_ctx = mla_r_cast<mla_linux_ssl_context_t*>(socket_data.asPointer);
    if (ssl_ctx == nullptr || ssl_ctx->ssl == nullptr || ssl_ctx->sock < 0) {
        return 0;
    }

    mla_size_t total_sent = 0;
    mla_size_t bytes_remaining = length;
    const char* ptr = mla_r_cast<const char*>(buffer) + offset;

    while (bytes_remaining > 0) {
        int sent = SSL_write(ssl_ctx->ssl, ptr + total_sent, mla_s_cast<int>(bytes_remaining));
        if (sent > 0) {
            total_sent += mla_s_cast<mla_size_t>(sent);
            bytes_remaining -= mla_s_cast<mla_size_t>(sent);
        } else {
            int err = SSL_get_error(ssl_ctx->ssl, sent);
            if (err == SSL_ERROR_WANT_WRITE || err == SSL_ERROR_WANT_READ) {
                fd_set write_set;
                FD_ZERO(&write_set);
                FD_SET(ssl_ctx->sock, &write_set);
                struct timeval timeout;
                timeout.tv_sec = 5;
                timeout.tv_usec = 0;
                if (err == SSL_ERROR_WANT_WRITE) {
                    if (select(ssl_ctx->sock + 1, nullptr, &write_set, nullptr, &timeout) <= 0) {
                        break;
                    }
                } else {
                    if (select(ssl_ctx->sock + 1, &write_set, nullptr, nullptr, &timeout) <= 0) {
                        break;
                    }
                }
                continue;
            }
            break;
        }
    }

    return total_sent;
}
#endif

mla_bool_t mla_linux_resolve_host(mla_network_host_t &host, const mla_string_t &hostname, mla_uint16_t port) {
    struct addrinfo hints = {
        0,
        AF_UNSPEC,    // IPv4 or IPv6
        SOCK_STREAM,
        IPPROTO_TCP,
        0,
        nullptr,
        nullptr,
        nullptr
    };

    mla_c_string_t cHostName = mla_string_to_cString(hostname);
    const mla_char_t* cHostName_c_str = mla_c_string_data(cHostName);

    if (cHostName_c_str == nullptr) {
        return false;
    }

    struct addrinfo *result = nullptr;
    if (getaddrinfo(cHostName_c_str, nullptr, &hints, &result) != 0) {
        return false;
    }

    // Extract IP address from first result
    if (result->ai_family == AF_INET) {
        struct sockaddr_in *addr = mla_r_cast<struct sockaddr_in *>(result->ai_addr);
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(addr->sin_addr), ip, INET_ADDRSTRLEN);

        host.address.address = mla_string_copy(ip, mla_strlen(ip));
        host.address.is_ipv6 = false;
        host.port = port;
    } else if (result->ai_family == AF_INET6) {
        struct sockaddr_in6 *addr = mla_r_cast<struct sockaddr_in6 *>(result->ai_addr);
        char ip[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &(addr->sin6_addr), ip, INET6_ADDRSTRLEN);

        host.address.address = mla_string_copy(ip, mla_strlen(ip));
        host.address.is_ipv6 = true;
        host.port = port;
    }

    freeaddrinfo(result);
    return true;
}

void mla_linux_socket_cleanup(const mla_dynamic_data_t& userData) {

    int sock = mla_s_cast<int>(userData.asInt32);
    if (sock >= 0) {
        close(sock);
    }
}

mla_size_t mla_linux_socket_read(mla_stream_input_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {
    (void)offset;
    mla_dynamic_data_t socket_data = mla_user_data_get_native_resource(input.userdata, mla_private_network_connection_user_data_id());
    int sock = socket_data.asInt32;
    if (sock < 0) {
        return 0;
    }

    ssize_t bytesRead = recv(sock, mla_r_cast<char*>(buffer) + offset, length, 0);
    if (bytesRead <= 0) {
        return 0;
    }

    return mla_s_cast<mla_size_t>(bytesRead);
}

mla_size_t mla_linux_socket_remaining_bytes(mla_stream_input_t& input) {
    mla_dynamic_data_t socket_data = mla_user_data_get_native_resource(input.userdata, mla_private_network_connection_user_data_id());
    int sock = socket_data.asInt32;
    if (sock < 0) {
        return 0;
    }

    int pending = 0;
    if (ioctl(sock, FIONREAD, &pending) == 0) {
        if (pending > 0) {
            return mla_size_max;
        }
    }

    return 0;
}

mla_size_t mla_linux_socket_write(mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {
    (void)offset;
    mla_dynamic_data_t socket_data = mla_user_data_get_native_resource(output.userdata, mla_private_network_connection_user_data_id());
    int sock = socket_data.asInt32;
    if (sock < 0) {
        return 0;
    }

    mla_size_t total_sent = 0;
    mla_size_t bytes_remaining = length;
    const char* ptr = mla_r_cast<const char*>(buffer) + offset;

    while (bytes_remaining > 0) {
        // MSG_NOSIGNAL prevents SIGPIPE if the other end closes the connection
        ssize_t sent = send(sock, ptr + total_sent, bytes_remaining, MSG_NOSIGNAL);

        if (sent > 0) {
            total_sent += mla_s_cast<mla_size_t>(sent);
            bytes_remaining -= mla_s_cast<mla_size_t>(sent);
        } else {
            if (sent < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                // Socket buffer is full; wait for it to become writable
                fd_set write_set;
                FD_ZERO(&write_set);
                FD_SET(sock, &write_set);

                // Select waits until the socket is writable again
                struct timeval timeout;
                timeout.tv_sec = 5; // 5 second safety timeout
                timeout.tv_usec = 0;
                if (select(sock + 1, nullptr, &write_set, nullptr, &timeout) <= 0) {
                    if (errno == EINTR) { continue; }
                    break; // Select error or timeout
                }
                continue;
            } else if (sent < 0 && errno == EINTR) {
                continue; // Interrupted by signal, retry
            }

            // Fatal error (e.g., connection reset, pipe broken)
            break;
        }
    }

    return total_sent;
}

mla_bool_t mla_linux_connect(mla_network_connection_t &connection, const mla_network_host_t &host,
                             mla_connection_type_t type, mla_size_t timeout_ms) {
    connection.host = host;

    // Create socket
    int sockType = (type == mla_connection_type_tcp) ? SOCK_STREAM : SOCK_DGRAM;
    int protocol = (type == mla_connection_type_tcp) ? IPPROTO_TCP : IPPROTO_UDP;
    int family = host.address.is_ipv6 ? AF_INET6 : AF_INET;

    int sock = socket(family, sockType, protocol);
    if (sock < 0) {
        return false;
    }

    // Set socket to non-blocking mode for timeout support
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    // Prepare address structure
    struct sockaddr_storage addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t addrLen;

    mla_c_string_t cAddress = mla_string_to_cString(host.address.address);
    const mla_char_t* cAddress_c_str = mla_c_string_data(cAddress);


    if (cAddress_c_str == nullptr) {
        close(sock);
        return false;
    }

    if (host.address.is_ipv6) {
        struct sockaddr_in6 *addr6 = mla_r_cast<struct sockaddr_in6*>(&addr);
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = htons(host.port);
        inet_pton(AF_INET6, cAddress_c_str, &addr6->sin6_addr);
        addrLen = sizeof(struct sockaddr_in6);
    } else {
        struct sockaddr_in *addr4 = mla_r_cast<struct sockaddr_in*>(&addr);
        addr4->sin_family = AF_INET;
        addr4->sin_port = htons(host.port);
        inet_pton(AF_INET, cAddress_c_str, &addr4->sin_addr);
        addrLen = sizeof(struct sockaddr_in);
    }

    // Attempt connection
    int result = connect(sock, mla_r_cast<struct sockaddr*>(&addr), addrLen);

    if (result < 0) {
        if (errno == EINPROGRESS) {
            // Wait for connection with timeout
            fd_set writeSet;
            FD_ZERO(&writeSet);
            FD_SET(sock, &writeSet);

            struct timeval timeout = {0, 0};
            timeout.tv_sec = timeout_ms / 1000;
            timeout.tv_usec = mla_s_cast<long>(timeout_ms % 1000) * 1000;

            result = select(sock + 1, nullptr, &writeSet, nullptr, &timeout);
            if (result <= 0) {
                close(sock);
                return false;
            }

            // Check if connection was successful
            int error = 0;
            socklen_t len = sizeof(error);
            if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error != 0) {
                close(sock);
                return false;
            }
        } else {
            close(sock);
            return false;
        }
    }

    // Set socket back to blocking mode
    //fcntl(sock, F_SETFL, flags);

    // Disable Nagle's algorithm (TCP_NODELAY) by default for better responsiveness
    if (type == mla_connection_type_tcp) {
        int nodelay = 1;
        setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));
    }

    mla_user_data_t userData = mla_user_data_empty();
    mla_user_data_set_native_resource(userData, mla_private_network_connection_user_data_id(), mla_dynamic_data_from_int32(sock), mla_linux_socket_cleanup);

    connection.inputStream = {
        userData,
        mla_linux_socket_read,
        mla_linux_socket_remaining_bytes
    };

    connection.outputStream = {
        userData,
        mla_linux_socket_write,
        nullptr
    };

    return true;
}

mla_bool_t mla_linux_connect_secure(
        mla_network_connection_t &connection,
        const mla_network_host_t &host,
        mla_connection_type_t type,
        mla_size_t timeout_ms,
        const mla_network_security_config_t &security_config) {
    if (mla_network_security_config_get_mode(security_config) == mla_network_security_mode_insecure) {
        return mla_linux_connect(connection, host, type, timeout_ms);
    }

#if defined(MLA_HAS_OPENSSL) && MLA_HAS_OPENSSL == 1
    if (type != mla_connection_type_tcp) {
        return false;
    }

    if (!mla_linux_connect(connection, host, type, timeout_ms)) {
        return false;
    }

    mla_dynamic_data_t socket_data = mla_user_data_get_native_resource(connection.inputStream.userdata, mla_private_network_connection_user_data_id());
    int sock = socket_data.asInt32;
    if (sock < 0) {
        mla_network_connection_disconnect(connection);
        return false;
    }

    SSL_CTX* ssl_ctx = mla_linux_get_client_ssl_ctx();
    if (ssl_ctx == nullptr) {
        mla_network_connection_disconnect(connection);
        return false;
    }

    SSL* ssl = SSL_new(ssl_ctx);
    if (ssl == nullptr) {
        mla_network_connection_disconnect(connection);
        return false;
    }

    SSL_set_fd(ssl, sock);

    mla_network_tls_config_t tls_config = mla_network_security_config_get_tls_config(security_config);
    mla_string_t server_name = mla_network_tls_config_get_server_name(tls_config);
    if (mla_string_is_empty(server_name)) {
        server_name = host.address.address;
    }

    if (!mla_string_is_empty(server_name)) {
        mla_c_string_t c_server_name = mla_string_to_cString(server_name);
        const mla_char_t* c_str = mla_c_string_data(c_server_name);
        if (c_str != nullptr) {
            SSL_set_tlsext_host_name(ssl, c_str);
            if (mla_network_tls_config_get_verify_host_name(tls_config)) {
                SSL_set1_host(ssl, c_str);
            }
        }
    }

    if (mla_network_tls_config_get_verify_peer(tls_config)) {
        SSL_set_verify(ssl, SSL_VERIFY_PEER, nullptr);
    } else {
        SSL_set_verify(ssl, SSL_VERIFY_NONE, nullptr);
    }

    mla_string_t ca_cert = mla_network_tls_config_get_ca_certificate(tls_config);
    if (!mla_string_is_empty(ca_cert)) {
        mla_c_string_t c_ca_cert = mla_string_to_cString(ca_cert);
        const mla_char_t* ca_str = mla_c_string_data(c_ca_cert);
        if (ca_str != nullptr) {
            BIO* bio = BIO_new_mem_buf(ca_str, -1);
            if (bio != nullptr) {
                X509* cert = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr);
                if (cert != nullptr) {
                    X509_STORE* store = SSL_CTX_get_cert_store(ssl_ctx);
                    if (store != nullptr) {
                        X509_STORE_add_cert(store, cert);
                    }
                    X509_free(cert);
                }
                BIO_free(bio);
            }
        }
    }

    mla_string_t client_cert = mla_network_tls_config_get_certificate(tls_config);
    mla_string_t client_key = mla_network_tls_config_get_private_key(tls_config);
    if (!mla_string_is_empty(client_cert) && !mla_string_is_empty(client_key)) {
        mla_c_string_t c_client_cert = mla_string_to_cString(client_cert);
        mla_c_string_t c_client_key = mla_string_to_cString(client_key);
        const mla_char_t* p_cert = mla_c_string_data(c_client_cert);
        const mla_char_t* p_key = mla_c_string_data(c_client_key);
        if (p_cert != nullptr && p_key != nullptr) {
            BIO* cert_bio = BIO_new_mem_buf(p_cert, -1);
            if (cert_bio != nullptr) {
                X509* cert = PEM_read_bio_X509(cert_bio, nullptr, nullptr, nullptr);
                if (cert != nullptr) {
                    SSL_use_certificate(ssl, cert);
                    X509_free(cert);
                }
                BIO_free(cert_bio);
            }
            BIO* key_bio = BIO_new_mem_buf(p_key, -1);
            if (key_bio != nullptr) {
                EVP_PKEY* pkey = PEM_read_bio_PrivateKey(key_bio, nullptr, nullptr, nullptr);
                if (pkey != nullptr) {
                    SSL_use_PrivateKey(ssl, pkey);
                    EVP_PKEY_free(pkey);
                }
                BIO_free(key_bio);
            }
        }
    }

    mla_bool_t handshake_done = false;
    mla_size_t elapsed_ms = 0;
    while (!handshake_done && elapsed_ms < timeout_ms) {
        int ret = SSL_connect(ssl);
        if (ret == 1) {
            handshake_done = true;
            break;
        }

        int err = SSL_get_error(ssl, ret);
        if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
            fd_set rset;
            fd_set wset;
            FD_ZERO(&rset);
            FD_ZERO(&wset);
            if (err == SSL_ERROR_WANT_READ) {
                FD_SET(sock, &rset);
            } else {
                FD_SET(sock, &wset);
            }

            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 50000;
            int sret = select(sock + 1, &rset, &wset, nullptr, &tv);
            if (sret < 0 && errno != EINTR) {
                break;
            }
            elapsed_ms += 50;
        } else {
            break;
        }
    }

    if (!handshake_done) {
        SSL_free(ssl);
        mla_network_connection_disconnect(connection);
        return false;
    }

    mla_linux_ssl_context_t* ssl_conn_ctx = mla_r_cast<mla_linux_ssl_context_t*>(mla_platform_malloc(sizeof(mla_linux_ssl_context_t)));
    if (ssl_conn_ctx == nullptr) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
        mla_network_connection_disconnect(connection);
        return false;
    }

    ssl_conn_ctx->sock = sock;
    ssl_conn_ctx->ssl = ssl;

    // Disown raw socket cleanup from old input and output streams so they don't close the socket
    mla_pointer_t in_ptr = mla_user_data_get_pointer(connection.inputStream.userdata, mla_private_network_connection_user_data_id());
    mla_native_resource_t* in_res = mla_native_resource_from_managed_pointer(in_ptr);
    if (in_res != nullptr) {
        in_res->asInt32 = -1;
    }

    mla_pointer_t out_ptr = mla_user_data_get_pointer(connection.outputStream.userdata, mla_private_network_connection_user_data_id());
    mla_native_resource_t* out_res = mla_native_resource_from_managed_pointer(out_ptr);
    if (out_res != nullptr) {
        out_res->asInt32 = -1;
    }

    mla_user_data_t ssl_user_data = mla_user_data_empty();
    mla_user_data_set_native_resource(ssl_user_data, mla_private_network_ssl_connection_user_data_id(), mla_dynamic_data_from_pointer(ssl_conn_ctx), mla_linux_ssl_socket_cleanup);

    connection.inputStream = {
        ssl_user_data,
        mla_linux_ssl_socket_read,
        mla_linux_ssl_socket_remaining_bytes
    };

    connection.outputStream = {
        ssl_user_data,
        mla_linux_ssl_socket_write,
        nullptr
    };

    return true;
#else
    (void)connection;
    (void)host;
    (void)type;
    (void)timeout_ms;
    (void)security_config;
    return false;
#endif
}

mla_bool_t mla_linux_accept_connection(const mla_network_listener_t& listener, mla_network_connection_t &connection) {
    mla_dynamic_data_t socket_data = mla_user_data_get_native_resource(listener.userdata, mla_private_network_connection_user_data_id());
    int listenSock = socket_data.asInt32;
    if (listenSock < 0) {
        return false;
    }

    int sockType = 0;
    socklen_t optLen = sizeof(sockType);
    if (getsockopt(listenSock, SOL_SOCKET, SO_TYPE, &sockType, &optLen) != 0 || sockType != SOCK_STREAM) {
        return false;
    }

    struct sockaddr_storage clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    int clientSock = accept(listenSock, mla_r_cast<struct sockaddr*>(&clientAddr), &clientLen);
    if (clientSock < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No pending connection; non-blocking accept
            return false;
        }
        return false;
    }

    // Set accepted socket to non blocking mode
    int flags = fcntl(clientSock, F_GETFL, 0);
    fcntl(clientSock, F_SETFL, flags | O_NONBLOCK);

    // Disable Nagle's algorithm (TCP_NODELAY) by default for better responsiveness
    int nodelay = 1;
    setsockopt(clientSock, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));

    // Fill connection.host from peer address
    mla_network_host_t peer = mla_network_host_invalid();
    char ip4[INET_ADDRSTRLEN] = {0};
    char ip6[INET6_ADDRSTRLEN] = {0};

    if (clientAddr.ss_family == AF_INET) {
        struct sockaddr_in* a4 = mla_r_cast<struct sockaddr_in*>(&clientAddr);
        inet_ntop(AF_INET, &a4->sin_addr, ip4, sizeof(ip4));
        peer.address.address = mla_string_copy(ip4, mla_strlen(ip4));
        peer.address.is_ipv6 = false;
        peer.port = ntohs(a4->sin_port);
    } else if (clientAddr.ss_family == AF_INET6) {
        struct sockaddr_in6* a6 = mla_r_cast<struct sockaddr_in6*>(&clientAddr);
        inet_ntop(AF_INET6, &a6->sin6_addr, ip6, sizeof(ip6));
        peer.address.address = mla_string_copy(ip6, mla_strlen(ip6));
        peer.address.is_ipv6 = true;
        peer.port = ntohs(a6->sin6_port);
    } else {
        close(clientSock);
        return false;
    }

    connection.host = peer;

    mla_user_data_t userData = mla_user_data_empty();
    mla_user_data_set_native_resource(userData, mla_private_network_connection_user_data_id(), mla_dynamic_data_from_int32(clientSock), mla_linux_socket_cleanup);

    connection.inputStream = {
        userData,
        mla_linux_socket_read,
        mla_linux_socket_remaining_bytes
    };

    connection.outputStream = {
        userData,
        mla_linux_socket_write,
        nullptr
    };

    return true;
}

mla_bool_t mla_linux_bind_and_listen(mla_network_listener_t &listener, const mla_network_host_t &host, mla_connection_type_t type) {
    listener.host = host;

    int family = host.address.is_ipv6 ? AF_INET6 : AF_INET;
    int sockType = (type == mla_connection_type_tcp) ? SOCK_STREAM : SOCK_DGRAM;
    int protocol = (type == mla_connection_type_tcp) ? IPPROTO_TCP : IPPROTO_UDP;

    int sock = socket(family, sockType, protocol);
    if (sock < 0) {
        return false;
    }

    // Enable address reuse
    int reuseAddr = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr));

    // Allow dual-stack for IPv6
    if (family == AF_INET6) {
        int v6only = 0;
        setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &v6only, sizeof(v6only));
    }

    struct sockaddr_storage ss;
    mla_memset(&ss, 0, sizeof(ss));
    socklen_t addrLen = 0;

    mla_c_string_t cAddress = mla_string_to_cString(host.address.address);
    const mla_char_t* cAddress_c_str = mla_c_string_data(cAddress);

    if (cAddress_c_str == nullptr) {
        close(sock);
        return false;
    }

    if (host.address.is_ipv6) {
        struct sockaddr_in6* addr6 = mla_r_cast<struct sockaddr_in6*>(&ss);
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = htons(host.port);
        mla_bool_t ok = (inet_pton(AF_INET6, cAddress_c_str, &addr6->sin6_addr) == 1);
        if (!ok) {
            addr6->sin6_addr = in6addr_any;
        }
        addrLen = sizeof(struct sockaddr_in6);
    } else {
        struct sockaddr_in* addr4 = mla_r_cast<struct sockaddr_in*>(&ss);
        addr4->sin_family = AF_INET;
        addr4->sin_port = htons(host.port);
        mla_bool_t ok = (inet_pton(AF_INET, cAddress_c_str, &addr4->sin_addr) == 1);
        if (!ok) {
            addr4->sin_addr.s_addr = htonl(INADDR_ANY);
        }
        addrLen = sizeof(struct sockaddr_in);
    }

    if (bind(sock, mla_r_cast<struct sockaddr*>(&ss), addrLen) < 0) {
        close(sock);
        return false;
    }

    if (type == mla_connection_type_tcp) {
        if (listen(sock, SOMAXCONN) < 0) {
            close(sock);
            return false;
        }

        // Make the listening socket non-blocking so accept() will not block
        int flags = fcntl(sock, F_GETFL, 0);
        fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    }

    mla_user_data_t userData = mla_user_data_empty();
    mla_user_data_set_native_resource(userData, mla_private_network_connection_user_data_id(), mla_dynamic_data_from_int32(sock), mla_linux_socket_cleanup);

    listener.accept_connection = mla_linux_accept_connection;
    listener.userdata = userData;

    return true;
}

mla_bool_t mla_linux_bind_and_listen_secure(
        mla_network_listener_t &listener,
        const mla_network_host_t &host,
        mla_connection_type_t type,
        const mla_network_security_config_t &security_config) {
    if (mla_network_security_config_get_mode(security_config) == mla_network_security_mode_insecure) {
        return mla_linux_bind_and_listen(listener, host, type);
    }

    return false;
}

mla_array_list_t<mla_init_struct(mla_network_ip_address_t)> mla_linux_get_local_ip_addresses() {
    mla_array_list_t<mla_init_struct(mla_network_ip_address_t)> local_ip_addresses = mla_array_list_empty<mla_init_struct(mla_network_ip_address_t)>();

    struct ifaddrs *ifaddr;
    struct ifaddrs *ifa;
    if (getifaddrs(&ifaddr) == -1) {
        return local_ip_addresses;
    }

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) {
            continue;
        }

        // Skip loopback interfaces and those that are down to match Windows behavior
        if ((ifa->ifa_flags & IFF_LOOPBACK) != 0U || (ifa->ifa_flags & IFF_UP) == 0U) {
            continue;
        }

        int family = ifa->ifa_addr->sa_family;
        if (family == AF_INET) {
            struct sockaddr_in *addr = mla_r_cast<struct sockaddr_in *>(ifa->ifa_addr);
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(addr->sin_addr), ip, INET_ADDRSTRLEN);

            mla_network_ip_address_t ip_address = mla_network_ip_address_invalid();
            ip_address.address = mla_string_copy(ip, mla_strlen(ip));
            ip_address.is_ipv6 = false;
            mla_array_list_add(local_ip_addresses, ip_address);

        } else if (family == AF_INET6) {
            struct sockaddr_in6 *addr = mla_r_cast<struct sockaddr_in6 *>(ifa->ifa_addr);
            char ip[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, &(addr->sin6_addr), ip, INET6_ADDRSTRLEN);

            mla_network_ip_address_t ip_address = mla_network_ip_address_invalid();
            ip_address.address = mla_string_copy(ip, mla_strlen(ip));
            ip_address.is_ipv6 = true;
            mla_array_list_add(local_ip_addresses, ip_address);
        }
    }

    freeifaddrs(ifaddr);
    return local_ip_addresses;
}

mla_network_low_level_operations_t g_network_low_level_operations = {
    mla_linux_resolve_host,
    mla_linux_connect,
    mla_linux_connect_secure,
    mla_linux_bind_and_listen,
    mla_linux_bind_and_listen_secure,
    mla_linux_get_local_ip_addresses
};


#endif