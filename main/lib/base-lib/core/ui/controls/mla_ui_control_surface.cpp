//
// Created by chris on 1/26/2026.
//

#include "mla_ui_control_surface.h"

#include "mla_ui_loading_indicator.h"
#include "../../task/mla_task_manager.h"
#include "../../system/mla_id.h"

#define mla_ui_control_surface_drawing_lock_timeout_ms 100
#define mla_ui_control_surface_rendering_lock_timeout_ms 50

mla_user_data_id_init(mla_ui_control_surface_task_id_user_data_name)

mla_ui_control_surface_drawing_t mla_ui_control_surface_drawing_empty() {
    return {
    mla_array_list_empty<mla_init_struct(mla_ui_surface_draw_command_t)>(),
        mla_array_list_empty<mla_init_struct(mla_ui_surface_input_event_t)>(),
        0
    };
}

mla_ui_control_surface_rendering_t mla_ui_control_surface_rendering_empty() {
    return {
        mla_array_list_empty<mla_init_struct(mla_ui_control_t)>(),
        mla_array_list_empty<mla_init_struct(mla_ui_control_input_area_t)>(),
        nullptr
    };
}

mla_ui_control_surface_rendering_t mla_ui_control_surface_rendering_create(const mla_ui_control_surface_process_task_t &renderingTask) {
    return {
        mla_array_list_empty<mla_init_struct(mla_ui_control_t)>(),
        mla_array_list_empty<mla_init_struct(mla_ui_control_input_area_t)>(),
        renderingTask
    };
}

mla_ui_control_surface_t mla_ui_control_surface_empty() {
    return {
        mla_pointer_null(),
        mla_ui_surface_invalid(),
        mla_mutex_invalid(),
        mla_ui_control_surface_rendering_empty(),
        mla_ui_control_surface_drawing_empty(),
        mla_user_data_empty()
    };
}

mla_ui_control_surface_t mla_ui_control_surface_create(const mla_ui_surface_t &surface, const mla_ui_control_surface_process_task_t &processTask) {
    mla_user_data_t userData = mla_user_data_empty();
    return mla_ui_control_surface_create(surface, processTask, userData);
}

mla_ui_control_surface_t mla_ui_control_surface_create(const mla_ui_surface_t &surface, const mla_ui_control_surface_process_task_t &renderingTask, mla_user_data_t& userData) {
    return {
        mla_pointer_null(),
        surface,
        mla_mutex_create("mla_ui_control_surface_lock"),
        mla_ui_control_surface_rendering_create(renderingTask),
        mla_ui_control_surface_drawing_empty(),
        userData
    };
}

mla_string_t mla_private_ui_control_surface_rendering_task_name(const mla_string_t &taskId) {
    return mla_string_concat("surface_rendering_", taskId);
}

mla_string_t mla_private_ui_control_surface_drawing_task_name(const mla_string_t &taskId) {
    return mla_string_concat("surface_drawing_", taskId);
}

mla_string_t mla_private_ui_control_surface_rendering_drawing_task_name(const mla_string_t &taskId) {
    return mla_string_concat("surface_rendering_drawing", taskId);
}

mla_string_t mla_private_ui_control_surface_get_task_id(mla_ui_control_surface_t& connector) {

    return mla_user_data_get_string(connector.userData, mla_ui_control_surface_task_id_user_data_name);
}

struct mla_private_ui_control_surface_clean_up_t {
    mla_ui_control_surface_t* connector;

    static mla_private_ui_control_surface_clean_up_t init() {
        return {nullptr};
    }
};



void mla_private_ui_control_surface_cleanup(mla_private_ui_control_surface_clean_up_t& data) {


    mla_ui_control_surface_t* connector = data.connector;

    if (connector != nullptr) {

        mla_string_t taskId = mla_private_ui_control_surface_get_task_id(*connector);

        // Abort rendering task
        if (!mla_string_is_empty(taskId)) {
            mla_task_manager_abort_task(mla_private_ui_control_surface_rendering_task_name(taskId));
            mla_task_manager_abort_task(mla_private_ui_control_surface_drawing_task_name(taskId));
        }

        mla_user_data_remove(connector->userData, mla_ui_control_surface_task_id_user_data_name);
    }

}

