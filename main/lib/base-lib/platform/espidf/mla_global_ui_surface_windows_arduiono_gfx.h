//
// Created by chris on 2/3/2026.
//

#ifndef MLA_GLOBAL_UI_SURFACE_ARDUINO_GFX_H
#define MLA_GLOBAL_UI_SURFACE_ARDUINO_GFX_H

#include "../../core/ui/surfaces/mla_ui_surface.h"
#include "../../core/ui/display/mla_ui_display_surface.h"
#include <Arduino_GFX_Library.h>

// ESP32-4848S040 ST7701 Display Configuration
#define TFT_WIDTH  480
#define TFT_HEIGHT 480

// ST7701 RGB Panel pins for ESP32-4848S040
#define TFT_DE   2
#define TFT_VSYNC 42
#define TFT_HSYNC 41
#define TFT_PCLK  1
#define TFT_R0   11
#define TFT_R1   12
#define TFT_R2   13
#define TFT_R3   14
#define TFT_R4   0
#define TFT_G0   8
#define TFT_G1   20
#define TFT_G2   3
#define TFT_G3   46
#define TFT_G4   9
#define TFT_G5   10
#define TFT_B0   4
#define TFT_B1   5
#define TFT_B2   6
#define TFT_B3   7
#define TFT_B4   15

// Backlight pin
#define TFT_BL   38

// Touch I2C pins (GT911)
#define TOUCH_SDA 19
#define TOUCH_SCL 45
#define TOUCH_INT 21
#define TOUCH_RST 17

#define mla_arduino_gfx_font_cache_size 8

// Global display instance
static Arduino_ESP32RGBPanel *g_rgbPanel = nullptr;
static Arduino_RGB_Display *g_gfx = nullptr;
static mla_bool_t g_gfx_initialized = false;

// Font cache item
struct mla_arduino_gfx_font_cache_item {
    mla_ui_surface_font_type_t font_type;
    const GFXfont *gfx_font;
    mla_uint8_t font_size_index;

    static mla_arduino_gfx_font_cache_item init() {
        return {
            mla_ui_surface_font_type_empty(),
            nullptr,
            0
        };
    }
};

// Render cache structure
struct mla_arduino_gfx_render_cache {
    mla_array_list_t<mla_init_struct(mla_arduino_gfx_font_cache_item)> fontCache;
    mla_uint16_t currentFillColor;
    mla_uint16_t currentStrokeColor;
};

mla_arduino_gfx_render_cache mla_private_arduino_gfx_cache_empty() {
    return {
        mla_array_list_empty<mla_init_struct(mla_arduino_gfx_font_cache_item)>(),
        0x0000,
        0x0000
    };
}

// Touch state structure
struct mla_arduino_gfx_touch_state {
    mla_bool_t touched;
    mla_int32_t x;
    mla_int32_t y;
    mla_bool_t last_touched;
};

struct mla_arduino_gfx_surface_t {
    mla_bool_t is_initialized;
    mla_ui_surface_size_t size;
    mla_arduino_gfx_render_cache renderCache;
    mla_arduino_gfx_touch_state touchState;
};

// Convert RGBA to RGB565
static inline mla_uint16_t mla_private_arduino_gfx_color_to_rgb565(const mla_ui_surface_draw_command_color_t &color) {
    return ((color.r & 0xF8) << 8) | ((color.g & 0xFC) << 3) | (color.b >> 3);
}

