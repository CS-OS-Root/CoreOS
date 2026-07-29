//
// Created by chris on 1/30/2026.
//

#ifndef MLA_GLOBAL_UI_SURFACE_WINDOWS_ESP32_S3_H
#define MLA_GLOBAL_UI_SURFACE_WINDOWS_ESP32_S3_H

#include "../../core/ui/surfaces/mla_ui_surface.h"
#include "../../core/ui/display/mla_ui_display_surface.h"

#if defined(ESP32)
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_heap_caps.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include <esp_cache.h>
#endif

// Basic 5x7 font - simplified ASCII
static const unsigned char font5x7[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, // Space
    0x00, 0x00, 0x5F, 0x00, 0x00, // !
    0x00, 0x07, 0x00, 0x07, 0x00, // "
    0x14, 0x7F, 0x14, 0x7F, 0x14, // #
    0x24, 0x2A, 0x7F, 0x2A, 0x12, // $
    0x23, 0x13, 0x08, 0x64, 0x62, // %
    0x36, 0x49, 0x55, 0x22, 0x50, // &
    0x00, 0x05, 0x03, 0x00, 0x00, // '
    0x00, 0x1C, 0x22, 0x41, 0x00, // (
    0x00, 0x41, 0x22, 0x1C, 0x00, // )
    0x14, 0x08, 0x3E, 0x08, 0x14, // *
    0x08, 0x08, 0x3E, 0x08, 0x08, // +
    0x00, 0x50, 0x30, 0x00, 0x00, // ,
    0x08, 0x08, 0x08, 0x08, 0x08, // -
    0x00, 0x60, 0x60, 0x00, 0x00, // .
    0x20, 0x10, 0x08, 0x04, 0x02, // /
    0x3E, 0x51, 0x49, 0x45, 0x3E, // 0
    0x00, 0x42, 0x7F, 0x40, 0x00, // 1
    0x42, 0x61, 0x51, 0x49, 0x46, // 2
    0x21, 0x41, 0x45, 0x4B, 0x31, // 3
    0x18, 0x14, 0x12, 0x7F, 0x10, // 4
    0x27, 0x45, 0x45, 0x45, 0x39, // 5
    0x3C, 0x4A, 0x49, 0x49, 0x30, // 6
    0x01, 0x71, 0x09, 0x05, 0x03, // 7
    0x36, 0x49, 0x49, 0x49, 0x36, // 8
    0x06, 0x49, 0x49, 0x29, 0x1E, // 9
    0x00, 0x36, 0x36, 0x00, 0x00, // :
    0x00, 0x56, 0x36, 0x00, 0x00, // ;
    0x08, 0x14, 0x22, 0x41, 0x00, // <
    0x14, 0x14, 0x14, 0x14, 0x14, // =
    0x00, 0x41, 0x22, 0x14, 0x08, // >
    0x02, 0x01, 0x51, 0x09, 0x06, // ?
    0x32, 0x49, 0x79, 0x41, 0x3E, // @
    0x7E, 0x11, 0x11, 0x11, 0x7E, // A
    0x7F, 0x49, 0x49, 0x49, 0x36, // B
    0x3E, 0x41, 0x41, 0x41, 0x22, // C
    0x7F, 0x41, 0x41, 0x22, 0x1C, // D
    0x7F, 0x49, 0x49, 0x49, 0x41, // E
    0x7F, 0x09, 0x09, 0x09, 0x01, // F
    0x3E, 0x41, 0x49, 0x49, 0x7A, // G
    0x7F, 0x08, 0x08, 0x08, 0x7F, // H
    0x00, 0x41, 0x7F, 0x41, 0x00, // I
    0x20, 0x40, 0x41, 0x3F, 0x01, // J
    0x7F, 0x08, 0x14, 0x22, 0x41, // K
    0x7F, 0x40, 0x40, 0x40, 0x40, // L
    0x7F, 0x02, 0x0C, 0x02, 0x7F, // M
    0x7F, 0x04, 0x08, 0x10, 0x7F, // N
    0x3E, 0x41, 0x41, 0x41, 0x3E, // O
    0x7F, 0x09, 0x09, 0x09, 0x06, // P
    0x3E, 0x41, 0x51, 0x21, 0x5E, // Q
    0x7F, 0x09, 0x19, 0x29, 0x46, // R
    0x46, 0x49, 0x49, 0x49, 0x31, // S
    0x01, 0x01, 0x7F, 0x01, 0x01, // T
    0x3F, 0x40, 0x40, 0x40, 0x3F, // U
    0x1F, 0x20, 0x40, 0x20, 0x1F, // V
    0x3F, 0x40, 0x38, 0x40, 0x3F, // W
    0x63, 0x14, 0x08, 0x14, 0x63, // X
    0x07, 0x08, 0x70, 0x08, 0x07, // Y
    0x61, 0x51, 0x49, 0x45, 0x43, // Z
    0x00, 0x7F, 0x41, 0x41, 0x00, // [
    0x02, 0x04, 0x08, 0x10, 0x20, // (backslash)
    0x00, 0x41, 0x41, 0x7F, 0x00, // ]
    0x04, 0x02, 0x01, 0x02, 0x04, // ^
    0x40, 0x40, 0x40, 0x40, 0x40, // _
    0x00, 0x01, 0x02, 0x04, 0x00, // `
    0x20, 0x54, 0x54, 0x54, 0x78, // a
    0x7F, 0x48, 0x44, 0x44, 0x38, // b
    0x38, 0x44, 0x44, 0x44, 0x20, // c
    0x38, 0x44, 0x44, 0x48, 0x7F, // d
    0x38, 0x54, 0x54, 0x54, 0x18, // e
    0x08, 0x7E, 0x09, 0x01, 0x02, // f
    0x0C, 0x52, 0x52, 0x52, 0x3E, // g
    0x7F, 0x08, 0x04, 0x04, 0x78, // h
    0x00, 0x44, 0x7D, 0x40, 0x00, // i
    0x20, 0x40, 0x44, 0x3D, 0x00, // j
    0x7F, 0x10, 0x28, 0x44, 0x00, // k
    0x00, 0x41, 0x7F, 0x40, 0x00, // l
    0x7C, 0x04, 0x18, 0x04, 0x78, // m
    0x7C, 0x08, 0x04, 0x04, 0x78, // n
    0x38, 0x44, 0x44, 0x44, 0x38, // o
    0x7C, 0x14, 0x14, 0x14, 0x08, // p
    0x08, 0x14, 0x14, 0x18, 0x7C, // q
    0x7C, 0x08, 0x04, 0x04, 0x08, // r
    0x48, 0x54, 0x54, 0x54, 0x20, // s
    0x04, 0x3F, 0x44, 0x40, 0x20, // t
    0x3C, 0x40, 0x40, 0x20, 0x7C, // u
    0x1C, 0x20, 0x40, 0x20, 0x1C, // v
    0x3C, 0x40, 0x30, 0x40, 0x3C, // w
    0x44, 0x28, 0x10, 0x28, 0x44, // x
    0x0C, 0x50, 0x50, 0x50, 0x3C, // y
    0x44, 0x64, 0x54, 0x4C, 0x44, // z
    0x00, 0x08, 0x36, 0x41, 0x00, // {
    0x00, 0x00, 0x7F, 0x00, 0x00, // |
    0x00, 0x41, 0x36, 0x08, 0x00, // }
    0x08, 0x08, 0x2A, 0x1C, 0x08  // ->
};

