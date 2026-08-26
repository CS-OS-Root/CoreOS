//
// Created for service platform abstraction module.
//

#include "mla_service.h"

mla_int32_t mla_service_install(const mla_string_t &p_ServiceName, const mla_string_t &p_ServiceArgs) {
    if (g_mla_service_platform.install == nullptr) {
        return MLA_SERVICE_ERROR_NOT_SUPPORTED;
    }

    if (mla_string_is_empty(p_ServiceName)) {
        return MLA_SERVICE_ERROR_INVALID_ARGUMENT;
    }

    return g_mla_service_platform.install(p_ServiceName, p_ServiceArgs);
}

mla_int32_t mla_service_uninstall(const mla_string_t &p_ServiceName) {
    if (g_mla_service_platform.uninstall == nullptr) {
        return MLA_SERVICE_ERROR_NOT_SUPPORTED;
    }

    if (mla_string_is_empty(p_ServiceName)) {
        return MLA_SERVICE_ERROR_INVALID_ARGUMENT;
    }

    return g_mla_service_platform.uninstall(p_ServiceName);
}

mla_string_t mla_service_get_install_summary(const mla_string_t &p_ServiceName, const mla_string_t &p_ServiceArgs) {
    if (g_mla_service_platform.get_install_summary == nullptr) {
        return mla_string_empty();
    }

    if (mla_string_is_empty(p_ServiceName)) {
        return mla_string_empty();
    }

    return g_mla_service_platform.get_install_summary(p_ServiceName, p_ServiceArgs);
}

