//
// Created for auto-update package.
//

#ifndef MLA_UPDATE_H
#define MLA_UPDATE_H

#include "../mla_data_types.h"
#include "../system/mla_string.h"
#include "../system/mla_stream.h"
#include "../system/mla_user_data.h"
#include "../url/mla_url.h"
#include "mla_update_version.h"

typedef mla_uint32_t mla_update_error_t;

#define MLA_UPDATE_SUCCESS 0
#define MLA_UPDATE_ERROR_INVALID_STREAM 1
#define MLA_UPDATE_ERROR_WRITE_FAILED 2
#define MLA_UPDATE_ERROR_SPAWN_FAILED 3
#define MLA_UPDATE_ERROR_NOT_SUPPORTED 4

struct mla_update_provider_t;

/**
 * @brief Provider struct for querying and fetching app updates.
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
 * @brief Queries the default global update provider for the latest available app version.
 *
 * @param p_OutVersion Output parameter to store the latest version string.
 * @return mla_bool_t true on success, false on failure.
 */
mla_bool_t mla_update_get_last_version(mla_string_t& p_OutVersion);

/**
 * @brief Sets the global update provider used by default queries.
 *
 * @param p_Provider The update provider to register as default.
 */
void mla_update_set_provider(const mla_update_provider_t& p_Provider);

/**
 * @brief Gets the current global update provider.
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
 * @brief Factory helper creating an HTTP-based update provider struct.
 *
 * @param p_BaseUrl The base URL of the HTTP update server (e.g., "http://localhost:8080").
 * @return mla_update_provider_t configured HTTP update provider.
 */
mla_update_provider_t mla_update_provider_http_create(const mla_string_t& p_BaseUrl);

#endif // MLA_UPDATE_H
