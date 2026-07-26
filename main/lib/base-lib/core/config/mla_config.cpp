#include "mla_config.h"

#include "../log/mla_logging.h"
#include "../system/mla_string_concat.h"

// Choose serializer based on configuration

#if defined(mla_config_binary_mode) && mla_config_binary_mode==1

#include "../serializer/mla_binary_serializer.h"
#define mla_config_create_serializer mla_binary_serializer
#define mla_config_create_deserializer mla_binary_deserializer

#else

#include "../serializer/mla_json_serializer.h"
#define mla_config_create_serializer(input) mla_json_serializer(input)
#define mla_config_create_deserializer(output) mla_json_deserializer(output)

#endif

// If configuration is disabled, provide dummy implementations
#if defined(mla_config_enabled) && mla_config_enabled==0

mla_bool_t mla_private_config_disabled_commit_output(mla_bytes_t output, mla_size_t unused_bytes) {
    (void)unused_bytes;
    output = mla_bytes_empty();
    return false;
}

mla_bool_t mla_private_config_disabled_reset() {
    return false;
}

static mla_config_low_level_operations_t g_low_level_operations = {
    mla_bytes_empty,
    mla_bytes_empty,
    mla_private_config_disabled_commit_output,
    mla_private_config_disabled_reset
};

#else

mla_global mla_config_low_level_operations_t g_config_low_level_operations;

#endif



mla_bool_t mla_config_manager_read(const mla_config_definition_t &definition, mla_pointer_t& config) {

    if (definition.definition.read_function == nullptr) {
        return false;
    }

    mla_bytes_t input = g_config_low_level_operations.read_config_input();

    if (input.size == 0) {
        return false;
    }

    mla_deserializer_t deserializer = mla_config_create_deserializer(
        mla_stream_input_from_buffer(mla_bytes_get_data_for_writing(input), input.size));
    // Start reading
    deserializer.read_next(deserializer);

    // First token should be start of struct
    if (deserializer.current_token.type != MLA_DESERIALIZER_STRUCT_START) {
        return false;
    }

    // Read the struct and find the matching config name from the definition

    mla_uint8_t nested_level = 0;

    while (deserializer.read_next(deserializer)) {
        if (deserializer.current_token.type == MLA_DESERIALIZER_STRUCT_START) {
            nested_level++;
        } else if (deserializer.current_token.type == MLA_DESERIALIZER_STRUCT_END) {
            nested_level--;
        } else if (deserializer.current_token.type == MLA_DESERIALIZER_PROPERTY_NAME && nested_level == 0) {
            // Check if this is the config we are looking for
            if (mla_string_equals(deserializer.current_token.complex.property_name, definition.config_name)) {
                // Next token should be struct start
                if (!deserializer.read_next(deserializer) || deserializer.current_token.type !=
                    MLA_DESERIALIZER_STRUCT_START) {
                    return false;
                }

                // If we are not at the start of a struct, then we did not find the config
                return mla_deserializer_read_struct_read_function(deserializer, config, definition.definition.read_function);
            }
        }
    }

    return false;
}

