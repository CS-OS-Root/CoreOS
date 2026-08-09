# Core-OS library source files

# Main source files
set(SOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/mla_data_types.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/mla_pointer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/mla_default_pointer_memory_manager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/mla_noop_pointer_memory_manager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/mla_native_resource_pointer_memory_manager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/mla_area_pointer_memory_manager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/system/mla_string.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/system/mla_string_concat.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/system/mla_string_builder.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/system/mla_string_utf.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/system/mla_string_convert.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/system/mla_bytes.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/system/mla_number.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/system/mla_id.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/system/mla_user_data.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/log/mla_logging.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/log/mla_logger_console.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/log/mla_logger.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/memory/mla_memory_hook.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/task/mla_mutx.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/task/mla_rw_lock.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/task/mla_atomic.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/task/mla_task.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/task/mla_task_local.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/task/mla_task_manager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/task/mla_task_cli_module.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/task/mla_task_manager_data_types.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/cli/mla_cli_parser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/cli/mla_cli_app.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/cli/mla_cli_command.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/cli/mla_cli_history.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/system/mla_stream.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/system/mla_stream_helper.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/system/mla_stream_wrapper.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/system/mla_stream_deflate.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/config/mla_config.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/serializer/mla_serializer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/serializer/mla_binary_serializer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/serializer/mla_json_serializer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/serializer/mla_xml_serializer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/utils/mla_endian_utils.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/utils/mla_math_utils.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/hash/mla_sha1.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/rpc/mla_rpc.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/lifecycle/mla_lifecycle_events.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/log/mla_logger_rpc.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/reflection/mla_reflection.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/reflection/mla_reflection_rpc.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/ui/controls/mla_ui_control.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/ui/controls/mla_ui_window.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/ui/controls/mla_ui_label.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/ui/controls/mla_ui_button.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/ui/controls/mla_ui_text_edit.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/ui/controls/mla_ui_loading_indicator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/ui/surfaces/mla_ui_surface_draw.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/ui/controls/mla_ui_control_surface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/ui/surfaces/mla_ui_surface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/update/mla_update.cpp
)

# Network source files
set(SOURCE_NETWORK_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/http/mla_http_header.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/http/mla_http_client.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/http/mla_http_server.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/http/mla_http_utils.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/http/mla_http_chunked_stream.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/http/mla_http_multipart.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/http/mla_websocket_utils.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/http/mla_websocket_client.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/url/mla_url.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/network/mla_network.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/rpc/mla_rpc_http_server.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/rpc/mla_rpc_http_client.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/ui/web/mla_ui_html_template_compile.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/ui/web/mla_ui_http_server.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/ui/web/mla_ui_web_remote_surface.cpp
)

# External task source files
set(SOURCE_EXTERNAL_TASK_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/external_task/mla_external_task.cpp
)

# File system source files
set(SOURCE_FILE_SYSTEM_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/filesystem/mla_file_system.cpp
)

# UI source files
set(SOURCE_UI_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/base-lib/core/ui/display/mla_ui_display_surface.cpp
)
