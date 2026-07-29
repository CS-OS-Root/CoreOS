//
// Basic serialization interface for structured data.
//
// Created by chris on 9/14/2025.
//

#ifndef MLA_SERIALIZER_H
#define MLA_SERIALIZER_H

#include "../system/mla_string.h"
#include "../system/mla_stream.h"
#include "../system/mla_array_list.h"
#include "../system/mla_bytes.h"

struct mla_serializer_t {

    mla_stream_output_t output;
    mla_user_data_t user_data;

    mla_bool_t (*write_start_struct)(mla_serializer_t& instance);
    mla_bool_t (*write_end_struct)(mla_serializer_t& instance);

    mla_bool_t (*write_start_list)(mla_serializer_t& instance);
    mla_bool_t (*write_end_list)(mla_serializer_t& instance);

    mla_bool_t (*write_property_name)(mla_serializer_t& instance, const mla_string_t& name);

    mla_bool_t (*write_bool)(mla_serializer_t& instance, const mla_bool_t value);

    mla_bool_t (*write_int8)(mla_serializer_t& instance, const mla_int8_t value);
    mla_bool_t (*write_int16)(mla_serializer_t& instance, const mla_int16_t value);
    mla_bool_t (*write_int32)(mla_serializer_t& instance, const mla_int32_t value);
    mla_bool_t (*write_int64)(mla_serializer_t& instance, const mla_int64_t value);
    mla_bool_t (*write_uint8)(mla_serializer_t& instance, const mla_uint8_t value);
    mla_bool_t (*write_uint16)(mla_serializer_t& instance, const mla_uint16_t value);
    mla_bool_t (*write_uint32)(mla_serializer_t& instance, const mla_uint32_t value);
    mla_bool_t (*write_uint64)(mla_serializer_t& instance, const mla_uint64_t value);

    mla_bool_t (*write_float)(mla_serializer_t& instance, const mla_float_t value);
    mla_bool_t (*write_double)(mla_serializer_t& instance, const mla_double_t value);

    mla_bool_t (*write_string)(mla_serializer_t& instance, const mla_string_t& value);
    mla_bool_t (*write_bytes)(mla_serializer_t& instance, const mla_bytes_t& bytes);

};

mla_serializer_t mla_serializer_invalid();
mla_bool_t mla_serializer_is_invalid(const mla_serializer_t& serializer);

enum mla_deserializer_token_type_t : mla_uint8_t {
    MLA_DESERIALIZER_NULL = 0,
    MLA_DESERIALIZER_PROPERTY_NAME = 1,
    MLA_DESERIALIZER_STRUCT_START = 2,
    MLA_DESERIALIZER_STRUCT_END = 3,
    MLA_DESERIALIZER_LIST_START = 4,
    MLA_DESERIALIZER_LIST_END = 5,

    // Value types
    MLA_DESERIALIZER_VALUE_BOOL = 6,
    MLA_DESERIALIZER_VALUE_INT8 = 7,
    MLA_DESERIALIZER_VALUE_INT16 = 8,
    MLA_DESERIALIZER_VALUE_INT32 = 9,
    MLA_DESERIALIZER_VALUE_INT64 = 10,
    MLA_DESERIALIZER_VALUE_UINT8 = 11,
    MLA_DESERIALIZER_VALUE_UINT16 = 12,
    MLA_DESERIALIZER_VALUE_UINT32 = 13,
    MLA_DESERIALIZER_VALUE_UINT64 = 14,
    MLA_DESERIALIZER_VALUE_FLOAT = 15,
    MLA_DESERIALIZER_VALUE_DOUBLE = 16,
    MLA_DESERIALIZER_VALUE_STRING = 17,
    MLA_DESERIALIZER_VALUE_BYTES = 18
};

mla_bool_t mla_deserializer_token_type_is_value(const mla_deserializer_token_type_t& token_type);

struct mla_deserializer_token_t {
    mla_deserializer_token_type_t type;

    struct {
        // All reference bytes an not be stored in an union
        // because they are not used at the same time as the value types
        mla_string_t property_name;
        mla_string_t string_value;
        mla_bytes_t bytes_value;
    } complex;

