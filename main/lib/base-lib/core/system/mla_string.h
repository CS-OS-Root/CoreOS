//
// Created by chris on 8/2/2025.
//

#ifndef MLA_STRING_H
#define MLA_STRING_H

#include "../mla_data_types.h"
#include "mla_array_list.h"
#include "../hash/mla_hash.h"

// Forward declaration
struct mla_multi_byte_char_t;
struct mla_string_t;

struct mla_c_string_t {
    mla_pointer_t c_heap_str;
};

// UTF-16 and UTF-32 string representations
// the data is owned by this struct.
// So you need to free it when you are done
struct mla_string_utf16_buffer_t {
    mla_pointer_t data;
    mla_size_t charCount; // Length of the UTF-16 string without the null terminator
};

const mla_utf_16_char_t* mla_string_utf16_data(const mla_string_utf16_buffer_t &p_Utf16Buffer);

struct mla_string_utf32_buffer_t {
    mla_pointer_t data;
    mla_size_t charCount; // Length of the UTF-32 string without the null terminator
};

const mla_utf_32_char_t* mla_string_utf32_data(const mla_string_utf32_buffer_t &p_Utf32Buffer);

// Bitmasks for extracting information
#define MLA_STRING_MEMORY_LOCATION_MASK 0xF0 // Mask for flags (11110000)
#define MLA_STRING_MEMORY_FORMAT_MASK   0x0F // Mask for type (00001111)

// Location flags (high bits)
#define MLA_STRING_LOCATION_EMBEDDED 0x10 // 00010000
#define MLA_STRING_LOCATION_HEAP     0x20 // 00100000
//#define MLA_STRING_LOCATION_NEXT_EXAMPLE 0x40 // 01000000

// Format types (low bits)
#define MLA_STRING_FORMAT_RAW        0x00
#define MLA_STRING_FORMAT_C_STRING   0x01
#define MLA_STRING_FORMAT_BUFFER     0x02
#define MLA_STRING_FORMAT_SUB_STRING 0x03

enum mla_string_memory_layout_t: mla_uint8_t {
    MLA_STRING_MEMORY_LAYOUT_EMBEDDED = MLA_STRING_LOCATION_EMBEDDED | MLA_STRING_FORMAT_RAW, // Embedded string (small string optimization)

    MLA_STRING_MEMORY_LAYOUT_C_STRING   = MLA_STRING_LOCATION_HEAP | MLA_STRING_FORMAT_C_STRING, // C-style string (with null terminator) which is located on in the heap
    MLA_STRING_MEMORY_LAYOUT_BUFFER     = MLA_STRING_LOCATION_HEAP | MLA_STRING_FORMAT_BUFFER, // Buffer-based string (without null terminator) which is located on in the heap
    MLA_STRING_MEMORY_LAYOUT_SUB_STRING = MLA_STRING_LOCATION_HEAP | MLA_STRING_FORMAT_SUB_STRING, // Substring (view into another string) which is located on in the heap
};

mla_pointer_t mla_create_char_array(const mla_size_t p_Length);


struct mla_multi_byte_char_t {
    mla_char_t bytes[4]; // Maximum 4 bytes for a UTF-8 character. UTF-8 can be 1 to 4 bytes. Bytes will be null-terminated.
};


mla_bool_t mla_string_equals(const mla_string_t &p_String1, const mla_string_t &p_String2);

struct mla_string_internal_heap_t {
    mla_string_memory_layout_t memoryLayout;
    mla_size_t length; // Buffer Length of the string. Not real Char count. This can be different in UTF8
    mla_size_t char_offset; // Offset which we need to add to the char index to get the correct byte index in the buffer. This is needed for substrings which are views into other strings
};

struct mla_string_internal_embedded_t {
    mla_string_memory_layout_t memoryLayout;
    mla_uint8_t length; // Length of the embedded string
    mla_char_t data[mla_global_config_string_sso_max_length];
};


struct mla_string_t {
    mla_pointer_t data_storage;
    union {
        mla_string_internal_embedded_t embedded;
        mla_string_internal_heap_t heap;
    };

    mla_bool_t operator==(const mla_string_t &other) const {

        mla_string_t thisString = *this;
        return mla_string_equals(thisString, other);
    }

    mla_bool_t operator!=(const mla_string_t &other) const {

        mla_string_t thisString = *this;
        return !mla_string_equals(thisString, other);
    }

    static mla_string_t init();
};

mla_string_t mla_string_empty();
mla_string_t mla_string_from_c_string(const mla_pointer_t& data, mla_size_t p_Length);
mla_string_t mla_string(const mla_pointer_t& data, mla_size_t p_Length);
mla_string_t mla_string(const mla_pointer_t& data);
mla_string_t mla_string(const mla_char_t *p_Data);
mla_string_t mla_string_copy(const mla_char_t *p_Data, mla_size_t p_Length);
mla_string_t mla_string_copy(const mla_string_t &p_String);



mla_size_t mla_string_length(const mla_string_t &p_String);
const mla_char_t *mla_string_data(const mla_string_t &p_String);

mla_int32_t mla_string_compare(const mla_string_t &p_String1, const mla_string_t &p_String2);
mla_int32_t mla_string_compare_ignore_case(const mla_string_t &p_String1, const mla_string_t &p_String2);

mla_string_t mla_string_to_lower(const mla_string_t &p_String);
mla_string_t mla_string_to_upper(const mla_string_t &p_String);

mla_bool_t mla_string_is_empty(const mla_string_t &p_String);
mla_bool_t mla_string_equals_ignore_case(const mla_string_t &p_String1, const mla_string_t &p_String2);

