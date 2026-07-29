//
// Created by christian on 9/13/25.
//

#ifndef MLA_CLI_PARSER_H
#define MLA_CLI_PARSER_H

#include "../system/mla_string.h"
#include "../system/mla_array_list.h"
#include "../system/mla_hash_map.h"
#include "mla_cli_command.h"


struct mla_cli_parser_t {
    mla_array_list_t<mla_init_struct(mla_cli_command_t)> availableCommands;
};

struct mla_cli_parser_result {

    mla_bool_t isValid; // True if all infos of the command are scuressfull parsed
    mla_cli_command_t matchingCommand; // The command which matches the input command
    mla_hash_map_t<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)> matchingParameters; // The parameters which are parsed from the input command
    mla_array_list_t<mla_init_struct(mla_string_t)> possibleAutoCompletions; // The possible auto completions for the input command

};

mla_cli_parser_t mla_cli_parser();

mla_cli_parser_result mla_cli_parser_parse(const mla_cli_parser_t& parser, const mla_string_t& command);

#endif