    union {

        mla_bool_t bool_value;
        mla_int8_t int8_value;
        mla_int16_t int16_value;
        mla_int32_t int32_value;
        mla_int64_t int64_value;
        mla_uint8_t uint8_value;
        mla_uint16_t uint16_value;
        mla_uint32_t uint32_value;
        mla_uint64_t uint64_value;
        mla_float_t float_value;
        mla_double_t double_value;
    } simple;
};

struct mla_deserializer_t {

    mla_stream_input_t input;
    mla_user_data_t user_data;

    mla_deserializer_token_t current_token;
    mla_bool_t (*read_next)(mla_deserializer_t& instance);
};

mla_deserializer_t mla_deserializer_invalid();
mla_bool_t mla_deserializer_is_invalid(const mla_deserializer_t& deserializer);

enum mla_deserializer_read_result_t : mla_uint8_t {
    MLA_DESERIALIZER_READ_HANDLED,
    MLA_DESERIALIZER_READ_SKIPPED,
    MLA_DESERIALIZER_READ_ERROR
};

// Called for every property in a struct
typedef mla_deserializer_read_result_t (*mla_serialize_definition_read_function_t)(mla_deserializer_t& deserializer, mla_pointer_t& config, const mla_string_t& property_name);

// Called to write the entire struct
typedef mla_bool_t (*mla_serialize_definition_write_function_t)(mla_serializer_t& serializer, const mla_pointer_t& config);

/// Definition of serialization functions for a specific data structure
struct mla_serialize_definition_t {
    mla_size_t data_size;
    mla_serialize_definition_read_function_t read_function;
    mla_serialize_definition_write_function_t write_function;
};

mla_serialize_definition_t mla_serialize_definition_invalid();

mla_serialize_definition_t mla_serialize_definition_create(
        mla_size_t data_size,
        const mla_serialize_definition_read_function_t& read_function,
        const mla_serialize_definition_write_function_t& write_function
);

/////////////////////////////////////////////////////////////////////////////
/// Serializer Helpers
//////////////////////////////////////////////////////////////////////////////

mla_bool_t mla_serializer_write_data_struct(mla_serializer_t& serializer, const mla_pointer_t& value, const mla_serialize_definition_write_function_t& write_function);
mla_bool_t mla_serializer_write_data_struct(mla_serializer_t& serializer, const mla_string_t& name, const mla_pointer_t& value, const mla_serialize_definition_write_function_t& write_function);

mla_bool_t mla_serializer_write_data_list(mla_serializer_t& serializer, const mla_string_t& name, const mla_array_list_t<mla_bool_t>& list);
mla_bool_t mla_serializer_write_data_list(mla_serializer_t& serializer, const mla_string_t& name, const mla_array_list_t<mla_uint8_t>& list);
mla_bool_t mla_serializer_write_data_list(mla_serializer_t& serializer, const mla_string_t& name, const mla_array_list_t<mla_uint16_t>& list);
mla_bool_t mla_serializer_write_data_list(mla_serializer_t& serializer, const mla_string_t& name, const mla_array_list_t<mla_uint32_t>& list);
mla_bool_t mla_serializer_write_data_list(mla_serializer_t& serializer, const mla_string_t& name, const mla_array_list_t<mla_uint64_t>& list);
mla_bool_t mla_serializer_write_data_list(mla_serializer_t& serializer, const mla_string_t& name, const mla_array_list_t<mla_int8_t>& list);
mla_bool_t mla_serializer_write_data_list(mla_serializer_t& serializer, const mla_string_t& name, const mla_array_list_t<mla_int16_t>& list);
mla_bool_t mla_serializer_write_data_list(mla_serializer_t& serializer, const mla_string_t& name, const mla_array_list_t<mla_int32_t>& list);
mla_bool_t mla_serializer_write_data_list(mla_serializer_t& serializer, const mla_string_t& name, const mla_array_list_t<mla_int64_t>& list);
mla_bool_t mla_serializer_write_data_list(mla_serializer_t& serializer, const mla_string_t& name, const mla_array_list_t<mla_float_t>& list);
mla_bool_t mla_serializer_write_data_list(mla_serializer_t& serializer, const mla_string_t& name, const mla_array_list_t<mla_double_t>& list);
mla_bool_t mla_serializer_write_data_list(mla_serializer_t& serializer, const mla_string_t& name, const mla_array_list_t<mla_init_struct(mla_string_t)>& list);

