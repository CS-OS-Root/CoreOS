//
// Created by chris on 12/18/2025.
//

#ifndef MLA_REFLECTION_RPC_H
#define MLA_REFLECTION_RPC_H

#include "mla_reflection.h"
#include "../system/mla_string.h"
#include "../system/mla_array_list.h"
#include "../serializer/mla_serializer.h"

struct mla_reflection_rpc_struct_metadata_field_t {
    mla_string_t name;
    mla_reflection_type_t data_type;
    mla_reflection_type_t element_type; // For lists
    mla_string_t struct_name;

    static mla_reflection_rpc_struct_metadata_field_t init() {
        return { mla_string_empty(), MLA_REFLECTION_TYPE_INVALID, MLA_REFLECTION_TYPE_INVALID, mla_string_empty() };
    }

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {

        const mla_reflection_rpc_struct_metadata_field_t* field = mla_pointer_get_data<const mla_reflection_rpc_struct_metadata_field_t>(obj);

        if (field == nullptr) {
            return false;
        }

        mla_serializer_write_string(serializer, mla_string_const("name"), field->name);
        mla_serializer_write_enum(serializer, mla_string_const("element_type"), field->element_type);
        mla_serializer_write_string(serializer, mla_string_const("struct_name"), field->struct_name);
        mla_serializer_write_enum(serializer, mla_string_const("data_type"), field->data_type);
        return true;

    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {

        mla_reflection_rpc_struct_metadata_field_t* field = mla_pointer_get_data<mla_reflection_rpc_struct_metadata_field_t>(obj);

        if (field == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "name")) {
            mla_deserializer_read_string(deserializer, field->name);
        } else if (mla_string_equals_const(property_name, "data_type")) {
            mla_deserializer_read_enum(mla_reflection_type_t, deserializer, field->data_type);
        } else if (mla_string_equals_const(property_name, "element_type")) {
            mla_deserializer_read_enum(mla_reflection_type_t, deserializer, field->element_type);
        } else if (mla_string_equals_const(property_name, "struct_name")) {
            mla_deserializer_read_string(deserializer, field->struct_name);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }

    static mla_reflection_struct_metadata_t metadata() {

        mla_reflection_struct_metadata_t meta = mla_reflection_struct_metadata(mla_reflection_rpc_struct_metadata_field_t);
        mla_reflection_struct_field_string(meta, mla_reflection_rpc_struct_metadata_field_t, name);
        mla_reflection_struct_field_string(meta, mla_reflection_rpc_struct_metadata_field_t, struct_name);
        mla_reflection_struct_field_enum(meta, mla_reflection_rpc_struct_metadata_field_t, element_type);
        mla_reflection_struct_field_enum(meta, mla_reflection_rpc_struct_metadata_field_t, data_type);
        mla_reflection_struct_metadata_freeze(meta);
        return meta;
    }
};

struct mla_reflection_rpc_struct_metadata_t {
    mla_string_t name;
    mla_array_list_t<mla_init_struct(mla_reflection_rpc_struct_metadata_field_t)> fields;

    static mla_reflection_rpc_struct_metadata_t init() {
        return { mla_string_empty(), mla_array_list_empty<mla_init_struct(mla_reflection_rpc_struct_metadata_field_t)>() };
    }

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {

        const mla_reflection_rpc_struct_metadata_t* metadata = mla_pointer_get_data<const mla_reflection_rpc_struct_metadata_t>(obj);

        if (metadata == nullptr) {
            return false;
        }

        mla_serializer_write_string(serializer, mla_string_const("name"), metadata->name);
        mla_serializer_write_list_struct(serializer, mla_string_const("fields"), metadata->fields, mla_reflection_rpc_struct_metadata_field_t);
        return true;


    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {

        mla_reflection_rpc_struct_metadata_t* metadata = mla_pointer_get_data<mla_reflection_rpc_struct_metadata_t>(obj);

        if (metadata == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "name")) {
            mla_deserializer_read_string(deserializer, metadata->name);
        } else if (mla_string_equals_const(property_name, "fields")) {
            mla_deserializer_read_list_struct(deserializer, metadata->fields, mla_reflection_rpc_struct_metadata_field_t);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }

    }

