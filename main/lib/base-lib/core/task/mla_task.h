//
// Created by christian on 8/9/25.
//

#ifndef MLA_TASK_H
#define MLA_TASK_H

#include "../system/mla_string.h"
#include "mla_task_manager_data_types.h"

typedef void (*mla_task_worker_one_time_t)(mla_user_data_t& userData);
typedef mla_task_process_result_state (*mla_task_worker_repeating_t)(mla_user_data_t& userData);


struct mla_task_t {
    mla_string_t name; // Name of the task
    mla_task_worker_t worker;
    mla_user_data_t userData; // User data for the task
    mla_task_priority priority; // Priority of the task
    mla_task_stack_size stack_size; // Stack size for the task

    // OS Resources
    mla_pointer_t taskResource; // Maybe move this to userdata

    // States
    mla_pointer_t sharedStates; // Shared states for the task (mla_task_shared_states)

    static mla_task_t init() {
        return {
            mla_string_empty(),
            nullptr,
            mla_user_data_empty(),
            TASK_PRIO_NORMAL,
            TASK_STACK_SIZE_DEFAULT,
            mla_pointer_null(),
            mla_pointer_null()
        };
    }
};

mla_task_t mla_task_repeating(const mla_string_t& name, mla_task_worker_repeating_t worker, mla_user_data_t& userData);
mla_task_process_result_state mla_private_task_worker_one_time(mla_user_data_t& userData);
mla_task_t mla_task_one_time(const mla_string_t& name, mla_task_worker_one_time_t worker, mla_user_data_t& userData);
mla_task_t mla_task_native(const mla_string_t& name, mla_task_worker_t worker, mla_user_data_t& userData);

void mla_task_update_stack_size(mla_task_t& task, mla_task_stack_size new_stack_size);

#endif
