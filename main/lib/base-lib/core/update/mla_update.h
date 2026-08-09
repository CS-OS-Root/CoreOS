//
// Created for MLA Framework Auto Update.
//

#ifndef MLA_UPDATE_H
#define MLA_UPDATE_H

#include "../mla_data_types.h"
#include "../system/mla_string.h"
#include "../system/mla_array_list.h"
#include "../http/mla_http_client.h"
#include "../filesystem/mla_file_system.h"
#include "../external_task/mla_external_task.h"

#ifndef MLA_APP_VERSION
#define MLA_APP_VERSION "snapshot"
#endif

/**
 * @brief Configuration parameters for the MLA auto update package.
 */
struct mla_update_config_t {
    mla_string_t server_url;    ///< Base URL of the release server (e.g., "https://releases.home.schlegel.ovh")
    mla_string_t product_name;  ///< Product directory name (e.g., "mla-core")
    mla_string_t platform;      ///< Target platform identifier (e.g., "linux_x86_64" or "windows64")
    mla_string_t config_name;   ///< Target build configuration name (e.g., "gcc-x86_64")
    mla_string_t binary_name;   ///< Target executable binary name (e.g., "mla-core-app")

    static mla_update_config_t init();
};

/**
 * @brief Creates default auto update configuration.
 *
 * @return mla_update_config_t Configured default values.
 */
mla_update_config_t mla_update_config_default();

/**
 * @brief Returns the current version of the application.
 *
 * Defaults to "snapshot" unless overridden by the MLA_APP_VERSION macro.
 *
 * @return mla_string_t String containing the current application version.
 */
mla_string_t mla_update_get_current_version();

/**
 * @brief Queries the release server to determine the latest available release version.
 *
 * Uses default configuration settings.
 *
 * @param out_version Output string to receive the latest version tag (e.g., "0.0.1").
 * @return mla_bool_t true if a version was successfully retrieved, false otherwise.
 */
mla_bool_t mla_update_get_last_version(mla_string_t& out_version);

/**
 * @brief Queries the release server with custom configuration to determine the latest version.
 *
 * @param p_Config Custom configuration parameters.
 * @param out_version Output string to receive the latest version tag.
 * @return mla_bool_t true if a version was successfully retrieved, false otherwise.
 */
mla_bool_t mla_update_get_last_version(const mla_update_config_t& p_Config, mla_string_t& out_version);

/**
 * @brief Downloads the specified release version, stages it in temporary storage,
 *        and launches the self-replacement process.
 *
 * Uses default configuration settings.
 *
 * @param p_Version The version string to upgrade to. If empty or "latest", fetches the latest version first.
 * @return mla_bool_t true if the update process was initiated successfully, false otherwise.
 */
mla_bool_t mla_update_upgrade_to_version(const mla_string_t& p_Version);

/**
 * @brief Downloads the specified release version with custom configuration and launches self-replacement.
 *
 * @param p_Config Custom configuration parameters.
 * @param p_Version The target version string to upgrade to.
 * @return mla_bool_t true if the update process was initiated successfully, false otherwise.
 */
mla_bool_t mla_update_upgrade_to_version(const mla_update_config_t& p_Config, const mla_string_t& p_Version);

/**
 * @brief Checks process startup arguments for the self-replacement directive.
 *
 * If the application was invoked with --mla-update-replace-target="<target_path>",
 * this function copies its own binary over <target_path>, executes <target_path>, and exits.
 *
 * @param p_CmdLine Process command line string to check.
 * @return mla_bool_t true if replacement argument was handled (process will exit), false otherwise.
 */
mla_bool_t mla_update_check_and_apply(const mla_string_t& p_CmdLine);

/**
 * @brief Helper for checking command line arguments array.
 *
 * @param argc Argument count.
 * @param argv Argument array.
 * @return mla_bool_t true if replacement argument was handled, false otherwise.
 */
mla_bool_t mla_update_check_and_apply(mla_int32_t argc, mla_char_t** argv);

#endif // MLA_UPDATE_H
