//
// Created by christian on 9/13/25.
//

#ifndef MLA_CLI_APP_TEST_H
#define MLA_CLI_APP_TEST_H

#include "../../lib/base-lib/test-support/mla_test_executor.h"
#include "../../lib/base-lib/core/cli/mla_cli_app.h"
#include "../../lib/base-lib/core/system/mla_stream.h"

// Test execution flags for command testing
static mla_bool_t test_command_executed = false;
static mla_string_t test_command_output = mla_string_empty();

// Test command execution function
inline mla_bool_t test_command_execute(const mla_cli_command_t& command, const mla_hash_map_t<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)>& parameters, const mla_cli_command_execute_outstream_t& out) {
    (void)command;
    (void)parameters;
    test_command_executed = true;
    test_command_output = mla_string("Command executed successfully");
    out.write(out.userdata, test_command_output);
    return true;
}

// Variables for count-based execution
static mla_int32_t test_command_execute_count = 0;

inline mla_bool_t test_command_execute_counter(const mla_cli_command_t& command, const mla_hash_map_t<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)>& parameters, const mla_cli_command_execute_outstream_t& out) {
    (void)command;
    (void)parameters;
    test_command_execute_count++;
    out.write(out.userdata, mla_string("Command executed"));
    return true;
}

inline mla_bool_t test_command_execute_fail(const mla_cli_command_t& command, const mla_hash_map_t<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)>& parameters, const mla_cli_command_execute_outstream_t& out) {
    (void)command;
    (void)parameters;
    test_command_execute_count++;
    out.write(out.userdata, mla_string("Command failed"));
    return false;
}

inline void SimpleNavigationTest() {

    mla_cli_module_t root = mla_cli_module(mla_string_const("Root"));
    mla_cli_module_t subModule = mla_cli_module(mla_string_const("SubModule1"));
    mla_cli_module_t subSubModule = mla_cli_module(mla_string_const("SubSubModule1"));
    mla_array_list_add(subModule.subModules, subSubModule);

    mla_array_list_add(root.subModules, subModule);

    mla_cli_module_t subModule2 = mla_cli_module(mla_string_const("SubModule2"));
    mla_array_list_add(root.subModules, subModule2);

    mla_stream_output_t output = mla_stream_noop_output();
    mla_cli_app_t app = mla_cli_app_init(root, output);

    if (mla_array_list_size(app.activeModules) == 1) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 0)->moduleName, mla_string("Root"), "Active module should be Root");
    } else {
        // If the size is not 1, fail the test immediately
        assert_fail("App should start with root module active");
    }

    // Process no input
    mla_stream_input_t input = mla_stream_noop_input();
    mla_cli_app_update_and_process_input(app, input, output);

    if (mla_array_list_size(app.activeModules) == 1) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 0)->moduleName, mla_string("Root"), "Active module should be Root");
    } else {
        // If the size is not 1, fail the test immediately
        assert_fail("App should start with root module active");
    }

    mla_string_t buffer = mla_string("SubModule1\n");
    input = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(mla_string_data(buffer))), mla_string_length(buffer));
    mla_cli_app_update_and_process_input(app, input, output);

    if (mla_array_list_size(app.activeModules) == 2) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 1)->moduleName, mla_string("SubModule1"), "Active module should be SubModule1");
    } else {
        // If the size is not 2, fail the test immediately
        assert_fail("App should have navigated to SubModule1");
    }

    buffer = mla_string("SubSubModule1\n");
    input = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(mla_string_data(buffer))), mla_string_length(buffer));
    mla_cli_app_update_and_process_input(app, input, output);

    if (mla_array_list_size(app.activeModules) == 3) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 2)->moduleName, mla_string("SubSubModule1"), "Active module should be SubSubModule1");
    } else {
        // If the size is not 3, fail the test immediately
        assert_fail("App should have navigated to SubSubModule1");
    }

    buffer = mla_string("exit\n");
    input = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(mla_string_data(buffer))), mla_string_length(buffer));
    mla_stream_output_t noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);

    if (mla_array_list_size(app.activeModules) == 2) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 1)->moduleName, mla_string("SubModule1"), "Active module should be SubModule1");
    } else {
        // If the size is not 2, fail the test immediately
        assert_fail("App should have navigated back to SubModule1");
    }

    buffer = mla_string("exit\n");
    input = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(mla_string_data(buffer))), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);

    if (mla_array_list_size(app.activeModules) == 1) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 0)->moduleName, mla_string("Root"), "Active module should be Root");
    } else {
        // If the size is not 1, fail the test immediately
        assert_fail("App should have navigated back to Root");
    }

    buffer = mla_string("exit\n");
    input = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(mla_string_data(buffer))), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);

    if (mla_array_list_size(app.activeModules) == 1) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 0)->moduleName, mla_string("Root"), "Active module should be Root");
    } else {
        // If the size is not 1, fail the test immediately
        assert_fail("App should remain at Root when exiting from root");
    }

}

