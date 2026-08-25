//
// Created by christian on 9/10/25.
//

#include "../lib/base-lib/platform/wasm/mla_global_platform_wasm.h"
#include "../lib/base-lib/platform/generic/mla_global_config_inmemory.h"
#include "../lib/base-lib/platform/wasm/mla_global_platform_task_manager_wasm.h"
#include "../lib/base-lib/platform/generic/mla_global_network_disabled.h"
#include "../lib/base-lib/platform/generic/mla_global_service_disabled.h"
#include "../lib/base-lib/platform/generic/mla_global_ui_surface_disabled.h"

#include "main_app.h"

int main() {

    mla_boot_os_application();
    return run();
}
