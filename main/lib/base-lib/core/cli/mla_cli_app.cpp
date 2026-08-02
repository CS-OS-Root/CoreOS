//
// Created by christian on 9/13/25.
//

#include "mla_cli_app.h"
#include "mla_cli_parser.h"
#include "../log/mla_logging.h"
#include "../system/mla_string_concat.h"

mla_user_data_id_init(mla_stream_output_user_data_name)
mla_user_data_id_init(mla_cli_app_user_data_name)
mla_user_data_id_init(mla_cli_submodule_user_data_name)

void mla_private_cli_write_string(mla_stream_output_t &outputStream, const mla_string_t& str) {
    outputStream.write(outputStream, 0, mla_string_length(str), mla_r_cast<const mla_byte_t*>(mla_string_data(str)));
}

void mla_private_cli_command_execute_outstream_to_stream_bridge(const mla_user_data_t& userdata, const mla_string_t &data) {
    mla_stream_output_t *output = mla_user_data_get_pointer_data<mla_stream_output_t>(userdata, mla_stream_output_user_data_name);

    if (output == nullptr) {
        return;
    }

    mla_private_cli_write_string(*output, data);
}

void mla_private_cli_command_execute_outstream_verbose_to_stream_bridge(const mla_user_data_t& userdata, const mla_string_t &data) {
    mla_stream_output_t *output = mla_user_data_get_pointer_data<mla_stream_output_t>(userdata, mla_stream_output_user_data_name);

    if (output == nullptr) {
        return;
    }

    mla_private_cli_write_string(*output, data);
}

void mla_private_cli_command_execute_outstream_verbose_to_stream_disabled(const mla_user_data_t& userdata, const mla_string_t &data) {
    (void)userdata;
    (void)data;
}

void mla_private_cli_command_execute_outstream_c_string_to_stream_bridge(const mla_user_data_t& userdata, const mla_char_t* data) {
    mla_stream_output_t *output = mla_user_data_get_pointer_data<mla_stream_output_t>(userdata, mla_stream_output_user_data_name);

    if (output == nullptr) {
        return;
    }

    output->write(*output, 0, mla_strlen(data), mla_r_cast<const mla_byte_t*>(data));
}

void mla_private_cli_command_execute_outstream_verbose_c_string_to_stream_bridge(const mla_user_data_t& userdata, const mla_char_t* data) {
    mla_stream_output_t *output = mla_user_data_get_pointer_data<mla_stream_output_t>(userdata, mla_stream_output_user_data_name);

    if (output == nullptr) {
        return;
    }

    output->write(*output, 0, mla_strlen(data), mla_r_cast<const mla_byte_t*>(data));
}

void mla_private_cli_command_execute_outstream_verbose_c_string_to_stream_disabled(const mla_user_data_t& userdata, const mla_char_t* data) {
    (void)userdata;
    (void)data;
}

void mla_private_cli_command_execute_outstream_buffer_to_stream_bridge(const mla_user_data_t& userdata, const mla_char_t* data, mla_size_t length) {

    mla_stream_output_t *output = mla_user_data_get_pointer_data<mla_stream_output_t>(userdata, mla_stream_output_user_data_name);

    if (output == nullptr) {
        return;
    }

    output->write(*output, 0, length, mla_r_cast<const mla_byte_t*>(data));
}

void mla_private_cli_command_execute_outstream_verbose_buffer_to_stream_bridge(const mla_user_data_t& userdata, const mla_char_t* data, mla_size_t length) {

    mla_stream_output_t *output = mla_user_data_get_pointer_data<mla_stream_output_t>(userdata, mla_stream_output_user_data_name);

    if (output == nullptr) {
        return;
    }

    output->write(*output, 0, length, mla_r_cast<const mla_byte_t*>(data));
}

void mla_private_cli_command_execute_outstream_verbose_buffer_to_stream_disabled(const mla_user_data_t& userdata, const mla_char_t* data, mla_size_t length) {
    (void)userdata;
    (void)data;
    (void)length;
}

mla_string_t mla_private_cli_build_module_prompt(mla_cli_app_t &app) {
    mla_string_t result = mla_string_empty();
    mla_size_t size = mla_array_list_size(app.activeModules);

    if (size > 1) {
        // Dont print the root module
        for (mla_size_t i = 1; i < size; ++i) {
            if (i != 1) {
                result = mla_string_concat(result, ">");
            }

            mla_string_t currentModuleName = mla_array_list_get_ref(app.activeModules, i)->moduleName;
            result = mla_string_concat(result, currentModuleName);
        }
    }

    return mla_string_concat(result, ">");
}

void mla_private_cli_write_module_prompt(mla_cli_app_t &app, mla_stream_output_t &outputStream) {
    mla_private_cli_write_string(outputStream, mla_private_cli_build_module_prompt(app));
}

