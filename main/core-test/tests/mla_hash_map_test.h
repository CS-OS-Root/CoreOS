//
// Created by christian on 8/4/25.
//

#ifndef MLA_HASH_MAP_TEST_H
#define MLA_HASH_MAP_TEST_H


#include "../../lib/base-lib/core/hash/mla_hash.h"
#include "../../lib/base-lib/core/system/mla_hash_map.h"
#include "../../lib/base-lib/core/system/mla_string_concat.h"
#include "../../lib/base-lib/test-support/mla_test_executor.h"
#include "../../lib/base-lib/test-support/mla_benchmark_executor.h"
#include "mla_list_contains_const.h"

struct my_hash_map_test_struct {
    mla_int32_t test1;
    mla_int32_t test2;

};

/**
 * You should never build a struc with const. The const are never copied.
 * This here is only an ugly trick for testing
 */
struct my_hash_map_with_const_test_struct {
    const mla_int32_t test1 = 0;
    mla_int32_t test2 = 0;

    my_hash_map_with_const_test_struct(mla_int32_t t1, mla_int32_t t2) : test1(t1), test2(t2) {}
    my_hash_map_with_const_test_struct() = default;
    my_hash_map_with_const_test_struct& operator=(const my_hash_map_with_const_test_struct &other) {

        if (this != &other) {
            mla_int32_t* nonConstTest1 = mla_c_cast<mla_int32_t*>(&test1);
            *nonConstTest1 = other.test1; // test1 is const, so we cannot assign it
            this->test2 = other.test2; // test1 is const, so we cannot assign it
        }
        return *this;
    }

    my_hash_map_with_const_test_struct(const my_hash_map_with_const_test_struct& other) {
        // Either implement copying logic here or just call your assignment operator
        *this = other;
    }


};


void HashMapContainsTest() {

    mla_hash_map_t<mla_init_data(mla_int32_t), mla_int32_hash_t, mla_init_data(mla_int32_t)> map = mla_hash_map<mla_init_data(mla_int32_t), mla_int32_hash_t, mla_init_data(mla_int32_t)>(10);
    mla_hash_map_push(map, mla_s_cast<mla_int32_t>(1), mla_s_cast<mla_int32_t>(100));
    mla_hash_map_push(map, mla_s_cast<mla_int32_t>(2), mla_s_cast<mla_int32_t>(200));
    mla_hash_map_push(map, mla_s_cast<mla_int32_t>(3), mla_s_cast<mla_int32_t>(300));
    mla_hash_map_push(map, mla_s_cast<mla_int32_t>(4), mla_s_cast<mla_int32_t>(400));
    mla_hash_map_push(map, mla_s_cast<mla_int32_t>(5), mla_s_cast<mla_int32_t>(500));

    assert_true(mla_hash_map_contains(map, (mla_int32_t)1), "HashMap should contain key 1");
    assert_true(mla_hash_map_contains(map, (mla_int32_t)2), "HashMap should contain key 2");
    assert_true(mla_hash_map_contains(map, (mla_int32_t)3), "HashMap should contain key 3");
    assert_true(mla_hash_map_contains(map, (mla_int32_t)4), "HashMap should contain key 4");
    assert_true(mla_hash_map_contains(map, (mla_int32_t)5), "HashMap should contain key 5");
    assert_false(mla_hash_map_contains(map, (mla_int32_t)6), "HashMap should not contain key 6");

}

void HashMapGetTest() {

    mla_hash_map_t<mla_init_data(mla_int32_t), mla_int32_hash_t, mla_init_data(mla_int32_t)> map = mla_hash_map<mla_init_data(mla_int32_t), mla_int32_hash_t, mla_init_data(mla_int32_t)>(10);
    mla_hash_map_push(map, mla_s_cast<mla_int32_t>(1), mla_s_cast<mla_int32_t>(100));
    mla_hash_map_push(map, mla_s_cast<mla_int32_t>(2), mla_s_cast<mla_int32_t>(200));
    mla_hash_map_push(map, mla_s_cast<mla_int32_t>(3), mla_s_cast<mla_int32_t>(300));

    // Try Get
    mla_int32_t value;
    assert_true(mla_hash_map_get(map, (mla_int32_t)2, value), "HashMap should successfully retrieve value for key 2");
    assert_equal(value, 200L, "HashMap should return value 200 for key 2");
    assert_false(mla_hash_map_get(map, (mla_int32_t)6, value), "HashMap should not find key 6");
    assert_equal(value, 0L, "Value should be default value for non-existing key 6");

}

