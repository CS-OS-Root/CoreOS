//
// Created by chris on 2/24/2026.
//

#ifndef MLA_MAIN_APP_MAIN_WINDOW_H
#define MLA_MAIN_APP_MAIN_WINDOW_H

#include "../../lib/base-lib/core/ui/controls/mla_ui_control.h"
#include "../../lib/base-lib/core/ui/surfaces/mla_ui_surface.h"
#include "../../lib/base-lib/core/ui/controls/mla_ui_label.h"
#include "../../lib/base-lib/core/ui/controls/mla_ui_button.h"
#include "../../lib/base-lib/core/ui/controls/mla_ui_text_edit.h"
#include "../../lib/base-lib/core/ui/controls/mla_ui_loading_indicator.h"

struct main_app_main_window_t {

    static void mla_private_primary_button_clicked(mla_ui_control_t &control,
                                                  const mla_ui_surface_input_event_click_t &clickEvent,
                                                  mla_array_list_t<mla_init_struct(mla_ui_control_t)> &
                                                  uiControls, mla_user_data_t &userData) {
        (void) control;
        (void) clickEvent;
        (void) uiControls;
        (void) userData;

        mla_ui_control_t *disabledButton = nullptr;

        if (!mla_ui_control_find_by_id(uiControls, mla_string_const("primary_disabled_button"), disabledButton)) {
            return;
        }

        mla_ui_button_set_disable(*disabledButton, !mla_ui_button_get_disable(*disabledButton));
    }

