//
// Created for auto-update package.
//

#ifndef MLA_GLOBAL_UPDATE_DISABLED_H
#define MLA_GLOBAL_UPDATE_DISABLED_H

#include "../../core/update/mla_update.h"

inline mla_update_error_t mla_private_update_disabled_upgrade_to_version(mla_stream_input_t& p_BinaryStream) {
    (void)p_BinaryStream;
    return MLA_UPDATE_ERROR_NOT_SUPPORTED;
}

inline mla_bool_t mla_private_update_disabled_check_and_apply_pending_update(int argc, char** argv) {
    (void)argc;
    (void)argv;
    return false;
}

#if !defined(UNIX) && !defined(WIN32)
mla_update_management_t g_update_management = {
    mla_private_update_disabled_upgrade_to_version,
    mla_private_update_disabled_check_and_apply_pending_update
};
#endif

#endif // MLA_GLOBAL_UPDATE_DISABLED_H
