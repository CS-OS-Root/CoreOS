//
// Created by chris on 11/3/2025.
//

#ifndef MLA_ID_TEST_H
#define MLA_ID_TEST_H

#include "../../lib/base-lib/core/system/mla_id.h"
#include "../../lib/base-lib/test-support/mla_test_executor.h"
#include "../../lib/base-lib/test-support/mla_benchmark_executor.h"


inline void GenerateUuidTest() {
    mla_string_t uuid1 = mla_generate_uuid();
    mla_string_t uuid2 = mla_generate_uuid();

    // Check UUID length (standard UUID format is 36 characters with hyphens)
    assert_equal(mla_string_length(uuid1), (mla_uint32_t)36, "UUID should be 36 characters long");
    assert_equal(mla_string_length(uuid2), (mla_uint32_t)36, "UUID should be 36 characters long");

    // Check that two generated UUIDs are different
    assert_false(mla_string_equals(uuid1, uuid2), "Two generated UUIDs should be different");

    const mla_char_t* uuid1_data = mla_string_data(uuid1);
    const mla_char_t* uuid2_data = mla_string_data(uuid2);

    if (uuid1_data != nullptr && uuid2_data != nullptr) {
        // Check hyphen positions (8-4-4-4-12 format)
        assert_equal(uuid1_data[8], '-', "UUID should have hyphen at position 8");
        assert_equal(uuid1_data[13], '-', "UUID should have hyphen at position 13");
        assert_equal(uuid1_data[18], '-', "UUID should have hyphen at position 18");
        assert_equal(uuid1_data[23], '-', "UUID should have hyphen at position 23");
    } else {
        assert_fail("UUID data should not be null");
    }
}

inline void GenerateRuntimeIdTest() {
    mla_string_t id1 = mla_generate_runtime_id();
    mla_string_t id2 = mla_generate_runtime_id();

    // Check that IDs are not empty
    assert_true(mla_string_length(id1) > 0, "Runtime ID should not be empty");
    assert_true(mla_string_length(id2) > 0, "Runtime ID should not be empty");

    // Check that two generated runtime IDs are different
    assert_false(mla_string_equals(id1, id2), "Two generated runtime IDs should be different");
}

inline void RandomUInt32Test() {
    mla_uint32_t rand1 = mla_random_uint32();
    mla_uint32_t rand2 = mla_random_uint32();

    // Check that two random numbers are different (statistically should be)
    assert_not_equal(rand1, rand2, "Two random uint32 values should be different");
}

inline void UuidUniquenessTest() {
    // Generate multiple UUIDs and verify they're all unique
    const mla_size_t count = 100;
    mla_array_list_t<mla_init_struct(mla_string_t)> uuids = mla_array_list<mla_init_struct(mla_string_t)>(count);

    for (mla_size_t i = 0; i < count; i++) {
        mla_array_list_add(uuids, mla_generate_uuid());
    }

    if (mla_array_list_size(uuids) == count) {
        for (mla_size_t i = 0; i < count; i++) {
            for (mla_size_t j = i + 1; j < count; j++) {
                assert_false(mla_string_equals(mla_array_list_get_unsafe(uuids, i), mla_array_list_get_unsafe(uuids, j)), "All generated UUIDs should be unique");
            }
        }
    } else {
        assert_fail("Failed to generate the expected number of UUIDs");
    }

}

inline void RuntimeIdUniquenessTest() {
    // Generate multiple runtime IDs and verify they're all unique
    const mla_size_t count = 100;
    mla_array_list_t<mla_init_struct(mla_string_t)> uuids = mla_array_list<mla_init_struct(mla_string_t)>(count);

    for (mla_size_t i = 0; i < count; i++) {
        mla_array_list_add(uuids, mla_generate_runtime_id());
    }

    if (mla_array_list_size(uuids) == count) {
        for (mla_size_t i = 0; i < count; i++) {
            for (mla_size_t j = i + 1; j < count; j++) {
                assert_false(mla_string_equals(mla_array_list_get_unsafe(uuids, i), mla_array_list_get_unsafe(uuids, j)), "All generated UUIDs should be unique");

            }
        }
    } else {
        assert_fail("Failed to generate the expected number of Runtime IDs");
    }
}

inline void RandomUInt32DistributionTest() {
    // Test that random numbers have reasonable distribution
    const mla_size_t count = 1000;
    mla_uint32_t sum = 0;

    for (mla_size_t i = 0; i < count; i++) {
        mla_uint32_t value = mla_random_uint32();
        sum += value;
    }

    // Average should be roughly half of max uint32 value
    mla_uint32_t average = sum / count;
    assert_true(average > 0, "Average random value should be greater than 0");
}

void RegisterIdTests(mla_test_executor_t &p_TestExecutor) {
    mla_test_t test = mla_test("GenerateUuid", test_category, GenerateUuidTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("GenerateRuntimeId", test_category, GenerateRuntimeIdTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("RandomUInt32", test_category, RandomUInt32Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UuidUniqueness", test_category, UuidUniquenessTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("RuntimeIdUniqueness", test_category, RuntimeIdUniquenessTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("RandomUInt32Distribution", test_category, RandomUInt32DistributionTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}

#endif
