#ifndef MLA_GLOBAL_UI_SURFACE_WINDOWS_GDIPLUS_H
#define MLA_GLOBAL_UI_SURFACE_WINDOWS_GDIPLUS_H

#include "../../core/ui/surfaces/mla_ui_surface.h"
#include "../../core/ui/display/mla_ui_display_surface.h"
#include <windows.h>
#include <gdiplus.h>

static ULONG_PTR g_mla_windows_gdiplus_token = 0;

enum mla_windows_gdiplus_gradient_kind_t : mla_uint8_t {
    MLA_WINDOWS_GDIPLUS_GRADIENT_KIND_NONE,
    MLA_WINDOWS_GDIPLUS_GRADIENT_KIND_LINEAR,
    MLA_WINDOWS_GDIPLUS_GRADIENT_KIND_RADIAL
};

struct mla_windows_gdiplus_gradient_state_t {
    mla_windows_gdiplus_gradient_kind_t kind;
    mla_ui_surface_draw_command_linear_gradient_t linear;
    mla_ui_surface_draw_command_radial_gradient_t radial;
    Gdiplus::Color stop0;
    Gdiplus::Color stop1;
    mla_uint8_t stopCount;
};

mla_windows_gdiplus_gradient_state_t mla_private_windows_gdiplus_gradient_state_empty() {
    mla_windows_gdiplus_gradient_state_t state = {};
    state.kind = MLA_WINDOWS_GDIPLUS_GRADIENT_KIND_NONE;
    state.stop0 = Gdiplus::Color(255, 0, 0, 0);
    state.stop1 = Gdiplus::Color(255, 255, 255, 255);
    state.stopCount = 0;
    return state;
}

struct mla_windows_window_surface_gdiplus_t {
    HWND hwnd;
    mla_bool_t is_initialized;
    mla_ui_surface_size_t default_size;

    HDC memoryDC;
    HBITMAP memoryBitmap;
    HBITMAP oldBitmap;
    mla_uint32_t backBufferWidth;
    mla_uint32_t backBufferHeight;

    mla_windows_gdiplus_gradient_state_t gradientState;

#ifdef mla_debug_build
    DWORD DEBUG_last_fps_time;
    int DEBUG_frames_accumulated;
    int DEBUG_current_fps;
#endif
};

void mla_private_windows_get_system_dpi(mla_double_t &outDpiX, mla_double_t &outDpiY) {
    outDpiX = 96.0;
    outDpiY = 96.0;

    HDC screenDc = GetDC(nullptr);
    if (screenDc != nullptr) {
        outDpiX = (mla_double_t)GetDeviceCaps(screenDc, LOGPIXELSX);
        outDpiY = (mla_double_t)GetDeviceCaps(screenDc, LOGPIXELSY);
        ReleaseDC(nullptr, screenDc);
    }
}

Gdiplus::Color mla_private_windows_gdiplus_convert_color(const mla_ui_surface_draw_command_color_t &color) {
    return Gdiplus::Color((BYTE)color.a, (BYTE)color.r, (BYTE)color.g, (BYTE)color.b);
}

void mla_private_windows_gdiplus_add_rounded_rect_path(Gdiplus::GraphicsPath &path, const mla_ui_surface_draw_command_rect_t &rect) {
    Gdiplus::REAL x = (Gdiplus::REAL)rect.x;
    Gdiplus::REAL y = (Gdiplus::REAL)rect.y;
    Gdiplus::REAL width = (Gdiplus::REAL)rect.width;
    Gdiplus::REAL height = (Gdiplus::REAL)rect.height;

    Gdiplus::REAL radiusX = (Gdiplus::REAL)rect.rx;
    Gdiplus::REAL radiusY = (Gdiplus::REAL)rect.ry;

    if (radiusX <= 0.0f || radiusY <= 0.0f) {
        path.AddRectangle(Gdiplus::RectF(x, y, width, height));
        return;
    }

    Gdiplus::REAL diameterX = radiusX * 2.0f;
    Gdiplus::REAL diameterY = radiusY * 2.0f;

    if (diameterX > width) diameterX = width;
    if (diameterY > height) diameterY = height;

    path.AddArc(x, y, diameterX, diameterY, 180.0f, 90.0f);
    path.AddArc(x + width - diameterX, y, diameterX, diameterY, 270.0f, 90.0f);
    path.AddArc(x + width - diameterX, y + height - diameterY, diameterX, diameterY, 0.0f, 90.0f);
    path.AddArc(x, y + height - diameterY, diameterX, diameterY, 90.0f, 90.0f);
    path.CloseFigure();
}

