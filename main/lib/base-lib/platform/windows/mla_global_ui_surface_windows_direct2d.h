//
// Created by chris on 1/21/2026.
//

#ifndef MLA_GLOBAL_UI_SURFACE_WINDOWS_H
#define MLA_GLOBAL_UI_SURFACE_WINDOWS_H

#include "../../core/ui/surfaces/mla_ui_surface.h"
#include "../../core/ui/display/mla_ui_display_surface.h"
#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>

#define mla_global_ui_surface_windows_direct2d_font_cache_size 16
#define mla_global_ui_surface_windows_direct2d_fps_target 60

// Global Direct2D factories
static ID2D1Factory *g_pD2DFactory = nullptr;
static IDWriteFactory *g_pDWriteFactory = nullptr;

struct mla_global_ui_surface_windows_direct2d_font_cache_item {
    mla_ui_surface_font_type_t font_type;
    mla_pointer_t textFormat; // IDWriteTextFormat

    static mla_global_ui_surface_windows_direct2d_font_cache_item init() {
        return {
            mla_ui_surface_font_type_empty(),
            mla_pointer_null()
        };
    }

};



// Render cache structure for performance optimization
struct la_global_ui_surface_windows_direct2d_Cache {
    // Reusable brushes
    mla_pointer_t solidBrush; // ID2D1SolidColorBrush
    D2D1_COLOR_F currentFillColor;

    // Font cache
    mla_array_list_t<mla_init_struct(mla_global_ui_surface_windows_direct2d_font_cache_item)> fontCache;

    static la_global_ui_surface_windows_direct2d_Cache init() {
        return {
            mla_pointer_null(),
            {0, 0, 0, 0},
            mla_array_list_empty<mla_init_struct(mla_global_ui_surface_windows_direct2d_font_cache_item)>()
        };
    }

};

la_global_ui_surface_windows_direct2d_Cache mla_private_global_ui_surface_windows_direct2d_cache_empty() {
    return la_global_ui_surface_windows_direct2d_Cache::init();
}

void mla_private_global_ui_surface_windows_direct2d_font_cache_solidBrush_cleanup(
    const mla_native_resource_t& data) {

    ID2D1SolidColorBrush *solid_brush = mla_s_cast<ID2D1SolidColorBrush *>(data.asPointer);
    if (solid_brush != nullptr) {
        solid_brush->Release();
    }

}


ID2D1SolidColorBrush *mla_private_global_ui_surface_windows_direct2d_cache_getSolid_brush(
    la_global_ui_surface_windows_direct2d_Cache &cache, ID2D1RenderTarget *renderTarget, const D2D1_COLOR_F &color) {

    mla_native_resource_t* native_resource_ptr = mla_native_resource_from_managed_pointer(cache.solidBrush);
    ID2D1SolidColorBrush* solid_brush = nullptr;

    if (native_resource_ptr != nullptr) {
        solid_brush = mla_s_cast<ID2D1SolidColorBrush*>(native_resource_ptr->asPointer);
    }


    if (solid_brush == nullptr) {

        renderTarget->CreateSolidColorBrush(color, &solid_brush);

        if (solid_brush == nullptr) {
            return nullptr;
        }

        mla_native_resource_t native_resource = mla_dynamic_data_from_pointer(solid_brush);
        cache.solidBrush = mla_native_resource_to_managed_pointer(native_resource, mla_private_global_ui_surface_windows_direct2d_font_cache_solidBrush_cleanup);

        if (mla_pointer_is_null(cache.solidBrush)) {
            solid_brush->Release();
            return nullptr;
        }

        cache.currentFillColor = color;

    } else if (mla_memcmp(&cache.currentFillColor, &color, sizeof(D2D1_COLOR_F)) != 0) {

        solid_brush->SetColor(color);
        cache.currentFillColor = color;
    }

    return solid_brush;
}

void mla_private_global_ui_surface_windows_direct2d_font_cache_WriteTextFormat_cleanup(
    const mla_native_resource_t& data) {

    IDWriteTextFormat *textFormat = mla_s_cast<IDWriteTextFormat *>(data.asPointer);
    if (textFormat != nullptr) {
        textFormat->Release();
    }

}

IDWriteTextFormat *mla_private_global_ui_surface_windows_direct2d_font_cache_getOrCreateTextFormat(
    la_global_ui_surface_windows_direct2d_Cache &cache, const mla_ui_surface_font_type_t &fontType) {
    if (mla_string_is_empty(fontType.family)) {
        return nullptr;
    }

    // Check if font already exists in cache
    for (mla_size_t i = 0; i < mla_array_list_size(cache.fontCache); i++) {
        const mla_global_ui_surface_windows_direct2d_font_cache_item &item = mla_array_list_get_unsafe(
            cache.fontCache, i);

        if (!mla_ui_surface_font_type_equals(item.font_type, fontType)) {
            continue;
        }

        mla_native_resource_t* resource = mla_native_resource_from_managed_pointer(item.textFormat);

        if (resource == nullptr) {
            continue;
        }


        return mla_s_cast<IDWriteTextFormat *>(resource->asPointer);
    }

    mla_string_utf16_buffer_t fontFamilyWide = mla_string_to_utf16_buffer(fontType.family);

    const mla_utf_16_char_t* fontFamilyWide_data = mla_string_utf16_data(fontFamilyWide);

    if (fontFamilyWide_data == nullptr) {
        return nullptr;
    }

    // Create new text format
    IDWriteTextFormat *textFormat = nullptr;

    DWRITE_FONT_STYLE fontStyle = DWRITE_FONT_STYLE_NORMAL;

    if (fontType.italic) {
        fontStyle = DWRITE_FONT_STYLE_ITALIC;
    }

    DWRITE_FONT_WEIGHT fontWeight = DWRITE_FONT_WEIGHT_NORMAL;

    if (fontType.bold) {
        fontWeight = DWRITE_FONT_WEIGHT_BOLD;
    }

    g_pDWriteFactory->CreateTextFormat(
        mla_r_cast<const WCHAR *>(fontFamilyWide_data),
        nullptr,
        fontWeight,
        fontStyle,
        DWRITE_FONT_STRETCH_NORMAL,
        mla_s_cast<FLOAT>(fontType.size),
        L"en-us",
        &textFormat
    );

    if (textFormat == nullptr) {
        return nullptr;
    }

    mla_native_resource_t native_resource = mla_dynamic_data_from_pointer(textFormat);

    mla_global_ui_surface_windows_direct2d_font_cache_item newItem = {
        fontType,
        mla_native_resource_to_managed_pointer(native_resource, mla_private_global_ui_surface_windows_direct2d_font_cache_WriteTextFormat_cleanup),
    };

    if (mla_array_list_size(cache.fontCache) < mla_global_ui_surface_windows_direct2d_font_cache_size) {
        // Add new item to cache
        mla_array_list_add(cache.fontCache, newItem);
    } else {
        mla_global_ui_surface_windows_direct2d_font_cache_item &oldItem = mla_array_list_get_unsafe(cache.fontCache, 0);
        // Overwrite which also trigger the cleanup of the old item
        oldItem = newItem;
    }

    return textFormat;
}

