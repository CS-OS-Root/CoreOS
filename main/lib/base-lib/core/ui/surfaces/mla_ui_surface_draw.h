//
// Created by chris on 1/22/2026.
//

#ifndef MLA_UI_SURFACE_DRAW_H
#define MLA_UI_SURFACE_DRAW_H

#include "../../mla_data_types.h"
#include "../../system/mla_string.h"
#include "../../system/mla_array_list.h"
#include "../../serializer/mla_serializer.h"

struct mla_ui_surface_draw_command_color_t {
    mla_uint8_t r;
    mla_uint8_t g;
    mla_uint8_t b;
    mla_uint8_t a;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_ui_surface_draw_command_color_t* self = mla_pointer_get_data<const mla_ui_surface_draw_command_color_t>(obj);

        if (self == nullptr) {
            return false;
        }

        mla_serializer_write_uint8(serializer, mla_string_const("r"), self->r);
        mla_serializer_write_uint8(serializer, mla_string_const("g"), self->g);
        mla_serializer_write_uint8(serializer, mla_string_const("b"), self->b);
        mla_serializer_write_uint8(serializer, mla_string_const("a"), self->a);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_ui_surface_draw_command_color_t* self = mla_pointer_get_data<mla_ui_surface_draw_command_color_t>(obj);

        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "r")) {
            mla_deserializer_read_uint8(deserializer, self->r);
        } else if (mla_string_equals_const(property_name, "g")) {
            mla_deserializer_read_uint8(deserializer, self->g);
        } else if (mla_string_equals_const(property_name, "b")) {
            mla_deserializer_read_uint8(deserializer, self->b);
        } else if (mla_string_equals_const(property_name, "a")) {
            mla_deserializer_read_uint8(deserializer, self->a);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }
};

struct mla_ui_surface_draw_point_t {
    mla_double_t x;
    mla_double_t y;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_ui_surface_draw_point_t* self = mla_pointer_get_data<const mla_ui_surface_draw_point_t>(obj);

        if (self == nullptr) {
            return false;
        }

        mla_serializer_write_double(serializer, mla_string_const("x"), self->x);
        mla_serializer_write_double(serializer, mla_string_const("y"), self->y);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_ui_surface_draw_point_t* self = mla_pointer_get_data<mla_ui_surface_draw_point_t>(obj);

        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "x")) {
            mla_deserializer_read_double(deserializer, self->x);
        } else if (mla_string_equals_const(property_name, "y")) {
            mla_deserializer_read_double(deserializer, self->y);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }
};

struct mla_ui_surface_draw_size_t {
    mla_double_t width;
    mla_double_t height;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_ui_surface_draw_size_t* self = mla_pointer_get_data<const mla_ui_surface_draw_size_t>(obj);

        if (self == nullptr) {
            return false;
        }

        mla_serializer_write_double(serializer, mla_string_const("width"),  self->width);
        mla_serializer_write_double(serializer, mla_string_const("height"), self->height);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_ui_surface_draw_size_t* self = mla_pointer_get_data<mla_ui_surface_draw_size_t>(obj);

        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "width")) {
            mla_deserializer_read_double(deserializer, self->width);
        } else if (mla_string_equals_const(property_name, "height")) {
            mla_deserializer_read_double(deserializer, self->height);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }
};

struct mla_ui_surface_font_type_t {
    mla_string_t family;
    mla_double_t size;
    mla_bool_t bold;
    mla_bool_t italic;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_ui_surface_font_type_t* self = mla_pointer_get_data<const mla_ui_surface_font_type_t>(obj);

        if (self == nullptr) {
            return false;
        }

        mla_serializer_write_string(serializer, mla_string_const("family"), self->family);
        mla_serializer_write_double(serializer, mla_string_const("size"),   self->size);
        mla_serializer_write_bool  (serializer, mla_string_const("bold"),   self->bold);
        mla_serializer_write_bool  (serializer, mla_string_const("italic"), self->italic);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_ui_surface_font_type_t* self = mla_pointer_get_data<mla_ui_surface_font_type_t>(obj);

        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "family")) {
            mla_deserializer_read_string(deserializer, self->family);
        } else if (mla_string_equals_const(property_name, "size")) {
            mla_deserializer_read_double(deserializer, self->size);
        } else if (mla_string_equals_const(property_name, "bold")) {
            mla_deserializer_read_bool(deserializer, self->bold);
        } else if (mla_string_equals_const(property_name, "italic")) {
            mla_deserializer_read_bool(deserializer, self->italic);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }
};

mla_ui_surface_font_type_t mla_ui_surface_font_type_empty();
mla_bool_t mla_ui_surface_font_type_equals(const mla_ui_surface_font_type_t& a, const mla_ui_surface_font_type_t& b);

struct mla_ui_surface_input_states_t {
    mla_ui_surface_draw_point_t cursorPosition;
    mla_bool_t leftMouseButtonDown;
    mla_bool_t rightMouseButtonDown;
    mla_bool_t middleMouseButtonDown;
    mla_bool_t shiftKeyDown;
    mla_bool_t ctrlKeyDown;
    mla_bool_t altKeyDown;
    mla_bool_t metaKeyDown;
    mla_uint32_t keyCodeDown;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_ui_surface_input_states_t* self = mla_pointer_get_data<const mla_ui_surface_input_states_t>(obj);

        if (self == nullptr) {
            return false;
        }

        mla_serializer_write_struct (serializer, mla_string_const("cursorPosition"),      self->cursorPosition, mla_ui_surface_draw_point_t);
        mla_serializer_write_bool   (serializer, mla_string_const("leftMouseButtonDown"), self->leftMouseButtonDown);
        mla_serializer_write_bool   (serializer, mla_string_const("rightMouseButtonDown"),self->rightMouseButtonDown);
        mla_serializer_write_bool   (serializer, mla_string_const("middleMouseButtonDown"),self->middleMouseButtonDown);
        mla_serializer_write_bool   (serializer, mla_string_const("shiftKeyDown"),        self->shiftKeyDown);
        mla_serializer_write_bool   (serializer, mla_string_const("ctrlKeyDown"),         self->ctrlKeyDown);
        mla_serializer_write_bool   (serializer, mla_string_const("altKeyDown"),          self->altKeyDown);
        mla_serializer_write_bool   (serializer, mla_string_const("metaKeyDown"),         self->metaKeyDown);
        mla_serializer_write_uint32 (serializer, mla_string_const("keyCodeDown"),         self->keyCodeDown);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_ui_surface_input_states_t* self = mla_pointer_get_data<mla_ui_surface_input_states_t>(obj);

        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "cursorPosition")) {
            mla_deserializer_read_struct(deserializer, self->cursorPosition, mla_ui_surface_draw_point_t);
        } else if (mla_string_equals_const(property_name, "leftMouseButtonDown")) {
            mla_deserializer_read_bool(deserializer, self->leftMouseButtonDown);
        } else if (mla_string_equals_const(property_name, "rightMouseButtonDown")) {
            mla_deserializer_read_bool(deserializer, self->rightMouseButtonDown);
        } else if (mla_string_equals_const(property_name, "middleMouseButtonDown")) {
            mla_deserializer_read_bool(deserializer, self->middleMouseButtonDown);
        } else if (mla_string_equals_const(property_name, "shiftKeyDown")) {
            mla_deserializer_read_bool(deserializer, self->shiftKeyDown);
        } else if (mla_string_equals_const(property_name, "ctrlKeyDown")) {
            mla_deserializer_read_bool(deserializer, self->ctrlKeyDown);
        } else if (mla_string_equals_const(property_name, "altKeyDown")) {
            mla_deserializer_read_bool(deserializer, self->altKeyDown);
        } else if (mla_string_equals_const(property_name, "metaKeyDown")) {
            mla_deserializer_read_bool(deserializer, self->metaKeyDown);
        } else if (mla_string_equals_const(property_name, "keyCodeDown")) {
            mla_deserializer_read_uint32(deserializer, self->keyCodeDown);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }
};

