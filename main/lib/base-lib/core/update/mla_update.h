//
// Created for auto-update package.
//

#ifndef MLA_UPDATE_H
#define MLA_UPDATE_H

#include "../mla_data_types.h"
#include "../system/mla_string.h"
#include "../system/mla_stream.h"
#include "../system/mla_user_data.h"
#include "mla_update_version.h"

/**
 * @brief Error codes returned by the auto update package operations.
 */
enum mla_update_error_t : mla_uint8_t {
    MLA_UPDATE_SUCCESS = 0,
    MLA_UPDATE_ERROR_NOT_SUPPORTED,
    MLA_UPDATE_ERROR_TEMP_FILE_FAILED,
    MLA_UPDATE_ERROR_WRITE_FAILED,
    MLA_UPDATE_ERROR_SPAWN_FAILED,
    MLA_UPDATE_ERROR_COPY_FAILED,
    MLA_UPDATE_ERROR_RESTART_FAILED,
    MLA_UPDATE_ERROR_INVALID_STREAM,
    MLA_UPDATE_ERROR_FETCH_FAILED
};

/**
 * @brief Update provider interface for retrieving remote version and binary content streams.
 */
struct mla_update_provider_t {
    mla_user_data_t user_data;
    mla_bool_t (*get_last_version)(const mla_update_provider_t& p_Provider, mla_string_t& p_OutVersion);
    mla_bool_t (*get_binary_content)(const mla_update_provider_t& p_Provider, const mla_string_t& p_Version, mla_stream_input_t& p_OutStream);
};

/**
 * @brief Platform management interface for target executable upgrade operations.
 */
struct mla_update_management_t {
    mla_update_error_t (*upgrade_to_version)(mla_stream_input_t& p_BinaryStream);
    mla_bool_t (*check_and_apply_pending_update)(int argc, char** argv);
};

/**
 * @brief Global platform management instance for auto update.
 */
extern mla_update_management_t g_update_management;

/**
 * @brief Returns the current compiled version of the application.
 *
 * @return mla_string_t containing the current version.
 */
mla_string_t mla_update_get_current_version();

/**
 * @brief Queries the remote update provider for the latest available app version.
 *
 * @param p_Provider The update provider to query.
 * @param p_OutVersion Output parameter to store the latest version string.
 * @return mla_bool_t true on success, false on failure.
 */
mla_bool_t mla_update_get_last_version(const mla_update_provider_t& p_Provider, mla_string_t& p_OutVersion);

/**
 * @brief Queries the configured global update provider for the latest available app version.
 *
 * @param p_OutVersion Output parameter to store the latest version string.
 * @return mla_bool_t true on success, false on failure.
 */
mla_bool_t mla_update_get_last_version(mla_string_t& p_OutVersion);

/**
 * @brief Configures the default global update provider.
 *
 * @param p_Provider The update provider to set as global default.
 */
void mla_update_set_provider(const mla_update_provider_t& p_Provider);

/**
 * @brief Gets the configured global update provider.
 *
 * @return mla_update_provider_t current global provider.
 */
mla_update_provider_t mla_update_get_provider();

/**
 * @brief Upgrades the application to the version provided in the input binary stream.
 *
 * Delegates execution to platform-specific code to save the binary to temporary storage,
 * start the update helper process with target executable path, and perform replacement.
 *
 * @param p_BinaryStream The input stream containing the new executable binary.
 * @return mla_update_error_t result code.
 */
mla_update_error_t mla_update_upgrade_to_version(mla_stream_input_t& p_BinaryStream);

/**
 * @brief Checks command line arguments on application startup for pending self-replacement updates.
 *
 * @param argc Argument count.
 * @param argv Argument array.
 * @return mla_bool_t true if a pending update was applied (process will restart and exit), false otherwise.
 */
mla_bool_t mla_update_check_and_apply_pending_update(int argc, char** argv);

/**
 * @brief Factory helper creating an HTTP-based update provider struct.
 *
 * @param p_BaseUrl The base URL of the HTTP update server (e.g., "http://localhost:8080").
 * @return mla_update_provider_t configured HTTP update provider.
 */
mla_update_provider_t mla_update_provider_http_create(const mla_string_t& p_BaseUrl);

#endif // MLA_UPDATE_H
