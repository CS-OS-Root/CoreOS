//
// Created for auto-update package.
//

#ifndef MLA_UPDATE_CLI_MODULE_TEST_H
#define MLA_UPDATE_CLI_MODULE_TEST_H

#include "../../lib/base-lib/core/update/mla_update.h"
#include "../../lib/base-lib/core/update/mla_update_cli_module.h"
#include "../../lib/base-lib/test-support/mla_test_executor.h"

struct UpdateCliTestOutputData {
    mla_byte_t buffer[1024];
    mla_size_t position;
};

mla_user_data_id_init(mla_update_cli_test_output_user_data_id)

inline void update_cli_test_output_write(const mla_user_data_t& userdata, const mla_string_t &data) {
    mla_native_resource_t res = mla_user_data_get_native_resource(userdata, mla_update_cli_test_output_user_data_id);
    UpdateCliTestOutputData* output = mla_r_cast<UpdateCliTestOutputData*>(res.asPointer);
    if (output != nullptr) {
        mla_size_t dataLen = mla_string_length(data);
        if (output->position + dataLen < 1024) {
            mla_memcpy(output->buffer + output->position, mla_string_data(data), dataLen);
            output->position += dataLen;
            output->buffer[output->position] = '\0';
        }
    }
}

inline void update_cli_test_output_write_buffer(const mla_user_data_t& userdata, const mla_char_t* data, mla_size_t length) {
    mla_native_resource_t res = mla_user_data_get_native_resource(userdata, mla_update_cli_test_output_user_data_id);
    UpdateCliTestOutputData* output = mla_r_cast<UpdateCliTestOutputData*>(res.asPointer);
    if (output != nullptr) {
        mla_size_t dataLen = length;
        if (output->position + dataLen < 1024) {
            mla_memcpy(output->buffer + output->position, data, dataLen);
            output->position += dataLen;
            output->buffer[output->position] = '\0';
        }
    }
}

inline void update_cli_test_output_write_cstring(const mla_user_data_t& userdata, const mla_char_t* data) {
    mla_native_resource_t res = mla_user_data_get_native_resource(userdata, mla_update_cli_test_output_user_data_id);
    UpdateCliTestOutputData* output = mla_r_cast<UpdateCliTestOutputData*>(res.asPointer);
    if (output != nullptr) {
        mla_size_t dataLen = mla_strlen(data);
        if (output->position + dataLen < 1024) {
            mla_memcpy(output->buffer + output->position, data, dataLen);
            output->position += dataLen;
            output->buffer[output->position] = '\0';
        }
    }
}

inline mla_cli_command_execute_outstream_t update_cli_test_create_outstream(UpdateCliTestOutputData& p_OutputData) {
    p_OutputData.position = 0;
    p_OutputData.buffer[0] = '\0';

    mla_cli_command_execute_outstream_t outStream = {
        mla_user_data_empty(),
        update_cli_test_output_write,
        update_cli_test_output_write_buffer,
        update_cli_test_output_write_cstring,
        update_cli_test_output_write,
        update_cli_test_output_write_buffer,
        update_cli_test_output_write_cstring
    };
    mla_native_resource_t res = mla_native_resource_empty();
    res.asPointer = mla_r_cast<mla_platform_pointer_t>(&p_OutputData);
    mla_user_data_set_native_resource(outStream.userdata, mla_update_cli_test_output_user_data_id, res, nullptr);
    return outStream;
}

inline mla_bool_t mock_cli_update_get_last_version_success(const mla_update_provider_t& p_Provider, mla_string_t& p_OutVersion) {
    (void)p_Provider;
    p_OutVersion = mla_string_const("2.0.0");
    return true;
}

inline mla_bool_t mock_cli_update_get_last_version_same(const mla_update_provider_t& p_Provider, mla_string_t& p_OutVersion) {
    (void)p_Provider;
    p_OutVersion = mla_update_get_current_version();
    return true;
}

inline mla_bool_t mock_cli_update_get_last_version_fail(const mla_update_provider_t& p_Provider, mla_string_t& p_OutVersion) {
    (void)p_Provider;
    (void)p_OutVersion;
    return false;
}

inline mla_bool_t mock_cli_update_get_binary_content_success(const mla_update_provider_t& p_Provider, const mla_string_t& p_Version, mla_stream_input_t& p_OutStream) {
    (void)p_Provider;
    (void)p_Version;
    p_OutStream = mla_stream_input_from_string(mla_string_const("mock_binary_data"));
    return true;
}

inline mla_bool_t mock_cli_update_get_binary_content_fail(const mla_update_provider_t& p_Provider, const mla_string_t& p_Version, mla_stream_input_t& p_OutStream) {
    (void)p_Provider;
    (void)p_Version;
    (void)p_OutStream;
    return false;
}