mla_ui_surface_input_states_t mla_ui_surface_input_states_empty();

enum mla_ui_surface_draw_command_kind: mla_uint8_t {
    MLA_UI_SURFACE_DRAW_COMMAND_KIND_NONE,
    MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT,
    MLA_UI_SURFACE_DRAW_COMMAND_KIND_CIRCLE,
    MLA_UI_SURFACE_DRAW_COMMAND_KIND_ELLIPSE,
    MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINE,
    MLA_UI_SURFACE_DRAW_COMMAND_KIND_POLYLINE,
    MLA_UI_SURFACE_DRAW_COMMAND_KIND_POLYGON,
    MLA_UI_SURFACE_DRAW_COMMAND_KIND_PATH,
    MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT,
    MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINEAR_GRADIENT,
    MLA_UI_SURFACE_DRAW_COMMAND_KIND_RADIAL_GRADIENT,
    MLA_UI_SURFACE_DRAW_COMMAND_KIND_STOP
};

struct mla_ui_surface_draw_command_rect_t {
    mla_double_t x;
    mla_double_t y;
    mla_double_t width;
    mla_double_t height;
    mla_double_t rx;
    mla_double_t ry;
    mla_ui_surface_draw_command_color_t color;
    mla_ui_surface_draw_command_color_t stroke;
    mla_double_t stroke_width;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_ui_surface_draw_command_rect_t* self = mla_pointer_get_data<const mla_ui_surface_draw_command_rect_t>(obj);

        if (self == nullptr) {
            return false;
        }

        mla_serializer_write_double(serializer, mla_string_const("x"),            self->x);
        mla_serializer_write_double(serializer, mla_string_const("y"),            self->y);
        mla_serializer_write_double(serializer, mla_string_const("width"),        self->width);
        mla_serializer_write_double(serializer, mla_string_const("height"),       self->height);
        mla_serializer_write_double(serializer, mla_string_const("rx"),           self->rx);
        mla_serializer_write_double(serializer, mla_string_const("ry"),           self->ry);
        mla_serializer_write_struct(serializer, mla_string_const("color"),        self->color,        mla_ui_surface_draw_command_color_t);
        mla_serializer_write_struct(serializer, mla_string_const("stroke"),       self->stroke,       mla_ui_surface_draw_command_color_t);
        mla_serializer_write_double(serializer, mla_string_const("stroke_width"), self->stroke_width);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_ui_surface_draw_command_rect_t* self = mla_pointer_get_data<mla_ui_surface_draw_command_rect_t>(obj);

        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "x")) {
            mla_deserializer_read_double(deserializer, self->x);
        } else if (mla_string_equals_const(property_name, "y")) {
            mla_deserializer_read_double(deserializer, self->y);
        } else if (mla_string_equals_const(property_name, "width")) {
            mla_deserializer_read_double(deserializer, self->width);
        } else if (mla_string_equals_const(property_name, "height")) {
            mla_deserializer_read_double(deserializer, self->height);
        } else if (mla_string_equals_const(property_name, "rx")) {
            mla_deserializer_read_double(deserializer, self->rx);
        } else if (mla_string_equals_const(property_name, "ry")) {
            mla_deserializer_read_double(deserializer, self->ry);
        } else if (mla_string_equals_const(property_name, "color")) {
            mla_deserializer_read_struct(deserializer, self->color, mla_ui_surface_draw_command_color_t);
        } else if (mla_string_equals_const(property_name, "stroke")) {
            mla_deserializer_read_struct(deserializer, self->stroke, mla_ui_surface_draw_command_color_t);
        } else if (mla_string_equals_const(property_name, "stroke_width")) {
            mla_deserializer_read_double(deserializer, self->stroke_width);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }
};

struct mla_ui_surface_draw_command_circle_t {
    mla_double_t cx;
    mla_double_t cy;
    mla_double_t r;
    mla_ui_surface_draw_command_color_t fill;
    mla_ui_surface_draw_command_color_t stroke;
    mla_double_t stroke_width;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_ui_surface_draw_command_circle_t* self = mla_pointer_get_data<const mla_ui_surface_draw_command_circle_t>(obj);

        if (self == nullptr) {
            return false;
        }

        mla_serializer_write_double(serializer, mla_string_const("cx"),           self->cx);
        mla_serializer_write_double(serializer, mla_string_const("cy"),           self->cy);
        mla_serializer_write_double(serializer, mla_string_const("r"),            self->r);
        mla_serializer_write_struct(serializer, mla_string_const("fill"),         self->fill,         mla_ui_surface_draw_command_color_t);
        mla_serializer_write_struct(serializer, mla_string_const("stroke"),       self->stroke,       mla_ui_surface_draw_command_color_t);
        mla_serializer_write_double(serializer, mla_string_const("stroke_width"), self->stroke_width);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_ui_surface_draw_command_circle_t* self = mla_pointer_get_data<mla_ui_surface_draw_command_circle_t>(obj);

        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "cx")) {
            mla_deserializer_read_double(deserializer, self->cx);
        } else if (mla_string_equals_const(property_name, "cy")) {
            mla_deserializer_read_double(deserializer, self->cy);
        } else if (mla_string_equals_const(property_name, "r")) {
            mla_deserializer_read_double(deserializer, self->r);
        } else if (mla_string_equals_const(property_name, "fill")) {
            mla_deserializer_read_struct(deserializer, self->fill, mla_ui_surface_draw_command_color_t);
        } else if (mla_string_equals_const(property_name, "stroke")) {
            mla_deserializer_read_struct(deserializer, self->stroke, mla_ui_surface_draw_command_color_t);
        } else if (mla_string_equals_const(property_name, "stroke_width")) {
            mla_deserializer_read_double(deserializer, self->stroke_width);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }
};

