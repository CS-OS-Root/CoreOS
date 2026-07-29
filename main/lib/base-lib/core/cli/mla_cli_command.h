//
// Created by christian on 9/13/25.
//

#ifndef MLA_CLI_COMMAND_H
#define MLA_CLI_COMMAND_H

#include "../system/mla_string.h"
#include "../system/mla_array_list.h"
#include "../system/mla_hash_map.h"
#include "../system/mla_stream.h"
#include "../system/mla_user_data.h"

#define mla_cli_command_verbose_parameter_name "verbose"

/**
 * @brief Forward declaration of the CLI command structure.
 */
struct mla_cli_command_t;

/**
 * @typedef mla_cli_parameter_value_autocomplete_fn
 * @brief Function pointer signature for parameter value autocompletion callbacks.
 *
 * @param command The matching CLI command being parsed.
 * @param parameterName The name of the parameter requiring value autocompletion.
 * @param currentValuePrefix The current prefix string typed by the user for the parameter value.
 * @param userData User data attached to the command.
 * @return mla_array_list_t<mla_string_t, mla_string_initializer> Candidate completion strings.
 */
typedef mla_array_list_t<mla_init_struct(mla_string_t)> (*mla_cli_parameter_value_autocomplete_fn)(
    const mla_cli_command_t &command,
    const mla_string_t &parameterName,
    const mla_string_t &currentValuePrefix,
    const mla_user_data_t &userData);

/**
 * @struct mla_cli_command_parameter_t
 * @brief Represents a single parameter associated with a CLI command.
 */
struct mla_cli_command_parameter_t {
    /** Name of the parameter (e.g., "host", "port", "format"). */
    mla_string_t parameterName;
    /** Human-readable description of the parameter for help text. */
    mla_string_t description;
    /** True if the parameter is required for command execution. */
    mla_bool_t mandatory;
    /** True when the parameter is activated by its presence and does not take a value. */
    mla_bool_t is_flag;
    /** Optional callback for parameter value autocompletion. */
    mla_cli_parameter_value_autocomplete_fn value_autocomplete_fn;

    static mla_cli_command_parameter_t init() {
        return {
            mla_string_empty(),
            mla_string_empty(),
            false,
            false,
            nullptr
        };
    }
};

/**
 * @brief Constructs a mla_cli_command_parameter_t instance without description.
 *
 * @param p_ParameterName Name of the parameter.
 * @param p_Mandatory Whether the parameter is mandatory.
 * @param p_IsFlag Whether the parameter is a boolean flag.
 * @param p_AutocompleteFn Optional value autocompletion callback.
 * @return mla_cli_command_parameter_t Initialized parameter structure.
 */
inline mla_cli_command_parameter_t mla_cli_command_parameter(const mla_string_t& p_ParameterName, mla_bool_t p_Mandatory,
    mla_bool_t p_IsFlag = false, mla_cli_parameter_value_autocomplete_fn p_AutocompleteFn = nullptr) {
    return {
        p_ParameterName,
        mla_string_empty(),
        p_Mandatory,
        p_IsFlag,
        p_AutocompleteFn
    };
}

/**
 * @brief Constructs a mla_cli_command_parameter_t instance with description.
 *
 * @param p_ParameterName Name of the parameter.
 * @param p_Description Description of the parameter.
 * @param p_Mandatory Whether the parameter is mandatory.
 * @param p_IsFlag Whether the parameter is a boolean flag.
 * @param p_AutocompleteFn Optional value autocompletion callback.
 * @return mla_cli_command_parameter_t Initialized parameter structure.
 */
inline mla_cli_command_parameter_t mla_cli_command_parameter(const mla_string_t& p_ParameterName,
    const mla_string_t& p_Description, mla_bool_t p_Mandatory, mla_bool_t p_IsFlag = false,
    mla_cli_parameter_value_autocomplete_fn p_AutocompleteFn = nullptr) {
    return {
        p_ParameterName,
        p_Description,
        p_Mandatory,
        p_IsFlag,
        p_AutocompleteFn
    };
}



