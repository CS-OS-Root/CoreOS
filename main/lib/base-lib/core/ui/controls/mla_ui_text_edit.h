//
// Created by chris on 1/28/2026.
//

#ifndef MLA_UI_TEXT_EDIT_H
#define MLA_UI_TEXT_EDIT_H

#include "mla_ui_control.h"
#include "mla_ui_style.h"

typedef void (*mla_ui_text_edit_text_changed_t)(mla_ui_control_t &control, mla_array_list_t<mla_init_struct(mla_ui_control_t)> &uiControls, mla_user_data_t& userData);

mla_ui_control_t mla_ui_text_edit();

mla_ui_text_edit_style_t mla_ui_text_edit_get_style(const mla_ui_control_t &textEdit);
mla_bool_t mla_ui_text_edit_set_style(mla_ui_control_t &textEdit, mla_ui_text_edit_style_t style);

mla_string_t mla_ui_text_edit_get_text(const mla_ui_control_t &textEdit);
mla_bool_t mla_ui_text_edit_set_text(mla_ui_control_t &textEdit, const mla_string_t& text);

mla_bool_t mla_ui_text_edit_get_disable(const mla_ui_control_t &textEdit);
mla_bool_t mla_ui_text_edit_set_disable(mla_ui_control_t &textEdit, mla_bool_t disable);

mla_size_t mla_ui_text_edit_get_cursor_position(const mla_ui_control_t &textEdit);
mla_bool_t mla_ui_text_edit_set_cursor_position(mla_ui_control_t &textEdit, mla_size_t position);

mla_string_t mla_ui_text_edit_get_selected_text(const mla_ui_control_t &textEdit);
mla_bool_t mla_ui_text_edit_set_selected_text(mla_ui_control_t &textEdit, const mla_string_t& selectedText);

mla_ui_text_edit_text_changed_t mla_ui_text_edit_get_text_changed_event(const mla_ui_control_t &textEdit);
mla_bool_t mla_ui_text_edit_set_text_changed_event(mla_ui_control_t &textEdit, mla_ui_text_edit_text_changed_t textChangedEvent);

#endif