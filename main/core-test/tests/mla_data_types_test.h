//
// Created by chris on 9/27/2025.
//

#ifndef MLA_DATA_TYPES_TEST_H
#define MLA_DATA_TYPES_TEST_H

#include "../../lib/base-lib/core/mla_data_types.h"
#include "../../lib/base-lib/test-support/mla_test_executor.h"

inline void SizeOfDataTypesTest() {

    assert_equal((mla_test_int32_t)sizeof(mla_int8_t), (mla_test_int32_t)1, "Size of mla_int8_t should be 1 byte");
    assert_equal((mla_test_int32_t)sizeof(mla_uint8_t), (mla_test_int32_t)1, "Size of mla_uint8_t should be 1 byte");
    assert_equal((mla_test_int32_t)sizeof(mla_int16_t), (mla_test_int32_t)2, "Size of mla_int16_t should be 2 bytes");
    assert_equal((mla_test_int32_t)sizeof(mla_uint16_t), (mla_test_int32_t)2, "Size of mla_uint16_t should be 2 bytes");
    assert_equal((mla_test_int32_t)sizeof(mla_int32_t), (mla_test_int32_t)4, "Size of mla_int32_t should be 4 bytes");
    assert_equal((mla_test_int32_t)sizeof(mla_uint32_t), (mla_test_int32_t)4, "Size of mla_uint32_t should be 4 bytes");
    assert_equal((mla_test_int32_t)sizeof(mla_int64_t), (mla_test_int32_t)8, "Size of mla_int64_t should be 8 bytes");
    assert_equal((mla_test_int32_t)sizeof(mla_uint64_t), (mla_test_int32_t)8, "Size of mla_uint64_t should be 8 bytes");

    assert_equal((mla_test_int32_t)sizeof(mla_float_t), (mla_test_int32_t)4, "Size of mla_float_t should be 4 bytes");
    assert_equal((mla_test_int32_t)sizeof(mla_double_t), (mla_test_int32_t)8, "Size of mla_double_t should be 8 bytes");

    assert_equal((mla_test_int32_t)sizeof(mla_bool_t), (mla_test_int32_t)1, "Size of mla_bool_t should be 1 byte");
    assert_equal((mla_test_int32_t)sizeof(mla_byte_t), (mla_test_int32_t)1, "Size of mla_bool_t should be 1 byte");

    assert_equal((mla_test_int32_t)sizeof(mla_char_t), (mla_test_int32_t)1, "Size of mla_bool_t should be 1 byte");
    assert_equal((mla_test_int32_t)sizeof(mla_utf_16_char_t), (mla_test_int32_t)2, "Size of mla_utf_16_char_t should be 2 bytes");
    assert_equal((mla_test_int32_t)sizeof(mla_utf_32_char_t), (mla_test_int32_t)4, "Size of mla_utf_32_char_t should be 4 bytes");
}

inline void StackTraceTest() {
    if (g_low_level_access.get_stack_trace == nullptr) {
        return;
    }

    mla_char_t buffer[2048] = {0};
    mla_size_t len = g_low_level_access.get_stack_trace(buffer, sizeof(buffer));
    if (len == 0) {
        return; // Stack traces are disabled in this build
    }

    assert_true(len > 0, "Stack trace returned length should be greater than 0");
    assert_true(buffer[0] != '\0', "Stack trace buffer should not be empty");
}

inline void StackTraceNullOrZeroBufferTest() {
    if (g_low_level_access.get_stack_trace == nullptr) {
        return;
    }

    mla_char_t buffer[256] = {0};
    assert_equal(g_low_level_access.get_stack_trace(nullptr, sizeof(buffer)), (mla_size_t)0, "Null buffer should return 0");
    assert_equal(g_low_level_access.get_stack_trace(buffer, 0), (mla_size_t)0, "Zero buffer size should return 0");
}

void RegisterDataTypesTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("SizeOfDataTypes", test_category, SizeOfDataTypesTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    mla_test_t test_st = mla_test("StackTrace", test_category, StackTraceTest);
    mla_test_executor_register_test(p_TestExecutor, test_st);

    mla_test_t test_st_null = mla_test("StackTraceNullOrZeroBuffer", test_category, StackTraceNullOrZeroBufferTest);
    mla_test_executor_register_test(p_TestExecutor, test_st_null);

}

#endif