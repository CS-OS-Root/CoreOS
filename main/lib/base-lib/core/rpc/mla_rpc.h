//
// Created by christian on 11/17/25.
//

#ifndef MLA_RPC_H
#define MLA_RPC_H

#include "../system/mla_string.h"
#include "../serializer/mla_serializer.h"
#include "../system/mla_hash_map.h"
#include "../lifecycle/mla_lifecycle_events.h"
#include "../reflection/mla_reflection.h"

// Define an RPC Parameter which is not used
#define mla_rpc_procedure_void_t mla_platform_pointer_t
#define mla_rpc_procedure_const_void_t mla_platform_const_pointer_t

typedef mla_bool_t (*mla_rpc_procedure_handler_unsafe_t)(mla_platform_const_pointer_t input_data, mla_platform_pointer_t output_data);

struct mla_rpc_procedure_unsafe_t {
    mla_string_t procedureName;
    mla_serialize_definition_t inputDefinition;
    mla_serialize_definition_t outputDefinition;
    mla_rpc_procedure_handler_unsafe_t execute;

    static mla_rpc_procedure_unsafe_t init() {
        return { mla_string_empty(), mla_serialize_definition_invalid(), mla_serialize_definition_invalid(), nullptr };
    }
};

mla_rpc_procedure_unsafe_t mla_rpc_procedure_unsafe_invalid();

typedef mla_bool_t (*mla_rpc_remote_endpoint_can_handle)(const mla_user_data_t &userdata, const mla_string_t &procedure_name);
typedef mla_bool_t (*mla_rpc_remote_endpoint_execute_procedure)(const mla_user_data_t &userdata, const mla_string_t &procedure_name, const mla_serialize_definition_t &input_definition, const mla_serialize_definition_t &output_definition,  const mla_pointer_t& input_data, mla_pointer_t& output_data);

struct mla_rpc_remote_endpoint_t {
    mla_user_data_t user_data;
    mla_string_t endpoint_id;
    mla_rpc_remote_endpoint_can_handle can_handle;
    mla_rpc_remote_endpoint_execute_procedure execute_procedure;

    static mla_rpc_remote_endpoint_t init() {
        return { mla_user_data_empty(), mla_string_empty(), nullptr, nullptr };
    }
};

mla_rpc_remote_endpoint_t mla_rpc_remote_endpoint_invalid();
mla_rpc_remote_endpoint_t mla_rpc_remote_endpoint_all(mla_rpc_remote_endpoint_execute_procedure execute_procedure_handler, mla_user_data_t& user_data);
mla_rpc_remote_endpoint_t mla_rpc_remote_endpoint_start_with(const mla_string_t& start_string, mla_rpc_remote_endpoint_execute_procedure execute_procedure_handler, mla_user_data_t& user_data);
mla_rpc_remote_endpoint_t mla_rpc_remote_endpoint(mla_rpc_remote_endpoint_can_handle can_handle_handler, mla_rpc_remote_endpoint_execute_procedure execute_procedure_handler, mla_user_data_t& user_data);
mla_bool_t mla_rpc_register_remote_endpoint(const mla_rpc_remote_endpoint_t &endpoint);
mla_bool_t mla_rpc_unregister_remote_endpoint(const mla_rpc_remote_endpoint_t &endpoint);
mla_bool_t mla_rpc_remote_endpoint_find_handler(const mla_string_t &procedure_name, mla_rpc_remote_endpoint_t& out_endpoint);
mla_bool_t mla_rpc_remote_endpoint_valid(const mla_rpc_remote_endpoint_t &endpoint);


mla_rpc_procedure_unsafe_t mla_rpc_procedure_unsafe(
        const mla_string_t &procedure_name,
        const mla_serialize_definition_t &input_definition,
        const mla_serialize_definition_t &output_definition,
        const mla_rpc_procedure_handler_unsafe_t &execute_handler
);
mla_bool_t mla_rpc_register_procedure(const mla_rpc_procedure_unsafe_t &procedure);
mla_bool_t mla_rpc_unregister_procedure(const mla_string_t &procedure_name);
mla_bool_t mla_rpc_find_procedure(const mla_string_t &procedure_name, mla_rpc_procedure_unsafe_t& out_procedure);
mla_bool_t mla_rpc_is_local_procedure(const mla_string_t &procedure_name);
mla_bool_t mla_rpc_execute_procedure(const mla_string_t &procedure_name, const mla_serialize_definition_t &input_definition, const mla_serialize_definition_t &output_definition, const mla_pointer_t& input_data, mla_pointer_t& output_data);
mla_bool_t mla_rpc_execute_procedure_remote(const mla_string_t &procedure_name, const mla_serialize_definition_t &input_definition, const mla_serialize_definition_t &output_definition, const mla_pointer_t& input_data, mla_pointer_t& output_data);
mla_array_list_t<mla_init_struct(mla_rpc_procedure_unsafe_t)> mla_rpc_list_procedures();


