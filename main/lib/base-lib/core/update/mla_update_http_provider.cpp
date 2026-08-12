//
// Created for auto-update package (HTTP Update Provider).
//

#include "mla_update.h"
#include "../http/mla_http_client.h"
#include "../http/mla_http_request.h"
#include "../http/mla_http_response.h"
#include "../system/mla_string_concat.h"

mla_user_data_id_init(mla_update_provider_url_id)
mla_user_data_id_init(mla_update_provider_module_id)
mla_user_data_id_init(mla_update_provider_platform_id)
mla_user_data_id_init(mla_update_provider_compiler_id)

mla_string_t mla_update_get_current_platform() {
#ifdef MLA_PLATFORM_NAME
    return mla_string_const(MLA_PLATFORM_NAME);
#elif defined(__EMSCRIPTEN__) || defined(MLA_WASM) || defined(__wasm__)
    return mla_string_const("wasm");
#elif defined(_WIN32) || defined(_WIN64)
    return mla_string_const("windows64");
#elif defined(__APPLE__) || defined(__DARWIN__)
  #if defined(__x86_64__) || defined(_M_X64)
    return mla_string_const("darwin_x86_64");
  #elif defined(__aarch64__) || defined(__arm64__)
    return mla_string_const("darwin_arm64");
  #else
    return mla_string_const("darwin");
  #endif
#elif defined(__linux__)
  #if defined(__x86_64__) || defined(_M_X64)
    return mla_string_const("linux_x86_64");
  #elif defined(__aarch64__)
    return mla_string_const("linux_aarch64");
  #else
    return mla_string_const("linux");
  #endif
#else
    return mla_string_const("unknown");
#endif
}

mla_string_t mla_update_get_current_compiler() {
#ifdef MLA_COMPILER_NAME
    return mla_string_const(MLA_COMPILER_NAME);
#elif defined(__filc__) || defined(__FILC__)
    return mla_string_const("filc");
#elif defined(__EMSCRIPTEN__)
  #if defined(MLA_JS_STANDALONE)
    return mla_string_const("emscripten_js");
  #else
    return mla_string_const("emscripten_std");
  #endif
#elif defined(__ZIG__) || defined(ZIG_COMPILER)
  #if defined(__wasm__) || defined(MLA_WASM_STANDALONE)
    return mla_string_const("zig_wasm");
  #else
    return mla_string_const("zig_native");
  #endif
#elif defined(__clang__)
    return mla_string_const("clang");
#elif defined(__GNUC__)
    return mla_string_const("gcc");
#elif defined(_MSC_VER)
    return mla_string_const("msvc");
#else
    return mla_string_const("unknown");
#endif
}

mla_string_t mla_private_update_extract_latest_version(const mla_string_t& p_Content) {
    mla_string_t trimmed = mla_string_trim(p_Content);
    if (mla_string_length(trimmed) > 0 && !mla_string_contains(trimmed, mla_string_const("<")) && !mla_string_contains(trimmed, mla_string_const(" "))) {
        return trimmed;
    }

    mla_size_t len = mla_string_length(p_Content);
    const mla_char_t* data = mla_string_data(p_Content);
    mla_string_t best_version = mla_string_empty();
    mla_size_t i = 0;

    while (i < len) {
        if (data[i] >= '0' && data[i] <= '9') {
            mla_size_t start = i;
            mla_uint32_t dot_count = 0;
            while (i < len && ((data[i] >= '0' && data[i] <= '9') || data[i] == '.')) {
                if (data[i] == '.') {
                    dot_count++;
                }
                i++;
            }
            if (dot_count >= 1 && (i - start) >= 3) {
                mla_string_t candidate = mla_string_substr(p_Content, start, i - start);
                if (mla_string_length(best_version) == 0 || mla_string_compare(candidate, best_version) > 0) {
                    best_version = candidate;
                }
            }
        } else {
            i++;
        }
    }

    return best_version;
}

