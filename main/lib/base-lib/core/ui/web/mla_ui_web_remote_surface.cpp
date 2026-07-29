//
// Created by chris on 2/23/2026.
//

#include "mla_ui_web_remote_surface.h"

#include "../../serializer/mla_json_serializer.h"

#define mla_ui_web_remote_surface_client_text_max_characters (128 + 1)
#define mla_ui_web_remote_surface_client_fps_target 30


struct mla_ui_web_remote_surface_client_text_size_t {
    mla_ui_surface_font_type_t fontType;
    // each element contains the size of the corresponding UTF-8 character we only handle the printable ASCII characters for now, so this will be the size of each character in the input text. We start the array at index 32 (space character) and end at index 126 (~ character).
    // Characters outside this range will use the 128th element as a fallback size.
    mla_ui_surface_draw_size_t size_per_char[mla_ui_web_remote_surface_client_text_max_characters];

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_ui_web_remote_surface_client_text_size_t* self = mla_pointer_get_data<const mla_ui_web_remote_surface_client_text_size_t>(obj);

        if (self == nullptr) {
            return false;
        }

        mla_serializer_write_struct(serializer, mla_string_const("fontType"), self->fontType, mla_ui_surface_font_type_t);
        mla_serializer_write_list_struct_fixed_size(serializer, mla_string_const("size_per_char"), self->size_per_char, mla_ui_surface_draw_size_t);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_ui_web_remote_surface_client_text_size_t* self = mla_pointer_get_data<mla_ui_web_remote_surface_client_text_size_t>(obj);

        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "fontType")) {
            mla_deserializer_read_struct(deserializer, self->fontType, mla_ui_surface_font_type_t);
        } else if (mla_string_equals_const(property_name, "size_per_char")) {
            mla_deserializer_read_list_struct_fixed_size(deserializer, self->size_per_char, mla_ui_surface_draw_size_t);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
        return MLA_DESERIALIZER_READ_HANDLED;
    }

    static mla_ui_web_remote_surface_client_text_size_t init() {
        return { mla_ui_surface_font_type_empty(), {} };
    }
};

struct mla_ui_web_remote_surface_client_message_t {

    mla_ui_surface_size_t surface_size;
    mla_array_list_t<mla_init_struct(mla_ui_surface_input_event_t)> inputEvents;
    mla_ui_surface_input_states_t inputStates;
    mla_array_list_t<mla_init_struct(mla_ui_web_remote_surface_client_text_size_t)> textSize;


    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_ui_web_remote_surface_client_message_t* self = mla_pointer_get_data<const mla_ui_web_remote_surface_client_message_t>(obj);

        if (self == nullptr) {
            return false;
        }

        mla_serializer_write_struct(serializer, mla_string_const("surface_size"), self->surface_size, mla_ui_surface_size_t);
        mla_serializer_write_list_struct(serializer, mla_string_const("inputEvents"),   self->inputEvents,   mla_ui_surface_input_event_t);
        mla_serializer_write_struct(serializer, mla_string_const("inputStates"),  self->inputStates,  mla_ui_surface_input_states_t);
        mla_serializer_write_list_struct(serializer, mla_string_const("textSize"),     self->textSize,     mla_ui_web_remote_surface_client_text_size_t);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        mla_ui_web_remote_surface_client_message_t* self = mla_pointer_get_data<mla_ui_web_remote_surface_client_message_t>(obj);

        if (self == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "surface_size")) {
            mla_deserializer_read_struct(deserializer, self->surface_size, mla_ui_surface_size_t);
        } else if (mla_string_equals_const(property_name, "inputEvents")) {
            mla_deserializer_read_list_struct(deserializer, self->inputEvents, mla_ui_surface_input_event_t);
        } else if (mla_string_equals_const(property_name, "inputStates")) {
            mla_deserializer_read_struct(deserializer, self->inputStates, mla_ui_surface_input_states_t);
        } else if (mla_string_equals_const(property_name, "textSize")) {
            mla_deserializer_read_list_struct(deserializer, self->textSize, mla_ui_web_remote_surface_client_text_size_t);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }

};

