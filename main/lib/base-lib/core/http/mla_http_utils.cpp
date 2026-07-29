//
// Created by chris on 10/23/2025.
//

#include "mla_http_utils.h"

mla_bool_t mla_http_utils_parse_http_version(const mla_string_t &versionStr, mla_http_version &version) {

    if (mla_string_equals_const(versionStr, "HTTP/1.0")) {
        version = MLA_HTTP_VERSION_1_0;
        return true;
    } else if (mla_string_equals_const(versionStr, "HTTP/1.1")) {
        version = MLA_HTTP_VERSION_1_1;
        return true;
    } else {
        return false; // Unsupported HTTP version
    }

}

mla_bool_t mla_http_utils_read_headers(mla_array_list_t<mla_init_struct(mla_http_header_t)> &headers, mla_stream_input_t & connection, mla_size_t timeout_ms) {

    mla_string_t doublePoint = mla_string_const(":");

    // Parse the response headers
    while (true) {
        mla_string_t headerLine = mla_string_empty();
        if (!mla_http_utils_read_line(connection, headerLine, timeout_ms)) {
            return false;
        }

        if (mla_string_is_empty(headerLine)) {
            // End of headers
            break;
        }

        mla_int32_t colonPos = mla_string_index_of(headerLine, doublePoint);
        if (colonPos < 0) {
            return false;
        }

        mla_string_t headerName = mla_string_trim(mla_string_substr(headerLine, 0, colonPos));
        mla_string_t headerValue = mla_string_trim(mla_string_substr(headerLine, colonPos + 1));

        // Add header to response
        mla_http_headers_add(headers, headerName, headerValue);
    }

    return true;
}

mla_bool_t mla_http_utils_write_headers(const mla_array_list_t<mla_init_struct(mla_http_header_t)> &headers, mla_stream_output_t & connection) {

    for (mla_size_t i = 0; i < mla_array_list_size(headers); i++) {

        const mla_http_header_t* header = mla_array_list_get_ref(headers, i);
        if (!mla_stream_output_write_string(connection, header->name)) {
            return false;
        }

        if (!mla_stream_output_write_string(connection, mla_string_const(": "))) {
            return false;
        }

        if (header->type == MLA_HTTP_HEADER_TYPE_SINGLE) {

            if (!mla_stream_output_write_string(connection, header->value)) {
                return false;
            }

        } else {

            for (mla_size_t j = 0; j < mla_array_list_size(header->values); j++) {
                const mla_string_t* value = mla_array_list_get_ref(header->values, j);
                if (!mla_stream_output_write_string(connection, *value)) {
                    return false;
                }
                if (j < mla_array_list_size(header->values) - 1) {
                    if (!mla_stream_output_write_string(connection, mla_string_const(", "))) {
                        return false;
                    }
                }
            }
        }
        if (!mla_stream_output_write_string(connection, mla_string_const("\r\n"))) {
            return false;
        }
    }

    return true;

}

