//
// XML Serializer/Deserializer implementation
// Simple XML format: simple types as attributes, complex types as nested tags
//
// Created by copilot on 12/30/2025.
//

#include "mla_xml_serializer.h"

#include "../log/mla_logging.h"
#include "../system/mla_number.h"
#include "../system/mla_string.h"
#include "../system/mla_string_concat.h"

///////////////////////////////////////////////////////////////////////////////
// XML Serializer
///////////////////////////////////////////////////////////////////////////////

#define mla_bytes_prefix "mla_bytes:"

mla_user_data_id_init(mla_xml_serializer_state_user_data_name)

struct mla_xml_serializer_state_t {
    mla_bool_t in_open_tag;
    mla_string_t pending_property_name;
    mla_array_list_t<mla_init_struct(mla_string_t)> tag_stack; // Stack to track tag names

    static mla_xml_serializer_state_t init() {
        return {
            false,
            mla_string_empty(),
            mla_array_list_empty<mla_init_struct(mla_string_t)>()
        };
    }
};


mla_xml_serializer_state_t *mla_private_xml_ser_get_state(mla_serializer_t &inst) {
    return mla_user_data_get_pointer_data<mla_xml_serializer_state_t>(inst.user_data, mla_xml_serializer_state_user_data_name);
}

mla_bool_t mla_private_xml_write_str(mla_stream_output_t &out, const mla_string_t& str) {
    return mla_stream_output_write_string(out, str);
}

mla_bool_t mla_private_xml_write_escaped(mla_stream_output_t &out, const mla_string_t &str) {

    const mla_char_t* data = mla_string_data(str);

    for (mla_size_t i = 0; i < mla_string_length(str); i++) {
        mla_char_t c = data[i];
        switch (c) {
            case '<':
                if (!mla_private_xml_write_str(out, mla_string_const("&lt;"))) {
                    return false;
                }
                break;
            case '>':
                if (!mla_private_xml_write_str(out, mla_string_const("&gt;"))) {
                    return false;
                }
                break;
            case '&':
                if (!mla_private_xml_write_str(out, mla_string_const("&amp;"))) {
                    return false;
                }
                break;
            case '"':
                if (!mla_private_xml_write_str(out, mla_string_const("&quot;"))) {
                    return false;
                }
                break;
            case '\'':
                if (!mla_private_xml_write_str(out, mla_string_const("&apos;"))) {
                    return false;
                }
                break;
            default:
                if (out.write(out, 0, 1, mla_r_cast<const mla_byte_t *>(&c)) != 1) {
                    return false;
                }
                break;
        }
    }
    return true;
}

mla_bool_t mla_private_xml_close_tag_if_open(mla_serializer_t &inst) {
    mla_xml_serializer_state_t *state = mla_private_xml_ser_get_state(inst);
    if (state == nullptr) {
        return false;
    }
    if (state->in_open_tag) {
        if (!mla_private_xml_write_str(inst.output, mla_string_const(">"))) {
            return false;
        }

        state->in_open_tag = false;
    }
    return true;
}

mla_bool_t mla_xml_serializer_write_start_struct(mla_serializer_t &inst) {
    if (!mla_private_xml_close_tag_if_open(inst)) {
        return false;
    }

    mla_xml_serializer_state_t *state = mla_private_xml_ser_get_state(inst);
    if (state == nullptr) {
        return false;
    }

    // Determine tag name: use property name if available, otherwise "item"
    mla_string_t tag_name = mla_string_empty();
    if (mla_string_length(state->pending_property_name) > 0) {
        tag_name = state->pending_property_name;
        state->pending_property_name = mla_string_empty();
    } else {
        tag_name = mla_string_const("item");
    }

    // Push tag name onto stack for closing
    mla_array_list_add(state->tag_stack, tag_name);

    // Write opening tag
    if (!mla_private_xml_write_str(inst.output, mla_string_const("<"))) {
        return false;
    }

    if (!mla_private_xml_write_escaped(inst.output, tag_name)) {
        return false;
    }

    state->in_open_tag = true;
    return true;
}

mla_bool_t mla_xml_serializer_write_end_struct(mla_serializer_t &inst) {
    mla_xml_serializer_state_t *state = mla_private_xml_ser_get_state(inst);

    // Get the tag name from the stack
    if (mla_array_list_size(state->tag_stack) == 0) {
        return false; // Error: unmatched end_struct
    }
    mla_string_t tag_name = *mla_array_list_get_ref(state->tag_stack, mla_array_list_size(state->tag_stack) - 1);
    mla_array_list_remove(state->tag_stack, mla_array_list_size(state->tag_stack) - 1);

    if (state->in_open_tag) {
        // Self-closing tag
        if (!mla_private_xml_write_str(inst.output, mla_string_const(" />"))) {
            return false;
        }

        state->in_open_tag = false;
    } else {
        // Closing tag
        if (!mla_private_xml_write_str(inst.output, mla_string_const("</"))) {
            return false;
        }

        if (!mla_private_xml_write_escaped(inst.output, tag_name)) {
            return false;
        }

        if (!mla_private_xml_write_str(inst.output, mla_string_const(">"))) {
            return false;
        }
    }

    // Clear pending property name to prevent it from being reused
    state->pending_property_name = mla_string_empty();

    return true;
}