// Initialize the ST7701 display
mla_bool_t mla_private_arduino_gfx_init_display() {
    if (g_gfx_initialized) {
        return true;
    }

    // Configure backlight
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    // Create RGB Panel bus - using new as required by Arduino_GFX library
    g_rgbPanel = new Arduino_ESP32RGBPanel(
        TFT_DE, TFT_VSYNC, TFT_HSYNC, TFT_PCLK,
        TFT_R0, TFT_R1, TFT_R2, TFT_R3, TFT_R4,
        TFT_G0, TFT_G1, TFT_G2, TFT_G3, TFT_G4, TFT_G5,
        TFT_B0, TFT_B1, TFT_B2, TFT_B3, TFT_B4,
        1,      // hsync_polarity
        46,     // hsync_front_porch
        2,      // hsync_pulse_width
        44,     // hsync_back_porch
        1,      // vsync_polarity
        50,     // vsync_front_porch
        16,     // vsync_pulse_width
        16      // vsync_back_porch
    );

    if (g_rgbPanel == nullptr) {
        return false;
    }

    // Create display instance - using new as required by Arduino_GFX library
    g_gfx = new Arduino_RGB_Display(
        TFT_WIDTH, TFT_HEIGHT,
        g_rgbPanel,
        0,      // rotation
        true,   // auto_flush
        nullptr, // bus for init (ST7701 uses RGB directly)
        GFX_NOT_DEFINED, // rst
        nullptr, // init operations
        0        // init operations length
    );

    if (g_gfx == nullptr) {
        delete g_rgbPanel;
        g_rgbPanel = nullptr;
        return false;
    }

    if (!g_gfx->begin()) {
        delete g_gfx;
        delete g_rgbPanel;
        g_gfx = nullptr;
        g_rgbPanel = nullptr;
        return false;
    }

    g_gfx->fillScreen(WHITE);
    g_gfx_initialized = true;

    return true;
}

// Get approximate GFX font based on size
static const GFXfont* mla_private_arduino_gfx_get_font_for_size(mla_double_t size) {
    // Arduino_GFX uses built-in fonts with fixed sizes
    // Map requested sizes to available fonts
    if (size <= 9) {
        return nullptr; // Default 6x8 font
    } else if (size <= 12) {
        return &FreeSans9pt7b;
    } else if (size <= 18) {
        return &FreeSans12pt7b;
    } else if (size <= 24) {
        return &FreeSans18pt7b;
    } else {
        return &FreeSans24pt7b;
    }
}

const GFXfont* mla_private_arduino_gfx_cache_getOrCreateFont(
    mla_arduino_gfx_render_cache &cache, const mla_ui_surface_font_type_t &fontType) {

    if (mla_string_is_empty(fontType.family)) {
        return nullptr;
    }

    // Check if font already exists in cache
    for (mla_size_t i = 0; i < mla_array_list_size(cache.fontCache); i++) {
        const mla_arduino_gfx_font_cache_item &item = mla_array_list_get_unsafe(cache.fontCache, i);

        if (mla_ui_surface_font_type_equals(item.font_type, fontType)) {
            return item.gfx_font;
        }
    }

    // Create new font mapping
    const GFXfont *gfxFont = mla_private_arduino_gfx_get_font_for_size(fontType.size);

    mla_arduino_gfx_font_cache_item newItem = {
        fontType,
        gfxFont,
        0
    };

    if (mla_array_list_size(cache.fontCache) < mla_arduino_gfx_font_cache_size) {
        mla_array_list_add(cache.fontCache, newItem);
    } else {
        mla_arduino_gfx_font_cache_item &oldItem = mla_array_list_get_unsafe(cache.fontCache, 0);
        oldItem = newItem;
    }

    return gfxFont;
}

// Surface function implementations
mla_ui_surface_size_t mla_private_arduino_gfx_surface_get_size(const mla_ui_surface_t &surface) {
    mla_ui_surface_size_t size = {0, 0};

    mla_arduino_gfx_surface_t *gfx_surface = mla_s_cast<mla_arduino_gfx_surface_t *>(surface.resource);

    if (gfx_surface == nullptr) {
        return size;
    }

    if (!gfx_surface->is_initialized) {
        return gfx_surface->size;
    }

    size.width = TFT_WIDTH;
    size.height = TFT_HEIGHT;

    return size;
}

