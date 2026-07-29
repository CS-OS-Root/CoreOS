//
// Created by chris on 2/5/2026.
//

#ifndef MLA_UI_WEB_REMOTE_SURFACE_H
#define MLA_UI_WEB_REMOTE_SURFACE_H

#include "../surfaces/mla_ui_surface.h"
#include "../../http/mla_http_server.h"


struct mla_ui_web_remote_surface_message_result_t {
    mla_bool_t handled;
    mla_array_list_t<mla_init_struct(mla_ui_surface_input_event_t)> inputEvents;
};

mla_ui_web_remote_surface_message_result_t mla_ui_web_remote_surface_message_result_no_successful();

struct mla_ui_web_remote_surface_t {
    mla_ui_surface_t surface;
    mla_ui_web_remote_surface_message_result_t (*handle_client_text_message)(mla_ui_web_remote_surface_t& surface, const mla_string_t& message);
};

mla_ui_web_remote_surface_t mla_ui_web_remote_surface_invalid();

mla_ui_web_remote_surface_t mla_ui_web_remote_surface_create(const mla_http_server_websocket_connection_t& connection);

#endif