struct mla_ui_surface_draw_command_ellipse_t {
    mla_double_t cx;
    mla_double_t cy;
    mla_double_t rx;
    mla_double_t ry;
    mla_ui_surface_draw_command_color_t fill;
    mla_ui_surface_draw_command_color_t stroke;
    mla_double_t stroke_width;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_ui_surface_draw_command_ellipse_t* self = mla_pointer_get_data<const mla_ui_surface_draw_command_ellipse_t>(obj);

        if (self == nullptr) {
            return false;
        }

        mla_serializer_write_double(serializer, mla_string_const("cx"),           self->cx);
        mla_serializer_write_double(serializer, mla_string_const("cy"),           self->cy);
        mla_serializer_write_double(serializer, mla_string_const("rx"),           self->rx);
        mla_serializer_write_double(serializer, mla_string_const("ry"),           self->ry);
        mla_serializer_write_struct(serializer, mla_string_const("fill"),         self->fill,         mla_ui_surface_draw_command_color_t);
        mla_serializer_write_struct(serializer, mla_string_const("stroke"),       self->stroke,       mla_ui_surface_draw_command_color_t);
        mla_serializer_write_double(serializer, mla_string_const("stroke_width"), self->stroke_width);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_ui_surface_draw_command_ellipse_t* self = mla_pointer_get_data<mla_ui_surface_draw_command_ellipse_t>(obj);

        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "cx")) {
            mla_deserializer_read_double(deserializer, self->cx);
        } else if (mla_string_equals_const(property_name, "cy")) {
            mla_deserializer_read_double(deserializer, self->cy);
        } else if (mla_string_equals_const(property_name, "rx")) {
            mla_deserializer_read_double(deserializer, self->rx);
        } else if (mla_string_equals_const(property_name, "ry")) {
            mla_deserializer_read_double(deserializer, self->ry);
        } else if (mla_string_equals_const(property_name, "fill")) {
            mla_deserializer_read_struct(deserializer, self->fill, mla_ui_surface_draw_command_color_t);
        } else if (mla_string_equals_const(property_name, "stroke")) {
            mla_deserializer_read_struct(deserializer, self->stroke, mla_ui_surface_draw_command_color_t);
        } else if (mla_string_equals_const(property_name, "stroke_width")) {
            mla_deserializer_read_double(deserializer, self->stroke_width);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }
};

struct mla_ui_surface_draw_command_line_t {
    mla_double_t x1;
    mla_double_t y1;
    mla_double_t x2;
    mla_double_t y2;
    mla_ui_surface_draw_command_color_t stroke;
    mla_double_t stroke_width;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_ui_surface_draw_command_line_t* self = mla_pointer_get_data<const mla_ui_surface_draw_command_line_t>(obj);

        if (self == nullptr) {
            return false;
        }

        mla_serializer_write_double(serializer, mla_string_const("x1"),           self->x1);
        mla_serializer_write_double(serializer, mla_string_const("y1"),           self->y1);
        mla_serializer_write_double(serializer, mla_string_const("x2"),           self->x2);
        mla_serializer_write_double(serializer, mla_string_const("y2"),           self->y2);
        mla_serializer_write_struct(serializer, mla_string_const("stroke"),       self->stroke,       mla_ui_surface_draw_command_color_t);
        mla_serializer_write_double(serializer, mla_string_const("stroke_width"), self->stroke_width);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_ui_surface_draw_command_line_t* self = mla_pointer_get_data<mla_ui_surface_draw_command_line_t>(obj);

        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "x1")) {
            mla_deserializer_read_double(deserializer, self->x1);
        } else if (mla_string_equals_const(property_name, "y1")) {
            mla_deserializer_read_double(deserializer, self->y1);
        } else if (mla_string_equals_const(property_name, "x2")) {
            mla_deserializer_read_double(deserializer, self->x2);
        } else if (mla_string_equals_const(property_name, "y2")) {
            mla_deserializer_read_double(deserializer, self->y2);
        } else if (mla_string_equals_const(property_name, "stroke")) {
            mla_deserializer_read_struct(deserializer, self->stroke, mla_ui_surface_draw_command_color_t);
        } else if (mla_string_equals_const(property_name, "stroke_width")) {
            mla_deserializer_read_double(deserializer, self->stroke_width);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }
};

struct mla_ui_surface_draw_command_polyline_t {
    mla_array_list_t<mla_ui_surface_draw_point_t> points;
    mla_ui_surface_draw_command_color_t fill;
    mla_ui_surface_draw_command_color_t stroke;
    mla_double_t stroke_width;

    static mla_ui_surface_draw_command_polyline_t init() {
        return { mla_array_list_empty<mla_ui_surface_draw_point_t>(), {0, 0, 0, 0}, {0, 0, 0, 0}, 0.0 };
    }

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_ui_surface_draw_command_polyline_t* self = mla_pointer_get_data<const mla_ui_surface_draw_command_polyline_t>(obj);

        if (self == nullptr) {
            return false;
        }

        mla_serializer_write_list_struct(serializer, mla_string_const("points"),       self->points, mla_ui_surface_draw_point_t);
        mla_serializer_write_struct     (serializer, mla_string_const("fill"),         self->fill,   mla_ui_surface_draw_command_color_t);
        mla_serializer_write_struct     (serializer, mla_string_const("stroke"),       self->stroke, mla_ui_surface_draw_command_color_t);
        mla_serializer_write_double     (serializer, mla_string_const("stroke_width"), self->stroke_width);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_ui_surface_draw_command_polyline_t* self = mla_pointer_get_data<mla_ui_surface_draw_command_polyline_t>(obj);

        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "points")) {
            mla_deserializer_read_list_struct(deserializer, self->points, mla_ui_surface_draw_point_t);
        } else if (mla_string_equals_const(property_name, "fill")) {
            mla_deserializer_read_struct(deserializer, self->fill, mla_ui_surface_draw_command_color_t);
        } else if (mla_string_equals_const(property_name, "stroke")) {
            mla_deserializer_read_struct(deserializer, self->stroke, mla_ui_surface_draw_command_color_t);
        } else if (mla_string_equals_const(property_name, "stroke_width")) {
            mla_deserializer_read_double(deserializer, self->stroke_width);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }
};

mla_ui_surface_draw_command_polyline_t mla_ui_surface_draw_command_polyline_empty();

struct mla_ui_surface_draw_command_polygon_t {
    mla_array_list_t<mla_ui_surface_draw_point_t> points;
    mla_ui_surface_draw_command_color_t fill;
    mla_ui_surface_draw_command_color_t stroke;
    mla_double_t stroke_width;

    static mla_ui_surface_draw_command_polygon_t init() {
        return { mla_array_list_empty<mla_ui_surface_draw_point_t>(), {0, 0, 0, 0}, {0, 0, 0, 0}, 0.0 };
    }

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_ui_surface_draw_command_polygon_t* self = mla_pointer_get_data<const mla_ui_surface_draw_command_polygon_t>(obj);

        if (self == nullptr) {
            return false;
        }

