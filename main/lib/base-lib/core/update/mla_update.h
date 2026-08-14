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
 * @brief Returns the current platform identifier string (e.g., "linux_x86_64", "windows64", "wasm").
 *
 * @return mla_string_t current platform string.
 */
mla_string_t mla_update_get_current_platform();

/**
 * @brief Returns the current compiler identifier string (e.g., "gcc", "clang", "filc", "zig_native", "emscripten_std", "emscripten_js", "zig_wasm", "msvc").
 *
 * @return mla_string_t current compiler string.
 */
mla_string_t mla_update_get_current_compiler();

/**
 * @brief Factory helper creating an HTTP-based update provider struct matching release server endpoints.
 *
 * @param p_Module The module/product name (e.g., "mla-core"). Required.
 * @param p_BaseUrl The base URL of the HTTP release server (defaults to "https://releases.home.schlegel.ovh").
 * @param p_Platform Target platform identifier (defaults to current platform via mla_update_get_current_platform()).
 * @param p_Compiler Target compiler identifier (defaults to current compiler via mla_update_get_current_compiler()).
 * @return mla_update_provider_t configured HTTP update provider.
 */
mla_int32_t mla_private_update_compare_versions(const mla_string_t& p_V1, const mla_string_t& p_V2);
mla_string_t mla_private_update_extract_latest_version(const mla_string_t& p_Content);

mla_update_provider_t mla_update_provider_http_create(
    const mla_string_t& p_Module,
    const mla_string_t& p_BaseUrl = mla_string_const("https://releases.home.schlegel.ovh"),
    const mla_string_t& p_Platform = mla_update_get_current_platform(),
    const mla_string_t& p_Compiler = mla_update_get_current_compiler()
);

#endif // MLA_UPDATE_H