struct mla_esp32_software_renderer_t {
    uint16_t* framebuffer;
    int width;
    int height;
};

static uint16_t mla_esp32_software_renderer_color565(const mla_ui_surface_draw_command_color_t &c) {
    return ((c.r & 0xF8) << 8) | ((c.g & 0xFC) << 3) | (c.b >> 3);
}

static void mla_esp32_software_renderer_draw_pixel(mla_esp32_software_renderer_t* r, int x, int y, uint16_t color) {
    if (x < 0 || x >= r->width || y < 0 || y >= r->height) return;
    r->framebuffer[y * r->width + x] = color;
}

static void mla_esp32_software_renderer_fill_rect(mla_esp32_software_renderer_t* r, int x, int y, int w, int h, uint16_t color) {
    if (x >= r->width || y >= r->height) return;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > r->width) w = r->width - x;
    if (y + h > r->height) h = r->height - y;
    if (w <= 0 || h <= 0) return;

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            r->framebuffer[(y + i) * r->width + (x + j)] = color;
        }
    }
}

static void mla_esp32_software_renderer_draw_rect(mla_esp32_software_renderer_t* r, int x, int y, int w, int h, uint16_t color, int stroke_width) {
    // Simplified stroke
    for(int i=0; i<stroke_width; i++) {
            // Top
            mla_esp32_software_renderer_fill_rect(r, x, y+i, w, 1, color);
            // Bottom
            mla_esp32_software_renderer_fill_rect(r, x, y+h-1-i, w, 1, color);
            // Left
            mla_esp32_software_renderer_fill_rect(r, x+i, y, 1, h, color);
            // Right
            mla_esp32_software_renderer_fill_rect(r, x+w-1-i, y, 1, h, color);
    }
}

