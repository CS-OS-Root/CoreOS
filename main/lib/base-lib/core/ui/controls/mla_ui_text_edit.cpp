//
// Created by chris on 1/28/2026.
//

#include "mla_ui_text_edit.h"
#include "../../system/mla_string_concat.h"

mla_bool_t mla_private_ui_text_edit_process_char_input_event(mla_ui_control_t &control,
                                                       const mla_ui_surface_input_event_char_input_t &charInputEvent,
                                                       mla_array_list_t<mla_init_struct(mla_ui_control_t)>
                                                       &uiControls, mla_user_data_t& userData) {
    mla_string_t currentText = mla_ui_text_edit_get_text(control);
    mla_size_t cursorPosition = mla_ui_text_edit_get_cursor_position(control);

    // Ensure cursor matches text bounds
    mla_size_t textLen = mla_string_length(currentText);

    if (cursorPosition > textLen) {
        cursorPosition = textLen;
    }

    bool textModified = false;

    // Check modifiers

    const bool shiftPressed = (charInputEvent.pressedControlKeys & MLA_UI_SURFACE_INPUT_EVENT_KIND_CONTROL_SHIFT) != 0;
    const bool ctrlPressed = (charInputEvent.pressedControlKeys & MLA_UI_SURFACE_INPUT_EVENT_KIND_CONTROL_CTRL) != 0;

    if (charInputEvent.kind == MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_INPUT) {
        // Handle Ctrl + A (Select All)
        // Check for ASCII Control Code (1) or 'a'/'A' with Control modifier
        if ((charInputEvent.character[0] == 1) ||
            (ctrlPressed && (charInputEvent.character[0] == 'a' || charInputEvent.character[0] == 'A'))) {
            mla_ui_text_edit_set_selected_text(control, currentText);
            return true;
        }

        // Construct string from input char
        // charInputEvent.character is char[4] (utf-8 buffer), create string from it.

        mla_size_t stringLen = 0;
        while (stringLen < 4 && charInputEvent.character[stringLen] != '\0') {
            stringLen++;
        }

        mla_string_t newChar = mla_string_copy(charInputEvent.character, stringLen);

        // Split text at cursor
        mla_string_t prefix = mla_string_substr(currentText, 0, cursorPosition);
        mla_string_t suffix = mla_string_substr(currentText, cursorPosition, textLen - cursorPosition);

        // Reassemble: prefix + newChar + suffix
        currentText = mla_string_concat(prefix, newChar, suffix);

        cursorPosition += stringLen;
        textModified = true;
    } else if (charInputEvent.kind == MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_BACKSPACE) {
        if (cursorPosition > 0) {
            // Remove character before cursor
            mla_string_t prefix = mla_string_substr(currentText, 0, cursorPosition - 1);
            mla_string_t suffix = mla_string_substr(currentText, cursorPosition, textLen - cursorPosition);

            currentText = mla_string_concat(prefix, suffix);
            cursorPosition--;
            textModified = true;
        }
    } else if (charInputEvent.kind == MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_DELETE) {
        if (cursorPosition < textLen) {
            // Remove character after cursor
            mla_string_t prefix = mla_string_substr(currentText, 0, cursorPosition);
            mla_string_t suffix = mla_string_substr(currentText, cursorPosition + 1, textLen - (cursorPosition + 1));

            currentText = mla_string_concat(prefix, suffix);
            textModified = true;
        }
    } else if (charInputEvent.kind == MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_ARROW_LEFT) {
        mla_size_t oldCursorPos = cursorPosition;

        if (cursorPosition > 0) {
            cursorPosition--;
        }

        if (shiftPressed) {
            // Resolve anchor from current selection before update
            mla_size_t anchor = oldCursorPos;
            mla_string_t activeSel = mla_ui_text_edit_get_selected_text(control);
            if (!mla_string_is_empty(activeSel)) {
                mla_int32_t idx = mla_string_index_of(currentText, activeSel);
                if (idx > -1) {
                    mla_size_t selLen = mla_string_length(activeSel);
                    // If cursor was at start of selection, anchor is at end
                    if (oldCursorPos == mla_s_cast<mla_size_t>(idx)) {
                        anchor = idx + selLen;
                    }

                    // If cursor was at end of selection, anchor is at start
                    else if (oldCursorPos == idx + selLen) {
                        anchor = idx;
                    }
                }
            }

            // Calculate new selection range
            mla_size_t start = (cursorPosition < anchor) ? cursorPosition : anchor;
            mla_size_t len = (cursorPosition < anchor) ? (anchor - cursorPosition) : (cursorPosition - anchor);
            mla_ui_text_edit_set_selected_text(control, mla_string_substr(currentText, start, len));
        } else {
            mla_ui_text_edit_set_selected_text(control, mla_string_empty());
        }
    } else if (charInputEvent.kind == MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_ARROW_RIGHT) {
        mla_size_t oldCursorPos = cursorPosition;

        if (cursorPosition < textLen) {
            cursorPosition++;
        }

        if (shiftPressed) {
            // Resolve anchor
            mla_size_t anchor = oldCursorPos;
            mla_string_t activeSel = mla_ui_text_edit_get_selected_text(control);
            if (!mla_string_is_empty(activeSel)) {
                mla_int32_t idx = mla_string_index_of(currentText, activeSel);
                if (idx > -1) {
                    mla_size_t selLen = mla_string_length(activeSel);
                    if (oldCursorPos == mla_s_cast<mla_size_t>(idx)) {
                        anchor = idx + selLen;
                    }
                    else if (oldCursorPos == idx + selLen) {
                        anchor = idx;
                    }
                }
            }

            // Calculate new selection range
            mla_size_t start = (cursorPosition < anchor) ? cursorPosition : anchor;
            mla_size_t len = (cursorPosition < anchor) ? (anchor - cursorPosition) : (cursorPosition - anchor);
            mla_ui_text_edit_set_selected_text(control, mla_string_substr(currentText, start, len));
        } else {
            mla_ui_text_edit_set_selected_text(control, mla_string_empty());
        }
    }

    if (textModified) {
        mla_ui_text_edit_set_text(control, currentText);

        // Clear selection on text change
        mla_ui_text_edit_set_selected_text(control, mla_string_empty());
    }

    // Always update cursor position
    mla_ui_text_edit_set_cursor_position(control, cursorPosition);


    if (textModified) {
        // Trigger text changed event if set
        mla_ui_text_edit_text_changed_t textChangedEvent = mla_ui_text_edit_get_text_changed_event(control);
        if (textChangedEvent != nullptr) {
            textChangedEvent(control, uiControls, userData);
        }
    }

    return true;
}