inline void InvalidNavigationTest() {
    mla_cli_module_t root = mla_cli_module(mla_string_const("Root"));
    mla_cli_module_t subModule = mla_cli_module(mla_string_const("ValidModule"));
    mla_array_list_add(root.subModules, subModule);

    mla_stream_output_t noopOutput = mla_stream_noop_output();
    mla_cli_app_t app = mla_cli_app_init(root, noopOutput);

    // Test invalid module navigation
    mla_string_t buffer = mla_string("InvalidModule\n");
    mla_stream_input_t input = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(mla_string_data(buffer))), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);
    assert_equal(mla_array_list_size(app.activeModules), (mla_size_t)1, "App should remain in root module on invalid navigation");

    if (mla_array_list_size(app.activeModules) == 1) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 0)->moduleName, mla_string("Root"), "Active module should still be Root");
    } else {
        // If the size is not 1, fail the test immediately
        assert_fail("App should start with root module active");
    }

    // Test empty input
    buffer = mla_string("\n");
    input = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(mla_string_data(buffer))), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);
    assert_equal(mla_array_list_size(app.activeModules), (mla_size_t)1, "App should remain in root module on empty input");

    // Test whitespace only input
    buffer = mla_string("   \n");
    input = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(mla_string_data(buffer))), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);
    assert_equal(mla_array_list_size(app.activeModules), (mla_size_t)1, "App should remain in root module on whitespace input");
}

inline void CommandExecutionTest() {
    test_command_executed = false;
    test_command_output = mla_string_empty();

    mla_cli_module_t root = mla_cli_module(mla_string_const("Root"));

    // Create a command with parameters
    mla_cli_command_t testCommand = mla_cli_command(mla_string_const("testcmd"), test_command_execute);
    mla_cli_command_add_parameter(testCommand, mla_string_const("param1"), true);
    mla_cli_command_add_parameter(testCommand, mla_string_const("param2"), false);

    mla_array_list_add(root.availableCommands, testCommand);

    mla_stream_output_t noopOutput = mla_stream_noop_output();
    mla_cli_app_t app = mla_cli_app_init(root, noopOutput);

    // Test command execution
    mla_string_t buffer = mla_string("testcmd --param1 value1\n");
    mla_stream_input_t input = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(mla_string_data(buffer))), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);

    assert_equal(test_command_executed, true, "Command should have been executed");
    assert_equal(mla_array_list_size(app.activeModules), (mla_size_t)1, "App should remain in same module after command execution");
}

