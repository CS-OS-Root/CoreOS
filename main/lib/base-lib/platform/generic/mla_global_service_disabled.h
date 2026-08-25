//
// Created for service platform abstraction module.
//

#ifndef MLA_GLOBAL_SERVICE_DISABLED_H
#define MLA_GLOBAL_SERVICE_DISABLED_H

#include "../../core/service/mla_service.h"

inline mla_int32_t mla_private_disabled_service_install(const mla_string_t &p_ServiceName, const mla_string_t &p_ServiceArgs) {
    (void)p_ServiceName;
    (void)p_ServiceArgs;
    return MLA_SERVICE_ERROR_NOT_SUPPORTED;
}

inline mla_int32_t mla_private_disabled_service_uninstall(const mla_string_t &p_ServiceName) {
    (void)p_ServiceName;
    return MLA_SERVICE_ERROR_NOT_SUPPORTED;
}

const mla_service_platform_t g_mla_service_platform = {
    mla_private_disabled_service_install,
    mla_private_disabled_service_uninstall
};

#endif // MLA_GLOBAL_SERVICE_DISABLED_H
