//
// Created for service platform abstraction module.
//

#ifndef MLA_SERVICE_H
#define MLA_SERVICE_H

#include "../mla_data_types.h"
#include "../system/mla_string.h"

#define MLA_SERVICE_SUCCESS 0
#define MLA_SERVICE_ERROR_INVALID_ARGUMENT 1
#define MLA_SERVICE_ERROR_NOT_SUPPORTED 2
#define MLA_SERVICE_ERROR_SYSTEM 3
#define MLA_SERVICE_ERROR_PERMISSION_DENIED 4
#define MLA_SERVICE_ERROR_NOT_FOUND 5

/**
 * @brief Platform abstraction interface for installing and uninstalling system services/daemons.
 */
typedef struct mla_service_platform_t {
    mla_int32_t (*install)(const mla_string_t &service_name, const mla_string_t &service_args);
    mla_int32_t (*uninstall)(const mla_string_t &service_name);
    mla_string_t (*get_install_summary)(const mla_string_t &service_name, const mla_string_t &service_args);
} mla_service_platform_t;

/**
 * @brief Global service platform instance configured per target platform.
 */
extern const mla_service_platform_t g_mla_service_platform;

/**
 * @brief Installs the currently executing binary as a system service.
 *
 * Automatically resolves and targets the path of the currently executing binary as the service payload,
 * appends optional startup arguments if provided, and configures the service entry in the host OS init system.
 *
 * @param p_ServiceName The unique name identifier for the service.
 * @param p_ServiceArgs Optional startup arguments to append to the daemon execution command line.
 * @return mla_int32_t 0 (MLA_SERVICE_SUCCESS) on success, or a non-zero error code.
 */
mla_int32_t mla_service_install(const mla_string_t &p_ServiceName, const mla_string_t &p_ServiceArgs = mla_string_empty());

/**
 * @brief Uninstalls and removes the specified service from the host system.
 *
 * Deregisters, removes, and cleans up the configured service entry for the application from the host system.
 *
 * @param p_ServiceName The unique name identifier of the service to remove.
 * @return mla_int32_t 0 (MLA_SERVICE_SUCCESS) on success, or a non-zero error code.
 */
mla_int32_t mla_service_uninstall(const mla_string_t &p_ServiceName);

/**
 * @brief Returns platform-specific service management instructions or installation summary.
 *
 * @param p_ServiceName The unique name identifier for the service.
 * @param p_ServiceArgs Optional startup arguments used when the service was configured.
 * @return mla_string_t Platform-specific formatted instructions, or empty string if unsupported.
 */
mla_string_t mla_service_get_install_summary(const mla_string_t &p_ServiceName, const mla_string_t &p_ServiceArgs = mla_string_empty());

#endif // MLA_SERVICE_H
