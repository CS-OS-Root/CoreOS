//
// Created for service platform abstraction module.
//

#ifndef MLA_SERVICE_TEST_H
#define MLA_SERVICE_TEST_H

#if defined(MLA_SERVICE_SUPPORTED) && MLA_SERVICE_SUPPORTED == 1

#include "../../lib/base-lib/core/service/mla_service.h"
#include "../../lib/base-lib/test-support/mla_test_executor.h"
#include "../../lib/base-lib/test-support/Test/mla_test.h"

void ServiceInvalidArgumentsTest() {
    mla_int32_t res1 = mla_service_install(mla_string_empty(), mla_string_empty());
    assert_equal(res1, (mla_test_int32_t)MLA_SERVICE_ERROR_INVALID_ARGUMENT, "install with empty service name must return invalid argument");

    mla_int32_t res2 = mla_service_install(mla_string_const(""), mla_string_empty());
    assert_equal(res2, (mla_test_int32_t)MLA_SERVICE_ERROR_INVALID_ARGUMENT, "install with blank service name must return invalid argument");

    mla_int32_t res3 = mla_service_uninstall(mla_string_empty());
    assert_equal(res3, (mla_test_int32_t)MLA_SERVICE_ERROR_INVALID_ARGUMENT, "uninstall with empty service name must return invalid argument");

    mla_int32_t res4 = mla_service_uninstall(mla_string_const(""));
    assert_equal(res4, (mla_test_int32_t)MLA_SERVICE_ERROR_INVALID_ARGUMENT, "uninstall with blank service name must return invalid argument");
}

void ServiceInstallAndUninstallFlowTest() {
    mla_string_t service_name = mla_string_const("mla_test_daemon_full");
    mla_string_t service_args = mla_string_const("--daemon --port 9090");

    mla_int32_t install_res = mla_service_install(service_name, service_args);
    assert_equal(install_res, (mla_test_int32_t)MLA_SERVICE_SUCCESS, "Service install with args should return success");

    mla_int32_t uninstall_res = mla_service_uninstall(service_name);
    assert_equal(uninstall_res, (mla_test_int32_t)MLA_SERVICE_SUCCESS, "Service uninstall should return success");
}

void ServiceInstallWithoutArgsTest() {
    mla_string_t service_name = mla_string_const("mla_test_daemon_noargs");

    mla_int32_t install_res = mla_service_install(service_name);
    assert_equal(install_res, (mla_test_int32_t)MLA_SERVICE_SUCCESS, "Service install without args should return success");

    mla_int32_t uninstall_res = mla_service_uninstall(service_name);
    assert_equal(uninstall_res, (mla_test_int32_t)MLA_SERVICE_SUCCESS, "Service uninstall should return success");
}

void ServiceApiWrapperTest() {
    mla_string_t service_name = mla_string_const("mla_test_daemon_wrapper");
    mla_string_t service_args = mla_string_const("--mode worker");

    mla_int32_t install_res = mla_service_install(service_name, service_args);
    assert_equal(install_res, (mla_test_int32_t)MLA_SERVICE_SUCCESS, "mla_service_install helper should return success");

    mla_int32_t uninstall_res = mla_service_uninstall(service_name);
    assert_equal(uninstall_res, (mla_test_int32_t)MLA_SERVICE_SUCCESS, "mla_service_uninstall helper should return success");
}

void ServiceGetInstallSummaryTest() {
    mla_string_t empty_summary = mla_service_get_install_summary(mla_string_empty());
    assert_true(mla_string_is_empty(empty_summary), "Empty service name must yield empty summary");

    mla_string_t service_name = mla_string_const("mla_test_service_summary");
    mla_string_t service_args = mla_string_const("--port 8080");
    mla_string_t summary = mla_service_get_install_summary(service_name, service_args);
    assert_false(mla_string_is_empty(summary), "Summary for valid service must not be empty");
    assert_true(mla_string_contains(summary, service_name), "Summary must contain service name");
    assert_true(mla_string_contains(summary, service_args), "Summary must contain service args");
}

void RegisterServiceTests(mla_test_executor_t &p_TestExecutor) {
    mla_test_t test1 = mla_test("ServiceInvalidArguments", test_category, ServiceInvalidArgumentsTest);
    mla_test_executor_register_test(p_TestExecutor, test1);

    mla_test_t test2 = mla_test("ServiceInstallAndUninstallFlow", test_category, ServiceInstallAndUninstallFlowTest);
    mla_test_executor_register_test(p_TestExecutor, test2);

    mla_test_t test3 = mla_test("ServiceInstallWithoutArgs", test_category, ServiceInstallWithoutArgsTest);
    mla_test_executor_register_test(p_TestExecutor, test3);

    mla_test_t test4 = mla_test("ServiceApiWrapper", test_category, ServiceApiWrapperTest);
    mla_test_executor_register_test(p_TestExecutor, test4);

    mla_test_t test5 = mla_test("ServiceGetInstallSummary", test_category, ServiceGetInstallSummaryTest);
    mla_test_executor_register_test(p_TestExecutor, test5);
}

#endif // MLA_SERVICE_SUPPORTED

#endif // MLA_SERVICE_TEST_H
