//
// Created by chris on 12/19/2025.
//

#ifndef MAIN_APP_WEB_UI_H
#define MAIN_APP_WEB_UI_H

#include "../../lib/base-lib/core/http/mla_http_server.h"
#include "../../lib/base-lib/core/rpc/mla_rpc_http_server.h"
#include "../../lib/base-lib/core/ui/web/mla_ui_http_server.h"
#include "../../lib/base-lib/core/log/mla_logger_rpc.h"

#ifndef mla_app_web_ui_http_task_count
#define mla_app_web_ui_http_task_count 2
#endif

#ifndef mla_app_web_ui_http_task_stack_size
#define mla_app_web_ui_http_task_stack_size TASK_STACK_SIZE_DEFAULT
#endif

static mla_http_server_t g_main_app_web_ui_http_server = mla_http_server_invalid();

inline void main_app_web_ui_init(const mla_ui_control_surface_process_task_t &processTask) {

    mla_info("Initializing Web UI...");
    g_main_app_web_ui_http_server = mla_http_server(mla_network_host_ip4(mla_string_const("0.0.0.0"), 8081));
    mla_rpc_http_server_initialize(g_main_app_web_ui_http_server);
    mla_ui_http_server_initialize(g_main_app_web_ui_http_server);
    mla_ui_http_server_add_web_surface(g_main_app_web_ui_http_server, mla_string_const("Test UI"), mla_string_const("test"), processTask);
    mla_http_server_start(g_main_app_web_ui_http_server, mla_app_web_ui_http_task_count, mla_app_web_ui_http_task_stack_size);

    mla_array_list_t<mla_init_struct(mla_network_ip_address_t)> local_ips = mla_network_get_local_ip_addresses();

    if (mla_array_list_size(local_ips) == 0) {
        mla_warning("No local IP addresses found. Web UI may not be accessible.");
    } else {
        for (mla_size_t i = 0; i < mla_array_list_size(local_ips); i++) {

            mla_network_ip_address_t& address = mla_array_list_get_unsafe(local_ips, i);

            mla_info(mla_string_concat("Web UI accessible at http://", address.address, ":8081/ui/"));

        }
    }
}

#endif