void mla_private_ui_control_surface_cleanup_single_thread(mla_private_ui_control_surface_clean_up_t& data) {

    mla_ui_control_surface_t* connector = data.connector;

    if (connector != nullptr) {

        mla_string_t taskId = mla_private_ui_control_surface_get_task_id(*connector);

        // Abort rendering task
        if (!mla_string_is_empty(taskId)) {
            mla_task_manager_abort_task(mla_private_ui_control_surface_rendering_drawing_task_name(taskId));
        }
    }

}

mla_user_data_id_init(mla_ui_control_surface_text_size_user_data_name)

mla_ui_surface_draw_size_t mla_private_ui_control_surface_calc_text_size(const mla_ui_control_context_t &context, const mla_ui_surface_font_type_t &font_type, const mla_string_t &text) {

    mla_ui_control_surface_t* connector = mla_user_data_get_pointer_data<mla_ui_control_surface_t>(context.userData, mla_ui_control_surface_text_size_user_data_name);

    if (connector == nullptr || connector->surface.calc_text_size == nullptr) {
        return {0, 0};
    }

    return connector->surface.calc_text_size(connector->surface, font_type, text);
}


mla_ui_control_t mla_private_ui_control_surface_build_loading_indicator(const mla_ui_control_context_t& context) {


    mla_ui_control_t loadingPanel = mla_ui_loading_indicator();
    // Center the loading indicator
    loadingPanel.layout.x = (context.width - 50) / 2.0; // Assuming 50x50 size for loading indicator
    loadingPanel.layout.y = (context.height - 50) / 2.0;
    loadingPanel.layout.width = 50;
    loadingPanel.layout.height = 50;
    return loadingPanel;

}

mla_user_data_id_init(mla_ui_control_surface_connector_user_data_name)