struct mla_windows_window_surface_t {
    HWND hwnd;
    mla_bool_t is_initialized;
    mla_ui_surface_size_t default_size;

    // Direct2D rendering resources
    ID2D1HwndRenderTarget *renderTarget;
    la_global_ui_surface_windows_direct2d_Cache renderCache;

#ifdef mla_debug_build
    DWORD DEBUG_last_fps_time;
    int DEBUG_frames_accumulated;
    int DEBUG_current_fps;
#endif

    static mla_windows_window_surface_t init() {
        return {
            nullptr,
            false,
            {800, 600}, // Default size if not set before initialization
            nullptr,
            mla_private_global_ui_surface_windows_direct2d_cache_empty(),
#ifdef mla_debug_build
            0,
            0,
            0,
#endif
        };
    }
};

// Stub implementations for the surface function pointers
mla_ui_surface_size_t mla_private_windows_surface_get_size(const mla_ui_surface_t &surface) {
    mla_ui_surface_size_t size = {0, 0};

    mla_windows_window_surface_t *window_surface = mla_pointer_get_data<mla_windows_window_surface_t>(surface.resource);

    if (window_surface == nullptr) {
        return size;
    }

    if (!window_surface->is_initialized) {
        return window_surface->default_size;
    }

    if (IsWindow(window_surface->hwnd) == FALSE) {
        return size;
    }

    RECT rect;
    if (GetClientRect(window_surface->hwnd, &rect) == TRUE) {
        // Get physical pixel size
        mla_uint32_t physicalWidth = mla_s_cast<mla_uint32_t>(rect.right - rect.left);
        mla_uint32_t physicalHeight = mla_s_cast<mla_uint32_t>(rect.bottom - rect.top);

        // Convert to DIPs using system DPI
        FLOAT dpiX = 96.0F;
        FLOAT dpiY = 96.0F;
        if (g_pD2DFactory != nullptr) {
            // NOLINTBEGIN(clang-diagnostic-deprecated-declarations)
            g_pD2DFactory->GetDesktopDpi(&dpiX, &dpiY);
            // NOLINTEND(clang-diagnostic-deprecated-declarations)
        }

        size.width = mla_s_cast<mla_uint32_t>(mla_s_cast<mla_double_t>(physicalWidth) * (96.0F / dpiX));
        size.height = mla_s_cast<mla_uint32_t>(mla_s_cast<mla_double_t>(physicalHeight) * (96.0F / dpiY));
    }

    return size;
}

mla_bool_t mla_private_windows_surface_set_size(const mla_ui_surface_t &surface, mla_ui_surface_size_t size) {

    mla_windows_window_surface_t *window_surface = mla_pointer_get_data<mla_windows_window_surface_t>(surface.resource);

    if (window_surface == nullptr) {
        return false;
    }

    if (!window_surface->is_initialized) {
        window_surface->default_size = size;
        return true;
    }

    if (IsWindow(window_surface->hwnd) == FALSE) {
        return false;
    }

    // Convert DIPs to physical pixels using system DPI
    FLOAT dpiX = 96.0F;
    FLOAT dpiY = 96.0F;
    if (g_pD2DFactory != nullptr) {
        // NOLINTBEGIN(clang-diagnostic-deprecated-declarations)
        g_pD2DFactory->GetDesktopDpi(&dpiX, &dpiY);
        // NOLINTEND(clang-diagnostic-deprecated-declarations)
    }

    mla_int32_t physicalWidth = mla_s_cast<mla_int32_t>(mla_s_cast<mla_double_t>(size.width) * (dpiX / 96.0F));
    mla_int32_t physicalHeight = mla_s_cast<mla_int32_t>(mla_s_cast<mla_double_t>(size.height) * (dpiY / 96.0F));

    if (SetWindowPos(window_surface->hwnd, nullptr, 0, 0, physicalWidth, physicalHeight,
                     SWP_NOMOVE | SWP_NOZORDER) == TRUE) {
        return true;
                     }

    return false;
}

LRESULT CALLBACK mla_private_windows_surface_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

mla_bool_t mla_private_create_windows_surface(mla_windows_window_surface_t *surface) {
    const char CLASS_NAME[] = "MLA_Window_Class";

    WNDCLASSEXA wc = {};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = mla_private_windows_surface_proc;
    wc.hInstance = GetModuleHandleA(nullptr);
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursorA(nullptr, IDC_ARROW);

    RegisterClassExA(&wc);

    int width = mla_s_cast<int>(surface->default_size.width);
    if (width == 0) {
        width = CW_USEDEFAULT;
    }

    int height = mla_s_cast<int>(surface->default_size.height);
    if (height == 0) {
        height = CW_USEDEFAULT;
    }

    HWND hwnd = CreateWindowExA(
        0,
        CLASS_NAME,
        "MLA App",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        nullptr,
        nullptr,
        GetModuleHandleA(nullptr),
        nullptr
    );

    if (hwnd == nullptr) {
        return false;
    }

    surface->renderCache = mla_private_global_ui_surface_windows_direct2d_cache_empty();
    surface->is_initialized = true;
    surface->hwnd = hwnd;
    ShowWindow(hwnd, SW_SHOWDEFAULT);

    return true;
}

D2D1_COLOR_F mla_private_convert_color(const mla_ui_surface_draw_command_color_t &color) {
    return D2D1::ColorF(
        mla_s_cast<FLOAT>(color.r) / 255.0F,
        mla_s_cast<FLOAT>(color.g) / 255.0F,
        mla_s_cast<FLOAT>(color.b) / 255.0F,
        mla_s_cast<FLOAT>(color.a) / 255.0F
    );
}