mla_bool_t mla_xml_serializer_write_start_list(mla_serializer_t &inst) {

    if (!mla_private_xml_close_tag_if_open(inst)) {
        return false;
    }

    mla_xml_serializer_state_t *state = mla_private_xml_ser_get_state(inst);

    // Determine tag name: use property name if available, otherwise "list"
    mla_string_t tag_name = mla_string_empty();
    if (mla_string_length(state->pending_property_name) > 0) {
        tag_name = state->pending_property_name;
        state->pending_property_name = mla_string_empty();
    } else {
        tag_name = mla_string_const("list");
    }

    // Push tag name onto stack for closing
    mla_array_list_add(state->tag_stack, tag_name);

    // Write opening tag
    if (!mla_private_xml_write_str(inst.output, mla_string_const("<"))) {
        return false;
    }

    if (!mla_private_xml_write_escaped(inst.output, tag_name)) {
        return false;
    }

    if (!mla_private_xml_write_str(inst.output, mla_string_const(">"))) {
        return false;
    }

    return true;
}

mla_bool_t mla_xml_serializer_write_end_list(mla_serializer_t &inst) {
    mla_xml_serializer_state_t *state = mla_private_xml_ser_get_state(inst);

    // Get the tag name from the stack
    if (mla_array_list_size(state->tag_stack) == 0) {
        return false; // Error: unmatched end_list
    }
    mla_string_t tag_name = *mla_array_list_get_ref(state->tag_stack, mla_array_list_size(state->tag_stack) - 1);
    mla_array_list_remove(state->tag_stack, mla_array_list_size(state->tag_stack) - 1);

    // Write closing tag
    if (!mla_private_xml_write_str(inst.output, mla_string_const("</"))) {
        return false;
    }

    if (!mla_private_xml_write_escaped(inst.output, tag_name)) {
        return false;
    }

    if (!mla_private_xml_write_str(inst.output, mla_string_const(">"))) {
        return false;
    }

    return true;
}

mla_bool_t mla_xml_serializer_write_property_name(mla_serializer_t &inst, const mla_string_t &name) {
    mla_xml_serializer_state_t *state = mla_private_xml_ser_get_state(inst);
    state->pending_property_name = name;
    return true;
}

static mla_bool_t mla_private_xml_write_attr(mla_serializer_t &inst, const mla_string_t &value) {
    mla_xml_serializer_state_t *state = mla_private_xml_ser_get_state(inst);

    if (!mla_private_xml_write_str(inst.output, mla_string_const(" "))) {
        return false;
    }

    if (!mla_private_xml_write_escaped(inst.output, state->pending_property_name)) {
        return false;
    }

    if (!mla_private_xml_write_str(inst.output, mla_string_const("=\""))) {
        return false;
    }

    if (!mla_private_xml_write_escaped(inst.output, value)) {
        return false;
    }

    if (!mla_private_xml_write_str(inst.output, mla_string_const("\""))) {
        return false;
    }

    return true;
}

mla_bool_t mla_xml_serializer_write_bool(mla_serializer_t &inst, const mla_bool_t value) {
    mla_xml_serializer_state_t *state = mla_private_xml_ser_get_state(inst);

    if (mla_string_length(state->pending_property_name) > 0) {
        return mla_private_xml_write_attr(inst, value ? mla_string_const("true") : mla_string_const("false"));
    } else {
        if (!mla_private_xml_close_tag_if_open(inst)) {
            return false;
        }

        if (!mla_private_xml_write_str(inst.output, mla_string_const("<value>"))) {
            return false;
        }

        if (!mla_private_xml_write_str(inst.output, value ? mla_string_const("true") : mla_string_const("false"))) {
            return false;
        }

        if (!mla_private_xml_write_str(inst.output, mla_string_const("</value>"))) {
            return false;
        }

        return true;
    }
}

mla_bool_t mla_xml_serializer_write_int8(mla_serializer_t &inst, const mla_int8_t value) {
    mla_xml_serializer_state_t *state = mla_private_xml_ser_get_state(inst);

    if (mla_string_length(state->pending_property_name) > 0) {
        return mla_private_xml_write_attr(inst, mla_string_from_int8(value));
    } else {
        if (!mla_private_xml_close_tag_if_open(inst)) {
            return false;
        }

        if (!mla_private_xml_write_str(inst.output, mla_string_const("<value>"))) {
            return false;
        }

        mla_string_t str = mla_string_from_int8(value);
        mla_bool_t result = mla_private_xml_write_str(inst.output, str);


        if (!result) {
            return false;
        }

        if (!mla_private_xml_write_str(inst.output, mla_string_const("</value>"))) {
            return false;
        }

        return true;
    }
}