    static mla_bool_t renderer(mla_array_list_t<mla_init_struct(mla_ui_control_t)> &root, const mla_ui_surface_size_t& surfaceSize, const mla_ui_surface_input_states_t &input_states) {

        if (mla_array_list_size(root) > 0) {
            return true;
        }

        // Create example labels (Left Column)
        mla_ui_control_t control = mla_ui_label();
        mla_ui_label_set_text(control, mla_string_const("Main Header Label"));
        mla_ui_label_set_font_size(control, MLA_UI_FONT_SIZE_PAGE_TITLE);
        mla_ui_label_set_text_kind(control, MLA_UI_TEXT_KIND_PRIMARY);
        control.layout.x = 20;
        control.layout.y = 20;
        control.layout.width = 400;
        control.layout.height = 40;
        mla_array_list_add(root, control);

        control = mla_ui_label();
        mla_ui_label_set_text(control, mla_string_const("Secondary Subtitle Style"));
        mla_ui_label_set_font_size(control, MLA_UI_FONT_SIZE_SECTION);
        mla_ui_label_set_text_kind(control, MLA_UI_TEXT_KIND_SECONDARY);
        control.layout.x = 20;
        control.layout.y = 70;
        control.layout.width = 400;
        control.layout.height = 30;
        mla_array_list_add(root, control);

        control = mla_ui_label();
        mla_ui_label_set_text(control, mla_string_const("Standard body text example at size 14."));
        mla_ui_label_set_font_size(control, MLA_UI_FONT_SIZE_DEFAULT);
        control.layout.x = 20;
        control.layout.y = 110;
        control.layout.width = 400;
        control.layout.height = 20;
        mla_array_list_add(root, control);

        control = mla_ui_label();
        mla_ui_label_set_text(control, mla_string_const("Error: Connection timeout occurred"));
        mla_ui_label_set_text_kind(control, MLA_UI_TEXT_KIND_ERROR);
        control.layout.x = 20;
        control.layout.y = 140;
        control.layout.width = 400;
        control.layout.height = 20;
        mla_array_list_add(root, control);

        control = mla_ui_label();
        mla_ui_label_set_text(control, mla_string_const("Success: File saved successfully"));
        mla_ui_label_set_text_kind(control, MLA_UI_TEXT_KIND_SUCCESS);
        control.layout.x = 20;
        control.layout.y = 170;
        control.layout.width = 400;
        control.layout.height = 20;
        mla_array_list_add(root, control);

        control = mla_ui_label();
        mla_ui_label_set_text(control, mla_string_const("Click here for more details"));
        mla_ui_label_set_text_kind(control, MLA_UI_TEXT_KIND_LINK);
        control.layout.x = 20;
        control.layout.y = 200;
        control.layout.width = 400;
        control.layout.height = 20;
        mla_array_list_add(root, control);

        control = mla_ui_label();
        mla_ui_label_set_text(control, mla_string_const("Disabled Text Example"));
        mla_ui_label_set_text_kind(control, MLA_UI_TEXT_KIND_DISABLED);
        control.layout.x = 20;
        control.layout.y = 230;
        control.layout.width = 400;
        control.layout.height = 20;
        mla_array_list_add(root, control);

        control = mla_ui_label();
        mla_ui_label_set_text(control, mla_string_const("Disabled Link Example"));
        mla_ui_label_set_text_kind(control, MLA_UI_TEXT_KIND_LINK_DISABLED);
        control.layout.x = 20;
        control.layout.y = 260;
        control.layout.width = 400;
        control.layout.height = 20;
        mla_array_list_add(root, control);

        control = mla_ui_label();
        mla_ui_label_set_text(control, mla_string_const("Warning: Battery low"));
        mla_ui_label_set_text_kind(control, MLA_UI_TEXT_KIND_WARNING);
        control.layout.x = 20;
        control.layout.y = 290;
        control.layout.width = 400;
        control.layout.height = 20;
        mla_array_list_add(root, control);

        control = mla_ui_label();
        mla_ui_label_set_text(control, mla_string_const("Info: System normal"));
        mla_ui_label_set_text_kind(control, MLA_UI_TEXT_KIND_INFO);
        control.layout.x = 20;
        control.layout.y = 320;
        control.layout.width = 400;
        control.layout.height = 20;
        mla_array_list_add(root, control);

        control = mla_ui_label();
        mla_ui_label_set_text(control, mla_string_const("Custom: Specific Hex Color"));
        mla_ui_label_set_text_kind(control, MLA_UI_TEXT_KIND_CUSTOM);
        mla_ui_label_set_custom_color(control, mla_string_const("#E040FB"));
        control.layout.x = 20;
        control.layout.y = 350;
        control.layout.width = 400;
        control.layout.height = 20;
        mla_array_list_add(root, control);

        // Create example buttons (Right Columns)


        // --- Primary ---
        control = mla_ui_button();
        mla_ui_button_set_text(control, mla_string_const("Primary"));
        mla_ui_button_set_style(control, MLA_UI_BUTTON_STYLE_PRIMARY);
        mla_ui_button_set_click_event(control, mla_private_primary_button_clicked);
        control.layout.x = 440;
        control.layout.y = 20;
        control.layout.width = 120;
        control.layout.height = 32;
        mla_array_list_add(root, control);

        control = mla_ui_button();
        control.id = mla_string_const("primary_disabled_button");
        mla_ui_button_set_text(control, mla_string_const("Primary Dis"));
        mla_ui_button_set_style(control, MLA_UI_BUTTON_STYLE_PRIMARY);
        mla_ui_button_set_disable(control, true);
        control.layout.x = 580;
        control.layout.y = 20;
        control.layout.width = 120;
        control.layout.height = 32;
        mla_array_list_add(root, control);

        // --- Secondary ---
        control = mla_ui_button();
        mla_ui_button_set_text(control, mla_string_const("Secondary"));
        mla_ui_button_set_style(control, MLA_UI_BUTTON_STYLE_SECONDARY);
        control.layout.x = 440;
        control.layout.y = 70;
        control.layout.width = 120;
        control.layout.height = 32;
        mla_array_list_add(root, control);

        control = mla_ui_button();
        mla_ui_button_set_text(control, mla_string_const("Secondary Dis"));
        mla_ui_button_set_style(control, MLA_UI_BUTTON_STYLE_SECONDARY);
        mla_ui_button_set_disable(control, true);
        control.layout.x = 580;
        control.layout.y = 70;
        control.layout.width = 120;
        control.layout.height = 32;
        mla_array_list_add(root, control);

        // --- Tertiary ---
        control = mla_ui_button();
        mla_ui_button_set_text(control, mla_string_const("Tertiary"));
        mla_ui_button_set_style(control, MLA_UI_BUTTON_STYLE_TERTIARY);
        control.layout.x = 440;
        control.layout.y = 120;
        control.layout.width = 120;
        control.layout.height = 32;
        mla_array_list_add(root, control);

        control = mla_ui_button();
        mla_ui_button_set_text(control, mla_string_const("Tertiary Dis"));
        mla_ui_button_set_style(control, MLA_UI_BUTTON_STYLE_TERTIARY);
        mla_ui_button_set_disable(control, true);
        control.layout.x = 580;
        control.layout.y = 120;
        control.layout.width = 120;
        control.layout.height = 32;
        mla_array_list_add(root, control);

        // --- Link ---
        control = mla_ui_button();
        mla_ui_button_set_text(control, mla_string_const("Link Style"));
        mla_ui_button_set_style(control, MLA_UI_BUTTON_STYLE_LINK);
        control.layout.x = 440;
        control.layout.y = 170;
        control.layout.width = 120;
        control.layout.height = 32;
        mla_array_list_add(root, control);

        control = mla_ui_button();
        mla_ui_button_set_text(control, mla_string_const("Link Style Dis"));
        mla_ui_button_set_style(control, MLA_UI_BUTTON_STYLE_LINK);
        mla_ui_button_set_disable(control, true);
        control.layout.x = 580;
        control.layout.y = 170;
        control.layout.width = 120;
        control.layout.height = 32;
        mla_array_list_add(root, control);

        // --- Text Edit (Standard) ---
        control = mla_ui_text_edit();
        mla_ui_text_edit_set_text(control, mla_string_const("Standard"));
        mla_ui_text_edit_set_style(control, MLA_UI_TEXT_EDIT_STYLE_STANDARD);
        control.layout.x = 440;
        control.layout.y = 220;
        control.layout.width = 120;
        control.layout.height = 32;
        mla_array_list_add(root, control);

        control = mla_ui_text_edit();
        mla_ui_text_edit_set_text(control, mla_string_const("Standard Dis"));
        mla_ui_text_edit_set_style(control, MLA_UI_TEXT_EDIT_STYLE_STANDARD);
        mla_ui_button_set_disable(control, true);
        control.layout.x = 580;
        control.layout.y = 220;
        control.layout.width = 120;
        control.layout.height = 32;
        mla_array_list_add(root, control);

        // --- Text Edit (Password) ---
        control = mla_ui_text_edit();
        mla_ui_text_edit_set_text(control, mla_string_const("Password"));
        mla_ui_text_edit_set_style(control, MLA_UI_TEXT_EDIT_STYLE_PASSWORD);
        control.layout.x = 440;
        control.layout.y = 270;
        control.layout.width = 120;
        control.layout.height = 32;
        mla_array_list_add(root, control);

        control = mla_ui_text_edit();
        mla_ui_text_edit_set_text(control, mla_string_const("Password Dis"));
        mla_ui_text_edit_set_style(control, MLA_UI_TEXT_EDIT_STYLE_PASSWORD);
        mla_ui_button_set_disable(control, true);
        control.layout.x = 580;
        control.layout.y = 270;
        control.layout.width = 120;
        control.layout.height = 32;
        mla_array_list_add(root, control);

        control = mla_ui_text_edit();
        mla_ui_text_edit_set_text(control, mla_string_const("Edit Selection"));
        mla_ui_text_edit_set_style(control, MLA_UI_TEXT_EDIT_STYLE_STANDARD);
        mla_ui_text_edit_set_selected_text(control, mla_string_const("Selection"));
        control.layout.x = 440;
        control.layout.y = 320;
        control.layout.width = 120;
        control.layout.height = 32;
        mla_array_list_add(root, control);

        (void)root;
        (void)surfaceSize;
        (void)input_states;
        return true;

    }

};

