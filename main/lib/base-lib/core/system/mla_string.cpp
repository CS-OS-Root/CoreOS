//
// Created by christian on 9/10/25.
//


#include "mla_string.h"
#include "../log/mla_logging.h"
#include "../utils/mla_char_utils.h"

mla_string_t mla_string_t::init() {
    return mla_string_empty();
}

mla_bool_t mla_string_is_embedded(const mla_string_t &p_String) {
    return (p_String.embedded.memoryLayout & MLA_STRING_MEMORY_LOCATION_MASK) == MLA_STRING_LOCATION_EMBEDDED;
}

mla_bool_t mla_string_is_heap(const mla_string_t &p_String) {
    return (p_String.embedded.memoryLayout & MLA_STRING_MEMORY_LOCATION_MASK) == MLA_STRING_LOCATION_HEAP;
}

mla_bool_t mla_string_is_c_string(const mla_string_t &p_String) {
    return (p_String.embedded.memoryLayout & MLA_STRING_MEMORY_FORMAT_MASK) == MLA_STRING_FORMAT_C_STRING;
}

mla_bool_t mla_string_is_sub_string(const mla_string_t &p_String) {
    return (p_String.embedded.memoryLayout & MLA_STRING_MEMORY_FORMAT_MASK) == MLA_STRING_FORMAT_SUB_STRING;
}

const mla_char_t* mla_c_string_data(const mla_c_string_t &p_CString) {
    return mla_pointer_get_data<mla_char_t>(p_CString.c_heap_str);
}

mla_pointer_t mla_create_char_array(const mla_size_t p_Length) {
    return mla_malloc(sizeof(mla_char_t) * p_Length, nullptr, mla_dynamic_data_empty());
}

