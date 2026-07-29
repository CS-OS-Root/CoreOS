#include "mla_ui_button.h"
#include "mla_ui_style.h"


static void mla_private_ui_button_calc_text_size(const mla_ui_control_context_t &context, const mla_ui_surface_font_type_t& fontType, const mla_string_t& text, mla_double_t& textWidth, mla_double_t& textHeight) {
    if (context.calcTextSize != nullptr) {
        mla_ui_surface_draw_size_t size = context.calcTextSize(context, fontType,text);
        textWidth = size.width;
        textHeight = size.height;
    } else {
        textWidth = mla_s_cast<mla_double_t>(mla_string_length(text)) * (mla_s_cast<mla_double_t>(fontType.size) * 0.5);
        textHeight = mla_s_cast<mla_double_t>(fontType.size);
    }
}

mla_bool_t mla_private_ui_button_process_click_event(mla_ui_control_t &control, const mla_ui_surface_input_event_click_t &clickEvent, const mla_ui_control_input_area_t &inputArea, mla_array_list_t<mla_init_struct(mla_ui_control_t)> &uiControls, mla_user_data_t& userData) {

    if (clickEvent.button != MLA_UI_SURFACE_INPUT_EVENT_CLICK_BUTTON_LEFT) {
        return false;
    }

    const mla_bool_t disabled = mla_ui_button_get_disable(control);
    if (disabled) {
        return false;
    }

    // check if it is the correct input area
    if (mla_string_equals(inputArea.event_name, mla_string_const("click"))) {

        mla_ui_button_click_event_t onClickEvent = mla_ui_button_get_click_event(control);
        if (onClickEvent != nullptr) {
            onClickEvent(control, clickEvent, uiControls, userData);
        }
    }

    return true;
}

