//
// Created for auto-update package.
//

#include "mla_update_cli_module.h"
#include "mla_update.h"
#include "../cli/mla_cli_command.h"
#include "../system/mla_string_builder.h"

mla_bool_t mla_private_update_cli_version(
    const mla_cli_command_t &p_Command,
    const mla_hash_map_t<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)> &p_Parameters,
    const mla_cli_command_execute_outstream_t &p_Out) {

    (void)p_Command;
    (void)p_Parameters;

    mla_string_t current_version = mla_update_get_current_version();
    p_Out.writeCString(p_Out.userdata, "Version: ");
    p_Out.write(p_Out.userdata, current_version);
    p_Out.writeCString(p_Out.userdata, "\n");
    return true;
}

mla_bool_t mla_private_update_cli_check(
    const mla_cli_command_t &p_Command,
    const mla_hash_map_t<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)> &p_Parameters,
    const mla_cli_command_execute_outstream_t &p_Out) {

    (void)p_Command;
    (void)p_Parameters;

    mla_string_t current_version = mla_update_get_current_version();
    mla_string_t latest_version = mla_string_empty();

    if (!mla_update_get_last_version(latest_version)) {
        p_Out.writeCString(p_Out.userdata, "Error: Failed to check for new version.\n");
        return false;
    }

    if (mla_string_equals(current_version, latest_version)) {
        p_Out.writeCString(p_Out.userdata, "App is up to date (version ");
        p_Out.write(p_Out.userdata, current_version);
        p_Out.writeCString(p_Out.userdata, ").\n");
    } else {
        p_Out.writeCString(p_Out.userdata, "New version available: ");
        p_Out.write(p_Out.userdata, latest_version);
        p_Out.writeCString(p_Out.userdata, " (current version: ");
        p_Out.write(p_Out.userdata, current_version);
        p_Out.writeCString(p_Out.userdata, ").\n");
    }

    return true;
}

mla_bool_t mla_private_update_cli_upgrade(
    const mla_cli_command_t &p_Command,
    const mla_hash_map_t<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)> &p_Parameters,
    const mla_cli_command_execute_outstream_t &p_Out) {

    (void)p_Command;

    mla_string_t target_version = mla_string_empty();
    mla_string_t version_param = mla_string_const("version");

    if (!mla_hash_map_get(p_Parameters, version_param, target_version) || mla_string_length(target_version) == 0) {
        if (!mla_update_get_last_version(target_version)) {
            p_Out.writeCString(p_Out.userdata, "Error: Failed to fetch latest version for upgrade.\n");
            return false;
        }
    }

    mla_update_provider_t provider = mla_update_get_provider();
    if (provider.get_binary_content == nullptr) {
        p_Out.writeCString(p_Out.userdata, "Error: No update provider registered.\n");
        return false;
    }

    mla_stream_input_t binary_stream = mla_stream_noop_input();
    if (!provider.get_binary_content(provider, target_version, binary_stream)) {
        p_Out.writeCString(p_Out.userdata, "Error: Failed to download binary for version '");
        p_Out.write(p_Out.userdata, target_version);
        p_Out.writeCString(p_Out.userdata, "'.\n");
        return false;
    }

    mla_update_error_t err = mla_update_upgrade_to_version(binary_stream);
    if (err != MLA_UPDATE_SUCCESS) {
        p_Out.writeCString(p_Out.userdata, "Error: Upgrade failed with error code ");
        mla_string_builder_t sb = mla_string_builder_create();
        mla_string_builder_append(sb, err);
        mla_string_t err_str = mla_string_builder_to_string(sb);
        p_Out.write(p_Out.userdata, err_str);
        p_Out.writeCString(p_Out.userdata, ".\n");
        return false;
    }

    p_Out.writeCString(p_Out.userdata, "Upgrading to version '");
    p_Out.write(p_Out.userdata, target_version);
    p_Out.writeCString(p_Out.userdata, "'...\n");
    return true;
}

mla_cli_module_t mla_update_cli_module_create() {
    mla_cli_module_t module = mla_cli_module(
        mla_string_const("update"),
        mla_string_const("Application update management commands")
    );

    // "version" command
    mla_cli_command_t cmdVersion = mla_cli_command(
        mla_string_const("version"),
        mla_string_const("Show the current application version"),
        mla_private_update_cli_version
    );
    mla_cli_module_add_command(module, cmdVersion);

    // "check" command
    mla_cli_command_t cmdCheck = mla_cli_command(
        mla_string_const("check"),
        mla_string_const("Check if a new version is available"),
        mla_private_update_cli_check
    );
    mla_cli_module_add_command(module, cmdCheck);

    // "upgrade" command
    mla_cli_command_t cmdUpgrade = mla_cli_command(
        mla_string_const("upgrade"),
        mla_string_const("Upgrade application to target version or latest"),
        mla_private_update_cli_upgrade
    );
    mla_cli_command_add_parameter(
        cmdUpgrade,
        mla_string_const("version"),
        mla_string_const("Target version to upgrade to (optional)"),
        false
    );
    mla_cli_module_add_command(module, cmdUpgrade);

    return module;
}