mla_bool_t mla_private_ui_text_edit_render_to_drawCommands(const mla_ui_control_context_t &context,
                                                     const mla_ui_control_t &element,
                                                     mla_array_list_t<mla_init_struct(mla_ui_surface_draw_command_t)> &drawCommands,
                                                     mla_array_list_t<mla_init_struct(mla_ui_control_input_area_t)> &inputAreas) {
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

    const mla_bool_t disabled = mla_ui_text_edit_get_disable(element);
    mla_string_t text = mla_ui_text_edit_get_text(element);
    const mla_ui_text_edit_style_t style = mla_ui_text_edit_get_style(element);
    const mla_bool_t hasFocus = mla_ui_control_has_focus(element);

    if (style == MLA_UI_TEXT_EDIT_STYLE_PASSWORD) {
        // For password style, replace text with asterisks
        text = mla_string_repeat(mla_string_const("•"), mla_string_length(text));
    }

    // Calculate hover state
    const mla_ui_control_layout_t bounds = {x, y, w, h};
    const mla_bool_t hovered = !disabled && mla_ui_control_is_hovered(context, bounds);

    // Color definitions based on state
    mla_ui_surface_draw_command_color_t bgFill;
    mla_ui_surface_draw_command_color_t strokeColor;
    mla_ui_surface_draw_command_color_t textColor;

    // Font Setup
    mla_ui_surface_font_type_t fontType = mla_ui_surface_font_type_empty();
    fontType.family = mla_string_const("Arial");
    fontType.size = 12.0;
    fontType.bold = false;

    // We track if we are drawing a selection to alter text rendering logic later
    mla_string_t activeSelectedText = mla_string_empty();

    if (hasFocus && !disabled) {
        // Focused State colors
        textColor = {0, 0, 0, 255}; // Default text black

        // 1. Focus Ring
        mla_ui_surface_draw_command_t ringCmd = mla_ui_surface_draw_command_empty();
        ringCmd.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT;
        ringCmd.rect.x = context.offsetX + x + 0.5;
        ringCmd.rect.y = context.offsetY + y + 0.5;
        ringCmd.rect.width = w - 1.0;
        ringCmd.rect.height = h - 1.0;
        ringCmd.rect.rx = 7.0;
        ringCmd.rect.ry = 7.0;
        ringCmd.rect.color = {0, 0, 0, 0}; // Transparent fill
        ringCmd.rect.stroke = {39, 174, 96, 255}; // #27ae60
        ringCmd.rect.stroke_width = 1.0;
        mla_array_list_add(drawCommands, ringCmd);

        // 2. Input Background
        mla_ui_surface_draw_command_t bgCmd = mla_ui_surface_draw_command_empty();
        bgCmd.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT;
        bgCmd.rect.x = context.offsetX + x + 2.0;
        bgCmd.rect.y = context.offsetY + y + 2.0;
        bgCmd.rect.width = w - 4.0;
        bgCmd.rect.height = h - 4.0;
        bgCmd.rect.rx = 6.0;
        bgCmd.rect.ry = 6.0;
        bgCmd.rect.color = {255, 255, 255, 255}; // #ffffff
        mla_array_list_add(drawCommands, bgCmd);

        // 3. Selection Background (if any)
        mla_string_t selectedText = mla_ui_text_edit_get_selected_text(element);
        if (!mla_string_is_empty(selectedText) && !mla_string_is_empty(text)) {
            // Measure selected text width
            mla_ui_surface_draw_size_t selSize = {0, 0};
            if (context.calcTextSize != nullptr) {
                selSize = context.calcTextSize(context, fontType, selectedText);
            }

            // Calculate x-offset based on text before the selection
            mla_double_t selectionXOffset = 0.0;
            if (context.calcTextSize != nullptr) {
                const mla_int32_t idx = mla_string_index_of(text, selectedText);
                if (idx >= 0) {
                    const mla_string_t prefix = mla_string_substr(text, 0, idx);
                    selectionXOffset = context.calcTextSize(context, fontType, prefix).width;

                    // Store for text drawing logic
                    activeSelectedText = selectedText;
                }
            }

            // Draw selection rect
            mla_ui_surface_draw_command_t selCmd = mla_ui_surface_draw_command_empty();
            selCmd.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT;
            selCmd.rect.x = context.offsetX + x + 10.0 + selectionXOffset;
            selCmd.rect.y = context.offsetY + y + ((h - 16.0) / 2.0);
            selCmd.rect.width = selSize.width > 0 ? selSize.width : 5.0; // Fallback width
            selCmd.rect.height = 16.0;
            selCmd.rect.rx = 2.0;
            selCmd.rect.ry = 2.0;
            selCmd.rect.color = {0, 120, 212, 255}; // #0078d4
            mla_array_list_add(drawCommands, selCmd);
        } else {
            // 4. Cursor (only if no selection)

            // Use mla_system_time_ms() to drive cursor blink — visible for 500ms, hidden for 500ms
            const mla_uint64_t currentTimeMs = mla_system_time_ms();
            if ((currentTimeMs % 1000) < 500) {
                mla_double_t cursorXOffset = 0.0;

                if (context.calcTextSize != nullptr && !mla_string_is_empty(text)) {
                    mla_size_t cursorPosition = mla_ui_text_edit_get_cursor_position(element);

                    // Clamp position to valid range
                    if (cursorPosition > mla_string_length(text)) {
                        cursorPosition = mla_string_length(text);
                    }

                    // Measure text up to cursor
                    mla_string_t textBeforeCursor = mla_string_substr(text, 0, cursorPosition);
                    mla_ui_surface_draw_size_t txtSize = context.calcTextSize(context, fontType, textBeforeCursor);
                    cursorXOffset = txtSize.width;
                }

                mla_ui_surface_draw_command_t curCmd = mla_ui_surface_draw_command_empty();
                curCmd.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT;
                curCmd.rect.x = context.offsetX + x + 10.0 + cursorXOffset;
                curCmd.rect.y = context.offsetY + y + ((h - 14.0) / 2.0);
                curCmd.rect.width = 1.0;
                curCmd.rect.height = 14.0;
                curCmd.rect.color = {0, 0, 0, 255};
                mla_array_list_add(drawCommands, curCmd);
            }
        }
    } else {
        // Normal / Disabled / Hover Logic
        if (disabled) {
            bgFill = {242, 242, 242, 255};
            strokeColor = {208, 208, 208, 255};
            textColor = {158, 158, 158, 255};
        } else if (hovered) {
            bgFill = {255, 255, 255, 255};
            strokeColor = {158, 158, 158, 255};
            textColor = {0, 0, 0, 255};
        } else {
            bgFill = {255, 255, 255, 255};
            strokeColor = {200, 200, 200, 255};
            textColor = {0, 0, 0, 255};
        }

        mla_ui_surface_draw_command_t bgCmd = mla_ui_surface_draw_command_empty();
        bgCmd.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT;
        bgCmd.rect.x = context.offsetX + x + 1.0;
        bgCmd.rect.y = context.offsetY + y + 1.0;
        bgCmd.rect.width = w - 2.0;
        bgCmd.rect.height = h - 2.0;
        bgCmd.rect.rx = 6.0;
        bgCmd.rect.ry = 6.0;
        bgCmd.rect.color = bgFill;
        bgCmd.rect.stroke = strokeColor;
        bgCmd.rect.stroke_width = 1.0;
        mla_array_list_add(drawCommands, bgCmd);
    }

    // Draw Text
    // SVG: x=10, y=21
    if (!mla_string_is_empty(text)) {
        if (!mla_string_is_empty(activeSelectedText) && context.calcTextSize != nullptr) {
            // Split Text Rendering: Prefix(Black) - Selected(White) - Suffix(Black)

            mla_int32_t idx = mla_string_index_of(text, activeSelectedText);
            mla_size_t len = mla_string_length(activeSelectedText);

            mla_string_t prefix = mla_string_substr(text, 0, idx);
            mla_string_t suffix = mla_string_substr(text, idx + len, mla_string_length(text) - (idx + len));

            mla_double_t currentX = context.offsetX + x + 10.0;
            mla_double_t yPos = context.offsetY + y + (h / 2.0) - (fontType.size / 2.0);

            // 1. Draw Prefix (Black)
            if (!mla_string_is_empty(prefix)) {
                mla_ui_surface_draw_command_t txtCmd = mla_ui_surface_draw_command_empty();
                txtCmd.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT;
                txtCmd.text.x = currentX;
                txtCmd.text.y = yPos;
                txtCmd.text.content = prefix;
                txtCmd.text.font_type = fontType;
                txtCmd.text.fill = {0, 0, 0, 255};
                mla_array_list_add(drawCommands, txtCmd);

                // Advance X
                currentX += context.calcTextSize(context, fontType, prefix).width;
            }

            // 2. Draw Selection (White)
            {
                mla_ui_surface_draw_command_t txtCmd = mla_ui_surface_draw_command_empty();
                txtCmd.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT;
                txtCmd.text.x = currentX;
                txtCmd.text.y = yPos;
                txtCmd.text.content = activeSelectedText;
                txtCmd.text.font_type = fontType;
                txtCmd.text.fill = {255, 255, 255, 255}; // Highlight Text Color
                mla_array_list_add(drawCommands, txtCmd);

                // Advance X
                currentX += context.calcTextSize(context, fontType, activeSelectedText).width;
            }

            // 3. Draw Suffix (Black)
            if (!mla_string_is_empty(suffix)) {
                mla_ui_surface_draw_command_t txtCmd = mla_ui_surface_draw_command_empty();
                txtCmd.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT;
                txtCmd.text.x = currentX;
                txtCmd.text.y = yPos;
                txtCmd.text.content = suffix;
                txtCmd.text.font_type = fontType;
                txtCmd.text.fill = {0, 0, 0, 255};
                mla_array_list_add(drawCommands, txtCmd);
            }
        } else {
            // Standard Single Text Drawing
            mla_ui_surface_draw_command_t txtCmd = mla_ui_surface_draw_command_empty();
            txtCmd.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT;
            txtCmd.text.x = context.offsetX + x + 10.0;
            txtCmd.text.y = context.offsetY + y + (h / 2.0) - (fontType.size / 2.0);
            txtCmd.text.content = text;
            txtCmd.text.font_type = fontType;
            txtCmd.text.fill = textColor;
            mla_array_list_add(drawCommands, txtCmd);
        }
    }

    // Input Area for clicking/focusing
    if (!disabled) {
        mla_ui_control_input_area_t inputArea = mla_ui_control_input_area(
            element.id, element.layout, mla_string_const("edit"), MLA_UI_SURFACE_INPUT_EVENT_KIND_CLICK);
        mla_array_list_add(inputAreas, inputArea);
    }

    return true;
}


