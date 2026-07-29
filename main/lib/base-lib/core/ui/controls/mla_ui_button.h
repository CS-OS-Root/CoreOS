//
// Created by chris on 1/23/2026.
//

#ifndef MLA_UI_BUTTON_H
#define MLA_UI_BUTTON_H

#include "mla_ui_control.h"
#include "mla_ui_style.h"

typedef void (*mla_ui_button_click_event_t)(mla_ui_control_t &control, const mla_ui_surface_input_event_click_t &clickEvent, mla_array_list_t<mla_init_struct(mla_ui_control_t)> &uiControls, mla_user_data_t& userData);

mla_ui_control_t mla_ui_button();

mla_ui_button_style_t mla_ui_button_get_style(const mla_ui_control_t &button);
mla_bool_t mla_ui_button_set_style(mla_ui_control_t &button, mla_ui_button_style_t style);

mla_bool_t mla_ui_button_get_disable(const mla_ui_control_t &button);
mla_bool_t mla_ui_button_set_disable(mla_ui_control_t &button, mla_bool_t disable);

mla_string_t mla_ui_button_get_text(const mla_ui_control_t &button);
mla_bool_t mla_ui_button_set_text(mla_ui_control_t &button, const mla_string_t& text);

mla_ui_button_click_event_t mla_ui_button_get_click_event(const mla_ui_control_t &button);
mla_bool_t mla_ui_button_set_click_event(mla_ui_control_t &button, mla_ui_button_click_event_t clickEvent);


#endif