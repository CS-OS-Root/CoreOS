//
// Created by chris on 9/14/2025.
//

#include "mla_task_manager.h"
#include "mla_task_cli_module.h"
#include "../cli/mla_cli_command.h"

mla_bool_t mla_private_task_cli_list_all_tasks(const mla_cli_command_t &command,
                                          const mla_hash_map_t<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)> &parameters,
                                          const mla_cli_command_execute_outstream_t &out) {

    (void)command;
    (void)parameters;

    out.writeCString(out.userdata, "Listing all tasks:\n");

    auto tasks = mla_task_manager_get_task_infos();

    for (mla_size_t i = 0; i < mla_array_list_size(tasks); ++i) {

        mla_task_info_t *task = mla_array_list_get_ref(tasks, i);
        // Format: - Name: 'TaskName', State: TaskState, Priority: TaskPriority, Stack Size: StackSize
        mla_string_t taskInfo = mla_string_concat(
            " - Name: '", task->name,
            "', State: ", mla_task_state_to_string(task->state),
            ", Priority: ", mla_task_priority_to_string(task->priority),
            ", Stack Size: ", mla_task_stack_size_to_string(task->stack_size), "\n"
        );

        out.write(out.userdata, taskInfo);
    }

    return true;

}

mla_bool_t mla_private_task_cli_kill_task(const mla_cli_command_t &command,
                                          const mla_hash_map_t<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)> &parameters,
                                          const mla_cli_command_execute_outstream_t &out) {

    (void)command;
    mla_string_t name_param = mla_string_const("name");
    mla_string_t name_value = mla_string_empty();

    // Get the task name parameter
    if (!mla_hash_map_get(parameters, name_param, name_value)) {
        out.writeCString(out.userdata, "Error: 'name' parameter is required.\n");
        return false;
    }

    if (!mla_task_manager_task_exists(name_value)) {
        // Task does not exist
        out.writeCString(out.userdata, "Error: Task with name '");
        out.write(out.userdata, name_value);
        out.writeCString(out.userdata, "' does not exist.\n");
        return false;
    }

    // Kill the task
    out.writeCString(out.userdata, "Killing task '");
    out.write(out.userdata, name_value);
    out.writeCString(out.userdata, "'... ");

    if (mla_task_manager_abort_task(name_value)) {
        out.writeCString(out.userdata, " [OK]\n");
        return true;
    } else {
        out.writeCString(out.userdata, " [ERROR]\n");
        return false;
    }

}


mla_cli_module_t mla_task_cli_module_create() {

    mla_cli_module_t module = mla_cli_module(mla_string_const("task"), mla_string_const("Task management commands"));

    // Add 'list' command
    mla_cli_command_t cmdList = mla_cli_command(mla_string_const("ls"), mla_string_const("List all tasks"), mla_private_task_cli_list_all_tasks);
    mla_cli_module_add_command(module, cmdList);

    // Add 'kill' command
    mla_cli_command_t cmdKill = mla_cli_command(mla_string_const("kill"), mla_string_const("Kill a task by Name"), mla_private_task_cli_kill_task);
    mla_cli_command_add_parameter(cmdKill, mla_string_const("name"), mla_string_const("Name of the task to kill"), true);
    mla_cli_module_add_command(module, cmdKill);

    return module;
}