/**
 * @brief Helper to set the value autocompletion callback on a CLI command parameter.
 *
 * @param parameter Target CLI command parameter.
 * @param p_AutocompleteFn Autocompletion callback function pointer.
 */
inline void mla_cli_command_parameter_set_value_autocomplete_fn(mla_cli_command_parameter_t &parameter,
    mla_cli_parameter_value_autocomplete_fn p_AutocompleteFn) {
    parameter.value_autocomplete_fn = p_AutocompleteFn;
}

/**
 * @brief Filters a list of candidate string values against a prefix.
 *
 * Dataflow: Iterates through the provided candidate string array list, checks if each candidate
 * starts with `prefix`, and returns a new array list containing all matching candidates.
 *
 * @param candidates List of candidate string values.
 * @param prefix Prefix string to filter candidates against.
 * @return mla_array_list_t<mla_string_t, mla_string_initializer> Array list of matching candidate strings.
 */
inline mla_array_list_t<mla_init_struct(mla_string_t)> mla_cli_parameter_value_autocomplete_filter_candidates(
    const mla_array_list_t<mla_init_struct(mla_string_t)> &candidates,
    const mla_string_t &prefix) {
    mla_array_list_t<mla_init_struct(mla_string_t)> result =
        mla_array_list_empty<mla_init_struct(mla_string_t)>();

    for (mla_size_t i = 0; i < mla_array_list_size(candidates); ++i) {
        const mla_string_t *candidate = mla_array_list_get_ref(candidates, i);
        if (candidate != nullptr && mla_string_starts_with(*candidate, prefix)) {
            mla_array_list_add(result, *candidate);
        }
    }
    return result;
}

/**
 * @brief Filters an array of C-string literal candidates against a prefix.
 *
 * Dataflow: Converts each non-null C-string in the candidate array into an mla_string_t, checks if it
 * starts with `prefix`, and populates the returned array list with matches.
 *
 * @param candidates Array of null-terminated C-string pointers.
 * @param count Number of elements in the candidates array.
 * @param prefix Prefix string to filter candidates against.
 * @return mla_array_list_t<mla_init_struct(mla_string_t)> Array list of matching candidate strings.
 */
inline mla_array_list_t<mla_init_struct(mla_string_t)> mla_cli_parameter_value_autocomplete_filter_c_strings(
    const mla_char_t* const* candidates,
    mla_size_t count,
    const mla_string_t &prefix) {
    mla_array_list_t<mla_init_struct(mla_string_t)> result =
        mla_array_list_empty<mla_init_struct(mla_string_t)>();

    for (mla_size_t i = 0; i < count; ++i) {
        if (candidates[i] != nullptr) {
            mla_string_t candStr = mla_string(candidates[i]);
            if (mla_string_starts_with(candStr, prefix)) {
                mla_array_list_add(result, candStr);
            }
        }
    }
    return result;
}

struct mla_cli_command_t;

struct mla_cli_command_execute_outstream_t {
    mla_user_data_t userdata;

    // Regular
    void (*write)(const mla_user_data_t& userdata, const mla_string_t &data);
    void (*writeBuffer)(const mla_user_data_t& userdata, const mla_char_t* data, mla_size_t length);
    void (*writeCString)(const mla_user_data_t& userdata, const mla_char_t* data);

    // Verbose
    void (*writeVerbose)(const mla_user_data_t& userdata, const mla_string_t &data);
    void (*writeVerboseBuffer)(const mla_user_data_t& userdata, const mla_char_t* data, mla_size_t length);
    void (*writeVerboseCString)(const mla_user_data_t& userdata, const mla_char_t* data);
};

mla_stream_output_t mla_cli_command_execute_outstream_as_stream_output(const mla_cli_command_execute_outstream_t &out);
mla_stream_output_t mla_cli_command_execute_outstream_verbose_as_stream_output(const mla_cli_command_execute_outstream_t &out);

typedef mla_bool_t (*mla_cli_command_execute_t)(const mla_cli_command_t &command,
                                          const mla_hash_map_t<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)> &parameters,
                                          const mla_cli_command_execute_outstream_t &out);