mla_bool_t mla_xml_serializer_write_int16(mla_serializer_t &inst, const mla_int16_t value) {
    mla_xml_serializer_state_t *state = mla_private_xml_ser_get_state(inst);

    if (mla_string_length(state->pending_property_name) > 0) {
        return mla_private_xml_write_attr(inst, mla_string_from_int16(value));
    } else {
        if (!mla_private_xml_close_tag_if_open(inst)) {
            return false;
        }

        if (!mla_private_xml_write_str(inst.output, mla_string_const("<value>"))) {
            return false;
        }

        mla_string_t str = mla_string_from_int16(value);
        mla_bool_t result = mla_private_xml_write_str(inst.output, str);


        if (!result) {
            return false;
        }

        if (!mla_private_xml_write_str(inst.output, mla_string_const("</value>"))) {
            return false;
        }

        return true;
    }
}

mla_bool_t mla_xml_serializer_write_int32(mla_serializer_t &inst, const mla_int32_t value) {
    mla_xml_serializer_state_t *state = mla_private_xml_ser_get_state(inst);

    if (mla_string_length(state->pending_property_name) > 0) {
        return mla_private_xml_write_attr(inst, mla_string_from_int32(value));
    } else {
        if (!mla_private_xml_close_tag_if_open(inst)) {
            return false;
        }

        if (!mla_private_xml_write_str(inst.output, mla_string_const("<value>"))) {
            return false;
        }

        mla_string_t str = mla_string_from_int32(value);
        mla_bool_t result = mla_private_xml_write_str(inst.output, str);


        if (!result) {
            return false;
        }

        if (!mla_private_xml_write_str(inst.output, mla_string_const("</value>"))) {
            return false;
        }

        return true;
    }
}

mla_bool_t mla_xml_serializer_write_int64(mla_serializer_t &inst, const mla_int64_t value) {
    mla_xml_serializer_state_t *state = mla_private_xml_ser_get_state(inst);

    if (mla_string_length(state->pending_property_name) > 0) {
        return mla_private_xml_write_attr(inst, mla_string_from_int64(value));
    } else {
        if (!mla_private_xml_close_tag_if_open(inst)) {
            return false;
        }

        if (!mla_private_xml_write_str(inst.output, mla_string_const("<value>"))) {
            return false;
        }

        mla_string_t str = mla_string_from_int64(value);
        mla_bool_t result = mla_private_xml_write_str(inst.output, str);

        if (!result) {
            return false;
        }

        if (!mla_private_xml_write_str(inst.output, mla_string_const("</value>"))) {
            return false;
        }

        return true;
    }
}

mla_bool_t mla_xml_serializer_write_uint8(mla_serializer_t &inst, const mla_uint8_t value) {
    mla_xml_serializer_state_t *state = mla_private_xml_ser_get_state(inst);

    if (mla_string_length(state->pending_property_name) > 0) {
        return mla_private_xml_write_attr(inst, mla_string_from_uint8(value));
    } else {
        if (!mla_private_xml_close_tag_if_open(inst)) {
            return false;
        }

        if (!mla_private_xml_write_str(inst.output, mla_string_const("<value>"))) {
            return false;
        }

        mla_string_t str = mla_string_from_uint8(value);
        mla_bool_t result = mla_private_xml_write_str(inst.output, str);

        if (!result) {
            return false;
        }

        if (!mla_private_xml_write_str(inst.output, mla_string_const("</value>"))) {
            return false;
        }

        return true;
    }
}

mla_bool_t mla_xml_serializer_write_uint16(mla_serializer_t &inst, const mla_uint16_t value) {
    mla_xml_serializer_state_t *state = mla_private_xml_ser_get_state(inst);

    if (mla_string_length(state->pending_property_name) > 0) {
        return mla_private_xml_write_attr(inst, mla_string_from_uint16(value));
    } else {
        if (!mla_private_xml_close_tag_if_open(inst)) {
            return false;
        }

        if (!mla_private_xml_write_str(inst.output, mla_string_const("<value>"))) {
            return false;
        }

        mla_string_t str = mla_string_from_uint16(value);
        mla_bool_t result = mla_private_xml_write_str(inst.output, str);

        if (!result) {
            return false;
        }

        if (!mla_private_xml_write_str(inst.output, mla_string_const("</value>"))) {
            return false;
        }

        return true;
    }
}

mla_bool_t mla_xml_serializer_write_uint32(mla_serializer_t &inst, const mla_uint32_t value) {
    mla_xml_serializer_state_t *state = mla_private_xml_ser_get_state(inst);

    if (mla_string_length(state->pending_property_name) > 0) {
        return mla_private_xml_write_attr(inst, mla_string_from_uint32(value));
    } else {
        if (!mla_private_xml_close_tag_if_open(inst)) {
            return false;
        }

        if (!mla_private_xml_write_str(inst.output, mla_string_const("<value>"))) {
            return false;
        }

        mla_string_t str = mla_string_from_uint32(value);
        mla_bool_t result = mla_private_xml_write_str(inst.output, str);

        if (!result) {
            return false;
        }

        if (!mla_private_xml_write_str(inst.output, mla_string_const("</value>"))) {
            return false;
        }

        return true;
    }
}

