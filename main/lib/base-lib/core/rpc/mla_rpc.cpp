//
// Created by christian on 11/17/25.
//

#include "mla_rpc.h"

#include "../system/mla_id.h"
#include "../task/mla_rw_lock.h"
#include "../log/mla_logging.h"
#include "../system/mla_string_concat.h"
#include "../serializer/mla_serializer.h"
#include "../serializer/mla_binary_serializer.h"

struct  mla_rpc_container_t {
    mla_array_list_t<mla_init_struct(mla_rpc_remote_endpoint_t)> remote_endpoints;
    mla_rw_lock_t remote_endpoints_lock;
    mla_hash_map_t<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_rpc_procedure_unsafe_t)> procedures;
    mla_bool_t isLocked;
};

mla_rpc_container_t g_rpc_container = {
    mla_array_list<mla_init_struct(mla_rpc_remote_endpoint_t)>(),
    mla_rw_lock_create("rpc_endpoints"),
    mla_hash_map<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_rpc_procedure_unsafe_t)>(),
    false
};

mla_rpc_procedure_unsafe_t mla_rpc_procedure_unsafe_invalid() {
    return {
        mla_string_empty(),
        mla_serialize_definition_invalid(),
        mla_serialize_definition_invalid(),
        nullptr
    };
}

mla_rpc_procedure_unsafe_t mla_rpc_procedure_unsafe(
        const mla_string_t &procedure_name,
        const mla_serialize_definition_t &input_definition,
        const mla_serialize_definition_t &output_definition,
        const mla_rpc_procedure_handler_unsafe_t &execute_handler) {
    return {
        procedure_name,
        input_definition,
        output_definition,
        execute_handler
    };
}

mla_bool_t mla_rpc_register_procedure(const mla_rpc_procedure_unsafe_t &procedure) {

    if (mla_string_starts_with(procedure.procedureName, mla_string_const("/"))) {
        mla_warning(mla_string_concat("Attempted to register RPC procedure ", procedure.procedureName, " with reserved '/' prefix."));
        return false; // Reserved prefix
    }

    if (g_rpc_container.isLocked) {
        mla_warning(mla_string_concat("Attempted to register RPC procedure ", procedure.procedureName, " while RPC container is locked."));
        return false; // RPC Container is locked
    }

    return mla_hash_map_push(g_rpc_container.procedures, procedure.procedureName, procedure) == MLA_HASH_MAP_PUSH_ADDED;
}

mla_bool_t mla_rpc_unregister_procedure(const mla_string_t &procedure_name) {

    if (g_rpc_container.isLocked) {
        mla_warning(mla_string_concat("Attempted to unregister RPC procedure ", procedure_name, " while RPC container is locked."));
        return false; // RPC Container is locked
    }

    return mla_hash_map_remove(g_rpc_container.procedures, procedure_name);
}

mla_bool_t mla_rpc_find_procedure(const mla_string_t &procedure_name, mla_rpc_procedure_unsafe_t& out_procedure) {

    return mla_hash_map_get(g_rpc_container.procedures, procedure_name, out_procedure);
}

mla_bool_t mla_rpc_is_local_procedure(const mla_string_t &procedure_name) {

    return mla_hash_map_contains(g_rpc_container.procedures, procedure_name);
}