void mla_private_cli_activate_module(mla_cli_app_t &app, mla_cli_module_t &module) {
    mla_array_list_add(app.activeModules, module);
}

mla_bool_t mla_private_cli_cmd_exit_execute(const mla_cli_command_t &command,
                                const mla_hash_map_t<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)> &parameters,
                                const mla_cli_command_execute_outstream_t &out) {
    (void) parameters;
    mla_cli_app_t *app = mla_user_data_get_pointer_data<mla_cli_app_t>(command.user_data, mla_cli_app_user_data_name);

    if (app == nullptr) {
        return false;
    }

    mla_size_t size = mla_array_list_size(app->activeModules);

    if (size > 1) {
        out.write(out.userdata, mla_string_concat("Close module '",  mla_array_list_get_ref(app->activeModules, size -1)->moduleName, "'\n"));
        mla_array_list_remove(app->activeModules, size - 1);

    }

    return true;
}

mla_string_t mla_private_cli_format_command(const mla_cli_command_t &command) {

    // Start with command usage
    mla_string_t result = mla_string_concat("  ", command.name);

    // Add parameters to usage line
    for (mla_size_t i = 0; i < mla_array_list_size(command.parameters); ++i) {
        mla_cli_command_parameter_t *param = mla_array_list_get_ref(command.parameters, i);
        if (param->mandatory && param->is_flag) {
            result = mla_string_concat(result, " --", param->parameterName);
        } else if (param->mandatory) {
            result = mla_string_concat(result, " --", param->parameterName, " <value>");
        } else if (param->is_flag) {
            result = mla_string_concat(result, " [--", param->parameterName, "]");
        } else {
            result = mla_string_concat(result, " [--", param->parameterName, " <value>]");
        }
    }

    // Add command description if available
    if (mla_string_length(command.description) > 0) {
        result = mla_string_concat(result, "\n    ", command.description);
    }

    // Add parameter descriptions
    for (mla_size_t i = 0; i < mla_array_list_size(command.parameters); ++i) {
        mla_cli_command_parameter_t *param = mla_array_list_get_ref(command.parameters, i);

        if (mla_string_length(param->description) > 0) {
            mla_string_t paramDesc = mla_string_concat("\n      --", param->parameterName, ": ",
                                                      param->description);
            if (param->mandatory) {
                paramDesc = mla_string_concat(paramDesc, " (required)");
            } else {
                paramDesc = mla_string_concat(paramDesc, " (optional)");
            }

            result = mla_string_concat(result, paramDesc);
        }
    }

    return result;
}

mla_bool_t mla_private_cli_cmd_help_execute(const mla_cli_command_t &command,
                                const mla_hash_map_t<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)> &parameters,
                                const mla_cli_command_execute_outstream_t &out) {
    (void) parameters;
    mla_cli_app_t *app = mla_user_data_get_pointer_data<mla_cli_app_t>(command.user_data, mla_cli_app_user_data_name);

    if (app == nullptr) {
        return false;
    }

    mla_size_t size = mla_array_list_size(app->activeModules);

    if (size == 0) {
        return true;
    }

    const mla_cli_module_t *activeModule = mla_array_list_get_ref(app->activeModules, size - 1);

    if (activeModule == nullptr) {
        return true;
    }

    // List all commands
    out.write(out.userdata, mla_string_concat("Help for module '", activeModule->moduleName, "'\n\n"));

    mla_size_t commandCount = mla_array_list_size(activeModule->availableCommands);

    if (commandCount > 0) {
        out.write(out.userdata, mla_string_const("Available commands:\n"));

        for (mla_size_t i = 0; i < commandCount; ++i) {
            mla_cli_command_t *commandOfModule = mla_array_list_get_ref(activeModule->availableCommands, i);

            // Remove the help command from the list
            if (mla_string_equals(command.name, commandOfModule->name)) {
                continue;
            }

            mla_string_t commandFormated = mla_private_cli_format_command(*commandOfModule);
            out.write(out.userdata, mla_string_concat(commandFormated, "\n"));
        }
    } else {
        out.write(out.userdata, mla_string_const("No commands available in this module.\n"));
    }

    mla_size_t subModuleCount = mla_array_list_size(activeModule->subModules);

    if (subModuleCount > 0) {
        out.write(out.userdata, mla_string_const("\nAvailable Modules:\n"));

        for (mla_size_t i = 0; i < subModuleCount; ++i) {
            mla_cli_module_t *subModule = mla_array_list_get_ref(activeModule->subModules, i);
            out.write(out.userdata, mla_string_concat("  ", subModule->moduleName, "\n"));

            if (mla_string_length(subModule->description) > 0) {
                out.write(out.userdata, mla_string_concat("    ", subModule->description, "\n"));
            }
        }
    }

    return true;
}