mla_bool_t mla_private_update_http_get_last_version(const mla_update_provider_t& p_Provider, mla_string_t& p_OutVersion) {
#if !defined(mla_test_disable_network) || mla_test_disable_network != 1
    mla_string_t base_url = mla_user_data_get_string(p_Provider.user_data, mla_update_provider_url_id);
    mla_string_t module_name = mla_user_data_get_string(p_Provider.user_data, mla_update_provider_module_id);

    if (mla_string_length(base_url) == 0) {
        return false;
    }

    mla_string_t prefix = base_url;
    if (!mla_string_ends_with(prefix, mla_string_const("/"))) {
        prefix = mla_string_concat(prefix, mla_string_const("/"));
    }

    // Try direct module version endpoint: ${base_url}/${module}/version
    mla_string_t url_version = prefix;
    if (mla_string_length(module_name) > 0) {
        url_version = mla_string_concat(url_version, module_name);
        url_version = mla_string_concat(url_version, mla_string_const("/version"));
    } else {
        url_version = mla_string_concat(url_version, mla_string_const("version"));
    }

    mla_http_request_t request = mla_http_get_request(url_version);
    mla_http_client_t client = mla_http_client();
    mla_http_client_set_timeout(client, 5000);
    mla_http_client_response_t client_res = mla_http_client_send_request(client, request);

    if (client_res.status == MLA_HTTP_CLIENT_RESPONSE_STATUS_OK && client_res.response.statusCode == 200) {
        mla_stream_input_t content_stream = client_res.response.content;
        p_OutVersion = mla_string_from_stream(content_stream, 10000);
        return true;
    }

    // Try root version endpoint fallback: ${base_url}/version
    if (mla_string_length(module_name) > 0) {
        mla_string_t url_root_version = mla_string_concat(prefix, mla_string_const("version"));
        request = mla_http_get_request(url_root_version);
        client_res = mla_http_client_send_request(client, request);
        if (client_res.status == MLA_HTTP_CLIENT_RESPONSE_STATUS_OK && client_res.response.statusCode == 200) {
            mla_stream_input_t content_stream = client_res.response.content;
            p_OutVersion = mla_string_from_stream(content_stream, 10000);
            return true;
        }
    }

    // Try release directory listing endpoint: ${base_url}/${module}/?raw=true
    mla_string_t url_listing = prefix;
    if (mla_string_length(module_name) > 0) {
        url_listing = mla_string_concat(url_listing, module_name);
        url_listing = mla_string_concat(url_listing, mla_string_const("/?raw=true"));
    } else {
        url_listing = mla_string_concat(url_listing, mla_string_const("?raw=true"));
    }

    request = mla_http_get_request(url_listing);
    client_res = mla_http_client_send_request(client, request);
    if (client_res.status == MLA_HTTP_CLIENT_RESPONSE_STATUS_OK && client_res.response.statusCode == 200) {
        mla_stream_input_t content_stream = client_res.response.content;
        mla_string_t raw_content = mla_string_from_stream(content_stream, 100000);
        mla_string_t extracted = mla_private_update_extract_latest_version(raw_content);
        if (mla_string_length(extracted) > 0) {
            p_OutVersion = extracted;
            return true;
        }
    }

    return false;
#else
    (void)p_Provider;
    (void)p_OutVersion;
    return false;
#endif
}

