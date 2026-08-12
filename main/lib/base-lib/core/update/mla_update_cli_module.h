//
// Created for auto-update package.
//

#ifndef MLA_UPDATE_CLI_MODULE_H
#define MLA_UPDATE_CLI_MODULE_H

#include "../cli/mla_cli_app.h"

/**
 * @brief Creates the CLI module for auto-update functionality.
 *
 * Provides commands for update management:
 * - "version": Show the latest version available from the update provider
 * - "check": Check if a new version is available compared to the current app version
 * - "upgrade": Upgrade the application to a specified version or the latest version
 *
 * @return mla_cli_module_t initialized update CLI module.
 */
mla_cli_module_t mla_update_cli_module_create();

#endif // MLA_UPDATE_CLI_MODULE_H