mla_ui_control_t mla_ui_text_edit() {
    mla_ui_control_t button = mla_ui_control();
    button.renderToDrawCommands = mla_private_ui_text_edit_render_to_drawCommands;
    button.processCharInputEvent = mla_private_ui_text_edit_process_char_input_event;
    return button;
}

mla_ui_text_edit_style_t mla_ui_text_edit_get_style(const mla_ui_control_t &textEdit) {
    return mla_s_cast<mla_ui_text_edit_style_t>(mla_ui_control_get_value_as_uint8(
        textEdit, mla_string_const("style"), MLA_UI_TEXT_EDIT_STYLE_STANDARD));
}

mla_bool_t mla_ui_text_edit_set_style(mla_ui_control_t &textEdit, mla_ui_text_edit_style_t style) {
    return mla_ui_control_set_value_as_uint8(textEdit, mla_string_const("style"), mla_s_cast<mla_uint8_t>(style));
}

mla_string_t mla_ui_text_edit_get_text(const mla_ui_control_t &textEdit) {
    return mla_ui_control_get_value_as_string(textEdit, mla_string_const("text"));
}

mla_bool_t mla_ui_text_edit_set_text(mla_ui_control_t &textEdit, const mla_string_t &text) {
    return mla_ui_control_set_value_as_string(textEdit, mla_string_const("text"), text);
}