template <typename T>
mla_bool_t mla_serializer_write_data_struct(mla_serializer_t& serializer, const T &value) {
    mla_pointer_t value_ptr = mla_platform_pointer_to_managed_pointer(&value);
    return mla_serializer_write_data_struct(serializer, value_ptr, T::serialize);
}

template <mla_array_list_template>
mla_bool_t mla_serializer_write_list(mla_serializer_t& serializer, const mla_string_t& name, const mla_array_list_t<T, TInit>& list, const mla_serialize_definition_write_function_t& write_function) {

    if (!serializer.write_property_name(serializer, name)) {
        return false;
    }


    if (!serializer.write_start_list(serializer)) {
        return false;
    }

    for (mla_size_t i = 0; i < mla_array_list_size(list); ++i) {

        mla_pointer_t data = mla_platform_pointer_to_managed_pointer(mla_array_list_get_ref(list, i));

        if (!mla_serializer_write_data_struct(serializer, data, write_function)) {
            return false;
        }

    }

    if (!serializer.write_end_list(serializer)) {
        return false;
    }

    return true;

}

template <mla_array_list_template>
mla_bool_t mla_serializer_write_list_fixed_size(mla_serializer_t& serializer, const mla_string_t& name, const T* data, const mla_serialize_definition_write_function_t& write_function, mla_size_t size) {

    if (!serializer.write_property_name(serializer, name)) {
        return false;
    }

    if (!serializer.write_start_list(serializer)) {
        return false;
    }

    for (mla_size_t i = 0; i < size; ++i) {

        mla_pointer_t data_ptr = mla_platform_pointer_to_managed_pointer(&data[i]);

        if (!mla_serializer_write_data_struct(serializer, data_ptr, write_function)) {
            return false;
        }
    }

    if (!serializer.write_end_list(serializer)) {
        return false;
    }

    return true;

}

//////////////////////////////////////////////////////////////////////////////////
/// Meta Definitions Helpers
///////////////////////////////////////////////////////////////////////////////

template<typename T>
mla_serialize_definition_t mla_serialize_definition()
{
    return mla_serialize_definition_create(sizeof(T),
                                           T::deserialize,
                                           T::serialize);
}

#define mla_serialize_definition_void() \
mla_serialize_definition_create(0, void_deserialize, void_serialize)

// Helpers for void datatypes
mla_bool_t void_serialize(mla_serializer_t& serializer, const mla_pointer_t& obj);
mla_deserializer_read_result_t void_deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name);

//////////////////////////////////////////////////////////////////////////////////
/// Deserializer Helpers
//////////////////////////////////////////////////////////////////////////////////

mla_bool_t mla_deserializer_read_struct_read_function(mla_deserializer_t& deserializer, mla_pointer_t& data, const mla_serialize_definition_read_function_t& read_function);

template <typename T>
mla_bool_t mla_serializer_read_data_struct(mla_deserializer_t& deserializer, T &value) {
    if (!deserializer.read_next(deserializer)) {
        return false;
    }

    mla_pointer_t value_ptr = mla_platform_pointer_to_managed_pointer(&value);

    return mla_deserializer_read_struct_read_function(deserializer, value_ptr, T::deserialize);
}