mla_bool_t mla_private_windows_gdiplus_ensure_backbuffer(mla_windows_window_surface_gdiplus_t *surface) {
    if (surface == nullptr || !IsWindow(surface->hwnd)) {
        return false;
    }

    RECT rc;
    if (!GetClientRect(surface->hwnd, &rc)) {
        return false;
    }

    mla_uint32_t width = (mla_uint32_t)(rc.right - rc.left);
    mla_uint32_t height = (mla_uint32_t)(rc.bottom - rc.top);

    if (width == 0 || height == 0) {
        return false;
    }

    if (surface->memoryBitmap != nullptr &&
        width == surface->backBufferWidth &&
        height == surface->backBufferHeight) {
        return true;
    }

    HDC windowDC = GetDC(surface->hwnd);
    if (windowDC == nullptr) {
        return false;
    }

    if (surface->memoryDC == nullptr) {
        surface->memoryDC = CreateCompatibleDC(windowDC);
    }

    if (surface->memoryDC == nullptr) {
        ReleaseDC(surface->hwnd, windowDC);
        return false;
    }

    if (surface->memoryBitmap != nullptr) {
        SelectObject(surface->memoryDC, surface->oldBitmap);
        DeleteObject(surface->memoryBitmap);
        surface->memoryBitmap = nullptr;
    }

    surface->memoryBitmap = CreateCompatibleBitmap(windowDC, (int)width, (int)height);
    ReleaseDC(surface->hwnd, windowDC);

    if (surface->memoryBitmap == nullptr) {
        return false;
    }

    surface->oldBitmap = (HBITMAP)SelectObject(surface->memoryDC, surface->memoryBitmap);
    surface->backBufferWidth = width;
    surface->backBufferHeight = height;

    return true;
}

mla_ui_surface_size_t mla_private_windows_surface_gdiplus_get_size(const mla_ui_surface_t &surface) {
    mla_ui_surface_size_t size = {0, 0};
    mla_windows_window_surface_gdiplus_t *window_surface = mla_s_cast<mla_windows_window_surface_gdiplus_t *>(surface.resource);
    if (window_surface == nullptr) {
        return size;
    }
    if (!window_surface->is_initialized) {
        return window_surface->default_size;
    }
    RECT rect;
    if (IsWindow(window_surface->hwnd) && GetClientRect(window_surface->hwnd, &rect)) {
        mla_double_t dpiX;
        mla_double_t dpiY;
        mla_private_windows_get_system_dpi(dpiX, dpiY);
        size.width = (mla_uint32_t)((mla_double_t)(rect.right - rect.left) * (96.0 / dpiX));
        size.height = (mla_uint32_t)((mla_double_t)(rect.bottom - rect.top) * (96.0 / dpiY));
    }
    return size;
}

mla_bool_t mla_private_windows_surface_gdiplus_set_size(const mla_ui_surface_t &surface, mla_ui_surface_size_t size) {
    mla_windows_window_surface_gdiplus_t *window_surface = mla_s_cast<mla_windows_window_surface_gdiplus_t *>(surface.resource);
    if (window_surface == nullptr) {
        return false;
    }
    if (!window_surface->is_initialized) {
        window_surface->default_size = size;
        return true;
    }
    if (!IsWindow(window_surface->hwnd)) {
        return false;
    }

    mla_double_t dpiX;
    mla_double_t dpiY;
    mla_private_windows_get_system_dpi(dpiX, dpiY);
    mla_int32_t physicalWidth = (mla_int32_t)((mla_double_t)size.width * (dpiX / 96.0));
    mla_int32_t physicalHeight = (mla_int32_t)((mla_double_t)size.height * (dpiY / 96.0));
    return SetWindowPos(window_surface->hwnd, nullptr, 0, 0, physicalWidth, physicalHeight, SWP_NOMOVE | SWP_NOZORDER) != 0;
}