inline void MultipleModuleNavigationTest() {
    mla_cli_module_t root = mla_cli_module(mla_string_const("Root"));
    mla_cli_module_t module1 = mla_cli_module(mla_string_const("Module1"));
    mla_cli_module_t module2 = mla_cli_module(mla_string_const("Module2"));
    mla_cli_module_t module3 = mla_cli_module(mla_string_const("Module3"));

    mla_array_list_add(root.subModules, module1);
    mla_array_list_add(root.subModules, module2);
    mla_array_list_add(root.subModules, module3);

    mla_stream_output_t noopOutput = mla_stream_noop_output();
    mla_cli_app_t app = mla_cli_app_init(root, noopOutput);

    // Navigate to Module2
    mla_string_t buffer = mla_string("Module2\n");
    mla_stream_input_t input = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(mla_string_data(buffer))), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);

    if (mla_array_list_size(app.activeModules) == 2) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 1)->moduleName, mla_string("Module2"), "Active module should be Module2");
    } else {
        // If the size is not 2, fail the test immediately
        assert_fail("App should have navigated to Module2");
    }



    // Navigate back to root and then to Module1
    buffer = mla_string("exit\n");
    input = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(mla_string_data(buffer))), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);

    buffer = mla_string("Module1\n");
    input = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(mla_string_data(buffer))), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);

    if (mla_array_list_size(app.activeModules) == 2) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 1)->moduleName, mla_string("Module1"), "Active module should be Module1");
    } else {
        // If the size is not 2, fail the test immediately
        assert_fail("App should have navigated to Module1");
    }

}

inline void DeepNavigationTest() {
    mla_cli_module_t root = mla_cli_module(mla_string_const("Root"));
    mla_cli_module_t level1 = mla_cli_module(mla_string_const("Level1"));
    mla_cli_module_t level2 = mla_cli_module(mla_string_const("Level2"));
    mla_cli_module_t level3 = mla_cli_module(mla_string_const("Level3"));
    mla_cli_module_t level4 = mla_cli_module(mla_string_const("Level4"));

    mla_array_list_add(level3.subModules, level4);
    mla_array_list_add(level2.subModules, level3);
    mla_array_list_add(level1.subModules, level2);
    mla_array_list_add(root.subModules, level1);

    mla_stream_output_t noopOutput = mla_stream_noop_output();
    mla_cli_app_t app = mla_cli_app_init(root, noopOutput);

    // Navigate deep into the hierarchy
    mla_string_t buffer = mla_string("Level1\n");
    mla_stream_input_t input = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(mla_string_data(buffer))), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);

    buffer = mla_string("Level2\n");
    input = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(mla_string_data(buffer))), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);

    buffer = mla_string("Level3\n");
    input = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(mla_string_data(buffer))), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);

    buffer = mla_string("Level4\n");
    input = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(mla_string_data(buffer))), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);


    if (mla_array_list_size(app.activeModules) == 5) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 0)->moduleName, mla_string("Root"), "Top module should be Root");
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 1)->moduleName, mla_string("Level1"), "Second module should be Level1");
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 2)->moduleName, mla_string("Level2"), "Third module should be Level2");
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 3)->moduleName, mla_string("Level3"), "Fourth module should be Level3");
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 4)->moduleName, mla_string("Level4"), "Deepest module should be Level4");
    } else {
        // If the size is not 5, fail the test immediately
        assert_fail("App should be at depth 5");
    }

    // Navigate back up multiple levels
    buffer = mla_string("exit\n");
    input = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(mla_string_data(buffer))), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);
    buffer = mla_string("exit\n");
    input = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(mla_string_data(buffer))), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);

    if (mla_array_list_size(app.activeModules) == 3) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 0)->moduleName, mla_string("Root"), "Top module should be Root");
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 1)->moduleName, mla_string("Level1"), "Second module should be Level1");
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 2)->moduleName, mla_string("Level2"), "Active module should be Level2");
    } else {
        // If the size is not 3, fail the test immediately
        assert_fail("App should be back at Level2");
    }

}

