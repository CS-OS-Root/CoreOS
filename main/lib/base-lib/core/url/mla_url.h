//
// Created by christian on 10/8/25.
//

#ifndef MLA_URL_H
#define MLA_URL_H

#include "../system/mla_array_list.h"
#include "../system/mla_string.h"

struct mla_url_query_param_t {
    mla_string_t key;
    mla_string_t value;

    static mla_url_query_param_t init() {
        return { mla_string_empty(), mla_string_empty() };
    }
};

mla_url_query_param_t mla_url_query_param_empty();

struct mla_url_t {
    mla_string_t scheme; // e.g., "http", "https"
    mla_string_t host;   // e.g., "www.example.com"
    mla_uint16_t port;   // e.g., 80, 443
    mla_string_t path;   // e.g., "/path/to/resource"
    mla_array_list_t<mla_init_struct(mla_url_query_param_t)> query;  // e.g., "key1=value1&key2=value2"
    mla_string_t fragment; // e.g., "section1"
};

mla_url_t mla_url_empty();

mla_bool_t mla_url_parse(const mla_string_t &urlString, mla_url_t &outUrl);
mla_bool_t mla_url_parse_pathAndQuery(const mla_string_t &pathAndQueryString, mla_url_t &outUrl);
mla_string_t mla_url_to_string(const mla_url_t &url);
mla_string_t mla_url_to_string_pathAndQuery(const mla_url_t &url);

#endif