mla_bool_t mla_serializer_read_list(mla_deserializer_t& deserializer, mla_array_list_t<mla_bool_t>& list);
mla_bool_t mla_serializer_read_list(mla_deserializer_t& deserializer, mla_array_list_t<mla_uint8_t>& list);
mla_bool_t mla_serializer_read_list(mla_deserializer_t& deserializer, mla_array_list_t<mla_uint16_t>& list);
mla_bool_t mla_serializer_read_list(mla_deserializer_t& deserializer, mla_array_list_t<mla_uint32_t>& list);
mla_bool_t mla_serializer_read_list(mla_deserializer_t& deserializer, mla_array_list_t<mla_uint64_t>& list);
mla_bool_t mla_serializer_read_list(mla_deserializer_t& deserializer, mla_array_list_t<mla_int8_t>& list);
mla_bool_t mla_serializer_read_list(mla_deserializer_t& deserializer, mla_array_list_t<mla_int16_t>& list);
mla_bool_t mla_serializer_read_list(mla_deserializer_t& deserializer, mla_array_list_t<mla_int32_t>& list);
mla_bool_t mla_serializer_read_list(mla_deserializer_t& deserializer, mla_array_list_t<mla_int64_t>& list);
mla_bool_t mla_serializer_read_list(mla_deserializer_t& deserializer, mla_array_list_t<mla_float_t>& list);
mla_bool_t mla_serializer_read_list(mla_deserializer_t& deserializer, mla_array_list_t<mla_double_t>& list);
mla_bool_t mla_serializer_read_list(mla_deserializer_t& deserializer, mla_array_list_t<mla_init_struct(mla_string_t)>& list);

template <mla_array_list_template>
mla_bool_t mla_serializer_read_list(mla_deserializer_t& deserializer, mla_array_list_t<T, TInit>& list, const mla_serialize_definition_read_function_t& read_function) {

    if (deserializer.current_token.type == MLA_DESERIALIZER_NULL) {
        return true;
    }

    if (deserializer.current_token.type == MLA_DESERIALIZER_LIST_START) {

        while (deserializer.read_next(deserializer)) {

            if (deserializer.current_token.type == MLA_DESERIALIZER_LIST_END) {
                // We are done
                return true;
            }

            if (deserializer.current_token.type != MLA_DESERIALIZER_STRUCT_START) {
                // Wrong struct ure
                return false;
            }

            T item = TInit::init();
            mla_pointer_t item_ptr = mla_platform_pointer_to_managed_pointer(&item);
            if (mla_deserializer_read_struct_read_function(deserializer, item_ptr, read_function)) {
                mla_array_list_add(list, item);
            } else {
                // Error reading struct
                return false;
            }

        }
    }

    if (deserializer.current_token.type != MLA_DESERIALIZER_LIST_END) {
        // Expected LIST_END
        return false;
    }

    return true;
}

template <mla_array_list_template>
mla_bool_t mla_serializer_read_list_fixed_size(mla_deserializer_t& deserializer, T* data, const mla_serialize_definition_read_function_t& read_function, mla_size_t size) {

    if (deserializer.current_token.type == MLA_DESERIALIZER_NULL) {
        return true;
    }

    if (deserializer.current_token.type == MLA_DESERIALIZER_LIST_START) {

        mla_size_t index = 0;

        while (deserializer.read_next(deserializer)) {

            if (deserializer.current_token.type == MLA_DESERIALIZER_LIST_END) {
                // We are done
                return true;
            }

            if (deserializer.current_token.type != MLA_DESERIALIZER_STRUCT_START) {
                // Wrong struct ure
                return false;
            }

            T item = TInit::init();

            mla_pointer_t item_ptr = mla_platform_pointer_to_managed_pointer(&item);

            if (mla_deserializer_read_struct_read_function(deserializer, item_ptr, read_function)) {

                if (index >= size) {
                    // Too many items in the list
                    return false;
                }

                data[index] = item;
                index++;
            } else {
                // Error reading struct
                return false;
            }

        }
    }

    if (deserializer.current_token.type != MLA_DESERIALIZER_LIST_END) {
        // Expected LIST_END
        return false;
    }

    return true;
}

void mla_deserializer_skip_property_value(mla_deserializer_t& deserializer);

///////////////////////////////////////////////////////////////////////////////
/// Conversion Helpers
//////////////////////////////////////////////////////////////////////////////

