//
// Created by christian on 10/8/25.
//

#include "mla_url.h"

#include "../system/mla_number.h"

mla_url_query_param_t mla_url_query_param_empty() {
    return {
        mla_string_empty(),
        mla_string_empty()
    };
}

mla_url_t mla_url_empty() {
    return {
        mla_string_empty(),
        mla_string_empty(),
        0,
        mla_string_empty(),
        mla_array_list_empty<mla_init_struct(mla_url_query_param_t)>(),
        mla_string_empty()
    };
}

mla_bool_t mla_url_parse(const mla_string_t &urlString, mla_url_t &outUrl) {

    if (mla_string_is_empty(urlString)) {
        return false;
    }

    mla_size_t pos = 0;

    // Parse scheme (protocol)
    mla_string_t schemeDelimiter = mla_string_const("://");
    mla_int32_t schemeEndPos = mla_string_index_of(urlString, schemeDelimiter);
    if (schemeEndPos == -1) {
        return false; // Invalid URL without scheme
    }

    outUrl.scheme = mla_string_substr(urlString, 0, schemeEndPos);
    pos = schemeEndPos + 3; // Skip "://"

    // Find the end of host (could be '/', '?', '#', or end of string)
    mla_size_t hostStart = pos;
    mla_size_t pathStart = pos;

    mla_size_t length = mla_string_length(urlString);
    const mla_char_t* data = mla_string_data(urlString);

    while (pathStart < length &&
           data[pathStart] != '/' &&
           data[pathStart] != '?' &&
           data[pathStart] != '#') {
        pathStart++;
    }

    // Check for port
    mla_int32_t portPos = -1;
    for (mla_size_t p = hostStart; p < pathStart; p++) {
        if (data[p] == ':') {
            portPos = mla_s_cast<mla_int32_t>(p);
            break;
        }
    }

    if (portPos != -1) {
        outUrl.host = mla_string_substr(urlString, hostStart, portPos - hostStart);
        // Parse port number
        mla_string_t portString = mla_string_substr(urlString, portPos + 1, pathStart - portPos - 1);
        mla_uint16_t portValue = 0;
        if (!mla_parse_uint16(portString, portValue)) {
            return false; // Invalid port
        }
        portString = mla_string_empty();
        outUrl.port = portValue;
    } else {
        outUrl.host = mla_string_substr(urlString, hostStart, pathStart - hostStart);

        // default ports based on scheme
        // NOLINTBEGIN(bugprone-branch-clone)
        if (mla_string_equals_const(outUrl.scheme, "http")) {
            outUrl.port = 80;
        } else if (mla_string_equals_const(outUrl.scheme, "https")) {
            outUrl.port = 443;
        } else if (mla_string_equals_const(outUrl.scheme, "ws")) {
            outUrl.port = 80;
        } else if (mla_string_equals_const(outUrl.scheme, "wss")) {
            outUrl.port = 443;
        } else {
            outUrl.port = 0; // Unknown scheme, port not set
        }
        // NOLINTEND(bugprone-branch-clone)
    }

    pos = pathStart;

    // Parse path
    if (pos < length && data[pos] == '/') {
        mla_size_t pathEnd = pos;
        while (pathEnd < length &&
               data[pathEnd] != '?' &&
               data[pathEnd] != '#') {
            pathEnd++;
        }
        outUrl.path = mla_string_substr(urlString, pos, pathEnd - pos);
        pos = pathEnd;
    } else {
        outUrl.path = mla_string_empty();
    }

    // Parse query parameters
    if (pos < length && data[pos] == '?') {
        pos++; // Skip '?'
        mla_size_t queryEnd = pos;
        while (queryEnd < length && data[queryEnd] != '#') {
            queryEnd++;
        }

        // Parse individual query parameters
        mla_size_t paramStart = pos;
        while (paramStart < queryEnd) {
            mla_size_t equalSign = paramStart;
            while (equalSign < queryEnd &&
                   data[equalSign] != '=' &&
                   data[equalSign] != '&') {
                equalSign++;
            }

            mla_url_query_param_t param = mla_url_query_param_empty();
            param.key = mla_string_substr(urlString, paramStart, equalSign - paramStart);

            if (equalSign < queryEnd && data[equalSign] == '=') {
                mla_size_t valueStart = equalSign + 1;
                mla_size_t valueEnd = valueStart;
                while (valueEnd < queryEnd && data[valueEnd] != '&') {
                    valueEnd++;
                }
                param.value = mla_string_substr(urlString, valueStart, valueEnd - valueStart);
                paramStart = valueEnd;
            } else {
                paramStart = equalSign;
            }

            if (paramStart < queryEnd && data[paramStart] == '&') {
                paramStart++;
            }

            mla_array_list_add(outUrl.query, param);
        }

        pos = queryEnd;
    }

    // Parse fragment
    if (pos < length && data[pos] == '#') {
        pos++; // Skip '#'
        outUrl.fragment = mla_string_substr(urlString, pos);
    } else {
        outUrl.fragment = mla_string_empty();
    }

    return true;
}