mla_task_process_result_state mla_private_ui_control_surface_render_task(mla_user_data_t& userData) {

    mla_ui_control_surface_t* connector = mla_user_data_get_pointer_data<mla_ui_control_surface_t>(userData, mla_ui_control_surface_connector_user_data_name);

    mla_array_list_t<mla_init_struct(mla_ui_surface_input_event_t)> unprocessedInputEvents = mla_array_list_empty<mla_init_struct(mla_ui_surface_input_event_t)>();

    if (!mla_mutex_trylock(connector->lock, mla_ui_control_surface_rendering_lock_timeout_ms)) {
        return TASK_PROCESS_RESULT_CONTINUE;
    }

    unprocessedInputEvents = connector->drawing.unprocessedInputEvents;
    connector->drawing.unprocessedInputEvents = mla_array_list_empty<mla_init_struct(mla_ui_surface_input_event_t)>();

    // Unlock while rendering
    mla_mutex_unlock(connector->lock);

    // Get surface size and input states
    mla_ui_surface_size_t surfaceSize = mla_ui_surface_get_size(connector->surface);
    mla_ui_surface_input_states_t input_states = mla_ui_surface_get_input_states(connector->surface);

    // Process input events
    mla_ui_control_process_input_events(connector->rendering.root, unprocessedInputEvents, connector->rendering.inputAreas, connector->userData);

    if (connector->rendering.processTask != nullptr) {

        // Call custom rendering task if provided
        if (!connector->rendering.processTask(connector->rendering.root, surfaceSize, input_states)) {
            mla_error("Custom rendering task failed");
            return TASK_PROCESS_RESULT_CONTINUE;
        }
    }

    // Update the draw commands
    mla_uint64_t currentTimeMs = mla_system_time_ms();
    mla_uint64_t timeSinceLastFrameMs = mla_max(1, currentTimeMs - connector->drawing.lastFrameTimeMs);

    mla_user_data_t context_user_data = mla_user_data_copy(connector->userData);
    mla_pointer_t connector_ptr = mla_platform_pointer_to_managed_pointer(connector);
    mla_user_data_set_pointer(context_user_data, mla_ui_control_surface_text_size_user_data_name, connector_ptr);

    mla_ui_control_context_t context = mla_ui_control_context(surfaceSize.width, surfaceSize.height, input_states, mla_private_ui_control_surface_calc_text_size, timeSinceLastFrameMs, context_user_data);
    mla_array_list_t<mla_init_struct(mla_ui_surface_draw_command_t)> drawCommands = mla_array_list<mla_init_struct(mla_ui_surface_draw_command_t)>();
    mla_array_list_t<mla_init_struct(mla_ui_control_input_area_t)> inputAreas = mla_array_list<mla_init_struct(mla_ui_control_input_area_t)>();

    mla_bool_t hasControlsToRender = mla_array_list_size(connector->rendering.root) > 0;

    if (!hasControlsToRender) {
        // If no controls to render, draw a loading screen
        mla_ui_control_t loadingIndicator = mla_private_ui_control_surface_build_loading_indicator(context);
        mla_array_list_add(connector->rendering.root, loadingIndicator);
    }

    mla_bool_t drawSucessfull = mla_ui_controls_render_to_draw_commands(context, connector->rendering.root, drawCommands, inputAreas);

    if (!hasControlsToRender) {
        mla_array_list_clear(connector->rendering.root);
    }

    if (!drawSucessfull) {
        mla_error("Failed to render UI controls to draw commands");
        return TASK_PROCESS_RESULT_CONTINUE;
    }

    // Lock again to update shared data
    if (!mla_mutex_trylock(connector->lock, mla_ui_control_surface_rendering_lock_timeout_ms)) {
        mla_warning("Failed to lock mutex to update draw commands");
        return TASK_PROCESS_RESULT_CONTINUE;
    }

    // Update drawing data
    connector->drawing.drawCommands = drawCommands;
    connector->rendering.inputAreas = inputAreas;

    mla_mutex_unlock(connector->lock);

    return TASK_PROCESS_RESULT_CONTINUE;
}



mla_task_process_result_state mla_private_ui_control_surface_drawing_task(mla_user_data_t& userData) {

    mla_ui_control_surface_t* connector = mla_user_data_get_pointer_data<mla_ui_control_surface_t>(userData, mla_ui_control_surface_connector_user_data_name);

    if (!mla_mutex_trylock(connector->lock, mla_ui_control_surface_drawing_lock_timeout_ms)) {
        return TASK_PROCESS_RESULT_CONTINUE;
    }

    mla_array_list_t<mla_init_struct(mla_ui_surface_draw_command_t)> drawCommands = connector->drawing.drawCommands;
    mla_mutex_unlock(connector->lock);

    mla_array_list_t<mla_init_struct(mla_ui_surface_input_event_t)> newUnprocessedInputEvents = mla_array_list_empty<mla_init_struct(mla_ui_surface_input_event_t)>();

    // Render to surface
    if (mla_ui_surface_render_draw_commands(
        connector->surface,
        drawCommands,
        newUnprocessedInputEvents, connector->drawing.lastFrameTimeMs
    )) {
        // If the frame was rendered successfully, update last frame time
        connector->drawing.lastFrameTimeMs = mla_system_time_ms();
    }

    if (!mla_mutex_lock(connector->lock)) {
        mla_warning(mla_string_const("Unable to lock mutex to update input events"));
        return TASK_PROCESS_RESULT_CONTINUE;
    }

    // Append new unprocessed input events
    mla_array_list_add_all(connector->drawing.unprocessedInputEvents, newUnprocessedInputEvents);

    mla_mutex_unlock(connector->lock);

    return TASK_PROCESS_RESULT_CONTINUE;
}

