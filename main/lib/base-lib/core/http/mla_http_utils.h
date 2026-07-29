//
// Created by chris on 10/23/2025.
//

#ifndef MLA_HTTP_UTILS_H
#define MLA_HTTP_UTILS_H

#include "../system/mla_stream.h"
#include "mla_http_header.h"
#include "mla_http_data_types.h"

mla_bool_t mla_http_utils_read_line(mla_stream_input_t & inputStream, mla_string_t & line, mla_size_t timeout_ms);
mla_bool_t mla_http_utils_write_headers(const mla_array_list_t<mla_init_struct(mla_http_header_t)> &headers, mla_stream_output_t & connection);
mla_bool_t mla_http_utils_read_headers(mla_array_list_t<mla_init_struct(mla_http_header_t)> &headers, mla_stream_input_t & connection, mla_size_t timeout_ms);
mla_bool_t mla_http_utils_parse_http_version(const mla_string_t &versionStr, mla_http_version &version);
mla_bool_t mla_http_utils_write_content_headers(const mla_array_list_t<mla_init_struct(mla_http_header_t)> &headers, mla_stream_input_t& content, mla_stream_output_t & connection);

mla_stream_input_t mla_http_content_fixed_size_input_stream(mla_stream_input_t &input, mla_size_t timeout_ms, mla_size_t content_size);

#endif