mla_bool_t mla_url_parse_pathAndQuery(const mla_string_t &pathAndQueryString, mla_url_t &outUrl) {

    if (mla_string_is_empty(pathAndQueryString)) {
        return false;
    }

    mla_size_t pos = 0;
    mla_size_t length = mla_string_length(pathAndQueryString);
    const mla_char_t* data = mla_string_data(pathAndQueryString);

    // Parse path
    if (pos < length && data[pos] == '/') {
        mla_size_t pathEnd = pos;
        while (pathEnd < length && data[pathEnd] != '?') {
            pathEnd++;
        }
        outUrl.path = mla_string_substr(pathAndQueryString, pos, pathEnd - pos);
        pos = pathEnd;
    } else {
        outUrl.path = mla_string_empty();
    }

    // Parse query parameters
    if (pos < length && data[pos] == '?') {
        pos++; // Skip '?'
        mla_size_t queryEnd = pos;
        while (queryEnd < length) {
            queryEnd++;
        }

        // Parse individual query parameters
        mla_size_t paramStart = pos;
        while (paramStart < queryEnd) {
            mla_size_t equalSign = paramStart;
            while (equalSign < queryEnd &&
                   data[equalSign] != '=' &&
                   data[equalSign] != '&') {
                equalSign++;
            }

            mla_url_query_param_t param = mla_url_query_param_empty();
            param.key = mla_string_substr(pathAndQueryString, paramStart, equalSign - paramStart);

            if (equalSign < queryEnd && data[equalSign] == '=') {
                mla_size_t valueStart = equalSign + 1;
                mla_size_t valueEnd = valueStart;
                while (valueEnd < queryEnd && data[valueEnd] != '&') {
                    valueEnd++;
                }
                param.value = mla_string_substr(pathAndQueryString, valueStart, valueEnd - valueStart);
                paramStart = valueEnd;
            } else {
                paramStart = equalSign;
            }

            if (paramStart < queryEnd && data[paramStart] == '&') {
                paramStart++;
            }

            mla_array_list_add(outUrl.query, param);
        }
    }

    return true;
}

