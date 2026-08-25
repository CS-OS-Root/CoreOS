//
// Created for service platform abstraction module.
//

#include "mla_service.h"

mla_int32_t mla_service_install(const mla_char_t *p_ServiceName, const mla_char_t *p_ServiceArgs) {
    if (g_mla_service_platform.install == nullptr) {
        return MLA_SERVICE_ERROR_NOT_SUPPORTED;
    }

    if (p_ServiceName == nullptr || p_ServiceName[0] == '\0') {
        return MLA_SERVICE_ERROR_INVALID_ARGUMENT;
    }

    return g_mla_service_platform.install(p_ServiceName, p_ServiceArgs);
}

mla_int32_t mla_service_uninstall(const mla_char_t *p_ServiceName) {
    if (g_mla_service_platform.uninstall == nullptr) {
        return MLA_SERVICE_ERROR_NOT_SUPPORTED;
    }

    if (p_ServiceName == nullptr || p_ServiceName[0] == '\0') {
        return MLA_SERVICE_ERROR_INVALID_ARGUMENT;
    }

    return g_mla_service_platform.uninstall(p_ServiceName);
}