mla_bool_t mla_private_arduino_gfx_surface_set_size(const mla_ui_surface_t &surface, mla_ui_surface_size_t size) {
    mla_arduino_gfx_surface_t *gfx_surface = mla_s_cast<mla_arduino_gfx_surface_t *>(surface.resource);

    if (gfx_surface == nullptr) {
        return false;
    }

    // For fixed display, just store the requested size
    gfx_surface->size = size;
    return true;
}

mla_ui_surface_input_states_t mla_private_arduino_gfx_surface_input_states(const mla_ui_surface_t &surface) {
    mla_ui_surface_input_states_t inputStates = mla_ui_surface_input_states_empty();

    mla_arduino_gfx_surface_t *gfx_surface = mla_s_cast<mla_arduino_gfx_surface_t *>(surface.resource);

    if (gfx_surface == nullptr || !gfx_surface->is_initialized) {
        return inputStates;
    }

    // Touch handling would require GT911 touch controller
    // For now, return current touch state
    if (gfx_surface->touchState.touched) {
        inputStates.cursorPosition.x = (mla_double_t)gfx_surface->touchState.x;
        inputStates.cursorPosition.y = (mla_double_t)gfx_surface->touchState.y;
        inputStates.leftMouseButtonDown = true;
    }

    return inputStates;
}

mla_ui_surface_draw_size_t mla_private_arduino_gfx_surface_calc_text_size(const mla_ui_surface_t &surface,
                                                                 const mla_ui_surface_font_type_t &font_type,
                                                                 const mla_string_t &text) {
    mla_ui_surface_draw_size_t size = {0, 0};

    if (mla_string_is_empty(text) || g_gfx == nullptr) {
        return size;
    }

    mla_arduino_gfx_surface_t *gfx_surface = mla_s_cast<mla_arduino_gfx_surface_t *>(surface.resource);

    if (gfx_surface == nullptr) {
        return size;
    }

    const GFXfont *font = mla_private_arduino_gfx_cache_getOrCreateFont(gfx_surface->renderCache, font_type);
    g_gfx->setFont(font);

    int16_t x1, y1;
    uint16_t w, h;

    // Convert mla_string to null-terminated string
    char *textStr = mla_string_to_cstr(text);
    if (textStr == nullptr) {
        return size;
    }

    g_gfx->getTextBounds(textStr, 0, 0, &x1, &y1, &w, &h);

    mla_platform_free(textStr);

    size.width = (mla_double_t)w;
    size.height = (mla_double_t)h;

    return size;
}

// Draw a filled rounded rectangle
static void mla_private_arduino_gfx_fill_rounded_rect(mla_int32_t x, mla_int32_t y,
                                             mla_int32_t w, mla_int32_t h,
                                             mla_int32_t rx, mla_int32_t ry,
                                             mla_uint16_t color) {
    if (rx == 0 && ry == 0) {
        g_gfx->fillRect(x, y, w, h, color);
    } else {
        mla_int32_t r = (rx > ry) ? rx : ry;
        g_gfx->fillRoundRect(x, y, w, h, r, color);
    }
}

// Draw a rounded rectangle outline
static void mla_private_arduino_gfx_draw_rounded_rect(mla_int32_t x, mla_int32_t y,
                                             mla_int32_t w, mla_int32_t h,
                                             mla_int32_t rx, mla_int32_t ry,
                                             mla_uint16_t color) {
    if (rx == 0 && ry == 0) {
        g_gfx->drawRect(x, y, w, h, color);
    } else {
        mla_int32_t r = (rx > ry) ? rx : ry;
        g_gfx->drawRoundRect(x, y, w, h, r, color);
    }
}