static void mla_esp32_software_renderer_draw_line(mla_esp32_software_renderer_t* r, int x0, int y0, int x1, int y1, uint16_t color, int stroke_width) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    for (;;) {
        if(stroke_width == 1)
            mla_esp32_software_renderer_draw_pixel(r, x0, y0, color);
        else
            mla_esp32_software_renderer_fill_rect(r, x0 - stroke_width/2, y0 - stroke_width/2, stroke_width, stroke_width, color);

        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

static void mla_esp32_software_renderer_draw_circle(mla_esp32_software_renderer_t* r, int cx, int cy, int rad, uint16_t color, bool fill) {
    int x = -rad, y = 0, err = 2 - 2 * rad;
    do {
        if (fill) {
            mla_esp32_software_renderer_draw_line(r, cx + x, cy - y, cx - x, cy - y, color, 1);
            mla_esp32_software_renderer_draw_line(r, cx + x, cy + y, cx - x, cy + y, color, 1);
        } else {
            mla_esp32_software_renderer_draw_pixel(r, cx - x, cy + y, color);
            mla_esp32_software_renderer_draw_pixel(r, cx - y, cy - x, color);
            mla_esp32_software_renderer_draw_pixel(r, cx + x, cy - y, color);
            mla_esp32_software_renderer_draw_pixel(r, cx + y, cy + x, color);
        }
        rad = err;
        if (rad <= y) err += ++y * 2 + 1;
        if (rad > x || err > y) err += ++x * 2 + 1;
    } while (x < 0);
}

static void mla_esp32_software_renderer_draw_ellipse(mla_esp32_software_renderer_t* r, int cx, int cy, int rx, int ry, uint16_t color)
{
    // Simple ellipse approximation or bounding box for now if complex
    // This is a minimal implementation placeholder
    mla_esp32_software_renderer_draw_circle(r, cx, cy, (rx+ry)/2, color, false);
}

static void mla_esp32_software_renderer_draw_char(mla_esp32_software_renderer_t* r, int x, int y, char c, uint16_t color, int size) {
    if (c < 32 || c > 127) return;
    const unsigned char* bitmap = font5x7 + (c - 32) * 5;
    for (int i = 0; i < 5; i++) {
        unsigned char line = bitmap[i];
        for (int j = 0; j < 8; j++) {
            if (line & 0x01) {
                if (size == 1)
                    mla_esp32_software_renderer_draw_pixel(r, x + i, y + j, color);
                else
                    mla_esp32_software_renderer_fill_rect(r, x + i*size, y + j*size, size, size, color);
            }
            line >>= 1;
        }
    }
}

static void mla_esp32_software_renderer_draw_string(mla_esp32_software_renderer_t* r, int x, int y, const char* str, uint16_t color, int size) {
    while (*str) {
        mla_esp32_software_renderer_draw_char(r, x, y, *str, color, size);
        x += 6 * size;
        str++;
    }
}

static void mla_esp32_software_renderer_clear(mla_esp32_software_renderer_t* r, uint16_t color) {
    for (int i = 0; i < r->width * r->height; i++) {
        r->framebuffer[i] = color;
    }
}


// ESP32-S3-4848S040 Pinout
#define LCD_PIXEL_CLOCK_HZ     (10 * 1000 * 1000)
#define LCD_BK_LIGHT           38
#define LCD_H_RES              480
#define LCD_V_RES              480

#define LCD_PIN_HSYNC          42
#define LCD_PIN_VSYNC          41
#define LCD_PIN_DE             40
#define LCD_PIN_PCLK           39
#define LCD_PIN_DATA0          45
#define LCD_PIN_DATA1          48
#define LCD_PIN_DATA2          47
#define LCD_PIN_DATA3          21
#define LCD_PIN_DATA4          14
#define LCD_PIN_DATA5          5
#define LCD_PIN_DATA6          6
#define LCD_PIN_DATA7          7
#define LCD_PIN_DATA8          15
#define LCD_PIN_DATA9          16
#define LCD_PIN_DATA10         4
#define LCD_PIN_DATA11         8
#define LCD_PIN_DATA12         3
#define LCD_PIN_DATA13         46
#define LCD_PIN_DATA14         9
#define LCD_PIN_DATA15         1

// SPI Init Pins (Shared)
#define LCD_SPI_CS             39
#define LCD_SPI_SDA            8
#define LCD_SPI_SCL            9

#if defined(ESP32)

static void st7701_spi_write_cmd(int cs, int sck, int sda, uint8_t cmd) {
    gpio_set_level((gpio_num_t)cs, 0);
    // 3-wire 9-bit SPI: 0 + 8bit cmd
    gpio_set_level((gpio_num_t)sck, 0);
    gpio_set_level((gpio_num_t)sda, 0); // Command bit = 0
    gpio_set_level((gpio_num_t)sck, 1);

    for (int i = 0; i < 8; i++) {
        gpio_set_level((gpio_num_t)sck, 0);
        if (cmd & 0x80) gpio_set_level((gpio_num_t)sda, 1);
        else            gpio_set_level((gpio_num_t)sda, 0);
        gpio_set_level((gpio_num_t)sck, 1);
        cmd <<= 1;
    }
    gpio_set_level((gpio_num_t)cs, 1);
}

static void st7701_spi_write_data(int cs, int sck, int sda, uint8_t data) {
    gpio_set_level((gpio_num_t)cs, 0);
    // 3-wire 9-bit SPI: 1 + 8bit data
    gpio_set_level((gpio_num_t)sck, 0);
    gpio_set_level((gpio_num_t)sda, 1); // Data bit = 1
    gpio_set_level((gpio_num_t)sck, 1);

    for (int i = 0; i < 8; i++) {
        gpio_set_level((gpio_num_t)sck, 0);
        if (data & 0x80) gpio_set_level((gpio_num_t)sda, 1);
        else            gpio_set_level((gpio_num_t)sda, 0);
        gpio_set_level((gpio_num_t)sck, 1);
        data <<= 1;
    }
    gpio_set_level((gpio_num_t)cs, 1);
}

static void st7701_init_sequence() {
    int cs = LCD_SPI_CS;
    int sck = LCD_SPI_SCL;
    int sda = LCD_SPI_SDA;

    // Config GPIOs for Bitbang
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << cs) | (1ULL << sck) | (1ULL << sda);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    gpio_set_level((gpio_num_t)cs, 1);
    gpio_set_level((gpio_num_t)sck, 1);
    gpio_set_level((gpio_num_t)sda, 1);

    mla_sleep(120);

    // Commands
    st7701_spi_write_cmd(cs, sck, sda, 0xFF); st7701_spi_write_data(cs, sck, sda, 0x77); st7701_spi_write_data(cs, sck, sda, 0x01); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x13);
    st7701_spi_write_cmd(cs, sck, sda, 0xEF); st7701_spi_write_data(cs, sck, sda, 0x08);
    st7701_spi_write_cmd(cs, sck, sda, 0xFF); st7701_spi_write_data(cs, sck, sda, 0x77); st7701_spi_write_data(cs, sck, sda, 0x01); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x10);
    st7701_spi_write_cmd(cs, sck, sda, 0xC0); st7701_spi_write_data(cs, sck, sda, 0x3B); st7701_spi_write_data(cs, sck, sda, 0x00);
    st7701_spi_write_cmd(cs, sck, sda, 0xC1); st7701_spi_write_data(cs, sck, sda, 0x0D); st7701_spi_write_data(cs, sck, sda, 0x02);
    st7701_spi_write_cmd(cs, sck, sda, 0xC2); st7701_spi_write_data(cs, sck, sda, 0x21); st7701_spi_write_data(cs, sck, sda, 0x08);
    st7701_spi_write_cmd(cs, sck, sda, 0xCC); st7701_spi_write_data(cs, sck, sda, 0x10);
    st7701_spi_write_cmd(cs, sck, sda, 0xB0); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x05); st7701_spi_write_data(cs, sck, sda, 0x0F); st7701_spi_write_data(cs, sck, sda, 0x0D); st7701_spi_write_data(cs, sck, sda, 0x13); st7701_spi_write_data(cs, sck, sda, 0x07); st7701_spi_write_data(cs, sck, sda, 0x01); st7701_spi_write_data(cs, sck, sda, 0x08); st7701_spi_write_data(cs, sck, sda, 0x09); st7701_spi_write_data(cs, sck, sda, 0x1E); st7701_spi_write_data(cs, sck, sda, 0x04); st7701_spi_write_data(cs, sck, sda, 0x12); st7701_spi_write_data(cs, sck, sda, 0x10); st7701_spi_write_data(cs, sck, sda, 0xA7); st7701_spi_write_data(cs, sck, sda, 0x2F); st7701_spi_write_data(cs, sck, sda, 0x18);
    st7701_spi_write_cmd(cs, sck, sda, 0xB1); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x0F); st7701_spi_write_data(cs, sck, sda, 0x17); st7701_spi_write_data(cs, sck, sda, 0x0C); st7701_spi_write_data(cs, sck, sda, 0x11); st7701_spi_write_data(cs, sck, sda, 0x06); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x08); st7701_spi_write_data(cs, sck, sda, 0x09); st7701_spi_write_data(cs, sck, sda, 0x1E); st7701_spi_write_data(cs, sck, sda, 0x03); st7701_spi_write_data(cs, sck, sda, 0x11); st7701_spi_write_data(cs, sck, sda, 0x10); st7701_spi_write_data(cs, sck, sda, 0xAA); st7701_spi_write_data(cs, sck, sda, 0x2F); st7701_spi_write_data(cs, sck, sda, 0x18);
    st7701_spi_write_cmd(cs, sck, sda, 0xFF); st7701_spi_write_data(cs, sck, sda, 0x77); st7701_spi_write_data(cs, sck, sda, 0x01); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x11);
    st7701_spi_write_cmd(cs, sck, sda, 0xB0); st7701_spi_write_data(cs, sck, sda, 0x4D);
    st7701_spi_write_cmd(cs, sck, sda, 0xB1); st7701_spi_write_data(cs, sck, sda, 0x4D); // Vcom
    st7701_spi_write_cmd(cs, sck, sda, 0xB2); st7701_spi_write_data(cs, sck, sda, 0x87);
    st7701_spi_write_cmd(cs, sck, sda, 0xB3); st7701_spi_write_data(cs, sck, sda, 0x80);
    st7701_spi_write_cmd(cs, sck, sda, 0xB5); st7701_spi_write_data(cs, sck, sda, 0x47);
    st7701_spi_write_cmd(cs, sck, sda, 0xB7); st7701_spi_write_data(cs, sck, sda, 0x85);
    st7701_spi_write_cmd(cs, sck, sda, 0xB8); st7701_spi_write_data(cs, sck, sda, 0x20);
    st7701_spi_write_cmd(cs, sck, sda, 0xC1); st7701_spi_write_data(cs, sck, sda, 0x78);
    st7701_spi_write_cmd(cs, sck, sda, 0xC2); st7701_spi_write_data(cs, sck, sda, 0x78);
    st7701_spi_write_cmd(cs, sck, sda, 0xD0); st7701_spi_write_data(cs, sck, sda, 0x88);
    st7701_spi_write_cmd(cs, sck, sda, 0xE0); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x02);
    st7701_spi_write_cmd(cs, sck, sda, 0xE1); st7701_spi_write_data(cs, sck, sda, 0x08); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x0A); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x07); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x09); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x33); st7701_spi_write_data(cs, sck, sda, 0x33);
    st7701_spi_write_cmd(cs, sck, sda, 0xE2); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x00);
    st7701_spi_write_cmd(cs, sck, sda, 0xE3); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x33); st7701_spi_write_data(cs, sck, sda, 0x33);
    st7701_spi_write_cmd(cs, sck, sda, 0xE4); st7701_spi_write_data(cs, sck, sda, 0x44); st7701_spi_write_data(cs, sck, sda, 0x44);
    st7701_spi_write_cmd(cs, sck, sda, 0xE5); st7701_spi_write_data(cs, sck, sda, 0x0E); st7701_spi_write_data(cs, sck, sda, 0x2D); st7701_spi_write_data(cs, sck, sda, 0xA0); st7701_spi_write_data(cs, sck, sda, 0xA0); st7701_spi_write_data(cs, sck, sda, 0x10); st7701_spi_write_data(cs, sck, sda, 0x2D); st7701_spi_write_data(cs, sck, sda, 0xA0); st7701_spi_write_data(cs, sck, sda, 0xA0); st7701_spi_write_data(cs, sck, sda, 0x0A); st7701_spi_write_data(cs, sck, sda, 0x2D); st7701_spi_write_data(cs, sck, sda, 0xA0); st7701_spi_write_data(cs, sck, sda, 0xA0); st7701_spi_write_data(cs, sck, sda, 0x0C); st7701_spi_write_data(cs, sck, sda, 0x2D); st7701_spi_write_data(cs, sck, sda, 0xA0); st7701_spi_write_data(cs, sck, sda, 0xA0);
    st7701_spi_write_cmd(cs, sck, sda, 0xE6); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x33); st7701_spi_write_data(cs, sck, sda, 0x33);
    st7701_spi_write_cmd(cs, sck, sda, 0xE7); st7701_spi_write_data(cs, sck, sda, 0x44); st7701_spi_write_data(cs, sck, sda, 0x44);
    st7701_spi_write_cmd(cs, sck, sda, 0xE8); st7701_spi_write_data(cs, sck, sda, 0x0D); st7701_spi_write_data(cs, sck, sda, 0x2D); st7701_spi_write_data(cs, sck, sda, 0xA0); st7701_spi_write_data(cs, sck, sda, 0xA0); st7701_spi_write_data(cs, sck, sda, 0x0F); st7701_spi_write_data(cs, sck, sda, 0x2D); st7701_spi_write_data(cs, sck, sda, 0xA0); st7701_spi_write_data(cs, sck, sda, 0xA0); st7701_spi_write_data(cs, sck, sda, 0x09); st7701_spi_write_data(cs, sck, sda, 0x2D); st7701_spi_write_data(cs, sck, sda, 0xA0); st7701_spi_write_data(cs, sck, sda, 0xA0); st7701_spi_write_data(cs, sck, sda, 0x0B); st7701_spi_write_data(cs, sck, sda, 0x2D); st7701_spi_write_data(cs, sck, sda, 0xA0); st7701_spi_write_data(cs, sck, sda, 0xA0);
    st7701_spi_write_cmd(cs, sck, sda, 0xEB); st7701_spi_write_data(cs, sck, sda, 0x02); st7701_spi_write_data(cs, sck, sda, 0x01); st7701_spi_write_data(cs, sck, sda, 0xE4); st7701_spi_write_data(cs, sck, sda, 0xE4); st7701_spi_write_data(cs, sck, sda, 0x44); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x40);
    st7701_spi_write_cmd(cs, sck, sda, 0xEC); st7701_spi_write_data(cs, sck, sda, 0x02); st7701_spi_write_data(cs, sck, sda, 0x01);
    st7701_spi_write_cmd(cs, sck, sda, 0xED); st7701_spi_write_data(cs, sck, sda, 0xAB); st7701_spi_write_data(cs, sck, sda, 0x89); st7701_spi_write_data(cs, sck, sda, 0x76); st7701_spi_write_data(cs, sck, sda, 0x54); st7701_spi_write_data(cs, sck, sda, 0x01); st7701_spi_write_data(cs, sck, sda, 0xFF); st7701_spi_write_data(cs, sck, sda, 0xFF); st7701_spi_write_data(cs, sck, sda, 0xFF); st7701_spi_write_data(cs, sck, sda, 0xFF); st7701_spi_write_data(cs, sck, sda, 0xFF); st7701_spi_write_data(cs, sck, sda, 0xFF); st7701_spi_write_data(cs, sck, sda, 0x10); st7701_spi_write_data(cs, sck, sda, 0x45); st7701_spi_write_data(cs, sck, sda, 0x67); st7701_spi_write_data(cs, sck, sda, 0x98); st7701_spi_write_data(cs, sck, sda, 0xBA);
    st7701_spi_write_cmd(cs, sck, sda, 0xFF); st7701_spi_write_data(cs, sck, sda, 0x77); st7701_spi_write_data(cs, sck, sda, 0x01); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x00); st7701_spi_write_data(cs, sck, sda, 0x00);
    st7701_spi_write_cmd(cs, sck, sda, 0x11); // Sleep Out
    vTaskDelay(pdMS_TO_TICKS(120));
    st7701_spi_write_cmd(cs, sck, sda, 0x29); // Display On
}