mla_bool_t mla_private_update_http_get_binary_content(const mla_update_provider_t& p_Provider, const mla_string_t& p_Version, mla_stream_input_t& p_OutStream) {
#if !defined(mla_test_disable_network) || mla_test_disable_network != 1
    mla_string_t base_url = mla_user_data_get_string(p_Provider.user_data, mla_update_provider_url_id);
    mla_string_t module_name = mla_user_data_get_string(p_Provider.user_data, mla_update_provider_module_id);
    mla_string_t platform = mla_user_data_get_string(p_Provider.user_data, mla_update_provider_platform_id);
    mla_string_t compiler = mla_user_data_get_string(p_Provider.user_data, mla_update_provider_compiler_id);

    if (mla_string_length(base_url) == 0) {
        return false;
    }

    mla_string_t prefix = base_url;
    if (!mla_string_ends_with(prefix, mla_string_const("/"))) {
        prefix = mla_string_concat(prefix, mla_string_const("/"));
    }

    // Candidate 1: ${base_url}/${module}/${version}/${platform}/${compiler}/app
    if (mla_string_length(module_name) > 0 && mla_string_length(platform) > 0 && mla_string_length(compiler) > 0) {
        mla_string_t release_url = prefix;
        release_url = mla_string_concat(release_url, module_name);
        release_url = mla_string_concat(release_url, mla_string_const("/"));
        release_url = mla_string_concat(release_url, p_Version);
        release_url = mla_string_concat(release_url, mla_string_const("/"));
        release_url = mla_string_concat(release_url, platform);
        release_url = mla_string_concat(release_url, mla_string_const("/"));
        release_url = mla_string_concat(release_url, compiler);
        release_url = mla_string_concat(release_url, mla_string_const("/app"));

        mla_http_request_t request = mla_http_get_request(release_url);
        mla_http_client_t client = mla_http_client();
        mla_http_client_set_timeout(client, 5000);
        mla_http_client_response_t client_res = mla_http_client_send_request(client, request);

        if (client_res.status == MLA_HTTP_CLIENT_RESPONSE_STATUS_OK && client_res.response.statusCode == 200) {
            p_OutStream = client_res.response.content;
            return true;
        }
    }

    // Candidate 2: ${base_url}/${module}/binary
    if (mla_string_length(module_name) > 0) {
        mla_string_t module_binary_url = prefix;
        module_binary_url = mla_string_concat(module_binary_url, module_name);
        module_binary_url = mla_string_concat(module_binary_url, mla_string_const("/binary"));

        mla_http_request_t request = mla_http_get_request(module_binary_url);
        mla_http_client_t client = mla_http_client();
        mla_http_client_set_timeout(client, 5000);
        mla_http_client_response_t client_res = mla_http_client_send_request(client, request);

        if (client_res.status == MLA_HTTP_CLIENT_RESPONSE_STATUS_OK && client_res.response.statusCode == 200) {
            p_OutStream = client_res.response.content;
            return true;
        }
    }

    // Candidate 3: ${base_url}/binary (legacy/mock fallback)
    mla_string_t legacy_url = mla_string_concat(prefix, mla_string_const("binary"));
    mla_http_request_t request = mla_http_get_request(legacy_url);
    mla_http_client_t client = mla_http_client();
    mla_http_client_set_timeout(client, 5000);
    mla_http_client_response_t client_res = mla_http_client_send_request(client, request);

    if (client_res.status == MLA_HTTP_CLIENT_RESPONSE_STATUS_OK && client_res.response.statusCode == 200) {
        p_OutStream = client_res.response.content;
        return true;
    }

    return false;
#else
    (void)p_Provider;
    (void)p_Version;
    (void)p_OutStream;
    return false;
#endif
}

mla_update_provider_t mla_update_provider_http_create(
    const mla_string_t& p_Module,
    const mla_string_t& p_BaseUrl,
    const mla_string_t& p_Platform,
    const mla_string_t& p_Compiler
) {
    mla_update_provider_t provider = {
        mla_user_data_empty(),
        mla_private_update_http_get_last_version,
        mla_private_update_http_get_binary_content
    };

    mla_string_t module_copy = p_Module;
    mla_string_t url_copy = p_BaseUrl;
    mla_string_t platform_copy = p_Platform;
    mla_string_t compiler_copy = p_Compiler;

    mla_user_data_set_string(provider.user_data, mla_update_provider_module_id, module_copy);
    mla_user_data_set_string(provider.user_data, mla_update_provider_url_id, url_copy);
    mla_user_data_set_string(provider.user_data, mla_update_provider_platform_id, platform_copy);
    mla_user_data_set_string(provider.user_data, mla_update_provider_compiler_id, compiler_copy);

    return provider;
}