mla_ui_web_remote_surface_client_message_t mla_ui_web_remote_surface_client_message_empty() {
    return {
        {0, 0},
        mla_array_list_empty<mla_init_struct(mla_ui_surface_input_event_t)>(),
        mla_ui_surface_input_states_empty(),
        mla_array_list_empty<mla_init_struct(mla_ui_web_remote_surface_client_text_size_t)>()
    };
}

struct mla_ui_web_remote_surface_data_t {
    mla_string_t connectionId; // We can not store the connection pointer directly in the surface resource because it may become invalid if the connection is closed. Instead, we can store the connection ID and look up the connection when needed.
    mla_http_server_t *server; // We need a reference to the server to look up the connection
    mla_ui_surface_size_t lastSurfaceSize; // We can store the last surface size to detect when it changes, since we can't rely on the client to tell us when it changes
    mla_array_list_t<mla_init_struct(mla_ui_web_remote_surface_client_text_size_t)> textSizeCache; // We can cache the text sizes for different font types to avoid recalculating them every frame
    mla_ui_surface_input_states_t lastInputStates;

    static mla_ui_web_remote_surface_data_t init() {
        return {
            mla_string_empty(),
            nullptr,
            {0, 0},
            mla_array_list_empty<mla_init_struct(mla_ui_web_remote_surface_client_text_size_t)>(),
            mla_ui_surface_input_states_empty()
        };
    }
};

struct mla_ui_web_remote_surface_server_message_t {
    const mla_array_list_t<mla_init_struct(mla_ui_surface_draw_command_t)>& drawCommands;
    mla_uint64_t timestamp;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const mla_ui_web_remote_surface_server_message_t* in = mla_pointer_get_data<const mla_ui_web_remote_surface_server_message_t>(obj);

        if (in == nullptr) {
            return false;
        }

        mla_serializer_write_list_struct(serializer, mla_string_const("drawCommands"), in->drawCommands, mla_ui_surface_draw_command_t);
        mla_serializer_write_uint64(serializer, mla_string_const("timestamp"), in->timestamp);
        return true;
    }

};

mla_ui_surface_size_t mla_private_ui_web_remote_surface_get_size(const mla_ui_surface_t& surface) {

    mla_ui_web_remote_surface_data_t* surfaceData = mla_pointer_get_data<mla_ui_web_remote_surface_data_t>(surface.resource);

    if (surfaceData == nullptr) {
        return {
            0, 0
        };
    }

    return surfaceData->lastSurfaceSize;
}

mla_bool_t mla_private_ui_web_remote_surface_set_size(const mla_ui_surface_t& surface, mla_ui_surface_size_t size) {

    (void)surface;
    (void)size;
    // not possible
    return false;
}

mla_user_data_id_init(mla_ui_web_remote_surface_draw_commands_message_user_data_name)
mla_user_data_id_init(mla_ui_web_remote_surface_timestamp_user_data_name)

mla_bool_t mla_private_ui_web_remote_surface_render_draw_commands_text_message_generator(mla_stream_output_t& output, mla_user_data_t& user_data) {

    mla_array_list_t<mla_init_struct(mla_ui_surface_draw_command_t)>* drawCommands = mla_user_data_get_pointer_data<mla_array_list_t<mla_init_struct(mla_ui_surface_draw_command_t)>>(user_data, mla_ui_web_remote_surface_draw_commands_message_user_data_name);
    mla_uint64_t timestamp = mla_user_data_get_uint64(user_data, mla_ui_web_remote_surface_timestamp_user_data_name);

    if (drawCommands == nullptr) {
        return false; // No draw commands to serialize
    }

    mla_serializer_t serializer = mla_json_serializer(output);

    mla_ui_web_remote_surface_server_message_t drawData = {
        *drawCommands,
        timestamp
    };

    return mla_serializer_write_data_struct(serializer, drawData);

}

