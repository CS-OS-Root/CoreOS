//
// Created by christian on 10/6/25.
//

#ifndef MLA_HTTP_RESPONSE_H
#define MLA_HTTP_RESPONSE_H

#include "../mla_data_types.h"
#include "mla_http_header.h"
#include "../system/mla_stream.h"
#include "../system/mla_string.h"
#include "mla_http_data_types.h"

// Http Status codes
#define mla_http_status_switching_protocols ((mla_uint16_t)101)
#define mla_http_status_ok ((mla_uint16_t)200)
#define mla_http_status_created ((mla_uint16_t)201)
#define mla_http_status_accepted ((mla_uint16_t)202)
#define mla_http_status_no_content ((mla_uint16_t)204)
#define mla_http_status_moved_permanently ((mla_uint16_t)301)
#define mla_http_status_found ((mla_uint16_t)302)
#define mla_http_status_not_modified ((mla_uint16_t)304)
#define mla_http_status_bad_request ((mla_uint16_t)400)
#define mla_http_status_unauthorized ((mla_uint16_t)401)
#define mla_http_status_forbidden ((mla_uint16_t)403)
#define mla_http_status_not_found ((mla_uint16_t)404)
#define mla_http_status_method_not_allowed ((mla_uint16_t)405)
#define mla_http_status_internal_server_error ((mla_uint16_t)500)
#define mla_http_status_not_implemented ((mla_uint16_t)501)
#define mla_http_status_bad_gateway ((mla_uint16_t)502)
#define mla_http_status_service_unavailable ((mla_uint16_t)503)
#define mla_http_status_gateway_timeout ((mla_uint16_t)504)

struct mla_http_response_content_writer_t;

typedef mla_bool_t (*mla_http_response_content_writer_func_t)(const mla_http_response_content_writer_t& writer, const mla_stream_output_t &outputStream);

struct mla_http_response_content_writer_t {
    mla_user_data_t userData;
    mla_http_response_content_writer_func_t writeTo;
};

inline mla_http_response_content_writer_t mla_http_response_content_writer_invalid() {
    return {
        mla_user_data_empty(),
        nullptr
    };
}

inline mla_http_response_content_writer_t mla_http_response_content_writer(
    const mla_user_data_t &userdata,
    const mla_http_response_content_writer_func_t &writer
) {
    return {
        userdata,
        writer
    };
}

mla_user_data_id_init(mla_http_response_content_writer_struct_user_data_id)

template<typename T>
mla_bool_t mla_http_response_content_writer_struct_execute(const mla_http_response_content_writer_t &writer, const mla_stream_output_t &outputStream) {
    T* writer_struct = mla_user_data_get_struct_data<T>(writer.userData, mla_http_response_content_writer_struct_user_data_id);

    if (writer_struct == nullptr) {
        return false;
    }

    return T::http_response_content_write(*writer_struct, writer, outputStream);
}


template<typename T>
mla_http_response_content_writer_t mla_http_response_content_writer_struct(mla_user_data_t &userdata, T& writer_struct) {
    if (!mla_user_data_set_struct(userdata, mla_http_response_content_writer_struct_user_data_id, writer_struct)) {
        return mla_http_response_content_writer_invalid();
    }

    return mla_http_response_content_writer(userdata, mla_http_response_content_writer_struct_execute<T>);
}



inline mla_bool_t mla_http_response_content_writer_is_valid(const mla_http_response_content_writer_t &writer) {
    return writer.writeTo != nullptr;
}

struct mla_http_response_t {
    mla_http_version version; // e.g., HTTP/1.1
    mla_uint16_t statusCode; // e.g., 200, 404, etc.
    mla_string_t statusMessage; // e.g., "OK", "Not Found", etc.
    mla_array_list_t<mla_init_struct(mla_http_header_t)> headers;
    mla_stream_input_t content; // Response body content
    mla_http_response_content_writer_t contentWriter; // Optional content writer for dynamic content
};

inline mla_http_response_t mla_http_response_empty() {
    return {MLA_HTTP_VERSION_1_0, 0, mla_string_empty(), mla_array_list_empty<mla_init_struct(mla_http_header_t)>(), mla_stream_noop_input(), mla_http_response_content_writer_invalid()};
}




#endif