inline void EmptyModuleTest() {
    mla_cli_module_t root = mla_cli_module(mla_string_const("Root"));
    mla_cli_module_t emptyModule = mla_cli_module(mla_string_const("EmptyModule"));

    // EmptyModule has no submodules or commands
    mla_array_list_add(root.subModules, emptyModule);

    mla_stream_output_t noopOutput = mla_stream_noop_output();
    mla_cli_app_t app = mla_cli_app_init(root, noopOutput);

    // Navigate to empty module
    mla_string_t buffer = mla_string("EmptyModule\n");
    mla_stream_input_t input = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(mla_string_data(buffer))), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);

    if (mla_array_list_size(app.activeModules) == 2) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 1)->moduleName, mla_string("EmptyModule"), "Active module should be EmptyModule");
    } else {
        // If the size is not 2, fail the test immediately
        assert_fail("App should have navigated to EmptyModule");
    }

    // Try to navigate to non-existent submodule
    buffer = mla_string("NonExistent\n");
    input = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(mla_string_data(buffer))), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);

    if (mla_array_list_size(app.activeModules) == 2) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 1)->moduleName, mla_string("EmptyModule"), "Active module should still be EmptyModule");
    } else {
        // If the size is not 2, fail the test immediately
        assert_fail("App should remain in EmptyModule on invalid navigation");
    }

}

inline void CaseSensitivityTest() {
    mla_cli_module_t root = mla_cli_module(mla_string_const("Root"));
    mla_cli_module_t testModule = mla_cli_module(mla_string_const("TestModule"));

    mla_array_list_add(root.subModules, testModule);

    mla_stream_output_t noopOutput = mla_stream_noop_output();
    mla_cli_app_t app = mla_cli_app_init(root, noopOutput);

    // Test case sensitivity - should not navigate with wrong case
    mla_string_t buffer = mla_string("testmodule\n");
    mla_stream_input_t input = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(mla_string_data(buffer))), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);

    if (mla_array_list_size(app.activeModules) == 1) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 0)->moduleName, mla_string("Root"), "Active module should still be Root");
    } else {
        // If the size is not 1, fail the test immediately
        assert_fail("App should not navigate with wrong case");
    }

    // Test correct case - should navigate
    buffer = mla_string("TestModule\n");
    input = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(mla_string_data(buffer))), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);

    if (mla_array_list_size(app.activeModules) == 2) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 1)->moduleName, mla_string("TestModule"), "Active module should be TestModule");
    } else {
        // If the size is not 2, fail the test immediately
        assert_fail("App should navigate with correct case");
    }

}

inline void SpecialCharacterInputTest() {
    mla_cli_module_t root = mla_cli_module(mla_string_const("Root"));
    mla_cli_module_t normalModule = mla_cli_module(mla_string_const("NormalModule"));

    mla_array_list_add(root.subModules, normalModule);

    mla_stream_output_t noopOutput = mla_stream_noop_output();
    mla_cli_app_t app = mla_cli_app_init(root, noopOutput);

    // Test special characters and symbols
    mla_string_t buffer = mla_string("@#$%^&*()\n");
    mla_stream_input_t input = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(mla_string_data(buffer))), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);
    assert_equal(mla_array_list_size(app.activeModules), (mla_size_t)1, "App should ignore special character input");

    // Test numbers
    buffer = mla_string("12345\n");
    input = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(mla_string_data(buffer))), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);
    assert_equal(mla_array_list_size(app.activeModules), (mla_size_t)1, "App should ignore numeric input");

    // Test mixed alphanumeric with special chars
    buffer = mla_string("Module-123!\n");
    input = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(mla_string_data(buffer))), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);
    assert_equal(mla_array_list_size(app.activeModules), (mla_size_t)1, "App should ignore mixed special character input");
}