    static mla_reflection_struct_metadata_t metadata() {
        mla_reflection_struct_metadata_t meta = mla_reflection_struct_metadata(mla_reflection_rpc_struct_metadata_t);
        mla_reflection_struct_field_string(meta, mla_reflection_rpc_struct_metadata_t, name);
        mla_reflection_struct_field_struct_list(meta, mla_reflection_rpc_struct_metadata_t, fields, mla_reflection_rpc_struct_metadata_field_t);
        mla_reflection_struct_metadata_freeze(meta);
        return meta;
    }
};

mla_reflection_rpc_struct_metadata_t mla_reflection_rpc_struct_metadata_invalid();

struct mla_reflection_struct_metadata_request_t {
    mla_string_t struct_name;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {

        const mla_reflection_struct_metadata_request_t* request = mla_pointer_get_data<const mla_reflection_struct_metadata_request_t>(obj);

        if (request == nullptr) {
            return false;
        }

        mla_serializer_write_string(serializer, mla_string_const("struct_name"), request->struct_name);
        return true;

    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {

        mla_reflection_struct_metadata_request_t* request = mla_pointer_get_data<mla_reflection_struct_metadata_request_t>(obj);

        if (request == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "struct_name")) {
            mla_deserializer_read_string(deserializer, request->struct_name);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }

    static mla_reflection_struct_metadata_t metadata() {

        mla_reflection_struct_metadata_t meta = mla_reflection_struct_metadata(mla_reflection_rpc_struct_metadata_t);
        mla_reflection_struct_field_string(meta, mla_reflection_struct_metadata_request_t, struct_name);
        mla_reflection_struct_metadata_freeze(meta);
        return meta;
    }
};

mla_reflection_struct_metadata_request_t mla_reflection_struct_metadata_request_empty();

struct mla_reflection_struct_metadata_response_t {

    mla_bool_t found;
    mla_reflection_rpc_struct_metadata_t meta_data;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {

        const mla_reflection_struct_metadata_response_t* response = mla_pointer_get_data<const mla_reflection_struct_metadata_response_t>(obj);

        if (response == nullptr) {
            return false;
        }

        mla_serializer_write_bool(serializer, mla_string_const("found"), response->found);
        mla_serializer_write_struct(serializer, mla_string_const("meta_data"), response->meta_data, mla_reflection_rpc_struct_metadata_t);
        return true;

    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {

        mla_reflection_struct_metadata_response_t* response = mla_pointer_get_data<mla_reflection_struct_metadata_response_t>(obj);

        if (response == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "meta_data")) {
            mla_deserializer_read_struct(deserializer, response->meta_data, mla_reflection_rpc_struct_metadata_t);
        } else if (mla_string_equals_const(property_name, "found")) {
            mla_deserializer_read_bool(deserializer, response->found);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }

    static mla_reflection_struct_metadata_t metadata() {

        mla_reflection_struct_metadata_t meta = mla_reflection_struct_metadata(mla_reflection_struct_metadata_response_t);
        mla_reflection_struct_field_bool(meta, mla_reflection_struct_metadata_response_t, found);
        mla_reflection_struct_field_struct(meta, mla_reflection_struct_metadata_response_t, meta_data, mla_reflection_rpc_struct_metadata_t);
        mla_reflection_struct_metadata_freeze(meta);
        return meta;
    }
};

mla_reflection_struct_metadata_response_t mla_reflection_struct_metadata_response_empty();

#define mla_rpc_procedure_get_meta_data_name "metadata/getMetaData"
#define mla_rpc_procedure_get_meta_data_signature mla_reflection_struct_metadata_request_t, mla_reflection_struct_metadata_response_t
mla_bool_t mla_reflection_metadata_rpc_get_metadata(const mla_reflection_struct_metadata_request_t* request, mla_reflection_struct_metadata_response_t* response);

#endif