mla_string_t mla_string_empty() {
    return  {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
}

mla_string_t mla_string_copy(const mla_char_t *p_Data, mla_size_t p_Length) {

    if (p_Data == nullptr || p_Length == 0) {
        return mla_string_empty();
    }

    if (p_Length <= mla_global_config_string_sso_max_length) {
        // Use embedded storage for small strings
        mla_string_t result =  {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(p_Length);
        mla_memcpy(result.embedded.data, p_Data, p_Length);
        return result;
    }

    mla_pointer_t newData = mla_create_char_array(p_Length);
    mla_char_t* new_data_ptr = mla_pointer_get_data<mla_char_t>(newData);

    if (new_data_ptr == nullptr) {
        return mla_string_empty(); // Memory allocation failed
    }

    mla_memcpy(new_data_ptr, p_Data, p_Length);

    mla_string_t result =  {newData, {{MLA_STRING_MEMORY_LAYOUT_BUFFER, 0, {0}}}};
    result.heap.length = p_Length;
    result.heap.char_offset = 0;
    return result;
}

mla_string_t mla_string_copy(const mla_string_t &p_String) {

    if (mla_string_is_embedded(p_String)) {
        return p_String; // Embedded strings can be returned as-is
    } else {
        const mla_char_t* data = mla_string_data(p_String);
        mla_size_t length = mla_string_length(p_String);
        return mla_string_copy(data, length);
    }
}

mla_string_t mla_string_from_c_string(const mla_pointer_t& data, mla_size_t p_Length) {
    mla_string_t result =  {data, {{MLA_STRING_MEMORY_LAYOUT_C_STRING, 0, {0}}}};
    result.heap.length = p_Length;
    return result;
}

mla_string_t mla_string(const mla_pointer_t& data, mla_size_t p_Length) {
    mla_string_t result =  {data, {{MLA_STRING_MEMORY_LAYOUT_BUFFER, 0, {0}}}};
    result.heap.length = p_Length;
    return result;
}

mla_string_t mla_string(const mla_pointer_t& data) {

    mla_char_t* str_data = mla_pointer_get_data<mla_char_t>(data);

    if (str_data == nullptr) {
        return mla_string_empty();
    }

    mla_string_t result =  {data, {{MLA_STRING_MEMORY_LAYOUT_C_STRING, 0, {0}}}};
    result.heap.length = mla_strlen(str_data);
    return result;
}

mla_string_t mla_string(const mla_pointer_t& data, const mla_char_t *p_End) {

    mla_char_t* str_data = mla_pointer_get_data<mla_char_t>(data);

    if (str_data == nullptr) {
        return mla_string_empty();
    }

    mla_string_t result =  {data, {{MLA_STRING_MEMORY_LAYOUT_BUFFER, 0, {0}}}};
    result.heap.length = mla_s_cast<mla_size_t>(p_End - str_data);
    return result;
}

mla_string_t mla_string(const mla_char_t *p_Data) {

    if (p_Data == nullptr) {
        return mla_string_empty();
    }

    mla_size_t length = mla_strlen(p_Data);

    if (length <= mla_global_config_string_sso_max_length) {
        // Use embedded storage for small strings
        mla_string_t result =  {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(mla_strlen(p_Data));
        mla_memcpy(result.embedded.data, p_Data, result.embedded.length);
        return result;
    }

    mla_pointer_t str_ptr = mla_platform_pointer_to_managed_pointer(p_Data);

    mla_string_t result =  {str_ptr, {{MLA_STRING_MEMORY_LAYOUT_C_STRING, 0, {0}}}};
    result.heap.length = length;
    return result;
}

mla_bool_t mla_string_equals(const mla_string_t &p_String1, const mla_string_t &p_String2) {

    mla_size_t length1 = mla_string_length(p_String1);
    mla_size_t length2 = mla_string_length(p_String2);

    if (length1 != length2) {
        return false; // Lengths differ, cannot be equal
    }

    if (length1 == 0) {
        return true; // Both strings are empty
    }

    const mla_char_t* data1 = mla_string_data(p_String1);
    const mla_char_t* data2 = mla_string_data(p_String2);

    if (data1 == data2) {
        return true; // Same pointer, same string
    }

    if (data1 == nullptr || data2 == nullptr) {
        return false; // One is null, the other is not
    }

    if (mla_string_is_c_string(p_String1) && mla_string_is_c_string(p_String2)) {

        // If both strings are C-style strings, use strstr for compare
        return mla_strstr(data1, data2) == data1;
    }

    if (length1 <= mla_global_config_string_equals_max_loop_check) {

        for (mla_size_t i = 0; i < length1; i++) {
            if (data1[i] != data2[i]) {
                return false;
            }
        }

        return true;

    }

    return mla_memcmp(data1, data2, length1) == 0; // Compare the actual data
}

mla_size_t mla_string_length(const mla_string_t &p_String) {

    if (mla_string_is_embedded(p_String)) {
        return p_String.embedded.length;
    } else {
        return p_String.heap.length;
    }

}

const mla_char_t *mla_string_data(const mla_string_t &p_String) {

    if (mla_string_is_embedded(p_String)) {

        if (p_String.embedded.length == 0) {
            return nullptr;
        }

        return p_String.embedded.data;
    }  else {

        if (p_String.heap.length == 0) {
            return nullptr;
        }

        return mla_pointer_get_data<mla_char_t>(p_String.data_storage) + p_String.heap.char_offset;
    }
}

mla_int32_t mla_string_compare(const mla_string_t &p_String1, const mla_string_t &p_String2) {

    mla_size_t length1 = mla_string_length(p_String1);
    mla_size_t length2 = mla_string_length(p_String2);

    mla_size_t minLength = length1 < length2 ? length1 : length2;

    const mla_char_t* data1 = mla_string_data(p_String1);
    const mla_char_t* data2 = mla_string_data(p_String2);

    mla_int32_t result = mla_memcmp(data1, data2, minLength);

    if (result != 0) {
        return result; // Characters differ
    }

    // All compared characters are equal, compare lengths
    if (length1 < length2) {
        return -1;
    } else if (length1 > length2) {
        return 1;
    }

    return 0; // Strings are equal

}

mla_int32_t mla_string_compare_ignore_case(const mla_string_t &p_String1, const mla_string_t &p_String2) {

    mla_size_t length1 = mla_string_length(p_String1);
    mla_size_t length2 = mla_string_length(p_String2);

    mla_size_t minLength = length1 < length2 ? length1 : length2;

    const mla_char_t* data1 = mla_string_data(p_String1);
    const mla_char_t* data2 = mla_string_data(p_String2);

    for (mla_size_t i = 0; i < minLength; ++i) {
        mla_char_t char1 = mla_char_toLower(data1[i]);
        mla_char_t char2 = mla_char_toLower(data2[i]);

        if (char1 < char2) {
            return -1;
        } else if (char1 > char2) {
            return 1;
        }
    }

    // All compared characters are equal, compare lengths
    if (length1 < length2) {
        return -1;
    } else if (length1 > length2) {
        return 1;
    }

    return 0; // Strings are equal ignoring case

}

mla_string_t mla_string_to_lower(const mla_string_t &p_String) {

    // Check if all characters are already lowercase
    mla_bool_t alreadyLower = true;

    const mla_char_t* data = mla_string_data(p_String);

    for (mla_size_t i = 0; i < mla_string_length(p_String); ++i) {
        if (data[i] != mla_char_toLower(data[i])) {
            alreadyLower = false;
            break;
        }
    }

    if (alreadyLower) {
        return p_String; // Return as-is, no conversion needed
    }

    if (mla_string_is_embedded(p_String)) {

        mla_string_t result =  {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = p_String.embedded.length;
        for (mla_uint8_t i = 0; i < p_String.embedded.length; ++i) {
            result.embedded.data[i] = mla_char_toLower(p_String.embedded.data[i]);
        }
        return result;

    } else {

        mla_pointer_t newData = mla_create_char_array(p_String.heap.length);

        mla_char_t* newData_ptr = mla_pointer_get_data<mla_char_t>(newData);

        if (newData_ptr == nullptr) {
            return mla_string_empty(); // Memory allocation failed
        }

        mla_char_t* oldData_ptr = mla_pointer_get_data<mla_char_t>(p_String.data_storage);

        for (mla_size_t i = 0; i < p_String.heap.length; ++i) {
            newData_ptr[i] = mla_char_toLower(oldData_ptr[i]);
        }

        mla_string_t result =  {newData, {{MLA_STRING_MEMORY_LAYOUT_BUFFER, 0, {0}}}};
        result.heap.length = p_String.heap.length;
        result.heap.char_offset = 0;
        return result;
    }

}

mla_string_t mla_string_to_upper(const mla_string_t &p_String) {

    const mla_char_t* data = mla_string_data(p_String);

    // Check if all characters are already uppercase
    mla_bool_t alreadyUpper = true;
    for (mla_size_t i = 0; i < mla_string_length(p_String); ++i) {
        if (data[i] != mla_char_toUpper(data[i])) {
            alreadyUpper = false;
            break;
        }
    }

    if (alreadyUpper) {
        return p_String; // Return as-is, no conversion needed
    }

    if (mla_string_is_embedded(p_String)) {

        mla_string_t result =  {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = p_String.embedded.length;
        for (mla_uint8_t i = 0; i < p_String.embedded.length; ++i) {
            result.embedded.data[i] = mla_char_toUpper(p_String.embedded.data[i]);
        }
        return result;

    } else {

        mla_pointer_t newData = mla_create_char_array(p_String.heap.length);

        mla_char_t* newData_ptr = mla_pointer_get_data<mla_char_t>(newData);

        if (newData_ptr == nullptr) {
            return mla_string_empty(); // Memory allocation failed
        }

        mla_char_t* oldData_ptr = mla_pointer_get_data<mla_char_t>(p_String.data_storage);

        for (mla_size_t i = 0; i < p_String.heap.length; ++i) {
            newData_ptr[i] = mla_char_toUpper(oldData_ptr[i]);
        }

        mla_string_t result =  {newData, {{MLA_STRING_MEMORY_LAYOUT_BUFFER, 0, {0}}}};
        result.heap.length = p_String.heap.length;
        result.heap.char_offset = 0;
        return result;
    }
}


mla_bool_t mla_string_is_empty(const mla_string_t &p_String) {

    return mla_string_length(p_String) == 0;
}

mla_bool_t mla_string_equals_ignore_case(const mla_string_t &p_String1, const mla_string_t &p_String2) {

    mla_size_t length1 = mla_string_length(p_String1);
    mla_size_t length2 = mla_string_length(p_String2);

    if (length1 == 0 && length2 == 0) {
        return true; // Both strings are empty
    }

    if (length1 != length2) {
        return false; // Lengths differ, cannot be equal
    }

    const mla_char_t* data1 = mla_string_data(p_String1);
    const mla_char_t* data2 = mla_string_data(p_String2);

    if (data1 == data2) {
        return true; // Same pointer, same string
    }

    if (data1 == nullptr) {
        return false;
    }

    if (data2 == nullptr) {
        return false;
    }

    for (mla_size_t i = 0; i < length1; ++i) {
        if (mla_char_toLower(data1[i]) != mla_char_toLower(data2[i])) {
            return false; // Characters differ, not equal
        }
    }
    return true; // All characters are equal ignoring case
}

mla_bool_t mla_string_contains(const mla_string_t &p_String, const mla_string_t &p_Substring) {

    mla_size_t length = mla_string_length(p_String);
    mla_size_t lengthSub = mla_string_length(p_Substring);

    if (lengthSub > length) {
        return false; // Substring cannot be longer than the string
    }

    if (mla_string_is_c_string(p_String) && mla_string_is_c_string(p_Substring)) {

        // If both strings are C-style strings, use strstr for substring search
        // because it is more efficient and optimized for this purpose
        return mla_strstr(mla_string_data(p_String), mla_string_data(p_Substring)) != nullptr; // Use strstr for substring search
    }

    const mla_char_t* data = mla_string_data(p_String);
    const mla_char_t* dataSub = mla_string_data(p_Substring);

    for (mla_size_t i = 0; i <= length - lengthSub; ++i) {

        if (mla_memcmp(data + i, dataSub, lengthSub) == 0) {
            return true;
        }

    }
    return false; // Substring not found
}

mla_bool_t mla_string_contains_ignore_case(const mla_string_t &p_String1, const mla_string_t &p_String2) {

    mla_size_t length1 = mla_string_length(p_String1);
    mla_size_t length2 = mla_string_length(p_String2);

    if (length2 > length1) {
        return false; // Substring cannot be longer than the string
    }

    const mla_char_t* data1 = mla_string_data(p_String1);
    const mla_char_t* data2 = mla_string_data(p_String2);

    for (mla_size_t i = 0; i <= length1 - length2; ++i) {

        mla_bool_t match = true;
        for (mla_size_t j = 0; j < length2; ++j) {
            if (mla_char_toLower(data1[i + j]) != mla_char_toLower(data2[j])) {
                match = false;
                break;
            }
        }
        if (match) {
            return true; // Substring found ignoring case
        }
    }
    return false; // Substring not found
}

mla_string_t mla_string_replace(const mla_string_t &p_String, const mla_string_t &p_OldSubstring, const mla_string_t &p_NewSubstring) {

    mla_size_t length = mla_string_length(p_String);
    mla_size_t lengthOldSub = mla_string_length(p_OldSubstring);

    // Edge cases
    if (lengthOldSub == 0) {
        return p_String; // Nothing to replace
    }
    if (length == 0) {
        return p_String; // Empty string, nothing to replace
    }

    if (lengthOldSub > length) {
        return p_String; // Old substring is longer than the string, cannot replace
    }

    const mla_char_t* data = mla_string_data(p_String);
    const mla_char_t* dataOldSub = mla_string_data(p_OldSubstring);

    // Count occurrences to calculate new size
    mla_size_t occurrenceCount = 0;
    mla_size_t pos = 0;
    while (pos <= length - lengthOldSub) {
        if (mla_memcmp(data + pos, dataOldSub, lengthOldSub) == 0) {
            ++occurrenceCount;
            pos += lengthOldSub; // Skip past the found substring
        } else {
            ++pos;
        }
    }

    if (occurrenceCount == 0) {
        return p_String; // No occurrences found, return original string
    }

    mla_size_t lengthNewSub = mla_string_length(p_NewSubstring);
    const mla_char_t* dataNewSub = mla_string_data(p_NewSubstring);

    // Calculate new string length
    mla_size_t oldTotalLength = occurrenceCount * lengthOldSub;
    mla_size_t newTotalLength = occurrenceCount * lengthNewSub;
    mla_size_t resultLength = length - oldTotalLength + newTotalLength;

    if (resultLength <= mla_global_config_string_sso_max_length) {

        mla_string_t result =  {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(resultLength);
        mla_size_t sourcePos = 0;
        mla_size_t destPos = 0;
        while (sourcePos < length) {
            // Check if we found the old substring at current position
            if (sourcePos <= length - lengthOldSub &&
                mla_memcmp(data + sourcePos, dataOldSub, lengthOldSub) == 0) {

                // Copy new substring
                if (lengthNewSub > 0) {
                    mla_memcpy(result.embedded.data + destPos, dataNewSub, lengthNewSub);
                    destPos += lengthNewSub;
                }
                sourcePos += lengthOldSub;
            } else {
                // Copy single character
                result.embedded.data[destPos++] = data[sourcePos++];
            }
        }
        return result;

    } else {

        // Allocate new buffer
        mla_pointer_t newData = mla_create_char_array(resultLength);

        mla_char_t* new_data_ptr = mla_pointer_get_data<mla_char_t>(newData);

        if (new_data_ptr == nullptr) {
            return mla_string_empty(); // Memory allocation failed
        }

        // Build the new string
        mla_size_t sourcePos = 0;
        mla_size_t destPos = 0;

        while (sourcePos < length) {
            // Check if we found the old substring at current position
            if (sourcePos <= length - lengthOldSub &&
                mla_memcmp(data + sourcePos, dataOldSub, lengthOldSub) == 0) {

                // Copy new substring
                if (lengthNewSub > 0) {
                    mla_memcpy(new_data_ptr + destPos, dataNewSub, lengthNewSub);
                    destPos += lengthNewSub;
                }
                sourcePos += lengthOldSub;
                } else {
                    // Copy single character
                    new_data_ptr[destPos++] = data[sourcePos++];
                }
        }

        mla_string_t result =  {newData, {{MLA_STRING_MEMORY_LAYOUT_BUFFER, 0, {0}}}};
        result.heap.length = resultLength;
        result.heap.char_offset = 0;
        return result;
    }
}


mla_bool_t mla_string_starts_with(const mla_string_t &p_String, const mla_string_t &p_Prefix) {

    mla_size_t length = mla_string_length(p_String);
    mla_size_t lengthPrefix = mla_string_length(p_Prefix);

    if (lengthPrefix > length) {
        return false; // Prefix cannot be longer than the string
    }

    const mla_char_t* data = mla_string_data(p_String);
    const mla_char_t* dataPrefix = mla_string_data(p_Prefix);

    return mla_memcmp(data, dataPrefix, lengthPrefix) == 0; // Compare the prefix

}

mla_bool_t mla_string_starts_with_ignore_case(const mla_string_t &p_String, const mla_string_t &p_Prefix) {

    mla_size_t length = mla_string_length(p_String);
    mla_size_t lengthPrefix = mla_string_length(p_Prefix);

    if (lengthPrefix > length) {
        return false; // Prefix cannot be longer than the string
    }

    const mla_char_t* data = mla_string_data(p_String);
    const mla_char_t* dataPrefix = mla_string_data(p_Prefix);

    for (mla_size_t i = 0; i < lengthPrefix; ++i) {
        if (mla_char_toLower(data[i]) != mla_char_toLower(dataPrefix[i])) {
            return false; // Characters differ, not a match
        }
    }
    return true; // All characters match ignoring case
}

mla_bool_t mla_string_ends_with(const mla_string_t &p_String, const mla_string_t &p_Suffix) {

    mla_size_t length = mla_string_length(p_String);
    mla_size_t lengthSufix = mla_string_length(p_Suffix);

    if (lengthSufix > length) {
        return false; // Suffix cannot be longer than the string
    }

    const mla_char_t* data = mla_string_data(p_String);
    const mla_char_t* dataSufix = mla_string_data(p_Suffix);

    return mla_memcmp(data + length - lengthSufix, dataSufix, lengthSufix) == 0; // Compare the suffix

}

mla_bool_t mla_string_ends_with_ignore_case(const mla_string_t &p_String, const mla_string_t &p_Suffix) {

    mla_size_t length = mla_string_length(p_String);
    mla_size_t lengthSufix = mla_string_length(p_Suffix);

    if (lengthSufix > length) {
        return false; // Suffix cannot be longer than the string
    }

    const mla_char_t* data = mla_string_data(p_String);
    const mla_char_t* dataSufix = mla_string_data(p_Suffix);

    for (mla_size_t i = 0; i < lengthSufix; ++i) {
        if (mla_char_toLower(data[length - lengthSufix + i]) != mla_char_toLower(dataSufix[i])) {
            return false; // Characters differ, not a match
        }
    }
    return true; // All characters match ignoring case
}

mla_int32_t mla_string_index_of(const mla_string_t &p_String, const mla_string_t &p_Substring) {
    return mla_string_index_of(p_String, p_Substring, 0);
}


mla_int32_t mla_string_index_of(const mla_string_t &p_String, const mla_string_t &p_Substring, mla_size_t p_Start) {

    mla_size_t length = mla_string_length(p_String);
    mla_size_t lengthSub = mla_string_length(p_Substring);

    if (p_Start >= length) {
        return -1; // Start index is outside the string
    }

    if (lengthSub == 0) {
        return mla_s_cast<mla_int32_t>(p_Start); // Empty substring matches at the start index
    }

    if (lengthSub > length || p_Start > length - lengthSub) {
        return -1; // Substring cannot fit from the start index
    }

    const mla_char_t* data = mla_string_data(p_String);
    const mla_char_t* dataSub = mla_string_data(p_Substring);

    if (mla_string_is_c_string(p_String) && mla_string_is_c_string(p_Substring)) {
        const mla_char_t *found = mla_strstr(data + p_Start, dataSub);

        if (found != nullptr) {
            return mla_s_cast<mla_int32_t>(found - data);
        } else {
            return -1; // Substring not found
        }
    }

    // Fast path without memcmp
    if (lengthSub == 1) {

        for (mla_size_t i = p_Start; i < length; ++i) {

            if (data[i] == dataSub[0]) {
                return mla_s_cast<mla_int32_t>(i);
            }

        }

        return -1; // Substring not found
    }

    if (lengthSub == 2) {
        for (mla_size_t i = p_Start; i <= length - lengthSub; ++i) {
            if (data[i] == dataSub[0] && data[i + 1] == dataSub[1]) {
                return mla_s_cast<mla_int32_t>(i);
            }
        }

        return -1;
    }

    // Manual search for the substring
    for (mla_size_t i = p_Start; i <= length - lengthSub; ++i) {

        if (mla_memcmp(data + i, dataSub, lengthSub) == 0) {
            return mla_s_cast<mla_int32_t>(i);
        }

    }
    return -1; // Substring not found
}


mla_int32_t mla_string_last_index_of(const mla_string_t &p_String, const mla_string_t &p_Substring) {

    mla_size_t length = mla_string_length(p_String);
    return mla_string_last_index_of(p_String, p_Substring, length - 1);
}

mla_int32_t mla_string_last_index_of(const mla_string_t &p_String, const mla_string_t &p_Substring, mla_size_t p_Start) {

    mla_size_t length = mla_string_length(p_String);
    mla_size_t lengthSub = mla_string_length(p_Substring);

    if (p_Start >= length) {
        return -1; // Start index is outside the string
    }

    if (lengthSub > length || p_Start < lengthSub - 1) {
        return -1; // Substring cannot fit before the start index
    }

    const mla_char_t* data = mla_string_data(p_String);
    const mla_char_t* dataSub = mla_string_data(p_Substring);

    for (mla_int32_t i = mla_s_cast<mla_int32_t>(p_Start); i >= 0; --i) {

        if (i + lengthSub > length) {
            continue; // Substring cannot fit at this position
        }

        if (mla_memcmp(data + i, dataSub, lengthSub) == 0) {
            return i; // Found the substring
        }
    }
    return -1; // Substring not found
}

mla_string_t mla_string_substr(const mla_string_t &p_String, mla_size_t p_Start) {

    if (p_Start == 0) {
        return p_String; // Return the original string if start index is 0
    }

    mla_size_t length = mla_string_length(p_String);

    if (p_Start >= length) {
        return mla_string_empty(); // Invalid start index
    }

    return mla_string_substr(p_String, p_Start, length - p_Start);

}

mla_string_t mla_string_substr(const mla_string_t &p_String, mla_size_t p_Start, mla_size_t p_Length) {

    mla_size_t length = mla_string_length(p_String);

    // For an substring we dont need to copy any data we can just play with pointers
    if (p_Start >= length || p_Length == 0) {
        return mla_string_empty(); // Invalid range
    }

    mla_uint64_t l_maxLength = mla_s_cast<mla_uint64_t>(p_Start) + mla_s_cast<mla_uint64_t>(p_Length);

    if (l_maxLength > mla_s_cast<mla_uint64_t>(length)) {
        p_Length = length - p_Start; // Adjust length to the maximum possible
    }

    if (mla_string_is_embedded(p_String)) {

        mla_string_t result =  {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(p_Length);
        mla_memcpy(result.embedded.data, mla_string_data(p_String) + p_Start, p_Length);
        return result;
    }

    if (p_Length <= mla_global_config_string_sso_max_length) {

        // I am not sure if this is a good idea to copy substrings into embedded strings
        // Maybe its faster just to create a substring view

        mla_string_t result =  {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(p_Length);
        mla_memcpy(result.embedded.data, mla_string_data(p_String) + p_Start, p_Length);
        return result;
    }

    mla_string_t result =  {p_String.data_storage, {{MLA_STRING_MEMORY_LAYOUT_SUB_STRING, 0, {0}}}};
    result.heap.length = p_Length;
    result.heap.char_offset = p_Start + p_String.heap.char_offset; // Adjust char offset for the substring view
    return result;
}

mla_string_t mla_string_repeat(const mla_string_t &p_String, mla_size_t p_Times) {

    mla_size_t length = mla_string_length(p_String);

    if (p_Times == 0 || length == 0) {
        return mla_string_empty();
    }

    mla_size_t resultLength = length * p_Times;
    const mla_char_t* source = mla_string_data(p_String);

    if (resultLength <= mla_global_config_string_sso_max_length) {

        mla_string_t result = {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(resultLength);

        for (mla_size_t i = 0; i < p_Times; ++i) {
            const mla_size_t offset = i * length;
            mla_memcpy(result.embedded.data + offset, source, length);
        }
        return result;
    }

    mla_pointer_t newData = mla_create_char_array(resultLength);
    mla_char_t* new_data_ptr = mla_pointer_get_data<mla_char_t>(newData);

    if (new_data_ptr == nullptr) {
        return mla_string_empty();
    }

    for (mla_size_t i = 0; i < p_Times; ++i) {
        const mla_size_t offset = i * length;
        mla_memcpy(new_data_ptr + offset, source, length);
    }

    mla_string_t result = {newData, {{MLA_STRING_MEMORY_LAYOUT_BUFFER, 0, {0}}}};
    result.heap.length = resultLength;
    result.heap.char_offset = 0;
    return result;
}

mla_array_list_t<mla_init_struct(mla_string_t)> mla_string_split(const mla_string_t &p_String, const mla_string_t &p_Delimiter) {

    mla_array_list_t<mla_init_struct(mla_string_t)> result = mla_array_list<mla_init_struct(mla_string_t)>(1);

    if (mla_string_is_empty(p_String)) {
        mla_array_list_add(result, p_String);
        return result;
    }

    if (mla_string_is_empty(p_Delimiter)) {
        // If the delimiter is empty, return the original string as the only element
        mla_array_list_add(result, p_String);
        return result;
    }

    // Split the string
    mla_size_t start = 0;
    mla_int32_t delimiterIndex = 0;

    mla_size_t length = mla_string_length(p_String);
    mla_size_t lengthDelimiter = mla_string_length(p_Delimiter);

    while (start < length) {
        // Create a substring from the current position
        mla_string_t searchString = mla_string_substr(p_String, start);

        // Find the next delimiter
        delimiterIndex = mla_string_index_of(searchString, p_Delimiter);

        if (delimiterIndex == -1) {
            // No more delimiters found, add the rest of the string
            mla_array_list_add(result, searchString);
            break;

        }

        // Add the substring before the delimiter
        mla_string_t part = mla_string_substr(searchString, 0, delimiterIndex);
        mla_array_list_add(result, part);

        // Move past the delimiter
        start = start + delimiterIndex + lengthDelimiter;
    }

    if (start == length) {
        // If the string ends with a delimiter, add an empty string at the end
        mla_array_list_add(result, mla_string_empty());
    }

    return result;
}

mla_string_t mla_string_join(const mla_array_list_t<mla_init_struct(mla_string_t)> &p_Strings, const mla_string_t &p_Delimiter) {

    mla_size_t totalLength = 0;
    mla_size_t count = mla_array_list_size(p_Strings);
    mla_size_t delimiterLength = mla_string_length(p_Delimiter);

    for (mla_size_t i = 0; i < count; ++i) {
        totalLength += mla_string_length(mla_array_list_get_unsafe(p_Strings, i));
    }

    if (count > 1) {
        totalLength += delimiterLength * (count - 1);
    }

    if (totalLength <= mla_global_config_string_sso_max_length) {

        mla_string_t result =  {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(totalLength);

        mla_size_t pos = 0;
        for (mla_size_t i = 0; i < count; ++i) {
            const mla_string_t &str = mla_array_list_get_unsafe(p_Strings, i);
            const mla_char_t* data = mla_string_data(str);
            mla_size_t length = mla_string_length(str);

            if (length > 0) {
                mla_memcpy(result.embedded.data + pos, data, length);
                pos += length;
            }

            // Add delimiter if not the last string
            if (i < count - 1 && delimiterLength > 0) {
                const mla_char_t* delimiterData = mla_string_data(p_Delimiter);
                mla_memcpy(result.embedded.data + pos, delimiterData, delimiterLength);
                pos += delimiterLength;
            }
        }
        return result;
    }

    // Allocate new buffer
    mla_pointer_t newData = mla_create_char_array(totalLength);
    mla_char_t* new_data_ptr = mla_pointer_get_data<mla_char_t>(newData);

    if (new_data_ptr == nullptr) {
        return mla_string_empty(); // Memory allocation failed
    }

    // Build the joined string
    mla_size_t pos = 0;
    for (mla_size_t i = 0; i < count; ++i) {
        const mla_string_t &str = mla_array_list_get_unsafe(p_Strings, i);
        const mla_char_t* data = mla_string_data(str);
        mla_size_t length = mla_string_length(str);

        if (length > 0) {
            mla_memcpy(new_data_ptr + pos, data, length);}
        pos += length;

        // Add delimiter if not the last string
        if (i < count - 1 && delimiterLength > 0) {
            const mla_char_t* delimiterData = mla_string_data(p_Delimiter);
            mla_memcpy(new_data_ptr + pos, delimiterData, delimiterLength);
            pos += delimiterLength;
        }
    }

    mla_string_t result =  {newData, {{MLA_STRING_MEMORY_LAYOUT_BUFFER, 0, {0}}}};
    result.heap.length = totalLength;
    result.heap.char_offset = 0;
    return result;
}

mla_string_t mla_string_trim(const mla_string_t &p_String) {

    mla_size_t length = mla_string_length(p_String);
    const mla_char_t* data = mla_string_data(p_String);

    mla_size_t start = 0;
    mla_size_t end = length;

    // Trim leading whitespace
    while (start < end && mla_char_is_whitespace(data[start])) {
        ++start;
    }

    // Trim trailing whitespace
    while (end > start && mla_char_is_whitespace(data[end - 1])) {
        --end;
    }

    if (start == 0 && end == length) {
        return p_String; // No trimming needed
    }

    if (mla_string_is_embedded(p_String)) {

        mla_string_t result =  {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(end - start);
        mla_memcpy(result.embedded.data, p_String.embedded.data + start, end - start);
        return result;
    }

    mla_string_t result =  {p_String.data_storage, {{MLA_STRING_MEMORY_LAYOUT_SUB_STRING, 0, {0}}}};
    result.heap.char_offset = start + p_String.heap.char_offset; // Adjust char offset for the trimmed view
    result.heap.length = end - start;
    return result;
}

mla_string_memory_layout_t mla_string_get_memory_layout(const mla_string_t &p_String) {
    return p_String.embedded.memoryLayout;
}

mla_bool_t mla_string_change_memory_layout(mla_string_t &p_String, mla_string_memory_layout_t p_NewLayout) {

    if (p_String.embedded.memoryLayout == p_NewLayout) {
        return true; // No change needed
    }

    if ((p_NewLayout & MLA_STRING_MEMORY_FORMAT_MASK) == MLA_STRING_FORMAT_SUB_STRING) {
        // Substrings are views into other strings, so we cannot convert to this layout directly
        mla_error("Cannot convert to substring layout directly.");
        return false;
    }

    if (p_NewLayout == MLA_STRING_MEMORY_LAYOUT_C_STRING) {

        mla_size_t length = mla_string_length(p_String);
        const mla_char_t* data = mla_string_data(p_String);

        // Convert to C-style string
        mla_size_t newLength = length + 1; // +1 for null terminator
        mla_pointer_t newData = mla_create_char_array(newLength);

        mla_char_t* newData_ptr = mla_pointer_get_data<mla_char_t>(newData);

        if (newData_ptr == nullptr) {
            return false; // Memory allocation failed
        }

        mla_memcpy(newData_ptr, data, length);
        newData_ptr[length] = '\0'; // Null-terminate the string
        p_String =  {newData, {{MLA_STRING_MEMORY_LAYOUT_C_STRING, 0, {0}}}};
        p_String.heap.char_offset = 0;
        p_String.heap.length = newLength -1;
        return true;
    } else if (p_NewLayout == MLA_STRING_MEMORY_LAYOUT_BUFFER) {
        // Convert to buffer-based string

        mla_size_t length = mla_string_length(p_String);
        const mla_char_t* data = mla_string_data(p_String);

        if (p_String.embedded.memoryLayout == MLA_STRING_MEMORY_LAYOUT_EMBEDDED) {
            // For embedded strings, we need to copy the data to a new buffer
            mla_pointer_t newData = mla_create_char_array(length);

            mla_char_t* newData_ptr = mla_pointer_get_data<mla_char_t>(newData);

            if (newData_ptr == nullptr) {
                return false; // Memory allocation failed
            }

            mla_memcpy(newData_ptr, data, length);
            p_String =  {newData, {{MLA_STRING_MEMORY_LAYOUT_BUFFER, 0, {0}}}};
            p_String.heap.char_offset = 0;
            p_String.heap.length = length;
            return true;

        } else if (p_String.embedded.memoryLayout == MLA_STRING_MEMORY_LAYOUT_C_STRING) {

            // For C-style strings, we can just adjust the length and keep the same data
            p_String.heap.length = length; // Exclude null terminator
            p_String.embedded.memoryLayout = MLA_STRING_MEMORY_LAYOUT_BUFFER;
            return true;

        } else {
            // Already a buffer-based string, no change needed
            return true;
        }

    } else if (p_NewLayout == MLA_STRING_MEMORY_LAYOUT_EMBEDDED) {
        // Convert to embedded string if possible
        mla_size_t length = mla_string_length(p_String);
        const mla_char_t* data = mla_string_data(p_String);

        if (length > mla_global_config_string_sso_max_length) {
            mla_error("Cannot convert to embedded layout: string too long.");
            return false; // Cannot convert to embedded layout
        }

        mla_string_t result =  {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(length);
        mla_memcpy(result.embedded.data, data, length);
        p_String = result;
        return true;

    } else {

        mla_error("Unknown target memory layout.");
        return false; // Unknown target layout
    }
}

mla_c_string_t mla_string_to_cString(const mla_string_t &p_String) {

    if (p_String.embedded.memoryLayout == MLA_STRING_MEMORY_LAYOUT_C_STRING) {
        return {p_String.data_storage}; // Already a C-style string, no need to copy
    }


    mla_size_t length = mla_string_length(p_String);
    const mla_char_t* data = mla_string_data(p_String);

    mla_pointer_t cString = mla_create_char_array(length + 1); // +1 for null terminator

    mla_char_t* cString_ptr = mla_pointer_get_data<mla_char_t>(cString);

    if (cString_ptr == nullptr) {
        return {mla_pointer_null()}; // Memory allocation failed
    }

    mla_memcpy(cString_ptr, data, length);
    cString_ptr[length] = '\0'; // Null-terminate the string
    return { cString };

}