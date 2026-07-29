//
// Created by chris on 1/19/2026.
//

#include "mla_ui_control.h"
#include "../../system/mla_stream.h"
#include "../../system/mla_id.h"
#include "../../log/mla_logging.h"
#include "../../system/mla_string_concat.h"

#define mla_ui_control_has_focus_attribute "hasFocus"

mla_ui_control_value_t mla_ui_control_value_empty() {
    return {
        mla_string_empty(),
        MLA_UI_CONTROL_VALUE_TYPE_INT32,
        {0},
        mla_string_empty()
    };
}

mla_ui_control_layout_t mla_ui_control_layout_empty() {
    return {0, 0, 0, 0};
}

mla_ui_control_t mla_ui_control() {
    return {
        mla_generate_runtime_id(),
        {0, 0, 0, 0},
        mla_array_list_empty<mla_init_struct(mla_ui_control_t)>(),
        nullptr,
        nullptr,
        nullptr,
        mla_array_list_empty<mla_init_struct(mla_ui_control_value_t)>(),
    };
}

mla_ui_control_t mla_ui_control_empty() {
    return {
        mla_string_empty(),
        {0, 0, 0, 0},
        mla_array_list_empty<mla_init_struct(mla_ui_control_t)>(),
        nullptr,
        nullptr,
        nullptr,
        mla_array_list_empty<mla_init_struct(mla_ui_control_value_t)>(),
    };
}

mla_bool_t mla_private_ui_control_find_value_by_name(const mla_ui_control_t &control, const mla_string_t &name,
                                               mla_ui_control_value_t* &out) {
    for (mla_size_t i = 0; i < mla_array_list_size(control.values); i++) {
        mla_ui_control_value_t* value = mla_array_list_get_ref_unsafe(control.values, i);
        if (mla_string_equals(value->name, name)) {
            out = value;
            return true;
        }
    }
    return false;
}

mla_uint8_t mla_ui_control_get_value_as_uint8(const mla_ui_control_t &control, const mla_string_t &name,
                                              mla_uint8_t defaultValue) {
    mla_ui_control_value_t* value = nullptr;
    if (mla_private_ui_control_find_value_by_name(control, name, value)) {
        return mla_s_cast<mla_uint8_t>(value->uint64Value);
    }
    return defaultValue;
}

mla_int8_t mla_ui_control_get_value_as_int8(const mla_ui_control_t &control, const mla_string_t &name,
                                            mla_int8_t defaultValue) {
    mla_ui_control_value_t* value = nullptr;
    if (mla_private_ui_control_find_value_by_name(control, name, value)) {
        return mla_s_cast<mla_int8_t>(value->int64Value);
    }
    return defaultValue;
}

mla_uint16_t mla_ui_control_get_value_as_uint16(const mla_ui_control_t &control, const mla_string_t &name,
                                                mla_uint16_t defaultValue) {
    mla_ui_control_value_t* value = nullptr;
    if (mla_private_ui_control_find_value_by_name(control, name, value)) {
        return mla_s_cast<mla_uint16_t>(value->uint64Value);
    }
    return defaultValue;
}

mla_int16_t mla_ui_control_get_value_as_int16(const mla_ui_control_t &control, const mla_string_t &name,
                                              mla_int16_t defaultValue) {
    mla_ui_control_value_t* value = nullptr;
    if (mla_private_ui_control_find_value_by_name(control, name, value)) {
        return mla_s_cast<mla_int16_t>(value->int64Value);
    }
    return defaultValue;
}

mla_uint32_t mla_ui_control_get_value_as_uint32(const mla_ui_control_t &control, const mla_string_t &name,
                                                mla_uint32_t defaultValue) {
    mla_ui_control_value_t* value = nullptr;
    if (mla_private_ui_control_find_value_by_name(control, name, value)) {
        return mla_s_cast<mla_uint32_t>(value->uint64Value);
    }
    return defaultValue;
}

mla_int32_t mla_ui_control_get_value_as_int32(const mla_ui_control_t &control, const mla_string_t &name,
                                              mla_int32_t defaultValue) {
    mla_ui_control_value_t* value = nullptr;
    if (mla_private_ui_control_find_value_by_name(control, name, value)) {
        return mla_s_cast<mla_int32_t>(value->int64Value);
    }
    return defaultValue;
}

