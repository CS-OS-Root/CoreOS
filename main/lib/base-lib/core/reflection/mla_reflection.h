//
// Created by chris on 12/15/2025.
//

#ifndef MLA_REFLECTION_H
#define MLA_REFLECTION_H

#include "../lifecycle/mla_lifecycle_events.h"
#include "../system/mla_string.h"

enum mla_reflection_type_t: mla_uint8_t {
    MLA_REFLECTION_TYPE_INVALID = 0,
    MLA_REFLECTION_TYPE_BOOL,

    MLA_REFLECTION_TYPE_INT8,
    MLA_REFLECTION_TYPE_INT16,
    MLA_REFLECTION_TYPE_INT32,
    MLA_REFLECTION_TYPE_INT64,

    MLA_REFLECTION_TYPE_UINT8,
    MLA_REFLECTION_TYPE_UINT16,
    MLA_REFLECTION_TYPE_UINT32,
    MLA_REFLECTION_TYPE_UINT64,

    MLA_REFLECTION_TYPE_FLOAT,
    MLA_REFLECTION_TYPE_DOUBLE,

    MLA_REFLECTION_TYPE_STRING,
    MLA_REFLECTION_TYPE_BYTES,

    MLA_REFLECTION_TYPE_STRUCT,
    MLA_REFLECTION_TYPE_LIST,
};

struct mla_reflection_struct_metadata_t;

typedef mla_reflection_struct_metadata_t (*mla_reflection_struct_metadata_provider_t)();

struct mla_reflection_struct_field_t {
    mla_string_t name;
    mla_size_t offset;
    mla_reflection_type_t type;
    mla_reflection_type_t element_type; // For lists
    mla_reflection_struct_metadata_provider_t struct_provider;

    static mla_reflection_struct_field_t init() {
        return { mla_string_empty(), 0, MLA_REFLECTION_TYPE_INVALID, MLA_REFLECTION_TYPE_INVALID, nullptr };
    }
};

struct mla_reflection_struct_metadata_t {
    mla_string_t name;
    mla_size_t size;
    mla_bool_t frozen;
    mla_array_list_t<mla_init_struct(mla_reflection_struct_field_t)> fields;

    static mla_reflection_struct_metadata_t init() {
        return { mla_string_empty(), 0, false, mla_array_list_empty<mla_init_struct(mla_reflection_struct_field_t)>() };
    }
};

mla_reflection_struct_metadata_t mla_reflection_struct_metadata_invalid();

mla_reflection_struct_metadata_t mla_reflection_struct_metadata_for(const mla_string_t &name, mla_size_t size);

#define mla_reflection_struct_name(T) #T
#define mla_reflection_struct_name_as_string(T) mla_string_const(#T)

#define mla_reflection_struct_metadata(T) \
mla_reflection_struct_metadata_for(mla_reflection_struct_name_as_string(T), sizeof(T))

void mla_reflection_struct_metadata_add_field(mla_reflection_struct_metadata_t& metadata, const mla_reflection_struct_field_t &field);
void mla_reflection_struct_metadata_freeze(mla_reflection_struct_metadata_t& metadata);
mla_reflection_struct_field_t mla_reflection_struct_field(const mla_string_t &name, mla_size_t offset, mla_reflection_type_t type, mla_reflection_type_t element_type, const mla_reflection_struct_metadata_provider_t& struct_metadata_provider);

typedef mla_reflection_struct_metadata_t (*mla_reflection_struct_metadata_provider_t)();

mla_bool_t mla_reflection_register_struct(const mla_string_t &name, mla_reflection_struct_metadata_provider_t provider);
mla_bool_t mla_reflection_is_struct_registered(const mla_string_t &name);
mla_bool_t mla_reflection_get_struct_metadata(const mla_string_t &name, mla_reflection_struct_metadata_t& out_metadata);
void mla_reflection_register_inner_structs(const mla_reflection_struct_metadata_t& metadata);

template<typename T>
mla_bool_t mla_reflection_register_struct()
{
    mla_reflection_struct_metadata_t metadata = T::metadata();
    mla_bool_t result = mla_reflection_register_struct(metadata.name, T::metadata);

    if (result) {
        mla_reflection_register_inner_structs(metadata);
    }

    return result;
}

