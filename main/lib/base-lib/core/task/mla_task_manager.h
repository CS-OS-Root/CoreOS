//
// Created by christian on 8/9/25.
//

#ifndef MLA_TASK_MANAGER_H
#define MLA_TASK_MANAGER_H

#include "mla_mutx.h"
#include "mla_task.h"
#include "mla_rw_lock.h"
#include "../system/mla_array_list.h"
#include "../log/mla_logging.h"
#include "../system/mla_string_concat.h"

// Forward declaration of the mutex type

struct mla_task_info_t {
    mla_string_t name; // Name of the task
    mla_task_priority priority; // Priority of the task
    mla_task_stack_size stack_size; // Stack size for the task
    mla_task_state state; // Current state of the task

    static mla_task_info_t init() {
        return {
            mla_string_empty(),
            TASK_PRIO_LOW,
            TASK_STACK_SIZE_DEFAULT,
            TASK_STATE_UNKNOWN
        };
    }
};

struct mla_task_manager_t {
    mla_array_list_t<mla_init_struct(mla_task_t)> tasks;
    mla_rw_lock_t taskLock;
};

enum mla_task_manager_state: mla_uint8_t {
    TASK_MANAGER_STATE_UNKNOWN,
    TASK_MANAGER_STATE_PROCESSING,
    TASK_MANAGER_STATE_WAITING_FOR_WORK
};

void mla_task_manager_cleanup();
mla_bool_t mla_task_manager_register_task(mla_task_t task);
mla_bool_t mla_task_manager_abort_task(const mla_string_t& name);
mla_array_list_t<mla_init_struct(mla_task_info_t)> mla_task_manager_get_task_infos();
mla_task_info_t mla_task_manager_get_task_info(const mla_string_t& name);
mla_bool_t mla_task_manager_task_exists(const mla_string_t& name);
mla_task_manager_state mla_task_manager_get_state();
void mla_task_manager_process_all_tasks();





#endif
