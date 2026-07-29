//
// Created by chris on 12/15/2025.
//

#include "mla_reflection.h"
#include "../log/mla_logging.h"
#include "../system/mla_string_concat.h"
#include "../system/mla_hash_map.h"

struct mla_reflection_container_t {
    mla_hash_map_t<mla_init_struct(mla_string_t), mla_string_hash_t, mla_reflection_struct_metadata_provider_t> structs;
};

mla_reflection_container_t g_reflection_container = {
    mla_hash_map<mla_init_struct(mla_string_t), mla_string_hash_t, mla_reflection_struct_metadata_provider_t>()
};

mla_reflection_struct_metadata_t mla_reflection_struct_metadata_invalid() {
    return {mla_string_empty(), 0, true, mla_array_list_empty<mla_init_struct(mla_reflection_struct_field_t)>()};
}

mla_reflection_struct_metadata_t mla_reflection_struct_metadata_for(const mla_string_t &name, mla_size_t size) {
    return {name, size, false, mla_array_list_empty<mla_init_struct(mla_reflection_struct_field_t)>()};
}

void mla_reflection_struct_metadata_add_field(mla_reflection_struct_metadata_t& metadata, const mla_reflection_struct_field_t &field) {

    if (metadata.frozen) {
        mla_error(mla_string_concat("Attempted to add field '", field.name, "' to frozen struct metadata '", metadata.name, "'"));
        return;
    }

    mla_array_list_add(metadata.fields, field);
}

void mla_reflection_struct_metadata_freeze(mla_reflection_struct_metadata_t& metadata) {
    metadata.frozen = true;
}

mla_reflection_struct_field_t mla_reflection_struct_field(const mla_string_t &name, mla_size_t offset, mla_reflection_type_t type, mla_reflection_type_t element_type, const mla_reflection_struct_metadata_provider_t& struct_metadata_provider) {
    return {name, offset, type, element_type, struct_metadata_provider};
}

mla_bool_t mla_reflection_register_struct(const mla_string_t &name, mla_reflection_struct_metadata_provider_t provider) {

    if (mla_hash_map_contains(g_reflection_container.structs, name)) {
        mla_debug(mla_string_concat("Struct '", name, "' is already registered in reflection system"));
        return false;
    }

    mla_hash_map_push(g_reflection_container.structs, name, provider);
    return true;
}

mla_bool_t mla_reflection_is_struct_registered(const mla_string_t &name) {
    return mla_hash_map_contains(g_reflection_container.structs, name);
}

mla_bool_t mla_reflection_get_struct_metadata(const mla_string_t &name, mla_reflection_struct_metadata_t& out_metadata) {

    mla_reflection_struct_metadata_provider_t provider = nullptr;

    if (!mla_hash_map_get(g_reflection_container.structs, name, provider)) {
        return false;
    }

    if (provider == nullptr) {
        return false;
    }

    out_metadata = provider();
    return true;
}

void mla_reflection_register_inner_structs(const mla_reflection_struct_metadata_t& metadata) {

    for (mla_size_t i = 0; i < mla_array_list_size(metadata.fields); ++i) {

        const mla_reflection_struct_field_t& field = mla_array_list_get_unsafe(metadata.fields, i);

        if (field.type == MLA_REFLECTION_TYPE_STRUCT) {

            if (field.struct_provider != nullptr) {
                mla_reflection_struct_metadata_t innerMeta = field.struct_provider();

                if (!mla_reflection_register_struct(innerMeta.name, field.struct_provider)) {
                    continue;
                }

                mla_reflection_register_inner_structs(innerMeta);
            }
        } else if (field.type == MLA_REFLECTION_TYPE_LIST && field.element_type == MLA_REFLECTION_TYPE_STRUCT) {

            if (field.struct_provider != nullptr) {
                mla_reflection_struct_metadata_t innerMeta = field.struct_provider();

                if (!mla_reflection_register_struct(innerMeta.name, field.struct_provider)) {
                    continue;
                }

                mla_reflection_register_inner_structs(innerMeta);
            }
        }
    }
}
