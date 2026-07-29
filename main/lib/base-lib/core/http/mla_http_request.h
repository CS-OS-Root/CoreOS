//
// Created by christian on 10/6/25.
//

#ifndef MLA_HTTP_REQUEST_H
#define MLA_HTTP_REQUEST_H

#include "../system/mla_string.h"
#include "../system/mla_array_list.h"
#include "mla_http_header.h"
#include "../system/mla_stream.h"
#include "mla_http_data_types.h"

struct mla_http_request_content_writer_t;

typedef mla_bool_t (*mla_http_request_content_writer_func_t)(const mla_http_request_content_writer_t& writer, const mla_stream_output_t &outputStream);

struct mla_http_request_content_writer_t {
    mla_user_data_t userData;
    mla_http_request_content_writer_func_t writeTo;
};

inline mla_http_request_content_writer_t mla_http_request_content_writer(
    const mla_user_data_t& userData,
    const mla_http_request_content_writer_func_t &writer
) {
    return {
        userData,
        writer
    };
}

inline mla_http_request_content_writer_t mla_http_request_content_writer_invalid() {
    return {
        mla_user_data_empty(),
        nullptr
    };
}

inline mla_bool_t mla_http_request_content_writer_is_valid(const mla_http_request_content_writer_t &writer) {
    return writer.writeTo != nullptr;
}


struct mla_http_request_t {
    mla_http_version version;
    mla_string_t url;
    mla_string_t method; // GET, POST, PUT, DELETE, etc.
    mla_array_list_t<mla_init_struct(mla_http_header_t)> headers;
    mla_stream_input_t  content; // Request body content
    mla_http_request_content_writer_t contentWriter; // Optional content writer for dynamic content
};

inline mla_http_request_t mla_http_request_empty() {
    return {
        MLA_HTTP_VERSION_1_0,
            mla_string_empty(),
            mla_string_empty(),
            mla_array_list_empty<mla_init_struct(mla_http_header_t)>(),
            mla_stream_noop_input(),
        mla_http_request_content_writer_invalid()
    };
}

inline mla_http_request_t mla_http_request(const mla_string_t &p_Url, const mla_string_t &p_Method) {
    return {
        MLA_HTTP_VERSION_1_1,
            p_Url,
            p_Method,
            mla_array_list_empty<mla_init_struct(mla_http_header_t)>(),
            mla_stream_noop_input(),
        mla_http_request_content_writer_invalid()
    };
}

////////////////////////////////////////////////////////////////
/// Helpers
////////////////////////////////////////////////////////////////

#define mla_http_method_get mla_string_const("GET")
#define mla_http_method_post mla_string_const("POST")
#define mla_http_method_put mla_string_const("PUT")
#define mla_http_method_delete mla_string_const("DELETE")
#define mla_http_method_options mla_string_const("OPTIONS")

inline mla_http_request_t mla_http_get_request(const mla_string_t &p_Url) {
    return mla_http_request(p_Url, mla_http_method_get);
}

inline mla_http_request_t mla_http_post_request(const mla_string_t &p_Url) {
    return mla_http_request(p_Url, mla_http_method_post);
}

#endif