inline void MultipleCommandsTest() {
    test_command_executed = false;

    mla_cli_module_t root = mla_cli_module(mla_string_const("Root"));

    // Create multiple commands
    mla_cli_command_t cmd1 = mla_cli_command(mla_string_const("command1"), test_command_execute);
    cmd1.execute = test_command_execute;

    mla_cli_command_t cmd2 = mla_cli_command(mla_string_const("command2"), test_command_execute);
    cmd2.execute = test_command_execute;

    mla_cli_command_t cmd3 = mla_cli_command(mla_string_const("help"), test_command_execute);
    cmd3.execute = test_command_execute;

    mla_array_list_add(root.availableCommands, cmd1);
    mla_array_list_add(root.availableCommands, cmd2);
    mla_array_list_add(root.availableCommands, cmd3);

    mla_stream_output_t noopOutput = mla_stream_noop_output();
    mla_cli_app_t app = mla_cli_app_init(root, noopOutput);

    // Execute different commands
    mla_string_t buffer = mla_string("command1\n");
    mla_stream_input_t input = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(mla_string_data(buffer))), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);
    assert_equal(test_command_executed, true, "First command should execute");

    test_command_executed = false;
    buffer = mla_string("help\n");
    input = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(mla_string_data(buffer))), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);
    assert_equal(test_command_executed, true, "Help command should execute");

    test_command_executed = false;
    buffer = mla_string("nonexistent\n");
    input = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(mla_string_data(buffer))), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);
    assert_equal(test_command_executed, false, "Non-existent command should not execute");
}

inline void JoinCommandsTest() {
    test_command_execute_count = 0;

    mla_cli_module_t root = mla_cli_module(mla_string_const("Root"));

    // Create commands
    mla_cli_command_t cmd1 = mla_cli_command(mla_string_const("cmd1"), test_command_execute_counter);
    mla_cli_command_t cmd2 = mla_cli_command(mla_string_const("cmd2"), test_command_execute_counter);
    mla_cli_command_t cmdfail = mla_cli_command(mla_string_const("cmdfail"), test_command_execute_fail);
    mla_cli_command_t cmd3 = mla_cli_command(mla_string_const("cmd3"), test_command_execute_counter);

    mla_array_list_add(root.availableCommands, cmd1);
    mla_array_list_add(root.availableCommands, cmd2);
    mla_array_list_add(root.availableCommands, cmdfail);
    mla_array_list_add(root.availableCommands, cmd3);

    mla_stream_output_t noopOutput = mla_stream_noop_output();
    mla_cli_app_t app = mla_cli_app_init(root, noopOutput);

    // Test successful joined execution
    mla_string_t buffer = mla_string("cmd1&&cmd2\n");
    mla_stream_input_t input = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(mla_string_data(buffer))), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);
    assert_equal(test_command_execute_count, (mla_int32_t)2, "Both joined commands should execute");

    test_command_execute_count = 0;

    // Test early exit on failure
    buffer = mla_string("cmd1&&cmdfail&&cmd3\n");
    input = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(mla_string_data(buffer))), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);
    assert_equal(test_command_execute_count, (mla_int32_t)2, "Third command should not execute because second failed");
}

// Feed a raw byte sequence to the CLI line editor as a single non-blocking read.
inline void FeedCliInput(mla_cli_app_t& app, mla_stream_output_t& output, const mla_string_t& data) {
    mla_stream_input_t input = mla_stream_input_from_buffer(
        mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(mla_string_data(data))), mla_string_length(data));
    mla_cli_app_update_and_process_input(app, input, output);
}