LRESULT CALLBACK mla_private_windows_surface_gdiplus_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

mla_bool_t mla_private_windows_create_windows_gdiplus_surface(mla_windows_window_surface_gdiplus_t *surface) {
    const char CLASS_NAME[] = "MLA_Window_Class_GDIPlus";
    WNDCLASSEXA wc = {};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = mla_private_windows_surface_gdiplus_proc;
    wc.hInstance = GetModuleHandleA(nullptr);
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursorA(nullptr, IDC_ARROW);
    RegisterClassExA(&wc);

    int width = surface->default_size.width == 0 ? CW_USEDEFAULT : (int)surface->default_size.width;
    int height = surface->default_size.height == 0 ? CW_USEDEFAULT : (int)surface->default_size.height;

    surface->hwnd = CreateWindowExA(0, CLASS_NAME, "MLA App", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height,
                                    nullptr, nullptr, GetModuleHandleA(nullptr), nullptr);
    if (surface->hwnd == nullptr) {
        return false;
    }

    surface->gradientState = mla_private_windows_gdiplus_gradient_state_empty();
    surface->is_initialized = true;
    ShowWindow(surface->hwnd, SW_SHOWDEFAULT);
    return true;
}

mla_ui_surface_input_states_t mla_private_windows_surface_gdiplus_input_states(const mla_ui_surface_t &surface) {
    mla_ui_surface_input_states_t inputStates = mla_ui_surface_input_states_empty();
    mla_windows_window_surface_gdiplus_t *window_surface = mla_s_cast<mla_windows_window_surface_gdiplus_t *>(surface.resource);
    if (window_surface == nullptr || !window_surface->is_initialized || !IsWindow(window_surface->hwnd)) {
        return inputStates;
    }

    POINT cursorPos;
    if (GetCursorPos(&cursorPos) && ScreenToClient(window_surface->hwnd, &cursorPos)) {
        mla_double_t dpiX;
        mla_double_t dpiY;
        mla_private_windows_get_system_dpi(dpiX, dpiY);
        inputStates.cursorPosition.x = (mla_double_t)cursorPos.x * (96.0 / dpiX);
        inputStates.cursorPosition.y = (mla_double_t)cursorPos.y * (96.0 / dpiY);
    }

    inputStates.leftMouseButtonDown = (GetKeyState(VK_LBUTTON) & 0x8000) != 0;
    inputStates.rightMouseButtonDown = (GetKeyState(VK_RBUTTON) & 0x8000) != 0;
    inputStates.middleMouseButtonDown = (GetKeyState(VK_MBUTTON) & 0x8000) != 0;
    inputStates.shiftKeyDown = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    inputStates.ctrlKeyDown = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    inputStates.altKeyDown = (GetKeyState(VK_MENU) & 0x8000) != 0;
    inputStates.metaKeyDown = ((GetKeyState(VK_LWIN) & 0x8000) != 0) || ((GetKeyState(VK_RWIN) & 0x8000) != 0);
    return inputStates;
}

mla_ui_surface_draw_size_t mla_private_windows_surface_gdiplus_calc_text_size(const mla_ui_surface_t &surface,
                                                                     const mla_ui_surface_font_type_t &font_type,
                                                                     const mla_string_t &text) {
    (void)surface;
    mla_ui_surface_draw_size_t size = {0, 0};
    if (mla_string_is_empty(text)) {
        return size;
    }

    Gdiplus::FontFamily fontFamily(L"Arial");
    Gdiplus::FontStyle fontStyle = Gdiplus::FontStyleRegular;
    if (font_type.bold) {
        fontStyle = (Gdiplus::FontStyle)(fontStyle | Gdiplus::FontStyleBold);
    }
    if (font_type.italic) {
        fontStyle = (Gdiplus::FontStyle)(fontStyle | Gdiplus::FontStyleItalic);
    }
    Gdiplus::Font font(&fontFamily, (Gdiplus::REAL)font_type.size, fontStyle, Gdiplus::UnitPixel);

    mla_string_utf16_buffer_t textWide = mla_string_to_utf16_buffer(text);
    if (textWide.data == nullptr) {
        return size;
    }

    HDC screenDc = GetDC(nullptr);
    if (screenDc != nullptr) {
        Gdiplus::Graphics graphics(screenDc);
        Gdiplus::RectF measured;
        graphics.MeasureString((const WCHAR *)textWide.data, (INT)textWide.charCount, &font, Gdiplus::PointF(0, 0), &measured);
        size.width = (mla_double_t)measured.Width;
        size.height = (mla_double_t)measured.Height;
        ReleaseDC(nullptr, screenDc);
    }

    mla_string_utf16_buffer_destroy(textWide);
    return size;
}