#define mla_reflection_auto_register_struct(sturct_type) \
void mla_reflection_auto_register_struct_##sturct_type() { \
    mla_reflection_register_struct<sturct_type>();  \
} \
mla_lifecycle_boot_event_static_register(mla_lifecycle_boot_event_priority_low_level_postSetup, mla_reflection_auto_register_struct_##sturct_type) \

#define mla_reflection_struct_field_bool(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_BOOL, MLA_REFLECTION_TYPE_INVALID, nullptr))

#define mla_reflection_struct_field_int8(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_INT8, MLA_REFLECTION_TYPE_INVALID, nullptr))

#define mla_reflection_struct_field_int16(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_INT16, MLA_REFLECTION_TYPE_INVALID, nullptr))

#define mla_reflection_struct_field_int32(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_INT32, MLA_REFLECTION_TYPE_INVALID, nullptr))

#define mla_reflection_struct_field_int64(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_INT64, MLA_REFLECTION_TYPE_INVALID, nullptr))

#define mla_reflection_struct_field_uint8(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_UINT8, MLA_REFLECTION_TYPE_INVALID, nullptr))

#define mla_reflection_struct_field_enum(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_UINT8, MLA_REFLECTION_TYPE_INVALID, nullptr))

#define mla_reflection_struct_field_uint16(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_UINT16, MLA_REFLECTION_TYPE_INVALID, nullptr))

#define mla_reflection_struct_field_uint32(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_UINT32, MLA_REFLECTION_TYPE_INVALID, nullptr))

#define mla_reflection_struct_field_uint64(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_UINT64, MLA_REFLECTION_TYPE_INVALID, nullptr))

#define mla_reflection_struct_field_float(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_FLOAT, MLA_REFLECTION_TYPE_INVALID, nullptr))

#define mla_reflection_struct_field_double(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_DOUBLE, MLA_REFLECTION_TYPE_INVALID, nullptr))

#define mla_reflection_struct_field_string(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_STRING, MLA_REFLECTION_TYPE_INVALID, nullptr))

#define mla_reflection_struct_field_bytes(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_BYTES, MLA_REFLECTION_TYPE_INVALID, nullptr))

#define mla_reflection_struct_field_struct(data, struct_type, field_name, field_struct_type) \
    mla_reflection_struct_metadata_add_field(data, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_STRUCT, MLA_REFLECTION_TYPE_INVALID, field_struct_type::metadata))

#define mla_reflection_struct_field_struct_list(data, struct_type, field_name, element_struct_type) \
    mla_reflection_struct_metadata_add_field(data, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_LIST, MLA_REFLECTION_TYPE_STRUCT, element_struct_type::metadata))

#define mla_reflection_struct_field_bool_list(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_LIST, MLA_REFLECTION_TYPE_BOOL, nullptr))

#define mla_reflection_struct_field_int8_list(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_LIST, MLA_REFLECTION_TYPE_INT8, nullptr))

#define mla_reflection_struct_field_int16_list(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_LIST, MLA_REFLECTION_TYPE_INT16, nullptr))

#define mla_reflection_struct_field_int32_list(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_LIST, MLA_REFLECTION_TYPE_INT32, nullptr))

#define mla_reflection_struct_field_int64_list(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_LIST, MLA_REFLECTION_TYPE_INT64, nullptr))

#define mla_reflection_struct_field_uint8_list(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_LIST, MLA_REFLECTION_TYPE_UINT8, nullptr))

#define mla_reflection_struct_field_uint16_list(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_LIST, MLA_REFLECTION_TYPE_UINT16, nullptr))

#define mla_reflection_struct_field_uint32_list(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_LIST, MLA_REFLECTION_TYPE_UINT32, nullptr))

#define mla_reflection_struct_field_uint64_list(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_LIST, MLA_REFLECTION_TYPE_UINT64, nullptr))

#define mla_reflection_struct_field_float_list(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_LIST, MLA_REFLECTION_TYPE_FLOAT, nullptr))

#define mla_reflection_struct_field_double_list(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_LIST, MLA_REFLECTION_TYPE_DOUBLE, nullptr))

#define mla_reflection_struct_field_string_list(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_LIST, MLA_REFLECTION_TYPE_STRING, nullptr))


#endif