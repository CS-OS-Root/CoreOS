//
// Created by christian on 9/13/25.
//

#ifndef MAIN_APP_CLI_H
#define MAIN_APP_CLI_H

#include "../../lib/base-lib/core/task/mla_task_manager.h"
#include "../../lib/base-lib/core/task/mla_task.h"
#include "../../lib/base-lib/core/cli/mla_cli_app.h"
#include "../../lib/base-lib/core/cli/mla_cli_history.h"
#include "../../lib/base-lib/core/task/mla_task_cli_module.h"
#include "../../lib/base-lib/core/system/mla_stream.h"


mla_cli_app_t g_main_app_cli = mla_cli_app_empty();
mla_cli_history_store_t g_main_app_cli_history = mla_cli_history_store_t::init(10);

inline mla_task_process_result_state mla_private_cli_task(mla_user_data_t& userdata) {

    (void)userdata;

    mla_stream_input_t input = mla_stream_input_stdin();
    mla_stream_output_t output = mla_stream_output_stdout();

    mla_cli_app_update_and_process_input(g_main_app_cli, input, output);

    return TASK_PROCESS_RESULT_CONTINUE;
}

inline mla_cli_module_t mla_private_cli_build_root_module() {

    mla_cli_module_t rootModule = mla_cli_module(mla_string_const("Root"));

    // Add sub-modules

    // Task Manager
    mla_cli_module_add_sub_module(rootModule, mla_task_cli_module_create());

    return rootModule;
}

inline void main_app_cli_init() {

    // Initialize CLI related components here
    mla_info("Welcome to the CommandLine Interface (CLI)");
    mla_info("Please enter a command:");

    mla_cli_history_load_from_file(g_main_app_cli_history, mla_string_const("/.cli_history.json"), 10);

    mla_cli_module_t root = mla_private_cli_build_root_module();

    mla_stream_output_t output = mla_stream_output_stdout();
    g_main_app_cli = mla_cli_app_init(root, output);

    mla_user_data_t app_user_data = mla_user_data_empty();
    mla_task_t task = mla_task_repeating(mla_string("cli"), mla_private_cli_task, app_user_data);
    mla_task_manager_register_task(task);

}

#endif //MAIN_APP_CLI_H