mla_bool_t mla_private_ScreenPosition_to_client_position(const HWND &hwnd, POINT &cursorPos,
                                                       mla_ui_surface_draw_point_t &out_clientPosition) {
    // 1. Mouse Position
    RECT clientRect;
    if (GetClientRect(hwnd, &clientRect) == TRUE) {
        if (PtInRect(&clientRect, cursorPos) == TRUE) {
            // Retrieve system DPI to convert physical pixels to logical DIPs (Device Independent Pixels)
            // This ensures input coordinates match the content drawn by Direct2D at High DPI.
            FLOAT dpiX = 96.0F;
            FLOAT dpiY = 96.0F;
            if (g_pD2DFactory != nullptr) {
                // NOLINTBEGIN(clang-diagnostic-deprecated-declarations)
                g_pD2DFactory->GetDesktopDpi(&dpiX, &dpiY);
                // NOLINTEND(clang-diagnostic-deprecated-declarations)
            }

            out_clientPosition.x = mla_s_cast<mla_double_t>(cursorPos.x) * (96.0F / dpiX);
            out_clientPosition.y = mla_s_cast<mla_double_t>(cursorPos.y) * (96.0F / dpiY);
            return true;
        }
    }

    return false;
}

mla_ui_surface_input_states_t mla_private_windows_surface_input_states(const mla_ui_surface_t &surface) {

    mla_ui_surface_input_states_t inputStates = mla_ui_surface_input_states_empty();

    mla_windows_window_surface_t *window_surface = mla_pointer_get_data<mla_windows_window_surface_t>(surface.resource);

    // Validate surface state
    if (window_surface == nullptr || !window_surface->is_initialized || (IsWindow(window_surface->hwnd) == FALSE)) {
        return inputStates;
    }

    // 1. Mouse Position
    POINT cursorPos;
    if (GetCursorPos(&cursorPos) == TRUE) {
        if (ScreenToClient(window_surface->hwnd, &cursorPos) == TRUE) {
            mla_private_ScreenPosition_to_client_position(window_surface->hwnd, cursorPos, inputStates.cursorPosition);
        }
    }
    // 2. Mouse Buttons
    // specific bits: 0x8000 means the key is currently down
    inputStates.leftMouseButtonDown = (GetKeyState(VK_LBUTTON) & 0x8000) != 0;
    inputStates.rightMouseButtonDown = (GetKeyState(VK_RBUTTON) & 0x8000) != 0;
    inputStates.middleMouseButtonDown = (GetKeyState(VK_MBUTTON) & 0x8000) != 0;

    // 3. Modifier Keys
    inputStates.shiftKeyDown = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    inputStates.ctrlKeyDown = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    inputStates.altKeyDown = (GetKeyState(VK_MENU) & 0x8000) != 0;
    inputStates.metaKeyDown = ((GetKeyState(VK_LWIN) & 0x8000) != 0) ||
                              ((GetKeyState(VK_RWIN) & 0x8000) != 0);

    // 4. Key Code Down (basic polling)
    // Scan common virtual key range for the first pressed key (skipping mouse buttons)
    inputStates.keyCodeDown = 0;
    for (int key = 0x08; key <= 0xFE; ++key) {
        if ((key == VK_SHIFT) || (key == VK_CONTROL) || (key == VK_MENU)) {
            continue;
        }


        if ((GetKeyState(key) & 0x8000) != 0) {
            inputStates.keyCodeDown = mla_s_cast<mla_uint32_t>(key);
            break; // Valid limitation: only reports the first detected key
        }
    }

    return inputStates;
}


mla_ui_surface_draw_size_t mla_private_windows_surface_calc_text_size(const mla_ui_surface_t &surface,
                                                            const mla_ui_surface_font_type_t &font_type,
                                                            const mla_string_t &text) {
    mla_ui_surface_draw_size_t size = {0, 0};

    if (mla_string_is_empty(text) || g_pDWriteFactory == nullptr) {
        return size;
    }

    // Try to get the cache from the surface to reuse text formats for performance
    mla_windows_window_surface_t *window_surface = mla_pointer_get_data<mla_windows_window_surface_t>(surface.resource);

    if (window_surface == nullptr) {
        return size;
    }

    IDWriteTextFormat *textFormat = mla_private_global_ui_surface_windows_direct2d_font_cache_getOrCreateTextFormat(
        window_surface->renderCache, font_type);

    if (textFormat != nullptr) {
        mla_string_utf16_buffer_t textWide = mla_string_to_utf16_buffer(text);

        const mla_utf_16_char_t* textWide_data = mla_string_utf16_data(textWide);

        if (textWide_data == nullptr) {
            return size;
        }

        // Calculate text length
        const WCHAR *textPtr = mla_r_cast<const WCHAR *>(textWide_data);
        UINT32 textLength = mla_s_cast<UINT32>(wcslen(textPtr));

        IDWriteTextLayout *textLayout = nullptr;

        // Create a layout with large constraints to measure natural size
        HRESULT hr = g_pDWriteFactory->CreateTextLayout(
            textPtr,
            textLength,
            textFormat,
            10000.0F, // Max width
            10000.0F, // Max height
            &textLayout
        );

        if (SUCCEEDED(hr) && textLayout != nullptr) {
            DWRITE_TEXT_METRICS metrics;
            if (SUCCEEDED(textLayout->GetMetrics(&metrics))) {
                size.width = mla_s_cast<mla_double_t>(metrics.widthIncludingTrailingWhitespace);
                size.height = mla_s_cast<mla_double_t>(metrics.height);
            }
            textLayout->Release();
        }
    }

    return size;
}


