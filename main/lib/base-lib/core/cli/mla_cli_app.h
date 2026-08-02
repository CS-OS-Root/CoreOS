//
// Created by christian on 9/13/25.
//

#ifndef MLA_CLI_APP_H
#define MLA_CLI_APP_H

#include "mla_cli_command.h"
#include "../system/mla_stream.h"

struct mla_cli_module_t {
    mla_string_t moduleName;
    mla_string_t description;
    mla_array_list_t<mla_init_struct(mla_cli_command_t)> availableCommands;
    mla_array_list_t<mla_init_struct(mla_cli_module_t)> subModules;

    static mla_cli_module_t init() {
        return {
            mla_string_empty(),
            mla_string_empty(),
            mla_array_list_empty<mla_init_struct(mla_cli_command_t)>(),
            mla_array_list_empty<mla_init_struct(mla_cli_module_t)>()
        };
    }
};

mla_cli_module_t mla_cli_module(const mla_string_t& name);
mla_cli_module_t mla_cli_module(const mla_string_t& name, const mla_string_t& description);

void mla_cli_module_add_command(mla_cli_module_t& module, const mla_cli_command_t& command);
void mla_cli_module_add_sub_module(mla_cli_module_t& module, const mla_cli_module_t& subModule);
const mla_cli_command_t* mla_cli_module_find_command(const mla_cli_module_t& module, const mla_string_t& commandName);

struct mla_cli_app_t {
    mla_array_list_t<mla_init_struct(mla_cli_module_t)> activeModules;

    // Interactive line editor state (see mla_cli_app.cpp).
    // stdin is read raw and non-blocking, so a multi-byte key (arrow keys,
    // Home/End, ...) can arrive split across reads. The parse state therefore
    // lives here and survives between calls to mla_cli_app_update_and_process_input.
    mla_string_t currentLine;   // the line currently being edited
    mla_size_t   cursorPos;     // insertion index into currentLine
    mla_uint8_t  escState;      // escape-sequence parser state
    mla_uint8_t  escParam;      // numeric parameter of a CSI sequence (e.g. ESC[3~)

    // Command history for up/down navigation
    mla_array_list_t<mla_init_struct(mla_string_t)> history;
    mla_int32_t  historyIndex;  // -1 == editing the live line
    mla_string_t savedLiveLine; // live line stashed while browsing history
};

mla_cli_app_t mla_cli_app_empty();
mla_cli_app_t mla_cli_app_init(mla_cli_module_t& rootModule, mla_stream_output_t& outputStream);
mla_bool_t mla_cli_app_update_and_process_input(mla_cli_app_t& app, mla_stream_input_t& inputStream, mla_stream_output_t& outputStream);
mla_string_t mla_private_cli_longest_common_prefix(const mla_array_list_t<mla_init_struct(mla_string_t)> &candidates);



#endif
