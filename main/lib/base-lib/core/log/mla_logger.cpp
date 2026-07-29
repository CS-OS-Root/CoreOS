//
// Created by christian on 9/10/25.
//

#include "mla_logger.h"

// Global Log Manager

mla_logger_manager_t g_logger_manager = {
    mla_array_list_empty<mla_init_struct(mla_logger_t)>()
};

mla_int32_t mla_log_indexOf_logger(const mla_string_t& loggerName) {

    for (mla_size_t i = 0; i < mla_array_list_size(g_logger_manager.loggers); ++i) {

        const mla_logger_t& logger = mla_array_list_get_unsafe(g_logger_manager.loggers, i);

        if (mla_string_equals(logger.name, loggerName)) {
            return mla_s_cast<mla_int32_t>(i);
        }
    }

    return -1; // Logger not found
}

mla_bool_t mla_log_register_logger(const mla_logger_t& logger) {

    mla_int32_t index = mla_log_indexOf_logger(logger.name);

    if (index >= 0) {
        return false; // Logger already registered
    }

    return mla_array_list_add(g_logger_manager.loggers, logger);
}

mla_bool_t mla_log_unregister_logger(const mla_string_t& loggerName) {

    mla_int32_t index = mla_log_indexOf_logger(loggerName);

    if (index < 0) {
        return false; // Logger not found
    }

    mla_array_list_remove(g_logger_manager.loggers, index);
    return true;
}

mla_bool_t mla_log_set_logger_level(const mla_string_t& loggerName, const mla_log_level level) {

    mla_int32_t index = mla_log_indexOf_logger(loggerName);

    if (index < 0) {
        return false; // Logger not found
    }

    mla_logger_t* logger = mla_array_list_get_ref(g_logger_manager.loggers, index);

    if (logger == nullptr) {
        return false;
    }

    logger->level = level;
    return true;
}

mla_log_level mla_log_get_logger_level(const mla_string_t& loggerName) {

    mla_int32_t index = mla_log_indexOf_logger(loggerName);

    if (index < 0) {
        return MLA_LOG_LEVEL_ERROR; // Default log level if logger not found
    }

    const mla_logger_t* logger = mla_array_list_get_ref(g_logger_manager.loggers, index);

    if (logger == nullptr) {
        return MLA_LOG_LEVEL_ERROR; // Default log level if logger not found
    }

    return logger->level;
}


mla_bool_t mla_log_is_active(const mla_log_level level) {

    for (mla_size_t i = 0; i < mla_array_list_size(g_logger_manager.loggers); ++i) {

        const mla_logger_t& logger = mla_array_list_get_unsafe(g_logger_manager.loggers, i);

        if (logger.level <= level) {
            return true;
        }
    }

    return false;
}


/// LOG MESSAGE
////////////////////////////////////////////////

void mla_log_message(const mla_log_level level, const mla_string_t& message, const mla_string_t& context1) {

    mla_string_t context1_copy = mla_string_empty();
    mla_string_t message_copy = mla_string_empty();

    mla_bool_t data_copied = false;

    for (mla_size_t i = 0; i < mla_array_list_size(g_logger_manager.loggers); ++i) {

        mla_logger_t& logger = mla_array_list_get_unsafe(g_logger_manager.loggers, i);

        if (logger.level <= level && logger.write != nullptr) {

            if (logger.need_full_managed_strings) {

                // Copy the temp strings into owned data only once
                if (!data_copied) {
                    // I am not sure if its really needed to copy here
                    message_copy = mla_string_copy(message);
                    context1_copy = mla_string_copy(context1);
                    data_copied = true;
                }

                if (mla_string_is_empty(message_copy) && mla_string_is_empty(context1_copy)) {
                    // Failed to copy strings, skip this logger
                    continue;
                }

                logger.write(level, message_copy, context1_copy, logger.userData);

            } else {
                mla_string_t context1_non_const = context1;
                mla_string_t message_non_const = message;

                logger.write(level, message_non_const, context1_non_const, logger.userData);
            }

        }
    }
}

void mla_log_message(const mla_log_level level, const mla_string_t& message, const mla_char_t* context1) {
    mla_log_message(level, message, mla_string(context1));
}