inline void HistoryNavigationTest() {
    mla_cli_module_t root = mla_cli_module(mla_string_const("Root"));
    mla_stream_output_t output = mla_stream_noop_output();
    mla_cli_app_t app = mla_cli_app_init(root, output);

    // Enter two commands to build up history
    FeedCliInput(app, output, mla_string("foo\n"));
    FeedCliInput(app, output, mla_string("bar\n"));

    assert_equal(mla_array_list_size(app.history), (mla_size_t)2, "History should contain two entries");
    assert_equal(app.historyIndex, (mla_int32_t)-1, "History index should start on the live line");
    assert_equal(mla_string_length(app.currentLine), (mla_size_t)0, "Current line should be empty after commit");

    // Up -> most recent entry
    FeedCliInput(app, output, mla_string("\x1b[A"));
    assert_struct_equal(mla_string_t, app.currentLine, mla_string("bar"), "Up should recall the newest history entry");
    assert_equal(app.cursorPos, (mla_size_t)3, "Cursor should be at end of recalled line");
    assert_equal(app.historyIndex, (mla_int32_t)1, "History index should point at newest entry");

    // Up -> older entry
    FeedCliInput(app, output, mla_string("\x1b[A"));
    assert_struct_equal(mla_string_t, app.currentLine, mla_string("foo"), "Up again should recall the older entry");
    assert_equal(app.historyIndex, (mla_int32_t)0, "History index should point at oldest entry");

    // Up at the oldest entry -> stays put
    FeedCliInput(app, output, mla_string("\x1b[A"));
    assert_struct_equal(mla_string_t, app.currentLine, mla_string("foo"), "Up at the oldest entry should not move");

    // Down -> newer entry
    FeedCliInput(app, output, mla_string("\x1b[B"));
    assert_struct_equal(mla_string_t, app.currentLine, mla_string("bar"), "Down should move to the newer entry");

    // Down past the newest -> back to the (empty) live line
    FeedCliInput(app, output, mla_string("\x1b[B"));
    assert_equal(mla_string_length(app.currentLine), (mla_size_t)0, "Down past newest should restore the live line");
    assert_equal(app.historyIndex, (mla_int32_t)-1, "History index should be back on the live line");
}

inline void LineEditingTest() {
    mla_cli_module_t root = mla_cli_module(mla_string_const("Root"));
    mla_stream_output_t output = mla_stream_noop_output();
    mla_cli_app_t app = mla_cli_app_init(root, output);

    // Type three characters (no newline, so the line is not committed)
    FeedCliInput(app, output, mla_string("abc"));
    assert_struct_equal(mla_string_t, app.currentLine, mla_string("abc"), "Typed characters should build the line");
    assert_equal(app.cursorPos, (mla_size_t)3, "Cursor should follow inserted characters");

    // Backspace removes the character before the cursor
    FeedCliInput(app, output, mla_string("\x7f"));
    assert_struct_equal(mla_string_t, app.currentLine, mla_string("ab"), "Backspace should delete the last character");
    assert_equal(app.cursorPos, (mla_size_t)2, "Cursor should move left after backspace");

    // Move left, then insert in the middle
    FeedCliInput(app, output, mla_string("\x1b[D"));
    assert_equal(app.cursorPos, (mla_size_t)1, "Left arrow should move the cursor left");
    FeedCliInput(app, output, mla_string("X"));
    assert_struct_equal(mla_string_t, app.currentLine, mla_string("aXb"), "Insert should happen at the cursor");
    assert_equal(app.cursorPos, (mla_size_t)2, "Cursor should advance past the inserted character");

    // Home, then Delete removes the character under the cursor
    FeedCliInput(app, output, mla_string("\x1b[H"));
    assert_equal(app.cursorPos, (mla_size_t)0, "Home should move the cursor to the start");
    FeedCliInput(app, output, mla_string("\x1b[3~"));
    assert_struct_equal(mla_string_t, app.currentLine, mla_string("Xb"), "Delete should remove the character at the cursor");

    // End moves the cursor to the end of the line
    FeedCliInput(app, output, mla_string("\x1b[F"));
    assert_equal(app.cursorPos, (mla_size_t)2, "End should move the cursor to the end");
}