void HashMapAddMuchItemsTest() {

    mla_hash_map_t<mla_init_data(mla_int32_t), mla_int32_hash_t, mla_init_data(mla_int32_t)> map = mla_hash_map<mla_init_data(mla_int32_t), mla_int32_hash_t, mla_init_data(mla_int32_t)>(16);

    for (mla_int32_t i = 0; i < 1000; ++i) {
        mla_hash_map_push(map, i, i + 1000);
    }

    for (mla_int32_t i = 0; i < 1000; ++i) {

        assert_true(mla_hash_map_contains(map, i), "HashMap should contain key");

    }

}

void HashMapRemoveTest() {

    mla_hash_map_t<mla_init_data(mla_int32_t), mla_int32_hash_t, mla_init_data(mla_int32_t)> map = mla_hash_map<mla_init_data(mla_int32_t), mla_int32_hash_t, mla_init_data(mla_int32_t)>(10);
    mla_hash_map_push(map, mla_s_cast<mla_int32_t>(1), mla_s_cast<mla_int32_t>(100));
    mla_hash_map_push(map, mla_s_cast<mla_int32_t>(2), mla_s_cast<mla_int32_t>(200));
    mla_hash_map_push(map, mla_s_cast<mla_int32_t>(3), mla_s_cast<mla_int32_t>(300));

    assert_true(mla_hash_map_remove(map, (mla_int32_t)2), "HashMap should successfully remove key 2");
    assert_false(mla_hash_map_contains(map, (mla_int32_t)2), "HashMap should not contain key 2 after removal");

    assert_false(mla_hash_map_remove(map, (mla_int32_t)4), "HashMap should not remove non-existing key 4");

}

void HashMapClearTest() {

    mla_hash_map_t<mla_init_data(mla_int32_t), mla_int32_hash_t, mla_init_data(mla_int32_t)> map = mla_hash_map<mla_init_data(mla_int32_t), mla_int32_hash_t, mla_init_data(mla_int32_t)>(10);
    mla_hash_map_push(map, mla_s_cast<mla_int32_t>(1), mla_s_cast<mla_int32_t>(100));
    mla_hash_map_push(map, mla_s_cast<mla_int32_t>(2), mla_s_cast<mla_int32_t>(200));
    mla_hash_map_push(map, mla_s_cast<mla_int32_t>(3), mla_s_cast<mla_int32_t>(300));

    mla_hash_map_clear(map);

    assert_equal(mla_hash_map_size(map), (mla_size_t)0L, "HashMap should be empty after clear");

}

void HashMapGetKeysTest() {

    mla_hash_map_t<mla_init_data(mla_int32_t), mla_int32_hash_t, mla_init_data(mla_int32_t)> map = mla_hash_map<mla_init_data(mla_int32_t), mla_int32_hash_t, mla_init_data(mla_int32_t)>(10);
    mla_hash_map_push(map, mla_s_cast<mla_int32_t>(1), mla_s_cast<mla_int32_t>(100));
    mla_hash_map_push(map, mla_s_cast<mla_int32_t>(2), mla_s_cast<mla_int32_t>(200));
    mla_hash_map_push(map, mla_s_cast<mla_int32_t>(3), mla_s_cast<mla_int32_t>(300));

    mla_array_list_t<mla_int32_t> keys = mla_hash_map_keys(map);

    assert_equal(mla_array_list_size(keys), (mla_size_t)3L, "HashMap should have 3 keys");
    assert_true(mla_array_list_contains(keys, (mla_int32_t)1), "Keys should contain 1");
    assert_true(mla_array_list_contains(keys, (mla_int32_t)2), "Keys should contain 2");
    assert_true(mla_array_list_contains(keys, (mla_int32_t)3), "Keys should contain 3");
    assert_false(mla_array_list_contains(keys, (mla_int32_t)4), "Keys should not contain 4");

}


void HashMapContainsMlaStringTest() {

    mla_hash_map_t<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)> map = mla_hash_map<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)>(10);

    mla_hash_map_push(map,  mla_string("key1"), mla_string("value1"));
    mla_hash_map_push(map,  mla_string("key2"), mla_string("value2"));
    mla_hash_map_push(map,  mla_string("key3"), mla_string("value3"));

    assert_true(mla_hash_map_contains(map, mla_string("key1")), "HashMap should contain key1");
    assert_true(mla_hash_map_contains(map, mla_string("key2")), "HashMap should contain key2");

    assert_false(mla_hash_map_contains(map, mla_string("key4")), "HashMap should not contain key4");
}