mla_string_t mla_url_to_string(const mla_url_t &url) {

    // Calculate required buffer size
    mla_size_t totalLength = mla_string_length(url.scheme) + 3; // "://"
    totalLength += mla_string_length(url.host);

    if (url.port > 0) {
        totalLength += 6; // ":" + max 5 digits
    }

    totalLength += mla_max(mla_string_length(url.path), 1); // Ensure at least 1 for "/"

    if (mla_array_list_size(url.query) > 0) {
        totalLength += 1; // "?"
        for (mla_size_t i = 0; i < mla_array_list_size(url.query); i++) {
            mla_url_query_param_t* param = mla_array_list_get_ref(url.query, i);
            totalLength += mla_string_length(param->key) + 1 + mla_string_length(param->value); // key=value

            if (i < mla_array_list_size(url.query) - 1) {
                totalLength += 1; // "&"
            }
        }
    }

    if (!mla_string_is_empty(url.fragment)) {
        totalLength += 1 + mla_string_length(url.fragment); // "#" + fragment
    }

    // Use small string optimization if possible
    if (totalLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(totalLength);

        mla_size_t offset = 0;

        // Build URL string in embedded buffer
        mla_size_t schemeLength = mla_string_length(url.scheme);
        mla_memcpy(result.embedded.data + offset, mla_string_data(url.scheme), schemeLength);
        offset += schemeLength;

        mla_memcpy(result.embedded.data + offset, "://", 3);
        offset += 3;

        mla_size_t hostLength = mla_string_length(url.host);
        mla_memcpy(result.embedded.data + offset, mla_string_data(url.host), hostLength);
        offset += hostLength;

        if (url.port > 0) {
            mla_string_t portStr = mla_string_from_uint16(url.port);
            result.embedded.data[offset++] = ':';
            mla_size_t portStrLength = mla_string_length(portStr);
            mla_memcpy(result.embedded.data + offset, mla_string_data(portStr), portStrLength);
            offset += portStrLength;
            portStr = mla_string_empty();
        }

        mla_size_t pathLength = mla_string_length(url.path);
        if (pathLength == 0) {
            result.embedded.data[offset++] = '/';
        } else {
            mla_memcpy(result.embedded.data + offset, mla_string_data(url.path), pathLength);
            offset += pathLength;
        }

        if (mla_array_list_size(url.query) > 0) {
            result.embedded.data[offset++] = '?';

            for (mla_size_t i = 0; i < mla_array_list_size(url.query); i++) {
                mla_url_query_param_t* param = mla_array_list_get_ref(url.query, i);

                mla_size_t keyLength = mla_string_length(param->key);
                mla_memcpy(result.embedded.data + offset, mla_string_data(param->key), keyLength);
                offset += keyLength;

                result.embedded.data[offset++] = '=';

                mla_size_t valueLength = mla_string_length(param->value);
                mla_memcpy(result.embedded.data + offset, mla_string_data(param->value), valueLength);
                offset += valueLength;

                if (i < mla_array_list_size(url.query) - 1) {
                    result.embedded.data[offset++] = '&';
                }
            }
        }

        if (!mla_string_is_empty(url.fragment)) {
            result.embedded.data[offset++] = '#';
            mla_size_t fragmentLength = mla_string_length(url.fragment);
            mla_memcpy(result.embedded.data + offset, mla_string_data(url.fragment), fragmentLength);
        }

        return result;
    }

    mla_pointer_t buffer_ptr = mla_create_char_array(totalLength);

    // Fall back to heap allocation for larger strings
    mla_char_t *buffer = mla_pointer_get_data<mla_char_t>(buffer_ptr);

    if (buffer == nullptr) {
        return mla_string_empty(); // Memory allocation failed
    }

    mla_size_t offset = 0;

    // Build URL string
    mla_size_t schemeLength = mla_string_length(url.scheme);
    mla_memcpy(buffer + offset, mla_string_data(url.scheme), schemeLength);
    offset += schemeLength;

    mla_memcpy(buffer + offset, "://", 3);
    offset += 3;

    mla_size_t hostLength = mla_string_length(url.host);
    mla_memcpy(buffer + offset, mla_string_data(url.host), hostLength);
    offset += hostLength;

    if (url.port > 0) {
        mla_string_t portStr = mla_string_from_uint16(url.port);
        buffer[offset++] = ':';
        mla_size_t portStrLength = mla_string_length(portStr);
        mla_memcpy(buffer + offset, mla_string_data(portStr), portStrLength);
        offset += portStrLength;
        portStr = mla_string_empty();
    }

    mla_size_t pathLength = mla_string_length(url.path);
    if (pathLength == 0) {
        buffer[offset++] = '/';
    } else {
        mla_memcpy(buffer + offset, mla_string_data(url.path), pathLength);
        offset += pathLength;
    }

    if (mla_array_list_size(url.query) > 0) {
        buffer[offset++] = '?';

        for (mla_size_t i = 0; i < mla_array_list_size(url.query); i++) {
            mla_url_query_param_t* param = mla_array_list_get_ref(url.query, i);

            mla_size_t keyLength = mla_string_length(param->key);
            mla_memcpy(buffer + offset, mla_string_data(param->key), keyLength);
            offset += keyLength;

            buffer[offset++] = '=';

            mla_size_t valueLength = mla_string_length(param->value);
            mla_memcpy(buffer + offset, mla_string_data(param->value), valueLength);
            offset += valueLength;

            if (i < mla_array_list_size(url.query) - 1) {
                buffer[offset++] = '&';
            }
        }
    }

    if (!mla_string_is_empty(url.fragment)) {
        buffer[offset++] = '#';
        mla_size_t fragmentLength = mla_string_length(url.fragment);
        mla_memcpy(buffer + offset, mla_string_data(url.fragment), fragmentLength);
        offset += fragmentLength;
    }

    return mla_string(buffer_ptr, offset);
}

