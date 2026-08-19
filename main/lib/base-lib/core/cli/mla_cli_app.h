//
// Created by christian on 9/13/25.
//

#ifndef MLA_CLI_APP_H
#define MLA_CLI_APP_H

#include "mla_cli_command.h"
#include "../system/mla_stream.h"

/**
 * @brief Represents a CLI module containing available commands and child sub-modules.
 */
struct mla_cli_module_t {
    mla_string_t moduleName;
    mla_string_t description;
    mla_array_list_t<mla_init_struct(mla_cli_command_t)> availableCommands;
    mla_array_list_t<mla_init_struct(mla_cli_module_t)> subModules;

    /**
     * @brief Creates an empty module instance.
     * @return Initialized empty mla_cli_module_t.
     */
    static mla_cli_module_t init() {
        return {
            mla_string_empty(),
            mla_string_empty(),
            mla_array_list_empty<mla_init_struct(mla_cli_command_t)>(),
            mla_array_list_empty<mla_init_struct(mla_cli_module_t)>()
        };
    }
};

/**
 * @brief Creates a CLI module with the given name.
 */
mla_cli_module_t mla_cli_module(const mla_string_t& name);

/**
 * @brief Creates a CLI module with name and descriptive summary.
 */
mla_cli_module_t mla_cli_module(const mla_string_t& name, const mla_string_t& description);

/**
 * @brief Adds a command to the specified CLI module.
 */
void mla_cli_module_add_command(mla_cli_module_t& module, const mla_cli_command_t& command);

/**
 * @brief Adds a sub-module to the specified CLI module.
 */
void mla_cli_module_add_sub_module(mla_cli_module_t& module, const mla_cli_module_t& subModule);

/**
 * @brief Finds a command in a module by name.
 */
const mla_cli_command_t* mla_cli_module_find_command(const mla_cli_module_t& module, const mla_string_t& commandName);

/**
 * @brief Core application state for the interactive CLI shell and line editor.
 */
struct mla_cli_app_t {
    mla_array_list_t<mla_init_struct(mla_cli_module_t)> activeModules; /**< Stack of active nested navigation modules */

    // Interactive line editor state (see mla_cli_app.cpp).
    // stdin is read raw and non-blocking, so a multi-byte key (arrow keys,
    // Home/End, ...) can arrive split across reads. The parse state therefore
    // lives here and survives between calls to mla_cli_app_update_and_process_input.
    mla_string_t currentLine;     /**< The line currently being edited by the user */
    mla_size_t   cursorPos;       /**< 0-indexed insertion point within currentLine */
    mla_uint8_t  escState;        /**< ANSI/Windows escape-sequence parser state */
    mla_uint8_t  escParam;        /**< Numeric parameter of a CSI sequence (e.g. 3 for Delete) */
    mla_size_t   lastDrawnLength; /**< Length of currentLine when last drawn to handle character erasure */

    // Command history for up/down navigation
    mla_array_list_t<mla_init_struct(mla_string_t)> history;
    mla_int32_t  historyIndex;  /**< Current index in command history (-1 indicates live line) */
    mla_string_t savedLiveLine; /**< Temporary copy of live input line saved while browsing history */

    // Interactive command execution state
    mla_bool_t   is_interactive;          /**< True if interactive prompting is enabled on this CLI app instance */
    mla_bool_t   in_interactive_mode;     /**< True while waiting for step-by-step parameter inputs */
    mla_cli_command_t interactive_command; /**< The command whose parameters are being collected */
    mla_hash_map_t<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)> interactive_parameters; /**< Parameters collected so far */
    mla_size_t   interactive_param_index; /**< Index in interactive_command.parameters currently being prompted */
};

/**
 * @brief Creates an empty CLI application instance.
 */
mla_cli_app_t mla_cli_app_empty();

/**
 * @brief Initializes a CLI application with a root module and initial prompt display.
 */
mla_cli_app_t mla_cli_app_init(mla_cli_module_t& rootModule, mla_stream_output_t& outputStream);

/**
 * @brief Sets whether the CLI application operates in interactive mode.
 */
void mla_cli_app_set_interactive(mla_cli_app_t& app, mla_bool_t isInteractive);

/**
 * @brief Returns true if the CLI application has interactive mode enabled.
 */
mla_bool_t mla_cli_app_is_interactive(const mla_cli_app_t& app);

/**
 * @brief Reads available bytes from input stream, updates editor state, handles control keys, and repaints line.
 * @return True if input processing succeeded, false if command execution failed.
 */
mla_bool_t mla_cli_app_update_and_process_input(mla_cli_app_t& app, mla_stream_input_t& inputStream, mla_stream_output_t& outputStream);

/**
 * @brief Computes the longest common prefix among a list of completion candidates.
 */
mla_string_t mla_private_cli_longest_common_prefix(const mla_array_list_t<mla_init_struct(mla_string_t)> &candidates);

#endif