struct main_app_main_window_performance_t {

    static mla_bool_t renderer(mla_array_list_t<mla_init_struct(mla_ui_control_t)> &root, const mla_ui_surface_size_t& surfaceSize, const mla_ui_surface_input_states_t &input_states) {

        (void)input_states;

        if (mla_array_list_size(root) > 0) {
            return true;
        }

        // Create multiple loading indicator near to each other starting from the top left corner each has witdh 50 and height 10 no spacing between them
        // We create as much loading indicators as possible until we reach the end of the surface width (surfaceSize.width) and then we move to the next line and repeat the process until we reach the end of the surface height (surfaceSize.height)

        // Loading indicator dimensions
        const mla_uint32_t indicatorWidth = 50;
        const mla_uint32_t indicatorHeight = 50;

        // Calculate how many indicators fit horizontally and vertically
        const mla_uint32_t indicatorsPerRow = surfaceSize.width / indicatorWidth;
        const mla_uint32_t indicatorsPerColumn = surfaceSize.height / indicatorHeight;

        // Create loading indicators in a grid pattern
        for (mla_uint32_t row = 0; row < indicatorsPerColumn; row++) {
            for (mla_uint32_t col = 0; col < indicatorsPerRow; col++) {
                mla_ui_control_t control = mla_ui_loading_indicator();

                control.layout.x = col * indicatorWidth;
                control.layout.y = row * indicatorHeight;
                control.layout.width = indicatorWidth;
                control.layout.height = indicatorHeight;

                mla_array_list_add(root, control);
            }
        }

        return true;
    }

};

#endif