void HashMapWithValueStructTest() {

    mla_hash_map_t<mla_init_data(mla_int32_t), mla_int32_hash_t, mla_init_data(my_hash_map_test_struct)> map = mla_hash_map<mla_init_data(mla_int32_t), mla_int32_hash_t, mla_init_data(my_hash_map_test_struct)>(10);

    my_hash_map_test_struct item1 = {1, 100};
    my_hash_map_test_struct item2 = {2, 200};
    my_hash_map_test_struct item3 = {3, 300};

    mla_hash_map_push(map, mla_s_cast<mla_int32_t>(1), item1);
    mla_hash_map_push(map, mla_s_cast<mla_int32_t>(2), item2);
    mla_hash_map_push(map, mla_s_cast<mla_int32_t>(3), item3);

    assert_true(mla_hash_map_contains(map, (mla_int32_t)1), "HashMap should contain key 1");
    assert_true(mla_hash_map_contains(map, (mla_int32_t)2), "HashMap should contain key 2");
    assert_true(mla_hash_map_contains(map, (mla_int32_t)3), "HashMap should contain key 3");

    my_hash_map_test_struct value;
    assert_true(mla_hash_map_get(map, (mla_int32_t)2, value), "HashMap should successfully retrieve value for key 2");
    assert_equal(value.test1, 2L, "Value test1 should be equal to 2");
    assert_equal(value.test2, 200L, "Value test2 should be equal to 200");

    my_hash_map_test_struct* valueRef = mla_hash_map_get_ref(map, mla_s_cast<mla_int32_t>(2));

    if (valueRef != nullptr) {
        valueRef->test2 = 0; // Reset value for next check
    } else {
        assert_fail("ValueRef should not be null for key 2");
    }

    assert_true(mla_hash_map_get(map, (mla_int32_t)2, value), "HashMap should successfully retrieve value for key 2");
    assert_equal(value.test1, 2L, "Value test1 should be equal to 2 after retrieval");
    assert_equal(value.test2, 0L, "Value test2 should be equal to 0 after retrieval");

    assert_null(mla_hash_map_get_ref(map, (mla_int32_t)4), "HashMap should return null for non-existing key 4");
}


void HashMapWithValueConstStructTest() {

    mla_hash_map_t<mla_init_data(mla_int32_t), mla_int32_hash_t, mla_init_data(my_hash_map_with_const_test_struct)> map = mla_hash_map<mla_init_data(mla_int32_t), mla_int32_hash_t, mla_init_data(my_hash_map_with_const_test_struct)>(10);

    my_hash_map_with_const_test_struct item1 = {1, 100};
    my_hash_map_with_const_test_struct item2 = {2, 200};
    my_hash_map_with_const_test_struct item3 = {3, 300};

    mla_hash_map_push(map, mla_s_cast<mla_int32_t>(1), item1);
    mla_hash_map_push(map, mla_s_cast<mla_int32_t>(2), item2);
    mla_hash_map_push(map, mla_s_cast<mla_int32_t>(3), item3);

    assert_true(mla_hash_map_contains(map, (mla_int32_t)1), "HashMap should contain key 1");
    assert_true(mla_hash_map_contains(map, (mla_int32_t)2), "HashMap should contain key 2");
    assert_true(mla_hash_map_contains(map, (mla_int32_t)3), "HashMap should contain key 3");

    my_hash_map_with_const_test_struct value;
    assert_true(mla_hash_map_get(map, (mla_int32_t)2, value), "HashMap should successfully retrieve value for key 2");
    assert_equal(value.test1, 2L, "Value test1 should be equal to 2");
    assert_equal(value.test2, 200L, "Value test2 should be equal to 200");

    my_hash_map_with_const_test_struct* valueRef = mla_hash_map_get_ref(map, mla_s_cast<mla_int32_t>(2));

    if (valueRef != nullptr) {
        valueRef->test2 = 0; // Reset value for next check
    } else {
        assert_fail("ValueRef should not be null for key 2");
    }

    assert_true(mla_hash_map_get(map, (mla_int32_t)2, value), "HashMap should successfully retrieve value for key 2");
    assert_equal(value.test1, 2L, "Value test1 should be equal to 2 after retrieval");
    assert_equal(value.test2, 0L, "Value test2 should be equal to 0 after retrieval");

    assert_null(mla_hash_map_get_ref(map, (mla_int32_t)4), "HashMap should return null for non-existing key 4");
}

