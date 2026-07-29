//
// Created by christian on 8/9/25.
//

#ifndef MLA_TASK_MANAGER_SINGLE_THREAD_H
#define MLA_TASK_MANAGER_SINGLE_THREAD_H

// This is a single-threaded task manager implementation.
// It is designed to run tasks in a single thread without any concurrency.
// It supported by all platforms and is the default implementation.

#include "../../core/task/mla_task_manager.h"
#include "../../core/mla_native_resource.h"

// Provide by the task manager implementation file.
extern mla_task_manager_t g_TaskManager;

mla_bool_t mla_task_manager_single_thread_create_task(
    const mla_task_worker_t worker,
    const mla_string_t& taskName,
    mla_user_data_t& userData,
    const mla_task_stack_size stackSize,
    const mla_task_priority priority,
    mla_pointer_t& outTaskResourceOwner,
    const mla_pointer_t& shared_states
) {
    (void)worker; // Silences the unused parameter warning
    (void)taskName; // Silences the unused parameter warning
    (void)userData; // Silences the unused parameter warning
    (void)stackSize; // Silences the unused parameter warning
    (void)priority; // Silences the unused parameter warning
    (void)outTaskResourceOwner; // Silences the unused parameter warning
    (void)shared_states; // Silences the unused parameter warning


    // There is no need for a task resource owner in single-threaded mode.
    return true;
}

// This is a dummy mutex implementation for single-threaded mode.
// It does not actually lock anything, as there is no concurrency.
// it just handles the double lock that can happen in single-threaded mode.
// an resource should never be locked twice in single-threaded mode, but we handle it gracefully.
struct mla_task_manager_single_thread_mutex {
    mla_volatile mla_bool_t locked;

    static mla_task_manager_single_thread_mutex init() {
        return { false };
    }
};

mla_bool_t mla_task_manager_single_thread_create_mutex(mla_pointer_t& outMutex, mla_bool_t supports_recursive_locking) {

    if (supports_recursive_locking) {
        // If recursive locking is supported, we can just return a null pointer (always unlocked).
        return true;
    }

    mla_pointer_t mutex_ptr = mla_malloc_struct(mla_task_manager_single_thread_mutex);
    mla_task_manager_single_thread_mutex* mutex = mla_pointer_get_data<mla_task_manager_single_thread_mutex>(mutex_ptr);

    if (mutex == nullptr) {
        return false; // Failed to allocate memory for mutex
    }
    mla_memset(mutex, 0, sizeof(mla_task_manager_single_thread_mutex));

    mutex->locked = false;
    outMutex = mutex_ptr;

    return true;
}

mla_bool_t mla_task_manager_single_thread_lock_mutex(const mla_pointer_t& mutexResource, mla_int32_t timeout) {

    (void)timeout; // Silences the unused parameter warning

    if (mla_pointer_is_null(mutexResource)) {
        // If the mutex is null, we return true because we support recursive locking and this is a dummy mutex that is always unlocked.
        return true;
    }

    mla_task_manager_single_thread_mutex* mutex = mla_pointer_get_data<mla_task_manager_single_thread_mutex>(mutexResource);

    if (mutex == nullptr) {
        return true;
    }

    if (mutex->locked) {
        // If the mutex is already locked, we return false.
        // In a single-threaded environment, this should not happen.
        return false;
    }

    mutex->locked = true;
    return true;
}

mla_bool_t mla_task_manager_single_thread_unlock_mutex(const mla_pointer_t& mutexResource) {

    if (mla_pointer_is_null(mutexResource)) {
        return true;
    }

    mla_task_manager_single_thread_mutex* mutex = mla_pointer_get_data<mla_task_manager_single_thread_mutex>(mutexResource);

    if (mutex == nullptr) {
        return true;
    }

    if (!mutex->locked) {
        // If the mutex is already locked, we return false.
        // In a single-threaded environment, this should not happen.
        return false;
    }

    mutex->locked = false;
    return true;
}


mla_uint32_t mla_task_manager_single_thread_run_with_prio(mla_task_priority priority) {

    mla_size_t currentTaskIndex = 0;

    mla_uint32_t processedTaskCount = 0;

    while (true) {

        // We dont care about the mutex of the task manager in single-threaded mode.
        // There is no other thread that can modify the task list.
        mla_size_t taskCount = mla_array_list_size(g_TaskManager.tasks);

        if (taskCount == 0) {
            return processedTaskCount;
        }

        if (currentTaskIndex >= taskCount) {
            return processedTaskCount;
        }

        mla_task_t* task = mla_array_list_get_ref(g_TaskManager.tasks, currentTaskIndex);


        // Check if the task matches the priority and is not completed
        if (task->priority == priority) {

            mla_task_shared_states* shared_states = mla_pointer_get_data<mla_task_shared_states>(task->sharedStates);

            if (shared_states != nullptr) {
                if (shared_states->processingState.value == TASK_STATE_ABORTING) {
                    shared_states->processingState.value = TASK_STATE_ABORTED;
                } else if (!mla_task_is_done(shared_states->processingState.value)) {

                    shared_states->processingState.value = TASK_STATE_RUNNING;

                    if (task->worker != nullptr) {

                        mla_task_process_result_state result_state = task->worker(task->userData);

                        if (result_state == TASK_PROCESS_RESULT_DONE) {
                            shared_states->processingState.value = TASK_STATE_COMPLETED;
                        }

                    } else {
                        // If the worker is null, we just mark it as completed.
                        shared_states->processingState.value = TASK_STATE_COMPLETED;
                    }

                    processedTaskCount++;

                }
            }

        }

        currentTaskIndex++;

    }

}