mla_bool_t mla_deserializer_convert_to_bool(const mla_deserializer_token_t& token, mla_bool_t* out_value);
mla_bool_t mla_deserializer_convert_to_int8(const mla_deserializer_token_t& token, mla_int8_t* out_value);
mla_bool_t mla_deserializer_convert_to_int16(const mla_deserializer_token_t& token, mla_int16_t* out_value);
mla_bool_t mla_deserializer_convert_to_int32(const mla_deserializer_token_t& token, mla_int32_t* out_value);
mla_bool_t mla_deserializer_convert_to_int64(const mla_deserializer_token_t& token, mla_int64_t* out_value);
mla_bool_t mla_deserializer_convert_to_uint8(const mla_deserializer_token_t& token, mla_uint8_t* out_value);
mla_bool_t mla_deserializer_convert_to_uint16(const mla_deserializer_token_t& token, mla_uint16_t* out_value);
mla_bool_t mla_deserializer_convert_to_uint32(const mla_deserializer_token_t& token, mla_uint32_t* out_value);
mla_bool_t mla_deserializer_convert_to_uint64(const mla_deserializer_token_t& token, mla_uint64_t* out_value);
mla_bool_t mla_deserializer_convert_to_float(const mla_deserializer_token_t& token, mla_float_t* out_value);
mla_bool_t mla_deserializer_convert_to_double(const mla_deserializer_token_t& token, mla_double_t* out_value);
mla_bool_t mla_deserializer_convert_to_string(const mla_deserializer_token_t& token, mla_string_t* out_value);
mla_bool_t mla_deserializer_convert_to_bytes(const mla_deserializer_token_t& token, mla_bytes_t* out_value);


///////////////////////////////////////////////////////////////////////////////
/// Macros for easier usage
//////////////////////////////////////////////////////////////////////////////

// Deserializer
#define mla_deserializer_read_bool(instance, setter)\
    {\
    mla_bool_t value = false;\
    if (mla_deserializer_convert_to_bool((instance).current_token, &value)) {\
        (setter) = value;\
        return MLA_DESERIALIZER_READ_HANDLED;\
    } else {\
        return MLA_DESERIALIZER_READ_ERROR;\
    }\
    }

#define mla_deserializer_read_int8(instance, setter)\
    {\
    mla_int8_t value = 0;\
    if (mla_deserializer_convert_to_int8((instance).current_token, &value)) {\
        (setter) = value;\
        return MLA_DESERIALIZER_READ_HANDLED;\
    } else {\
        return MLA_DESERIALIZER_READ_ERROR;\
    }\
    }

#define mla_deserializer_read_int16(instance, setter)\
    {\
    mla_int16_t value = 0;\
    if (mla_deserializer_convert_to_int16((instance).current_token, &value)) {\
        (setter) = value;\
        return MLA_DESERIALIZER_READ_HANDLED;\
    } else {\
        return MLA_DESERIALIZER_READ_ERROR;\
    }\
    }

#define mla_deserializer_read_int32(instance, setter)\
    {\
    mla_int32_t value = 0;\
    if (mla_deserializer_convert_to_int32((instance).current_token, &value)) {\
        (setter) = value;\
        return MLA_DESERIALIZER_READ_HANDLED;\
    } else {\
        return MLA_DESERIALIZER_READ_ERROR;\
    }\
    }

#define mla_deserializer_read_int64(instance, setter)\
    {\
    mla_int64_t value = 0;\
    if (mla_deserializer_convert_to_int64((instance).current_token, &value)) {\
        (setter) = value;\
        return MLA_DESERIALIZER_READ_HANDLED;\
    } else {\
        return MLA_DESERIALIZER_READ_ERROR;\
    }\
    }

#define mla_deserializer_read_uint8(instance, setter)\
    {\
    mla_uint8_t value = 0;\
    if (mla_deserializer_convert_to_uint8((instance).current_token, &value)) {\
        (setter) = value;\
        return MLA_DESERIALIZER_READ_HANDLED;\
    } else {\
        return MLA_DESERIALIZER_READ_ERROR;\
    }\
    }

#define mla_deserializer_read_enum(enum_type, instance, setter)\
    {\
    mla_uint8_t value = 0;\
    if (mla_deserializer_convert_to_uint8((instance).current_token, &value)) {\
        (setter) = mla_s_cast<enum_type>(value);\
        return MLA_DESERIALIZER_READ_HANDLED;\
    } else {\
        return MLA_DESERIALIZER_READ_ERROR;\
    }\
    }