#endif


struct mla_esp32_surface_t {
    mla_platform_pointer_t panel;
    uint16_t *frame_buffer;
    mla_esp32_software_renderer_t renderer;
    mla_ui_surface_size_t size;
};

// Cleanup function
inline mla_buffer_cleanup_mode mla_private_esp32_surface_cleanup(mla_platform_pointer_t data, mla_callback_userdata userData) {
    (void) userData;
    mla_esp32_surface_t *surface = (mla_esp32_surface_t *) data;
    if (surface) {
        if (surface->panel) {
            surface->panel = nullptr;
        }

        if (surface->frame_buffer) {
            surface->frame_buffer = nullptr;
        }

        // panel handle cleanup if needed, usually we don't deinit display in embedded
    }
    return CLEAN_UP_NEEDED;
}

inline mla_ui_surface_size_t mla_private_esp32_surface_get_size(const mla_ui_surface_t &surface) {
     mla_esp32_surface_t *esp_surface = (mla_esp32_surface_t *) surface.resource;
     if(!esp_surface) return {0,0};
     return esp_surface->size;
}

inline mla_bool_t mla_private_esp32_surface_set_size(const mla_ui_surface_t &surface, mla_ui_surface_size_t size) {
    (void) surface; (void) size;
    return false; // Cannot resize physical display
}