inline void AutocompleteTest() {
    mla_cli_module_t root = mla_cli_module(mla_string_const("Root"));
    mla_cli_module_t alpha = mla_cli_module(mla_string_const("Alpha"));
    mla_cli_module_t almond = mla_cli_module(mla_string_const("Almond"));
    mla_array_list_add(root.subModules, alpha);
    mla_array_list_add(root.subModules, almond);

    mla_stream_output_t output = mla_stream_noop_output();
    mla_cli_app_t app = mla_cli_app_init(root, output);

    // Unique prefix -> completes fully
    FeedCliInput(app, output, mla_string("Alp"));
    FeedCliInput(app, output, mla_string("\t"));
    assert_struct_equal(mla_string_t, app.currentLine, mla_string("Alpha"), "Tab should complete a unique prefix");
    assert_equal(app.cursorPos, (mla_size_t)5, "Cursor should be at end of the completed line");

    // Clear the line (Ctrl-C)
    FeedCliInput(app, output, mla_string("\x03"));
    assert_equal(mla_string_length(app.currentLine), (mla_size_t)0, "Ctrl-C should clear the current line");

    // Ambiguous prefix -> line is left unchanged (candidates are listed instead)
    FeedCliInput(app, output, mla_string("Al"));
    FeedCliInput(app, output, mla_string("\t"));
    assert_struct_equal(mla_string_t, app.currentLine, mla_string("Al"), "Tab on an ambiguous prefix should not change the line");
    assert_equal(app.cursorPos, (mla_size_t)2, "Cursor should be unchanged on an ambiguous completion");
}

inline void SplitEscapeSequenceTest() {
    mla_cli_module_t root = mla_cli_module(mla_string_const("Root"));
    mla_stream_output_t output = mla_stream_noop_output();
    mla_cli_app_t app = mla_cli_app_init(root, output);

    FeedCliInput(app, output, mla_string("foo\n"));

    // Deliver an Up-arrow escape sequence split across two non-blocking reads
    FeedCliInput(app, output, mla_string("\x1b"));
    FeedCliInput(app, output, mla_string("[A"));

    assert_struct_equal(mla_string_t, app.currentLine, mla_string("foo"), "Split escape sequence should still be recognised as Up");
    assert_equal(app.historyIndex, (mla_int32_t)0, "Split Up arrow should recall the history entry");
}

inline void WindowsArrowKeyTest() {
    mla_cli_module_t root = mla_cli_module(mla_string_const("Root"));
    mla_stream_output_t output = mla_stream_noop_output();
    mla_cli_app_t app = mla_cli_app_init(root, output);

    FeedCliInput(app, output, mla_string("foo\n"));

    // Windows conio Up arrow: 0xE0 prefix followed by scan code 0x48 ('H')
    FeedCliInput(app, output, mla_string("\xe0H"));
    assert_struct_equal(mla_string_t, app.currentLine, mla_string("foo"), "Windows Up arrow should recall the history entry");
    assert_equal(app.cursorPos, (mla_size_t)3, "Cursor should be at end of the recalled line");
}

inline void RegisterCliAppTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("SimpleNavigation", test_category, SimpleNavigationTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("InvalidNavigation", test_category, InvalidNavigationTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("CommandExecution", test_category, CommandExecutionTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("MultipleModuleNavigation", test_category, MultipleModuleNavigationTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("DeepNavigation", test_category, DeepNavigationTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("EmptyModule", test_category, EmptyModuleTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("CaseSensitivity", test_category, CaseSensitivityTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("SpecialCharacterInput", test_category, SpecialCharacterInputTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("MultipleCommands", test_category, MultipleCommandsTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("JoinCommands", test_category, JoinCommandsTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("HistoryNavigation", test_category, HistoryNavigationTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("LineEditing", test_category, LineEditingTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Autocomplete", test_category, AutocompleteTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("SplitEscapeSequence", test_category, SplitEscapeSequenceTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("WindowsArrowKey", test_category, WindowsArrowKeyTest);
    mla_test_executor_register_test(p_TestExecutor, test);

}

#endif
