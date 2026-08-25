//
// Created by chris on 9/11/2025.
//

#include "../lib/base-lib/platform/windows/mla_global_network_windows.h"
#include "../lib/base-lib/platform/windows/mla_global_platform_windows.h"
#include "../lib/base-lib/platform/windows/mla_global_config_windows.h"
#include "../lib/base-lib/platform/windows/mla_global_platform_task_manager_windows.h"
#include "../lib/base-lib/platform/windows/mla_global_external_task_windows.h"
#include "../lib/base-lib/platform/windows/mla_global_service_windows.h"
#include "../lib/base-lib/platform/windows/mla_global_ui_surface_windows_direct2d.h"

#include "main_app.h"

int main(int argc, char** argv) {

    mla_boot_os_application();
    return run(argc, argv);
}