inline mla_ui_surface_draw_size_t mla_private_esp32_surface_calc_text_size(const mla_ui_surface_t &surface, const mla_ui_surface_font_type_t &font_type, const mla_string_t &text) {
     (void) surface; (void) font_type;
     // Basic 5x7 font calculation
     mla_ui_surface_draw_size_t size;
     size.height = 7;
     // We treat font size as scale factor in our simple renderer
     if(font_type.size > 0) size.height = 7.0 * (font_type.size / 10.0); // Rough scaling
     if(size.height < 7) size.height = 7;

     // Access string data for length
     mla_size_t len = mla_string_length(text);

     size.width = (double)(len * 6 * (size.height / 7.0));
     return size;
}

inline mla_ui_surface_input_states_t mla_private_esp32_surface_get_input_states(const mla_ui_surface_t &surface) {
    (void) surface;
    // TODO: Implement GT911 touch driver reading
    return mla_ui_surface_input_states_empty();
}

inline mla_bool_t mla_private_esp32_surface_render_draw_commands(const mla_ui_surface_t& surface, const mla_array_list_t<mla_init_struct(mla_ui_surface_draw_command_t)>& drawCommands, mla_array_list_t<mla_init_struct(mla_ui_surface_input_event_t)>& eventsSinceLastFame) {
    (void) eventsSinceLastFame;
    mla_esp32_surface_t *esp_surface = (mla_esp32_surface_t *) surface.resource;
    if (!esp_surface)
        return false;

    ESP_LOGI("LCD", "Rendering %d draw commands", (int)mla_array_list_size(drawCommands));

    mla_esp32_software_renderer_t *r = &esp_surface->renderer;

    // Clear background if needed? Or we assume commands cover it.
    // Let's iterate commands
    for (mla_size_t i = 0; i < mla_array_list_size(drawCommands); i++) {
        const mla_ui_surface_draw_command_t &cmd = mla_array_list_get_unsafe(drawCommands, i);

        switch (cmd.kind) {
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT: {
                uint16_t color = mla_esp32_software_renderer_color565(cmd.rect.color);
                // Fill
                mla_esp32_software_renderer_fill_rect(r, (int)cmd.rect.x, (int)cmd.rect.y, (int)cmd.rect.width, (int)cmd.rect.height, color);
                // Stroke
                if(cmd.rect.stroke_width > 0) {
                     uint16_t strokeColor = mla_esp32_software_renderer_color565(cmd.rect.stroke);
                     mla_esp32_software_renderer_draw_rect(r, (int)cmd.rect.x, (int)cmd.rect.y, (int)cmd.rect.width, (int)cmd.rect.height, strokeColor, (int)cmd.rect.stroke_width);
                }
                break;
            }
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_CIRCLE: {
                uint16_t color = mla_esp32_software_renderer_color565(cmd.circle.fill);
                // Fill
                mla_esp32_software_renderer_draw_circle(r, (int)cmd.circle.cx, (int)cmd.circle.cy, (int)cmd.circle.r, color, true);
                // Stroke
                 if(cmd.circle.stroke_width > 0) {
                     uint16_t strokeColor = mla_esp32_software_renderer_color565(cmd.circle.stroke);
                     mla_esp32_software_renderer_draw_circle(r, (int)cmd.circle.cx, (int)cmd.circle.cy, (int)cmd.circle.r, strokeColor, false);
                }
                break;
            }
             case MLA_UI_SURFACE_DRAW_COMMAND_KIND_ELLIPSE: {
                uint16_t color = mla_esp32_software_renderer_color565(cmd.ellipse.fill);
                mla_esp32_software_renderer_draw_ellipse(r, (int)cmd.ellipse.cx, (int)cmd.ellipse.cy, (int)cmd.ellipse.rx, (int)cmd.ellipse.ry, color);
                break;
            }
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINE: {
                 uint16_t color = mla_esp32_software_renderer_color565(cmd.line.stroke);
                 mla_esp32_software_renderer_draw_line(r, (int)cmd.line.x1, (int)cmd.line.y1, (int)cmd.line.x2, (int)cmd.line.y2, color, (int)(cmd.line.stroke_width > 0 ? cmd.line.stroke_width : 1));
                 break;
            }
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT: {
                 uint16_t color = mla_esp32_software_renderer_color565(cmd.text.fill);

                 const char* str_data = mla_string_data(cmd.text.content);

                 if(str_data == nullptr) break;

                 // Rough font size handling
                 int scale = 1;
                 if(cmd.text.font_type.size > 12) scale = 2;
                 if(cmd.text.font_type.size > 24) scale = 3;

                 mla_esp32_software_renderer_draw_string(r, (int)cmd.text.x, (int)cmd.text.y, str_data, color, scale);
                 break;
            }
            default:
                break;
        }
    }

    // Push framebuffer to display
    esp_lcd_panel_handle_t panel = (esp_lcd_panel_handle_t)(esp_surface->panel);
    if (panel) {
        size_t buffer_size = LCD_H_RES * LCD_V_RES * sizeof(uint16_t);
        esp_cache_msync((void *)r->framebuffer, buffer_size, ESP_CACHE_MSYNC_FLAG_DIR_C2M);
        esp_err_t err = esp_lcd_panel_draw_bitmap(panel, 0, 0, LCD_H_RES, LCD_V_RES, (void *)r->framebuffer);
        if (err != ESP_OK) {
            ESP_LOGE("LCD", "Failed to draw bitmap to panel: %s", esp_err_to_name(err));
            return false;
        }
    }

    return true;
}

