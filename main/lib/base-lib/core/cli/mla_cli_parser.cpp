//
// Created by christian on 9/13/25.
//

#include "mla_cli_parser.h"

#include "../system/mla_string_concat.h"

mla_cli_parser_t mla_cli_parser() {
    return {
        mla_array_list<mla_init_struct(mla_cli_command_t)>()
    };
}

mla_cli_parser_result mla_cli_parser_parse(const mla_cli_parser_t &parser, const mla_string_t &command) {
    mla_cli_parser_result result = {
        false,
        mla_cli_command_t::init(),
        mla_hash_map_empty<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)>(),
        mla_array_list_empty<mla_init_struct(mla_string_t)>()
    };

    // We will parse the command char by char with following rules
    // 1. The command starts with the command name followed by a space
    // 2. After the command name, parameters have the form --parameterName parameterValue.
    //    Parameters declared as flags have the form --parameterName and do not take a value.

    // Parameter values can be sounded by quotes to allow spaces in the value

    // if not all parts of the command are successfully aligned to a command is not valid

    mla_size_t matchedPositon = 0;

    mla_size_t commandLength = mla_string_length(command);
    const mla_char_t* commandData = mla_string_data(command);

    // We are looking for the command name
    for (mla_size_t j = 0; j < mla_array_list_size(parser.availableCommands); j++) {
        const mla_cli_command_t &cmd = mla_array_list_get_unsafe(parser.availableCommands, j);

        if (mla_string_starts_with(command, cmd.name)) {

            mla_size_t cmdNameLength = mla_string_length(cmd.name);
            // Check for Perfect Match
            if (commandLength == cmdNameLength || commandData[cmdNameLength] == ' ') {
                result.matchingCommand = cmd;
                matchedPositon = cmdNameLength;
                break;
            }
        }
    }

    if (mla_string_length(result.matchingCommand.name) == 0) {

        // Autocomplete possible commands
        for (mla_size_t j = 0; j < mla_array_list_size(parser.availableCommands); j++) {
            const mla_cli_command_t &cmd = mla_array_list_get_unsafe(parser.availableCommands, j);
            if (mla_string_starts_with(cmd.name, command)) {
                mla_array_list_add(result.possibleAutoCompletions,
                                   mla_string_substr(cmd.name, commandLength));
            }
        }

        // No matching command found
        return result;
    }

    // Skip whitespace between command and parameters
    while (matchedPositon < commandLength && commandData[matchedPositon] == ' ') {
        matchedPositon++;
    }

    mla_string_t activeParamForValueAutocomplete = mla_string_empty();
    mla_string_t activeParamValuePrefix = mla_string_empty();
    mla_bool_t isValueAutocompleteActive = false;

    // Match the parameters
    while (matchedPositon < commandLength) {
        // Skip whitespace before parameter
        while (matchedPositon < commandLength && commandData[matchedPositon] == ' ') {
            matchedPositon++;
        }

        if (matchedPositon >= commandLength) {
            break;
        }

        // Check if we are at the beginning of a parameter (--paramName)
        if (matchedPositon + 1 >= commandLength || commandData[matchedPositon] != '-' || commandData[matchedPositon + 1] != '-') {
            break;
        }

        // Find the end of the parameter name
        mla_size_t paramNameStart = matchedPositon + 2;
        mla_size_t paramNameEnd = paramNameStart;
        while (paramNameEnd < commandLength && commandData[paramNameEnd] != ' ' && commandData[paramNameEnd] != '=') {
            paramNameEnd++;
        }

        if (paramNameEnd == paramNameStart) {
            // No parameter name found
            break;
        }

        mla_string_t paramName = mla_string_substr(command, paramNameStart, paramNameEnd - paramNameStart);

        mla_bool_t isFlag = false;
        const mla_cli_command_parameter_t *matchingParamRef = nullptr;
        for (mla_size_t i = 0; i < mla_array_list_size(result.matchingCommand.parameters); ++i) {
            const mla_cli_command_parameter_t *commandParameter =
                mla_array_list_get_ref(result.matchingCommand.parameters, i);
            if (mla_string_equals(commandParameter->parameterName, paramName)) {
                isFlag = commandParameter->is_flag;
                matchingParamRef = commandParameter;
                break;
            }
        }

        if (isFlag) {
            isValueAutocompleteActive = false;
            mla_hash_map_push(result.matchingParameters, paramName, mla_string_empty());
            matchedPositon = paramNameEnd;

            // Skip whitespace after flag
            while (matchedPositon < commandLength && commandData[matchedPositon] == ' ') {
                matchedPositon++;
            }
            continue;
        }

        // If the parameter name ends right at the end of the input string without a space or '=',
        // the user is still editing the parameter name. Do not activate parameter value autocomplete.
        if (paramNameEnd >= commandLength) {
            isValueAutocompleteActive = false;
            break;
        }

        if (matchingParamRef != nullptr) {
            matchedPositon = paramNameEnd;
        }

        // Parameter values require a space or '=' delimiter following the parameter name
        if (commandData[paramNameEnd] != ' ' && commandData[paramNameEnd] != '=') {
            isValueAutocompleteActive = false;
            break;
        }

        matchedPositon = paramNameEnd;

        // Skip whitespace and optional '=' between parameter name and value
        while (matchedPositon < commandLength && (commandData[matchedPositon] == ' ' || commandData[matchedPositon] == '=')) {
            matchedPositon++;
        }

        // Extract the parameter value
        mla_size_t paramValueStart = matchedPositon;

        // Check if the parameter value is quoted
        mla_bool_t isQuoted = false;
        mla_char_t quoteChar = '\"';

        if (paramValueStart < commandLength && (commandData[paramValueStart] == '\"' || commandData[paramValueStart] == '\'')) {
            isQuoted = true;
            quoteChar = commandData[paramValueStart];
            paramValueStart++;
        }

        if (paramValueStart >= commandLength && !isQuoted) {
            // No parameter value found yet
            if (matchingParamRef != nullptr && matchingParamRef->value_autocomplete_fn != nullptr) {
                activeParamForValueAutocomplete = paramName;
                activeParamValuePrefix = mla_string_empty();
                isValueAutocompleteActive = true;
            } else {
                isValueAutocompleteActive = false;
            }
            break;
        }

        // Find the end of the parameter value
        mla_size_t paramValueEnd = paramValueStart;
        mla_bool_t closedQuoteFound = false;

        if (isQuoted) {
            while (paramValueEnd < commandLength) {
                if (commandData[paramValueEnd] == '\\' && paramValueEnd + 1 < commandLength) {
                    paramValueEnd += 2;
                } else if (commandData[paramValueEnd] == quoteChar) {
                    closedQuoteFound = true;
                    break;
                } else {
                    paramValueEnd++;
                }
            }
        } else {
            while (paramValueEnd < commandLength && commandData[paramValueEnd] != ' ') {
                paramValueEnd++;
            }
        }

        if (isQuoted && !closedQuoteFound) {
            // Unterminated quote
            if (matchingParamRef != nullptr && matchingParamRef->value_autocomplete_fn != nullptr) {
                activeParamForValueAutocomplete = paramName;
                activeParamValuePrefix = mla_string_substr(command, paramValueStart, paramValueEnd - paramValueStart);
                isValueAutocompleteActive = true;
            } else {
                isValueAutocompleteActive = false;
            }
            break;
        }

        mla_string_t paramValue = mla_string_empty();

        if (isQuoted) {
            mla_bool_t hasEscapes = false;
            for (mla_size_t k = paramValueStart; k < paramValueEnd; ++k) {
                if (commandData[k] == '\\') {
                    hasEscapes = true;
                    break;
                }
            }

            if (hasEscapes) {
                mla_size_t rawLen = paramValueEnd - paramValueStart;
                mla_char_t* tempBuf = mla_r_cast<mla_char_t*>(mla_platform_malloc(rawLen + 1));
                mla_size_t outIdx = 0;
                for (mla_size_t k = paramValueStart; k < paramValueEnd; ++k) {
                    if (commandData[k] == '\\' && k + 1 < paramValueEnd) {
                        k++;
                        tempBuf[outIdx++] = commandData[k];
                    } else {
                        tempBuf[outIdx++] = commandData[k];
                    }
                }
                tempBuf[outIdx] = '\0';
                paramValue = mla_string_copy(tempBuf, outIdx);
                mla_platform_free(tempBuf);
            } else {
                paramValue = mla_string_substr(command, paramValueStart, paramValueEnd - paramValueStart);
            }

            if (paramValueEnd < commandLength && commandData[paramValueEnd] == quoteChar) {
                paramValueEnd++; // Skip the ending quote
            }
        } else {
            paramValue = mla_string_substr(command, paramValueStart, paramValueEnd - paramValueStart);
        }

        mla_hash_map_push(result.matchingParameters, paramName, paramValue);
        matchedPositon = paramValueEnd;

        if ((paramValueEnd == commandLength || (paramValueEnd < commandLength && commandData[paramValueEnd] == ' ')) &&
            matchingParamRef != nullptr && matchingParamRef->value_autocomplete_fn != nullptr) {
            activeParamForValueAutocomplete = paramName;
            activeParamValuePrefix = paramValue;
            isValueAutocompleteActive = true;
        } else {
            isValueAutocompleteActive = false;
        }

        // Skip whitespace between parameters
        while (matchedPositon < commandLength && commandData[matchedPositon] == ' ') {
            matchedPositon++;
        }
    }

    // Skip ending whitespace
    while (matchedPositon < commandLength && commandData[matchedPositon] == ' ') {
        matchedPositon++;
    }

    // All Data Parsed or ended cleanly
    if (matchedPositon == commandLength) {
        result.isValid = true;
    }

    // Autocomplete possible parameters or parameter values

    if (isValueAutocompleteActive && !mla_string_is_empty(activeParamForValueAutocomplete)) {
        const mla_cli_command_parameter_t *matchingParam = nullptr;
        for (mla_size_t i = 0; i < mla_array_list_size(result.matchingCommand.parameters); ++i) {
            const mla_cli_command_parameter_t *p = mla_array_list_get_ref(result.matchingCommand.parameters, i);
            if (mla_string_equals(p->parameterName, activeParamForValueAutocomplete)) {
                matchingParam = p;
                break;
            }
        }

        if (matchingParam != nullptr && matchingParam->value_autocomplete_fn != nullptr) {
            mla_array_list_t<mla_init_struct(mla_string_t)> rawCompletions =
                matchingParam->value_autocomplete_fn(result.matchingCommand, activeParamForValueAutocomplete,
                    activeParamValuePrefix, result.matchingCommand.user_data);

            mla_size_t prefixLen = mla_string_length(activeParamValuePrefix);
            for (mla_size_t i = 0; i < mla_array_list_size(rawCompletions); ++i) {
                const mla_string_t *candidate = mla_array_list_get_ref(rawCompletions, i);
                if (candidate != nullptr && mla_string_starts_with(*candidate, activeParamValuePrefix)) {
                    mla_string_t suffix = mla_string_substr(*candidate, prefixLen);
                    if (mla_string_length(suffix) > 0) {
                        mla_array_list_add(result.possibleAutoCompletions, suffix);
                    }
                }
            }
        }
    } else if (matchedPositon + 1 < commandLength && commandData[matchedPositon] == '-' && commandData[matchedPositon + 1] == '-') {

        // We are in the middle of a parameter name
        mla_size_t paramNameStart = matchedPositon + 2;
        mla_size_t paramNameEnd = paramNameStart;
        while (paramNameEnd < commandLength && commandData[paramNameEnd] != ' ' && commandData[paramNameEnd] != '=') {
            paramNameEnd++;
        }
        mla_string_t paramName = mla_string_substr(command, paramNameStart, paramNameEnd - paramNameStart);
        for (mla_size_t i = 0; i < mla_array_list_size(result.matchingCommand.parameters); ++i) {
            mla_cli_command_parameter_t *command_parameter = mla_array_list_get_ref(result.matchingCommand.parameters, i);

            if (mla_hash_map_contains(result.matchingParameters, command_parameter->parameterName)) {
                continue; // Command already used
            }

            if (mla_string_starts_with(command_parameter->parameterName, paramName)) {
                mla_string_t suffix = mla_string_substr(command_parameter->parameterName, mla_string_length(paramName));
                if (mla_string_length(suffix) > 0) {
                    mla_array_list_add(result.possibleAutoCompletions, suffix);
                }
            }
        }
    } else {
        // List all the Parameters which are not already used

        for (mla_size_t i = 0; i < mla_array_list_size(result.matchingCommand.parameters); ++i) {
            mla_cli_command_parameter_t *command_parameter = mla_array_list_get_ref(result.matchingCommand.parameters, i);

            if (mla_hash_map_contains(result.matchingParameters, command_parameter->parameterName)) {
                continue; // Command already used
            }

            mla_array_list_add(result.possibleAutoCompletions, mla_string_concat(" --", command_parameter->parameterName));
        }
    }

    return result;
}