mla_bool_t mla_xml_serializer_write_uint64(mla_serializer_t &inst, const mla_uint64_t value) {
    mla_xml_serializer_state_t *state = mla_private_xml_ser_get_state(inst);

    if (mla_string_length(state->pending_property_name) > 0) {
        return mla_private_xml_write_attr(inst, mla_string_from_uint64(value));
    } else {
        if (!mla_private_xml_close_tag_if_open(inst)) {
            return false;
        }

        if (!mla_private_xml_write_str(inst.output, mla_string_const("<value>"))) {
            return false;
        }

        mla_string_t str = mla_string_from_uint64(value);
        mla_bool_t result = mla_private_xml_write_str(inst.output, str);

        if (!result) {
            return false;
        }

        if (!mla_private_xml_write_str(inst.output, mla_string_const("</value>"))) {
            return false;
        }

        return true;
    }
}

mla_bool_t mla_xml_serializer_write_float(mla_serializer_t &inst, const mla_float_t value) {
    mla_xml_serializer_state_t *state = mla_private_xml_ser_get_state(inst);

    if (mla_string_length(state->pending_property_name) > 0) {
        return mla_private_xml_write_attr(inst, mla_string_from_float(value, 6));
    } else {
        if (!mla_private_xml_close_tag_if_open(inst)) {
            return false;
        }

        if (!mla_private_xml_write_str(inst.output, mla_string_const("<value>"))) {
            return false;
        }

        mla_string_t str = mla_string_from_float(value, 6);
        mla_bool_t result = mla_private_xml_write_str(inst.output, str);

        if (!result) {
            return false;
        }

        if (!mla_private_xml_write_str(inst.output, mla_string_const("</value>"))) {
            return false;
        }

        return true;
    }
}

mla_bool_t mla_xml_serializer_write_double(mla_serializer_t &inst, const mla_double_t value) {
    mla_xml_serializer_state_t *state = mla_private_xml_ser_get_state(inst);

    if (mla_string_length(state->pending_property_name) > 0) {
        return mla_private_xml_write_attr(inst, mla_string_from_double(value, 6));
    } else {
        if (!mla_private_xml_close_tag_if_open(inst)) {
            return false;
        }

        if (!mla_private_xml_write_str(inst.output, mla_string_const("<value>"))) {
            return false;
        }

        mla_string_t str = mla_string_from_double(value, 6);
        mla_bool_t result = mla_private_xml_write_str(inst.output, str);

        if (!result) {
            return false;
        }

        if (!mla_private_xml_write_str(inst.output, mla_string_const("</value>"))) {
            return false;
        }

        return true;
    }
}

mla_bool_t mla_xml_serializer_write_string(mla_serializer_t &inst, const mla_string_t &value) {
    mla_xml_serializer_state_t *state = mla_private_xml_ser_get_state(inst);

    if (mla_string_length(state->pending_property_name) > 0) {
        return mla_private_xml_write_attr(inst, value);
    } else {
        if (!mla_private_xml_close_tag_if_open(inst)) {
            return false;
        }

        if (!mla_private_xml_write_str(inst.output, mla_string_const("<value>"))) {
            return false;
        }

        if (!mla_private_xml_write_escaped(inst.output, value)) {
            return false;
        }

        if (!mla_private_xml_write_str(inst.output, mla_string_const("</value>"))) {
            return false;
        }

        return true;
    }
}

mla_bool_t mla_xml_serializer_write_bytes(mla_serializer_t &inst, const mla_bytes_t &value) {
    mla_xml_serializer_state_t *state = mla_private_xml_ser_get_state(inst);
    mla_string_t base64 = mla_bytes_to_base64(value);

    if (mla_string_length(state->pending_property_name) > 0) {
        if (!mla_private_xml_write_str(inst.output, mla_string_const(" "))) {
            return false;
        }

        if (!mla_private_xml_write_escaped(inst.output, state->pending_property_name)) {
            return false;
        }

        if (!mla_private_xml_write_str(inst.output, mla_string_const("=\""))) {
            return false;
        }

        if (!mla_private_xml_write_escaped(inst.output, mla_string_const(mla_bytes_prefix))) {
            return false;
        }

        if (!mla_private_xml_write_escaped(inst.output, base64)) {
            return false;
        }

        if (!mla_private_xml_write_str(inst.output, mla_string_const("\""))) {
            return false;
        }

        return true;
    } else {
        if (!mla_private_xml_close_tag_if_open(inst)) {
            return false;
        }

        if (!mla_private_xml_write_str(inst.output, mla_string_const("<value>"))) {
            return false;
        }

        if (!mla_private_xml_write_escaped(inst.output, mla_string_const(mla_bytes_prefix))) {
            return false;
        }

        if (!mla_private_xml_write_escaped(inst.output, base64)) {
            return false;
        }

        if (!mla_private_xml_write_str(inst.output, mla_string_const("</value>"))) {
            return false;
        }

        return true;
    }
}