struct mla_cli_command_t {
    mla_string_t name;
    mla_string_t description;
    mla_array_list_t<mla_init_struct(mla_cli_command_parameter_t)> parameters;
    mla_cli_command_execute_t execute;
    mla_user_data_t user_data;

    static mla_cli_command_t init() {
        return {
            mla_string_empty(),
            mla_string_empty(),
            mla_array_list_empty<mla_init_struct(mla_cli_command_parameter_t)>(),
            nullptr,
            mla_user_data_empty()
        };
    }
};

inline mla_cli_command_t mla_cli_command(const mla_string_t &p_Name, const mla_string_t &p_Description, const mla_cli_command_execute_t &p_Execute) {
    return {
        p_Name,
        p_Description,
        mla_array_list_empty<mla_init_struct(mla_cli_command_parameter_t)>(),
        p_Execute,
        mla_user_data_empty()
    };
}

inline mla_cli_command_t mla_cli_command(const mla_string_t &p_Name, const mla_cli_command_execute_t &p_Execute) {
    return {
        p_Name,
        mla_string_empty(),
        mla_array_list_empty<mla_init_struct(mla_cli_command_parameter_t)>(),
        p_Execute,
        mla_user_data_empty()
    };
}

inline void mla_cli_command_add_parameter(mla_cli_command_t &command, const mla_cli_command_parameter_t &parameter) {
    mla_array_list_add(command.parameters, parameter);
}

inline void mla_cli_command_add_parameter(mla_cli_command_t &command, const mla_string_t &parameterName, mla_bool_t mandatory) {
    mla_cli_command_add_parameter(command, mla_cli_command_parameter(parameterName, mandatory));
}

inline void mla_cli_command_add_parameter(mla_cli_command_t &command, const mla_string_t &parameterName,
    mla_bool_t mandatory, mla_bool_t isFlag) {
    mla_cli_command_add_parameter(command, mla_cli_command_parameter(parameterName, mandatory, isFlag));
}

inline void mla_cli_command_add_parameter(mla_cli_command_t &command, const mla_string_t &parameterName, const mla_string_t &description, mla_bool_t mandatory) {
    mla_cli_command_add_parameter(command, mla_cli_command_parameter(parameterName, description, mandatory));
}

inline void mla_cli_command_add_parameter(mla_cli_command_t &command, const mla_string_t &parameterName,
    const mla_string_t &description, mla_bool_t mandatory, mla_bool_t isFlag) {
    mla_cli_command_add_parameter(command, mla_cli_command_parameter(parameterName, description, mandatory, isFlag));
}

inline void mla_cli_command_add_parameter_verbose_output(mla_cli_command_t &command) {
    mla_cli_command_add_parameter(command, mla_string(mla_cli_command_verbose_parameter_name),
        mla_string("Enable verbose output"), false, true);
}

inline mla_string_t mla_cli_command_get_parameter_value(const mla_cli_command_t &command,
    const mla_hash_map_t<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)> &parameters,
    const mla_string_t &parameterName, const mla_string_t &defaultValue = mla_string_empty()) {

    (void)command;

    mla_string_t out = mla_string_empty();

    if (!mla_hash_map_get(parameters, parameterName, out)) {
        return defaultValue;
    }

    return out;
}

inline mla_bool_t mla_cli_command_get_switch_value(const mla_cli_command_t &command, const mla_hash_map_t<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)> &parameters,
    const mla_string_t &parameterName, mla_bool_t defaultValue = false) {

    mla_string_t value = mla_cli_command_get_parameter_value(command, parameters, parameterName);

    if (mla_string_is_empty(value)) {

        // maybe it is a switch parameter, check if the parameter exists in the parameters map
        mla_bool_t exists = mla_hash_map_contains(parameters, parameterName);
        return exists ? true : defaultValue;

    }

    return mla_string_equals_ignore_case(value, mla_string_const("true")) || mla_string_equals_ignore_case(value, mla_string_const("1"));

}

inline mla_bool_t mla_cli_command_parameter_verbose_output_active(const mla_cli_command_t &command,const mla_hash_map_t<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)> &parameters) {

    return mla_cli_command_get_switch_value(command, parameters, mla_string(mla_cli_command_verbose_parameter_name));
}

#endif