static bool s_is_initialized = false;
static mla_esp32_surface_t *s_global_surface = nullptr;

inline mla_bool_t mla_private_esp32_create_surface(mla_ui_surface_t &outSurface) {
    if (s_is_initialized && s_global_surface) {
         // Return existing
         outSurface.resource = s_global_surface;
         outSurface.resourceOwner = mla_buffer_reference(s_global_surface, false, mla_private_esp32_surface_cleanup);
         outSurface.get_size = mla_private_esp32_surface_get_size;
         outSurface.set_size = mla_private_esp32_surface_set_size;
         outSurface.render_draw_commands = mla_private_esp32_surface_render_draw_commands;
         outSurface.calc_text_size = mla_private_esp32_surface_calc_text_size;
         outSurface.get_input_states = mla_private_esp32_surface_get_input_states;
         return true;
    }

#if defined(ESP32)
    // 1. Initialize ST7701 via SPI
    ESP_LOGI("LCD", "Initializing ST7701...");
    st7701_init_sequence();

    // 2. Initialize RGB Panel
    ESP_LOGI("LCD", "Initializing RGB Panel...");

    // Check for PSRAM
    size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    ESP_LOGI("LCD", "Free PSRAM: %d bytes", (int)free_psram);
    if (free_psram < (LCD_H_RES * LCD_V_RES * 2)) {
        ESP_LOGE("LCD", "Insufficient PSRAM for Framebuffer! Required: %d, Available: %d", (LCD_H_RES * LCD_V_RES * 2), (int)free_psram);
        ESP_LOGE("LCD", "Please ensure PSRAM is enabled in sdkconfig (CONFIG_SPIRAM=y)");
    }

    // Enable Backlight
    gpio_config_t bk_conf = {};
    bk_conf.pin_bit_mask = (1ULL << LCD_BK_LIGHT);
    bk_conf.mode = GPIO_MODE_OUTPUT;
    bk_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    bk_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    bk_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&bk_conf);
    gpio_set_level((gpio_num_t)LCD_BK_LIGHT, 1);

    esp_lcd_rgb_panel_config_t panel_config = {};
    panel_config.clk_src = LCD_CLK_SRC_PLL160M;
    panel_config.timings.pclk_hz = LCD_PIXEL_CLOCK_HZ;
    panel_config.timings.h_res = LCD_H_RES;
    panel_config.timings.v_res = LCD_V_RES;
    panel_config.timings.hsync_pulse_width = 10;
    panel_config.timings.hsync_back_porch = 50;
    panel_config.timings.hsync_front_porch = 50;
    panel_config.timings.vsync_pulse_width = 10;
    panel_config.timings.vsync_back_porch = 20;
    panel_config.timings.vsync_front_porch = 20;
    panel_config.timings.flags.pclk_active_neg = true;

    // IMPORTANT: Set bounce buffer to avoid large internal RAM allocation
    panel_config.bounce_buffer_size_px = LCD_H_RES * 10;

    panel_config.data_width = 16;
    panel_config.psram_trans_align = 64;
    panel_config.hsync_gpio_num = (gpio_num_t)LCD_PIN_HSYNC;
    panel_config.vsync_gpio_num = (gpio_num_t)LCD_PIN_VSYNC;
    panel_config.de_gpio_num = (gpio_num_t)LCD_PIN_DE;
    panel_config.pclk_gpio_num = (gpio_num_t)LCD_PIN_PCLK;

    panel_config.data_gpio_nums[0] = (gpio_num_t)LCD_PIN_DATA0;
    panel_config.data_gpio_nums[1] = (gpio_num_t)LCD_PIN_DATA1;
    panel_config.data_gpio_nums[2] = (gpio_num_t)LCD_PIN_DATA2;
    panel_config.data_gpio_nums[3] = (gpio_num_t)LCD_PIN_DATA3;
    panel_config.data_gpio_nums[4] = (gpio_num_t)LCD_PIN_DATA4;
    panel_config.data_gpio_nums[5] = (gpio_num_t)LCD_PIN_DATA5;
    panel_config.data_gpio_nums[6] = (gpio_num_t)LCD_PIN_DATA6;
    panel_config.data_gpio_nums[7] = (gpio_num_t)LCD_PIN_DATA7;
    panel_config.data_gpio_nums[8] = (gpio_num_t)LCD_PIN_DATA8;
    panel_config.data_gpio_nums[9] = (gpio_num_t)LCD_PIN_DATA9;
    panel_config.data_gpio_nums[10] = (gpio_num_t)LCD_PIN_DATA10;
    panel_config.data_gpio_nums[11] = (gpio_num_t)LCD_PIN_DATA11;
    panel_config.data_gpio_nums[12] = (gpio_num_t)LCD_PIN_DATA12;
    panel_config.data_gpio_nums[13] = (gpio_num_t)LCD_PIN_DATA13;
    panel_config.data_gpio_nums[14] = (gpio_num_t)LCD_PIN_DATA14;
    panel_config.data_gpio_nums[15] = (gpio_num_t)LCD_PIN_DATA15;

    panel_config.disp_gpio_num = GPIO_NUM_NC;
    panel_config.num_fbs = 1;
    panel_config.flags.fb_in_psram = 1;

    ESP_LOGI("LCD", "Creating RGB Panel...");

    esp_lcd_panel_handle_t panel_handle = nullptr;
    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    ESP_LOGI("LCD", "RGB Panel initialized.");

    void *fb = nullptr;
    ESP_ERROR_CHECK(esp_lcd_rgb_panel_get_frame_buffer(panel_handle, 1, &fb));

    mla_esp32_surface_t *esp_surface = (mla_esp32_surface_t *) mla_platform_malloc(sizeof(mla_esp32_surface_t));

    if (esp_surface == nullptr) {
        ESP_LOGE("LCD", "Failed to allocate memory for esp32 surface.");
        return false;
    }

    esp_surface->panel = (mla_platform_pointer_t)panel_handle;
    esp_surface->frame_buffer = (uint16_t*)fb;
    esp_surface->size = {LCD_H_RES, LCD_V_RES};
    esp_surface->renderer = {(uint16_t*)fb, LCD_H_RES, LCD_V_RES};

    // Initial clear
    mla_esp32_software_renderer_clear(&esp_surface->renderer, 0x0000);

    outSurface.resource = esp_surface;
    outSurface.resourceOwner = mla_buffer_reference(esp_surface, false, mla_private_esp32_surface_cleanup);
    outSurface.get_size = mla_private_esp32_surface_get_size;
    outSurface.set_size = mla_private_esp32_surface_set_size;
    outSurface.render_draw_commands = mla_private_esp32_surface_render_draw_commands;
    outSurface.calc_text_size = mla_private_esp32_surface_calc_text_size;
    outSurface.get_input_states = mla_private_esp32_surface_get_input_states;

    s_global_surface = esp_surface;
    s_is_initialized = true;

    ESP_LOGI("LCD", "Surface created successfully.");

    return true;
#else
    return false;
#endif
}

mla_ui_display_surface_low_level_access_t g_ui_display_surface_low_level_access = {
    mla_private_esp32_create_surface
};

#endif
