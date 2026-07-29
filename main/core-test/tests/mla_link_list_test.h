//
// Created by chris on 8/3/2025.
//

#ifndef MLA_LINK_LIST_TEST_H
#define MLA_LINK_LIST_TEST_H

#include "../../lib/base-lib/core/system/mla_string.h"
#include "../../lib/base-lib/core/system/mla_link_list.h"
#include "../../lib/base-lib/core/system/mla_string_concat.h"
#include "../../lib/base-lib/test-support/mla_test_executor.h"
#include "../../lib/base-lib/test-support/mla_benchmark_executor.h"
#include "mla_list_contains_const.h"

struct my_link_list_test_struct {
    mla_int32_t test1;
    mla_int32_t test2;

    mla_bool_t operator==(const my_link_list_test_struct &other) const {
        return (this->test1 == other.test1 && this->test2 == other.test2);
    }
};

/**
 * You should never build a struc with const. The const are never copied.
 * This here is only an ugly trick for testing
 */
struct my_link_list_with_const_test_struct {
    const mla_int32_t test1 = 0;
    mla_int32_t test2 = 0;

    my_link_list_with_const_test_struct(mla_int32_t t1, mla_int32_t t2) : test1(t1), test2(t2) {}
    my_link_list_with_const_test_struct() = default;
    my_link_list_with_const_test_struct& operator=(const my_link_list_with_const_test_struct &other) {

        if (this != &other) {
            // Very hack trick to assign a const member variable
            mla_int32_t* nonConstTest1 = mla_c_cast<mla_int32_t*>(&test1);
            *nonConstTest1 = other.test1; // test1 is const, so we cannot assign it
            this->test2 = other.test2; // test1 is const, so we cannot assign it
        }
        return *this;
    }

    mla_bool_t operator==(const my_link_list_with_const_test_struct &other) const {
        return (this->test1 == other.test1 && this->test2 == other.test2);
    }

    my_link_list_with_const_test_struct(const my_link_list_with_const_test_struct& other) {
        // Either implement copying logic here or just call your assignment operator
        *this = other;
    }
};


void LinkListContainsMlaStringTest() {

    mla_link_list_t<mla_init_struct(mla_string_t)> mla_list = mla_link_list<mla_init_struct(mla_string_t)>();

    mla_link_list_add(mla_list, mla_string("Hello"));
    mla_link_list_add(mla_list, mla_string("World"));
    mla_link_list_add(mla_list, mla_string("!"));

    mla_bool_t found = mla_link_list_contains<mla_string_t>(mla_list, mla_string("World"));
    assert_true(found, "List should contain 'World'");

    found = mla_link_list_contains<mla_string_t>(mla_list, mla_string("NotInList"));
    assert_false(found, "List should not contain 'NotInList'");


}

void LinkListContainsTest() {

    mla_link_list_t<mla_test_int16_t> mla_list = mla_link_list<mla_test_int16_t>();
    mla_link_list_add(mla_list, mla_s_cast<mla_test_int16_t>(1));
    mla_link_list_add(mla_list, mla_s_cast<mla_test_int16_t>(2));
    mla_link_list_add(mla_list, mla_s_cast<mla_test_int16_t>(3));

    mla_bool_t found = mla_link_list_contains(mla_list, mla_s_cast<mla_test_int16_t>(2));
    assert_true(found, "List should contain '2'");
    found = mla_link_list_contains(mla_list, mla_s_cast<mla_test_int16_t>(4));
    assert_false(found, "List should not contain '4'");

    mla_test_int16_t value;
    assert_true(mla_link_list_get(mla_list, 0, value), "Should get first item");
    assert_equal(value, (mla_test_int16_t)1, "First item should be '1'");
    assert_true(mla_link_list_get(mla_list, 1, value), "Should get second item");
    assert_equal(value, (mla_test_int16_t)2, "Second item should be '2'");

    assert_false(mla_link_list_get(mla_list, 3, value), "Should not get item at index 3 (out of bounds)");
    assert_equal(value, (mla_test_int16_t)0, "Value should be unchanged when getting out of bounds");

}