mla_uint64_t mla_ui_control_get_value_as_uint64(const mla_ui_control_t &control, const mla_string_t &name,
                                                mla_uint64_t defaultValue) {
    mla_ui_control_value_t* value = nullptr;
    if (mla_private_ui_control_find_value_by_name(control, name, value)) {
        return value->uint64Value;
    }
    return defaultValue;
}

mla_int64_t mla_ui_control_get_value_as_int64(const mla_ui_control_t &control, const mla_string_t &name,
                                              mla_int64_t defaultValue) {
    mla_ui_control_value_t* value = nullptr;
    if (mla_private_ui_control_find_value_by_name(control, name, value)) {
        return value->int64Value;
    }
    return defaultValue;
}

mla_float_t mla_ui_control_get_value_as_float(const mla_ui_control_t &control, const mla_string_t &name,
                                              mla_float_t defaultValue) {
    mla_ui_control_value_t* value = nullptr;
    if (mla_private_ui_control_find_value_by_name(control, name, value)) {
        return mla_s_cast<mla_float_t>(value->doubleValue);
    }
    return defaultValue;
}

mla_double_t mla_ui_control_get_value_as_double(const mla_ui_control_t &control, const mla_string_t &name,
                                                mla_double_t defaultValue) {
    mla_ui_control_value_t* value = nullptr;
    if (mla_private_ui_control_find_value_by_name(control, name, value)) {
        return value->doubleValue;
    }
    return defaultValue;
}

mla_string_t mla_ui_control_get_value_as_string(const mla_ui_control_t &control, const mla_string_t &name,
                                                const mla_string_t &defaultValue) {
    mla_ui_control_value_t* value = nullptr;
    if (mla_private_ui_control_find_value_by_name(control, name, value)) {
        return value->stringValue;
    }
    return defaultValue;
}

mla_bool_t mla_ui_control_get_value_as_bool(const mla_ui_control_t &control, const mla_string_t &name,
                                            mla_bool_t defaultValue) {
    mla_ui_control_value_t* value = nullptr;
    if (mla_private_ui_control_find_value_by_name(control, name, value)) {
        return value->boolValue;
    }
    return defaultValue;
}

mla_platform_pointer_t mla_ui_control_get_value_as_pointer(const mla_ui_control_t &control, const mla_string_t &name, mla_platform_pointer_t defaultValue) {
    mla_ui_control_value_t* value = nullptr;
    if (mla_private_ui_control_find_value_by_name(control, name, value)) {
        return value->pointerValue;
    }
    return defaultValue;
}

mla_bool_t mla_ui_control_set_value_as_uint8(mla_ui_control_t &control, const mla_string_t &name, mla_uint8_t value) {
    mla_ui_control_value_t* internalValue = nullptr;
    if (mla_private_ui_control_find_value_by_name(control, name, internalValue)) {
        internalValue->uint64Value = mla_s_cast<mla_uint64_t>(value);
        return true;
    }

    // Not found, create new
    mla_ui_control_value_t newValue = mla_ui_control_value_empty();
    newValue.name = name;
    newValue.uint64Value = mla_s_cast<mla_uint64_t>(value);
    return mla_array_list_add(control.values, newValue);
}

mla_bool_t mla_ui_control_set_value_as_int8(mla_ui_control_t &control, const mla_string_t &name, mla_int8_t value) {
    mla_ui_control_value_t* internalValue = nullptr;
    if (mla_private_ui_control_find_value_by_name(control, name, internalValue)) {
        internalValue->int64Value = mla_s_cast<mla_int64_t>(mla_s_cast<mla_int32_t>(value));
        return true;
    }

    // Not found, create new
    mla_ui_control_value_t newValue = mla_ui_control_value_empty();
    newValue.name = name;
    newValue.int64Value = mla_s_cast<mla_int64_t>(mla_s_cast<mla_int32_t>(value));
    return mla_array_list_add(control.values, newValue);
}

mla_bool_t mla_ui_control_set_value_as_uint16(mla_ui_control_t &control, const mla_string_t &name, mla_uint16_t value) {
    mla_ui_control_value_t* internalValue = nullptr;
    if (mla_private_ui_control_find_value_by_name(control, name, internalValue)) {
        internalValue->uint64Value = mla_s_cast<mla_uint64_t>(value);
        return true;
    }

    // Not found, create new
    mla_ui_control_value_t newValue = mla_ui_control_value_empty();
    newValue.name = name;
    newValue.uint64Value = mla_s_cast<mla_uint64_t>(value);
    return mla_array_list_add(control.values, newValue);
}

