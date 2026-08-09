//
// Created for MLA Framework Auto Update.
//

#include "mla_update.h"
#include "../system/mla_string_builder.h"
#include "../log/mla_logging.h"

#ifdef _WIN32
#include <windows.h>
#elif !defined(__wasm__) && !defined(__EMSCRIPTEN__)
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#endif

#if (defined(mla_test_disable_network) && mla_test_disable_network == 1) || \
    (defined(mla_test_disable_file_system) && mla_test_disable_file_system == 1) || \
    (defined(mla_test_disable_external_task) && mla_test_disable_external_task == 1) || \
    defined(__wasm__) || defined(__EMSCRIPTEN__) || defined(MLA_WASM_STANDALONE)
#define MLA_UPDATE_DISABLED 1
#else
#define MLA_UPDATE_DISABLED 0
#endif

mla_update_config_t mla_update_config_t::init() {
    return mla_update_config_default();
}

mla_update_config_t mla_update_config_default() {
#ifdef _WIN32
    return {
        mla_string_const("https://releases.home.schlegel.ovh"),
        mla_string_const("mla-core"),
        mla_string_const("windows64"),
        mla_string_const("msvc"),
        mla_string_const("mla-core-app.exe")
    };
#else
    return {
        mla_string_const("https://releases.home.schlegel.ovh"),
        mla_string_const("mla-core"),
        mla_string_const("linux_x86_64"),
        mla_string_const("gcc-x86_64"),
        mla_string_const("mla-core-app")
    };
#endif
}

mla_string_t mla_update_get_current_version() {
    return mla_string_const(MLA_APP_VERSION);
}

/**
 * @brief Private helper to extract the highest semantic version string from text content.
 */
mla_bool_t mla_private_update_extract_highest_version(const mla_string_t& p_Text, mla_string_t& out_version) {
    const mla_char_t* raw = mla_string_data(p_Text);
    mla_size_t len = mla_string_length(p_Text);

    if (raw == nullptr || len == 0) {
        return false;
    }

    mla_int32_t best_major = -1;
    mla_int32_t best_minor = -1;
    mla_int32_t best_patch = -1;
    mla_string_t best_version_str = mla_string_empty();

    mla_size_t i = 0;
    while (i < len) {
        // Look for digit sequence starting potential semantic version string
        if (raw[i] >= '0' && raw[i] <= '9') {
            // Check if preceded by a letter/digit that would mean it's part of another word
            if (i > 0 && ((raw[i - 1] >= 'a' && raw[i - 1] <= 'z') || (raw[i - 1] >= 'A' && raw[i - 1] <= 'Z'))) {
                i++;
                continue;
            }

            mla_size_t start = i;
            mla_int32_t major = 0;
            mla_int32_t minor = 0;
            mla_int32_t patch = 0;

            // Parse major
            while (i < len && raw[i] >= '0' && raw[i] <= '9') {
                major = (major * 10) + (raw[i] - '0');
                i++;
            }

            if (i < len && raw[i] == '.') {
                i++; // skip dot
                // Parse minor
                if (i < len && raw[i] >= '0' && raw[i] <= '9') {
                    while (i < len && raw[i] >= '0' && raw[i] <= '9') {
                        minor = (minor * 10) + (raw[i] - '0');
                        i++;
                    }

                    if (i < len && raw[i] == '.') {
                        i++; // skip dot
                        // Parse patch
                        if (i < len && raw[i] >= '0' && raw[i] <= '9') {
                            while (i < len && raw[i] >= '0' && raw[i] <= '9') {
                                patch = (patch * 10) + (raw[i] - '0');
                                i++;
                            }

                            mla_size_t ver_len = i - start;
                            mla_string_t cand = mla_string_copy(raw + start, ver_len);

                            // Compare with best version found so far
                            mla_bool_t is_higher = false;
                            if (major > best_major) {
                                is_higher = true;
                            } else if (major == best_major && minor > best_minor) {
                                is_higher = true;
                            } else if (major == best_major && minor == best_minor && patch > best_patch) {
                                is_higher = true;
                            }

                            if (is_higher) {
                                best_major = major;
                                best_minor = minor;
                                best_patch = patch;
                                best_version_str = cand;
                            }
                            continue;
                        }
                    }
                }
            }
        }
        i++;
    }

    if (best_major >= 0) {
        out_version = best_version_str;
        return true;
    }

    return false;
}