void LinkListRemoveTest() {

    mla_link_list_t<int> mla_list = mla_link_list<int>();
    mla_link_list_add(mla_list, 1);
    mla_link_list_add(mla_list, 2);
    mla_link_list_add(mla_list, 3);
    assert_equal(mla_link_list_size(mla_list), (mla_size_t)3, "List size should be 3 after adding 3 items");

    mla_bool_t removed = mla_link_list_remove<int>(mla_list, 1);
    assert_true(removed, "List should remove '2'");
    assert_false(mla_link_list_contains<int>(mla_list, 2), "List should not contain '2' after removal");
    assert_true(mla_link_list_contains<int>(mla_list, 1), "List should still contain '1'");
    assert_true(mla_link_list_contains<int>(mla_list, 3), "List should still contain '3'");
    assert_equal(mla_link_list_size(mla_list), (mla_size_t)2, "List size should be 2 after removal");

}

void LinkListClearTest() {

    mla_link_list_t<int> mla_list = mla_link_list<int>();
    mla_link_list_add(mla_list, 1);
    mla_link_list_add(mla_list, 2);
    mla_link_list_add(mla_list, 3);

    mla_link_list_clear(mla_list);
    assert_equal(mla_link_list_size(mla_list), (mla_size_t)0, "List size should be 0 after clearing");
    assert_false(mla_link_list_contains<int>(mla_list, 1), "List should not contain '1' after clearing");
    assert_false(mla_link_list_contains<int>(mla_list, 2), "List should not contain '2' after clearing");
    assert_false(mla_link_list_contains<int>(mla_list, 3), "List should not contain '3' after clearing");
}


void LinkListIndexOfTest() {

    mla_link_list_t<int> mla_list = mla_link_list<int>();
    mla_link_list_add(mla_list, 1);
    mla_link_list_add(mla_list, 2);
    mla_link_list_add(mla_list, 3);

    mla_int32_t index = mla_link_list_index_of<int>(mla_list, 2);
    assert_equal(index, (mla_int32_t)1, "Index of '2' should be 1");
    index = mla_link_list_index_of<int>(mla_list, 4);
    assert_equal(index, (mla_int32_t)-1, "Index of '4' should be -1 (not found)");
}

void LinkListAddMuchItemsTest() {

    mla_link_list_t<int> mla_list = mla_link_list<int>();
    for (int i = 0; i < 1000; ++i) {
        mla_link_list_add(mla_list, i);
    }

    assert_equal(mla_link_list_size(mla_list), (mla_size_t)1000, "List size should be 1000 after adding 1000 items");

    for (int i = 0; i < 1000; ++i) {
        assert_true(mla_link_list_contains<int>(mla_list, i), "List should contain item");
    }
}

void LinkListItemMemoryManagementTest() {

    // Make sure that the string has its own memory management
    mla_string_t mla_str1 = mla_string_concat(mla_string("Hel"), mla_string("lo"));
    mla_string_change_memory_layout(mla_str1, MLA_STRING_MEMORY_LAYOUT_BUFFER);
    mla_string_t mla_str2 = mla_string_concat(mla_string("Wor"), mla_string("ld"));
    mla_string_change_memory_layout(mla_str2, MLA_STRING_MEMORY_LAYOUT_BUFFER);

    if (!mla_pointer_is_null(mla_str1.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str1.data_storage), (mla_int32_t)1, "String 1 should have refCount of 1");
    } else {
        assert_fail("String 1 dataOwner buffer should not be null");
    }

    if (!mla_pointer_is_null(mla_str2.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str2.data_storage), (mla_int32_t)1, "String 2 should have refCount of 1");
    } else {
        assert_fail("String 2 dataOwner buffer should not be null");
    }


    mla_link_list_t<mla_init_struct(mla_string_t)> mla_arr = mla_link_list<mla_init_struct(mla_string_t)>();
    mla_link_list_add(mla_arr, mla_str1);
    mla_link_list_add(mla_arr, mla_str2);

    if (!mla_pointer_is_null(mla_str1.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str1.data_storage), (mla_int32_t)2, "String 1 should have refCount of 2 after adding to list");
    } else {
        assert_fail("String 1 dataOwner buffer should not be null after adding to list");
    }

    if (!mla_pointer_is_null(mla_str2.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str2.data_storage), (mla_int32_t)2, "String 2 should have refCount of 2 after adding to list");
    } else {
        assert_fail("String 2 dataOwner buffer should not be null after adding to list");
    }

    mla_link_list_remove(mla_arr, 0);

    if (!mla_pointer_is_null(mla_str1.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str1.data_storage), (mla_int32_t)1, "String 1 should have refCount of 1 after removal from list");
    } else {
        assert_fail("String 1 dataOwner buffer should not be null after removal from list");
    }

    if (!mla_pointer_is_null(mla_str2.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str2.data_storage), (mla_int32_t)2, "String 2 should still have refCount of 2 after removal of String 1");
    } else {
        assert_fail("String 2 dataOwner buffer should not be null after removal of String 1");
    }

    mla_link_list_clear(mla_arr);

    if (!mla_pointer_is_null(mla_str1.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str1.data_storage), (mla_int32_t)1, "String 1 should have refCount of 1 after clearing list");
    } else {
        assert_fail("String 1 dataOwner buffer should not be null after clearing list");
    }

    if (!mla_pointer_is_null(mla_str2.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str2.data_storage), (mla_int32_t)1, "String 2 should have refCount of 1 after clearing list");
    } else {
        assert_fail("String 2 dataOwner buffer should not be null after clearing list");
    }

}