mla_bool_t mla_http_utils_read_line(mla_stream_input_t & inputStream, mla_string_t & line, mla_size_t timeout_ms) {

    mla_char_t buffer[mla_global_config_stream_fast_read_buffer_size];
    mla_size_t bytesRead = 0;

    mla_pointer_t finalResultBuffer_ptr = mla_pointer_null();
    mla_char_t* finalResultBuffer = nullptr;
    mla_size_t finalResultBufferSize = 0;

    mla_int32_t remaining_timeout = mla_s_cast<mla_int32_t>(timeout_ms);

    while (true) {
        mla_size_t result = inputStream.read(inputStream, bytesRead, 1, mla_r_cast<mla_byte_t*>(&buffer[0]));

        if (result == 0) {

            if (remaining_timeout > 0) {
                // Wait and retry
                mla_sleep(10);
                remaining_timeout -= 10;
                continue;
            }

            // End of stream
            break;
        }

        bytesRead += result;

        if (bytesRead >= 2 && buffer[bytesRead - 2] == '\r' && buffer[bytesRead - 1] == '\n') {
            // End of line found

            if (finalResultBuffer == nullptr) {
                // Fast path, line is in buffer
                line = mla_string_copy(buffer, bytesRead - 2); // Exclude \r\n
                return true;
            } else {
                // Combine final buffer and current buffer
                mla_pointer_t new_buffer_ptr = mla_create_char_array(finalResultBufferSize + bytesRead - 2);  // Exclude \r\n

                mla_char_t* newBuffer = mla_pointer_get_data<mla_char_t>(new_buffer_ptr);

                if (newBuffer == nullptr) {
                    return false;
                }

                mla_memcpy(newBuffer, finalResultBuffer, finalResultBufferSize);
                mla_memcpy(newBuffer + finalResultBufferSize, buffer, bytesRead - 2);
                line = mla_string(new_buffer_ptr, finalResultBufferSize + bytesRead - 2);
                return true;
            }

        }

        if (bytesRead >= mla_global_config_stream_fast_read_buffer_size) {
            // Move to finalbuffer
            if (finalResultBuffer == nullptr) {
                finalResultBuffer_ptr = mla_create_char_array(bytesRead);
                finalResultBuffer = mla_pointer_get_data<mla_char_t>(finalResultBuffer_ptr);

                if (finalResultBuffer == nullptr) {
                    // Memory allocation failed
                    return false;
                }

                mla_memcpy(finalResultBuffer, buffer, bytesRead);
                finalResultBufferSize = bytesRead;
            } else {
                mla_pointer_t newBuffer_ptr = mla_create_char_array(finalResultBufferSize + bytesRead);
                mla_char_t* newBuffer = mla_pointer_get_data<mla_char_t>(newBuffer_ptr);

                if (newBuffer == nullptr) {
                    // Memory allocation failed
                    return false;
                }

                mla_memcpy(newBuffer, finalResultBuffer, finalResultBufferSize);
                mla_memcpy(newBuffer + finalResultBufferSize, buffer, bytesRead);
                finalResultBuffer_ptr = newBuffer_ptr;
                finalResultBuffer = newBuffer;
                finalResultBufferSize += bytesRead;
            }

            bytesRead = 0;
        }

        if ((finalResultBufferSize + bytesRead) >= mla_global_config_http_max_header_size) {
            // Line too long
            return false;
        }
    }

    return false;

}


mla_stream_input_t mla_http_content_fixed_size_input_stream( mla_stream_input_t &input, mla_size_t timeout_ms, mla_size_t content_size) {

    mla_stream_input_t timeout_stream = mla_stream_input_timeout_wrapper(input, timeout_ms);
    return mla_stream_input_limited_wrapper(timeout_stream, content_size);

}

mla_bool_t mla_http_utils_write_content_headers(const mla_array_list_t<mla_init_struct(mla_http_header_t)> &headers, mla_stream_input_t& content, mla_stream_output_t & connection) {

    // Check if Content-Length header is set for body
    mla_string_t contentLengthStr = mla_http_headers_get_value(headers, mla_string_const("Content-Length"));

    if (!mla_string_is_empty(contentLengthStr)) {
        return true;
    }

    mla_size_t bytes = 0;

    if (content.remaining_bytes != nullptr) {
        bytes = content.remaining_bytes(content);
    }

    if (bytes > 0 && bytes != mla_uint32_max) {
        // Set Content-Length header
        mla_string_t contentLengthValue = mla_string_from_uint32(bytes);

        if (!mla_stream_output_write_string(connection, mla_string_const("Content-Length: "))) {
            return false;
        }

        if (!mla_stream_output_write_string(connection, contentLengthValue)) {
            return false;
        }

        if (!mla_stream_output_write_string(connection, mla_string_const("\r\n"))) {
            return false;
        }
    }

    return true;

}