mla_bool_t mla_update_get_last_version(mla_string_t& out_version) {
    mla_update_config_t cfg = mla_update_config_default();
    return mla_update_get_last_version(cfg, out_version);
}

mla_bool_t mla_update_get_last_version(const mla_update_config_t& p_Config, mla_string_t& out_version) {
#if MLA_UPDATE_DISABLED == 1
    (void)p_Config;
    (void)out_version;
    return false;
#else
    mla_string_builder_t sb = mla_string_builder_create();
    mla_string_builder_append(sb, p_Config.server_url);
    mla_string_builder_append(sb, mla_string_const("/"));
    mla_string_builder_append(sb, p_Config.product_name);
    mla_string_builder_append(sb, mla_string_const("/?raw=true"));

    mla_string_t url = mla_string_builder_to_string(sb);
    mla_http_request_t req = mla_http_get_request(url);
    mla_http_client_response_t res = mla_http_client_send_request(req);

    if (res.status != MLA_HTTP_CLIENT_RESPONSE_STATUS_OK || res.response.statusCode != mla_http_status_ok) {
        mla_warning("Failed to fetch releases index from server");
        return false;
    }

    mla_stream_input_t body_stream = res.response.content;
    mla_string_t body_content = mla_string_from_stream(body_stream, mla_size_max);

    return mla_private_update_extract_highest_version(body_content, out_version);
#endif
}

/**
 * @brief Private helper to resolve the OS absolute path of the running executable.
 */
mla_string_t mla_private_update_get_current_executable_path() {
    mla_char_t path_buffer[1024];
    mla_memset(path_buffer, 0, sizeof(path_buffer));

#ifdef _WIN32
    DWORD len = GetModuleFileNameA(NULL, path_buffer, sizeof(path_buffer) - 1);
    if (len > 0) {
        return mla_string_copy(path_buffer, mla_s_cast<mla_size_t>(len));
    }
#elif !defined(__wasm__) && !defined(__EMSCRIPTEN__)
    ssize_t len = readlink("/proc/self/exe", path_buffer, sizeof(path_buffer) - 1);
    if (len > 0) {
        return mla_string_copy(path_buffer, mla_s_cast<mla_size_t>(len));
    }
#endif

    return mla_string_empty();
}

mla_bool_t mla_update_upgrade_to_version(const mla_string_t& p_Version) {
    mla_update_config_t cfg = mla_update_config_default();
    return mla_update_upgrade_to_version(cfg, p_Version);
}

mla_bool_t mla_update_upgrade_to_version(const mla_update_config_t& p_Config, const mla_string_t& p_Version) {
#if MLA_UPDATE_DISABLED == 1
    (void)p_Config;
    (void)p_Version;
    return false;
#else
    mla_string_t target_version = p_Version;

    if (mla_string_is_empty(target_version) || mla_string_equals(target_version, mla_string_const("latest"))) {
        if (!mla_update_get_last_version(p_Config, target_version)) {
            mla_error("Could not resolve latest version for upgrade");
            return false;
        }
    }

    // Construct binary download URL: ${server_url}/${product_name}/${version}/${platform}/${config_name}/${binary_name}
    mla_string_builder_t sb = mla_string_builder_create();
    mla_string_builder_append(sb, p_Config.server_url);
    mla_string_builder_append(sb, mla_string_const("/"));
    mla_string_builder_append(sb, p_Config.product_name);
    mla_string_builder_append(sb, mla_string_const("/"));
    mla_string_builder_append(sb, target_version);
    mla_string_builder_append(sb, mla_string_const("/"));
    mla_string_builder_append(sb, p_Config.platform);
    mla_string_builder_append(sb, mla_string_const("/"));
    mla_string_builder_append(sb, p_Config.config_name);
    mla_string_builder_append(sb, mla_string_const("/"));
    mla_string_builder_append(sb, p_Config.binary_name);

    mla_string_t binary_url = mla_string_builder_to_string(sb);

    mla_http_request_t req = mla_http_get_request(binary_url);
    mla_http_client_response_t res = mla_http_client_send_request(req);

    if (res.status != MLA_HTTP_CLIENT_RESPONSE_STATUS_OK || res.response.statusCode != mla_http_status_ok) {
        mla_error("Failed to download release binary from server");
        return false;
    }

    // Temporary path to store binary before self-replacement
#ifdef _WIN32
    mla_string_t temp_binary_path = mla_string_const("C:\\Windows\\Temp\\mla_update_tmp.exe");
#else
    mla_string_t temp_binary_path = mla_string_const("/tmp/mla_update_tmp");
#endif

    mla_stream_input_t body_stream = res.response.content;
    if (!mla_fs_copy_stream_to_file(body_stream, temp_binary_path)) {
        mla_error("Failed to save downloaded release binary to temporary path");
        return false;
    }

#if !defined(_WIN32) && !defined(__wasm__) && !defined(__EMSCRIPTEN__)
    const mla_char_t* raw_temp_path = mla_string_data(temp_binary_path);
    if (raw_temp_path != nullptr) {
        chmod(raw_temp_path, 0755);
    }
#endif

    mla_string_t current_exe = mla_private_update_get_current_executable_path();
    if (mla_string_is_empty(current_exe)) {
        mla_error("Failed to resolve current executable path");
        return false;
    }

    // Launch temp binary with argument specifying current binary to replace
    mla_string_builder_t cmd_sb = mla_string_builder_create();
    mla_string_builder_append(cmd_sb, temp_binary_path);
    mla_string_builder_append(cmd_sb, mla_string_const(" --mla-update-replace-target=\""));
    mla_string_builder_append(cmd_sb, current_exe);
    mla_string_builder_append(cmd_sb, mla_string_const("\""));

    mla_string_t launch_cmd = mla_string_builder_to_string(cmd_sb);
    mla_external_task_t task = mla_external_task_create(launch_cmd);

    if (mla_external_task_get_state(task) == MLA_EXTERNAL_TASK_STATE_STOPPED) {
        mla_error("Failed to execute staged update process");
        return false;
    }

    mla_info("Staged update binary executed successfully. Exiting current process.");
    return true;
#endif
}