void LinkListItemMemoryManagementDestroyTest() {

    // Make sure that the string has its own memory management
    mla_string_t mla_str1 = mla_string_concat(mla_string("Hel"), mla_string("lo"));
    mla_string_change_memory_layout(mla_str1, MLA_STRING_MEMORY_LAYOUT_BUFFER);
    mla_string_t mla_str2 = mla_string_concat(mla_string("Wor"), mla_string("ld"));
    mla_string_change_memory_layout(mla_str2, MLA_STRING_MEMORY_LAYOUT_BUFFER);

    if (!mla_pointer_is_null(mla_str1.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str1.data_storage), (mla_int32_t)1, "String 1 should have refCount of 1");
    } else {
        assert_fail("String 1 dataOwner buffer should not be null");
    }

    if (!mla_pointer_is_null(mla_str2.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str2.data_storage), (mla_int32_t)1, "String 2 should have refCount of 1");
    } else {
        assert_fail("String 2 dataOwner buffer should not be null");
    }

    {
        mla_link_list_t<mla_init_struct(mla_string_t)> mla_arr = mla_link_list<mla_init_struct(mla_string_t)>();
        mla_link_list_add(mla_arr, mla_str1);
        mla_link_list_add(mla_arr, mla_str2);

        if (!mla_pointer_is_null(mla_str1.data_storage)) {
            assert_equal(mla_pointer_ref_count(mla_str1.data_storage), (mla_int32_t)2, "String 1 should have refCount of 2 after adding to list");
        } else {
            assert_fail("String 1 dataOwner buffer should not be null after adding to list");
        }

        if (!mla_pointer_is_null(mla_str2.data_storage)) {
            assert_equal(mla_pointer_ref_count(mla_str2.data_storage), (mla_int32_t)2, "String 2 should have refCount of 2 after adding to list");
        } else {
            assert_fail("String 2 dataOwner buffer should not be null after adding to list");
        }


    }

    // After the link list is destroyed, the strings should be destroyed as well
    if (!mla_pointer_is_null(mla_str1.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str1.data_storage), (mla_int32_t)1, "String 1 should have refCount of 1 after link list destruction");
    } else {
        assert_fail("String 1 dataOwner buffer should not be null after link list destruction");
    }

    if (!mla_pointer_is_null(mla_str2.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str2.data_storage), (mla_int32_t)1, "String 2 should have refCount of 1 after link list destruction");
    } else {
        assert_fail("String 2 dataOwner buffer should not be null after link list destruction");
    }

}

