//
// Created by christian on 10/6/25.
//

#ifndef MLA_HTTP_HEADER_H
#define MLA_HTTP_HEADER_H

#include "../system/mla_string.h"
#include "../system/mla_array_list.h"

enum mla_http_header_type_t: mla_uint8_t {
    MLA_HTTP_HEADER_TYPE_SINGLE, // Single value header
    MLA_HTTP_HEADER_TYPE_MULTI // Multi value header (e.g., Set-Cookie)
};

struct mla_http_header_t {
    mla_string_t name;
    mla_http_header_type_t type;
    mla_array_list_t<mla_init_struct(mla_string_t)> values; // Multiple values
    mla_string_t value; // Single value

    static mla_http_header_t init() {
        return { mla_string_empty(), MLA_HTTP_HEADER_TYPE_SINGLE, mla_array_list_empty<mla_init_struct(mla_string_t)>(), mla_string_empty() };
    }
};

mla_http_header_t mla_http_header_empty();

void mla_http_headers_add(mla_array_list_t<mla_init_struct(mla_http_header_t)> &p_Headers, const mla_string_t &p_Name, const mla_string_t &p_Value);
void mla_http_headers_add(mla_array_list_t<mla_init_struct(mla_http_header_t)> &p_Headers, const mla_string_t &p_Name, const mla_string_t &p_Value, const mla_string_t &value_seperator);

mla_string_t mla_http_headers_get_value(const mla_array_list_t<mla_init_struct(mla_http_header_t)> &p_Headers, const mla_string_t &p_Name);

mla_array_list_t<mla_init_struct(mla_string_t)> mla_http_headers_get_values(const mla_array_list_t<mla_init_struct(mla_http_header_t)> &p_Headers, const mla_string_t &p_Name);

mla_bool_t mla_http_headers_has_header_value(const mla_array_list_t<mla_init_struct(mla_http_header_t)> &p_Headers, const mla_string_t &p_Name, const mla_string_t &p_Value);

mla_bool_t mla_http_headers_has_header_value(const mla_array_list_t<mla_init_struct(mla_http_header_t)> &p_Headers, const mla_string_t &p_Name, const mla_string_t &p_Value, const mla_string_t &value_seperator);


#endif
