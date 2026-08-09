//
// Created for MLA Framework Auto Update Tests.
//

#ifndef MLA_UPDATE_TEST_H
#define MLA_UPDATE_TEST_H

#include "../../lib/base-lib/test-support/mla_test_executor.h"
#include "../../lib/base-lib/core/update/mla_update.h"

static const mla_char_t update_test_category[] = "Update";

// Forward declaration of private helper declared in mla_update.cpp
mla_bool_t mla_private_update_extract_highest_version(const mla_string_t& p_Text, mla_string_t& out_version);

inline void test_mla_update_get_current_version() {
    mla_string_t ver = mla_update_get_current_version();
    assert_true(mla_string_equals(ver, mla_string_const("snapshot")), "Current version should default to snapshot");
}

inline void test_mla_update_config_default() {
    mla_update_config_t cfg = mla_update_config_default();
    assert_true(!mla_string_is_empty(cfg.server_url), "Server URL should not be empty");
    assert_true(!mla_string_is_empty(cfg.product_name), "Product name should not be empty");
    assert_true(!mla_string_is_empty(cfg.platform), "Platform should not be empty");
    assert_true(!mla_string_is_empty(cfg.config_name), "Config name should not be empty");
    assert_true(!mla_string_is_empty(cfg.binary_name), "Binary name should not be empty");
}

inline void test_mla_update_extract_highest_version() {
    mla_string_t html = mla_string_const(
        "<html><body>"
        "<a href=\"0.0.1/\">0.0.1/</a>"
        "<a href=\"0.0.2/\">0.0.2/</a>"
        "<a href=\"0.1.0/\">0.1.0/</a>"
        "<a href=\"0.0.9/\">0.0.9/</a>"
        "</body></html>"
    );

    mla_string_t parsed_ver = mla_string_empty();
    mla_bool_t ok = mla_private_update_extract_highest_version(html, parsed_ver);

    assert_true(ok, "Should extract highest version successfully");
    assert_true(mla_string_equals(parsed_ver, mla_string_const("0.1.0")), "Highest version should be 0.1.0");
}

inline void test_mla_update_check_and_apply_negative() {
    mla_string_t cmdline = mla_string_const("./app --some-other-arg=true");
    mla_bool_t handled = mla_update_check_and_apply(cmdline);
    assert_true(!handled, "Non-matching command line should return false");
}

inline void RegisterUpdateTests(mla_test_executor_t& executor) {
    mla_test_t test = mla_test("GetCurrentVersion", update_test_category, test_mla_update_get_current_version);
    mla_test_executor_register_test(executor, test);

    test = mla_test("ConfigDefault", update_test_category, test_mla_update_config_default);
    mla_test_executor_register_test(executor, test);

    test = mla_test("ExtractHighestVersion", update_test_category, test_mla_update_extract_highest_version);
    mla_test_executor_register_test(executor, test);

    test = mla_test("CheckAndApplyNegative", update_test_category, test_mla_update_check_and_apply_negative);
    mla_test_executor_register_test(executor, test);
}

#endif // MLA_UPDATE_TEST_H