        mla_serializer_write_list_struct(serializer, mla_string_const("points"),       self->points, mla_ui_surface_draw_point_t);
        mla_serializer_write_struct     (serializer, mla_string_const("fill"),         self->fill,   mla_ui_surface_draw_command_color_t);
        mla_serializer_write_struct     (serializer, mla_string_const("stroke"),       self->stroke, mla_ui_surface_draw_command_color_t);
        mla_serializer_write_double     (serializer, mla_string_const("stroke_width"), self->stroke_width);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_ui_surface_draw_command_polygon_t* self = mla_pointer_get_data<mla_ui_surface_draw_command_polygon_t>(obj);

        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "points")) {
            mla_deserializer_read_list_struct(deserializer, self->points, mla_ui_surface_draw_point_t);
        } else if (mla_string_equals_const(property_name, "fill")) {
            mla_deserializer_read_struct(deserializer, self->fill, mla_ui_surface_draw_command_color_t);
        } else if (mla_string_equals_const(property_name, "stroke")) {
            mla_deserializer_read_struct(deserializer, self->stroke, mla_ui_surface_draw_command_color_t);
        } else if (mla_string_equals_const(property_name, "stroke_width")) {
            mla_deserializer_read_double(deserializer, self->stroke_width);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }
};

mla_ui_surface_draw_command_polygon_t mla_ui_surface_draw_command_polygon_empty();

// Path Command Structures

enum mla_ui_surface_draw_path_command_kind_t: mla_uint8_t {
    MLA_UI_SURFACE_DRAW_PATH_COMMAND_MOVE_TO = 0,
    MLA_UI_SURFACE_DRAW_PATH_COMMAND_LINE_TO,
    MLA_UI_SURFACE_DRAW_PATH_COMMAND_QUADRATIC_CURVE_TO,
    MLA_UI_SURFACE_DRAW_PATH_COMMAND_CUBIC_CURVE_TO,
    MLA_UI_SURFACE_DRAW_PATH_COMMAND_ARC_TO,
    MLA_UI_SURFACE_DRAW_PATH_COMMAND_CLOSE_PATH
};

struct mla_ui_surface_draw_path_command_move_to_t {
    mla_double_t x;
    mla_double_t y;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_ui_surface_draw_path_command_move_to_t* self = mla_pointer_get_data<const mla_ui_surface_draw_path_command_move_to_t>(obj);

        if (self == nullptr) {
            return false;
        }

        mla_serializer_write_double(serializer, mla_string_const("x"), self->x);
        mla_serializer_write_double(serializer, mla_string_const("y"), self->y);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_ui_surface_draw_path_command_move_to_t* self = mla_pointer_get_data<mla_ui_surface_draw_path_command_move_to_t>(obj);

        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "x")) {
            mla_deserializer_read_double(deserializer, self->x);
        } else if (mla_string_equals_const(property_name, "y")) {
            mla_deserializer_read_double(deserializer, self->y);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }
};

struct mla_ui_surface_draw_path_command_line_to_t {
    mla_double_t x;
    mla_double_t y;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_ui_surface_draw_path_command_line_to_t* self = mla_pointer_get_data<const mla_ui_surface_draw_path_command_line_to_t>(obj);

        if (self == nullptr) {
            return false;
        }

        mla_serializer_write_double(serializer, mla_string_const("x"), self->x);
        mla_serializer_write_double(serializer, mla_string_const("y"), self->y);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_ui_surface_draw_path_command_line_to_t* self = mla_pointer_get_data<mla_ui_surface_draw_path_command_line_to_t>(obj);

        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "x")) {
            mla_deserializer_read_double(deserializer, self->x);
        } else if (mla_string_equals_const(property_name, "y")) {
            mla_deserializer_read_double(deserializer, self->y);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }
};

struct mla_ui_surface_draw_path_command_quadratic_curve_to_t {
    mla_double_t cpx;
    mla_double_t cpy;
    mla_double_t x;
    mla_double_t y;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_ui_surface_draw_path_command_quadratic_curve_to_t* self = mla_pointer_get_data<const mla_ui_surface_draw_path_command_quadratic_curve_to_t>(obj);

        if (self == nullptr) {
            return false;
        }

        mla_serializer_write_double(serializer, mla_string_const("cpx"), self->cpx);
        mla_serializer_write_double(serializer, mla_string_const("cpy"), self->cpy);
        mla_serializer_write_double(serializer, mla_string_const("x"),   self->x);
        mla_serializer_write_double(serializer, mla_string_const("y"),   self->y);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_ui_surface_draw_path_command_quadratic_curve_to_t* self = mla_pointer_get_data<mla_ui_surface_draw_path_command_quadratic_curve_to_t>(obj);

        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "cpx")) {
            mla_deserializer_read_double(deserializer, self->cpx);
        } else if (mla_string_equals_const(property_name, "cpy")) {
            mla_deserializer_read_double(deserializer, self->cpy);
        } else if (mla_string_equals_const(property_name, "x")) {
            mla_deserializer_read_double(deserializer, self->x);
        } else if (mla_string_equals_const(property_name, "y")) {
            mla_deserializer_read_double(deserializer, self->y);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }
};

struct mla_ui_surface_draw_path_command_cubic_curve_to_t {
    mla_double_t cp1x;
    mla_double_t cp1y;
    mla_double_t cp2x;
    mla_double_t cp2y;
    mla_double_t x;
    mla_double_t y;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_ui_surface_draw_path_command_cubic_curve_to_t* self = mla_pointer_get_data<const mla_ui_surface_draw_path_command_cubic_curve_to_t>(obj);

        if (self == nullptr) {
            return false;
        }

        mla_serializer_write_double(serializer, mla_string_const("cp1x"), self->cp1x);
        mla_serializer_write_double(serializer, mla_string_const("cp1y"), self->cp1y);
        mla_serializer_write_double(serializer, mla_string_const("cp2x"), self->cp2x);
        mla_serializer_write_double(serializer, mla_string_const("cp2y"), self->cp2y);
        mla_serializer_write_double(serializer, mla_string_const("x"),    self->x);
        mla_serializer_write_double(serializer, mla_string_const("y"),    self->y);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_ui_surface_draw_path_command_cubic_curve_to_t* self = mla_pointer_get_data<mla_ui_surface_draw_path_command_cubic_curve_to_t>(obj);

        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "cp1x")) {
            mla_deserializer_read_double(deserializer, self->cp1x);
        } else if (mla_string_equals_const(property_name, "cp1y")) {
            mla_deserializer_read_double(deserializer, self->cp1y);
        } else if (mla_string_equals_const(property_name, "cp2x")) {
            mla_deserializer_read_double(deserializer, self->cp2x);
        } else if (mla_string_equals_const(property_name, "cp2y")) {
            mla_deserializer_read_double(deserializer, self->cp2y);
        } else if (mla_string_equals_const(property_name, "x")) {
            mla_deserializer_read_double(deserializer, self->x);
        } else if (mla_string_equals_const(property_name, "y")) {
            mla_deserializer_read_double(deserializer, self->y);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }
};