inline mla_update_error_t mock_cli_update_upgrade_success(mla_stream_input_t& p_BinaryStream) {
    (void)p_BinaryStream;
    return MLA_UPDATE_SUCCESS;
}

inline void UpdateCliModuleCreateTest() {
    mla_cli_module_t module = mla_update_cli_module_create();
    assert_true(mla_string_equals(module.moduleName, mla_string_const("update")), "Module name should be 'update'");

    const mla_cli_command_t* cmdVersion = mla_cli_module_find_command(module, mla_string_const("version"));
    assert_not_null(cmdVersion, "Version command should exist");

    const mla_cli_command_t* cmdCheck = mla_cli_module_find_command(module, mla_string_const("check"));
    assert_not_null(cmdCheck, "Check command should exist");

    const mla_cli_command_t* cmdUpgrade = mla_cli_module_find_command(module, mla_string_const("upgrade"));
    assert_not_null(cmdUpgrade, "Upgrade command should exist");
}

inline void UpdateCliModuleVersionCommandTest() {
    mla_cli_module_t module = mla_update_cli_module_create();
    const mla_cli_command_t* cmdVersion = mla_cli_module_find_command(module, mla_string_const("version"));
    assert_not_null(cmdVersion, "Version command should exist");

    if (cmdVersion != nullptr) {
        mla_update_provider_t provider = { mla_user_data_empty(), mock_cli_update_get_last_version_success, mock_cli_update_get_binary_content_success };
        mla_update_set_provider(provider);

        UpdateCliTestOutputData outputData;
        mla_cli_command_execute_outstream_t outStream = update_cli_test_create_outstream(outputData);
        mla_hash_map_t<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)> parameters =
            mla_hash_map_empty<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)>();

        mla_bool_t res = cmdVersion->execute(*cmdVersion, parameters, outStream);
        assert_true(res, "Version command should succeed");

        mla_string_t output = mla_string_copy(mla_r_cast<const mla_char_t*>(outputData.buffer), outputData.position);
        assert_true(mla_string_contains(output, mla_string_const("Latest version: 2.0.0")), "Output should contain latest version 2.0.0");
    }
}

inline void UpdateCliModuleCheckCommandTest() {
    mla_cli_module_t module = mla_update_cli_module_create();
    const mla_cli_command_t* cmdCheck = mla_cli_module_find_command(module, mla_string_const("check"));
    assert_not_null(cmdCheck, "Check command should exist");

    if (cmdCheck != nullptr) {
        // Case 1: Up to date
        mla_update_provider_t providerSame = { mla_user_data_empty(), mock_cli_update_get_last_version_same, mock_cli_update_get_binary_content_success };
        mla_update_set_provider(providerSame);

        UpdateCliTestOutputData outputData1;
        mla_cli_command_execute_outstream_t outStream1 = update_cli_test_create_outstream(outputData1);
        mla_hash_map_t<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)> parameters =
            mla_hash_map_empty<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)>();

        mla_bool_t res1 = cmdCheck->execute(*cmdCheck, parameters, outStream1);
        assert_true(res1, "Check command should succeed");
        mla_string_t output1 = mla_string_copy(mla_r_cast<const mla_char_t*>(outputData1.buffer), outputData1.position);
        assert_true(mla_string_contains(output1, mla_string_const("App is up to date")), "Output should indicate app is up to date");

        // Case 2: New version available
        mla_update_provider_t providerNew = { mla_user_data_empty(), mock_cli_update_get_last_version_success, mock_cli_update_get_binary_content_success };
        mla_update_set_provider(providerNew);

        UpdateCliTestOutputData outputData2;
        mla_cli_command_execute_outstream_t outStream2 = update_cli_test_create_outstream(outputData2);

        mla_bool_t res2 = cmdCheck->execute(*cmdCheck, parameters, outStream2);
        assert_true(res2, "Check command should succeed when new version available");
        mla_string_t output2 = mla_string_copy(mla_r_cast<const mla_char_t*>(outputData2.buffer), outputData2.position);
        assert_true(mla_string_contains(output2, mla_string_const("New version available: 2.0.0")), "Output should announce new version 2.0.0");
    }
}

inline void UpdateCliModuleUpgradeImplicitVersionTest() {
    mla_cli_module_t module = mla_update_cli_module_create();
    const mla_cli_command_t* cmdUpgrade = mla_cli_module_find_command(module, mla_string_const("upgrade"));
    assert_not_null(cmdUpgrade, "Upgrade command should exist");

    if (cmdUpgrade != nullptr) {
        mla_update_provider_t provider = { mla_user_data_empty(), mock_cli_update_get_last_version_success, mock_cli_update_get_binary_content_success };
        mla_update_set_provider(provider);

        mla_update_management_t orig_mgmt = g_update_management;
        g_update_management.upgrade_to_version = mock_cli_update_upgrade_success;

        UpdateCliTestOutputData outputData;
        mla_cli_command_execute_outstream_t outStream = update_cli_test_create_outstream(outputData);
        mla_hash_map_t<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)> parameters =
            mla_hash_map_empty<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)>();

        mla_bool_t res = cmdUpgrade->execute(*cmdUpgrade, parameters, outStream);
        assert_true(res, "Upgrade command without parameters should succeed");

        mla_string_t output = mla_string_copy(mla_r_cast<const mla_char_t*>(outputData.buffer), outputData.position);
        assert_true(mla_string_contains(output, mla_string_const("Upgrading to version '2.0.0'")), "Output should state upgrading to latest version 2.0.0");

        g_update_management = orig_mgmt;
    }
}