mla_bool_t mla_update_check_and_apply(const mla_string_t& p_CmdLine) {
#if MLA_UPDATE_DISABLED == 1
    (void)p_CmdLine;
    return false;
#else
    const mla_char_t* flag = "--mla-update-replace-target=\"";
    const mla_char_t* raw_cmd = mla_string_data(p_CmdLine);

    if (raw_cmd == nullptr) {
        return false;
    }

    const mla_char_t* found = mla_strstr(raw_cmd, flag);
    if (found == nullptr) {
        // Try without quotes
        flag = "--mla-update-replace-target=";
        found = mla_strstr(raw_cmd, flag);
        if (found == nullptr) {
            return false;
        }
    }

    const mla_char_t* target_start = found + mla_strlen(flag);
    mla_size_t target_len = 0;
    while (target_start[target_len] != '\0' && target_start[target_len] != '"' && target_start[target_len] != ' ') {
        target_len++;
    }

    if (target_len == 0) {
        return false;
    }

    mla_string_t target_path = mla_string_copy(target_start, target_len);
    mla_string_t self_path = mla_private_update_get_current_executable_path();

    if (mla_string_is_empty(self_path)) {
        return false;
    }

    // Brief pause to ensure caller process handle releases target file
    g_low_level_access.sleep(200);

    // Overwrite old binary with self
    if (!mla_fs_copy_file_to(self_path, target_path)) {
        mla_error("Self-replacement failed to overwrite target executable");
        return false;
    }

#if !defined(_WIN32) && !defined(__wasm__) && !defined(__EMSCRIPTEN__)
    const mla_char_t* raw_target = mla_string_data(target_path);
    if (raw_target != nullptr) {
        chmod(raw_target, 0755);
    }
#endif

    // Restart updated target binary
    mla_external_task_t restarted_task = mla_external_task_create(target_path);

    if (mla_external_task_get_state(restarted_task) != MLA_EXTERNAL_TASK_STATE_STOPPED) {
        mla_info("Updated binary restarted successfully. Exiting updater process.");
#if !defined(_WIN32) && !defined(__wasm__) && !defined(__EMSCRIPTEN__)
        exit(0);
#endif
    }

    return false;
#endif
}

mla_bool_t mla_update_check_and_apply(mla_int32_t argc, mla_char_t** argv) {
    if (argc <= 1 || argv == nullptr) {
        return false;
    }

    mla_string_builder_t sb = mla_string_builder_create();
    for (mla_int32_t i = 1; i < argc; ++i) {
        if (argv[i] != nullptr) {
            mla_string_builder_append(sb, mla_string(argv[i]));
            mla_string_builder_append(sb, mla_string_const(" "));
        }
    }

    mla_string_t cmdline = mla_string_builder_to_string(sb);
    return mla_update_check_and_apply(cmdline);
}
