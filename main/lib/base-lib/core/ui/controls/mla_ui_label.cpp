//
// Created by chris on 1/19/2026.
//

#include "mla_ui_label.h"
#include "mla_ui_style.h"

void mla_private_ui_label_calc_text_size(const mla_ui_control_context_t &context, const mla_ui_surface_font_type_t& fontType, const mla_string_t& text, mla_double_t& textWidth, mla_double_t& textHeight) {

    if (context.calcTextSize != nullptr) {
        mla_ui_surface_draw_size_t size = context.calcTextSize(context, fontType,text);
        textWidth = size.width;
        textHeight = size.height;
    } else {
        // Approximate text width calculation based on font size and character count (fallback)
        textWidth = mla_s_cast<mla_double_t>(mla_string_length(text)) * (mla_s_cast<mla_double_t>(fontType.size) * 0.5);
        textHeight = mla_s_cast<mla_double_t>(fontType.size);
    }

}

mla_bool_t mla_private_ui_label_render_to_drawCommands(const mla_ui_control_context_t &context, const mla_ui_control_t &element, mla_array_list_t<mla_init_struct(mla_ui_surface_draw_command_t)>& drawCommands, mla_array_list_t<mla_init_struct(mla_ui_control_input_area_t)> &inputAreas) {

    (void)inputAreas;

    // Resolve position and size (same approach as window: 0 means "use remaining context")
    mla_double_t x = element.layout.x;
    mla_double_t y = element.layout.y;


    const mla_string_t text = mla_ui_label_get_text(element);
    if (mla_string_is_empty(text)) {
        return true; // Nothing to render is not an error
    }

    const mla_uint16_t fontSize = mla_ui_label_get_font_size(element);
    const mla_ui_text_kind_t kind = mla_ui_label_get_text_kind(element);

    mla_ui_surface_draw_command_color_t color;

    switch (kind) {
        case MLA_UI_TEXT_KIND_SECONDARY:
            color = MLA_UI_COLOR_TEXT_SECONDARY;
            break;
        case MLA_UI_TEXT_KIND_DISABLED:
            color = MLA_UI_COLOR_TEXT_DISABLED;
            break;
        case MLA_UI_TEXT_KIND_LINK:
            color = MLA_UI_COLOR_TEXT_LINK;
            break;
        case MLA_UI_TEXT_KIND_LINK_DISABLED:
            color = MLA_UI_COLOR_TEXT_LINK_DISABLED;
            break;
        case MLA_UI_TEXT_KIND_ERROR:
            color = MLA_UI_COLOR_TEXT_ERROR;
            break;
        case MLA_UI_TEXT_KIND_WARNING:
            color = MLA_UI_COLOR_TEXT_WARNING;
            break;
        case MLA_UI_TEXT_KIND_SUCCESS:
            color = MLA_UI_COLOR_TEXT_SUCCESS;
            break;
        case MLA_UI_TEXT_KIND_INFO:
            color = MLA_UI_COLOR_TEXT_INFO;
            break;
        case MLA_UI_TEXT_KIND_CUSTOM:
        {
            const mla_string_t customColorStr = mla_ui_label_get_custom_color(element);
            if (!mla_string_is_empty(customColorStr)) {
                mla_ui_surface_draw_command_color_t customColor = {0, 0, 0, 255};
                if (mla_ui_surface_parse_color_from_hex_string(customColorStr, customColor)) {
                    color = customColor;
                    break;
                }
            }
            // Fallback to primary if custom color is invalid
            color = MLA_UI_COLOR_TEXT_PRIMARY;
            break;
        }
        case MLA_UI_TEXT_KIND_PRIMARY:
        default:
            color = MLA_UI_COLOR_TEXT_PRIMARY;
            break;
    }

    mla_double_t textWidth = 0.0;
    mla_double_t textHeight = 0.0;

    mla_ui_surface_font_type_t fontType = mla_ui_surface_font_type_empty();
    fontType.family = mla_string_const(MLA_UI_FONT_FAMILY_DEFAULT);
    fontType.size = mla_s_cast<mla_double_t>(fontSize);

    if (kind == MLA_UI_TEXT_KIND_LINK || kind == MLA_UI_TEXT_KIND_LINK_DISABLED) {
        mla_private_ui_label_calc_text_size(context, fontType, text, textWidth, textHeight);
    }

    mla_ui_surface_draw_command_t command = mla_ui_surface_draw_command_empty();
    command.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT;

    // SVG x="2" -> 2px offset
    command.text.x = context.offsetX + x + 2.0;
    // Use calculated baseline
    command.text.y = context.offsetY + y;

    command.text.content = text;
    command.text.font_type = fontType;
    command.text.fill = color;

    if (kind == MLA_UI_TEXT_KIND_LINK) {
        // Check if the text is hovered if yes change color to hover color
        const mla_ui_control_layout_t textLayout = {x + 2.0, y, textWidth, textHeight};
        if (mla_ui_control_is_hovered(context, textLayout)) {
            color = MLA_UI_COLOR_TEXT_LINK_HOVER;
            command.text.font_type.bold = true;

            // Recalculate text size for bold font
            mla_private_ui_label_calc_text_size(context, command.text.font_type, text, textWidth, textHeight);
        }
    }

    mla_array_list_add(drawCommands, command);

    // Add Underline for Links
    if (kind == MLA_UI_TEXT_KIND_LINK || kind == MLA_UI_TEXT_KIND_LINK_DISABLED) {

        mla_ui_surface_draw_command_t underscore = mla_ui_surface_draw_command_empty();
        underscore.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINE;
        underscore.line.x1 = command.text.x;
        // Position slightly below text baseline
        underscore.line.y1 = command.text.y + textHeight;
        underscore.line.x2 = command.text.x + textWidth;
        underscore.line.y2 = underscore.line.y1;
        underscore.line.stroke = color;
        underscore.line.stroke_width = 1.0;

        mla_array_list_add(drawCommands, underscore);
    }

    return true;

}