void LinkListItemMemoryManagementDestroy2Test() {

    // Make sure that the string has its own memory management
    mla_string_t mla_str1 = mla_string_concat(mla_string("Hel"), mla_string("lo"));
    mla_string_change_memory_layout(mla_str1, MLA_STRING_MEMORY_LAYOUT_BUFFER);
    mla_string_t mla_str2 = mla_string_concat(mla_string("Wor"), mla_string("ld"));
    mla_string_change_memory_layout(mla_str2, MLA_STRING_MEMORY_LAYOUT_BUFFER);

    if (!mla_pointer_is_null(mla_str1.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str1.data_storage), (mla_int32_t)1, "String 1 should have refCount of 1");
    } else {
        assert_fail("String 1 dataOwner buffer should not be null");
    }

    if (!mla_pointer_is_null(mla_str2.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str2.data_storage), (mla_int32_t)1, "String 2 should have refCount of 1");
    } else {
        assert_fail("String 2 dataOwner buffer should not be null");
    }

    {

        mla_link_list_t<mla_init_struct(mla_string_t)> other = mla_link_list<mla_init_struct(mla_string_t)>();

        {
            mla_link_list_t<mla_init_struct(mla_string_t)> mla_arr = mla_link_list<mla_init_struct(mla_string_t)>();
            mla_link_list_add(mla_arr, mla_str1);
            mla_link_list_add(mla_arr, mla_str2);

            if (mla_arr.data != nullptr && mla_arr.data->head != nullptr) {
                assert_equal(mla_pointer_ref_count(mla_arr.data->head->nextOwner), (mla_int32_t)2, "String 2 should have refCount of 2 after adding to list");
            } else {
                assert_fail("String 1 dataOwner buffer should not be null after adding to list");
            }

            other = mla_arr;

            if (mla_arr.data != nullptr && mla_arr.data->head != nullptr) {
                assert_equal(mla_pointer_ref_count(mla_arr.data->head->nextOwner), (mla_int32_t)3, "Array should have refCount of 3 after assignment to other list");
            } else {
                assert_fail("Array dataOwner buffer should not be null after assignment to other list");
            }

            if (other.data != nullptr && other.data->head != nullptr) {
                assert_equal(mla_pointer_ref_count(other.data->head->nextOwner), (mla_int32_t)3, "Other list should have refCount of 3 after assignment");
            } else {
                assert_fail("Other list dataOwner buffer should not be null after assignment to other list");
            }
        }

        assert_equal(mla_link_list_size(other), (mla_size_t)2, "Other list should have size 2 after assignment");

        if (other.data != nullptr && other.data->head != nullptr) {
            assert_equal(mla_pointer_ref_count(other.data->head->nextOwner), (mla_int32_t)2, "link should have refCount of 2 after other list is removed");
        } else {
            assert_fail("Other list dataOwner buffer should not be null after other list is removed");
        }

        if (!mla_pointer_is_null(mla_str1.data_storage)) {
            assert_equal(mla_pointer_ref_count(mla_str1.data_storage), (mla_int32_t)2, "String 1 should have refCount of 2 after assignment to other list");
        } else {
            assert_fail("String 1 dataOwner buffer should not be null after assignment to other list");
        }

        if (!mla_pointer_is_null(mla_str2.data_storage)) {
            assert_equal(mla_pointer_ref_count(mla_str2.data_storage), (mla_int32_t)2, "String 2 should have refCount of 2 after assignment to other list");
        } else {
            assert_fail("String 2 dataOwner buffer should not be null after assignment to other list");
        }

    }

    // After the link list is destroyed, the strings should be destroyed as well
    if (!mla_pointer_is_null(mla_str1.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str1.data_storage), (mla_int32_t)1, "String 1 should have refCount of 1 after link list destruction");
    } else {
        assert_fail("String 1 dataOwner buffer should not be null after link list destruction");
    }

    if (!mla_pointer_is_null(mla_str2.data_storage)) {
        assert_equal(mla_pointer_ref_count(mla_str2.data_storage), (mla_int32_t)1, "String 2 should have refCount of 1 after link list destruction");
    } else {
        assert_fail("String 2 dataOwner buffer should not be null after link list destruction");
    }
}


void LinkListWithValueStructTest() {

    mla_link_list_t<my_link_list_test_struct> list = mla_link_list<my_link_list_test_struct>();

    my_link_list_test_struct item1 = {1, 100};
    my_link_list_test_struct item2 = {2, 200};
    my_link_list_test_struct item3 = {3, 300};

    mla_link_list_add(list, item1);
    mla_link_list_add(list, item2);
    mla_link_list_add(list, item3);


    assert_true(mla_link_list_contains(list, item1), "linkList should contain item1");
    assert_true(mla_link_list_contains(list, item2), "linkList should contain item2");
    assert_true(mla_link_list_contains(list, item3), "linkList should contain item3");


    my_link_list_test_struct value;
    assert_true(mla_link_list_get(list, 1, value), "linkList should successfully retrieve value for index 1");
    assert_equal(value.test1, 2L, "Value test1 should be equal to 2");
    assert_equal(value.test2, 200L, "Value test2 should be equal to 200");

    my_link_list_test_struct* valueRef = mla_link_list_get_ref(list, 1);

    if (valueRef != nullptr) {
        valueRef->test2 = 0; // Reset value for next check
    } else {
        assert_fail("ValueRef should not be null for index 1");
    }


    assert_true(mla_link_list_get(list, 1, value), "linkList should successfully retrieve value for index 1");
    assert_equal(value.test1, 2L, "Value test1 should be equal to 2 after retrieval");
    assert_equal(value.test2, 0L, "Value test2 should be equal to 0 after retrieval");

    assert_null(mla_link_list_get_ref(list, 3), "linkList should return null for index 3 (out of bounds)");
}