mla_bool_t mla_private_cli_cmd_open_sub_module_execute(const mla_cli_command_t &command,
                                           const mla_hash_map_t<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)> &parameters,
                                           const mla_cli_command_execute_outstream_t &out) {
    (void) parameters;

    mla_cli_app_t *app = mla_user_data_get_pointer_data<mla_cli_app_t>(command.user_data, mla_cli_app_user_data_name);

    if (app == nullptr) {
        return false;
    }

    mla_cli_module_t *subModule = mla_user_data_get_pointer_data<mla_cli_module_t>(command.user_data, mla_cli_submodule_user_data_name);

    if (subModule == nullptr) {
        return false;
    }

    out.write(out.userdata, mla_string_concat("Open module '", subModule->moduleName, "'\n"));
    mla_private_cli_activate_module(*app, *subModule);
    return true;
}

mla_cli_parser_t mla_private_cli_setup_parser(mla_cli_app_t &app) {
    mla_cli_parser_t parser = mla_cli_parser();

    mla_size_t moduleCount = mla_array_list_size(app.activeModules);

    mla_pointer_t app_ptr = mla_platform_pointer_to_managed_pointer(&app);

    // Add all commands from the last active module
    if (moduleCount > 0) {
        mla_cli_module_t *currentModule = mla_array_list_get_ref(app.activeModules, moduleCount - 1);

        for (mla_size_t i = 0; i < mla_array_list_size(currentModule->availableCommands); ++i) {
            mla_cli_command_t *cmd = mla_array_list_get_ref(currentModule->availableCommands, i);
            mla_array_list_add(parser.availableCommands, *cmd);
        }

        // Add all commands to enter sub modules
        for (mla_size_t i = 0; i < mla_array_list_size(currentModule->subModules); ++i) {
            mla_cli_module_t *subModule = mla_array_list_get_ref(currentModule->subModules, i);
            mla_cli_command_t cmdEnterModule = mla_cli_command(subModule->moduleName, mla_private_cli_cmd_open_sub_module_execute);
            mla_pointer_t subModule_ptr = mla_platform_pointer_to_managed_pointer(subModule);

            mla_user_data_set_pointer(cmdEnterModule.user_data, mla_cli_app_user_data_name, app_ptr);
            mla_user_data_set_pointer(cmdEnterModule.user_data, mla_cli_submodule_user_data_name, subModule_ptr);

            mla_array_list_add(parser.availableCommands, cmdEnterModule);
        }
    }


    // Back Command if we are not in the root module
    if (moduleCount > 1) {
        mla_cli_command_t cmdExit = mla_cli_command(mla_string_const("exit"), mla_private_cli_cmd_exit_execute);

        mla_user_data_set_pointer(cmdExit.user_data, mla_cli_app_user_data_name, app_ptr);
        mla_array_list_add(parser.availableCommands, cmdExit);
    }

    // Help command
    mla_cli_command_t cmdHelp = mla_cli_command(mla_string_const("help"), mla_private_cli_cmd_help_execute);
    mla_user_data_set_pointer(cmdHelp.user_data, mla_cli_app_user_data_name, app_ptr);
    mla_array_list_add(parser.availableCommands, cmdHelp);

    return parser;
}