mla_bool_t mla_ui_control_set_value_as_int16(mla_ui_control_t &control, const mla_string_t &name, mla_int16_t value) {
    mla_ui_control_value_t* internalValue = nullptr;
    if (mla_private_ui_control_find_value_by_name(control, name, internalValue)) {
        internalValue->int64Value = mla_s_cast<mla_int64_t>(value);
        return true;
    }

    // Not found, create new
    mla_ui_control_value_t newValue = mla_ui_control_value_empty();
    newValue.name = name;
    newValue.int64Value = mla_s_cast<mla_int64_t>(value);
    return mla_array_list_add(control.values, newValue);
}

mla_bool_t mla_ui_control_set_value_as_uint32(mla_ui_control_t &control, const mla_string_t &name, mla_uint32_t value) {
    mla_ui_control_value_t* internalValue = nullptr;
    if (mla_private_ui_control_find_value_by_name(control, name, internalValue)) {
        internalValue->uint64Value = mla_s_cast<mla_uint64_t>(value);
        return true;
    }

    // Not found, create new
    mla_ui_control_value_t newValue = mla_ui_control_value_empty();
    newValue.name = name;
    newValue.uint64Value = mla_s_cast<mla_uint64_t>(value);
    return mla_array_list_add(control.values, newValue);
}

mla_bool_t mla_ui_control_set_value_as_int32(mla_ui_control_t &control, const mla_string_t &name, mla_int32_t value) {
    mla_ui_control_value_t* internalValue = nullptr;
    if (mla_private_ui_control_find_value_by_name(control, name, internalValue)) {
        internalValue->int64Value = mla_s_cast<mla_int64_t>(value);
        return true;
    }

    // Not found, create new
    mla_ui_control_value_t newValue = mla_ui_control_value_empty();
    newValue.name = name;
    newValue.int64Value = mla_s_cast<mla_int64_t>(value);
    return mla_array_list_add(control.values, newValue);
}

mla_bool_t mla_ui_control_set_value_as_uint64(mla_ui_control_t &control, const mla_string_t &name, mla_uint64_t value) {
    mla_ui_control_value_t* internalValue = nullptr;
    if (mla_private_ui_control_find_value_by_name(control, name, internalValue)) {
        internalValue->uint64Value = value;
        return true;
    }

    // Not found, create new
    mla_ui_control_value_t newValue = mla_ui_control_value_empty();
    newValue.name = name;
    newValue.uint64Value = value;
    return mla_array_list_add(control.values, newValue);
}

mla_bool_t mla_ui_control_set_value_as_int64(mla_ui_control_t &control, const mla_string_t &name, mla_int64_t value) {
    mla_ui_control_value_t* internalValue = nullptr;
    if (mla_private_ui_control_find_value_by_name(control, name, internalValue)) {
        internalValue->int64Value = value;
        return true;
    }

    // Not found, create new
    mla_ui_control_value_t newValue = mla_ui_control_value_empty();
    newValue.name = name;
    newValue.int64Value = value;
    return mla_array_list_add(control.values, newValue);
}

mla_bool_t mla_ui_control_set_value_as_float(mla_ui_control_t &control, const mla_string_t &name, mla_float_t value) {
    mla_ui_control_value_t* internalValue = nullptr;
    if (mla_private_ui_control_find_value_by_name(control, name, internalValue)) {
        internalValue->doubleValue = mla_s_cast<mla_double_t>(value);
        return true;
    }

    // Not found, create new
    mla_ui_control_value_t newValue = mla_ui_control_value_empty();
    newValue.name = name;
    newValue.doubleValue = mla_s_cast<mla_double_t>(value);
    return mla_array_list_add(control.values, newValue);
}

mla_bool_t mla_ui_control_set_value_as_double(mla_ui_control_t &control, const mla_string_t &name, mla_double_t value) {
    mla_ui_control_value_t* internalValue = nullptr;
    if (mla_private_ui_control_find_value_by_name(control, name, internalValue)) {
        internalValue->doubleValue = value;
        return true;
    }

    // Not found, create new
    mla_ui_control_value_t newValue = mla_ui_control_value_empty();
    newValue.name = name;
    newValue.doubleValue = value;
    return mla_array_list_add(control.values, newValue);
}