void mla_log_message(const mla_log_level level, const mla_char_t* message, const mla_char_t* context1) {
    mla_log_message(level, mla_string(message), mla_string(context1));
}

/// VERBOSE
///////////////////////////////////////////////////

void mla_log_verbose(const mla_string_t& message, const mla_string_t& context1) {
    mla_log_message(MLA_LOG_LEVEL_VERBOSE, message, context1);
}

void mla_log_verbose(const mla_string_t& message, const mla_char_t* context1) {
    mla_log_message(MLA_LOG_LEVEL_VERBOSE, message, mla_string(context1));
}

void mla_log_verbose(const mla_char_t* message, const mla_char_t* context1) {
    mla_log_message(MLA_LOG_LEVEL_VERBOSE, mla_string(message), mla_string(context1));
}

/// DEBUG
///////////////////////////////////////////////////

void mla_log_debug(const mla_string_t& message, const mla_string_t& context1) {
    mla_log_message(MLA_LOG_LEVEL_DEBUG, message, context1);
}

void mla_log_debug(const mla_string_t& message, const mla_char_t* context1) {
    mla_log_message(MLA_LOG_LEVEL_DEBUG, message, mla_string(context1));
}

void mla_log_debug(const mla_char_t* message, const mla_char_t* context1) {
    mla_log_message(MLA_LOG_LEVEL_DEBUG, mla_string(message), mla_string(context1));
}

/// INFO
///////////////////////////////////////////////////

void mla_log_info(const mla_string_t& message, const mla_string_t& context1) {
    mla_log_message(MLA_LOG_LEVEL_INFO, message, context1);
}

void mla_log_info(const mla_string_t& message, const mla_char_t* context1) {
    mla_log_message(MLA_LOG_LEVEL_INFO, message, mla_string(context1));
}

void mla_log_info(const mla_char_t* message, const mla_char_t* context1) {
    mla_log_message(MLA_LOG_LEVEL_INFO, mla_string(message), mla_string(context1));
}

/// WARNING
///////////////////////////////////////////////////

void mla_log_warning(const mla_string_t& message, const mla_string_t& context1) {
    mla_log_message(MLA_LOG_LEVEL_WARNING, message, context1);
}

void mla_log_warning(const mla_string_t& message, const mla_char_t* context1) {
    mla_log_message(MLA_LOG_LEVEL_WARNING, message, mla_string(context1));
}

void mla_log_warning(const mla_char_t* message, const mla_char_t* context1) {
    mla_log_message(MLA_LOG_LEVEL_WARNING, mla_string(message), mla_string(context1));
}

/// ERROR
///////////////////////////////////////////////////

void mla_log_error(const mla_string_t& message, const mla_string_t& context1) {
    mla_log_message(MLA_LOG_LEVEL_ERROR, message, context1);
}

void mla_log_error(const mla_string_t& message, const mla_char_t* context1) {
    mla_log_message(MLA_LOG_LEVEL_ERROR, message, mla_string(context1));
}

void mla_log_error(const mla_char_t* message, const mla_char_t* context1) {
    mla_log_message(MLA_LOG_LEVEL_ERROR, mla_string(message), mla_string(context1));
}



/// Utils
//////////////////////////////////////////////////////

const mla_char_t* mla_log_level_to_string(const mla_log_level level) {

    switch (level) {
        case MLA_LOG_LEVEL_VERBOSE:
            return "VERBOSE";
        case MLA_LOG_LEVEL_DEBUG:
            return "DEBUG";
        case MLA_LOG_LEVEL_INFO:
            return "INFO";
        case MLA_LOG_LEVEL_WARNING:
            return "WARNING";
        case MLA_LOG_LEVEL_ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

mla_size_t mla_log_level_to_string_length(const mla_log_level level) {

    switch (level) {
        case MLA_LOG_LEVEL_VERBOSE:
            return 7;
        case MLA_LOG_LEVEL_DEBUG:
            return 5;
        case MLA_LOG_LEVEL_INFO:
            return 4;
        case MLA_LOG_LEVEL_WARNING:
            return 7;
        case MLA_LOG_LEVEL_ERROR:
            return 5;
        default:
            return 7; // "UNKNOWN"
    }
}