mla_ui_control_t mla_ui_label() {

    mla_ui_control_t label = mla_ui_control();
    label.renderToDrawCommands = mla_private_ui_label_render_to_drawCommands;
    return label;
}

mla_string_t mla_ui_label_get_text(const mla_ui_control_t &label) {
    return mla_ui_control_get_value_as_string(label, mla_string_const("text"), mla_string_empty());
}
mla_bool_t mla_ui_label_set_text(mla_ui_control_t &label, const mla_string_t& text) {
    return mla_ui_control_set_value_as_string(label, mla_string_const("text"), text);
}

mla_uint16_t mla_ui_label_get_font_size(const mla_ui_control_t &label) {
    return mla_s_cast<mla_uint16_t>(mla_ui_control_get_value_as_uint16(label, mla_string_const("font_size"), MLA_UI_FONT_SIZE_DEFAULT));
}

mla_bool_t mla_ui_label_set_font_size(mla_ui_control_t &label, mla_uint16_t fontSize) {
    return mla_ui_control_set_value_as_uint16(label, mla_string_const("font_size"), fontSize);
}

mla_ui_text_kind_t mla_ui_label_get_text_kind(const mla_ui_control_t &label) {
    return mla_s_cast<mla_ui_text_kind_t>(mla_ui_control_get_value_as_uint8(label, mla_string_const("text_kind"), MLA_UI_TEXT_KIND_PRIMARY));
}

mla_bool_t mla_ui_label_set_text_kind(mla_ui_control_t &label, mla_ui_text_kind_t kind) {
    return mla_ui_control_set_value_as_uint8(label, mla_string_const("text_kind"), mla_s_cast<mla_uint8_t>(kind));
}

mla_string_t mla_ui_label_get_custom_color(const mla_ui_control_t &label) {
    return mla_ui_control_get_value_as_string(label, mla_string_const("custom_color"), mla_string_empty());
}

mla_bool_t mla_ui_label_set_custom_color(mla_ui_control_t &label, const mla_string_t& color) {
    return mla_ui_control_set_value_as_string(label, mla_string_const("custom_color"), color);
}