struct mla_ui_surface_draw_path_command_arc_to_t {
    mla_double_t rx;
    mla_double_t ry;
    mla_double_t x_axis_rotation;
    mla_bool_t large_arc_flag;
    mla_bool_t sweep_flag;
    mla_double_t x;
    mla_double_t y;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_ui_surface_draw_path_command_arc_to_t* self = mla_pointer_get_data<const mla_ui_surface_draw_path_command_arc_to_t>(obj);

        if (self == nullptr) {
            return false;
        }

        mla_serializer_write_double(serializer, mla_string_const("rx"),              self->rx);
        mla_serializer_write_double(serializer, mla_string_const("ry"),              self->ry);
        mla_serializer_write_double(serializer, mla_string_const("x_axis_rotation"), self->x_axis_rotation);
        mla_serializer_write_bool  (serializer, mla_string_const("large_arc_flag"),  self->large_arc_flag);
        mla_serializer_write_bool  (serializer, mla_string_const("sweep_flag"),      self->sweep_flag);
        mla_serializer_write_double(serializer, mla_string_const("x"),               self->x);
        mla_serializer_write_double(serializer, mla_string_const("y"),               self->y);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_ui_surface_draw_path_command_arc_to_t* self = mla_pointer_get_data<mla_ui_surface_draw_path_command_arc_to_t>(obj);

        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "rx")) {
            mla_deserializer_read_double(deserializer, self->rx);
        } else if (mla_string_equals_const(property_name, "ry")) {
            mla_deserializer_read_double(deserializer, self->ry);
        } else if (mla_string_equals_const(property_name, "x_axis_rotation")) {
            mla_deserializer_read_double(deserializer, self->x_axis_rotation);
        } else if (mla_string_equals_const(property_name, "large_arc_flag")) {
            mla_deserializer_read_bool(deserializer, self->large_arc_flag);
        } else if (mla_string_equals_const(property_name, "sweep_flag")) {
            mla_deserializer_read_bool(deserializer, self->sweep_flag);
        } else if (mla_string_equals_const(property_name, "x")) {
            mla_deserializer_read_double(deserializer, self->x);
        } else if (mla_string_equals_const(property_name, "y")) {
            mla_deserializer_read_double(deserializer, self->y);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }
};

struct mla_ui_surface_draw_path_command_t {
    mla_ui_surface_draw_path_command_kind_t kind;

    union {
        mla_ui_surface_draw_path_command_move_to_t move_to;
        mla_ui_surface_draw_path_command_line_to_t line_to;
        mla_ui_surface_draw_path_command_quadratic_curve_to_t quadratic_curve_to;
        mla_ui_surface_draw_path_command_cubic_curve_to_t cubic_curve_to;
        mla_ui_surface_draw_path_command_arc_to_t arc_to;
    };

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_ui_surface_draw_path_command_t* self = mla_pointer_get_data<const mla_ui_surface_draw_path_command_t>(obj);

        if (self == nullptr) {
            return false;
        }

        mla_serializer_write_enum(serializer, mla_string_const("kind"), self->kind);
        if (self->kind == MLA_UI_SURFACE_DRAW_PATH_COMMAND_MOVE_TO) {
            mla_serializer_write_struct(serializer, mla_string_const("moveTo"), self->move_to, mla_ui_surface_draw_path_command_move_to_t);
        } else if (self->kind == MLA_UI_SURFACE_DRAW_PATH_COMMAND_LINE_TO) {
            mla_serializer_write_struct(serializer, mla_string_const("lineTo"), self->line_to, mla_ui_surface_draw_path_command_line_to_t);
        } else if (self->kind == MLA_UI_SURFACE_DRAW_PATH_COMMAND_QUADRATIC_CURVE_TO) {
            mla_serializer_write_struct(serializer, mla_string_const("quadraticCurveTo"), self->quadratic_curve_to, mla_ui_surface_draw_path_command_quadratic_curve_to_t);
        } else if (self->kind == MLA_UI_SURFACE_DRAW_PATH_COMMAND_CUBIC_CURVE_TO) {
            mla_serializer_write_struct(serializer, mla_string_const("cubicCurveTo"), self->cubic_curve_to, mla_ui_surface_draw_path_command_cubic_curve_to_t);
        } else if (self->kind == MLA_UI_SURFACE_DRAW_PATH_COMMAND_ARC_TO) {
            mla_serializer_write_struct(serializer, mla_string_const("arcTo"), self->arc_to, mla_ui_surface_draw_path_command_arc_to_t);
        }
        // MLA_UI_SURFACE_DRAW_PATH_COMMAND_CLOSE_PATH has no parameters
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_ui_surface_draw_path_command_t* self = mla_pointer_get_data<mla_ui_surface_draw_path_command_t>(obj);

        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "kind")) {
            mla_deserializer_read_enum(mla_ui_surface_draw_path_command_kind_t, deserializer, self->kind);
        } else if (mla_string_equals_const(property_name, "moveTo")) {
            mla_deserializer_read_struct(deserializer, self->move_to, mla_ui_surface_draw_path_command_move_to_t);
        } else if (mla_string_equals_const(property_name, "lineTo")) {
            mla_deserializer_read_struct(deserializer, self->line_to, mla_ui_surface_draw_path_command_line_to_t);
        } else if (mla_string_equals_const(property_name, "quadraticCurveTo")) {
            mla_deserializer_read_struct(deserializer, self->quadratic_curve_to, mla_ui_surface_draw_path_command_quadratic_curve_to_t);
        } else if (mla_string_equals_const(property_name, "cubicCurveTo")) {
            mla_deserializer_read_struct(deserializer, self->cubic_curve_to, mla_ui_surface_draw_path_command_cubic_curve_to_t);
        } else if (mla_string_equals_const(property_name, "arcTo")) {
            mla_deserializer_read_struct(deserializer, self->arc_to, mla_ui_surface_draw_path_command_arc_to_t);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
        return MLA_DESERIALIZER_READ_HANDLED;
    }

    static mla_ui_surface_draw_path_command_t init() {
        mla_ui_surface_draw_path_command_t cmd;
        cmd.kind = MLA_UI_SURFACE_DRAW_PATH_COMMAND_CLOSE_PATH;
        return cmd;
    }
};

struct mla_ui_surface_draw_command_path_t {
    mla_array_list_t<mla_init_struct(mla_ui_surface_draw_path_command_t)> commands;
    mla_ui_surface_draw_command_color_t fill;
    mla_ui_surface_draw_command_color_t stroke;
    mla_double_t stroke_width;

    static mla_ui_surface_draw_command_path_t init() {
        return { mla_array_list_empty<mla_init_struct(mla_ui_surface_draw_path_command_t)>(), {0, 0, 0, 0}, {0, 0, 0, 0}, 0.0 };
    }

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_ui_surface_draw_command_path_t* self = mla_pointer_get_data<const mla_ui_surface_draw_command_path_t>(obj);

        if (self == nullptr) {
            return false;
        }

