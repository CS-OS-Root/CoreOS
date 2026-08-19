//
// Created by christian on 9/10/25.
//

#ifndef MAIN_APP_H
#define MAIN_APP_H

// Include Global Storage
#include "../lib/base-lib/core/log/mla_logger_console.h"
#include "../lib/base-lib/core/log/mla_logging.h"
#include "modules/main_app_cli.h"
#include "modules/main_app_web_ui.h"
#include "modules/main_app_window_ui.h"
#include "modules/main_app_main_window.h"
#include "modules/main_app_background_task.h"

void mla_private_main_app_make_crash() {
    // NOLINTBEGIN(clang-analyzer-core.NullDereference, clang-diagnostic-null-dereference)
    volatile int* null_ptr = nullptr;
    *null_ptr = 42;
    // NOLINTEND(clang-analyzer-core.NullDereference, clang-diagnostic-null-dereference)
}

void mla_private_main_app_make_malloc_fail() {
    g_low_level_access.on_malloc_failure(1048576, "main_app.h", "mla_private_main_app_make_malloc_fail");
}

int run(int argc = 0, char** argv = nullptr) {

    // Low Level Setup
    mla_log_to_console_activate();

    if (argv != nullptr) {
        for (int i = 1; i < argc; ++i) {
            if (argv[i] != nullptr && mla_strstr(argv[i], "crash") != nullptr) {
                mla_info("Crash argument detected! Triggering segmentation fault...");
                mla_private_main_app_make_crash();
            } else if (argv[i] != nullptr && mla_strstr(argv[i], "malloc_fail") != nullptr) {
                mla_info("Malloc_fail argument detected! Triggering malloc failure handler...");
                mla_private_main_app_make_malloc_fail();
            }
        }
    }

    mla_info("Starting Test Application...");
    mla_info("Test Application started successfully!");
    main_app_cli_init(argc, argv);

    //mla_ui_control_surface_process_task_t task = main_app_main_window_performance_t::renderer;
    mla_ui_control_surface_process_task_t task = main_app_main_window_t::renderer;

    main_app_web_ui_init(task);
    main_app_window_ui_init(task);

    // Setup Background Tasks
    main_app_background_tasks_init();

    mla_task_manager_process_all_tasks();

    mla_info("Exiting Test Application...");

    return 0;
}

#endif //MAIN_APP_H