mla_bool_t mla_private_cli_process_parser_result(const mla_string_t& inputCommand, const mla_cli_parser_result &parser_result, mla_stream_output_t &outputStream) {

    if (parser_result.isValid && mla_string_length(parser_result.matchingCommand.name) != 0) {
        // Validate mandatory parameters
        mla_bool_t missingMandatoryParameter = false;

        for (mla_size_t i = 0; i < mla_array_list_size(parser_result.matchingCommand.parameters); ++i) {
            mla_cli_command_parameter_t *param = mla_array_list_get_ref(parser_result.matchingCommand.parameters, i);

            if (param->mandatory && !mla_hash_map_contains(parser_result.matchingParameters, param->parameterName)) {
                mla_private_cli_write_string(outputStream,
                                       mla_string_concat(mla_string("Parameter '"), param->parameterName,
                                                         mla_string("' is mandatory but not provided\n")));
                missingMandatoryParameter = true;
            }
        }

        if (missingMandatoryParameter) {
            // not all mandatory parameters are provided
            return false;
        }

        // Execute the command
        if (parser_result.matchingCommand.execute != nullptr) {

            mla_user_data_t user_data = mla_user_data_empty();
            mla_pointer_t outputStream_ptr = mla_platform_pointer_to_managed_pointer(&outputStream);
            mla_user_data_set_pointer(user_data, mla_stream_output_user_data_name, outputStream_ptr);

            mla_cli_command_execute_outstream_t stringOutstream = {
                user_data,
                mla_private_cli_command_execute_outstream_to_stream_bridge,
                mla_private_cli_command_execute_outstream_buffer_to_stream_bridge,
                mla_private_cli_command_execute_outstream_c_string_to_stream_bridge,

                mla_private_cli_command_execute_outstream_verbose_to_stream_disabled,
                mla_private_cli_command_execute_outstream_verbose_buffer_to_stream_disabled,
                mla_private_cli_command_execute_outstream_verbose_c_string_to_stream_disabled
            };

            if (mla_cli_command_parameter_verbose_output_active(parser_result.matchingCommand, parser_result.matchingParameters)) {
                stringOutstream.writeVerbose = mla_private_cli_command_execute_outstream_verbose_to_stream_bridge;
                stringOutstream.writeVerboseBuffer = mla_private_cli_command_execute_outstream_verbose_buffer_to_stream_bridge;
                stringOutstream.writeVerboseCString = mla_private_cli_command_execute_outstream_verbose_c_string_to_stream_bridge;
            }

            return parser_result.matchingCommand.execute(parser_result.matchingCommand, parser_result.matchingParameters,
                                                  stringOutstream);
        } else {
            mla_error(
                mla_string_concat(mla_string("Command '"), parser_result.matchingCommand.name, mla_string(
                    "' has no execute function")));
            return false;
        }

    } else {
        mla_private_cli_write_string(outputStream, mla_string("Unknown Command :\n"));
        mla_private_cli_write_string(outputStream, mla_string("  "));
        mla_private_cli_write_string(outputStream, inputCommand);
        outputStream.write(outputStream, 0, 1,  mla_r_cast<const mla_byte_t*>("\n"));

        // There is something missing show the possible auto completions
        if (mla_array_list_size(parser_result.possibleAutoCompletions) > 0) {
            outputStream.write(outputStream, 0, 1,  mla_r_cast<const mla_byte_t*>("\n"));
            mla_private_cli_write_string(outputStream, mla_string("Do you mean:\n"));
            for (mla_size_t i = 0; i < mla_array_list_size(parser_result.possibleAutoCompletions); ++i) {
                mla_string_t *completion = mla_array_list_get_ref(parser_result.possibleAutoCompletions, i);
                mla_private_cli_write_string(outputStream, mla_string("  "));
                mla_private_cli_write_string(outputStream, inputCommand);
                mla_private_cli_write_string(outputStream, *completion);
                outputStream.write(outputStream, 0, 1,  mla_r_cast<const mla_byte_t*>("\n"));
            }
        } else {

            // No possible completions found
            mla_private_cli_write_string(outputStream, mla_string("Type 'help' to see available commands.\n"));

        }

        return false;
    }
}

mla_bool_t mla_private_cli_parser_parse_and_execute_command(mla_cli_app_t &app, const mla_string_t &command,
                                                mla_stream_output_t &outputStream) {
    // Setup the parser
    mla_cli_parser_t parser = mla_private_cli_setup_parser(app);

    // Parse the command
    const mla_cli_parser_result parser_result = mla_cli_parser_parse(parser, command);

    // Process the result
    return mla_private_cli_process_parser_result(command, parser_result, outputStream);
}

mla_cli_app_t mla_cli_app_empty() {
    return {
        mla_array_list_empty<mla_init_struct(mla_cli_module_t)>(),
        mla_string_empty(), // currentLine
        0,                  // cursorPos
        0,                  // escState
        0,                  // escParam
        mla_array_list_empty<mla_init_struct(mla_string_t)>(), // history
        -1,                 // historyIndex
        mla_string_empty()  // savedLiveLine
    };
}

mla_cli_app_t mla_cli_app_init(mla_cli_module_t &rootModule, mla_stream_output_t &outputStream) {
    mla_cli_app_t app = {
        mla_array_list<mla_init_struct(mla_cli_module_t)>(2),
        mla_string_empty(), // currentLine
        0,                  // cursorPos
        0,                  // escState
        0,                  // escParam
        mla_array_list<mla_init_struct(mla_string_t)>(8), // history
        -1,                 // historyIndex
        mla_string_empty()  // savedLiveLine
    };

    mla_private_cli_activate_module(app, rootModule);
    mla_private_cli_write_module_prompt(app, outputStream);
    return app;
}

// --------------------------------------------------------------------------
// Interactive line editor
//
// stdin is read in raw, non-blocking mode (see the platform std_read
// implementations). The terminal driver no longer performs line editing or
// echo, so we do it ourselves here: printable characters are inserted at the
// cursor, control keys are interpreted, and the whole line is repainted using
// ANSI escape sequences. A completed line is only handed to the parser when
// the user presses Enter.
//
// Multi-byte keys arrive in two flavours we both understand:
//   * ANSI escape sequences (Linux / VT terminals):  ESC '[' final-byte,
//     e.g. ESC[A = Up, ESC[3~ = Delete.
//   * Windows conio special keys: a 0x00 / 0xE0 prefix followed by a scan
//     code, e.g. 0xE0 0x48 = Up.
// Because reads are non-blocking a sequence can be split across reads, so the
// parse state (escState/escParam) lives on mla_cli_app_t.
// --------------------------------------------------------------------------