        mla_serializer_write_list_struct(serializer, mla_string_const("commands"),     self->commands, mla_ui_surface_draw_path_command_t);
        mla_serializer_write_struct     (serializer, mla_string_const("fill"),         self->fill,     mla_ui_surface_draw_command_color_t);
        mla_serializer_write_struct     (serializer, mla_string_const("stroke"),       self->stroke,   mla_ui_surface_draw_command_color_t);
        mla_serializer_write_double     (serializer, mla_string_const("stroke_width"), self->stroke_width);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_ui_surface_draw_command_path_t* self = mla_pointer_get_data<mla_ui_surface_draw_command_path_t>(obj);

        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "commands")) {
            mla_deserializer_read_list_struct(deserializer, self->commands, mla_ui_surface_draw_path_command_t);
        } else if (mla_string_equals_const(property_name, "fill")) {
            mla_deserializer_read_struct(deserializer, self->fill, mla_ui_surface_draw_command_color_t);
        } else if (mla_string_equals_const(property_name, "stroke")) {
            mla_deserializer_read_struct(deserializer, self->stroke, mla_ui_surface_draw_command_color_t);
        } else if (mla_string_equals_const(property_name, "stroke_width")) {
            mla_deserializer_read_double(deserializer, self->stroke_width);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }
};

mla_ui_surface_draw_command_path_t mla_ui_surface_draw_command_path_empty();

struct mla_ui_surface_draw_command_text_t {
    mla_double_t x;
    mla_double_t y;
    mla_string_t content;
    mla_ui_surface_font_type_t font_type;
    mla_ui_surface_draw_command_color_t fill;

    static mla_ui_surface_draw_command_text_t init() {
        return { 0.0, 0.0, mla_string_empty(), mla_ui_surface_font_type_empty(), {0, 0, 0, 0} };
    }

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_ui_surface_draw_command_text_t* self = mla_pointer_get_data<const mla_ui_surface_draw_command_text_t>(obj);

        if (self == nullptr) {
            return false;
        }

        mla_serializer_write_double(serializer, mla_string_const("x"),         self->x);
        mla_serializer_write_double(serializer, mla_string_const("y"),         self->y);
        mla_serializer_write_string(serializer, mla_string_const("content"),   self->content);
        mla_serializer_write_struct(serializer, mla_string_const("font_type"), self->font_type, mla_ui_surface_font_type_t);
        mla_serializer_write_struct(serializer, mla_string_const("fill"),      self->fill,      mla_ui_surface_draw_command_color_t);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_ui_surface_draw_command_text_t* self = mla_pointer_get_data<mla_ui_surface_draw_command_text_t>(obj);

        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "x")) {
            mla_deserializer_read_double(deserializer, self->x);
        } else if (mla_string_equals_const(property_name, "y")) {
            mla_deserializer_read_double(deserializer, self->y);
        } else if (mla_string_equals_const(property_name, "content")) {
            mla_deserializer_read_string(deserializer, self->content);
        } else if (mla_string_equals_const(property_name, "font_type")) {
            mla_deserializer_read_struct(deserializer, self->font_type, mla_ui_surface_font_type_t);
        } else if (mla_string_equals_const(property_name, "fill")) {
            mla_deserializer_read_struct(deserializer, self->fill, mla_ui_surface_draw_command_color_t);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }
};

mla_ui_surface_draw_command_text_t mla_ui_surface_draw_command_text_empty();

struct mla_ui_surface_draw_command_linear_gradient_t {
    mla_double_t x1;
    mla_double_t y1;
    mla_double_t x2;
    mla_double_t y2;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_ui_surface_draw_command_linear_gradient_t* self = mla_pointer_get_data<const mla_ui_surface_draw_command_linear_gradient_t>(obj);

        if (self == nullptr) {
            return false;
        }

        mla_serializer_write_double(serializer, mla_string_const("x1"), self->x1);
        mla_serializer_write_double(serializer, mla_string_const("y1"), self->y1);
        mla_serializer_write_double(serializer, mla_string_const("x2"), self->x2);
        mla_serializer_write_double(serializer, mla_string_const("y2"), self->y2);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_ui_surface_draw_command_linear_gradient_t* self = mla_pointer_get_data<mla_ui_surface_draw_command_linear_gradient_t>(obj);

        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "x1")) {
            mla_deserializer_read_double(deserializer, self->x1);
        } else if (mla_string_equals_const(property_name, "y1")) {
            mla_deserializer_read_double(deserializer, self->y1);
        } else if (mla_string_equals_const(property_name, "x2")) {
            mla_deserializer_read_double(deserializer, self->x2);
        } else if (mla_string_equals_const(property_name, "y2")) {
            mla_deserializer_read_double(deserializer, self->y2);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }
};

struct mla_ui_surface_draw_command_radial_gradient_t {
    mla_double_t cx;
    mla_double_t cy;
    mla_double_t r;
    mla_double_t fx;
    mla_double_t fy;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_ui_surface_draw_command_radial_gradient_t* self = mla_pointer_get_data<const mla_ui_surface_draw_command_radial_gradient_t>(obj);

        if (self == nullptr) {
            return false;
        }

        mla_serializer_write_double(serializer, mla_string_const("cx"), self->cx);
        mla_serializer_write_double(serializer, mla_string_const("cy"), self->cy);
        mla_serializer_write_double(serializer, mla_string_const("r"),  self->r);
        mla_serializer_write_double(serializer, mla_string_const("fx"), self->fx);
        mla_serializer_write_double(serializer, mla_string_const("fy"), self->fy);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_ui_surface_draw_command_radial_gradient_t* self = mla_pointer_get_data<mla_ui_surface_draw_command_radial_gradient_t>(obj);

        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "cx")) {
            mla_deserializer_read_double(deserializer, self->cx);
        } else if (mla_string_equals_const(property_name, "cy")) {
            mla_deserializer_read_double(deserializer, self->cy);
        } else if (mla_string_equals_const(property_name, "r")) {
            mla_deserializer_read_double(deserializer, self->r);
        } else if (mla_string_equals_const(property_name, "fx")) {
            mla_deserializer_read_double(deserializer, self->fx);
        } else if (mla_string_equals_const(property_name, "fy")) {
            mla_deserializer_read_double(deserializer, self->fy);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }
};

struct mla_ui_surface_draw_command_stop_t {
    mla_double_t offset;
    mla_ui_surface_draw_command_color_t stop_color;
    mla_double_t stop_opacity;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_ui_surface_draw_command_stop_t* self = mla_pointer_get_data<const mla_ui_surface_draw_command_stop_t>(obj);

        if (self == nullptr) {
            return false;
        }