mla_bool_t mla_private_ui_button_render_to_drawCommands(const mla_ui_control_context_t &context, const mla_ui_control_t &element, mla_array_list_t<mla_init_struct(mla_ui_surface_draw_command_t)>& drawCommands, mla_array_list_t<mla_init_struct(mla_ui_control_input_area_t)> &inputAreas) {

    mla_double_t x = element.layout.x;
    mla_double_t y = element.layout.y;
    mla_double_t w = element.layout.width;
    mla_double_t h = element.layout.height;

    // Default sizing
    if (w == 0) {
        w = context.width - x;
    }
    if (h == 0) {
        h = context.height - y;
    }

    const mla_ui_button_style_t style = mla_ui_button_get_style(element);

    // Custom style delegates rendering to children (handled externally) or does nothing here
    if (style == MLA_UI_BUTTON_STYLE_CUSTOM) {
        return true;
    }

    const mla_string_t text = mla_ui_button_get_text(element);
    const mla_bool_t disabled = mla_ui_button_get_disable(element);

    // Calculate hover and pressed state
    const mla_ui_control_layout_t bounds = { x, y, w, h };
    const mla_bool_t hovered = !disabled && mla_ui_control_is_hovered(context, bounds);

    // Pressed if hovered and any mouse button is down
    const mla_bool_t pressed = hovered && (context.input_states.leftMouseButtonDown || context.input_states.rightMouseButtonDown || context.input_states.middleMouseButtonDown);

    // Style variables
    mla_ui_surface_draw_command_color_t bgFill = {0, 0, 0, 0};
    mla_ui_surface_draw_command_color_t bgStroke = {0, 0, 0, 0};
    mla_double_t bgStrokeWidth = 0.0;
    mla_ui_surface_draw_command_color_t textColor = {0, 0, 0, 255};
    mla_bool_t drawBackground = false;

    // Rect geometry relative to button position
    mla_double_t rx = 6.0;
    mla_double_t ry = 6.0;
    mla_double_t rX = 0.0;
    mla_double_t rY = 0.0;
    mla_double_t rW = w;
    mla_double_t rH = h;

    // Font configuration
    mla_ui_surface_font_type_t fontType = mla_ui_surface_font_type_empty();
    fontType.family = mla_string_const(MLA_UI_FONT_FAMILY_DEFAULT);
    fontType.size = MLA_UI_FONT_SIZE_BUTTON;
    fontType.bold = false;

    // Apply styles
    switch (style) {
        case MLA_UI_BUTTON_STYLE_PRIMARY:
            drawBackground = true;
            // Inset 1px to mimic SVG padding if necessary, or just use full size.
            // SVGs show x=1, y=1, w=118, h=30 in 120x32 box.
            rX = 1.0; rY = 1.0; rW -= 2.0; rH -= 2.0;

            if (disabled) {
                bgFill = MLA_UI_COLOR_BUTTON_PRIMARY_BG_DISABLED;
                textColor = MLA_UI_COLOR_BUTTON_PRIMARY_TEXT_DISABLED;
            } else if (pressed) {
                bgFill = MLA_UI_COLOR_BUTTON_PRIMARY_BG_PRESSED;
                textColor = MLA_UI_COLOR_BUTTON_PRIMARY_TEXT_ENABLED;
            } else if (hovered) {
                bgFill = MLA_UI_COLOR_BUTTON_PRIMARY_BG_HOVERED;
                textColor = MLA_UI_COLOR_BUTTON_PRIMARY_TEXT_HOVERED;
            } else {
                bgFill = MLA_UI_COLOR_BUTTON_PRIMARY_BG_ENABLED;
                textColor = MLA_UI_COLOR_BUTTON_PRIMARY_TEXT_ENABLED;
            }
            break;

        case MLA_UI_BUTTON_STYLE_SECONDARY:
            drawBackground = true;
            bgStrokeWidth = 1.0;
            rX = 1.0; rY = 1.0; rW -= 2.0; rH -= 2.0;

            if (disabled) {
                bgFill = MLA_UI_COLOR_BUTTON_SECONDARY_BG_DISABLED;
                bgStroke = MLA_UI_COLOR_BUTTON_SECONDARY_BORDER_DISABLED;
                textColor = MLA_UI_COLOR_BUTTON_SECONDARY_TEXT_DISABLED;
            } else if (pressed) {
                bgFill = MLA_UI_COLOR_BUTTON_SECONDARY_BG_PRESSED;
                bgStroke = MLA_UI_COLOR_BUTTON_SECONDARY_BORDER_PRESSED;
                textColor = MLA_UI_COLOR_BUTTON_SECONDARY_TEXT_ENABLED;
            } else if (hovered) {
                bgFill = MLA_UI_COLOR_BUTTON_SECONDARY_BG_HOVERED;
                bgStroke = MLA_UI_COLOR_BUTTON_SECONDARY_BORDER_HOVERED;
                textColor = MLA_UI_COLOR_BUTTON_SECONDARY_TEXT_HOVERED;
            } else {
                bgFill = MLA_UI_COLOR_BUTTON_SECONDARY_BG_ENABLED;
                bgStroke = MLA_UI_COLOR_BUTTON_SECONDARY_BORDER_ENABLED;
                textColor = MLA_UI_COLOR_BUTTON_SECONDARY_TEXT_ENABLED;
            }
            break;

        case MLA_UI_BUTTON_STYLE_TERTIARY:
             if (disabled) {
                textColor = MLA_UI_COLOR_BUTTON_TERTIARY_TEXT_DISABLED;
            } else if (pressed) {
                 drawBackground = true;
                 bgFill = MLA_UI_COLOR_BUTTON_TERTIARY_BG_PRESSED;
                 textColor = MLA_UI_COLOR_BUTTON_TERTIARY_TEXT_ENABLED;
                 rx = 4.0; ry = 4.0;
                 // Specific geometry for Tertiary Pressed: x=8, y=4, w=104(-16), h=24(-8)
                 rX += 8.0; rY += 4.0; rW -= 16.0; rH -= 8.0;
            } else if (hovered) {
                drawBackground = true;
                bgFill = MLA_UI_COLOR_BUTTON_TERTIARY_BG_HOVERED;
                textColor = MLA_UI_COLOR_BUTTON_TERTIARY_TEXT_HOVERED;
                rx = 4.0; ry = 4.0;
                // Specific geometry for Tertiary Hover: x+10, y+4, w-20, h-8
                rX += 10.0; rY += 4.0; rW -= 20.0; rH -= 8.0;
            } else {
                textColor = MLA_UI_COLOR_BUTTON_TERTIARY_TEXT_ENABLED;
            }
            break;

        case MLA_UI_BUTTON_STYLE_LINK:
             if (disabled) {
                 textColor = MLA_UI_COLOR_TEXT_DISABLED;
             } else if (hovered) {
                 // Link has no distinct pressed style requested, treating as hovered
                 textColor = MLA_UI_COLOR_TEXT_LINK_HOVER;
                 fontType.bold = true;
             } else {
                 textColor = MLA_UI_COLOR_TEXT_LINK;
             }
             break;

        default:
            break;
    }

    // Draw Background
    if (drawBackground) {
        mla_ui_surface_draw_command_t bgCmd = mla_ui_surface_draw_command_empty();
        bgCmd.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT;
        bgCmd.rect.x = context.offsetX + x + rX;
        bgCmd.rect.y = context.offsetY + y + rY;
        bgCmd.rect.width = rW;
        bgCmd.rect.height = rH;
        bgCmd.rect.rx = rx;
        bgCmd.rect.ry = ry;
        bgCmd.rect.color = bgFill;
        bgCmd.rect.stroke = bgStroke;
        bgCmd.rect.stroke_width = bgStrokeWidth;
        mla_array_list_add(drawCommands, bgCmd);
    }

    // Process Text
    if (!mla_string_is_empty(text)) {
        mla_double_t textWidth = 0.0;
        mla_double_t textHeight = 0.0;
        mla_private_ui_button_calc_text_size(context, fontType, text, textWidth, textHeight);

        if (textWidth != 0.0 && textHeight != 0.0) {
            // Only draw text if it has measurable size (prevents issues with empty strings or zero-size fonts)

            mla_ui_surface_draw_command_t txtCmd = mla_ui_surface_draw_command_empty();
            txtCmd.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT;

            // Center text in button area
            // X: Absolute X + (Button Width - Text Width) / 2
            txtCmd.text.x = context.offsetX + x + (w / 2.0) - (textWidth / 2.0);

            // Y: Absolute Y + (Button Height / 2) - (Text Height / 2) - 1 (SVG baseline adjustment)
            txtCmd.text.y = context.offsetY + y + (h / 2.0) - (textHeight / 2.0) - 1.0;

            txtCmd.text.content = text;
            txtCmd.text.font_type = fontType;
            txtCmd.text.fill = textColor;

            mla_array_list_add(drawCommands, txtCmd);

            // Draw Underline for Links
            if (style == MLA_UI_BUTTON_STYLE_LINK) {
                mla_ui_surface_draw_command_t underscore = mla_ui_surface_draw_command_empty();
                underscore.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINE;
                underscore.line.x1 = txtCmd.text.x;
                // Position slightly below text baseline
                underscore.line.y1 = txtCmd.text.y + textHeight + 2.0;
                underscore.line.x2 = txtCmd.text.x + textWidth;
                underscore.line.y2 = underscore.line.y1;
                underscore.line.stroke = textColor;
                underscore.line.stroke_width = 1.0;

                mla_array_list_add(drawCommands, underscore);
            }
        }
    }

    if (!disabled) {
        // Add Input Area for button if not disabled
        mla_ui_control_input_area_t inputArea = mla_ui_control_input_area(element.id, element.layout, mla_string_const("click"), MLA_UI_SURFACE_INPUT_EVENT_KIND_CLICK);
        mla_array_list_add(inputAreas, inputArea);
    }

    return true;
}