enum mla_private_cli_escape_t :mla_uint8_t {
    MLA_CLI_ESC_NORMAL = 0, // not inside an escape sequence
    MLA_CLI_ESC_GOT_ESC,    // saw ESC (0x1B)
    MLA_CLI_ESC_GOT_CSI,    // saw ESC '[' (ANSI control sequence introducer)
    MLA_CLI_ESC_GOT_WIN     // saw 0x00 / 0xE0 (Windows conio special-key prefix)
};

enum mla_private_cli_key_t: mla_uint8_t {
    MLA_CLI_KEY_UP,
    MLA_CLI_KEY_DOWN,
    MLA_CLI_KEY_LEFT,
    MLA_CLI_KEY_RIGHT,
    MLA_CLI_KEY_HOME,
    MLA_CLI_KEY_END,
    MLA_CLI_KEY_DELETE
};

void mla_private_cli_write_c_string(mla_stream_output_t &outputStream, const mla_char_t *str) {
    outputStream.write(outputStream, 0, mla_strlen(str), mla_r_cast<const mla_byte_t*>(str));
}

mla_string_t mla_private_cli_char_to_string(mla_char_t c) {
    mla_char_t buffer[1] = { c };
    return mla_string_copy(buffer, 1);
}

// Repaint the current line: return to column 0, clear the line, write the
// prompt and the edited line, then reposition the cursor.
void mla_private_cli_redraw_line(mla_cli_app_t &app, mla_stream_output_t &outputStream) {
    mla_private_cli_write_c_string(outputStream, "\r\x1b[K");
    mla_private_cli_write_string(outputStream, mla_private_cli_build_module_prompt(app));
    mla_private_cli_write_string(outputStream, app.currentLine);

    mla_size_t lineLength = mla_string_length(app.currentLine);
    if (app.cursorPos < lineLength) {
        // Move the cursor left so it sits at cursorPos again
        mla_string_t moveLeft = mla_string_concat("\x1b[", mla_string_from_size(lineLength - app.cursorPos), "D");
        mla_private_cli_write_string(outputStream, moveLeft);
    }
}

void mla_private_cli_editor_insert_char(mla_cli_app_t &app, mla_char_t c) {
    mla_string_t prefix = mla_string_substr(app.currentLine, 0, app.cursorPos);
    mla_string_t suffix = mla_string_substr(app.currentLine, app.cursorPos);
    app.currentLine = mla_string_concat(prefix, mla_private_cli_char_to_string(c), suffix);
    app.cursorPos++;
}

void mla_private_cli_editor_backspace(mla_cli_app_t &app) {
    if (app.cursorPos == 0) {
        return;
    }
    mla_string_t prefix = mla_string_substr(app.currentLine, 0, app.cursorPos - 1);
    mla_string_t suffix = mla_string_substr(app.currentLine, app.cursorPos);
    app.currentLine = mla_string_concat(prefix, suffix);
    app.cursorPos--;
}

void mla_private_cli_editor_delete(mla_cli_app_t &app) {
    mla_size_t lineLength = mla_string_length(app.currentLine);
    if (app.cursorPos >= lineLength) {
        return;
    }
    mla_string_t prefix = mla_string_substr(app.currentLine, 0, app.cursorPos);
    mla_string_t suffix = mla_string_substr(app.currentLine, app.cursorPos + 1);
    app.currentLine = mla_string_concat(prefix, suffix);
}

// Replace the edited line and move the cursor to its end.
void mla_private_cli_editor_set_line(mla_cli_app_t &app, const mla_string_t &line) {
    app.currentLine = line;
    app.cursorPos = mla_string_length(app.currentLine);
}

void mla_private_cli_history_previous(mla_cli_app_t &app) {
    mla_size_t historySize = mla_array_list_size(app.history);
    if (historySize == 0) {
        return;
    }

    if (app.historyIndex == -1) {
        // Leaving the live line: remember it so Down can restore it later
        app.savedLiveLine = app.currentLine;
        app.historyIndex = mla_s_cast<mla_int32_t>(historySize) - 1;
    } else if (app.historyIndex > 0) {
        app.historyIndex--;
    } else {
        return; // already at the oldest entry
    }

    mla_string_t *entry = mla_array_list_get_ref(app.history, mla_s_cast<mla_size_t>(app.historyIndex));
    mla_private_cli_editor_set_line(app, mla_string_copy(*entry));
}