void mla_private_windows_gdiplus_fill_path(mla_windows_window_surface_gdiplus_t *window_surface,
                                 Gdiplus::Graphics &graphics,
                                 Gdiplus::GraphicsPath &path,
                                 const mla_ui_surface_draw_command_color_t &fillColor) {
    if (window_surface->gradientState.kind == MLA_WINDOWS_GDIPLUS_GRADIENT_KIND_LINEAR) {
        Gdiplus::LinearGradientBrush gradientBrush(
            Gdiplus::PointF((Gdiplus::REAL)window_surface->gradientState.linear.x1, (Gdiplus::REAL)window_surface->gradientState.linear.y1),
            Gdiplus::PointF((Gdiplus::REAL)window_surface->gradientState.linear.x2, (Gdiplus::REAL)window_surface->gradientState.linear.y2),
            window_surface->gradientState.stop0,
            window_surface->gradientState.stop1
        );
        graphics.FillPath(&gradientBrush, &path);
        return;
    }

    if (window_surface->gradientState.kind == MLA_WINDOWS_GDIPLUS_GRADIENT_KIND_RADIAL) {
        Gdiplus::RectF bounds;
        path.GetBounds(&bounds);
        Gdiplus::GraphicsPath ellipsePath;
        ellipsePath.AddEllipse(bounds);
        Gdiplus::PathGradientBrush radialBrush(&ellipsePath);
        Gdiplus::Color surroundColor = window_surface->gradientState.stop1;
        INT colorCount = 1;
        radialBrush.SetSurroundColors(&surroundColor, &colorCount);
        radialBrush.SetCenterColor(window_surface->gradientState.stop0);
        graphics.FillPath(&radialBrush, &path);
        return;
    }

    Gdiplus::SolidBrush fillBrush(mla_private_windows_gdiplus_convert_color(fillColor));
    graphics.FillPath(&fillBrush, &path);
}