mla_string_t mla_url_to_string_pathAndQuery(const mla_url_t &url) {

    mla_size_t totalLength = mla_max(mla_string_length(url.path), 1); // Default to "/" if empty

    if (mla_array_list_size(url.query) > 0) {
        totalLength += 1; // "?"
        for (mla_size_t i = 0; i < mla_array_list_size(url.query); i++) {
            mla_url_query_param_t* param = mla_array_list_get_ref(url.query, i);
            totalLength += mla_string_length(param->key) + 1 + mla_string_length(param->value); // key=value

            if (i < mla_array_list_size(url.query) - 1) {
                totalLength += 1; // "&"
            }
        }
    }

    // Use small string optimization if possible
    if (totalLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(totalLength);

        mla_size_t offset = 0;

        // Add path
        mla_size_t pathLength = mla_string_length(url.path);
        if (pathLength == 0) {
            result.embedded.data[offset++] = '/';
        } else {
            mla_memcpy(result.embedded.data + offset, mla_string_data(url.path), pathLength);
            offset += pathLength;
        }

        // Add query parameters
        if (mla_array_list_size(url.query) > 0) {
            result.embedded.data[offset++] = '?';

            for (mla_size_t i = 0; i < mla_array_list_size(url.query); i++) {
                mla_url_query_param_t* param = mla_array_list_get_ref(url.query, i);

                mla_size_t keyLength = mla_string_length(param->key);
                mla_memcpy(result.embedded.data + offset, mla_string_data(param->key), keyLength);
                offset += keyLength;

                result.embedded.data[offset++] = '=';

                mla_size_t valueLength = mla_string_length(param->value);
                mla_memcpy(result.embedded.data + offset, mla_string_data(param->value), valueLength);
                offset += valueLength;

                if (i < mla_array_list_size(url.query) - 1) {
                    result.embedded.data[offset++] = '&';
                }
            }
        }

        return result;
    }

    mla_pointer_t buffer_ptr = mla_create_char_array(totalLength);

    // Fall back to heap allocation for larger strings
    mla_char_t *buffer = mla_pointer_get_data<mla_char_t>(buffer_ptr);

    if (buffer == nullptr) {
        return mla_string_empty(); // Memory allocation failed
    }

    mla_size_t offset = 0;

    // Add path
    mla_size_t pathLength = mla_string_length(url.path);
    if (pathLength == 0) {
        buffer[offset++] = '/';
    } else {
        mla_memcpy(buffer + offset, mla_string_data(url.path), pathLength);
        offset += pathLength;
    }

    // Add query parameters
    if (mla_array_list_size(url.query) > 0) {
        buffer[offset++] = '?';

        for (mla_size_t i = 0; i < mla_array_list_size(url.query); i++) {
            mla_url_query_param_t* param = mla_array_list_get_ref(url.query, i);

            mla_size_t keyLength = mla_string_length(param->key);
            mla_memcpy(buffer + offset, mla_string_data(param->key), keyLength);
            offset += keyLength;

            buffer[offset++] = '=';

            mla_size_t valueLength = mla_string_length(param->value);
            mla_memcpy(buffer + offset, mla_string_data(param->value), valueLength);
            offset += valueLength;

            if (i < mla_array_list_size(url.query) - 1) {
                buffer[offset++] = '&';
            }
        }
    }

    return mla_string(buffer_ptr, offset);
}