mla_task_process_result_state mla_private_ui_control_surface_render_and_draw_task(mla_user_data_t& userData) {

    mla_ui_control_surface_t* connector = mla_user_data_get_pointer_data<mla_ui_control_surface_t>(userData, mla_ui_control_surface_connector_user_data_name);

    if (connector == nullptr) {
        return TASK_PROCESS_RESULT_DONE;
    }

    mla_ui_control_surface_execute_render_and_draw(*connector);
    return TASK_PROCESS_RESULT_CONTINUE;

}

mla_bool_t mla_ui_control_surface_start(mla_ui_control_surface_t &connector) {

    if (!mla_string_is_empty(mla_private_ui_control_surface_get_task_id(connector))) {
        mla_warning("Connector is already started");
        return false;
    }

    // Init rendering
    connector.rendering.inputAreas = mla_array_list_empty<mla_init_struct(mla_ui_control_input_area_t)>();
    connector.drawing.lastFrameTimeMs = mla_uint64_max;

    mla_string_t id = mla_generate_runtime_id();

    mla_pointer_t connector_ptr = mla_platform_pointer_to_managed_pointer(&connector);
    mla_user_data_set_pointer(connector.userData, mla_ui_control_surface_connector_user_data_name, connector_ptr);

    // Create rendering task
    mla_task_t render_task = mla_task_repeating(
            mla_private_ui_control_surface_rendering_task_name(id),
            mla_private_ui_control_surface_render_task,
            connector.userData
        );

    // Create the Drawing Task
    mla_task_t drawing_task = mla_task_repeating(
            mla_private_ui_control_surface_drawing_task_name(id),
            mla_private_ui_control_surface_drawing_task,
            connector.userData
        );

    // Create rendering thread
    if (!mla_task_manager_register_task(render_task)) {
        mla_error("Failed to register UI control surface connector rendering task!");
        return false;
    }

    // Create drawing thread
    if (!mla_task_manager_register_task(drawing_task)) {
        mla_error("Failed to register UI control surface connector drawing task!");

        // Abort previously created rendering task
        mla_task_manager_abort_task(render_task.name);
        return false;
    }

    // Store task reference
    connector.cleanup = mla_malloc_struct_cleanup_hook(mla_private_ui_control_surface_clean_up_t, mla_private_ui_control_surface_cleanup);

    mla_private_ui_control_surface_clean_up_t* clean_up_data = mla_pointer_get_data<mla_private_ui_control_surface_clean_up_t>(connector.cleanup);

    if (clean_up_data == nullptr) {
        mla_error("Failed to create cleanup buffer reference for UI control surface connector!");

        // Abort previously created tasks
        mla_task_manager_abort_task(render_task.name);
        mla_task_manager_abort_task(drawing_task.name);
        return false;
    }

    clean_up_data->connector = &connector;
    mla_user_data_set_string(connector.userData, mla_ui_control_surface_task_id_user_data_name, id);

    return true;
}

mla_bool_t mla_ui_control_surface_start_single_threaded_mode(mla_ui_control_surface_t &connector) {

    if (!mla_string_is_empty(mla_private_ui_control_surface_get_task_id(connector))) {
        mla_warning("Connector is already started");
        return false;
    }

    // Init rendering
    connector.rendering.inputAreas = mla_array_list_empty<mla_init_struct(mla_ui_control_input_area_t)>();
    connector.drawing.lastFrameTimeMs = mla_uint64_max;

    mla_string_t id = mla_generate_runtime_id();

    mla_pointer_t connector_ptr = mla_platform_pointer_to_managed_pointer(&connector);
    mla_user_data_set_pointer(connector.userData, mla_ui_control_surface_connector_user_data_name, connector_ptr);

    // Create rendering task
    mla_task_t render_draw_task = mla_task_repeating(
            mla_private_ui_control_surface_rendering_drawing_task_name(id),
            mla_private_ui_control_surface_render_and_draw_task,
            connector.userData
        );

    if (!mla_task_manager_register_task(render_draw_task)) {
        mla_error("Failed to register UI control surface connector rendering/drawing task!");
        return false;
    }

    // Store task reference

    connector.cleanup = mla_malloc_struct_cleanup_hook(mla_private_ui_control_surface_clean_up_t, mla_private_ui_control_surface_cleanup_single_thread);

    mla_private_ui_control_surface_clean_up_t* clean_up_data = mla_pointer_get_data<mla_private_ui_control_surface_clean_up_t>(connector.cleanup);

    if (clean_up_data == nullptr) {
        mla_error("Failed to create cleanup buffer reference for UI control surface connector!");

        // Abort previously created task
        mla_task_manager_abort_task(render_draw_task.name);
        return false;
    }

    clean_up_data->connector = &connector;

    //connector.cleanup = mla_buffer_reference_create(&connector, true, mla_private_ui_control_surface_cleanup, mla_dynamic_data_empty());
    mla_user_data_set_string(connector.userData, mla_ui_control_surface_task_id_user_data_name, id);

    return true;
}

