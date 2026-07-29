#ifndef MLA_LOGGER_RPC_TEST_H
#define MLA_LOGGER_RPC_TEST_H

#include "../../lib/base-lib/core/log/mla_logger_rpc.h"
#include "../../lib/base-lib/test-support/mla_test_executor.h"

inline void LoggerRpcActivationLifecycleTest() {

    // Make sure its diabled
    mla_logger_rpc_deactivate();

    assert_true(mla_logger_rpc_activate(), "Logger RPC failed to activate");
    assert_true(mla_logger_rpc_active(), "Logger RPC should report active after activation");
    assert_true(mla_logger_rpc_deactivate(), "Logger RPC failed to deactivate");
    assert_false(mla_logger_rpc_active(), "Logger RPC should report inactive after deactivation");
}

inline void LoggerRpcSetLoglevelProcedureRegistrationTest() {

    mla_rpc_procedure_safe_t<mla_rpc_procedure_set_loglevel_signature> procedure =
        mla_rpc_procedure_safe_invalid<mla_rpc_procedure_set_loglevel_signature>();

    bool test = mla_rpc_find_procedure<mla_rpc_procedure_set_loglevel_signature>(
                mla_string_const(mla_rpc_procedure_set_loglevel_name),
                procedure
            );

    assert_true(test, "log/setLoglevel procedure was not registered");
}

inline void LoggerRpcGetLoglevelProcedureRegistrationTest() {

    mla_rpc_procedure_safe_t<void, mla_logger_rpc_log_level_t> procedure =
        mla_rpc_procedure_safe_invalid<void, mla_logger_rpc_log_level_t>();

    bool test = mla_rpc_find_procedure<void, mla_logger_rpc_log_level_t>(
                mla_string_const(mla_rpc_procedure_get_loglevel_name),
                procedure);

    assert_true(test, "log/getLoglevel procedure was not registered");
}

inline void LoggerRpcGetMessagesProcedureRegistrationTest() {

    mla_rpc_procedure_safe_t<void, mla_logger_rpc_log_messages_t> procedure =
        mla_rpc_procedure_safe_invalid<void, mla_logger_rpc_log_messages_t>();

    bool test = mla_rpc_find_procedure<void, mla_logger_rpc_log_messages_t>(
            mla_string_const(mla_rpc_procedure_log_message_name),
            procedure
        );

    assert_true(test, "log/getMessages procedure was not registered");
}

inline void LoggerRpcGetMessagesAfterLoggingTest() {
    // First activate the logger RPC
    assert_true(mla_logger_rpc_activate(), "Logger RPC failed to activate");

    // Trigger some log messages
    mla_warning("Test warning message");
    mla_error("Test error message");

    // Get the logged messages via RPC
    mla_logger_rpc_log_messages_t messages = {
        mla_array_list_empty<mla_init_struct(mla_logger_rpc_log_entry_t)>()
    };

    assert_true(
        mla_rpc_execute_procedure_void_input(
            mla_string_const(mla_rpc_procedure_log_message_name),
            &messages
        ),
        "Failed to retrieve log messages via RPC"
    );

    // Verify we got messages
    assert_true(
        mla_array_list_size(messages.entries) >= 2,
        "Expected at least 2 log messages"
    );

    // Check if our test messages are present
    bool foundWarning = false;
    bool foundError = false;

    for (mla_uint32_t i = 0; i < mla_array_list_size(messages.entries); i++) {
        mla_logger_rpc_log_entry_t& entry = mla_array_list_get_unsafe(messages.entries, i);

        if (entry.level == MLA_LOG_LEVEL_WARNING &&
            mla_string_equals_const(entry.message, "Test warning message")) {
            foundWarning = true;
        }

        if (entry.level == MLA_LOG_LEVEL_ERROR &&
            mla_string_equals_const(entry.message, "Test error message")) {
            foundError = true;
        }
    }

    assert_true(foundWarning, "Warning message not found in RPC log messages");
    assert_true(foundError, "Error message not found in RPC log messages");

    // Cleanup
    messages.entries = mla_array_list_empty<mla_init_struct(mla_logger_rpc_log_entry_t)>();
    mla_logger_rpc_deactivate();
}

inline void LoggerRpcSetAndGetLoglevelTest() {
    // Activate logger RPC
    assert_true(mla_logger_rpc_activate(), "Logger RPC failed to activate");

    // Set log level to ERROR via RPC
    mla_logger_rpc_log_level_t setLevel = {
        MLA_LOG_LEVEL_ERROR
    };

    assert_true(
        mla_rpc_execute_procedure_void_output(
            mla_string_const(mla_rpc_procedure_set_loglevel_name),
            &setLevel
        ),
        "Failed to set log level via RPC"
    );

    // Get log level via RPC
    mla_logger_rpc_log_level_t getLevel = {
        MLA_LOG_LEVEL_VERBOSE
    };

    assert_true(
        mla_rpc_execute_procedure_void_input(
            mla_string_const(mla_rpc_procedure_get_loglevel_name),
            &getLevel
        ),
        "Failed to get log level via RPC"
    );

    // Verify the log level was set correctly
    assert_equal(getLevel.level, MLA_LOG_LEVEL_ERROR, "Log level mismatch");

    // Cleanup
    mla_logger_rpc_deactivate();
}


inline void RegisterLoggerRpcTests(mla_test_executor_t &p_TestExecutor) {
    mla_test_t test = mla_test("LoggerRpcActivationLifecycle", test_category, LoggerRpcActivationLifecycleTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("LoggerRpcSetLoglevelProcedureRegistration", test_category, LoggerRpcSetLoglevelProcedureRegistrationTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("LoggerRpcGetLoglevelProcedureRegistration", test_category, LoggerRpcGetLoglevelProcedureRegistrationTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("LoggerRpcGetMessagesProcedureRegistration", test_category, LoggerRpcGetMessagesProcedureRegistrationTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("LoggerRpcGetMessagesAfterLogging", test_category, LoggerRpcGetMessagesAfterLoggingTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("LoggerRpcSetAndGetLoglevel", test_category, LoggerRpcSetAndGetLoglevelTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}


#endif