#define mla_deserializer_read_uint16(instance, setter)\
    {\
    mla_uint16_t value = 0;\
    if (mla_deserializer_convert_to_uint16((instance).current_token, &value)) {\
        (setter) = value;\
        return MLA_DESERIALIZER_READ_HANDLED;\
    } else {\
        return MLA_DESERIALIZER_READ_ERROR;\
    }\
    }

#define mla_deserializer_read_uint32(instance, setter)\
    {\
    mla_uint32_t value = 0;\
    if (mla_deserializer_convert_to_uint32((instance).current_token, &value)) {\
        (setter) = value;\
        return MLA_DESERIALIZER_READ_HANDLED;\
    } else {\
        return MLA_DESERIALIZER_READ_ERROR;\
    }\
    }

#define mla_deserializer_read_uint64(instance, setter)\
    {\
    mla_uint64_t value = 0;\
    if (mla_deserializer_convert_to_uint64((instance).current_token, &value)) {\
        (setter) = value;\
        return MLA_DESERIALIZER_READ_HANDLED;\
    } else {\
        return MLA_DESERIALIZER_READ_ERROR;\
    }\
    }

#define mla_deserializer_read_float(instance, setter)\
    {\
    mla_float_t value = 0;\
    if (mla_deserializer_convert_to_float((instance).current_token, &value)) {\
        (setter) = value;\
        return MLA_DESERIALIZER_READ_HANDLED;\
    } else {\
        return MLA_DESERIALIZER_READ_ERROR;\
    }\
    }

#define mla_deserializer_read_double(instance, setter)\
    {\
    mla_double_t value = 0;\
    if (mla_deserializer_convert_to_double((instance).current_token, &value)) {\
        (setter) = value;\
        return MLA_DESERIALIZER_READ_HANDLED;\
    } else {\
        return MLA_DESERIALIZER_READ_ERROR;\
    }\
    }

#define mla_deserializer_read_string(instance, setter)\
    {\
    mla_string_t value = mla_string_empty();\
    if (mla_deserializer_convert_to_string((instance).current_token, &value)) {\
        (setter) = value;\
        return MLA_DESERIALIZER_READ_HANDLED;\
    } else {\
        return MLA_DESERIALIZER_READ_ERROR;\
    }\
    }

#define mla_deserializer_read_bytes(instance, setter)\
    {\
    mla_bytes_t value = mla_bytes_empty();\
    if (mla_deserializer_convert_to_bytes((instance).current_token, &value)) {\
        (setter) = value;\
        return MLA_DESERIALIZER_READ_HANDLED;\
    } else {\
        return MLA_DESERIALIZER_READ_ERROR;\
    }\
    }

#define mla_deserializer_read_list_struct(instance, setter, struct_data)\
    {\
    if (mla_serializer_read_list<struct_data>((instance), (setter), struct_data::deserialize)) {\
        return MLA_DESERIALIZER_READ_HANDLED;\
    } else {\
        return MLA_DESERIALIZER_READ_ERROR;\
    }\
    }

#define mla_deserializer_read_list_struct_fixed_size(instance, setter, struct_data)\
    {\
    if (mla_serializer_read_list_fixed_size<struct_data>((instance), (setter), struct_data::deserialize, sizeof(setter))) {\
        return MLA_DESERIALIZER_READ_HANDLED;\
    } else {\
        return MLA_DESERIALIZER_READ_ERROR;\
    }\
    }

#define mla_deserializer_read_struct(instance, setter, struct_data)\
    {\
        mla_pointer_t setter_data = mla_platform_pointer_to_managed_pointer(&(setter));\
        if (mla_deserializer_read_struct_read_function((instance), setter_data, struct_data::deserialize)) {\
            return MLA_DESERIALIZER_READ_HANDLED;\
        } else {\
            return MLA_DESERIALIZER_READ_ERROR;\
        }\
    }

// Serializer