        mla_serializer_write_double(serializer, mla_string_const("offset"),       self->offset);
        mla_serializer_write_struct(serializer, mla_string_const("stop_color"),   self->stop_color, mla_ui_surface_draw_command_color_t);
        mla_serializer_write_double(serializer, mla_string_const("stop_opacity"), self->stop_opacity);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_ui_surface_draw_command_stop_t* self = mla_pointer_get_data<mla_ui_surface_draw_command_stop_t>(obj);

        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "offset")) {
            mla_deserializer_read_double(deserializer, self->offset);
        } else if (mla_string_equals_const(property_name, "stop_color")) {
            mla_deserializer_read_struct(deserializer, self->stop_color, mla_ui_surface_draw_command_color_t);
        } else if (mla_string_equals_const(property_name, "stop_opacity")) {
            mla_deserializer_read_double(deserializer, self->stop_opacity);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }
};

struct mla_ui_surface_draw_command_t {
    mla_ui_surface_draw_command_kind kind;

    union {
        mla_ui_surface_draw_command_rect_t rect;
        mla_ui_surface_draw_command_circle_t circle;
        mla_ui_surface_draw_command_ellipse_t ellipse;
        mla_ui_surface_draw_command_line_t line;
        mla_ui_surface_draw_command_linear_gradient_t linear_gradient;
        mla_ui_surface_draw_command_radial_gradient_t radial_gradient;
        mla_ui_surface_draw_command_stop_t stop;
    };

    mla_ui_surface_draw_command_text_t text;
    mla_ui_surface_draw_command_path_t path;
    mla_ui_surface_draw_command_polyline_t polyline;
    mla_ui_surface_draw_command_polygon_t polygon;

    static mla_ui_surface_draw_command_t init() {
        return { MLA_UI_SURFACE_DRAW_COMMAND_KIND_NONE, {{0, 0, 0, 0, 0, 0, {0, 0, 0, 0}, {0, 0, 0, 0}, 0}}, mla_ui_surface_draw_command_text_t::init(), mla_ui_surface_draw_command_path_t::init(), mla_ui_surface_draw_command_polyline_t::init(), mla_ui_surface_draw_command_polygon_t::init() };
    }

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_ui_surface_draw_command_t* self = mla_pointer_get_data<const mla_ui_surface_draw_command_t>(obj);

        if (self == nullptr) {
            return false;
        }

        mla_serializer_write_enum(serializer, mla_string_const("kind"), self->kind);
        if (self->kind == MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT) {
            mla_serializer_write_struct(serializer, mla_string_const("rect"), self->rect, mla_ui_surface_draw_command_rect_t);
        } else if (self->kind == MLA_UI_SURFACE_DRAW_COMMAND_KIND_CIRCLE) {
            mla_serializer_write_struct(serializer, mla_string_const("circle"), self->circle, mla_ui_surface_draw_command_circle_t);
        } else if (self->kind == MLA_UI_SURFACE_DRAW_COMMAND_KIND_ELLIPSE) {
            mla_serializer_write_struct(serializer, mla_string_const("ellipse"), self->ellipse, mla_ui_surface_draw_command_ellipse_t);
        } else if (self->kind == MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINE) {
            mla_serializer_write_struct(serializer, mla_string_const("line"), self->line, mla_ui_surface_draw_command_line_t);
        } else if (self->kind == MLA_UI_SURFACE_DRAW_COMMAND_KIND_POLYLINE) {
            mla_serializer_write_struct(serializer, mla_string_const("polyline"), self->polyline, mla_ui_surface_draw_command_polyline_t);
        } else if (self->kind == MLA_UI_SURFACE_DRAW_COMMAND_KIND_POLYGON) {
            mla_serializer_write_struct(serializer, mla_string_const("polygon"), self->polygon, mla_ui_surface_draw_command_polygon_t);
        } else if (self->kind == MLA_UI_SURFACE_DRAW_COMMAND_KIND_PATH) {
            mla_serializer_write_struct(serializer, mla_string_const("path"), self->path, mla_ui_surface_draw_command_path_t);
        } else if (self->kind == MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT) {
            mla_serializer_write_struct(serializer, mla_string_const("text"), self->text, mla_ui_surface_draw_command_text_t);
        } else if (self->kind == MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINEAR_GRADIENT) {
            mla_serializer_write_struct(serializer, mla_string_const("linearGradient"), self->linear_gradient, mla_ui_surface_draw_command_linear_gradient_t);
        } else if (self->kind == MLA_UI_SURFACE_DRAW_COMMAND_KIND_RADIAL_GRADIENT) {
            mla_serializer_write_struct(serializer, mla_string_const("radialGradient"), self->radial_gradient, mla_ui_surface_draw_command_radial_gradient_t);
        } else if (self->kind == MLA_UI_SURFACE_DRAW_COMMAND_KIND_STOP) {
            mla_serializer_write_struct(serializer, mla_string_const("stop"), self->stop, mla_ui_surface_draw_command_stop_t);
        }
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_ui_surface_draw_command_t* self = mla_pointer_get_data<mla_ui_surface_draw_command_t>(obj);

        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "kind")) {
            mla_deserializer_read_enum(mla_ui_surface_draw_command_kind, deserializer, self->kind);
        } else if (mla_string_equals_const(property_name, "rect")) {
            mla_deserializer_read_struct(deserializer, self->rect, mla_ui_surface_draw_command_rect_t);
        } else if (mla_string_equals_const(property_name, "circle")) {
            mla_deserializer_read_struct(deserializer, self->circle, mla_ui_surface_draw_command_circle_t);
        } else if (mla_string_equals_const(property_name, "ellipse")) {
            mla_deserializer_read_struct(deserializer, self->ellipse, mla_ui_surface_draw_command_ellipse_t);
        } else if (mla_string_equals_const(property_name, "line")) {
            mla_deserializer_read_struct(deserializer, self->line, mla_ui_surface_draw_command_line_t);
        } else if (mla_string_equals_const(property_name, "polyline")) {
            mla_deserializer_read_struct(deserializer, self->polyline, mla_ui_surface_draw_command_polyline_t);
        } else if (mla_string_equals_const(property_name, "polygon")) {
            mla_deserializer_read_struct(deserializer, self->polygon, mla_ui_surface_draw_command_polygon_t);
        } else if (mla_string_equals_const(property_name, "path")) {
            mla_deserializer_read_struct(deserializer, self->path, mla_ui_surface_draw_command_path_t);
        } else if (mla_string_equals_const(property_name, "text")) {
            mla_deserializer_read_struct(deserializer, self->text, mla_ui_surface_draw_command_text_t);
        } else if (mla_string_equals_const(property_name, "linearGradient")) {
            mla_deserializer_read_struct(deserializer, self->linear_gradient, mla_ui_surface_draw_command_linear_gradient_t);
        } else if (mla_string_equals_const(property_name, "radialGradient")) {
            mla_deserializer_read_struct(deserializer, self->radial_gradient, mla_ui_surface_draw_command_radial_gradient_t);
        } else if (mla_string_equals_const(property_name, "stop")) {
            mla_deserializer_read_struct(deserializer, self->stop, mla_ui_surface_draw_command_stop_t);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }
};

mla_ui_surface_draw_command_t mla_ui_surface_draw_command_empty();

