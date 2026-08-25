//
// Created for service platform abstraction module.
//

#ifndef MLA_GLOBAL_SERVICE_WINDOWS_H
#define MLA_GLOBAL_SERVICE_WINDOWS_H

#include "../../core/service/mla_service.h"
#include <windows.h>
#include <stdio.h>

#ifndef MLA_SERVICE_SUPPORTED
#define MLA_SERVICE_SUPPORTED 1
#endif

inline mla_int32_t mla_private_windows_service_install(const mla_string_t &p_ServiceName, const mla_string_t &p_ServiceArgs) {
    if (mla_string_is_empty(p_ServiceName)) {
        return MLA_SERVICE_ERROR_INVALID_ARGUMENT;
    }

    mla_c_string_t serviceNameCStr = mla_string_to_cString(p_ServiceName);
    const mla_char_t* serviceName = mla_c_string_data(serviceNameCStr);
    if (serviceName == nullptr || serviceName[0] == '\0') {
        return MLA_SERVICE_ERROR_INVALID_ARGUMENT;
    }

    char exe_path[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, exe_path, MAX_PATH);
    if (len == 0) {
        return MLA_SERVICE_ERROR_SYSTEM;
    }

    char cmdline[MAX_PATH * 2];
    if (!mla_string_is_empty(p_ServiceArgs)) {
        mla_c_string_t argsCStr = mla_string_to_cString(p_ServiceArgs);
        const mla_char_t* args = mla_c_string_data(argsCStr);
        snprintf(cmdline, sizeof(cmdline), "\"%s\" %s", exe_path, args != nullptr ? args : "");
    } else {
        snprintf(cmdline, sizeof(cmdline), "\"%s\"", exe_path);
    }

    SC_HANDLE scm = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scm == NULL) {
        scm = OpenSCManagerA(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    }
    if (scm == NULL) {
        DWORD err = GetLastError();
        if (err == ERROR_ACCESS_DENIED) {
            return MLA_SERVICE_ERROR_PERMISSION_DENIED;
        }
        return MLA_SERVICE_ERROR_SYSTEM;
    }

    SC_HANDLE svc = CreateServiceA(
        scm,
        serviceName,
        serviceName,
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        cmdline,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );

    if (svc == NULL) {
        DWORD err = GetLastError();
        CloseServiceHandle(scm);
        if (err == ERROR_ACCESS_DENIED) {
            return MLA_SERVICE_ERROR_PERMISSION_DENIED;
        }
        return MLA_SERVICE_ERROR_SYSTEM;
    }

    CloseServiceHandle(svc);
    CloseServiceHandle(scm);
    return MLA_SERVICE_SUCCESS;
}

inline mla_int32_t mla_private_windows_service_uninstall(const mla_string_t &p_ServiceName) {
    if (mla_string_is_empty(p_ServiceName)) {
        return MLA_SERVICE_ERROR_INVALID_ARGUMENT;
    }

    mla_c_string_t serviceNameCStr = mla_string_to_cString(p_ServiceName);
    const mla_char_t* serviceName = mla_c_string_data(serviceNameCStr);
    if (serviceName == nullptr || serviceName[0] == '\0') {
        return MLA_SERVICE_ERROR_INVALID_ARGUMENT;
    }

    SC_HANDLE scm = OpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT);
    if (scm == NULL) {
        DWORD err = GetLastError();
        if (err == ERROR_ACCESS_DENIED) {
            return MLA_SERVICE_ERROR_PERMISSION_DENIED;
        }
        return MLA_SERVICE_ERROR_SYSTEM;
    }

    SC_HANDLE svc = OpenServiceA(scm, serviceName, SERVICE_STOP | DELETE);
    if (svc == NULL) {
        DWORD err = GetLastError();
        CloseServiceHandle(scm);
        if (err == ERROR_SERVICE_DOES_NOT_EXIST) {
            return MLA_SERVICE_ERROR_NOT_FOUND;
        }
        if (err == ERROR_ACCESS_DENIED) {
            return MLA_SERVICE_ERROR_PERMISSION_DENIED;
        }
        return MLA_SERVICE_ERROR_SYSTEM;
    }

    SERVICE_STATUS status;
    ControlService(svc, SERVICE_CONTROL_STOP, &status);
    BOOL deleted = DeleteService(svc);
    DWORD err = deleted ? ERROR_SUCCESS : GetLastError();

    CloseServiceHandle(svc);
    CloseServiceHandle(scm);

    if (err == ERROR_SUCCESS) {
        return MLA_SERVICE_SUCCESS;
    }
    if (err == ERROR_ACCESS_DENIED) {
        return MLA_SERVICE_ERROR_PERMISSION_DENIED;
    }
    return MLA_SERVICE_ERROR_SYSTEM;
}

const mla_service_platform_t g_mla_service_platform = {
    mla_private_windows_service_install,
    mla_private_windows_service_uninstall
};

#endif // MLA_GLOBAL_SERVICE_WINDOWS_H