void mla_task_manager_single_thread_run() {

    mla_uint32_t processedTaskCount = 0;

    do {
        mla_uint64_t start_time = mla_system_time_ms();

        processedTaskCount = 0;
        // High get 4 Times more CPU than Low
        processedTaskCount = processedTaskCount + mla_task_manager_single_thread_run_with_prio(TASK_PRIO_HIGH);
        processedTaskCount = processedTaskCount + mla_task_manager_single_thread_run_with_prio(TASK_PRIO_NORMAL);
        processedTaskCount = processedTaskCount + mla_task_manager_single_thread_run_with_prio(TASK_PRIO_HIGH);

        processedTaskCount = processedTaskCount + mla_task_manager_single_thread_run_with_prio(TASK_PRIO_HIGH);
        processedTaskCount = processedTaskCount + mla_task_manager_single_thread_run_with_prio(TASK_PRIO_NORMAL);
        processedTaskCount = processedTaskCount + mla_task_manager_single_thread_run_with_prio(TASK_PRIO_HIGH);

        processedTaskCount = processedTaskCount + mla_task_manager_single_thread_run_with_prio(TASK_PRIO_LOW);

        // if the loop is to fast we wait a little bit to dont burn the cpu the whole time
        if ((mla_system_time_ms() - start_time) < 25) {
            mla_sleep(10);
        }


    } while (processedTaskCount != 0);

}

mla_multi_task_mode mla_task_manager_single_thread_multi_task_mode() {
    return MULTI_TASK_MODE_SIMULATED;
}

mla_int32_t mla_task_manager_single_thread_atomic_int32_increment(mla_atomic_int32_t& value) {
    return ++value.value;
}

mla_int32_t mla_task_manager_single_thread_atomic_int32_decrement(mla_atomic_int32_t& value) {
    return --value.value;
}

mla_int32_t mla_task_manager_single_thread_atomic_int32_add(mla_atomic_int32_t& value, mla_int32_t addend) {
    value.value += addend;
    return value.value;
}

mla_int32_t mla_task_manager_single_thread_atomic_int32_subtract(mla_atomic_int32_t& value, mla_int32_t subtrahend) {
    value.value -= subtrahend;
    return value.value;
}

mla_int32_t mla_task_manager_single_thread_atomic_int32_exchange(mla_atomic_int32_t& value, mla_int32_t newValue) {
    value.value = newValue;
    return value.value;
}

mla_bool_t mla_task_manager_single_thread_atomic_int32_compare_exchange(mla_atomic_int32_t& value, mla_int32_t expectedValue, mla_int32_t newValue) {

    if (value.value == expectedValue) {
        value.value = newValue;
        return true;
    }
    return false;

}

struct mla_task_manager_single_thread_task_local {
    mla_platform_pointer_t value;

    static mla_task_manager_single_thread_task_local init() {
        return { nullptr };
    }
};

mla_bool_t mla_task_manager_single_thread_create_task_local(mla_pointer_t& outTaskLocal) {

    mla_pointer_t local_ptr = mla_malloc_struct(mla_task_manager_single_thread_task_local);
    mla_task_manager_single_thread_task_local* local = mla_pointer_get_data<mla_task_manager_single_thread_task_local>(local_ptr);

    if (local == nullptr) {
        return false;
    }
    mla_memset(local, 0, sizeof(mla_task_manager_single_thread_task_local));

    local->value = nullptr;
    outTaskLocal = local_ptr;

    return true;
}

mla_bool_t mla_task_manager_single_thread_set_task_local(const mla_pointer_t& taskLocal, mla_platform_pointer_t value) {

    mla_task_manager_single_thread_task_local* local = mla_pointer_get_data<mla_task_manager_single_thread_task_local>(taskLocal);

    if (local == nullptr) {
        return false;
    }

    local->value = value;
    return true;
}

mla_platform_pointer_t mla_task_manager_single_thread_get_task_local(const mla_pointer_t& taskLocal) {

    mla_task_manager_single_thread_task_local* local = mla_pointer_get_data<mla_task_manager_single_thread_task_local>(taskLocal);

    if (local == nullptr) {
        return nullptr;
    }

    return local->value;
}

mla_task_manager_low_level_access g_task_low_level_access = {
    mla_task_manager_single_thread_create_task,
    mla_task_manager_single_thread_run,
    mla_task_manager_single_thread_create_mutex,
    mla_task_manager_single_thread_lock_mutex,
    mla_task_manager_single_thread_unlock_mutex,
    mla_task_manager_single_thread_multi_task_mode,
    mla_task_manager_single_thread_create_task_local,
    mla_task_manager_single_thread_set_task_local,
    mla_task_manager_single_thread_get_task_local,
    mla_task_manager_single_thread_atomic_int32_increment,
    mla_task_manager_single_thread_atomic_int32_decrement,
    mla_task_manager_single_thread_atomic_int32_add,
    mla_task_manager_single_thread_atomic_int32_subtract,
    mla_task_manager_single_thread_atomic_int32_exchange,
    mla_task_manager_single_thread_atomic_int32_compare_exchange
};


#endif
