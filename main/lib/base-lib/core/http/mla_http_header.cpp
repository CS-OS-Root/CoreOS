//
// Created by christian on 10/6/25.
//

#include "mla_http_header.h"

#include "../system/mla_string_concat.h"

mla_http_header_t mla_http_header_empty() {
    return {
        mla_string_empty(),
        MLA_HTTP_HEADER_TYPE_SINGLE,
        mla_array_list_empty<mla_init_struct(mla_string_t)>(),
        mla_string_empty()
    };
}

void mla_http_headers_add(mla_array_list_t<mla_init_struct(mla_http_header_t)> &p_Headers, const mla_string_t &p_Name, const mla_string_t &p_Value, const mla_string_t &value_seperator) {

    // Check if header already exists
    for (mla_size_t i = 0; i < mla_array_list_size(p_Headers); i++) {

        mla_http_header_t* header = mla_array_list_get_ref(p_Headers, i);

        if (mla_string_equals_ignore_case(header->name, p_Name)) {

            // Update the header value
            header->value = mla_string_concat(header->value, value_seperator, p_Value);
            return;
        }
    }

    // Header does not exist, create new
    mla_http_header_t new_header = {
        p_Name,
        MLA_HTTP_HEADER_TYPE_SINGLE,
        mla_array_list_empty<mla_init_struct(mla_string_t)>(),
        p_Value,

    };

    mla_array_list_add(p_Headers, new_header);}

void mla_http_headers_add(mla_array_list_t<mla_init_struct(mla_http_header_t)> &p_Headers, const mla_string_t &p_Name, const mla_string_t &p_Value) {

    // Check if header already exists
    for (mla_size_t i = 0; i < mla_array_list_size(p_Headers); i++) {

        mla_http_header_t* header = mla_array_list_get_ref(p_Headers, i);

        if (mla_string_equals_ignore_case(header->name, p_Name)) {

            // Header exists, add value
            if (header->type == MLA_HTTP_HEADER_TYPE_SINGLE) {
                // Convert to multi-value header
                mla_string_t existing_value = header->value;
                header->value = mla_string_empty();
                header->type = MLA_HTTP_HEADER_TYPE_MULTI;
                header->values = mla_array_list<mla_init_struct(mla_string_t)>();
                mla_array_list_add(header->values, existing_value);
            }

            // Add new value
            mla_array_list_add(header->values, p_Value);
            return;
        }
    }

    // Header does not exist, create new
    mla_http_header_t new_header = {
        p_Name,
        MLA_HTTP_HEADER_TYPE_SINGLE,
        mla_array_list_empty<mla_init_struct(mla_string_t)>(),
        p_Value,

    };

    mla_array_list_add(p_Headers, new_header);
}

mla_string_t mla_http_headers_get_value(const mla_array_list_t<mla_init_struct(mla_http_header_t)> &p_Headers, const mla_string_t &p_Name) {

    // Check if header already exists
    for (mla_size_t i = 0; i < mla_array_list_size(p_Headers); i++) {

        mla_http_header_t* header = mla_array_list_get_ref(p_Headers, i);

        if (mla_string_equals_ignore_case(header->name, p_Name)) {

            // Header exists, add value
            if (header->type == MLA_HTTP_HEADER_TYPE_SINGLE) {
                return header->value;
            }

            // Return first value for multi-value header
            if (mla_array_list_size(header->values) > 0) {
                return mla_array_list_get_unsafe(header->values, 0);
            }

        }
    }

    return mla_string_empty();

}

mla_array_list_t<mla_init_struct(mla_string_t)> mla_http_headers_get_values(const mla_array_list_t<mla_init_struct(mla_http_header_t)> &p_Headers, const mla_string_t &p_Name) {

    // Check if header already exists
    for (mla_size_t i = 0; i < mla_array_list_size(p_Headers); i++) {

        mla_http_header_t* header = mla_array_list_get_ref(p_Headers, i);

        if (mla_string_equals_ignore_case(header->name, p_Name)) {

            // Header exists, return values
            if (header->type == MLA_HTTP_HEADER_TYPE_SINGLE) {
                mla_array_list_t<mla_init_struct(mla_string_t)> values = mla_array_list<mla_init_struct(mla_string_t)>(1);
                mla_array_list_add(values, header->value);
                return values;
            }

            // Return all values for multi-value header
            return header->values;
        }
    }

    return mla_array_list_empty<mla_init_struct(mla_string_t)>();
}

mla_bool_t mla_http_headers_has_header_value(const mla_array_list_t<mla_init_struct(mla_http_header_t)> &p_Headers, const mla_string_t &p_Name, const mla_string_t &p_Value) {

    for (mla_size_t i = 0; i < mla_array_list_size(p_Headers); i++) {
        mla_http_header_t* header = mla_array_list_get_ref(p_Headers, i);

        if (mla_string_equals_ignore_case(header->name, p_Name)) {
            if (header->type == MLA_HTTP_HEADER_TYPE_SINGLE) {

                if (mla_string_equals_ignore_case(header->value, p_Value)) {
                    return true;
                }

            } else {

                for (mla_size_t j = 0; j < mla_array_list_size(header->values); j++) {

                    if (mla_string_equals_ignore_case(mla_array_list_get_unsafe(header->values, j), p_Value)) {
                        return true;
                    }

                }
            }
        }
    }

    return false;
}

mla_bool_t mla_private_http_headers_has_value(const mla_string_t &headerValue, const mla_string_t &valueToCheck, const mla_string_t &value_seperator) {

    // split the value by the seperator
    mla_array_list_t<mla_init_struct(mla_string_t)> splittedValues = mla_string_split(headerValue, value_seperator);

    for (mla_size_t i = 0; i < mla_array_list_size(splittedValues); i++) {

        mla_string_t value = mla_array_list_get_unsafe(splittedValues, i);
        value = mla_string_trim(value);

        // check if there is a 100% match
        if (mla_string_equals_ignore_case(value, valueToCheck)) {
            return true;
        }

        // check if its a sub category which the most time
        // sec-websocket-extensions: permessage-deflate; client_max_window_bits
        if (mla_string_starts_with(mla_string_concat(value, ";"), valueToCheck)) {
            return true;
        }
    }

    return false;
}

mla_bool_t mla_http_headers_has_header_value(const mla_array_list_t<mla_init_struct(mla_http_header_t)> &p_Headers, const mla_string_t &p_Name, const mla_string_t &p_Value, const mla_string_t &value_seperator) {


    for (mla_size_t i = 0; i < mla_array_list_size(p_Headers); i++) {
        mla_http_header_t* header = mla_array_list_get_ref(p_Headers, i);

        if (mla_string_equals_ignore_case(header->name, p_Name)) {
            if (header->type == MLA_HTTP_HEADER_TYPE_SINGLE) {

                // split the value by the seperator
                if (mla_private_http_headers_has_value(header->value, p_Value, value_seperator)) {
                    return true;
                }

            } else {

                for (mla_size_t j = 0; j < mla_array_list_size(header->values); j++) {

                    // split the value by the seperator
                    if (mla_private_http_headers_has_value(header->value, mla_array_list_get_unsafe(header->values, j), value_seperator)) {
                        return true;
                    }

                }
            }
        }
    }

    return false;

}