mla_bool_t mla_rpc_execute_procedure(const mla_string_t &procedure_name, const mla_serialize_definition_t &input_definition, const mla_serialize_definition_t &output_definition, const mla_pointer_t& input_data, mla_pointer_t& output_data) {

    mla_rpc_procedure_unsafe_t procedure = mla_rpc_procedure_unsafe_invalid();

    if (!mla_rpc_find_procedure(procedure_name, procedure)) {
        // Try to execute the procedure remotely
        return mla_rpc_execute_procedure_remote(procedure_name, input_definition, output_definition, input_data, output_data);
    }

    if (procedure.execute == nullptr) {
        return false; // Procedure has no handler
    }

    // Serialize also local calls to have the same behavior as remote calls
    mla_pointer_t serialized_input = mla_pointer_null();

    mla_memory_stream_t memory_stream = mla_memory_stream_empty();

    // Serialize input data
    if (!mla_pointer_is_null(input_data) && input_definition.data_size > 0) {

        serialized_input = mla_malloc_buffer(input_definition.data_size);

        if (mla_pointer_is_null(serialized_input)) {
            return false; // Memory allocation failed
        }

        mla_serializer_t binarySerializer = mla_binary_serializer(memory_stream.output);
        if (!mla_serializer_write_data_struct(binarySerializer, input_data, input_definition.write_function)) {
            return false; // Serialization failed
        }

        mla_memory_stream_set_position(memory_stream, 0);

        mla_deserializer_t binaryDeserializer = mla_binary_deserializer(memory_stream.input);
        // Start reading
        binaryDeserializer.read_next(binaryDeserializer);

        if (!mla_deserializer_read_struct_read_function(binaryDeserializer, serialized_input, input_definition.read_function)) {
            return false; // Serialization failed
        }

    }

    // Create Output Buffer
    mla_pointer_t serialized_output = mla_pointer_null();

    if (!mla_pointer_is_null(output_data) && output_definition.data_size > 0) {
        serialized_output = mla_malloc_buffer(output_definition.data_size);

        if (mla_pointer_is_null(serialized_output)) {
            return false; // Memory allocation failed
        }
    }

    mla_bool_t result = procedure.execute(mla_pointer_get_platform_pointer(serialized_input), mla_pointer_get_platform_pointer(serialized_output));



    if (result) {

        if (!mla_pointer_is_null(serialized_output)) {
            mla_memory_stream_reset(memory_stream);

            // Deserialize output data
            mla_serializer_t binarySerializer = mla_binary_serializer(memory_stream.output);
            if (!mla_serializer_write_data_struct(binarySerializer, serialized_output, output_definition.write_function)) {
                return false; // Serialization failed
            }

            mla_memory_stream_set_position(memory_stream, 0);

            mla_deserializer_t binaryDeserializer = mla_binary_deserializer(memory_stream.input);
            // Start reading
            binaryDeserializer.read_next(binaryDeserializer);
            if (!mla_deserializer_read_struct_read_function(binaryDeserializer, output_data, output_definition.read_function)) {
                return false; // Serialization failed
            }
        }

        return true;

    } else {

        return false; // Procedure execution failed
    }
}

mla_bool_t mla_rpc_execute_procedure_remote(const mla_string_t &procedure_name, const mla_serialize_definition_t &input_definition, const mla_serialize_definition_t &output_definition, const mla_pointer_t& input_data, mla_pointer_t& output_data) {


    mla_rpc_remote_endpoint_t endpoint = mla_rpc_remote_endpoint_invalid();

    if (!mla_rpc_remote_endpoint_find_handler(procedure_name, endpoint)) {
        return false; // No remote endpoint can handle this procedure
    }

    return endpoint.execute_procedure(endpoint.user_data, procedure_name, input_definition, output_definition, input_data, output_data);
}

mla_array_list_t<mla_init_struct(mla_rpc_procedure_unsafe_t)> mla_rpc_list_procedures() {
    return mla_hash_map_values(g_rpc_container.procedures);
}

mla_rpc_remote_endpoint_t mla_rpc_remote_endpoint_invalid() {
    return {
        mla_user_data_empty(),
        mla_string_empty(),
        nullptr,
        nullptr
    };
}

mla_bool_t mla_private_rpc_remote_endpoint_all_checker(const mla_user_data_t &userdata, const mla_string_t &procedure_name) {
    (void)userdata;
    (void)procedure_name;
    return true;
}

mla_rpc_remote_endpoint_t mla_rpc_remote_endpoint_all(mla_rpc_remote_endpoint_execute_procedure execute_procedure_handler, mla_user_data_t& user_data) {

    return {
        user_data,
        mla_generate_runtime_id(),
        mla_private_rpc_remote_endpoint_all_checker,
        execute_procedure_handler
    };
}

struct mla_rpc_remote_endpoint_start_with_user_data_t {
    mla_string_t prefix;

    static mla_rpc_remote_endpoint_start_with_user_data_t init() {
        return { mla_string_empty() };
    }
};

mla_user_data_id_init(mla_rpc_endpoint_start_with_user_data_name)

mla_bool_t mla_private_rpc_remote_endpoint_start_with_checker(const mla_user_data_t& user_data,
                                                    const mla_string_t &procedure_name) {

    mla_rpc_remote_endpoint_start_with_user_data_t *pathPrefix = mla_user_data_get_pointer_data<mla_rpc_remote_endpoint_start_with_user_data_t>(user_data, mla_rpc_endpoint_start_with_user_data_name);

    if (pathPrefix == nullptr) {
        return false;
    }

    return mla_string_starts_with(procedure_name, pathPrefix->prefix);
}