mla_bool_t mla_ui_control_set_value_as_string(mla_ui_control_t &control, const mla_string_t &name,
                                              const mla_string_t &value) {
    mla_ui_control_value_t* internalValue = nullptr;
    if (mla_private_ui_control_find_value_by_name(control, name, internalValue)) {
        internalValue->stringValue = value;
        return true;
    }

    // Not found, create new
    mla_ui_control_value_t newValue = mla_ui_control_value_empty();
    newValue.name = name;
    newValue.stringValue = value;
    return mla_array_list_add(control.values, newValue);
}

mla_bool_t mla_ui_control_set_value_as_bool(mla_ui_control_t &control, const mla_string_t &name, mla_bool_t value) {
    mla_ui_control_value_t* internalValue = nullptr;
    if (mla_private_ui_control_find_value_by_name(control, name, internalValue)) {
        internalValue->boolValue = value;
        return true;
    }

    // Not found, create new
    mla_ui_control_value_t newValue = mla_ui_control_value_empty();
    newValue.name = name;
    newValue.boolValue = value;
    return mla_array_list_add(control.values, newValue);
}

mla_bool_t mla_ui_control_set_value_as_pointer(mla_ui_control_t &control, const mla_string_t &name, mla_platform_pointer_t value) {
    mla_ui_control_value_t* internalValue = nullptr;
    if (mla_private_ui_control_find_value_by_name(control, name, internalValue)) {
        internalValue->pointerValue = value;
        return true;
    }

    // Not found, create new
    mla_ui_control_value_t newValue = mla_ui_control_value_empty();
    newValue.name = name;
    newValue.pointerValue = value;
    return mla_array_list_add(control.values, newValue);
}

void mla_ui_control_reset_values(const mla_array_list_t<mla_init_struct(mla_ui_control_t)> &uiControls,
                                 const mla_string_t &name) {
    for (mla_size_t i = 0; i < mla_array_list_size(uiControls); i++) {
        mla_ui_control_t &control = mla_array_list_get_unsafe(uiControls, i);

        // Remove value with the given name
        for (mla_size_t j = 0; j < mla_array_list_size(control.values); j++) {
            mla_ui_control_value_t &value = mla_array_list_get_unsafe(control.values, j);
            if (mla_string_equals(value.name, name)) {
                mla_array_list_remove(control.values, j);
                break;
            }
        }

        // Reset values in children
        mla_ui_control_reset_values(control.children, name);
    }
}

mla_bool_t mla_ui_control_add_child(mla_ui_control_t &parent, const mla_ui_control_t &child) {
    return mla_array_list_add(parent.children, child);
}

mla_bool_t mla_ui_control_render_to_draw_commands(const mla_ui_control_context_t &context,
                                                  const mla_ui_control_t &control,
                                                  mla_array_list_t<mla_init_struct(mla_ui_surface_draw_command_t)> &drawCommands,
                                                  mla_array_list_t<mla_init_struct(mla_ui_control_input_area_t)> &inputAreas) {
    if (control.renderToDrawCommands != nullptr) {
        return control.renderToDrawCommands(context, control, drawCommands, inputAreas);
    }
    return false;
}

mla_bool_t mla_ui_controls_render_to_draw_commands(const mla_ui_control_context_t &context, const mla_array_list_t<mla_init_struct(mla_ui_control_t)> &uiControls, mla_array_list_t<mla_init_struct(mla_ui_surface_draw_command_t)>& drawCommands, mla_array_list_t<mla_init_struct(mla_ui_control_input_area_t)> &inputAreas) {

    mla_size_t controlCount = mla_array_list_size(uiControls);
    for (mla_size_t i = 0; i < controlCount; i++) {
        const mla_ui_control_t &control = mla_array_list_get_unsafe(uiControls, i);

        if (!mla_ui_control_render_to_draw_commands(context, control, drawCommands, inputAreas)) {
            return false;
        }
    }
    return true;
}