mla_bool_t mla_ui_control_surface_execute_render_and_draw(mla_ui_control_surface_t &connector) {

    // No locking here as it's single threaded mode

    // Get surface size and input states
    mla_ui_surface_size_t surfaceSize = mla_ui_surface_get_size(connector.surface);
    mla_ui_surface_input_states_t input_states = mla_ui_surface_get_input_states(connector.surface);

    // Process input events
    mla_ui_control_process_input_events(connector.rendering.root, connector.drawing.unprocessedInputEvents, connector.rendering.inputAreas, connector.userData);
    mla_array_list_clear(connector.drawing.unprocessedInputEvents);

    if (connector.rendering.processTask != nullptr) {

        // Call custom rendering task if provided
        if (!connector.rendering.processTask(connector.rendering.root, surfaceSize, input_states)) {
            mla_error("Custom rendering task failed");
            return false;
        }
    }

    // Update the draw commands
    mla_uint64_t currentTimeMs = mla_system_time_ms();
    mla_uint64_t timeSinceLastFrameMs = mla_max(1, currentTimeMs - connector.drawing.lastFrameTimeMs);

    mla_user_data_t context_user_data = mla_user_data_copy(connector.userData);
    mla_pointer_t connector_ptr = mla_platform_pointer_to_managed_pointer(&connector);
    mla_user_data_set_pointer(context_user_data, mla_ui_control_surface_text_size_user_data_name, connector_ptr);

    mla_ui_control_context_t context = mla_ui_control_context(surfaceSize.width, surfaceSize.height, input_states, mla_private_ui_control_surface_calc_text_size, timeSinceLastFrameMs, context_user_data);

    mla_array_list_clear(connector.drawing.drawCommands);
    mla_array_list_clear(connector.rendering.inputAreas);

    mla_bool_t hasControlsToRender = mla_array_list_size(connector.rendering.root) > 0;

    if (!hasControlsToRender) {
        // If no controls to render, draw a loading screen
        mla_ui_control_t loadingIndicator = mla_private_ui_control_surface_build_loading_indicator(context);
        mla_array_list_add(connector.rendering.root, loadingIndicator);
    }

    mla_bool_t drawSucessfull = mla_ui_controls_render_to_draw_commands(context, connector.rendering.root, connector.drawing.drawCommands, connector.rendering.inputAreas);

    if (!hasControlsToRender) {
        mla_array_list_clear(connector.rendering.root);
    }

    if (!drawSucessfull) {
        mla_error("Failed to render UI controls to draw commands");
        return false;
    }

    // Render to surface
    if (mla_ui_surface_render_draw_commands(connector.surface, connector.drawing.drawCommands, connector.drawing.unprocessedInputEvents, timeSinceLastFrameMs)) {
        // If the frame was rendered successfully, update last frame time
        connector.drawing.lastFrameTimeMs = currentTimeMs;
    }



    return true;

}

mla_bool_t mla_ui_control_surface_stop(mla_ui_control_surface_t &connector) {

    // Clear rendering task
    connector.cleanup = mla_pointer_null();
    return true;
}
