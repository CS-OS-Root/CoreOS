//
// Created by chris on 2/6/2026.
//

#include "mla_ui_loading_indicator.h"
#include "mla_ui_style.h"
#include "../../utils/mla_math_utils.h"

mla_bool_t mla_private_ui_indicator_render_to_drawCommands(const mla_ui_control_context_t &context, const mla_ui_control_t &element, mla_array_list_t<mla_init_struct(mla_ui_surface_draw_command_t)>& drawCommands, mla_array_list_t<mla_init_struct(mla_ui_control_input_area_t)> &inputAreas) {
    (void)inputAreas;

    // Calculate absolute position based on parent context offset and local element layout
    mla_double_t offX = context.offsetX + element.layout.x;
    mla_double_t offY = context.offsetY + element.layout.y;
    mla_double_t w = element.layout.width;
    mla_double_t h = element.layout.height;

    // Determine scale based on 24x24 reference SVG
    // We maintain aspect ratio (circle), so use min dimension
    mla_double_t minDim = (w < h) ? w : h;
    if (minDim <= 0.0) {
        return true; // Don't draw if size is effectively zero
    }

    mla_double_t scale = minDim / 24.0;

    // Center of the element
    mla_double_t cx = offX + (w / 2.0);
    mla_double_t cy = offY + (h / 2.0);

    // Scaled parameters
    mla_double_t r = 9.0 * scale;
    mla_double_t strokeWidth = 2.0 * scale;

    // 1. Background circle
    // <circle cx="12" cy="12" r="9" fill="none" stroke="#d0d0d0" stroke-width="2"/>
    mla_ui_surface_draw_command_t bgCircle = mla_ui_surface_draw_command_empty();
    bgCircle.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_CIRCLE;
    bgCircle.circle.cx = cx;
    bgCircle.circle.cy = cy;
    bgCircle.circle.r = r;
    bgCircle.circle.fill = {0, 0, 0, 0}; // Transparent
    bgCircle.circle.stroke = {208, 208, 208, 255}; // #d0d0d0
    bgCircle.circle.stroke_width = strokeWidth;

    mla_array_list_add(drawCommands, bgCircle);

    // 2. Active segment
    // Determine rotation based on system time (syncs all indicators)
    mla_uint64_t timeMs = mla_system_time_ms();
    mla_double_t periodMs = 1000.0;

    // Calculate phase (0.0 to 1.0)
    mla_double_t phase = mla_s_cast<mla_double_t>(timeMs % mla_s_cast<mla_uint64_t>(periodMs)) / periodMs;
    mla_double_t currentRotation = phase * 2.0 * mla_math_pi;

    // Original Arc: Starts at top (-PI/2) and goes to right (0)
    // We add the currentRotation to these base angles
    mla_double_t startAngle = (-mla_math_pi / 2.0) + currentRotation;
    mla_double_t endAngle = 0.0 + currentRotation;

    // Calculate coordinates for the path
    // Note: Y is down in UI coords, so standard cartesian math works for clockwise rotation if we consider Y-down
    mla_double_t sx = cx + (r * mla_math_cos(startAngle));
    mla_double_t sy = cy + (r * mla_math_sin(startAngle));

    mla_double_t ex = cx + (r * mla_math_cos(endAngle));
    mla_double_t ey = cy + (r * mla_math_sin(endAngle));

    mla_ui_surface_draw_command_t activeSegment = mla_ui_surface_draw_command_empty();
    activeSegment.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_PATH;
    activeSegment.path = mla_ui_surface_draw_command_path_t::init();
    activeSegment.path.fill = {0, 0, 0, 0};
    activeSegment.path.stroke = MLA_UI_COLOR_TEXT_SUCCESS; // #27ae60
    activeSegment.path.stroke_width = strokeWidth;

    // Move To Start Point
    mla_ui_surface_draw_path_command_t moveCmd = {};
    moveCmd.kind = MLA_UI_SURFACE_DRAW_PATH_COMMAND_MOVE_TO;
    moveCmd.move_to.x = sx;
    moveCmd.move_to.y = sy;
    mla_array_list_add(activeSegment.path.commands, moveCmd);

    // Arc To End Point
    mla_ui_surface_draw_path_command_t arcCmd = {};
    arcCmd.kind = MLA_UI_SURFACE_DRAW_PATH_COMMAND_ARC_TO;
    arcCmd.arc_to.rx = r;
    arcCmd.arc_to.ry = r;
    arcCmd.arc_to.x_axis_rotation = 0.0;
    arcCmd.arc_to.large_arc_flag = false; // Segment is 90 degrees (< 180)
    arcCmd.arc_to.sweep_flag = true;      // Clockwise
    arcCmd.arc_to.x = ex;
    arcCmd.arc_to.y = ey;
    mla_array_list_add(activeSegment.path.commands, arcCmd);

    mla_array_list_add(drawCommands, activeSegment);

    return true;
}

mla_ui_control_t mla_ui_loading_indicator() {
    mla_ui_control_t indicator = mla_ui_control();
    indicator.renderToDrawCommands = mla_private_ui_indicator_render_to_drawCommands;
    return indicator;
}