mla_bool_t mla_private_windows_surface_render_draw_commands(const mla_ui_surface_t &surface,
                                                  const mla_array_list_t<mla_init_struct(mla_ui_surface_draw_command_t)> &drawCommands,
                                                  mla_array_list_t<mla_init_struct(mla_ui_surface_input_event_t)> &eventsSinceLastFame, mla_uint64_t timeSinceLastFrameMs) {

    mla_windows_window_surface_t *window_surface = mla_pointer_get_data<mla_windows_window_surface_t>(surface.resource);
    if (window_surface == nullptr) {
        return false;
    }

    if (!window_surface->is_initialized) {
        if (!mla_private_create_windows_surface(window_surface)) {
            mla_warning("Failed to initialize Windows UI surface for drawing.");
            return false;
        }
    }

    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.hwnd == window_surface->hwnd) {
            // Handle Mouse Release Events (Clicks)
            if (msg.message == WM_LBUTTONUP || msg.message == WM_RBUTTONUP || msg.message == WM_MBUTTONUP) {
                // 1. Extract physical client coordinates
                // (short) cast ensures negative coordinates are handled correctly
                int physicalX = mla_s_cast<short>(LOWORD(msg.lParam));
                int physicalY = mla_s_cast<short>(HIWORD(msg.lParam));
                POINT cursorPos = {physicalX, physicalY};
                mla_ui_surface_input_event_t clickEvent = mla_ui_surface_input_event_empty();
                clickEvent.kind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CLICK;
                mla_private_ScreenPosition_to_client_position(window_surface->hwnd, cursorPos, clickEvent.click.position);

                if (msg.message == WM_LBUTTONUP) {
                    clickEvent.click.button = MLA_UI_SURFACE_INPUT_EVENT_CLICK_BUTTON_LEFT;
                } else if (msg.message == WM_RBUTTONUP) {
                    clickEvent.click.button = MLA_UI_SURFACE_INPUT_EVENT_CLICK_BUTTON_RIGHT;
                } else if (msg.message == WM_MBUTTONUP) {
                    clickEvent.click.button = MLA_UI_SURFACE_INPUT_EVENT_CLICK_BUTTON_MIDDLE;
                }

                mla_array_list_add(eventsSinceLastFame, clickEvent);
            } else if (msg.message == WM_KEYDOWN) {

                // Check if Shift or Ctrl is held down to avoid generating char input for shortcuts
                surface_input_event_spical_control_char_kind pressedControlKeys = MLA_UI_SURFACE_INPUT_EVENT_KIND_CONTROL_NONE;

                if ((GetKeyState(VK_SHIFT) & 0x8000) != 0) {
                    pressedControlKeys = mla_s_cast<surface_input_event_spical_control_char_kind>(
                        pressedControlKeys | MLA_UI_SURFACE_INPUT_EVENT_KIND_CONTROL_SHIFT);
                }

                if ((GetKeyState(VK_CONTROL) & 0x8000) != 0) {
                    pressedControlKeys = mla_s_cast<surface_input_event_spical_control_char_kind>(
                        pressedControlKeys | MLA_UI_SURFACE_INPUT_EVENT_KIND_CONTROL_CTRL);
                }

                if ((GetKeyState(VK_MENU) & 0x8000) != 0) {
                    pressedControlKeys = mla_s_cast<surface_input_event_spical_control_char_kind>(
                        pressedControlKeys | MLA_UI_SURFACE_INPUT_EVENT_KIND_CONTROL_ALT);
                }


                mla_ui_surface_input_event_char_input_kind specialKeyKind = mla_s_cast<mla_ui_surface_input_event_char_input_kind>(
                        0xFF);

                switch (msg.wParam) {
                    case VK_RETURN: specialKeyKind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_ENTER;
                        break;
                    case VK_BACK: specialKeyKind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_BACKSPACE;
                        break;
                    case VK_DELETE: specialKeyKind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_DELETE;
                        break;
                    case VK_TAB: specialKeyKind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_TAB;
                        break;
                    case VK_ESCAPE: specialKeyKind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_ESCAPE;
                        break;
                    case VK_UP: specialKeyKind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_ARROW_UP;
                        break;
                    case VK_DOWN: specialKeyKind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_ARROW_DOWN;
                        break;
                    case VK_LEFT: specialKeyKind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_ARROW_LEFT;
                        break;
                    case VK_RIGHT: specialKeyKind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_ARROW_RIGHT;
                        break;
                    default:
                        break;
                }

                if (specialKeyKind != 0xFF) {
                    mla_ui_surface_input_event_t event = mla_ui_surface_input_event_empty();
                    event.kind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR;
                    event.char_input.kind = specialKeyKind;
                    event.char_input.pressedControlKeys = pressedControlKeys;
                    mla_memset(event.char_input.character, 0, 4);
                    mla_array_list_add(eventsSinceLastFame, event);
                } else {
                    // For other keys, we can generate a char input event if it's a printable character
                    BYTE keyboardState[256];
                    GetKeyboardState(keyboardState);

                    WCHAR unicodeChar[4];
                    int result = ToUnicode(mla_s_cast<UINT>(msg.wParam), mla_s_cast<UINT>((msg.lParam >> 16) & 0xFF), keyboardState,
                                           unicodeChar, 4, 0);
                    if (result > 0) {
                        mla_ui_surface_input_event_t charInputEvent = mla_ui_surface_input_event_empty();
                        charInputEvent.kind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR;
                        charInputEvent.char_input.kind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_INPUT;
                        charInputEvent.char_input.pressedControlKeys = pressedControlKeys;

                        // Convert WCHAR to UTF-8
                        int utf8Length = WideCharToMultiByte(CP_UTF8, 0, unicodeChar, result, nullptr, 0, nullptr,
                                                             nullptr);
                        if (utf8Length > 0 && utf8Length < 4) {
                            WideCharToMultiByte(CP_UTF8, 0, unicodeChar, result, charInputEvent.char_input.character,
                                                utf8Length, nullptr, nullptr);
                            charInputEvent.char_input.character[utf8Length] = '\0'; // Null-terminate
                            mla_array_list_add(eventsSinceLastFame, charInputEvent);
                        }
                    }
                }
            }
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Create render target if needed
    if (window_surface->renderTarget == nullptr) {
        RECT rc;
        GetClientRect(window_surface->hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

        // Quality Improvement: Use explicit 32-bit color format
        D2D1_PIXEL_FORMAT pixelFormat = D2D1::PixelFormat(
            DXGI_FORMAT_B8G8R8A8_UNORM,
            D2D1_ALPHA_MODE_PREMULTIPLIED
        );

        // Quality Improvement: Allow D2D to auto-detect DPI (0,0) based on factory settings
        D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            pixelFormat,
            0, 0,
            D2D1_RENDER_TARGET_USAGE_NONE,
            D2D1_FEATURE_LEVEL_DEFAULT
        );

        g_pD2DFactory->CreateHwndRenderTarget(
            props,
#ifdef mla_debug
            // In debug builds, use immediate presentation for lower latency and max fps
            D2D1::HwndRenderTargetProperties(window_surface->hwnd, size, D2D1_PRESENT_OPTIONS_IMMEDIATELY),
#else
            D2D1::HwndRenderTargetProperties(window_surface->hwnd, size),
#endif

            &window_surface->renderTarget
        );


        if (window_surface->renderTarget == nullptr) {
            return false;
        }

        window_surface->renderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        window_surface->renderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
    } else {
        // Fix: Ensure RenderTarget matches Window Client Size
        // If we don't do this, D2D scales the output, making it look blurry.
        RECT rc;
        GetClientRect(window_surface->hwnd, &rc);
        D2D1_SIZE_U windowSize = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
        D2D1_SIZE_U targetSize = window_surface->renderTarget->GetPixelSize();

        if (windowSize.width != targetSize.width || windowSize.height != targetSize.height) {
            window_surface->renderTarget->Resize(windowSize);
        }
    }

    if (timeSinceLastFrameMs < mla_s_cast<mla_uint64_t>(1000 * 0.75 / mla_global_ui_surface_windows_direct2d_fps_target)) {
        // Skip rendering to maintain target FPS
        return false;
    }


    ID2D1HwndRenderTarget *renderTarget = window_surface->renderTarget;
    la_global_ui_surface_windows_direct2d_Cache cache = window_surface->renderCache;

    // Begin drawing
    renderTarget->BeginDraw();

    // Clear background
    renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

    // Temporary brushes for gradients
    ID2D1LinearGradientBrush *currentLinearGradient = nullptr;
    ID2D1RadialGradientBrush *currentRadialGradient = nullptr;
    ID2D1SolidColorBrush *strokeBrush = nullptr;

    // Track current path being built
    D2D1_POINT_2F currentPathPoint = {0, 0};

    for (mla_size_t i = 0; i < mla_array_list_size(drawCommands); i++) {
        const mla_ui_surface_draw_command_t &cmd = mla_array_list_get_unsafe(drawCommands, i);

        switch (cmd.kind) {
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT: {
                const auto &rect = cmd.rect;
                D2D1_RECT_F d2dRect = D2D1::RectF(
                    mla_s_cast<FLOAT>(rect.x), mla_s_cast<FLOAT>(rect.y),
                    mla_s_cast<FLOAT>(rect.x + rect.width), mla_s_cast<FLOAT>(rect.y + rect.height)
                );

                // Fill
                ID2D1SolidColorBrush *solidBrush = mla_private_global_ui_surface_windows_direct2d_cache_getSolid_brush(
                    cache, renderTarget, mla_private_convert_color(rect.color));
                if (rect.rx > 0 || rect.ry > 0) {
                    D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(d2dRect, mla_s_cast<FLOAT>(rect.rx), mla_s_cast<FLOAT>(rect.ry));
                    renderTarget->FillRoundedRectangle(roundedRect, solidBrush);
                } else {
                    renderTarget->FillRectangle(d2dRect, solidBrush);
                }

                // Stroke
                if (rect.stroke_width > 0) {
                    if (strokeBrush == nullptr) {
                        renderTarget->CreateSolidColorBrush(mla_private_convert_color(rect.stroke), &strokeBrush);
                    } else {
                        strokeBrush->SetColor(mla_private_convert_color(rect.stroke));
                    }

                    if (rect.rx > 0 || rect.ry > 0) {
                        D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(d2dRect, mla_s_cast<FLOAT>(rect.rx), mla_s_cast<FLOAT>(rect.ry));
                        renderTarget->DrawRoundedRectangle(roundedRect, strokeBrush, mla_s_cast<FLOAT>(rect.stroke_width));
                    } else {
                        renderTarget->DrawRectangle(d2dRect, strokeBrush, mla_s_cast<FLOAT>(rect.stroke_width));
                    }
                }
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_CIRCLE: {
                const auto &circle = cmd.circle;
                D2D1_ELLIPSE ellipse = D2D1::Ellipse(
                    D2D1::Point2F(mla_s_cast<FLOAT>(circle.cx), mla_s_cast<FLOAT>(circle.cy)),
                    mla_s_cast<FLOAT>(circle.r), mla_s_cast<FLOAT>(circle.r)
                );

                ID2D1SolidColorBrush *solidBrush = mla_private_global_ui_surface_windows_direct2d_cache_getSolid_brush(
                    cache, renderTarget, mla_private_convert_color(circle.fill));
                renderTarget->FillEllipse(ellipse, solidBrush);

                if (circle.stroke_width > 0) {
                    if (strokeBrush == nullptr) {
                        renderTarget->CreateSolidColorBrush(mla_private_convert_color(circle.stroke), &strokeBrush);
                    } else {
                        strokeBrush->SetColor(mla_private_convert_color(circle.stroke));
                    }
                    renderTarget->DrawEllipse(ellipse, strokeBrush, mla_s_cast<FLOAT>(circle.stroke_width));
                }
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_ELLIPSE: {
                const auto &ellipseCmd = cmd.ellipse;
                D2D1_ELLIPSE ellipse = D2D1::Ellipse(
                    D2D1::Point2F(mla_s_cast<FLOAT>(ellipseCmd.cx), mla_s_cast<FLOAT>(ellipseCmd.cy)),
                    mla_s_cast<FLOAT>(ellipseCmd.rx), mla_s_cast<FLOAT>(ellipseCmd.ry)
                );

                ID2D1SolidColorBrush *solidBrush = mla_private_global_ui_surface_windows_direct2d_cache_getSolid_brush(
                    cache, renderTarget, mla_private_convert_color(ellipseCmd.fill));
                renderTarget->FillEllipse(ellipse, solidBrush);

                if (ellipseCmd.stroke_width > 0) {
                    if (strokeBrush == nullptr) {
                        renderTarget->CreateSolidColorBrush(mla_private_convert_color(ellipseCmd.stroke), &strokeBrush);
                    } else {
                        strokeBrush->SetColor(mla_private_convert_color(ellipseCmd.stroke));
                    }
                    renderTarget->DrawEllipse(ellipse, strokeBrush, mla_s_cast<FLOAT>(ellipseCmd.stroke_width));
                }
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINE: {
                const auto &line = cmd.line;
                if (strokeBrush == nullptr) {
                    renderTarget->CreateSolidColorBrush(mla_private_convert_color(line.stroke), &strokeBrush);
                } else {
                    strokeBrush->SetColor(mla_private_convert_color(line.stroke));
                }

                renderTarget->DrawLine(
                    D2D1::Point2F(mla_s_cast<FLOAT>(line.x1), mla_s_cast<FLOAT>(line.y1)),
                    D2D1::Point2F(mla_s_cast<FLOAT>(line.x2), mla_s_cast<FLOAT>(line.y2)),
                    strokeBrush,
                    mla_s_cast<FLOAT>(line.stroke_width)
                );
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_POLYLINE: {
                const auto &polyline = cmd.polyline;
                if (mla_array_list_size(polyline.points) < 2) {
                    break;
                }

                ID2D1PathGeometry *geometry = nullptr;
                ID2D1GeometrySink *sink = nullptr;

                g_pD2DFactory->CreatePathGeometry(&geometry);
                geometry->Open(&sink);

                const mla_ui_surface_draw_point_t &firstPoint = mla_array_list_get_unsafe(polyline.points, 0);
                sink->BeginFigure(D2D1::Point2F(mla_s_cast<FLOAT>(firstPoint.x), mla_s_cast<FLOAT>(firstPoint.y)), D2D1_FIGURE_BEGIN_HOLLOW);

                for (mla_size_t j = 1; j < mla_array_list_size(polyline.points); j++) {
                    const mla_ui_surface_draw_point_t &point = mla_array_list_get_unsafe(polyline.points, j);
                    sink->AddLine(D2D1::Point2F(mla_s_cast<FLOAT>(point.x), mla_s_cast<FLOAT>(point.y)));
                }

                sink->EndFigure(D2D1_FIGURE_END_OPEN);
                sink->Close();

                if (strokeBrush == nullptr) {
                    renderTarget->CreateSolidColorBrush(mla_private_convert_color(polyline.stroke), &strokeBrush);
                } else {
                    strokeBrush->SetColor(mla_private_convert_color(polyline.stroke));
                }
                renderTarget->DrawGeometry(geometry, strokeBrush, mla_s_cast<FLOAT>(polyline.stroke_width));

                sink->Release();
                geometry->Release();
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_POLYGON: {
                const auto &polygon = cmd.polygon;
                if (mla_array_list_size(polygon.points) < 3) {
                    break;
                }

                ID2D1PathGeometry *geometry = nullptr;
                ID2D1GeometrySink *sink = nullptr;

                g_pD2DFactory->CreatePathGeometry(&geometry);
                geometry->Open(&sink);

                const mla_ui_surface_draw_point_t &firstPoint = mla_array_list_get_unsafe(polygon.points, 0);
                sink->BeginFigure(D2D1::Point2F(mla_s_cast<FLOAT>(firstPoint.x), mla_s_cast<FLOAT>(firstPoint.y)), D2D1_FIGURE_BEGIN_FILLED);

                for (mla_size_t j = 1; j < mla_array_list_size(polygon.points); j++) {
                    const mla_ui_surface_draw_point_t &point = mla_array_list_get_unsafe(polygon.points, j);
                    sink->AddLine(D2D1::Point2F(mla_s_cast<FLOAT>(point.x), mla_s_cast<FLOAT>(point.y)));
                }

                sink->EndFigure(D2D1_FIGURE_END_CLOSED);
                sink->Close();

                ID2D1SolidColorBrush *solidBrush = mla_private_global_ui_surface_windows_direct2d_cache_getSolid_brush(
                    cache, renderTarget, mla_private_convert_color(polygon.fill));
                renderTarget->FillGeometry(geometry, solidBrush);

                if (polygon.stroke_width > 0) {
                    if (strokeBrush == nullptr) {
                        renderTarget->CreateSolidColorBrush(mla_private_convert_color(polygon.stroke), &strokeBrush);
                    } else {
                        strokeBrush->SetColor(mla_private_convert_color(polygon.stroke));
                    }
                    renderTarget->DrawGeometry(geometry, strokeBrush, mla_s_cast<FLOAT>(polygon.stroke_width));
                }

                sink->Release();
                geometry->Release();
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_PATH: {
                const auto &path = cmd.path;
                if (mla_array_list_size(path.commands) == 0) {
                    break;
                }

                ID2D1PathGeometry *geometry = nullptr;
                ID2D1GeometrySink *sink = nullptr;

                g_pD2DFactory->CreatePathGeometry(&geometry);
                geometry->Open(&sink);

                bool figureStarted = false;

                for (mla_size_t j = 0; j < mla_array_list_size(path.commands); j++) {
                    const auto &pathCmd = mla_array_list_get_unsafe(path.commands, j);

                    switch (pathCmd.kind) {
                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_MOVE_TO:
                            if (figureStarted) {
                                sink->EndFigure(D2D1_FIGURE_END_OPEN);
                            }
                            currentPathPoint = D2D1::Point2F(mla_s_cast<FLOAT>(pathCmd.move_to.x), mla_s_cast<FLOAT>(pathCmd.move_to.y));
                            sink->BeginFigure(currentPathPoint, D2D1_FIGURE_BEGIN_FILLED);
                            figureStarted = true;
                            break;

                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_LINE_TO:
                            if (figureStarted) {
                                currentPathPoint = D2D1::Point2F(mla_s_cast<FLOAT>(pathCmd.line_to.x), mla_s_cast<FLOAT>(pathCmd.line_to.y));
                                sink->AddLine(currentPathPoint);
                            }
                            break;

                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_QUADRATIC_CURVE_TO:
                            if (figureStarted) {
                                D2D1_QUADRATIC_BEZIER_SEGMENT segment;
                                segment.point1 = D2D1::Point2F(mla_s_cast<FLOAT>(pathCmd.quadratic_curve_to.cpx),
                                                               mla_s_cast<FLOAT>(pathCmd.quadratic_curve_to.cpy));
                                segment.point2 = D2D1::Point2F(mla_s_cast<FLOAT>(pathCmd.quadratic_curve_to.x),
                                                               mla_s_cast<FLOAT>(pathCmd.quadratic_curve_to.y));
                                sink->AddQuadraticBezier(segment);
                                currentPathPoint = segment.point2;
                            }
                            break;

                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_CUBIC_CURVE_TO:
                            if (figureStarted) {
                                D2D1_BEZIER_SEGMENT segment;
                                segment.point1 = D2D1::Point2F(mla_s_cast<FLOAT>(pathCmd.cubic_curve_to.cp1x),
                                                               mla_s_cast<FLOAT>(pathCmd.cubic_curve_to.cp1y));
                                segment.point2 = D2D1::Point2F(mla_s_cast<FLOAT>(pathCmd.cubic_curve_to.cp2x),
                                                               mla_s_cast<FLOAT>(pathCmd.cubic_curve_to.cp2y));
                                segment.point3 = D2D1::Point2F(mla_s_cast<FLOAT>(pathCmd.cubic_curve_to.x),
                                                               mla_s_cast<FLOAT>(pathCmd.cubic_curve_to.y));
                                sink->AddBezier(segment);
                                currentPathPoint = segment.point3;
                            }
                            break;

                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_ARC_TO:
                            if (figureStarted) {
                                D2D1_ARC_SEGMENT segment;
                                segment.point = D2D1::Point2F(mla_s_cast<FLOAT>(pathCmd.arc_to.x), mla_s_cast<FLOAT>(pathCmd.arc_to.y));
                                segment.size = D2D1::SizeF(mla_s_cast<FLOAT>(pathCmd.arc_to.rx), mla_s_cast<FLOAT>(pathCmd.arc_to.ry));
                                segment.rotationAngle = mla_s_cast<FLOAT>(pathCmd.arc_to.x_axis_rotation);
                                segment.sweepDirection = pathCmd.arc_to.sweep_flag
                                                             ? D2D1_SWEEP_DIRECTION_CLOCKWISE
                                                             : D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE;
                                segment.arcSize = pathCmd.arc_to.large_arc_flag
                                                      ? D2D1_ARC_SIZE_LARGE
                                                      : D2D1_ARC_SIZE_SMALL;
                                sink->AddArc(segment);
                                currentPathPoint = segment.point;
                            }
                            break;

                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_CLOSE_PATH:
                            if (figureStarted) {
                                sink->EndFigure(D2D1_FIGURE_END_CLOSED);
                                figureStarted = false;
                            }
                            break;
                    }
                }

                if (figureStarted) {
                    sink->EndFigure(D2D1_FIGURE_END_OPEN);
                }

                sink->Close();

                ID2D1SolidColorBrush *solidBrush = mla_private_global_ui_surface_windows_direct2d_cache_getSolid_brush(
                    cache, renderTarget, mla_private_convert_color(path.fill));
                renderTarget->FillGeometry(geometry, solidBrush);

                if (path.stroke_width > 0) {
                    if (strokeBrush == nullptr) {
                        renderTarget->CreateSolidColorBrush(mla_private_convert_color(path.stroke), &strokeBrush);
                    } else {
                        strokeBrush->SetColor(mla_private_convert_color(path.stroke));
                    }
                    renderTarget->DrawGeometry(geometry, strokeBrush, mla_s_cast<FLOAT>(path.stroke_width));
                }

                sink->Release();
                geometry->Release();
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT: {
                if (mla_string_is_empty(cmd.text.content)) {
                    break;
                }

                IDWriteTextFormat *textFormat =
                        mla_private_global_ui_surface_windows_direct2d_font_cache_getOrCreateTextFormat(
                            cache, cmd.text.font_type);

                if (textFormat != nullptr) {
                    ID2D1SolidColorBrush *solidBrush = mla_private_global_ui_surface_windows_direct2d_cache_getSolid_brush(
                        cache, renderTarget, mla_private_convert_color(cmd.text.fill));

                    mla_string_utf16_buffer_t contentWide = mla_string_to_utf16_buffer(cmd.text.content);

                    const mla_utf_16_char_t* contentWide_data = mla_string_utf16_data(contentWide);

                    if (contentWide_data == nullptr) {
                        break;
                    }

                    // Calculate text length
                    const WCHAR *textPtr = mla_r_cast<const WCHAR *>(contentWide_data);
                    UINT32 textLength = mla_s_cast<UINT32>(wcslen(textPtr));

                    renderTarget->DrawText(
                        textPtr,
                        textLength,
                        textFormat,
                        D2D1::RectF(mla_s_cast<FLOAT>(cmd.text.x), mla_s_cast<FLOAT>(cmd.text.y), 10000.0F, 10000.0F),
                        solidBrush
                    );
                }

                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINEAR_GRADIENT: {
                const auto &gradient = cmd.linear_gradient;

                if (currentLinearGradient != nullptr) {
                    currentLinearGradient->Release();
                    currentLinearGradient = nullptr;
                }

                D2D1_GRADIENT_STOP stops[2];
                stops[0].color = D2D1::ColorF(0, 0, 0, 1);
                stops[0].position = 0.0F;
                stops[1].color = D2D1::ColorF(1, 1, 1, 1);
                stops[1].position = 1.0F;

                ID2D1GradientStopCollection *stopCollection = nullptr;
                renderTarget->CreateGradientStopCollection(stops, 2, &stopCollection);

                if (stopCollection != nullptr) {
                    renderTarget->CreateLinearGradientBrush(
                        D2D1::LinearGradientBrushProperties(
                            D2D1::Point2F(mla_s_cast<FLOAT>(gradient.x1), mla_s_cast<FLOAT>(gradient.y1)),
                            D2D1::Point2F(mla_s_cast<FLOAT>(gradient.x2), mla_s_cast<FLOAT>(gradient.y2))
                        ),
                        stopCollection,
                        &currentLinearGradient
                    );
                    stopCollection->Release();
                }
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_RADIAL_GRADIENT: {
                const auto &gradient = cmd.radial_gradient;

                if (currentRadialGradient != nullptr) {
                    currentRadialGradient->Release();
                    currentRadialGradient = nullptr;
                }

                D2D1_GRADIENT_STOP stops[2];
                stops[0].color = D2D1::ColorF(1, 1, 1, 1);
                stops[0].position = 0.0F;
                stops[1].color = D2D1::ColorF(0, 0, 0, 1);
                stops[1].position = 1.0F;

                ID2D1GradientStopCollection *stopCollection = nullptr;
                renderTarget->CreateGradientStopCollection(stops, 2, &stopCollection);

                if (stopCollection != nullptr) {
                    renderTarget->CreateRadialGradientBrush(
                        D2D1::RadialGradientBrushProperties(
                            D2D1::Point2F(mla_s_cast<FLOAT>(gradient.cx), mla_s_cast<FLOAT>(gradient.cy)),
                            D2D1::Point2F(0, 0),
                            mla_s_cast<FLOAT>(gradient.r),
                            mla_s_cast<FLOAT>(gradient.r)
                        ),
                        stopCollection,
                        &currentRadialGradient
                    );
                    stopCollection->Release();
                }
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_STOP: {
                // Note: Direct2D gradient stops need to be set during gradient creation
                // This simplified implementation doesn't support dynamic stop updates
                break;
            }

            default:
                break;
        }
    }

    // Cleanup temporary brushes
    if (currentLinearGradient != nullptr) {
        currentLinearGradient->Release();
    }
    if (currentRadialGradient != nullptr) {
        currentRadialGradient->Release();
    }

    if (strokeBrush != nullptr) {
        strokeBrush->Release();
    }

#ifdef mla_debug_build
    DWORD current_time = GetTickCount();
    if (window_surface->DEBUG_last_fps_time == 0) {
        window_surface->DEBUG_last_fps_time = current_time;
    }

    window_surface->DEBUG_frames_accumulated++;

    if (current_time - window_surface->DEBUG_last_fps_time >= 1000) {
        window_surface->DEBUG_current_fps = window_surface->DEBUG_frames_accumulated;
        window_surface->DEBUG_frames_accumulated = 0;
        window_surface->DEBUG_last_fps_time = current_time;
    }

    mla_ui_surface_font_type_t debugFontType = mla_ui_surface_font_type_empty();
    debugFontType.family = mla_string_const("Arial");
    debugFontType.size = 15.0F;

    // Draw FPS counter
    IDWriteTextFormat *debugFont = mla_private_global_ui_surface_windows_direct2d_font_cache_getOrCreateTextFormat(
        cache, debugFontType);

    if (debugFont != nullptr) {
        ID2D1SolidColorBrush *debugBrush = nullptr;
        renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &debugBrush);

        char buffer[64];
        wsprintfA(buffer, "FPS: %d", window_surface->DEBUG_current_fps);

        WCHAR wBuffer[64];
        MultiByteToWideChar(CP_ACP, 0, buffer, -1, wBuffer, 64);

        D2D1_SIZE_F rtSize = renderTarget->GetSize();

        renderTarget->DrawText(
            wBuffer,
            mla_s_cast<UINT32>(wcslen(wBuffer)),
            debugFont,
            D2D1::RectF(rtSize.width - 75, rtSize.height - 25, rtSize.width, rtSize.height),
            debugBrush
        );

        if (debugBrush != nullptr) {
            debugBrush->Release();
        }
    }
#endif

    HRESULT hr = renderTarget->EndDraw();

    // Handle device lost
    if (hr == D2DERR_RECREATE_TARGET) {
        // Release resources because they are no longer valid
        window_surface->renderCache = mla_private_global_ui_surface_windows_direct2d_cache_empty();
        renderTarget->Release();
        window_surface->renderTarget = nullptr;

        return true; // Will recreate on next frame
    }

    return SUCCEEDED(hr);
}

void mla_private_windows_surface_buffer_cleanup(mla_windows_window_surface_t& window_surface) {


    // Cleanup Direct2D resources
    window_surface.renderCache = mla_private_global_ui_surface_windows_direct2d_cache_empty();

    if (window_surface.renderTarget != nullptr) {
        window_surface.renderTarget->Release();
        window_surface.renderTarget = nullptr;
    }

    if (IsWindow(window_surface.hwnd) == TRUE) {
        DestroyWindow(window_surface.hwnd);
    }
}

mla_bool_t mla_private_windows_create_surface(mla_ui_surface_t &outSurface) {

    mla_pointer_t window_surface_ptr = mla_malloc_struct_cleanup_hook(mla_windows_window_surface_t, mla_private_windows_surface_buffer_cleanup);

    mla_windows_window_surface_t *window_surface = mla_pointer_get_data<mla_windows_window_surface_t>(window_surface_ptr);

    if (window_surface == nullptr) {
        return false;
    }

    mla_memset(window_surface, 0, sizeof(mla_windows_window_surface_t));
    window_surface->is_initialized = false;
    window_surface->default_size = {0, 0};

#ifdef mla_debug_build
    window_surface->DEBUG_last_fps_time = 0;
    window_surface->DEBUG_frames_accumulated = 0;
    window_surface->DEBUG_current_fps = 0;
#endif

    outSurface.resource = window_surface_ptr;
    outSurface.get_size = mla_private_windows_surface_get_size;
    outSurface.set_size = mla_private_windows_surface_set_size;
    outSurface.render_draw_commands = mla_private_windows_surface_render_draw_commands;
    outSurface.calc_text_size = mla_private_windows_surface_calc_text_size;
    outSurface.get_input_states = mla_private_windows_surface_input_states;

    return true;
}

mla_ui_display_surface_low_level_access_t g_ui_display_surface_low_level_access = {
    mla_private_windows_create_surface
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Startup and Shutdown functions
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///

// Initialize Direct2D (call once at startup)
void mla_private_windows_d2d_init() {
    SetProcessDPIAware();

    if (g_pD2DFactory == nullptr) {
        D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_pD2DFactory);
    }
    if (g_pDWriteFactory == nullptr) {
        DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            mla_r_cast<IUnknown **>(&g_pDWriteFactory)
        );
    }
}

// Cleanup Direct2D (call at shutdown)
void mla_private_windows_d2d_shutdown() {
    if (g_pDWriteFactory != nullptr) {
        g_pDWriteFactory->Release();
        g_pDWriteFactory = nullptr;
    }
    if (g_pD2DFactory != nullptr) {
        g_pD2DFactory->Release();
        g_pD2DFactory = nullptr;
    }
}

struct MlaD2DAutoInit {
    MlaD2DAutoInit() { mla_private_windows_d2d_init(); }
    ~MlaD2DAutoInit() { mla_private_windows_d2d_shutdown(); }
};

// Single global instance
static MlaD2DAutoInit g_mlaD2DAutoInit;


#endif