mla_serializer_t mla_xml_serializer(const mla_stream_output_t &output) {
    // Note: state memory is allocated but framework has no destructor pattern
    // This is consistent with the overall framework design where serializers
    // are expected to be short-lived
    mla_pointer_t state_ptr = mla_malloc_struct(mla_xml_serializer_state_t);

    mla_xml_serializer_state_t *state = mla_pointer_get_data<mla_xml_serializer_state_t>(state_ptr);

    if (state == nullptr) {
        return mla_serializer_invalid();
    }

    mla_memset(state, 0, sizeof(mla_xml_serializer_state_t));
    state->in_open_tag = false;
    state->pending_property_name = mla_string_empty();
    state->tag_stack = mla_array_list<mla_init_struct(mla_string_t)>();

    mla_user_data_t user_data = mla_user_data_empty();
    mla_user_data_set_pointer(user_data, mla_xml_serializer_state_user_data_name, state_ptr);

    return {
        output,
        user_data,
        mla_xml_serializer_write_start_struct,
        mla_xml_serializer_write_end_struct,
        mla_xml_serializer_write_start_list,
        mla_xml_serializer_write_end_list,
        mla_xml_serializer_write_property_name,
        mla_xml_serializer_write_bool,
        mla_xml_serializer_write_int8,
        mla_xml_serializer_write_int16,
        mla_xml_serializer_write_int32,
        mla_xml_serializer_write_int64,
        mla_xml_serializer_write_uint8,
        mla_xml_serializer_write_uint16,
        mla_xml_serializer_write_uint32,
        mla_xml_serializer_write_uint64,
        mla_xml_serializer_write_float,
        mla_xml_serializer_write_double,
        mla_xml_serializer_write_string,
        mla_xml_serializer_write_bytes
    };
}

///////////////////////////////////////////////////////////////////////////////
// XML Deserializer
/**
 * @struct mla_xml_attr_t
 * @brief Represents a single XML attribute with its name and value.
 *
 * This structure is used to define an attribute within an XML element.
 * It consists of two members:
 * - `name`: The name of the attribute as a string.
 * - `value`: The value of the attribute as a string.
 *
 * Attributes are generally parsed and processed during XML serialization
 * or deserialization.
 */

struct mla_xml_attr_t {
    mla_string_t name;
    mla_string_t value;

    static mla_xml_attr_t init() {
        return {
            mla_string_empty(),
            mla_string_empty()
        };

    }
};

struct mla_xml_deser_state_t {
    mla_array_list_t<mla_bool_t> container_is_list_stack; // Track if each open container is a list
    mla_array_list_t<mla_xml_attr_t, mla_xml_attr_t> attrs;
    mla_string_t pending_tag_name; // Tag name waiting to be processed as property name
    mla_size_t attr_idx;
    mla_size_t buffered_count; // Number of buffered characters
    mla_char_t buffered_chars[16]; // Multi-character lookahead buffer
    mla_bool_t returning_attr_val;
    mla_bool_t pending_tag_is_struct;
    mla_bool_t pending_tag_is_list;
    mla_bool_t pending_struct_end;

    static mla_xml_deser_state_t init() {
        return {
            mla_array_list_empty<mla_bool_t>(),
            mla_array_list_empty<mla_xml_attr_t, mla_xml_attr_t>(),
            mla_string_empty(),
            0,
            0,
            {0},
            false,
            false,
            false,
            false
        };
    }
};

mla_user_data_id_init(mla_deserializer_state_user_data_name)

static mla_xml_deser_state_t *mla_private_xml_deser_get_state(mla_deserializer_t &inst) {
    return mla_user_data_get_pointer_data<mla_xml_deser_state_t>(inst.user_data, mla_deserializer_state_user_data_name);
}

static mla_bool_t mla_private_xml_read_char(mla_deserializer_t &inst, mla_char_t &c) {
    mla_xml_deser_state_t *state = mla_private_xml_deser_get_state(inst);
    if (state == nullptr) {
        return false;
    }

    if (state->buffered_count > 0) {
        c = state->buffered_chars[--state->buffered_count];
        return true;
    }

    return inst.input.read(inst.input, 0, 1, mla_r_cast<mla_byte_t *>(&c)) == 1;
}

static void mla_private_xml_unread_char(mla_deserializer_t &inst, mla_char_t c) {
    mla_xml_deser_state_t *state = mla_private_xml_deser_get_state(inst);
    if (state != nullptr && state->buffered_count < 16) {
        state->buffered_chars[state->buffered_count++] = c;
    }
}

static void mla_private_xml_skip_ws(mla_deserializer_t &inst) {
    mla_char_t c;
    while (mla_private_xml_read_char(inst, c)) {
        if (c != ' ' && c != '\t' && c != '\r' && c != '\n') {
            mla_private_xml_unread_char(inst, c);
            break;
        }
    }
}

static mla_string_t mla_private_xml_read_until(mla_deserializer_t &inst, mla_char_t stop) {
    mla_string_t result = mla_string_empty();
    mla_char_t buf[mla_global_config_stream_fast_read_buffer_size];
    mla_size_t pos = 0;
    mla_char_t c;

    while (mla_private_xml_read_char(inst, c)) {
        if (c == stop) {
            mla_private_xml_unread_char(inst, c);
            break;
        }

        if (pos >= sizeof(buf) - 1) {
            result = mla_string_concat(result, mla_string(mla_platform_pointer_to_managed_pointer(buf), pos));
            pos = 0;
        }
        buf[pos++] = c;
    }

    if (pos > 0) {
        result = mla_string_concat(result, mla_string(mla_platform_pointer_to_managed_pointer(buf), pos));
    }

    return result;
}