void mla_private_cli_history_next(mla_cli_app_t &app) {
    if (app.historyIndex == -1) {
        return; // already on the live line
    }

    mla_size_t historySize = mla_array_list_size(app.history);
    if (mla_s_cast<mla_size_t>(app.historyIndex) + 1 < historySize) {
        app.historyIndex++;
        mla_string_t *entry = mla_array_list_get_ref(app.history, mla_s_cast<mla_size_t>(app.historyIndex));
        mla_private_cli_editor_set_line(app, mla_string_copy(*entry));
    } else {
        // Past the newest entry: restore the line the user was typing
        app.historyIndex = -1;
        mla_private_cli_editor_set_line(app, app.savedLiveLine);
    }
}

void mla_private_cli_handle_special_key(mla_cli_app_t &app, mla_private_cli_key_t key, mla_stream_output_t &outputStream) {
    mla_size_t lineLength = mla_string_length(app.currentLine);

    switch (key) {
        case MLA_CLI_KEY_UP:
            mla_private_cli_history_previous(app);
            break;
        case MLA_CLI_KEY_DOWN:
            mla_private_cli_history_next(app);
            break;
        case MLA_CLI_KEY_LEFT:
            if (app.cursorPos > 0) {
                app.cursorPos--;
            }
            break;
        case MLA_CLI_KEY_RIGHT:
            if (app.cursorPos < lineLength) {
                app.cursorPos++;
            }
            break;
        case MLA_CLI_KEY_HOME:
            app.cursorPos = 0;
            break;
        case MLA_CLI_KEY_END:
            app.cursorPos = lineLength;
            break;
        case MLA_CLI_KEY_DELETE:
            mla_private_cli_editor_delete(app);
            break;
    }

    mla_private_cli_redraw_line(app, outputStream);
}

mla_string_t mla_private_cli_longest_common_prefix(const mla_array_list_t<mla_init_struct(mla_string_t)> &candidates) {
    mla_size_t count = mla_array_list_size(candidates);
    if (count == 0) {
        return mla_string_empty();
    }

    const mla_string_t *first = mla_array_list_get_ref(candidates, 0);
    if (first == nullptr) {
        return mla_string_empty();
    }

    mla_size_t firstLen = mla_string_length(*first);
    if (firstLen == 0) {
        return mla_string_empty();
    }

    const mla_char_t *firstData = mla_string_data(*first);
    mla_size_t lcpLen = 0;

    for (mla_size_t charIdx = 0; charIdx < firstLen; ++charIdx) {
        mla_char_t ch = firstData[charIdx];
        mla_bool_t matchAll = true;

        for (mla_size_t i = 1; i < count; ++i) {
            const mla_string_t *cand = mla_array_list_get_ref(candidates, i);
            if (cand == nullptr || charIdx >= mla_string_length(*cand) || mla_string_data(*cand)[charIdx] != ch) {
                matchAll = false;
                break;
            }
        }

        if (!matchAll) {
            break;
        }
        lcpLen++;
    }

    if (lcpLen == 0) {
        return mla_string_empty();
    }

    return mla_string_substr(*first, 0, lcpLen);
}

void mla_private_cli_autocomplete(mla_cli_app_t &app, mla_stream_output_t &outputStream) {
    mla_cli_parser_t parser = mla_private_cli_setup_parser(app);
    mla_cli_parser_result result = mla_cli_parser_parse(parser, app.currentLine);

    mla_size_t completionCount = mla_array_list_size(result.possibleAutoCompletions);

    if (completionCount == 0) {
        // Nothing to complete
        mla_private_cli_write_c_string(outputStream, "\a");
        return;
    }

    if (completionCount == 1) {
        // Unique completion: append the missing suffix and keep editing
        mla_string_t *completion = mla_array_list_get_ref(result.possibleAutoCompletions, 0);
        app.currentLine = mla_string_concat(app.currentLine, *completion);
        app.cursorPos = mla_string_length(app.currentLine);
        mla_private_cli_redraw_line(app, outputStream);
        return;
    }

    // Multiple candidates: check for Longest Common Prefix (LCP)
    mla_string_t lcp = mla_private_cli_longest_common_prefix(result.possibleAutoCompletions);
    mla_size_t lcpLen = mla_string_length(lcp);

    if (lcpLen > 0) {
        app.currentLine = mla_string_concat(app.currentLine, lcp);
        app.cursorPos = mla_string_length(app.currentLine);
    }

    // List remaining candidates underneath, then redraw the prompt with current line
    mla_private_cli_write_c_string(outputStream, "\r\n");
    for (mla_size_t i = 0; i < completionCount; ++i) {
        mla_string_t *completion = mla_array_list_get_ref(result.possibleAutoCompletions, i);
        mla_private_cli_write_c_string(outputStream, "  ");
        mla_private_cli_write_string(outputStream, app.currentLine);
        if (lcpLen > 0 && mla_string_starts_with(*completion, lcp)) {
            mla_string_t remainingSuffix = mla_string_substr(*completion, lcpLen);
            mla_private_cli_write_string(outputStream, remainingSuffix);
        } else {
            mla_private_cli_write_string(outputStream, *completion);
        }
        mla_private_cli_write_c_string(outputStream, "\n");
    }
    mla_private_cli_redraw_line(app, outputStream);
}