mla_rpc_remote_endpoint_t mla_rpc_remote_endpoint_start_with(const mla_string_t& start_string, mla_rpc_remote_endpoint_execute_procedure execute_procedure_handler, mla_user_data_t& user_data) {

    mla_pointer_t user_data_payload_ptr = mla_malloc_struct(mla_rpc_remote_endpoint_start_with_user_data_t);

    mla_rpc_remote_endpoint_start_with_user_data_t* user_data_payload = mla_pointer_get_data<mla_rpc_remote_endpoint_start_with_user_data_t>(user_data_payload_ptr);

    if (user_data_payload == nullptr) {
        return mla_rpc_remote_endpoint_invalid();
    }

    user_data_payload->prefix = start_string;


    mla_user_data_set_pointer(user_data, mla_rpc_endpoint_start_with_user_data_name, user_data_payload_ptr);

    return {
        user_data,
        mla_generate_runtime_id(),
        mla_private_rpc_remote_endpoint_start_with_checker,
        execute_procedure_handler
    };
}

mla_rpc_remote_endpoint_t mla_rpc_remote_endpoint(mla_rpc_remote_endpoint_can_handle can_handle_handler, mla_rpc_remote_endpoint_execute_procedure execute_procedure_handler, mla_user_data_t& user_data) {

    return {
        user_data,
        mla_generate_runtime_id(),
        can_handle_handler,
        execute_procedure_handler
    };
}

mla_bool_t mla_rpc_register_remote_endpoint(const mla_rpc_remote_endpoint_t &endpoint) {

    if (!mla_rw_lock_write(g_rpc_container.remote_endpoints_lock)) {
        return false;
    }

    if (endpoint.can_handle == nullptr || endpoint.execute_procedure == nullptr) {
        mla_rw_unlock_write(g_rpc_container.remote_endpoints_lock);
        return false;
    }

    mla_bool_t result = mla_array_list_add(g_rpc_container.remote_endpoints, endpoint);
    mla_rw_unlock_write(g_rpc_container.remote_endpoints_lock);

    return result;
}

mla_bool_t mla_rpc_unregister_remote_endpoint(const mla_rpc_remote_endpoint_t &endpoint) {

    if (!mla_rw_lock_write(g_rpc_container.remote_endpoints_lock)) {
        return false;
    }

    mla_bool_t result = false;

    for (mla_size_t i = 0; i < mla_array_list_size(g_rpc_container.remote_endpoints); i++) {
        mla_rpc_remote_endpoint_t& current_endpoint = mla_array_list_get_unsafe(g_rpc_container.remote_endpoints, i);
        if (mla_string_equals(endpoint.endpoint_id, current_endpoint.endpoint_id)) {
            mla_array_list_remove(g_rpc_container.remote_endpoints, i);
            result = true;
            break;
        }
    }

    mla_rw_unlock_write(g_rpc_container.remote_endpoints_lock);
    return result;
}

mla_bool_t mla_rpc_remote_endpoint_find_handler(const mla_string_t &procedure_name, mla_rpc_remote_endpoint_t& out_endpoint) {

    if (!mla_rw_lock_read(g_rpc_container.remote_endpoints_lock)) {
        return false;
    }

    for (mla_size_t i = 0; i < mla_array_list_size(g_rpc_container.remote_endpoints); i++) {
        mla_rpc_remote_endpoint_t& endpoint = mla_array_list_get_unsafe(g_rpc_container.remote_endpoints, i);

        if (endpoint.can_handle(endpoint.user_data, procedure_name)) {
            out_endpoint = endpoint;
            mla_rw_unlock_read(g_rpc_container.remote_endpoints_lock);
            return true;
        }
    }

    mla_rw_unlock_read(g_rpc_container.remote_endpoints_lock);
    return false;

}

mla_bool_t mla_rpc_remote_endpoint_valid(const mla_rpc_remote_endpoint_t &endpoint) {
    return endpoint.execute_procedure != nullptr;
}

void mla_private_rpc_container_lock() {
    g_rpc_container.isLocked = true;
}

mla_lifecycle_boot_event_static_register(mla_lifecycle_boot_event_priority_rpc_setup, mla_private_rpc_container_lock);