mla_bool_t mla_private_windows_surface_gdiplus_render_draw_commands(const mla_ui_surface_t &surface,
                                                           const mla_array_list_t<mla_init_struct(mla_ui_surface_draw_command_t)> &drawCommands,
                                                           mla_array_list_t<mla_init_struct(mla_ui_surface_input_event_t)> &eventsSinceLastFame) {
    mla_windows_window_surface_gdiplus_t *window_surface = mla_s_cast<mla_windows_window_surface_gdiplus_t *>(surface.resource);
    if (window_surface == nullptr) {
        return false;
    }

    if (!window_surface->is_initialized && !mla_private_windows_create_windows_gdiplus_surface(window_surface)) {
        return false;
    }

    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.hwnd == window_surface->hwnd && (msg.message == WM_LBUTTONUP || msg.message == WM_RBUTTONUP || msg.message == WM_MBUTTONUP)) {
            mla_ui_surface_input_event_t clickEvent = mla_ui_surface_input_event_empty();
            clickEvent.kind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CLICK;
            clickEvent.click.position.x = (mla_double_t)(short)LOWORD(msg.lParam);
            clickEvent.click.position.y = (mla_double_t)(short)HIWORD(msg.lParam);
            clickEvent.click.button = msg.message == WM_LBUTTONUP ? MLA_UI_SURFACE_INPUT_EVENT_CLICK_BUTTON_LEFT :
                                     msg.message == WM_RBUTTONUP ? MLA_UI_SURFACE_INPUT_EVENT_CLICK_BUTTON_RIGHT :
                                     MLA_UI_SURFACE_INPUT_EVENT_CLICK_BUTTON_MIDDLE;
            mla_array_list_add(eventsSinceLastFame, clickEvent);
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (!mla_private_windows_gdiplus_ensure_backbuffer(window_surface)) {
        return false;
    }

    Gdiplus::Graphics graphics(window_surface->memoryDC);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
    graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
    graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);
    graphics.Clear(Gdiplus::Color(255, 255, 255, 255));

    window_surface->gradientState = mla_private_windows_gdiplus_gradient_state_empty();
    Gdiplus::PointF currentPathPoint(0.0f, 0.0f);

    for (mla_size_t i = 0; i < mla_array_list_size(drawCommands); i++) {
        const mla_ui_surface_draw_command_t &cmd = mla_array_list_get_unsafe(drawCommands, i);
        switch (cmd.kind) {
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT: {
                Gdiplus::GraphicsPath path;
                mla_private_windows_gdiplus_add_rounded_rect_path(path, cmd.rect);
                mla_private_windows_gdiplus_fill_path(window_surface, graphics, path, cmd.rect.color);
                if (cmd.rect.stroke_width > 0.0) {
                    Gdiplus::Pen strokePen(mla_private_windows_gdiplus_convert_color(cmd.rect.stroke), (Gdiplus::REAL)cmd.rect.stroke_width);
                    graphics.DrawPath(&strokePen, &path);
                }
                break;
            }
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_CIRCLE: {
                Gdiplus::GraphicsPath path;
                path.AddEllipse((Gdiplus::REAL)(cmd.circle.cx - cmd.circle.r), (Gdiplus::REAL)(cmd.circle.cy - cmd.circle.r),
                                (Gdiplus::REAL)(cmd.circle.r * 2.0), (Gdiplus::REAL)(cmd.circle.r * 2.0));
                mla_private_windows_gdiplus_fill_path(window_surface, graphics, path, cmd.circle.fill);
                if (cmd.circle.stroke_width > 0.0) {
                    Gdiplus::Pen strokePen(mla_private_windows_gdiplus_convert_color(cmd.circle.stroke), (Gdiplus::REAL)cmd.circle.stroke_width);
                    graphics.DrawPath(&strokePen, &path);
                }
                break;
            }
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_ELLIPSE: {
                Gdiplus::GraphicsPath path;
                path.AddEllipse((Gdiplus::REAL)(cmd.ellipse.cx - cmd.ellipse.rx), (Gdiplus::REAL)(cmd.ellipse.cy - cmd.ellipse.ry),
                                (Gdiplus::REAL)(cmd.ellipse.rx * 2.0), (Gdiplus::REAL)(cmd.ellipse.ry * 2.0));
                mla_private_windows_gdiplus_fill_path(window_surface, graphics, path, cmd.ellipse.fill);
                if (cmd.ellipse.stroke_width > 0.0) {
                    Gdiplus::Pen strokePen(mla_private_windows_gdiplus_convert_color(cmd.ellipse.stroke), (Gdiplus::REAL)cmd.ellipse.stroke_width);
                    graphics.DrawPath(&strokePen, &path);
                }
                break;
            }
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINE: {
                Gdiplus::Pen strokePen(mla_private_windows_gdiplus_convert_color(cmd.line.stroke), (Gdiplus::REAL)cmd.line.stroke_width);
                graphics.DrawLine(&strokePen, (Gdiplus::REAL)cmd.line.x1, (Gdiplus::REAL)cmd.line.y1,
                                  (Gdiplus::REAL)cmd.line.x2, (Gdiplus::REAL)cmd.line.y2);
                break;
            }
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_POLYLINE: {
                mla_size_t pointCount = mla_array_list_size(cmd.polyline.points);
                if (pointCount < 2) {
                    break;
                }
                Gdiplus::PointF *points = (Gdiplus::PointF *)mla_platform_malloc(sizeof(Gdiplus::PointF) * pointCount);
                if (points == nullptr) {
                    break;
                }
                for (mla_size_t j = 0; j < pointCount; j++) {
                    const mla_ui_surface_draw_point_t &point = mla_array_list_get_unsafe(cmd.polyline.points, j);
                    points[j].X = (Gdiplus::REAL)point.x;
                    points[j].Y = (Gdiplus::REAL)point.y;
                }
                Gdiplus::Pen strokePen(mla_private_windows_gdiplus_convert_color(cmd.polyline.stroke), (Gdiplus::REAL)cmd.polyline.stroke_width);
                graphics.DrawLines(&strokePen, points, (INT)pointCount);
                mla_platform_free(points);
                break;
            }
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_POLYGON: {
                mla_size_t pointCount = mla_array_list_size(cmd.polygon.points);
                if (pointCount < 3) {
                    break;
                }
                Gdiplus::PointF *points = (Gdiplus::PointF *)mla_platform_malloc(sizeof(Gdiplus::PointF) * pointCount);
                if (points == nullptr) {
                    break;
                }
                for (mla_size_t j = 0; j < pointCount; j++) {
                    const mla_ui_surface_draw_point_t &point = mla_array_list_get_unsafe(cmd.polygon.points, j);
                    points[j].X = (Gdiplus::REAL)point.x;
                    points[j].Y = (Gdiplus::REAL)point.y;
                }
                Gdiplus::GraphicsPath path;
                path.AddPolygon(points, (INT)pointCount);
                mla_private_windows_gdiplus_fill_path(window_surface, graphics, path, cmd.polygon.fill);
                if (cmd.polygon.stroke_width > 0.0) {
                    Gdiplus::Pen strokePen(mla_private_windows_gdiplus_convert_color(cmd.polygon.stroke), (Gdiplus::REAL)cmd.polygon.stroke_width);
                    graphics.DrawPath(&strokePen, &path);
                }
                mla_platform_free(points);
                break;
            }
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_PATH: {
                Gdiplus::GraphicsPath path;
                for (mla_size_t j = 0; j < mla_array_list_size(cmd.path.commands); j++) {
                    const mla_ui_surface_draw_path_command_t &pathCmd = mla_array_list_get_unsafe(cmd.path.commands, j);
                    if (pathCmd.kind == MLA_UI_SURFACE_DRAW_PATH_COMMAND_MOVE_TO) {
                        path.StartFigure();
                        currentPathPoint = Gdiplus::PointF((Gdiplus::REAL)pathCmd.move_to.x, (Gdiplus::REAL)pathCmd.move_to.y);
                    } else if (pathCmd.kind == MLA_UI_SURFACE_DRAW_PATH_COMMAND_LINE_TO) {
                        Gdiplus::PointF nextPoint((Gdiplus::REAL)pathCmd.line_to.x, (Gdiplus::REAL)pathCmd.line_to.y);
                        path.AddLine(currentPathPoint, nextPoint);
                        currentPathPoint = nextPoint;
                    } else if (pathCmd.kind == MLA_UI_SURFACE_DRAW_PATH_COMMAND_QUADRATIC_CURVE_TO) {
                        Gdiplus::PointF cp((Gdiplus::REAL)pathCmd.quadratic_curve_to.cpx, (Gdiplus::REAL)pathCmd.quadratic_curve_to.cpy);
                        Gdiplus::PointF endPoint((Gdiplus::REAL)pathCmd.quadratic_curve_to.x, (Gdiplus::REAL)pathCmd.quadratic_curve_to.y);
                        Gdiplus::PointF cp1(
                            currentPathPoint.X + (2.0f / 3.0f) * (cp.X - currentPathPoint.X),
                            currentPathPoint.Y + (2.0f / 3.0f) * (cp.Y - currentPathPoint.Y)
                        );
                        Gdiplus::PointF cp2(
                            endPoint.X + (2.0f / 3.0f) * (cp.X - endPoint.X),
                            endPoint.Y + (2.0f / 3.0f) * (cp.Y - endPoint.Y)
                        );
                        path.AddBezier(currentPathPoint, cp1, cp2, endPoint);
                        currentPathPoint = endPoint;
                    } else if (pathCmd.kind == MLA_UI_SURFACE_DRAW_PATH_COMMAND_CUBIC_CURVE_TO) {
                        Gdiplus::PointF cp1((Gdiplus::REAL)pathCmd.cubic_curve_to.cp1x, (Gdiplus::REAL)pathCmd.cubic_curve_to.cp1y);
                        Gdiplus::PointF cp2((Gdiplus::REAL)pathCmd.cubic_curve_to.cp2x, (Gdiplus::REAL)pathCmd.cubic_curve_to.cp2y);
                        Gdiplus::PointF endPoint((Gdiplus::REAL)pathCmd.cubic_curve_to.x, (Gdiplus::REAL)pathCmd.cubic_curve_to.y);
                        path.AddBezier(currentPathPoint, cp1, cp2, endPoint);
                        currentPathPoint = endPoint;
                    } else if (pathCmd.kind == MLA_UI_SURFACE_DRAW_PATH_COMMAND_ARC_TO) {
                        Gdiplus::PointF arcEnd((Gdiplus::REAL)pathCmd.arc_to.x, (Gdiplus::REAL)pathCmd.arc_to.y);
                        path.AddLine(currentPathPoint, arcEnd);
                        currentPathPoint = arcEnd;
                    } else if (pathCmd.kind == MLA_UI_SURFACE_DRAW_PATH_COMMAND_CLOSE_PATH) {
                        path.CloseFigure();
                    }
                }
                mla_private_windows_gdiplus_fill_path(window_surface, graphics, path, cmd.path.fill);
                if (cmd.path.stroke_width > 0.0) {
                    Gdiplus::Pen strokePen(mla_private_windows_gdiplus_convert_color(cmd.path.stroke), (Gdiplus::REAL)cmd.path.stroke_width);
                    graphics.DrawPath(&strokePen, &path);
                }
                break;
            }
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT: {
                mla_string_utf16_buffer_t contentWide = mla_string_to_utf16_buffer(cmd.text.content);
                if (contentWide.data != nullptr) {
                    Gdiplus::FontFamily fontFamily(L"Arial");
                    Gdiplus::FontStyle fontStyle = Gdiplus::FontStyleRegular;
                    if (cmd.text.font_type.bold) fontStyle = (Gdiplus::FontStyle)(fontStyle | Gdiplus::FontStyleBold);
                    if (cmd.text.font_type.italic) fontStyle = (Gdiplus::FontStyle)(fontStyle | Gdiplus::FontStyleItalic);
                    Gdiplus::Font font(&fontFamily, (Gdiplus::REAL)cmd.text.font_type.size, fontStyle, Gdiplus::UnitPixel);
                    Gdiplus::SolidBrush textBrush(mla_private_windows_gdiplus_convert_color(cmd.text.fill));
                    graphics.DrawString((const WCHAR *)contentWide.data, (INT)contentWide.charCount, &font,
                                        Gdiplus::PointF((Gdiplus::REAL)cmd.text.x, (Gdiplus::REAL)cmd.text.y), &textBrush);
                }
                mla_string_utf16_buffer_destroy(contentWide);
                break;
            }
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINEAR_GRADIENT:
                window_surface->gradientState.kind = MLA_WINDOWS_GDIPLUS_GRADIENT_KIND_LINEAR;
                window_surface->gradientState.linear = cmd.linear_gradient;
                break;
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_RADIAL_GRADIENT:
                window_surface->gradientState.kind = MLA_WINDOWS_GDIPLUS_GRADIENT_KIND_RADIAL;
                window_surface->gradientState.radial = cmd.radial_gradient;
                break;
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_STOP:
                if (window_surface->gradientState.stopCount == 0) {
                    window_surface->gradientState.stop0 = Gdiplus::Color((BYTE)((mla_double_t)cmd.stop.stop_color.a * cmd.stop.stop_opacity),
                                                                         (BYTE)cmd.stop.stop_color.r, (BYTE)cmd.stop.stop_color.g, (BYTE)cmd.stop.stop_color.b);
                } else {
                    window_surface->gradientState.stop1 = Gdiplus::Color((BYTE)((mla_double_t)cmd.stop.stop_color.a * cmd.stop.stop_opacity),
                                                                         (BYTE)cmd.stop.stop_color.r, (BYTE)cmd.stop.stop_color.g, (BYTE)cmd.stop.stop_color.b);
                }
                if (window_surface->gradientState.stopCount < 2) {
                    window_surface->gradientState.stopCount++;
                }
                break;
            default:
                break;
        }
    }

    HDC windowDC = GetDC(window_surface->hwnd);
    if (windowDC == nullptr) {
        return false;
    }
    BitBlt(windowDC, 0, 0, (int)window_surface->backBufferWidth, (int)window_surface->backBufferHeight,
           window_surface->memoryDC, 0, 0, SRCCOPY);
    ReleaseDC(window_surface->hwnd, windowDC);
    return true;
}