// Submit the current line: echo a newline, push to history, execute it, and
// print a fresh prompt. Returns false if the executed command reported failure.
mla_bool_t mla_private_cli_commit_line(mla_cli_app_t &app, mla_stream_output_t &outputStream) {
    mla_private_cli_write_c_string(outputStream, "\r\n");

    mla_string_t line = app.currentLine;

    // Reset the editor for the next line before executing, so a command can
    // safely inspect/modify the app state
    app.currentLine = mla_string_empty();
    app.cursorPos = 0;
    app.historyIndex = -1;
    app.savedLiveLine = mla_string_empty();

    mla_bool_t success = true;

    if (!mla_string_is_empty(line)) {
        mla_array_list_add(app.history, mla_string_copy(line));

        // Split the command using the && operator into multiple commands
        mla_array_list_t<mla_init_struct(mla_string_t)> commands = mla_string_split(line, mla_string_const("&&"));

        mla_size_t commandCount = mla_array_list_size(commands);

        if (commandCount == 0) {
            // Should never happen, but just in case
            success = mla_private_cli_parser_parse_and_execute_command(app, line, outputStream);
        } else {

            for (mla_size_t i = 0; i < commandCount; ++i) {

                mla_string_t& raw_command = mla_array_list_get_unsafe(commands, i);
                mla_string_t command = mla_string_trim(raw_command);

                if (mla_string_is_empty(command)) {
                    continue;
                }

                success = mla_private_cli_parser_parse_and_execute_command(app, command, outputStream);

                if (!success) {
                    break;
                }

            }

        }

    }

    mla_private_cli_write_module_prompt(app, outputStream);
    return success;
}

// Handle a single byte while not inside an escape sequence.
// Returns false if committing a command reported failure.
mla_bool_t mla_private_cli_handle_normal_byte(mla_cli_app_t &app, mla_uint8_t b, mla_stream_output_t &outputStream) {
    switch (b) {
        case 0x1B: // ESC -> possibly an ANSI escape sequence
            app.escState = MLA_CLI_ESC_GOT_ESC;
            return true;
        case 0x00:
        case 0xE0: // Windows conio special-key prefix
            app.escState = MLA_CLI_ESC_GOT_WIN;
            return true;
        case '\r':
        case '\n':
            return mla_private_cli_commit_line(app, outputStream);
        case '\t':
            mla_private_cli_autocomplete(app, outputStream);
            return true;
        case 0x7F: // DEL (Backspace on most terminals)
        case 0x08: // BS  (Ctrl-H / Backspace)
            mla_private_cli_editor_backspace(app);
            mla_private_cli_redraw_line(app, outputStream);
            return true;
        case 0x03: // Ctrl-C -> discard the current line
            mla_private_cli_write_c_string(outputStream, "^C\r\n");
            app.currentLine = mla_string_empty();
            app.cursorPos = 0;
            app.historyIndex = -1;
            app.savedLiveLine = mla_string_empty();
            mla_private_cli_write_module_prompt(app, outputStream);
            return true;
        default:
            break;
    }

    // Printable ASCII: insert it at the cursor. All other control bytes are ignored.
    if (b >= 0x20 && b <= 0x7E) {
        mla_private_cli_editor_insert_char(app, mla_s_cast<mla_char_t>(b));
        mla_private_cli_redraw_line(app, outputStream);
    }

    return true;
}

// Handle a byte after ESC '[' (an ANSI control sequence).
void mla_private_cli_handle_csi_byte(mla_cli_app_t &app, mla_uint8_t b, mla_stream_output_t &outputStream) {
    if (b >= '0' && b <= '9') {
        // Accumulate the numeric parameter (e.g. the 3 in ESC[3~)
        app.escParam = mla_s_cast<mla_uint8_t>((app.escParam * 10) + (b - '0'));
        return; // still inside the sequence
    }

    mla_bool_t handled = true;
    mla_private_cli_key_t key = MLA_CLI_KEY_UP;

    switch (b) {
        case 'A': key = MLA_CLI_KEY_UP; break;
        case 'B': key = MLA_CLI_KEY_DOWN; break;
        case 'C': key = MLA_CLI_KEY_RIGHT; break;
        case 'D': key = MLA_CLI_KEY_LEFT; break;
        case 'H': key = MLA_CLI_KEY_HOME; break;
        case 'F': key = MLA_CLI_KEY_END; break;
        case '~':
            // ESC [ <n> ~  (1/7 = Home, 3 = Delete, 4/8 = End)
            switch (app.escParam) {
                case 1: case 7: key = MLA_CLI_KEY_HOME; break;
                case 4: case 8: key = MLA_CLI_KEY_END; break;
                case 3: key = MLA_CLI_KEY_DELETE; break;
                default: handled = false; break;
            }
            break;
        default:
            handled = false;
            break;
    }

    if (handled) {
        mla_private_cli_handle_special_key(app, key, outputStream);
    }

    app.escState = MLA_CLI_ESC_NORMAL;
    app.escParam = 0;
}