static mla_string_t mla_private_xml_unescape(const mla_string_t &str) {
    mla_string_t result = mla_string_empty();

    mla_size_t str_length = mla_string_length(str);
    const mla_char_t* str_data = mla_string_data(str);

    for (mla_size_t i = 0; i < str_length; i++) {
        if (str_data[i] == '&') {
            if (i + 4 <= str_length && mla_memcmp(&str_data[i], "&lt;", 4) == 0) {
                result = mla_string_concat(result, mla_string_const("<"));
                i += 3;
            } else if (i + 4 <= str_length && mla_memcmp(&str_data[i], "&gt;", 4) == 0) {
                result = mla_string_concat(result, mla_string_const(">"));
                i += 3;
            } else if (i + 5 <= str_length && mla_memcmp(&str_data[i], "&amp;", 5) == 0) {
                result = mla_string_concat(result, mla_string_const("&"));
                i += 4;
            } else if (i + 6 <= str_length && mla_memcmp(&str_data[i], "&quot;", 6) == 0) {
                result = mla_string_concat(result, mla_string_const("\""));
                i += 5;
            } else if (i + 6 <= str_length && mla_memcmp(&str_data[i], "&apos;", 6) == 0) {
                result = mla_string_concat(result, mla_string_const("'"));
                i += 5;
            } else {
                result = mla_string_concat(result, mla_string(mla_platform_pointer_to_managed_pointer(&str_data[i]), 1));
            }
        } else {
            result = mla_string_concat(result, mla_string(mla_platform_pointer_to_managed_pointer(&str_data[i]), 1));
        }
    }

    return result;
}

static void mla_private_xml_parse_val(mla_deserializer_t &inst, const mla_string_t &val_str) {
    if (mla_string_equals_const(val_str, "true")) {
        inst.current_token.type = MLA_DESERIALIZER_VALUE_BOOL;
        inst.current_token.simple.bool_value = true;
    } else if (mla_string_equals_const(val_str, "false")) {
        inst.current_token.type = MLA_DESERIALIZER_VALUE_BOOL;
        inst.current_token.simple.bool_value = false;
    } else {
        mla_int64_t int_val;
        mla_double_t dbl_val;

        mla_string_t byte_prefix = mla_string_const(mla_bytes_prefix);

        if (mla_parse_int64(val_str, int_val)) {
            if (int_val >= mla_int8_min && int_val <= mla_int8_max) {
                inst.current_token.type = MLA_DESERIALIZER_VALUE_INT8;
                inst.current_token.simple.int8_value = mla_s_cast<mla_int8_t>(int_val);
            } else if (int_val >= mla_int16_min && int_val <= mla_int16_max) {
                inst.current_token.type = MLA_DESERIALIZER_VALUE_INT16;
                inst.current_token.simple.int16_value = mla_s_cast<mla_int16_t>(int_val);
            } else if (int_val >= mla_int32_min && int_val <= mla_int32_max) {
                inst.current_token.type = MLA_DESERIALIZER_VALUE_INT32;
                inst.current_token.simple.int32_value = mla_s_cast<mla_int32_t>(int_val);
            } else {
                inst.current_token.type = MLA_DESERIALIZER_VALUE_INT64;
                inst.current_token.simple.int64_value = int_val;
            }
        } else if (mla_parse_double(val_str, dbl_val)) {
            inst.current_token.type = MLA_DESERIALIZER_VALUE_DOUBLE;
            inst.current_token.simple.double_value = dbl_val;
        } else if (mla_string_starts_with(val_str, byte_prefix)) {
            mla_string_t base64Part = mla_string_substr(val_str, mla_string_length(byte_prefix));
            inst.current_token.type = MLA_DESERIALIZER_VALUE_BYTES;
            inst.current_token.complex.bytes_value = mla_bytes_from_base64(base64Part);
        } else {
            inst.current_token.type = MLA_DESERIALIZER_VALUE_STRING;
            inst.current_token.complex.string_value = val_str;
        }
    }
}