void HashMapItemMemoryManagementTest() {

    // Make sure that the string has its own memory management
    mla_string_t mla_str1 = mla_string_concat(mla_string("Hel"), mla_string("lo"));
    mla_string_change_memory_layout(mla_str1, MLA_STRING_MEMORY_LAYOUT_BUFFER);
    mla_string_t mla_str2 = mla_string_concat(mla_string("Wor"), mla_string("ld"));
    mla_string_change_memory_layout(mla_str2, MLA_STRING_MEMORY_LAYOUT_BUFFER);

    if (!mla_pointer_is_null(mla_str1.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str1.data_storage), (mla_int32_t)1, "String 1 should have refCount of 1");
    } else {
        assert_fail("String 1 dataOwner buffer is null");
    }

    if (!mla_pointer_is_null(mla_str2.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str2.data_storage), (mla_int32_t)1, "String 2 should have refCount of 1");
    } else {
        assert_fail("String 2 dataOwner buffer is null");
    }


    mla_hash_map_t<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)> mla_map = mla_hash_map<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)>();
    mla_hash_map_push(mla_map, mla_str1, mla_str2);
    mla_hash_map_push(mla_map, mla_str2, mla_str1);

    if (!mla_pointer_is_null(mla_str1.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str1.data_storage), (mla_int32_t)3, "String 1 should have refCount of 2 after adding to list");
    } else {
        assert_fail("String 1 dataOwner buffer is null after adding to list");
    }

    if (!mla_pointer_is_null(mla_str2.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str2.data_storage), (mla_int32_t)3, "String 2 should have refCount of 2 after adding to list");
    } else {
        assert_fail("String 2 dataOwner buffer is null after adding to list");
    }

    mla_hash_map_remove(mla_map, mla_str1);

    if (!mla_pointer_is_null(mla_str1.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str1.data_storage), (mla_int32_t)2, "String 1 should have refCount of 1 after removal from list");
    } else {
        assert_fail("String 1 dataOwner buffer is null after removal from list");
    }

    if (!mla_pointer_is_null(mla_str2.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str2.data_storage), (mla_int32_t)2, "String 2 should still have refCount of 2 after removal of String 1");
    } else {
        assert_fail("String 2 dataOwner buffer is null after removal of String 1");
    }

    mla_hash_map_clear(mla_map);

    if (!mla_pointer_is_null(mla_str1.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str1.data_storage), (mla_int32_t)1, "String 1 should have refCount of 1 after clearing list");
    } else {
        assert_fail("String 1 dataOwner buffer is null after clearing list");
    }

    if (!mla_pointer_is_null(mla_str2.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str2.data_storage), (mla_int32_t)1, "String 2 should have refCount of 1 after clearing list");
    } else {
        assert_fail("String 2 dataOwner buffer is null after clearing list");
    }

}