mla_bool_t mla_private_ui_web_remote_surface_render_draw_commands(const mla_ui_surface_t& surface, const mla_array_list_t<mla_init_struct(mla_ui_surface_draw_command_t)>& drawCommands, mla_array_list_t<mla_init_struct(mla_ui_surface_input_event_t)>& eventsSinceLastFame, mla_uint64_t timeSinceLastFrameMs) {

    (void)eventsSinceLastFame;

    if (timeSinceLastFrameMs < mla_s_cast<mla_uint64_t>(1000 * 0.75 / mla_ui_web_remote_surface_client_fps_target)) {
        return false; // Skip rendering to maintain target FPS
    }

    mla_ui_web_remote_surface_data_t* surface_data = mla_pointer_get_data<mla_ui_web_remote_surface_data_t>(surface.resource);

    if (surface_data == nullptr) {
        return false; // Can't render without a valid connection
    }

    mla_http_server_websocket_connection_t connection = mla_http_server_websocket_connection_invalid();

    if (!mla_http_server_find_websocket_connection(*surface_data->server, surface_data->connectionId, connection)) {
        return false; // Can't render if we can't find the connection
    }

    if (!mla_http_server_is_websocket_connection_open(connection)){
        return false; // Can't render if the connection is closed
    }

    mla_user_data_t messageData = mla_user_data_empty();
    mla_pointer_t drawCommands_ptr = mla_platform_pointer_to_managed_pointer(&drawCommands);
    mla_user_data_set_pointer(messageData, mla_ui_web_remote_surface_draw_commands_message_user_data_name, drawCommands_ptr);
    mla_user_data_set_uint64(messageData, mla_ui_web_remote_surface_timestamp_user_data_name, mla_system_time_ms());

    // Serialize the commands to JSON
    mla_bool_t sended = mla_http_server_try_send_websocket_text_message(connection, messageData, mla_private_ui_web_remote_surface_render_draw_commands_text_message_generator, mla_s_cast<mla_int32_t>(1000 / mla_ui_web_remote_surface_client_fps_target));
    return sended;

}

mla_ui_surface_draw_size_t mla_private_ui_web_remote_surface_render_calc_text_size(const mla_ui_surface_t &surface, const mla_ui_surface_font_type_t &font_type, const mla_string_t &text) {

    mla_ui_web_remote_surface_data_t* surface_data = mla_pointer_get_data<mla_ui_web_remote_surface_data_t>(surface.resource);
    if (surface_data == nullptr) {
        return {0, 0}; // Can't calculate text size without a valid connection
    }

    // Check if we have a cached size for this font type
    for (mla_size_t i = 0; i < mla_array_list_size(surface_data->textSizeCache); i++) {
        const mla_ui_web_remote_surface_client_text_size_t& cachedSize = mla_array_list_get_unsafe(surface_data->textSizeCache, i);
        if (mla_ui_surface_font_type_equals(cachedSize.fontType, font_type)) {

            // We have a cached size for this font type, now we need to calculate the size of the text based on the size of each character in the font type
            mla_ui_surface_draw_size_t textSize = {0, 0};

            const mla_char_t* char_data = mla_string_data(text);

            if (char_data == nullptr) {
                return textSize;
            }


            for (mla_size_t j = 0; j < mla_string_length(text); j++) {

                const mla_char_t c = char_data[j];

                mla_size_t charIndex = (c >= 32 && c <= 126) ? (c - 32) : (mla_ui_web_remote_surface_client_text_max_characters - 1);
                textSize.width += cachedSize.size_per_char[charIndex].width;
                textSize.height = mla_max(textSize.height, cachedSize.size_per_char[charIndex].height);
            }
            return textSize;
        }
    }

    return {0, 0}; // No cached size for this font type, we can't calculate the text size

}

mla_ui_surface_input_states_t mla_private_ui_web_remote_surface_get_input_states(const mla_ui_surface_t &surface) {

    mla_ui_web_remote_surface_data_t* surface_data = mla_pointer_get_data<mla_ui_web_remote_surface_data_t>(surface.resource);
    if (surface_data == nullptr) {
        return mla_ui_surface_input_states_empty();
    }

    return surface_data->lastInputStates; // We can return the last input states we received from the client, since we can't get real-time input states from the client

}