mla_bool_t mla_string_contains(const mla_string_t &p_String, const mla_string_t &p_Substring);
mla_bool_t mla_string_contains_ignore_case(const mla_string_t &p_String1, const mla_string_t &p_String2);

mla_string_t mla_string_replace(const mla_string_t &p_String, const mla_string_t &p_OldSubstring, const mla_string_t &p_NewSubstring);
mla_bool_t mla_string_starts_with(const mla_string_t &p_String, const mla_string_t &p_Prefix);
mla_bool_t mla_string_starts_with_ignore_case(const mla_string_t &p_String, const mla_string_t &p_Prefix);
mla_bool_t mla_string_ends_with(const mla_string_t &p_String, const mla_string_t &p_Suffix);
mla_bool_t mla_string_ends_with_ignore_case(const mla_string_t &p_String, const mla_string_t &p_Suffix);

mla_int32_t mla_string_index_of(const mla_string_t &p_String, const mla_string_t &p_Substring);
mla_int32_t mla_string_index_of(const mla_string_t &p_String, const mla_string_t &p_Substring, mla_size_t p_Start);
mla_int32_t mla_string_last_index_of(const mla_string_t &p_String, const mla_string_t &p_Substring);
mla_int32_t mla_string_last_index_of(const mla_string_t &p_String, const mla_string_t &p_Substring, mla_size_t p_Start);

mla_string_t mla_string_substr(const mla_string_t &p_String, mla_size_t p_Start);
mla_string_t mla_string_substr(const mla_string_t &p_String, mla_size_t p_Start, mla_size_t p_Length);
mla_string_t mla_string_repeat(const mla_string_t &p_String, mla_size_t p_Times);
mla_array_list_t<mla_init_struct(mla_string_t)> mla_string_split(const mla_string_t &p_String, const mla_string_t &p_Delimiter);
mla_string_t mla_string_join(const mla_array_list_t<mla_init_struct(mla_string_t)> &p_Strings, const mla_string_t &p_Delimiter);
mla_string_t mla_string_trim(const mla_string_t &p_String);

// This function returns a multi-byte character at the specified index
// the most time its fine if you just access the buffer directly
// but if you want to be sure you get the right character in UTF8 use this function
mla_multi_byte_char_t mla_string_multi_byte_char_at(const mla_string_t &p_String, mla_size_t p_Index);
mla_size_t mla_string_multi_byte_char_count(const mla_string_t &p_String);

mla_string_utf16_buffer_t mla_string_to_utf16_buffer(const mla_string_t &p_String);
mla_string_t mla_string_from_utf16_buffer(const mla_string_utf16_buffer_t &p_Utf16Buffer);

mla_string_utf32_buffer_t mla_string_to_utf32_buffer(const mla_string_t &p_String);
mla_string_t mla_string_from_utf32_buffer(const mla_string_utf32_buffer_t &p_Utf32Buffer);

// Memory layout conversion
// Note: Changing memory layout may involve copying data

mla_string_memory_layout_t mla_string_get_memory_layout(const mla_string_t &p_String);
mla_bool_t mla_string_change_memory_layout(mla_string_t &p_String, mla_string_memory_layout_t p_NewLayout);
mla_c_string_t mla_string_to_cString(const mla_string_t &p_String);
const mla_char_t* mla_c_string_data(const mla_c_string_t &p_CString);

// String creation from basic types
mla_string_t mla_string_from_int8(mla_int8_t p_Value);
mla_string_t mla_string_from_uint8(mla_uint8_t p_Value);
mla_string_t mla_string_from_int16(mla_int16_t p_Value);
mla_string_t mla_string_from_uint16(mla_uint16_t p_Value);
mla_string_t mla_string_from_int32(mla_int32_t p_Value);
mla_string_t mla_string_from_uint32(mla_uint32_t p_Value);
mla_string_t mla_string_from_int64(mla_int64_t p_Value);
mla_string_t mla_string_from_uint64(mla_uint64_t p_Value);
mla_string_t mla_string_from_size(mla_size_t p_Value);
mla_string_t mla_string_from_float(mla_float_t p_Value, mla_size_t p_DecimalPlaces);
mla_string_t mla_string_from_double(mla_double_t p_Value, mla_size_t p_DecimalPlaces);
mla_string_t mla_string_from_bool(mla_bool_t p_Value);

// Create string in format 0x0000...
mla_string_t mla_string_from_uint8_hex(mla_uint8_t p_Value);
mla_string_t mla_string_from_uint16_hex(mla_uint16_t p_Value);
mla_string_t mla_string_from_uint32_hex(mla_uint32_t p_Value);
mla_string_t mla_string_from_uint64_hex(mla_uint64_t p_Value);

// Create string in format FF
mla_string_t mla_string_from_uint8_hex_short(mla_uint8_t p_Value);
mla_string_t mla_string_from_uint16_hex_short(mla_uint16_t p_Value);
mla_string_t mla_string_from_uint32_hex_short(mla_uint32_t p_Value);
mla_string_t mla_string_from_uint64_hex_short(mla_uint64_t p_Value);


struct mla_string_hash_t {

    static mla_size_t hash(const mla_string_t& value) {
        // Call the optimized hash function from the hash module
        return mla_string_hash(mla_string_data(value), mla_string_length(value));
    }

};

// Helper to create mla_string_t from string literals
// Usage: mla_string_const("Hello World")
template<mla_size_t N>
mla_string_t mla_string_const(const mla_char_t (&literal)[N]) {

    if (N == 0) {
        return mla_string_empty();
    }

    mla_pointer_t str_ptr = mla_platform_pointer_to_managed_pointer(literal);
    return mla_string_from_c_string(str_ptr, N-1);  // N includes null terminator
}

# define mla_string_equals_const(value1, value2) mla_string_equals(value1, mla_string_const(value2))


#endif