inline void UpdateCliModuleUpgradeExplicitVersionTest() {
    mla_cli_module_t module = mla_update_cli_module_create();
    const mla_cli_command_t* cmdUpgrade = mla_cli_module_find_command(module, mla_string_const("upgrade"));
    assert_not_null(cmdUpgrade, "Upgrade command should exist");

    if (cmdUpgrade != nullptr) {
        mla_update_provider_t provider = { mla_user_data_empty(), mock_cli_update_get_last_version_success, mock_cli_update_get_binary_content_success };
        mla_update_set_provider(provider);

        mla_update_management_t orig_mgmt = g_update_management;
        g_update_management.upgrade_to_version = mock_cli_update_upgrade_success;

        UpdateCliTestOutputData outputData;
        mla_cli_command_execute_outstream_t outStream = update_cli_test_create_outstream(outputData);
        mla_hash_map_t<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)> parameters =
            mla_hash_map_empty<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)>();
        mla_hash_map_push(parameters, mla_string_const("version"), mla_string_const("1.5.0"));

        mla_bool_t res = cmdUpgrade->execute(*cmdUpgrade, parameters, outStream);
        assert_true(res, "Upgrade command with explicit version parameter should succeed");

        mla_string_t output = mla_string_copy(mla_r_cast<const mla_char_t*>(outputData.buffer), outputData.position);
        assert_true(mla_string_contains(output, mla_string_const("Upgrading to version '1.5.0'")), "Output should state upgrading to version 1.5.0");

        g_update_management = orig_mgmt;
    }
}

inline void UpdateCliModuleFailuresTest() {
    mla_cli_module_t module = mla_update_cli_module_create();
    const mla_cli_command_t* cmdVersion = mla_cli_module_find_command(module, mla_string_const("version"));
    const mla_cli_command_t* cmdCheck = mla_cli_module_find_command(module, mla_string_const("check"));
    const mla_cli_command_t* cmdUpgrade = mla_cli_module_find_command(module, mla_string_const("upgrade"));

    mla_update_provider_t providerFail = { mla_user_data_empty(), mock_cli_update_get_last_version_fail, mock_cli_update_get_binary_content_fail };
    mla_update_set_provider(providerFail);

    mla_hash_map_t<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)> parameters =
        mla_hash_map_empty<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)>();

    if (cmdVersion != nullptr) {
        UpdateCliTestOutputData outputData1;
        mla_cli_command_execute_outstream_t outStream1 = update_cli_test_create_outstream(outputData1);
        mla_bool_t res1 = cmdVersion->execute(*cmdVersion, parameters, outStream1);
        assert_false(res1, "Version command should fail when provider fails");
    }

    if (cmdCheck != nullptr) {
        UpdateCliTestOutputData outputData2;
        mla_cli_command_execute_outstream_t outStream2 = update_cli_test_create_outstream(outputData2);
        mla_bool_t res2 = cmdCheck->execute(*cmdCheck, parameters, outStream2);
        assert_false(res2, "Check command should fail when provider fails");
    }

    if (cmdUpgrade != nullptr) {
        UpdateCliTestOutputData outputData3;
        mla_cli_command_execute_outstream_t outStream3 = update_cli_test_create_outstream(outputData3);
        mla_bool_t res3 = cmdUpgrade->execute(*cmdUpgrade, parameters, outStream3);
        assert_false(res3, "Upgrade command should fail when provider fails to fetch version");
    }
}

inline void RegisterUpdateCliModuleTests(mla_test_executor_t& p_TestExecutor) {
    mla_test_t test = mla_test("UpdateCliModuleCreate", test_category, UpdateCliModuleCreateTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UpdateCliModuleVersionCommand", test_category, UpdateCliModuleVersionCommandTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UpdateCliModuleCheckCommand", test_category, UpdateCliModuleCheckCommandTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UpdateCliModuleUpgradeImplicitVersion", test_category, UpdateCliModuleUpgradeImplicitVersionTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UpdateCliModuleUpgradeExplicitVersion", test_category, UpdateCliModuleUpgradeExplicitVersionTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UpdateCliModuleFailures", test_category, UpdateCliModuleFailuresTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}

#endif // MLA_UPDATE_CLI_MODULE_TEST_H
