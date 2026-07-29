//
// Created by chris on 1/26/2026.
//

#ifndef MLA_UI_CONTROL_SURFACE_H
#define MLA_UI_CONTROL_SURFACE_H

#include "../surfaces/mla_ui_surface.h"
#include "mla_ui_control.h"
#include "../../task/mla_mutx.h"

struct mla_ui_control_surface_drawing_t {
    mla_array_list_t<mla_init_struct(mla_ui_surface_draw_command_t)> drawCommands;
    mla_array_list_t<mla_init_struct(mla_ui_surface_input_event_t)> unprocessedInputEvents;
    mla_uint64_t lastFrameTimeMs; // Time when the last frame was rendered
};

mla_ui_control_surface_drawing_t mla_ui_control_surface_drawing_empty();

struct mla_ui_control_surface_rendering_t;

typedef mla_bool_t (*mla_ui_control_surface_process_task_t)(mla_array_list_t<mla_init_struct(mla_ui_control_t)> &root, const mla_ui_surface_size_t& surfaceSize, const mla_ui_surface_input_states_t &input_states);

struct mla_ui_control_surface_rendering_t {
    mla_array_list_t<mla_init_struct(mla_ui_control_t)> root;
    mla_array_list_t<mla_init_struct(mla_ui_control_input_area_t)> inputAreas;
    mla_ui_control_surface_process_task_t processTask;
};

mla_ui_control_surface_rendering_t mla_ui_control_surface_rendering_empty();

struct mla_ui_control_surface_t {
    mla_pointer_t cleanup;
    mla_ui_surface_t surface;
    mla_mutex_t lock;
    mla_ui_control_surface_rendering_t rendering;
    mla_ui_control_surface_drawing_t drawing;
    mla_user_data_t userData;
};

mla_ui_control_surface_t mla_ui_control_surface_empty();

mla_ui_control_surface_t mla_ui_control_surface_create(const mla_ui_surface_t &surface, const mla_ui_control_surface_process_task_t &processTask);
mla_ui_control_surface_t mla_ui_control_surface_create(const mla_ui_surface_t &surface, const mla_ui_control_surface_process_task_t &renderingTask, mla_user_data_t& userData);

mla_bool_t mla_ui_control_surface_start(mla_ui_control_surface_t &connector);
mla_bool_t mla_ui_control_surface_start_single_threaded_mode(mla_ui_control_surface_t &connector);

mla_bool_t mla_ui_control_surface_execute_render_and_draw(mla_ui_control_surface_t &connector);

mla_bool_t mla_ui_control_surface_stop(mla_ui_control_surface_t &connector);

#endif