//
// Created by christian on 8/8/25.
//

#ifndef MLA_LOGGER_H
#define MLA_LOGGER_H

#include "../system/mla_string.h"
#include "../system/mla_array_list.h"
#include "../system/mla_user_data.h"

enum mla_log_level: mla_uint8_t {
    MLA_LOG_LEVEL_VERBOSE = 0, 	// Used for show verboses messages
    MLA_LOG_LEVEL_DEBUG = 1,   	// Used for show debug messages
    MLA_LOG_LEVEL_INFO = 2,		// Used for show info messages
    MLA_LOG_LEVEL_WARNING = 3,	// Used for show warning messages
    MLA_LOG_LEVEL_ERROR = 4,		// Used for show error messages
};

typedef void(*mla_log_writer_t)(const mla_log_level level, mla_string_t& message, mla_string_t& context1, mla_user_data_t& userData);

// Logger which can be register at the mla_logger_manager
struct mla_logger_t {
    mla_string_t name; // The name of the logger
    mla_log_level level; // The log level of the logger
    mla_bool_t need_full_managed_strings; // Indicates if the logger needs fully managed strings
    mla_log_writer_t write;  // The function to write log messages
    mla_user_data_t userData; // User data for the log writer function

    mla_bool_t operator==(const mla_logger_t& other) const {
        return level == other.level && write == other.write && mla_user_data_equal(userData, other.userData);
    }

    static mla_logger_t init() {
        return  {
            mla_string_empty(),
            MLA_LOG_LEVEL_ERROR,
            false,
            nullptr,
            mla_user_data_empty()
        };
    }
};

struct mla_logger_manager_t {
    mla_array_list_t<mla_init_struct(mla_logger_t)> loggers;
};

mla_int32_t mla_log_indexOf_logger(const mla_string_t& loggerName);
mla_bool_t mla_log_register_logger(const mla_logger_t& logger);
mla_bool_t mla_log_unregister_logger(const mla_string_t& loggerName);
mla_bool_t mla_log_set_logger_level(const mla_string_t& loggerName, const mla_log_level level);
mla_log_level mla_log_get_logger_level(const mla_string_t& loggerName);
mla_bool_t mla_log_is_active(const mla_log_level level);

/// LOG MESSAGE
////////////////////////////////////////////////

void mla_log_message(const mla_log_level level, const mla_string_t& message, const mla_string_t& context1);
void mla_log_message(const mla_log_level level, const mla_string_t& message, const mla_char_t* context1);
void mla_log_message(const mla_log_level level, const mla_char_t* message, const mla_char_t* context1);

/// VERBOSE
///////////////////////////////////////////////////

void mla_log_verbose(const mla_string_t& message, const mla_string_t& context1);
void mla_log_verbose(const mla_string_t& message, const mla_char_t* context1);
void mla_log_verbose(const mla_char_t* message, const mla_char_t* context1);

/// DEBUG
///////////////////////////////////////////////////

void mla_log_debug(const mla_string_t& message, const mla_string_t& context1);
void mla_log_debug(const mla_string_t& message, const mla_char_t* context1);
void mla_log_debug(const mla_char_t* message, const mla_char_t* context1);

/// INFO
///////////////////////////////////////////////////

void mla_log_info(const mla_string_t& message, const mla_string_t& context1);
void mla_log_info(const mla_string_t& message, const mla_char_t* context1);
void mla_log_info(const mla_char_t* message, const mla_char_t* context1);

/// WARNING
///////////////////////////////////////////////////

void mla_log_warning(const mla_string_t& message, const mla_string_t& context1);
void mla_log_warning(const mla_string_t& message, const mla_char_t* context1);
void mla_log_warning(const mla_char_t* message, const mla_char_t* context1);

/// ERROR
///////////////////////////////////////////////////

void mla_log_error(const mla_string_t& message, const mla_string_t& context1);
void mla_log_error(const mla_string_t& message, const mla_char_t* context1);
void mla_log_error(const mla_char_t* message, const mla_char_t* context1);



/// Utils
//////////////////////////////////////////////////////

const mla_char_t* mla_log_level_to_string(const mla_log_level level);
mla_size_t mla_log_level_to_string_length(const mla_log_level level);





#endif