mla_bool_t mla_ui_text_edit_get_disable(const mla_ui_control_t &textEdit) {
    return mla_ui_control_get_value_as_bool(textEdit, mla_string_const("disabled"), false);
}

mla_bool_t mla_ui_text_edit_set_disable(mla_ui_control_t &textEdit, mla_bool_t disable) {
    return mla_ui_control_set_value_as_bool(textEdit, mla_string_const("disabled"), disable);
}

mla_size_t mla_ui_text_edit_get_cursor_position(const mla_ui_control_t &textEdit) {
    mla_string_t text = mla_ui_text_edit_get_text(textEdit);
    return mla_ui_control_get_value_as_uint32(textEdit, mla_string_const("cursor_position"), mla_string_length(text));
}

mla_bool_t mla_ui_text_edit_set_cursor_position(mla_ui_control_t &textEdit, mla_size_t position) {
    return mla_ui_control_set_value_as_uint32(textEdit, mla_string_const("cursor_position"), position);
}

mla_string_t mla_ui_text_edit_get_selected_text(const mla_ui_control_t &textEdit) {
    return mla_ui_control_get_value_as_string(textEdit, mla_string_const("selected_text"));
}

mla_bool_t mla_ui_text_edit_set_selected_text(mla_ui_control_t &textEdit, const mla_string_t &selectedText) {
    // Check if the selectedText is a substring of the current text
    mla_string_t currentText = mla_ui_text_edit_get_text(textEdit);

    if (mla_string_contains(currentText, selectedText)) {
        return mla_ui_control_set_value_as_string(textEdit, mla_string_const("selected_text"), selectedText);
    }

    return false;
}

mla_ui_text_edit_text_changed_t mla_ui_text_edit_get_text_changed_event(const mla_ui_control_t &textEdit) {
    return mla_r_cast<mla_ui_text_edit_text_changed_t>(mla_ui_control_get_value_as_pointer(
        textEdit, mla_string_const("text_changed_event"), nullptr));
}

mla_bool_t mla_ui_text_edit_set_text_changed_event(mla_ui_control_t &textEdit,
                                                   mla_ui_text_edit_text_changed_t textChangedEvent) {
    return mla_ui_control_set_value_as_pointer(textEdit, mla_string_const("text_changed_event"),
                                               mla_r_cast<void *>(textChangedEvent));
}