mla_bool_t mla_private_arduino_gfx_surface_render_draw_commands(const mla_ui_surface_t &surface,
                                                       const mla_array_list_t<mla_init_struct(mla_ui_surface_draw_command_t)> &drawCommands,
                                                       mla_array_list_t<mla_init_struct(mla_ui_surface_input_event_t)> &eventsSinceLastFrame) {
    mla_arduino_gfx_surface_t *gfx_surface = mla_s_cast<mla_arduino_gfx_surface_t *>(surface.resource);

    if (gfx_surface == nullptr) {
        return false;
    }

    if (!gfx_surface->is_initialized) {
        if (!mla_private_arduino_gfx_init_display()) {
            return false;
        }
        gfx_surface->is_initialized = true;
    }

    if (g_gfx == nullptr) {
        return false;
    }

    // Handle touch events (GT911 touch controller)
    // This is a simplified implementation - actual GT911 reading would go here
    mla_bool_t currentlyTouched = gfx_surface->touchState.touched;

    if (gfx_surface->touchState.last_touched && !currentlyTouched) {
        // Touch released - generate click event
        mla_ui_surface_input_event_t clickEvent = mla_ui_surface_input_event_empty();
        clickEvent.kind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CLICK;
        clickEvent.click.position.x = (mla_double_t)gfx_surface->touchState.x;
        clickEvent.click.position.y = (mla_double_t)gfx_surface->touchState.y;
        clickEvent.click.button = MLA_UI_SURFACE_INPUT_EVENT_CLICK_BUTTON_LEFT;
        mla_array_list_add(eventsSinceLastFrame, clickEvent);
    }
    gfx_surface->touchState.last_touched = currentlyTouched;

    // Clear screen with white background
    g_gfx->fillScreen(WHITE);

    // Track current path point for path commands
    mla_int32_t pathX = 0;
    mla_int32_t pathY = 0;

    for (mla_size_t i = 0; i < mla_array_list_size(drawCommands); i++) {
        const mla_ui_surface_draw_command_t &cmd = mla_array_list_get_unsafe(drawCommands, i);

        switch (cmd.kind) {
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT: {
                const auto &rect = cmd.rect;
                mla_uint16_t fillColor = mla_private_arduino_gfx_color_to_rgb565(rect.color);

                mla_int32_t x = (mla_int32_t)rect.x;
                mla_int32_t y = (mla_int32_t)rect.y;
                mla_int32_t w = (mla_int32_t)rect.width;
                mla_int32_t h = (mla_int32_t)rect.height;
                mla_int32_t rx = (mla_int32_t)rect.rx;
                mla_int32_t ry = (mla_int32_t)rect.ry;

                // Fill
                mla_private_arduino_gfx_fill_rounded_rect(x, y, w, h, rx, ry, fillColor);

                // Stroke
                if (rect.stroke_width > 0) {
                    mla_uint16_t strokeColor = mla_private_arduino_gfx_color_to_rgb565(rect.stroke);
                    mla_private_arduino_gfx_draw_rounded_rect(x, y, w, h, rx, ry, strokeColor);
                }
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_CIRCLE: {
                const auto &circle = cmd.circle;
                mla_uint16_t fillColor = mla_private_arduino_gfx_color_to_rgb565(circle.fill);

                mla_int32_t cx = (mla_int32_t)circle.cx;
                mla_int32_t cy = (mla_int32_t)circle.cy;
                mla_int32_t r = (mla_int32_t)circle.r;

                g_gfx->fillCircle(cx, cy, r, fillColor);

                if (circle.stroke_width > 0) {
                    mla_uint16_t strokeColor = mla_private_arduino_gfx_color_to_rgb565(circle.stroke);
                    g_gfx->drawCircle(cx, cy, r, strokeColor);
                }
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_ELLIPSE: {
                const auto &ellipse = cmd.ellipse;
                mla_uint16_t fillColor = mla_private_arduino_gfx_color_to_rgb565(ellipse.fill);

                mla_int32_t cx = (mla_int32_t)ellipse.cx;
                mla_int32_t cy = (mla_int32_t)ellipse.cy;
                mla_int32_t rx = (mla_int32_t)ellipse.rx;
                mla_int32_t ry = (mla_int32_t)ellipse.ry;

                g_gfx->fillEllipse(cx, cy, rx, ry, fillColor);

                if (ellipse.stroke_width > 0) {
                    mla_uint16_t strokeColor = mla_private_arduino_gfx_color_to_rgb565(ellipse.stroke);
                    g_gfx->drawEllipse(cx, cy, rx, ry, strokeColor);
                }
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINE: {
                const auto &line = cmd.line;
                mla_uint16_t strokeColor = mla_private_arduino_gfx_color_to_rgb565(line.stroke);

                g_gfx->drawLine(
                    (mla_int32_t)line.x1, (mla_int32_t)line.y1,
                    (mla_int32_t)line.x2, (mla_int32_t)line.y2,
                    strokeColor
                );
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_POLYLINE: {
                const auto &polyline = cmd.polyline;
                if (mla_array_list_size(polyline.points) < 2) break;

                mla_uint16_t strokeColor = mla_private_arduino_gfx_color_to_rgb565(polyline.stroke);

                for (mla_size_t j = 0; j < mla_array_list_size(polyline.points) - 1; j++) {
                    const mla_ui_surface_draw_point_t &p1 = mla_array_list_get_unsafe(polyline.points, j);
                    const mla_ui_surface_draw_point_t &p2 = mla_array_list_get_unsafe(polyline.points, j + 1);

                    g_gfx->drawLine(
                        (mla_int32_t)p1.x, (mla_int32_t)p1.y,
                        (mla_int32_t)p2.x, (mla_int32_t)p2.y,
                        strokeColor
                    );
                }
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_POLYGON: {
                const auto &polygon = cmd.polygon;
                if (mla_array_list_size(polygon.points) < 3) break;

                mla_uint16_t fillColor = mla_private_arduino_gfx_color_to_rgb565(polygon.fill);
                mla_uint16_t strokeColor = mla_private_arduino_gfx_color_to_rgb565(polygon.stroke);

                // For triangles, use fillTriangle
                if (mla_array_list_size(polygon.points) == 3) {
                    const mla_ui_surface_draw_point_t &p0 = mla_array_list_get_unsafe(polygon.points, 0);
                    const mla_ui_surface_draw_point_t &p1 = mla_array_list_get_unsafe(polygon.points, 1);
                    const mla_ui_surface_draw_point_t &p2 = mla_array_list_get_unsafe(polygon.points, 2);

                    g_gfx->fillTriangle(
                        (mla_int32_t)p0.x, (mla_int32_t)p0.y,
                        (mla_int32_t)p1.x, (mla_int32_t)p1.y,
                        (mla_int32_t)p2.x, (mla_int32_t)p2.y,
                        fillColor
                    );

                    if (polygon.stroke_width > 0) {
                        g_gfx->drawTriangle(
                            (mla_int32_t)p0.x, (mla_int32_t)p0.y,
                            (mla_int32_t)p1.x, (mla_int32_t)p1.y,
                            (mla_int32_t)p2.x, (mla_int32_t)p2.y,
                            strokeColor
                        );
                    }
                } else {
                    // For other polygons, draw outline only (fill is complex)
                    for (mla_size_t j = 0; j < mla_array_list_size(polygon.points); j++) {
                        const mla_ui_surface_draw_point_t &p1 = mla_array_list_get_unsafe(polygon.points, j);
                        const mla_ui_surface_draw_point_t &p2 = mla_array_list_get_unsafe(polygon.points,
                            (j + 1) % mla_array_list_size(polygon.points));

                        g_gfx->drawLine(
                            (mla_int32_t)p1.x, (mla_int32_t)p1.y,
                            (mla_int32_t)p2.x, (mla_int32_t)p2.y,
                            strokeColor
                        );
                    }
                }
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_PATH: {
                const auto &path = cmd.path;
                if (mla_array_list_size(path.commands) == 0) break;

                mla_uint16_t strokeColor = mla_private_arduino_gfx_color_to_rgb565(path.stroke);

                for (mla_size_t j = 0; j < mla_array_list_size(path.commands); j++) {
                    const auto &pathCmd = mla_array_list_get_unsafe(path.commands, j);

                    switch (pathCmd.kind) {
                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_MOVE_TO:
                            pathX = (mla_int32_t)pathCmd.move_to.x;
                            pathY = (mla_int32_t)pathCmd.move_to.y;
                            break;

                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_LINE_TO: {
                            mla_int32_t newX = (mla_int32_t)pathCmd.line_to.x;
                            mla_int32_t newY = (mla_int32_t)pathCmd.line_to.y;
                            g_gfx->drawLine(pathX, pathY, newX, newY, strokeColor);
                            pathX = newX;
                            pathY = newY;
                            break;
                        }

                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_QUADRATIC_CURVE_TO: {
                            // Approximate quadratic curve with line segments
                            mla_int32_t cpx = (mla_int32_t)pathCmd.quadratic_curve_to.cpx;
                            mla_int32_t cpy = (mla_int32_t)pathCmd.quadratic_curve_to.cpy;
                            mla_int32_t endX = (mla_int32_t)pathCmd.quadratic_curve_to.x;
                            mla_int32_t endY = (mla_int32_t)pathCmd.quadratic_curve_to.y;

                            const int segments = 8;
                            mla_int32_t prevX = pathX;
                            mla_int32_t prevY = pathY;

                            for (int s = 1; s <= segments; s++) {
                                float t = (float)s / segments;
                                float mt = 1.0f - t;

                                mla_int32_t x = (mla_int32_t)(mt * mt * pathX + 2 * mt * t * cpx + t * t * endX);
                                mla_int32_t y = (mla_int32_t)(mt * mt * pathY + 2 * mt * t * cpy + t * t * endY);

                                g_gfx->drawLine(prevX, prevY, x, y, strokeColor);
                                prevX = x;
                                prevY = y;
                            }

                            pathX = endX;
                            pathY = endY;
                            break;
                        }

                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_CUBIC_CURVE_TO: {
                            // Approximate cubic curve with line segments
                            mla_int32_t cp1x = (mla_int32_t)pathCmd.cubic_curve_to.cp1x;
                            mla_int32_t cp1y = (mla_int32_t)pathCmd.cubic_curve_to.cp1y;
                            mla_int32_t cp2x = (mla_int32_t)pathCmd.cubic_curve_to.cp2x;
                            mla_int32_t cp2y = (mla_int32_t)pathCmd.cubic_curve_to.cp2y;
                            mla_int32_t endX = (mla_int32_t)pathCmd.cubic_curve_to.x;
                            mla_int32_t endY = (mla_int32_t)pathCmd.cubic_curve_to.y;

                            const int segments = 12;
                            mla_int32_t prevX = pathX;
                            mla_int32_t prevY = pathY;

                            for (int s = 1; s <= segments; s++) {
                                float t = (float)s / segments;
                                float mt = 1.0f - t;
                                float mt2 = mt * mt;
                                float mt3 = mt2 * mt;
                                float t2 = t * t;
                                float t3 = t2 * t;

                                mla_int32_t x = (mla_int32_t)(mt3 * pathX + 3 * mt2 * t * cp1x +
                                                              3 * mt * t2 * cp2x + t3 * endX);
                                mla_int32_t y = (mla_int32_t)(mt3 * pathY + 3 * mt2 * t * cp1y +
                                                              3 * mt * t2 * cp2y + t3 * endY);

                                g_gfx->drawLine(prevX, prevY, x, y, strokeColor);
                                prevX = x;
                                prevY = y;
                            }

                            pathX = endX;
                            pathY = endY;
                            break;
                        }

                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_ARC_TO:
                            // Simplified arc - just draw line to endpoint
                            g_gfx->drawLine(pathX, pathY,
                                           (mla_int32_t)pathCmd.arc_to.x,
                                           (mla_int32_t)pathCmd.arc_to.y,
                                           strokeColor);
                            pathX = (mla_int32_t)pathCmd.arc_to.x;
                            pathY = (mla_int32_t)pathCmd.arc_to.y;
                            break;

                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_CLOSE_PATH:
                            // Path closing handled by the path structure
                            break;
                    }
                }
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT: {
                if (mla_string_is_empty(cmd.text.content)) break;

                const GFXfont *font = mla_private_arduino_gfx_cache_getOrCreateFont(
                    gfx_surface->renderCache, cmd.text.font_type);

                g_gfx->setFont(font);
                g_gfx->setTextColor(mla_private_arduino_gfx_color_to_rgb565(cmd.text.fill));

                char *textStr = mla_string_to_cstr(cmd.text.content);
                if (textStr != nullptr) {
                    // Adjust Y position for baseline
                    mla_int32_t y = (mla_int32_t)cmd.text.y;
                    if (font != nullptr) {
                        y += font->yAdvance;
                    } else {
                        y += 8; // Default font height
                    }

                    g_gfx->setCursor((mla_int32_t)cmd.text.x, y);
                    g_gfx->print(textStr);
                    mla_platform_free(textStr);
                }
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINEAR_GRADIENT:
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_RADIAL_GRADIENT:
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_STOP:
                // Gradients not directly supported in Arduino_GFX
                // Would require custom implementation
                break;

            default:
                break;
        }
    }

    return true;
}

mla_buffer_cleanup_mode mla_private_arduino_gfx_surface_buffer_cleanup(mla_platform_pointer_t data, mla_callback_userdata userData) {
    (void)userData;

    mla_arduino_gfx_surface_t *gfx_surface = mla_s_cast<mla_arduino_gfx_surface_t *>(data);

    if (gfx_surface != nullptr) {
        gfx_surface->renderCache = mla_private_arduino_gfx_cache_empty();
    }

    return CLEAN_UP_NEEDED;
}

mla_bool_t mla_private_arduino_gfx_create_surface(mla_ui_surface_t &outSurface) {
    mla_arduino_gfx_surface_t *gfx_surface = mla_s_cast<mla_arduino_gfx_surface_t *>(
        mla_platform_malloc(sizeof(mla_arduino_gfx_surface_t)));

    if (gfx_surface == nullptr) {
        return false;
    }

    mla_memset(gfx_surface, 0, sizeof(mla_arduino_gfx_surface_t));
    gfx_surface->is_initialized = false;
    gfx_surface->size = {TFT_WIDTH, TFT_HEIGHT};
    gfx_surface->renderCache = mla_private_arduino_gfx_cache_empty();
    gfx_surface->touchState = {false, 0, 0, false};

    outSurface.resource = gfx_surface;
    outSurface.resourceOwner = mla_buffer_reference(gfx_surface, true, mla_private_arduino_gfx_surface_buffer_cleanup);
    outSurface.get_size = mla_private_arduino_gfx_surface_get_size;
    outSurface.set_size = mla_private_arduino_gfx_surface_set_size;
    outSurface.render_draw_commands = mla_private_arduino_gfx_surface_render_draw_commands;
    outSurface.calc_text_size = mla_private_arduino_gfx_surface_calc_text_size;
    outSurface.get_input_states = mla_private_arduino_gfx_surface_input_states;

    return true;
}

mla_ui_display_surface_low_level_access_t g_ui_display_surface_low_level_access = {
    mla_private_arduino_gfx_create_surface
};

// Shutdown function
void mla_private_arduino_gfx_shutdown() {
    if (g_gfx != nullptr) {
        delete g_gfx;
        g_gfx = nullptr;
    }if (g_rgbPanel != nullptr) {
        delete g_rgbPanel;
        g_rgbPanel = nullptr;
    }
    g_gfx_initialized = false;
}

#endif