void HashMapItemMemoryManagementDestroyTest() {

    // Make sure that the string has its own memory management
    mla_string_t mla_str1 = mla_string_concat(mla_string("Hel"), mla_string("lo"));
    mla_string_change_memory_layout(mla_str1, MLA_STRING_MEMORY_LAYOUT_BUFFER);
    mla_string_t mla_str2 = mla_string_concat(mla_string("Wor"), mla_string("ld"));
    mla_string_change_memory_layout(mla_str2, MLA_STRING_MEMORY_LAYOUT_BUFFER);

    if (!mla_pointer_is_null(mla_str1.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str1.data_storage), (mla_int32_t)1, "String 1 should have refCount of 1");
    } else {
        assert_fail("String 1 dataOwner buffer is null");
    }

    if (!mla_pointer_is_null(mla_str2.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str2.data_storage), (mla_int32_t)1, "String 2 should have refCount of 1");
    } else {
        assert_fail("String 2 dataOwner buffer is null");
    }

    {
        mla_hash_map_t<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)> mla_map = mla_hash_map<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)>();
        mla_hash_map_push(mla_map, mla_str1, mla_str2);
        mla_hash_map_push(mla_map, mla_str2, mla_str1);

        if (!mla_pointer_is_null(mla_str1.data_storage)) {
            assert_equal(mla_pointer_ref_count(mla_str1.data_storage), (mla_int32_t)3, "String 1 should have refCount of 2 after adding to list");
        } else {
            assert_fail("String 1 dataOwner buffer is null after adding to list");
        }

        if (!mla_pointer_is_null(mla_str2.data_storage)) {
            assert_equal(mla_pointer_ref_count(mla_str2.data_storage), (mla_int32_t)3, "String 2 should have refCount of 2 after adding to list");
        } else {
            assert_fail("String 2 dataOwner buffer is null after adding to list");
        }

    }

    // After the array list is destroyed, the strings should be destroyed as well
    if (!mla_pointer_is_null(mla_str1.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str1.data_storage), (mla_int32_t)1, "String 1 should have refCount of 1 after array list destruction");
    } else {
        assert_fail("String 1 dataOwner buffer is null after array list destruction");
    }

    if (!mla_pointer_is_null(mla_str2.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str2.data_storage), (mla_int32_t)1, "String 2 should have refCount of 1 after array list destruction");
    } else {
        assert_fail("String 2 dataOwner buffer is null after array list destruction");
    }

}


void HashMapItemMemoryManagementDestroy2Test() {

    // Make sure that the string has its own memory management
    mla_string_t mla_str1 = mla_string_concat(mla_string("Hel"), mla_string("lo"));
    mla_string_change_memory_layout(mla_str1, MLA_STRING_MEMORY_LAYOUT_BUFFER);
    mla_string_t mla_str2 = mla_string_concat(mla_string("Wor"), mla_string("ld"));
    mla_string_change_memory_layout(mla_str2, MLA_STRING_MEMORY_LAYOUT_BUFFER);

    if (!mla_pointer_is_null(mla_str1.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str1.data_storage), (mla_int32_t)1, "String 1 should have refCount of 1");
    } else {
        assert_fail("String 1 dataOwner buffer is null");
    }

    if (!mla_pointer_is_null(mla_str2.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str2.data_storage), (mla_int32_t)1, "String 2 should have refCount of 1");
    } else {
        assert_fail("String 2 dataOwner buffer is null");
    }

    {

        mla_hash_map_t<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)> other = mla_hash_map<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)>();

        {
            mla_hash_map_t<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)> mla_map = mla_hash_map<mla_init_struct(mla_string_t), mla_string_hash_t, mla_init_struct(mla_string_t)>();
            mla_hash_map_push(mla_map, mla_str1, mla_str2);
            mla_hash_map_push(mla_map, mla_str2, mla_str1);

            if (!mla_pointer_is_null(mla_str1.data_storage)) {
                assert_equal(mla_pointer_ref_count(mla_str1.data_storage), (mla_int32_t)3, "String 1 should have refCount of 3 after assignment to other list");
            } else {
                assert_fail("String 1 dataOwner buffer is null after adding to list");
            }

            if (!mla_pointer_is_null(mla_str2.data_storage)) {
                assert_equal(mla_pointer_ref_count(mla_str2.data_storage), (mla_int32_t)3, "String 2 should have refCount of 3 after assignment to other list");
            } else {
                assert_fail("String 2 dataOwner buffer is null after adding to list");
            }

            assert_equal(mla_pointer_ref_count(mla_map.buckets.items), (mla_int32_t)1, "Map buckets should have refCount of 1 after creation");

            other = mla_map;

            assert_equal(mla_pointer_ref_count(mla_map.buckets.items), (mla_int32_t)2, "Map buckets should have refCount of 2 after assignment to other list");

            // Ref COunter should not change because we are assigning the map to another variable
            // The strings are still owned by the internal array
            if (!mla_pointer_is_null(mla_str1.data_storage)) {
                assert_equal(mla_pointer_ref_count(mla_str1.data_storage), (mla_int32_t)3, "String 1 should have refCount of 3 after assignment to other list");
            } else {
                assert_fail("String 1 dataOwner buffer is null after adding to list");
            }

            if (!mla_pointer_is_null(mla_str2.data_storage)) {
                assert_equal(mla_pointer_ref_count(mla_str2.data_storage), (mla_int32_t)3, "String 2 should have refCount of 3 after assignment to other list");
            } else {
                assert_fail("String 2 dataOwner buffer is null after adding to list");
            }

        }

        assert_equal(mla_pointer_ref_count(other.buckets.items), (mla_int32_t)1, "refcount should be 1 after map destruction");

        assert_equal(mla_hash_map_size(other), (mla_size_t)2, "Other list should have size 2 after assignment");

        if (!mla_pointer_is_null(mla_str1.data_storage)) {
            assert_equal(mla_pointer_ref_count(mla_str1.data_storage), (mla_int32_t)3, "String 1 should have refCount of 3 after assignment to other list");
        } else {
            assert_fail("String 1 dataOwner buffer is null after adding to list");
        }

        if (!mla_pointer_is_null(mla_str2.data_storage)) {
            assert_equal(mla_pointer_ref_count(mla_str2.data_storage), (mla_int32_t)3, "String 2 should have refCount of 3 after assignment to other list");
        } else {
            assert_fail("String 2 dataOwner buffer is null after adding to list");
        }

    }

    // After the array list is destroyed, the strings should be destroyed as well
    if (!mla_pointer_is_null(mla_str1.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str1.data_storage), (mla_int32_t)1, "String 1 should have refCount of 1 after array list destruction");
    } else {
        assert_fail("String 1 dataOwner buffer is null after array list destruction");
    }

    if (!mla_pointer_is_null(mla_str2.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str2.data_storage), (mla_int32_t)1, "String 2 should have refCount of 1 after array list destruction");
    } else {
        assert_fail("String 2 dataOwner buffer is null after array list destruction");
    }

}


void RegisterHashMapTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("Contains", test_category, HashMapContainsTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Get", test_category, HashMapGetTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("AddMuchItems", test_category, HashMapAddMuchItemsTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Remove", test_category, HashMapRemoveTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Clear", test_category, HashMapClearTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("GetKeys", test_category, HashMapGetKeysTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ContainsMlaString", test_category, HashMapContainsMlaStringTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("WithValueStruct", test_category, HashMapWithValueStructTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("WithValueConstStruct", test_category, HashMapWithValueConstStructTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ItemMemoryManagement", test_category, HashMapItemMemoryManagementTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("HashMapItemMemoryManagementDestroy", test_category, HashMapItemMemoryManagementDestroyTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("HashMapItemMemoryManagementDestroy2", test_category, HashMapItemMemoryManagementDestroy2Test);
    mla_test_executor_register_test(p_TestExecutor, test);
}

static mla_hash_map_t<mla_init_data(short), mla_int16_hash_t, mla_init_data(short)> mla_map = mla_hash_map_empty<mla_init_data(short), mla_int16_hash_t, mla_init_data(short)>();

void SetupHashMapContainsBenchmark() {

    mla_map = mla_hash_map<mla_init_data(short), mla_int16_hash_t, mla_init_data(short)>(16);

    for (short i = 0; i < CONST_LIST_CONTAINS_COUNT; ++i) {
        mla_hash_map_push(mla_map, i, i);
    }
}

void HashMapContainsBenchmark() {


    for (short i = 0; i < CONST_LIST_CONTAINS_COUNT; ++i) {

        mla_bool_t found = mla_hash_map_contains(mla_map, i);

        if (!found) {
            // Fail
            static_assert(true, "Element not found in array");
        }
    }
}

void TearDownHashMapContainsBenchmark() {

    mla_map = mla_hash_map_empty<mla_init_data(short), mla_int16_hash_t, mla_init_data(short)>();
}


void HashMapAddMuchItemsBenchmark() {

    mla_hash_map_t<mla_init_data(short), mla_int16_hash_t, mla_init_data(short)> local_map = mla_hash_map<mla_init_data(short), mla_int16_hash_t, mla_init_data(short)>(16);

    for (short i = 0; i < 1000; ++i) {
        mla_hash_map_push(local_map, i, i);
    }
}


void RegisterHashMapBenchmarks(mla_benchmark_executor_t &p_BenchmarkExecutor) {

    mla_benchmark_t benchmark = mla_benchmark("Contains", benchmark_category, HashMapContainsBenchmark, SetupHashMapContainsBenchmark, TearDownHashMapContainsBenchmark);
    mla_benchmark_set_iteration_division(benchmark, 100);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("AddMuchItems", benchmark_category, HashMapAddMuchItemsBenchmark);
    mla_benchmark_set_iteration_division(benchmark, 100);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

}


#endif