mla_ui_control_context_t mla_ui_control_context(mla_double_t width, mla_double_t height,
                                                const mla_ui_surface_input_states_t &input_states,
                                                mla_ui_control_context_calcTextSize_t *calcTextSize, mla_uint64_t timeSinceLastFrameMs, mla_user_data_t& userData) {
    return {
        0,
        0,
        width,
        height,
        timeSinceLastFrameMs,
        userData,
        input_states,
        calcTextSize,
    };
}

mla_ui_control_context_t mla_ui_control_context(mla_double_t width, mla_double_t height,
                                                const mla_ui_surface_input_states_t &input_states,
                                                mla_ui_control_context_calcTextSize_t *calcTextSize, mla_uint64_t timeSinceLastFrameMs) {

    mla_user_data_t user_data = mla_user_data_empty();
    return mla_ui_control_context(width, height, input_states, calcTextSize, timeSinceLastFrameMs, user_data);
}

mla_ui_control_context_t mla_ui_control_create_context_for_children(const mla_ui_control_context_t &parentContext,
                                                                    const mla_ui_control_t &control) {
    mla_ui_control_context_t context = parentContext;

    if (control.layout.width != 0) {
        context.width = control.layout.width;
    }

    if (control.layout.height != 0) {
        context.height = control.layout.height;
    }

    return context;
}

mla_bool_t mla_ui_control_is_hovered(const mla_ui_control_context_t &context, const mla_ui_control_layout_t &layout) {
    // If cursor position is negative, it's outside the surface
    if (context.input_states.cursorPosition.x < 0 || context.input_states.cursorPosition.y < 0) {
        return false;
    }

    mla_double_t mouseX = context.input_states.cursorPosition.x;
    mla_double_t mouseY = context.input_states.cursorPosition.y;

    mla_double_t controlX = layout.x + context.offsetX;
    mla_double_t controlY = layout.y + context.offsetY;
    mla_double_t controlW = layout.width;
    mla_double_t controlH = layout.height;

    if (controlW == 0) {
        controlW = context.width - layout.x;
    }

    if (controlH == 0) {
        controlH = context.height - layout.y;
    }

    return (mouseX >= controlX && mouseX <= (controlX + controlW) &&
            mouseY >= controlY && mouseY <= (controlY + controlH));
}

mla_bool_t mla_ui_control_has_focus(const mla_ui_control_t &control) {
    return mla_ui_control_get_value_as_bool(control, mla_string_const(mla_ui_control_has_focus_attribute), false);
}

mla_bool_t mla_ui_control_is_hovered(const mla_ui_control_context_t &context, const mla_ui_control_t &control) {
    return mla_ui_control_is_hovered(context, control.layout);
}

mla_ui_control_input_area_t mla_ui_control_input_area_empty() {
    return {
        mla_string_empty(),
        {0, 0, 0, 0},
        mla_string_empty(),
        mla_user_data_empty(),
        MLA_UI_SURFACE_INPUT_EVENT_KIND_NONE,
    };
}

mla_ui_control_input_area_t mla_ui_control_input_area(const mla_string_t &controlId, mla_ui_control_layout_t position, const mla_string_t& event_name,
                                                      mla_ui_surface_input_event_kind acceptedEvents) {
    return {
        controlId,
        position,
        event_name,
        mla_user_data_empty(),
        acceptedEvents,
    };
}

mla_ui_control_input_area_t mla_ui_control_input_area(const mla_string_t &controlId, mla_ui_control_layout_t position, const mla_string_t& event_name,
                                                      mla_ui_surface_input_event_kind acceptedEvents,
                                                      mla_user_data_t& userData) {
    return {
        controlId,
        position,
        event_name,
        userData,
        acceptedEvents,
    };
}

mla_bool_t mla_ui_control_find_by_id(const mla_array_list_t<mla_init_struct(mla_ui_control_t)> &uiControls,
                                     const mla_string_t &controlId, mla_ui_control_t* &outControl) {
    for (mla_size_t i = 0; i < mla_array_list_size(uiControls); i++) {
        mla_ui_control_t* control = mla_array_list_get_ref_unsafe(uiControls, i);

        if (mla_string_equals(control->id, controlId)) {
            outControl = control;
            return true;
        }

        // Search in children
        if (mla_ui_control_find_by_id(control->children, controlId, outControl)) {
            return true;
        }
    }
    return false;
}

