//
// Created by chris on 1/21/2026.
//

#include "../surfaces/mla_ui_surface.h"

mla_ui_surface_t mla_ui_surface_invalid() {
    return {
        mla_pointer_null(), nullptr, nullptr, nullptr, nullptr, nullptr
    };
}

mla_bool_t mla_ui_surface_is_valid(const mla_ui_surface_t& surface) {
    return !mla_pointer_is_null(surface.resource);
}

mla_ui_surface_size_t mla_ui_surface_get_size(const mla_ui_surface_t& surface) {

    if (surface.get_size == nullptr) {
        return {0, 0};
    }

    return surface.get_size(surface);
}

mla_bool_t mla_ui_surface_set_size(const mla_ui_surface_t& surface, mla_ui_surface_size_t size) {

    if (surface.set_size == nullptr) {
        return false;
    }

    return surface.set_size(surface, size);
}

mla_ui_surface_input_states_t mla_ui_surface_get_input_states(const mla_ui_surface_t &surface) {

    if (surface.get_input_states == nullptr) {
        return mla_ui_surface_input_states_empty();
    }

    return surface.get_input_states(surface);
}

mla_bool_t mla_ui_surface_render_draw_commands(const mla_ui_surface_t& surface, const mla_array_list_t<mla_init_struct(mla_ui_surface_draw_command_t)>& drawCommands, mla_array_list_t<mla_init_struct(mla_ui_surface_input_event_t)>& eventsSinceLastFame, mla_uint64_t timeSinceLastFrameMs) {

    if (surface.render_draw_commands == nullptr) {
        return false;
    }

    return surface.render_draw_commands(surface, drawCommands, eventsSinceLastFame, timeSinceLastFrameMs);
}