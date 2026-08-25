//
// Created by christian on 6/7/26.
//

#include "../lib/base-lib/platform/linux/mla_global_platform_linux.h"
#include "../lib/base-lib/platform/linux/mla_global_config_linux.h"
#include "../lib/base-lib/platform/linux/mla_global_network_linux.h"
#include "../lib/base-lib/platform/linux/mla_global_external_task_linux.h"
#include "../lib/base-lib/platform/linux/mla_global_service_linux.h"
#include "../lib/base-lib/platform/linux/mla_global_platform_task_manager_linux.h"
#include "../lib/base-lib/platform/generic/mla_global_ui_surface_disabled.h"

#include "main_app.h"

int main(int argc, char** argv)
{

    mla_boot_os_application();
    return run(argc, argv);
}