mla_bool_t mla_ui_control_find_focused_control(const mla_array_list_t<mla_init_struct(mla_ui_control_t)> &uiControls, mla_ui_control_t* &outControl) {
    for (mla_size_t i = 0; i < mla_array_list_size(uiControls); i++) {
        mla_ui_control_t* control = mla_array_list_get_ref_unsafe(uiControls, i);

        if (mla_ui_control_has_focus(*control)) {
            outControl = control;
            return true;
        }

        // Search in children
        if (mla_ui_control_find_focused_control(control->children, outControl)) {
            return true;
        }
    }
    return false;
}

void mla_ui_control_process_input_events(
    mla_array_list_t<mla_init_struct(mla_ui_control_t)> &uiControls,
    const mla_array_list_t<mla_init_struct(mla_ui_surface_input_event_t)> &inputEvents,
    const mla_array_list_t<mla_init_struct(mla_ui_control_input_area_t)> &inputAreas,
    mla_user_data_t& userData) {

    mla_size_t inputAreaCount = mla_array_list_size(inputAreas);
    mla_size_t inputEventCount = mla_array_list_size(inputEvents);

    for (mla_size_t i = 0; i < inputEventCount; i++) {

        const mla_ui_surface_input_event_t &event = mla_array_list_get_unsafe(inputEvents, i);

        // Check if its an click event
        if ((event.kind == MLA_UI_SURFACE_INPUT_EVENT_KIND_CLICK)) {
            // Best matching input area
            mla_ui_control_input_area_t bestMatchInputArea = mla_ui_control_input_area_empty();

            // Match against input areas
            for (mla_size_t j = 0; j < inputAreaCount; j++) {
                const mla_ui_control_input_area_t &inputArea = mla_array_list_get_unsafe(inputAreas, j);

                // Check if event kind is accepted by this input area
                if ((inputArea.acceptedEvents & MLA_UI_SURFACE_INPUT_EVENT_KIND_CLICK) == 0) {
                    continue;
                }

                // Check if event position is within input area
                mla_double_t areaX = inputArea.position.x;
                mla_double_t areaY = inputArea.position.y;
                mla_double_t areaW = inputArea.position.width;
                mla_double_t areaH = inputArea.position.height;

                if (event.click.position.x >= areaX && event.click.position.x <= (areaX + areaW) &&
                    event.click.position.y >= areaY && event.click.position.y <= (areaY + areaH)) {
                    // Found a matching input area Check if the area is more specific (smaller) than the current best match
                    mla_double_t bestMatchAreaSize =
                            bestMatchInputArea.position.width * bestMatchInputArea.position.height;
                    mla_double_t currentAreaSize = areaW * areaH;
                    if (bestMatchAreaSize == 0 || currentAreaSize < bestMatchAreaSize) {
                        bestMatchInputArea = inputArea;
                    }
                }
            }

            if (!mla_string_is_empty(bestMatchInputArea.controlId)) {
                // Find control by id
                mla_ui_control_t* control = nullptr;
                if (mla_ui_control_find_by_id(uiControls, bestMatchInputArea.controlId, control)) {
                    if (!mla_ui_control_has_focus(*control)) {
                        // set focus
                        mla_ui_control_reset_values(uiControls, mla_string_const(mla_ui_control_has_focus_attribute));
                        mla_ui_control_set_value_as_bool(*control, mla_string_const(mla_ui_control_has_focus_attribute),
                                                         true);
                    }

                    if (control->processClickEvent != nullptr) {
                        control->processClickEvent(*control, event.click, bestMatchInputArea, uiControls, userData);
                    }

                }
            } else {
                // No matching input area, reset focus on all controls
                mla_ui_control_reset_values(uiControls, mla_string_const(mla_ui_control_has_focus_attribute));
            }
        } else if (event.kind == MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR) {

            mla_ui_control_t* control = nullptr;
            // Find control with focus
            if (mla_ui_control_find_focused_control(uiControls, control)) {
                if (control->processCharInputEvent != nullptr) {
                    control->processCharInputEvent(*control, event.char_input, uiControls, userData);
                }
            }

        } else {
            mla_warning(
                mla_string_concat("mla_ui_control_process_input_events: Unsupported input event kind ",
                    mla_string_from_int32(event.kind)));
        }
    }
}
