//
// Created by Copilot on 5/15/2026.
//

#include "mla_external_task.h"

mla_global mla_external_task_management_t g_external_task_management;

mla_user_data_id_init(mla_external_task_native_resource_user_data_name)

mla_size_t mla_private_external_task_read_stdout(mla_stream_input_t& p_Input, mla_size_t p_Offset, mla_size_t p_Length, mla_byte_t* p_Buffer) {

    if (g_external_task_management.read_stdout == nullptr || p_Buffer == nullptr) {
        return 0;
    }

    mla_pointer_t taskResource = mla_user_data_get_pointer(p_Input.userdata, mla_external_task_native_resource_user_data_name);
    if (mla_pointer_is_null(taskResource)) {
        return 0;
    }

    return g_external_task_management.read_stdout(taskResource, p_Offset, p_Length, p_Buffer);
}

mla_size_t mla_private_external_task_write_stdin(mla_stream_output_t& p_Output, mla_size_t p_Offset, mla_size_t p_Length, const mla_byte_t* p_Buffer) {

    if (g_external_task_management.write_stdin == nullptr || p_Buffer == nullptr) {
        return 0;
    }

    mla_pointer_t taskResource = mla_user_data_get_pointer(p_Output.userdata, mla_external_task_native_resource_user_data_name);
    if (mla_pointer_is_null(taskResource)) {
        return 0;
    }

    return g_external_task_management.write_stdin(taskResource, p_Offset, p_Length, p_Buffer);
}

mla_external_task_t mla_external_task_invalid() {
    return {
        mla_pointer_null(),
        mla_string_empty(),
        mla_stream_noop_input(),
        mla_stream_noop_output()
    };
}

mla_external_task_t mla_private_external_task_create_internal(const mla_string_t& p_CmdLine, const mla_string_t &p_WorkingDirectory) {
    if (mla_string_is_empty(p_CmdLine)) {
        return mla_external_task_invalid();
    }

    if (g_external_task_management.create_process == nullptr) {
        return mla_external_task_invalid();
    }

    mla_external_task_t result = mla_external_task_invalid();
    if (!g_external_task_management.create_process(result.native_resource, p_CmdLine, p_WorkingDirectory)) {
        return mla_external_task_invalid();
    }

    result.cmdline = mla_string_copy(p_CmdLine);

    mla_user_data_t userData = mla_user_data_empty();
    mla_user_data_set_pointer(userData, mla_external_task_native_resource_user_data_name, result.native_resource);

    result.std_out = {
        userData,
        mla_private_external_task_read_stdout,
        nullptr
    };

    result.std_in = {
        userData,
        mla_private_external_task_write_stdin,
        nullptr
    };

    return result;
}

mla_external_task_t mla_external_task_create(const mla_string_t& p_CmdLine) {
    return mla_private_external_task_create_internal(p_CmdLine, mla_string_empty());
}

mla_external_task_t mla_external_task_create(const mla_string_t& p_CmdLine, const mla_string_t &p_WorkingDirectory) {
    if (mla_string_is_empty(p_WorkingDirectory)) {
        return mla_external_task_invalid();
    }
    return mla_private_external_task_create_internal(p_CmdLine, p_WorkingDirectory);
}


mla_external_task_state mla_external_task_get_state(const mla_external_task_t& p_Task) {

    if (mla_pointer_is_null(p_Task.native_resource)) {
        return MLA_EXTERNAL_TASK_STATE_STOPPED;
    }

    if (g_external_task_management.get_state == nullptr) {
        return MLA_EXTERNAL_TASK_STATE_STOPPED;
    }

    return g_external_task_management.get_state(p_Task.native_resource);
}

void mla_external_task_stop(mla_external_task_t& p_Task) {

    p_Task.std_out = mla_stream_noop_input();
    p_Task.std_in = mla_stream_noop_output();
    p_Task.native_resource = mla_pointer_null();
}

mla_int32_t mla_external_task_read_result_code(const mla_external_task_t& p_Task) {

    if (mla_pointer_is_null(p_Task.native_resource)) {
        return -1;
    }

    mla_external_task_state state = mla_external_task_get_state(p_Task);
    if (state != MLA_EXTERNAL_TASK_STATE_STOPPED) {
        return -1;
    }

    if (g_external_task_management.read_result_code != nullptr) {
        return g_external_task_management.read_result_code(p_Task.native_resource);
    }

    return 0;
}



void mla_external_task_close_stdin(mla_external_task_t& p_Task) {

    if (mla_pointer_is_null(p_Task.native_resource)) {
        return;
    }

    if (g_external_task_management.close_stdin != nullptr) {
        g_external_task_management.close_stdin(p_Task.native_resource);
    }

    // Mark stdin stream as closed so callers don't try to write again
    p_Task.std_in = mla_stream_noop_output();
}