////////////////////////////////////////////////////////////
///// Safe Helpers
/////////////////////////////////////////////////////////////

#define mla_rpc_safe_template_parameters typename TInput, typename TOutput

template<mla_rpc_safe_template_parameters>
struct mla_rpc_procedure_safe_t {
    mla_string_t procedureName;
    mla_serialize_definition_t inputDefinition;
    mla_serialize_definition_t outputDefinition;
    mla_bool_t (*execute)(const TInput *input_data, TOutput *output_data);
};

template<mla_rpc_safe_template_parameters>
mla_rpc_procedure_safe_t<TInput, TOutput> mla_rpc_procedure_safe(const mla_string_t &procedure_name, const mla_serialize_definition_t &input_definition, const mla_serialize_definition_t &output_definition,
        mla_bool_t (*execute_handler)(const TInput *input_data, TOutput *output_data)
) {
    return {
        procedure_name,
        input_definition,
        output_definition,
        execute_handler
    };
}
template<mla_rpc_safe_template_parameters>
mla_rpc_procedure_safe_t<TInput, TOutput> mla_rpc_procedure_safe_invalid() {
    return {
        mla_string_empty(),
        mla_serialize_definition_invalid(),
        mla_serialize_definition_invalid(),
        nullptr
    };
}

template<mla_rpc_safe_template_parameters>
mla_rpc_procedure_unsafe_t mla_rpc_procedure_safe_to_unsafe(const mla_rpc_procedure_safe_t<TInput, TOutput> &safe_procedure) {
    return {
        safe_procedure.procedureName,
        safe_procedure.inputDefinition,
        safe_procedure.outputDefinition,
        mla_r_cast<mla_rpc_procedure_handler_unsafe_t>(safe_procedure.execute)
    };
}

template<mla_rpc_safe_template_parameters>
mla_bool_t mla_rpc_register_procedure(const mla_rpc_procedure_safe_t<TInput, TOutput> &procedure) {
    mla_rpc_procedure_unsafe_t unsafe_procedure = mla_rpc_procedure_safe_to_unsafe(procedure);
    return mla_rpc_register_procedure(unsafe_procedure);
}

template<mla_rpc_safe_template_parameters>
mla_bool_t mla_rpc_execute_procedure(const mla_string_t &procedure_name, const TInput *input_data, TOutput *output_data) {
    auto input_definition = mla_serialize_definition<TInput>();
    auto output_definition = mla_serialize_definition<TOutput>();
    const mla_pointer_t input_ptr = mla_platform_pointer_to_managed_pointer(input_data);
    mla_pointer_t output_ptr = mla_platform_pointer_to_managed_pointer(output_data);
    return mla_rpc_execute_procedure(procedure_name, input_definition, output_definition, input_ptr, output_ptr);
}

template<typename TInput>
mla_bool_t mla_rpc_execute_procedure_void_output(const mla_string_t &procedure_name, const TInput *input_data) {
    auto input_definition = mla_serialize_definition<TInput>();
    auto output_definition = mla_serialize_definition_invalid();
    const mla_pointer_t input_ptr =  mla_platform_pointer_to_managed_pointer(input_data);
    mla_pointer_t output_ptr = mla_pointer_null();
    return mla_rpc_execute_procedure(procedure_name, input_definition, output_definition, input_ptr, output_ptr);
}

template<typename TOutput>
mla_bool_t mla_rpc_execute_procedure_void_input(const mla_string_t &procedure_name, TOutput *output_data) {
    auto input_definition = mla_serialize_definition_invalid();
    auto output_definition = mla_serialize_definition<TOutput>();
    const mla_pointer_t input_ptr = mla_pointer_null();
    mla_pointer_t output_ptr = mla_platform_pointer_to_managed_pointer(output_data);
    return mla_rpc_execute_procedure(procedure_name, input_definition, output_definition, input_ptr, output_ptr);
}

inline mla_bool_t mla_rpc_execute_procedure(const mla_string_t &procedure_name) {
    auto input_definition = mla_serialize_definition_invalid();
    auto output_definition = mla_serialize_definition_invalid();
    mla_pointer_t out = mla_pointer_null();
    const mla_pointer_t input_ptr = mla_pointer_null();
    return mla_rpc_execute_procedure(procedure_name, input_definition, output_definition, input_ptr, out);
}

template<mla_rpc_safe_template_parameters>
mla_bool_t mla_rpc_execute_procedure_remote(const mla_string_t &procedure_name, const TInput *input_data, TOutput *output_data) {
    auto input_definition = mla_serialize_definition<TInput>();
    auto output_definition = mla_serialize_definition<TOutput>();
    const mla_pointer_t input_ptr = mla_platform_pointer_to_managed_pointer(input_data);
    mla_pointer_t output_ptr = mla_platform_pointer_to_managed_pointer(output_data);
    return mla_rpc_execute_procedure_remote(procedure_name, input_definition, output_definition, input_ptr, output_ptr);
}