mla_bool_t mla_config_manager_write(const mla_config_definition_t &definition, const mla_pointer_t& config) {

    if (definition.definition.write_function == nullptr) {
        return false;
    }


    mla_bytes_t input = g_config_low_level_operations.read_config_input();
    mla_bytes_t output = g_config_low_level_operations.create_config_output_buffer();
    mla_size_t unused_bytes = 0;

    // Create scope for memory cleanup
    {
        mla_stream_output_t output_stream = mla_stream_output_to_buffer(mla_bytes_get_data_for_writing(output), output.size);

        mla_deserializer_t deserializer = mla_config_create_deserializer(
            mla_stream_input_from_buffer(mla_bytes_get_data_for_writing(input), input.size));
        mla_serializer_t serializer = mla_config_create_serializer(output_stream);

        // Save is working like this we are copy the content from the deserializer to the serializer
        // until we find the config we want to update, then we write the new config and continue copying the rest
        // from the deserializer to the serializer

        if (input.size == 0) {
            // If input is empty, just write the new config as the only content
            serializer.write_start_struct(serializer);
        }


        mla_bool_t found = false;
        mla_uint8_t nested_level = 0;

        // Stream from deserializer to serializer until we find the config we want to update
        while (deserializer.read_next(deserializer)) {
            switch (deserializer.current_token.type) {
                case MLA_DESERIALIZER_NULL:
                    //QQQ
                    break;
                case MLA_DESERIALIZER_PROPERTY_NAME:
                    // Check if this is the config we are looking for
                    if (nested_level == 1 && mla_string_equals(deserializer.current_token.complex.property_name,
                                                               definition.config_name)) {
                        // We found the config we want to update, break out of the loop
                        found = true;
                        mla_deserializer_skip_property_value(deserializer);
                        break;
                    } else {
                        // Write the property name to the serializer
                        serializer.write_property_name(serializer, deserializer.current_token.complex.property_name);
                    }
                    break;
                case MLA_DESERIALIZER_STRUCT_START:
                    nested_level++;
                    serializer.write_start_struct(serializer);
                    break;
                case MLA_DESERIALIZER_STRUCT_END:
                    nested_level--;

                    // Only write the end struct if we are not at the top level
                    if (nested_level != 0) {
                        serializer.write_end_struct(serializer);
                    }

                    break;
                case MLA_DESERIALIZER_LIST_START:
                    serializer.write_start_list(serializer);
                    break;
                case MLA_DESERIALIZER_LIST_END:
                    serializer.write_end_list(serializer);
                    break;
                case MLA_DESERIALIZER_VALUE_BOOL:
                    serializer.write_bool(serializer, deserializer.current_token.simple.bool_value);
                    break;
                case MLA_DESERIALIZER_VALUE_INT8:
                    serializer.write_int8(serializer, deserializer.current_token.simple.int8_value);
                    break;
                case MLA_DESERIALIZER_VALUE_INT16:
                    serializer.write_int16(serializer, deserializer.current_token.simple.int16_value);
                    break;
                case MLA_DESERIALIZER_VALUE_INT32:
                    serializer.write_int32(serializer, deserializer.current_token.simple.int32_value);
                    break;
                case MLA_DESERIALIZER_VALUE_INT64:
                    serializer.write_int64(serializer, deserializer.current_token.simple.int64_value);
                    break;
                case MLA_DESERIALIZER_VALUE_UINT8:
                    serializer.write_uint8(serializer, deserializer.current_token.simple.uint8_value);
                    break;
                case MLA_DESERIALIZER_VALUE_UINT16:
                    serializer.write_uint16(serializer, deserializer.current_token.simple.uint16_value);
                    break;
                case MLA_DESERIALIZER_VALUE_UINT32:
                    serializer.write_uint32(serializer, deserializer.current_token.simple.uint32_value);
                    break;
                case MLA_DESERIALIZER_VALUE_UINT64:
                    serializer.write_uint64(serializer, deserializer.current_token.simple.uint64_value);
                    break;
                case MLA_DESERIALIZER_VALUE_FLOAT:
                    serializer.write_float(serializer, deserializer.current_token.simple.float_value);
                    break;
                case MLA_DESERIALIZER_VALUE_DOUBLE:
                    serializer.write_double(serializer, deserializer.current_token.simple.double_value);
                    break;
                case MLA_DESERIALIZER_VALUE_STRING:
                    serializer.write_string(serializer, deserializer.current_token.complex.string_value);
                    break;
                case MLA_DESERIALIZER_VALUE_BYTES:
                    serializer.write_bytes(serializer, deserializer.current_token.complex.bytes_value);
                    break;
                default:
                    mla_error(mla_string_concat("Invalid token type in deserializer: ", mla_string_from_int32(
                        (mla_int32_t) deserializer.current_token.type)));
                    return false;
            }
        }

        mla_serializer_write_struct_write_function(serializer, definition.config_name, config, definition.definition.write_function);

        if (found) {
            // Stream the rest of the deserializer to the serializer
            while (deserializer.read_next(deserializer)) {
                switch (deserializer.current_token.type) {
                    case MLA_DESERIALIZER_NULL:
                        //QQQ
                        break;
                    case MLA_DESERIALIZER_PROPERTY_NAME:
                        serializer.write_property_name(serializer, deserializer.current_token.complex.property_name);
                        break;
                    case MLA_DESERIALIZER_STRUCT_START:
                        serializer.write_start_struct(serializer);
                        break;
                    case MLA_DESERIALIZER_STRUCT_END:
                        serializer.write_end_struct(serializer);
                        break;
                    case MLA_DESERIALIZER_LIST_START:
                        serializer.write_start_list(serializer);
                        break;
                    case MLA_DESERIALIZER_LIST_END:
                        serializer.write_end_list(serializer);
                        break;
                    case MLA_DESERIALIZER_VALUE_BOOL:
                        serializer.write_bool(serializer, deserializer.current_token.simple.bool_value);
                        break;
                    case MLA_DESERIALIZER_VALUE_INT8:
                        serializer.write_int8(serializer, deserializer.current_token.simple.int8_value);
                        break;
                    case MLA_DESERIALIZER_VALUE_INT16:
                        serializer.write_int16(serializer, deserializer.current_token.simple.int16_value);
                        break;
                    case MLA_DESERIALIZER_VALUE_INT32:
                        serializer.write_int32(serializer, deserializer.current_token.simple.int32_value);
                        break;
                    case MLA_DESERIALIZER_VALUE_INT64:
                        serializer.write_int64(serializer, deserializer.current_token.simple.int64_value);
                        break;
                    case MLA_DESERIALIZER_VALUE_UINT8:
                        serializer.write_uint8(serializer, deserializer.current_token.simple.uint8_value);
                        break;
                    case MLA_DESERIALIZER_VALUE_UINT16:
                        serializer.write_uint16(serializer, deserializer.current_token.simple.uint16_value);
                        break;
                    case MLA_DESERIALIZER_VALUE_UINT32:
                        serializer.write_uint32(serializer, deserializer.current_token.simple.uint32_value);
                        break;
                    case MLA_DESERIALIZER_VALUE_UINT64:
                        serializer.write_uint64(serializer, deserializer.current_token.simple.uint64_value);
                        break;
                    case MLA_DESERIALIZER_VALUE_FLOAT:
                        serializer.write_float(serializer, deserializer.current_token.simple.float_value);
                        break;
                    case MLA_DESERIALIZER_VALUE_DOUBLE:
                        serializer.write_double(serializer, deserializer.current_token.simple.double_value);
                        break;
                    case MLA_DESERIALIZER_VALUE_STRING:
                        serializer.write_string(serializer, deserializer.current_token.complex.string_value);
                        break;
                    case MLA_DESERIALIZER_VALUE_BYTES:
                        serializer.write_bytes(serializer, deserializer.current_token.complex.bytes_value);
                        break;
                    default:
                        mla_error(mla_string_concat("Invalid token type in deserializer: ",
                            mla_string_from_int32((mla_int32_t) deserializer.current_token.type)));
                        return false;
                }
            }
        }

        if (input.size == 0 || !found) {
            // If input is empty, just write the new config as the only content
            serializer.write_end_struct(serializer);
        }

        if (output_stream.available_bytes != nullptr) {
            unused_bytes = output.size - output_stream.available_bytes(output_stream);
        }
    }

    // Something is wrong if there is no content
    if (unused_bytes == 0) {
        mla_error("Unknown error: No content in output buffer after serialization");
        return false;
    }

    return g_config_low_level_operations.commit_config_output(output, unused_bytes);
}

mla_bool_t mla_config_manager_reset() {
    return g_config_low_level_operations.reset();
}