// Handle a Windows conio scan code after a 0x00 / 0xE0 prefix.
void mla_private_cli_handle_win_key(mla_cli_app_t &app, mla_uint8_t b, mla_stream_output_t &outputStream) {
    mla_bool_t handled = true;
    mla_private_cli_key_t key = MLA_CLI_KEY_UP;

    switch (b) {
        case 0x48: key = MLA_CLI_KEY_UP; break;
        case 0x50: key = MLA_CLI_KEY_DOWN; break;
        case 0x4B: key = MLA_CLI_KEY_LEFT; break;
        case 0x4D: key = MLA_CLI_KEY_RIGHT; break;
        case 0x47: key = MLA_CLI_KEY_HOME; break;
        case 0x4F: key = MLA_CLI_KEY_END; break;
        case 0x53: key = MLA_CLI_KEY_DELETE; break;
        default: handled = false; break;
    }

    if (handled) {
        mla_private_cli_handle_special_key(app, key, outputStream);
    }

    app.escState = MLA_CLI_ESC_NORMAL;
}

mla_bool_t mla_cli_app_update_and_process_input(mla_cli_app_t &app, mla_stream_input_t &inputStream,
                                          mla_stream_output_t &outputStream) {

    mla_byte_t buffer[mla_global_config_stream_fast_read_buffer_size] = {0};
    mla_size_t bytesRead = inputStream.read(inputStream, 0, sizeof(buffer), buffer);

    if (bytesRead == 0) {
        return true; // Nothing available this tick
    }

    mla_bool_t success = true;

    for (mla_size_t i = 0; i < bytesRead; ++i) {
        mla_uint8_t b = buffer[i];

        switch (app.escState) {
            case MLA_CLI_ESC_GOT_ESC:
                if (b == '[') {
                    app.escState = MLA_CLI_ESC_GOT_CSI;
                    app.escParam = 0;
                } else {
                    // Lone ESC (or Alt-<key>): handle this byte normally
                    app.escState = MLA_CLI_ESC_NORMAL;
                    if (!mla_private_cli_handle_normal_byte(app, b, outputStream)) {
                        success = false;
                    }
                }
                break;
            case MLA_CLI_ESC_GOT_CSI:
                mla_private_cli_handle_csi_byte(app, b, outputStream);
                break;
            case MLA_CLI_ESC_GOT_WIN:
                mla_private_cli_handle_win_key(app, b, outputStream);
                break;
            default:
                if (!mla_private_cli_handle_normal_byte(app, b, outputStream)) {
                    success = false;
                }
                break;
        }
    }

    return success;
}


mla_cli_module_t mla_cli_module(const mla_string_t& name) {
    return {
        name,
        mla_string_empty(),
        mla_array_list_empty<mla_init_struct(mla_cli_command_t)>(),
        mla_array_list_empty<mla_init_struct(mla_cli_module_t)>()
    };
}

mla_cli_module_t mla_cli_module(const mla_string_t& name, const mla_string_t& description) {
    return {
        name,
        description,
        mla_array_list_empty<mla_init_struct(mla_cli_command_t)>(),
        mla_array_list_empty<mla_init_struct(mla_cli_module_t)>()
    };
}

void mla_cli_module_add_command(mla_cli_module_t& module, const mla_cli_command_t& command) {
    mla_array_list_add(module.availableCommands, command);
}

void mla_cli_module_add_sub_module(mla_cli_module_t& module, const mla_cli_module_t& subModule) {
    mla_array_list_add(module.subModules, subModule);
}

const mla_cli_command_t* mla_cli_module_find_command(const mla_cli_module_t& module, const mla_string_t& commandName) {

    for (mla_size_t i = 0; i < mla_array_list_size(module.availableCommands); ++i) {

        const mla_cli_command_t* command = mla_array_list_get_ref(module.availableCommands, i);

        if (mla_string_equals(command->name, commandName)) {
            return mla_array_list_get_ref(module.availableCommands, i);
        }

    }

    return nullptr;

}