template<typename TInput>
mla_bool_t mla_rpc_execute_procedure_remote_void_output(const mla_string_t &procedure_name, const TInput *input_data) {
    auto input_definition = mla_serialize_definition<TInput>();
    auto output_definition = mla_serialize_definition_invalid();
    const mla_pointer_t input_ptr = mla_platform_pointer_to_managed_pointer(input_data);
    mla_pointer_t output_ptr = mla_pointer_null();
    return mla_rpc_execute_procedure_remote(procedure_name, input_definition, output_definition, input_ptr, output_ptr);
}

template<typename TOutput>
mla_bool_t mla_rpc_execute_procedure_remote_void_input(const mla_string_t &procedure_name, TOutput *output_data) {
    auto input_definition = mla_serialize_definition_invalid();
    auto output_definition = mla_serialize_definition<TOutput>();
    const mla_pointer_t input_ptr = mla_pointer_null();
    mla_pointer_t output_ptr = mla_platform_pointer_to_managed_pointer(output_data);
    return mla_rpc_execute_procedure_remote(procedure_name, input_definition, output_definition, input_ptr, output_ptr);
}

inline mla_bool_t mla_rpc_execute_procedure_remote(const mla_string_t &procedure_name) {
    auto input_definition = mla_serialize_definition_invalid();
    auto output_definition = mla_serialize_definition_invalid();
    const mla_pointer_t input_ptr = mla_pointer_null();
    mla_pointer_t output_ptr = mla_pointer_null();
    return mla_rpc_execute_procedure_remote(procedure_name, input_definition, output_definition, input_ptr, output_ptr);
}

template<mla_rpc_safe_template_parameters>
mla_bool_t mla_rpc_find_procedure(const mla_string_t &procedure_name, mla_rpc_procedure_safe_t<TInput, TOutput>& out_procedure) {

    mla_rpc_procedure_unsafe_t unsafe_procedure = mla_rpc_procedure_unsafe_invalid();

    if (mla_rpc_find_procedure(procedure_name, unsafe_procedure)) {
        out_procedure.procedureName = unsafe_procedure.procedureName;
        out_procedure.execute = mla_r_cast<mla_bool_t (*)(const TInput *, TOutput *)>(unsafe_procedure.execute);
        return true;
    } else {
        return false;
    }
}

#define mla_rpc_auto_register_procedure(procedure_name, input, output, handler) \
void mla_rpc_auto_register_procedure_##handler() { \
    mla_reflection_register_struct<input>(); \
    mla_reflection_register_struct<output>(); \
    auto input_definition = mla_serialize_definition<input>(); \
    auto output_defintion = mla_serialize_definition<output>(); \
    auto procedure = mla_rpc_procedure_safe<input, output>(mla_string_const(procedure_name), input_definition, output_defintion, handler); \
    mla_rpc_register_procedure<input, output>(procedure); \
} \
mla_lifecycle_boot_event_static_register(mla_lifecycle_boot_event_priority_rpc_preSetup, mla_rpc_auto_register_procedure_##handler) \

#define mla_rpc_auto_register_procedure_void_output(procedure_name, input, handler) \
void mla_rpc_auto_register_procedure_##handler() { \
    auto input_definition = mla_serialize_definition<input>(); \
    mla_reflection_register_struct<input>(); \
    auto output_defintion = mla_serialize_definition_void(); \
    auto procedure = mla_rpc_procedure_safe<input, void>(mla_string_const(procedure_name), input_definition, output_defintion, handler); \
    mla_rpc_register_procedure<input, void>(procedure); \
} \
mla_lifecycle_boot_event_static_register(mla_lifecycle_boot_event_priority_rpc_preSetup, mla_rpc_auto_register_procedure_##handler) \

#define mla_rpc_auto_register_procedure_void_input(procedure_name, output, handler) \
void mla_rpc_auto_register_procedure_##handler() { \
    mla_reflection_register_struct<output>(); \
    auto input_definition = mla_serialize_definition_void(); \
    auto output_defintion = mla_serialize_definition<output>(); \
    auto procedure = mla_rpc_procedure_safe<void, output>(mla_string_const(procedure_name), input_definition, output_defintion, handler); \
    mla_rpc_register_procedure<void, output>(procedure); \
} \
mla_lifecycle_boot_event_static_register(mla_lifecycle_boot_event_priority_rpc_preSetup, mla_rpc_auto_register_procedure_##handler) \

#define mla_rpc_auto_register_procedure_void(procedure_name, handler) \
void mla_rpc_auto_register_procedure_##handler() { \
    auto input_definition = mla_serialize_definition_void(); \
    auto output_defintion = mla_serialize_definition_void(); \
    auto procedure = mla_rpc_procedure_safe<void, void>(mla_string_const(procedure_name), input_definition, output_defintion, handler); \
    mla_rpc_register_procedure<void, void>(procedure); \
} \
mla_lifecycle_boot_event_static_register(mla_lifecycle_boot_event_priority_rpc_preSetup, mla_rpc_auto_register_procedure_##handler) \



#endif
