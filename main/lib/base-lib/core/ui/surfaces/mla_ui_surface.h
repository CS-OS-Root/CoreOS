//
// Created by chris on 1/21/2026.
//

#ifndef MLA_UI_SURFACE_H
#define MLA_UI_SURFACE_H

#include "../../mla_data_types.h"
#include "../../system/mla_string.h"
#include "mla_ui_surface_draw.h"


struct mla_ui_surface_size_t {
    mla_uint32_t width;
    mla_uint32_t height;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_ui_surface_size_t* self = mla_pointer_get_data<const mla_ui_surface_size_t>(obj);

        if (self == nullptr) {
            return false;
        }

        mla_serializer_write_uint32(serializer, mla_string_const("width"),  self->width);
        mla_serializer_write_uint32(serializer, mla_string_const("height"), self->height);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_ui_surface_size_t* self = mla_pointer_get_data<mla_ui_surface_size_t>(obj);

        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "width")) {
            mla_deserializer_read_uint32(deserializer, self->width);
        } else if (mla_string_equals_const(property_name, "height")) {
            mla_deserializer_read_uint32(deserializer, self->height);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }
};

struct mla_ui_surface_t {
    mla_pointer_t resource; // Pointer to the surface resource

    // Size management
    mla_ui_surface_size_t (*get_size)(const mla_ui_surface_t& surface);
    mla_bool_t (*set_size)(const mla_ui_surface_t& surface, mla_ui_surface_size_t size);

    // Render draw commands
    mla_bool_t (*render_draw_commands)(const mla_ui_surface_t& surface, const mla_array_list_t<mla_init_struct(mla_ui_surface_draw_command_t)>& drawCommands, mla_array_list_t<mla_init_struct(mla_ui_surface_input_event_t)>& eventsSinceLastFame, mla_uint64_t timeSinceLastFrameMs);

    // Text size calculation
    mla_ui_surface_draw_size_t (*calc_text_size)(const mla_ui_surface_t &surface, const mla_ui_surface_font_type_t &font_type, const mla_string_t &text);

    // Input states retrieval
    mla_ui_surface_input_states_t (*get_input_states)(const mla_ui_surface_t &surface);

};

mla_ui_surface_t mla_ui_surface_invalid();
mla_bool_t mla_ui_surface_is_valid(const mla_ui_surface_t& surface);

mla_ui_surface_size_t mla_ui_surface_get_size(const mla_ui_surface_t& surface);
mla_bool_t mla_ui_surface_set_size(const mla_ui_surface_t& surface, mla_ui_surface_size_t size);

mla_ui_surface_input_states_t mla_ui_surface_get_input_states(const mla_ui_surface_t &surface);

mla_bool_t mla_ui_surface_render_draw_commands(const mla_ui_surface_t& surface, const mla_array_list_t<mla_init_struct(mla_ui_surface_draw_command_t)>& drawCommands, mla_array_list_t<mla_init_struct(mla_ui_surface_input_event_t)>& eventsSinceLastFame, mla_uint64_t timeSinceLastFrameMs);

#endif