mla_ui_web_remote_surface_t mla_ui_web_remote_surface_invalid() {
    return {
        mla_ui_surface_invalid(),
        nullptr,
    };
}

mla_ui_web_remote_surface_message_result_t mla_ui_web_remote_surface_message_result_no_successful() {
    return {false, mla_array_list_empty<mla_init_struct(mla_ui_surface_input_event_t)>()};
}


mla_ui_web_remote_surface_message_result_t mla_private_ui_web_remote_surface_handle_client_text_message(mla_ui_web_remote_surface_t& surface, const mla_string_t& message) {

    mla_ui_web_remote_surface_data_t* data_client = mla_pointer_get_data<mla_ui_web_remote_surface_data_t>(surface.surface.resource);

    if (data_client == nullptr) {
        return mla_ui_web_remote_surface_message_result_no_successful();
    }

    mla_ui_web_remote_surface_client_message_t clientMessage = mla_ui_web_remote_surface_client_message_empty();

    mla_deserializer_t deserializer = mla_json_deserializer(mla_stream_input_from_string(message));

    if (!mla_serializer_read_data_struct(deserializer, clientMessage)) {
        return mla_ui_web_remote_surface_message_result_no_successful();
    }


    mla_ui_web_remote_surface_message_result_t result = {
        true, // We successfully handled the message
        clientMessage.inputEvents
    };

    // Update the surface data with the new input states and surface size from the client message
    data_client->lastInputStates = clientMessage.inputStates;
    data_client->lastSurfaceSize = clientMessage.surface_size;


    for (mla_size_t j = 0; j < mla_array_list_size(clientMessage.textSize); j++) {
        const mla_ui_web_remote_surface_client_text_size_t& textSize = mla_array_list_get_unsafe(clientMessage.textSize, j);

        // Update the text size cache with the new text size from the client message if it's not already in the cache
        bool foundFontTypeInCache = false;
        for (mla_size_t i = 0; i < mla_array_list_size(data_client->textSizeCache); i++) {
            const mla_ui_web_remote_surface_client_text_size_t& cachedSize = mla_array_list_get_unsafe(data_client->textSizeCache, i);
            if (mla_ui_surface_font_type_equals(cachedSize.fontType, textSize.fontType)) {
                foundFontTypeInCache = true;
                break;
            }
        }

        if (!foundFontTypeInCache) {
            mla_array_list_add(data_client->textSizeCache, textSize);
        }
    }

    return result;
}

mla_ui_web_remote_surface_t mla_ui_web_remote_surface_create(const mla_http_server_websocket_connection_t& connection) {

    mla_pointer_t surfaceData_ptr = mla_malloc_struct(mla_ui_web_remote_surface_data_t);

    mla_ui_web_remote_surface_data_t* surfaceData = mla_pointer_get_data<mla_ui_web_remote_surface_data_t>(surfaceData_ptr);

    if (surfaceData == nullptr) {
        return mla_ui_web_remote_surface_invalid();
    }

    mla_memset(surfaceData, 0, sizeof(mla_ui_web_remote_surface_data_t));
    surfaceData->connectionId = connection.id;
    surfaceData->server = connection.server;
    surfaceData->lastSurfaceSize = {
        1920,
        1080
    };
    surfaceData->lastInputStates = mla_ui_surface_input_states_empty();
    surfaceData->textSizeCache = mla_array_list_empty<mla_init_struct(mla_ui_web_remote_surface_client_text_size_t)>();

    return {
        {
            surfaceData_ptr,
            mla_private_ui_web_remote_surface_get_size,
            mla_private_ui_web_remote_surface_set_size,
            mla_private_ui_web_remote_surface_render_draw_commands,
            mla_private_ui_web_remote_surface_render_calc_text_size,
            mla_private_ui_web_remote_surface_get_input_states
        },
        mla_private_ui_web_remote_surface_handle_client_text_message
    };

}