void LinkListWithValueConstStructTest() {

    mla_link_list_t<my_link_list_with_const_test_struct> list = mla_link_list<my_link_list_with_const_test_struct>();

    my_link_list_with_const_test_struct item1 = {1, 100};
    my_link_list_with_const_test_struct item2 = {2, 200};
    my_link_list_with_const_test_struct item3 = {3, 300};

    mla_link_list_add(list, item1);
    mla_link_list_add(list, item2);
    mla_link_list_add(list, item3);


    assert_true(mla_link_list_contains(list, item1), "linkList should contain item1");
    assert_true(mla_link_list_contains(list, item2), "linkList should contain item2");
    assert_true(mla_link_list_contains(list, item3), "linkList should contain item3");


    my_link_list_with_const_test_struct value;
    assert_true(mla_link_list_get(list, 1, value), "linkList should successfully retrieve value for index 1");
    assert_equal(value.test1, 2L, "Value test1 should be equal to 2");
    assert_equal(value.test2, 200L, "Value test2 should be equal to 200");

    my_link_list_with_const_test_struct* valueRef = mla_link_list_get_ref(list, 1);

    if (valueRef != nullptr) {
        valueRef->test2 = 0; // Reset value for next check
    } else {
        assert_fail("ValueRef should not be null for index 1");
    }

    assert_true(mla_link_list_get(list, 1, value), "linkList should successfully retrieve value for index 1");
    assert_equal(value.test1, 2L, "Value test1 should be equal to 2 after retrieval");
    assert_equal(value.test2, 0L, "Value test2 should be equal to 0 after retrieval");

    assert_null(mla_link_list_get_ref(list, 3), "linkList should return null for index 3 (out of bounds)");
}

void LinkListDestructionCycleTest() {
    mla_link_list_t<int> list = mla_link_list<int>();
    mla_link_list_add(list, 10);
    mla_link_list_add(list, 20);
    mla_link_list_add(list, 30);
    assert_equal(mla_link_list_size(list), (mla_size_t)3, "List size should be 3 before destruction");
}


void RegisterLinkListTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("DestructionCycle", test_category, LinkListDestructionCycleTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ContainsMlaString", test_category, LinkListContainsMlaStringTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Contains", test_category, LinkListContainsTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Remove", test_category, LinkListRemoveTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Clear", test_category, LinkListClearTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("IndexOf", test_category, LinkListIndexOfTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("AddMuchItems", test_category, LinkListAddMuchItemsTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ItemMemoryManagement", test_category, LinkListItemMemoryManagementTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ItemMemoryManagementDestroy", test_category, LinkListItemMemoryManagementDestroyTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ItemMemoryManagementDestroy2", test_category, LinkListItemMemoryManagementDestroy2Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("WithValueStruct", test_category, LinkListWithValueStructTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("WithValueConstStruct", test_category, LinkListWithValueConstStructTest);
    mla_test_executor_register_test(p_TestExecutor, test);


}

static mla_link_list_t<int> mla_arr_data = mla_link_list_empty<int>();

void SetupLinkListContainsBenchmark() {

    for (int i = 0; i < CONST_LIST_CONTAINS_COUNT; ++i) {
        mla_link_list_add(mla_arr_data, i);
    }

}

void LinkListContainsBenchmark() {

    for (int i = 0; i < CONST_LIST_CONTAINS_COUNT; ++i) {
        mla_bool_t found = mla_link_list_contains<int>(mla_arr_data, i);

        if (!found) {
            // Fail
            static_assert(true, "Element not found in link");
        }
    }

}


void TearDownLinkListContainsBenchmark() {

    mla_arr_data = mla_link_list_empty<int>();
}

void LinkListAddMuchItemsBenchmark() {

    mla_link_list_t<int> mla_arr = mla_link_list<int>();
    for (int i = 0; i < 1000; ++i) {
        mla_link_list_add(mla_arr, i);
    }
}

void RegisterLinkListBenchmarks(mla_benchmark_executor_t &p_BenchmarkExecutor) {

    mla_benchmark_t benchmark = mla_benchmark("Contains", benchmark_category, LinkListContainsBenchmark, SetupLinkListContainsBenchmark, TearDownLinkListContainsBenchmark);
    mla_benchmark_set_iteration_division(benchmark, 5000);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("AddMuchItems", benchmark_category, LinkListAddMuchItemsBenchmark);
    mla_benchmark_set_iteration_division(benchmark, 100);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

}

#endif