mla_buffer_cleanup_mode mla_private_windows_surface_gdiplus_buffer_cleanup(mla_platform_pointer_t data, const mla_dynamic_data_t &userData) {
    (void)userData;
    mla_windows_window_surface_gdiplus_t *window_surface = mla_s_cast<mla_windows_window_surface_gdiplus_t *>(data);
    if (window_surface != nullptr) {
        if (window_surface->memoryDC != nullptr) {
            if (window_surface->memoryBitmap != nullptr) {
                SelectObject(window_surface->memoryDC, window_surface->oldBitmap);
                DeleteObject(window_surface->memoryBitmap);
            }
            DeleteDC(window_surface->memoryDC);
        }
        if (IsWindow(window_surface->hwnd)) {
            DestroyWindow(window_surface->hwnd);
        }
    }
    return CLEAN_UP_NEEDED;
}

mla_bool_t mla_private_windows_create_gdiplus_surface(mla_ui_surface_t &outSurface) {
    mla_windows_window_surface_gdiplus_t *window_surface =
        mla_s_cast<mla_windows_window_surface_gdiplus_t *>(mla_platform_malloc(sizeof(mla_windows_window_surface_gdiplus_t)));
    if (window_surface == nullptr) {
        return false;
    }
    mla_memset(window_surface, 0, sizeof(mla_windows_window_surface_gdiplus_t));
    window_surface->is_initialized = false;
    window_surface->default_size = {0, 0};
    window_surface->gradientState = mla_private_windows_gdiplus_gradient_state_empty();

    outSurface.resource = window_surface;
    outSurface.resourceOwner = mla_buffer_reference_create(window_surface, true, mla_private_windows_surface_gdiplus_buffer_cleanup, mla_dynamic_data_empty());
    outSurface.get_size = mla_private_windows_surface_gdiplus_get_size;
    outSurface.set_size = mla_private_windows_surface_gdiplus_set_size;
    outSurface.render_draw_commands = mla_private_windows_surface_gdiplus_render_draw_commands;
    outSurface.calc_text_size = mla_private_windows_surface_gdiplus_calc_text_size;
    outSurface.get_input_states = mla_private_windows_surface_gdiplus_input_states;
    return true;
}

mla_ui_display_surface_low_level_access_t g_ui_display_surface_low_level_access = {
    mla_private_windows_create_gdiplus_surface
};

void mla_private_windows_gdiplus_init() {
    SetProcessDPIAware();
    if (g_mla_windows_gdiplus_token == 0) {
        Gdiplus::GdiplusStartupInput startupInput;
        Gdiplus::GdiplusStartup(&g_mla_windows_gdiplus_token, &startupInput, nullptr);
    }
}

void mla_private_windows_gdiplus_shutdown() {
    if (g_mla_windows_gdiplus_token != 0) {
        Gdiplus::GdiplusShutdown(g_mla_windows_gdiplus_token);
        g_mla_windows_gdiplus_token = 0;
    }
}

struct MlaGDIPlusAutoInit {
    MlaGDIPlusAutoInit() { mla_private_windows_gdiplus_init(); }
    ~MlaGDIPlusAutoInit() { mla_private_windows_gdiplus_shutdown(); }
};

static MlaGDIPlusAutoInit g_mlaGDIPlusAutoInit;

#endif