#define mla_serializer_write_bool(instance, name, value) \
    if (!(instance).write_property_name(instance, name)) { \
        return false;\
    }\
    if (!(instance).write_bool(instance, value)) {\
        return false;\
    }

#define mla_serializer_write_int8(instance, name, value)\
    if (!(instance).write_property_name(instance, name)) {\
        return false;\
    }\
    if (!(instance).write_int8(instance, value)) {\
        return false;\
    }

#define mla_serializer_write_int16(instance, name, value)\
    if (!(instance).write_property_name(instance, name)) {\
        return false;\
    }\
    if (!(instance).write_int16(instance, value)) {\
        return false;\
    }

#define mla_serializer_write_int32(instance, name, value)\
    if (!(instance).write_property_name(instance, name)) {\
        return false;\
    }\
    if (!(instance).write_int32(instance, value)) {\
        return false;\
    }

#define mla_serializer_write_int64(instance, name, value)\
    if (!(instance).write_property_name(instance, name)) {\
        return false;\
    }\
    if (!(instance).write_int64(instance, value)) {\
        return false;\
    }

#define mla_serializer_write_uint8(instance, name, value)\
    if (!(instance).write_property_name(instance, name)) {\
        return false;\
    }\
    if (!(instance).write_uint8(instance, value)) {\
        return false;\
    }

#define mla_serializer_write_enum(instance, name, value)\
    if (!(instance).write_property_name(instance, name)) {\
        return false;\
    }\
    if (!(instance).write_uint8(instance, mla_s_cast<mla_uint8_t>(value))) {\
        return false;\
    }

#define mla_serializer_write_uint16(instance, name, value)\
    if (!(instance).write_property_name(instance, name)) {\
        return false;\
    }\
    if (!(instance).write_uint16(instance, value)) {\
        return false;\
    }

#define mla_serializer_write_uint32(instance, name, value)\
    if (!(instance).write_property_name(instance, name)) {\
        return false;\
    }\
    if (!(instance).write_uint32(instance, value)) {\
        return false;\
    }

#define mla_serializer_write_uint64(instance, name, value)\
    if (!(instance).write_property_name(instance, name)) {\
        return false;\
    }\
    if (!(instance).write_uint64(instance, value)) {\
        return false;\
    }

#define mla_serializer_write_float(instance, name, value)\
    if (!(instance).write_property_name(instance, name)) {\
        return false;\
    }\
    if (!(instance).write_float(instance, value)) {\
        return false;\
    }

#define mla_serializer_write_double(instance, name, value)\
    if (!(instance).write_property_name(instance, name)) {\
        return false;\
    }\
    if (!(instance).write_double(instance, value)) {\
        return false;\
    }

#define mla_serializer_write_string(instance, name, value)\
    if (!(instance).write_property_name(instance, name)) {\
        return false;\
    }\
    if (!(instance).write_string(instance, value)) {\
        return false;\
    }

#define mla_serializer_write_bytes(instance, name, bytes)\
    if (!(instance).write_property_name(instance, name)) {\
        return false;\
    }\
    if (!(instance).write_bytes(instance, bytes)) {\
        return false;\
    }



#define mla_serializer_write_list(instance, name, list)\
    if (!mla_serializer_write_data_list(instance, name, list)) {\
        return false;\
    }

#define mla_serializer_write_list_struct(instance, name, list, struct_data)\
    if (!mla_serializer_write_list<struct_data>(instance, name, list, struct_data::serialize)) {\
        return false;\
    }

#define mla_serializer_write_list_struct_fixed_size(instance, name, list, struct_data)\
    if (!mla_serializer_write_list_fixed_size<struct_data>(instance, name, list, struct_data::serialize, sizeof(list))) {\
        return false;\
    }

#define mla_serializer_write_struct(instance, name, value, struct_data)\
    if (!mla_serializer_write_data_struct(instance, name, mla_platform_pointer_to_managed_pointer(&(value)), struct_data::serialize)) {\
        return false;\
    }


#define mla_serializer_write_struct_write_function(instance, name, value, write_function)\
    if (!mla_serializer_write_data_struct(instance, name, value, write_function)) {\
        return false;\
    }

#endif