void mla_ui_surface_draw_commands_scale(mla_array_list_t<mla_init_struct(mla_ui_surface_draw_command_t)>& drawCommands, mla_double_t scaleX, mla_double_t scaleY);
void mla_ui_surface_draw_command_scale(mla_ui_surface_draw_command_t& command, mla_double_t scaleX, mla_double_t scaleY);

mla_bool_t mla_ui_surface_parse_color_from_hex_string(const mla_string_t& colorStr, mla_ui_surface_draw_command_color_t& outColor);


enum mla_ui_surface_input_event_click_button : mla_uint8_t {
    MLA_UI_SURFACE_INPUT_EVENT_CLICK_BUTTON_NONE,
    MLA_UI_SURFACE_INPUT_EVENT_CLICK_BUTTON_LEFT,
    MLA_UI_SURFACE_INPUT_EVENT_CLICK_BUTTON_RIGHT,
    MLA_UI_SURFACE_INPUT_EVENT_CLICK_BUTTON_MIDDLE
};

struct mla_ui_surface_input_event_click_t {
    mla_ui_surface_draw_point_t position;
    mla_ui_surface_input_event_click_button button;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_ui_surface_input_event_click_t* self = mla_pointer_get_data<const mla_ui_surface_input_event_click_t>(obj);

        if (self == nullptr) {
            return false;
        }

        mla_serializer_write_struct(serializer, mla_string_const("position"), self->position, mla_ui_surface_draw_point_t);
        mla_serializer_write_enum  (serializer, mla_string_const("button"),   self->button);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_ui_surface_input_event_click_t* self = mla_pointer_get_data<mla_ui_surface_input_event_click_t>(obj);

        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "position")) {
            mla_deserializer_read_struct(deserializer, self->position, mla_ui_surface_draw_point_t);
        } else if (mla_string_equals_const(property_name, "button")) {
            mla_deserializer_read_enum(mla_ui_surface_input_event_click_button, deserializer, self->button);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }
};

enum mla_ui_surface_input_event_char_input_kind : mla_uint8_t {
    MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_INPUT,
    MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_ENTER,
    MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_BACKSPACE,
    MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_DELETE,
    MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_TAB,
    MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_ESCAPE,
    MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_ARROW_UP,
    MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_ARROW_DOWN,
    MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_ARROW_LEFT,
    MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_ARROW_RIGHT,
};

enum surface_input_event_spical_control_char_kind: mla_uint8_t {
    MLA_UI_SURFACE_INPUT_EVENT_KIND_CONTROL_NONE = 0,
    MLA_UI_SURFACE_INPUT_EVENT_KIND_CONTROL_SHIFT,
    MLA_UI_SURFACE_INPUT_EVENT_KIND_CONTROL_CTRL,
    MLA_UI_SURFACE_INPUT_EVENT_KIND_CONTROL_ALT,
};

struct mla_ui_surface_input_event_char_input_t {
    mla_ui_surface_input_event_char_input_kind kind;
    mla_char_t character[4];
    surface_input_event_spical_control_char_kind pressedControlKeys;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_ui_surface_input_event_char_input_t* self = mla_pointer_get_data<const mla_ui_surface_input_event_char_input_t>(obj);

        if (self == nullptr) {
            return false;
        }

        mla_serializer_write_enum  (serializer, mla_string_const("kind"),               self->kind);
        mla_serializer_write_uint8 (serializer, mla_string_const("character0"),         mla_s_cast<mla_uint8_t>(self->character[0]));
        mla_serializer_write_uint8 (serializer, mla_string_const("character1"),         mla_s_cast<mla_uint8_t>(self->character[1]));
        mla_serializer_write_uint8 (serializer, mla_string_const("character2"),         mla_s_cast<mla_uint8_t>(self->character[2]));
        mla_serializer_write_uint8 (serializer, mla_string_const("character3"),         mla_s_cast<mla_uint8_t>(self->character[3]));
        mla_serializer_write_enum  (serializer, mla_string_const("pressedControlKeys"), self->pressedControlKeys);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_ui_surface_input_event_char_input_t* self = mla_pointer_get_data<mla_ui_surface_input_event_char_input_t>(obj);

        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "kind")) {
            mla_deserializer_read_enum(mla_ui_surface_input_event_char_input_kind, deserializer, self->kind);
        } else if (mla_string_equals_const(property_name, "character0")) {
            mla_deserializer_read_uint8(deserializer, self->character[0]);
        } else if (mla_string_equals_const(property_name, "character1")) {
            mla_deserializer_read_uint8(deserializer, self->character[1]);
        } else if (mla_string_equals_const(property_name, "character2")) {
            mla_deserializer_read_uint8(deserializer, self->character[2]);
        } else if (mla_string_equals_const(property_name, "character3")) {
            mla_deserializer_read_uint8(deserializer, self->character[3]);
        } else if (mla_string_equals_const(property_name, "pressedControlKeys")) {
            mla_deserializer_read_enum(surface_input_event_spical_control_char_kind, deserializer, self->pressedControlKeys);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }
};

enum mla_ui_surface_input_event_kind : mla_uint8_t {
    MLA_UI_SURFACE_INPUT_EVENT_KIND_NONE,
    MLA_UI_SURFACE_INPUT_EVENT_KIND_CLICK,
    MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR,
};

struct mla_ui_surface_input_event_t;
mla_ui_surface_input_event_t mla_ui_surface_input_event_empty();

struct mla_ui_surface_input_event_t {
    mla_ui_surface_input_event_kind kind;

    union {
        mla_ui_surface_input_event_click_t click;
        mla_ui_surface_input_event_char_input_t char_input;
    };

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_ui_surface_input_event_t* self = mla_pointer_get_data<const mla_ui_surface_input_event_t>(obj);

        if (self == nullptr) {
            return false;
        }

        mla_serializer_write_enum(serializer, mla_string_const("kind"), self->kind);
        if (self->kind == MLA_UI_SURFACE_INPUT_EVENT_KIND_CLICK) {
            mla_serializer_write_struct(serializer, mla_string_const("click"), self->click, mla_ui_surface_input_event_click_t);
        } else if (self->kind == MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR) {
            mla_serializer_write_struct(serializer, mla_string_const("charInput"), self->char_input, mla_ui_surface_input_event_char_input_t);
        }
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_ui_surface_input_event_t* self = mla_pointer_get_data<mla_ui_surface_input_event_t>(obj);

        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "kind")) {
            mla_deserializer_read_enum(mla_ui_surface_input_event_kind, deserializer, self->kind);
        } else if (mla_string_equals_const(property_name, "click")) {
            mla_deserializer_read_struct(deserializer, self->click, mla_ui_surface_input_event_click_t);
        } else if (mla_string_equals_const(property_name, "charInput")) {
            mla_deserializer_read_struct(deserializer, self->char_input, mla_ui_surface_input_event_char_input_t);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
        return MLA_DESERIALIZER_READ_HANDLED;
    }

    static mla_ui_surface_input_event_t init() {
        return mla_ui_surface_input_event_empty();
    }
};

#endif