mla_ui_control_t mla_ui_button() {

    mla_ui_control_t button = mla_ui_control();
    button.renderToDrawCommands = mla_private_ui_button_render_to_drawCommands;
    button.processClickEvent = mla_private_ui_button_process_click_event;
    return button;
}

mla_ui_button_style_t mla_ui_button_get_style(const mla_ui_control_t &button) {
    return mla_s_cast<mla_ui_button_style_t>(mla_ui_control_get_value_as_uint8(button, mla_string_const("style"), MLA_UI_BUTTON_STYLE_PRIMARY));
}

mla_bool_t mla_ui_button_set_style(mla_ui_control_t &button, mla_ui_button_style_t style) {
    return mla_ui_control_set_value_as_uint8(button, mla_string_const("style"), mla_s_cast<mla_uint8_t>(style));
}

mla_bool_t mla_ui_button_get_disable(const mla_ui_control_t &button) {
    return mla_ui_control_get_value_as_bool(button, mla_string_const("disabled"), false);
}

mla_bool_t mla_ui_button_set_disable(mla_ui_control_t &button, mla_bool_t disable) {
    return mla_ui_control_set_value_as_bool(button, mla_string_const("disabled"), disable);
}

mla_string_t mla_ui_button_get_text(const mla_ui_control_t &button) {
    return mla_ui_control_get_value_as_string(button, mla_string_const("text"), mla_string_empty());
}

mla_bool_t mla_ui_button_set_text(mla_ui_control_t &button, const mla_string_t& text) {
    return mla_ui_control_set_value_as_string(button, mla_string_const("text"), text);
}

mla_ui_button_click_event_t mla_ui_button_get_click_event(const mla_ui_control_t &button) {
    return mla_r_cast<mla_ui_button_click_event_t>(mla_ui_control_get_value_as_pointer(button, mla_string_const("click"), nullptr));
}

mla_bool_t mla_ui_button_set_click_event(mla_ui_control_t &button, mla_ui_button_click_event_t processClickEvent) {
    return mla_ui_control_set_value_as_pointer(button, mla_string_const("click"), mla_r_cast<mla_platform_pointer_t>(processClickEvent));
}