mla_bool_t mla_xml_deserializer_read_next(mla_deserializer_t &inst) {
    mla_xml_deser_state_t *state = mla_private_xml_deser_get_state(inst);
    if (state == nullptr) {
        inst.current_token.type = MLA_DESERIALIZER_STRUCT_END;
        return false;
    }

    inst.current_token.complex = {mla_string_empty(), mla_string_empty(), mla_bytes_empty()};
    inst.current_token.simple = {false};

    // If we have a pending property tag, return the struct/list start now
    if (state->pending_tag_is_struct || state->pending_tag_is_list) {
        if (state->pending_tag_is_struct) {
            inst.current_token.type = MLA_DESERIALIZER_STRUCT_START;
            state->attr_idx = 0;
            state->pending_tag_is_struct = false;
        } else {
            inst.current_token.type = MLA_DESERIALIZER_LIST_START;
            state->pending_tag_is_list = false;
        }
        state->pending_tag_name = mla_string_empty();
        return true;
    }

    // If we just returned a property name from attribute, return the value
    if (state->returning_attr_val) {
        mla_xml_attr_t *attr = mla_array_list_get_ref(state->attrs, state->attr_idx - 1);
        mla_string_t unesc = mla_private_xml_unescape(attr->value);
        mla_private_xml_parse_val(inst, unesc);
        state->returning_attr_val = false;
        return true;
    }

    // Check if we have more attributes to process
    if (state->attr_idx < mla_array_list_size(state->attrs)) {
        mla_xml_attr_t *attr = mla_array_list_get_ref(state->attrs, state->attr_idx);
        state->attr_idx++;

        inst.current_token.type = MLA_DESERIALIZER_PROPERTY_NAME;
        inst.current_token.complex.property_name = attr->name;
        state->returning_attr_val = true;
        return true;
    }

    // NOW check if we need to return a struct end for a self-closing tag (after all attributes are processed)
    if (state->pending_struct_end) {
        state->pending_struct_end = false;
        inst.current_token.type = MLA_DESERIALIZER_STRUCT_END;
        return true;
    }

    mla_private_xml_skip_ws(inst);

    mla_char_t c;
    if (!mla_private_xml_read_char(inst, c)) {
        return false;
    }

    if (c != '<') {
        return false;
    }

    if (!mla_private_xml_read_char(inst, c)) {
        return false;
    }

    if (c == '/') {
        // Closing tag
        mla_string_t tag_name = mla_private_xml_read_until(inst, '>');
        mla_private_xml_read_char(inst, c); // consume '>'

        if (mla_string_equals_const(tag_name, "value")) {
            return mla_xml_deserializer_read_next(inst);
        }

        if (mla_string_equals_const(tag_name, "item")) {
            inst.current_token.type = MLA_DESERIALIZER_STRUCT_END;
            return true;
        }

        // Pop from container stack to determine type
        if (mla_array_list_size(state->container_is_list_stack) > 0) {
            mla_bool_t was_list = *mla_array_list_get_ref(state->container_is_list_stack, mla_array_list_size(state->container_is_list_stack) - 1);
            mla_array_list_remove(state->container_is_list_stack, mla_array_list_size(state->container_is_list_stack) - 1);

            inst.current_token.type = was_list ? MLA_DESERIALIZER_LIST_END : MLA_DESERIALIZER_STRUCT_END;
        } else {
            inst.current_token.type = MLA_DESERIALIZER_STRUCT_END;
        }

        return true;
    }

    mla_private_xml_unread_char(inst, c);

    // Read tag name
    mla_string_t tag_name = mla_string_empty(); {
        mla_char_t buf[256];
        mla_size_t pos = 0;

        while (mla_private_xml_read_char(inst, c)) {
            if (c == ' ' || c == '>' || c == '/') {
                mla_private_xml_unread_char(inst, c);
                break;
            }
            if (pos >= 255) {
                tag_name = mla_string_concat(tag_name, mla_string(mla_platform_pointer_to_managed_pointer(buf), pos));
                pos = 0;
            }
            buf[pos++] = c;
        }
        if (pos > 0) {
            tag_name = mla_string_concat(tag_name, mla_string(mla_platform_pointer_to_managed_pointer(buf), pos));
        }
    }

    // Parse attributes
    mla_array_list_clear(state->attrs);

    while (true) {
        mla_private_xml_skip_ws(inst);

        if (!mla_private_xml_read_char(inst, c)) {
            break;
        }

        if (c == '>' || c == '/') {
            mla_private_xml_unread_char(inst, c);
            break;
        }

        mla_private_xml_unread_char(inst, c);

        mla_string_t attr_name = mla_private_xml_read_until(inst, '=');
        mla_private_xml_read_char(inst, c); // consume '='

        mla_private_xml_skip_ws(inst);
        mla_private_xml_read_char(inst, c); // quote

        mla_string_t attr_val = mla_private_xml_read_until(inst, c);
        mla_private_xml_read_char(inst, c); // consume quote

        mla_xml_attr_t attr = {attr_name, attr_val};
        mla_array_list_add(state->attrs, attr);
    }

    if (!mla_private_xml_read_char(inst, c)) {
        return false;
    }

    if (c == '/') {
        // Self-closing tag with attributes
        mla_private_xml_read_char(inst, c); // consume '>'

        if (mla_string_equals_const(tag_name, "item")) {
            // Self-closing <item/> in a list - return STRUCT_START, attributes will follow
            inst.current_token.type = MLA_DESERIALIZER_STRUCT_START;
            state->attr_idx = 0;
            state->pending_struct_end = true;
            return true;
        } else if (mla_string_equals_const(tag_name, "value")) {
            // Self-closing value - shouldn't happen but handle it
            inst.current_token.type = MLA_DESERIALIZER_VALUE_STRING;
            inst.current_token.complex.string_value = mla_string_empty();
            return true;
        } else {
            // Self-closing property tag like <innerStruct int32Value="0" boolValue="false" />
            // First return PROPERTY_NAME for "innerStruct"
            inst.current_token.type = MLA_DESERIALIZER_PROPERTY_NAME;
            inst.current_token.complex.property_name = tag_name;

            // Mark that next call should return STRUCT_START
            state->pending_tag_is_struct = true;
            state->pending_struct_end = true;
            state->attr_idx = 0; // Reset attribute index for processing

            return true;
        }
    } else if (c == '>') {
        // Opening tag
        if (mla_string_equals_const(tag_name, "item")) {
            // Always a struct element
            inst.current_token.type = MLA_DESERIALIZER_STRUCT_START;
            state->attr_idx = 0;
        } else if (mla_string_equals_const(tag_name, "value")) {
            // Primitive value - read content directly
            mla_string_t content = mla_private_xml_read_until(inst, '<');
            mla_private_xml_read_char(inst, c); // '<'
            mla_private_xml_read_char(inst, c); // '/'
            mla_string_t closing = mla_private_xml_read_until(inst, '>');
            mla_private_xml_read_char(inst, c); // '>'

            mla_string_t unesc = mla_private_xml_unescape(content);
            mla_private_xml_parse_val(inst, unesc);
            return true;
        } else if (mla_string_equals_const(tag_name, "list")) {
            inst.current_token.type = MLA_DESERIALIZER_LIST_START;
        } else {
            // Property tag - return the property name now
            inst.current_token.type = MLA_DESERIALIZER_PROPERTY_NAME;
            inst.current_token.complex.property_name = tag_name;

            // Peek ahead to determine if it's a list (next tag is <item> or <value>) or struct
            mla_private_xml_skip_ws(inst);

            mla_char_t peek_chars[6];
            mla_size_t peek_count = 0;
            mla_bool_t is_list = false;

            // Try to read "<item" or "<value"
            if (mla_private_xml_read_char(inst, peek_chars[0])) {
                peek_count = 1;
                if (peek_chars[0] == '<' && mla_private_xml_read_char(inst, peek_chars[1])) {
                    peek_count = 2;
                    if (peek_chars[1] == 'i') {
                        // Check for "<item"
                        if (mla_private_xml_read_char(inst, peek_chars[2])) {
                            peek_count = 3;
                            if (peek_chars[2] == 't' && mla_private_xml_read_char(inst, peek_chars[3])) {
                                peek_count = 4;
                                if (peek_chars[3] == 'e' && mla_private_xml_read_char(inst, peek_chars[4])) {
                                    peek_count = 5;
                                    if (peek_chars[4] == 'm') {
                                        is_list = true;
                                    }
                                }
                            }
                        }
                    } else if (peek_chars[1] == 'v') {
                        // Check for "<value"
                        if (mla_private_xml_read_char(inst, peek_chars[2])) {
                            peek_count = 3;
                            if (peek_chars[2] == 'a' && mla_private_xml_read_char(inst, peek_chars[3])) {
                                peek_count = 4;
                                if (peek_chars[3] == 'l' && mla_private_xml_read_char(inst, peek_chars[4])) {
                                    peek_count = 5;
                                    if (peek_chars[4] == 'u' && mla_private_xml_read_char(inst, peek_chars[5])) {
                                        peek_count = 6;
                                        if (peek_chars[5] == 'e') {
                                            is_list = true;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Unread all peeked characters in reverse order
            for (mla_size_t i = peek_count; i > 0; i--) {
                mla_private_xml_unread_char(inst, peek_chars[i - 1]);
            }

            if (is_list) {
                state->pending_tag_is_list = true;
                mla_array_list_add(state->container_is_list_stack, true);
            } else {
                state->pending_tag_is_struct = true;
                mla_array_list_add(state->container_is_list_stack, false);
            }
            state->pending_tag_name = tag_name;
        }

        return true;
    }

    return false;
}

mla_deserializer_t mla_xml_deserializer(const mla_stream_input_t &input) {
    // Note: state memory is allocated but framework has no destructor pattern
    // This is consistent with the overall framework design where deserializers
    // are expected to be short-lived
    mla_pointer_t state_ptr = mla_malloc_struct(mla_xml_deser_state_t);

    mla_xml_deser_state_t *state = mla_pointer_get_data<mla_xml_deser_state_t>(state_ptr);

    if (state == nullptr) {
        return mla_deserializer_invalid();
    }

    mla_memset(state, 0, sizeof(mla_xml_deser_state_t));

    state->attrs = mla_array_list<mla_xml_attr_t, mla_xml_attr_t>();
    state->attr_idx = 0;
    state->returning_attr_val = false;
    state->buffered_count = 0;
    state->pending_tag_name = mla_string_empty();
    state->pending_tag_is_struct = false;
    state->pending_tag_is_list = false;
    state->pending_struct_end = false;

    mla_user_data_t user_data = mla_user_data_empty();
    mla_user_data_set_pointer(user_data, mla_deserializer_state_user_data_name, state_ptr);

    return {
        input,
        user_data,
        {MLA_DESERIALIZER_NULL, {mla_string_empty(), mla_string_empty(), mla_bytes_empty()}, {false}},
        mla_xml_deserializer_read_next
    };
}
