//
// Created by chris on 8/2/2025.
//

#ifndef MLA_STRING_TEST_H
#define MLA_STRING_TEST_H

#include "../../lib/base-lib/core/memory/mla_memory_hook.h"
#include "../../lib/base-lib/core/system/mla_string.h"
#include "../../lib/base-lib/core/system/mla_string_concat.h"
#include "../../lib/base-lib/test-support/mla_test_executor.h"
#include "../../lib/base-lib/test-support/mla_benchmark_executor.h"
#include "../../lib/base-lib/test-support/mla_test_utils.h"

void SizeOfTest() {
    assert_true(sizeof(mla_string_t) <= 32, "Size of mla_string_t should be less than or equal to 32 bytes");
    assert_true(sizeof(mla_string_internal_embedded_t) >= sizeof(mla_string_internal_heap_t), "Size of embedded layout should be greater than or equal to heap layout");
}

void ContainsCLayoutTest() {
    mla_char_t data[14] = "Hello, World!";
    mla_string_t mla_str = mla_string(mla_platform_pointer_to_managed_pointer(data));
    assert_equal(mla_string_get_memory_layout(mla_str), MLA_STRING_MEMORY_LAYOUT_C_STRING, "MlaString should be C layout");
    assert_true(mla_string_contains(mla_str, mla_string("World")), "MlaString should contain 'World'");
    assert_false(mla_string_contains(mla_str, mla_string("world")), "MlaString should not contain 'world'");
}

void ContainsBufferLayoutTest() {
    mla_string_t mla_str = mla_string(mla_platform_pointer_to_managed_pointer("Hello, World!"), 13);
    assert_equal(mla_string_get_memory_layout(mla_str), MLA_STRING_MEMORY_LAYOUT_BUFFER, "MlaString should be C layout");
    assert_true(mla_string_contains(mla_str, mla_string("World")), "MlaString should contain 'World'");
    assert_false(mla_string_contains(mla_str, mla_string("world")), "MlaString should not contain 'world'");
}

void ContainsEmbeddedLayoutTest() {
    mla_string_t mla_str = mla_string_const("Hello, World!");
    mla_string_change_memory_layout(mla_str, MLA_STRING_MEMORY_LAYOUT_EMBEDDED);
    assert_equal(mla_string_get_memory_layout(mla_str), MLA_STRING_MEMORY_LAYOUT_EMBEDDED, "MlaString should be C layout");
    assert_true(mla_string_contains(mla_str, mla_string("World")), "MlaString should contain 'World'");
    assert_false(mla_string_contains(mla_str, mla_string("world")), "MlaString should not contain 'world'");
}

void StartsWithTest() {
    mla_string_t mla_str = mla_string("Hello, World!");
    assert_true(mla_string_starts_with(mla_str, mla_string("Hello")), "MlaString should start with 'Hello'");
    assert_false(mla_string_starts_with(mla_str, mla_string("World")), "MlaString should not start with 'World'");
}

void LengthTest() {
    mla_string_t mla_str = mla_string("Hello, World!");
    assert_equal(mla_string_length(mla_str), (mla_uint32_t)13, "MlaString length should be 13");
    mla_string_t empty_mla_str = mla_string("");
    assert_equal(mla_string_length(empty_mla_str), (mla_uint32_t)0, "Length of empty MlaString should be 0");
}

void EndsWithTest() {
    mla_string_t mla_str = mla_string("Hello, World!");
    assert_true(mla_string_ends_with(mla_str, mla_string("World!")), "MlaString should end with 'World!'");
    assert_false(mla_string_ends_with(mla_str, mla_string("Hello")), "MlaString should not end with 'Hello'");
}

void EqualsTest() {
    mla_string_t mla_str1 = mla_string("Hello, World!");
    mla_string_t mla_str2 = mla_string("Hello, World!");
    mla_string_t mla_str3 = mla_string("Goodbye, World!");

    assert_true(mla_string_equals(mla_str1, mla_str2), "MlaString should be equal");
    assert_false(mla_string_equals(mla_str1, mla_str3), "MlaString should not be equal");
}

void EqualsIgnoreCaseTest() {

    mla_string_t mla_str1 = mla_string("Hello, World!");
    mla_string_t mla_str2 = mla_string("hello, wOrld!");
    mla_string_t mla_str3 = mla_string("Goodbye, World!");

    assert_true(mla_string_equals_ignore_case(mla_str1, mla_str2), "MlaString should be equal ignoring case");
    assert_false(mla_string_equals_ignore_case(mla_str1, mla_str3), "MlaString should not be equal ignoring case");
}

void IndexOfCLayoutTest() {
    mla_char_t data[14] = "Hello, World!";
    mla_string_t mla_str = mla_string(mla_platform_pointer_to_managed_pointer(data));
    assert_equal(mla_string_get_memory_layout(mla_str), MLA_STRING_MEMORY_LAYOUT_C_STRING, "MlaString should be C layout");
    assert_equal(mla_string_index_of(mla_str, mla_string("World")), (mla_int32_t)7,
                 "MlaString index of 'World' should be 7");
    assert_equal(mla_string_index_of(mla_str, mla_string("world")), (mla_int32_t)-1,
                 "MlaString index of 'world' should be -1 (not found)");
}

void IndexOfBufferLayoutTest() {

    mla_string_t mla_str = mla_string(mla_platform_pointer_to_managed_pointer("Hello, World!"), 13);
    assert_equal(mla_string_get_memory_layout(mla_str), MLA_STRING_MEMORY_LAYOUT_BUFFER, "MlaString should be buffer layout");
    assert_equal(mla_string_index_of(mla_str, mla_string("World")), (mla_int32_t)7,
                 "MlaString index of 'World' should be 7");
    assert_equal(mla_string_index_of(mla_str, mla_string("world")), (mla_int32_t)-1,
                 "MlaString index of 'world' should be -1 (not found)");
}

void IndexOfEmbeddedLayoutTest() {

    mla_string_t mla_str = mla_string_const("Hello, World!");
    mla_string_change_memory_layout(mla_str, MLA_STRING_MEMORY_LAYOUT_EMBEDDED);
    assert_equal(mla_string_get_memory_layout(mla_str), MLA_STRING_MEMORY_LAYOUT_EMBEDDED, "MlaString should be embedded layout");
    assert_equal(mla_string_index_of(mla_str, mla_string("World")), (mla_int32_t)7,
                 "MlaString index of 'World' should be 7");
    assert_equal(mla_string_index_of(mla_str, mla_string("world")), (mla_int32_t)-1,
                 "MlaString index of 'world' should be -1 (not found)");
}

void IndexOfWithStartCLayoutTest() {
    mla_char_t data[31] = "Hello, World! Hello, Universe!";
    mla_char_t helloData[6] = "Hello";

    mla_string_t mla_str = mla_string(mla_platform_pointer_to_managed_pointer(data));
    mla_string_t hello = mla_string(mla_platform_pointer_to_managed_pointer(helloData));

    assert_equal(mla_string_get_memory_layout(mla_str), MLA_STRING_MEMORY_LAYOUT_C_STRING, "MlaString should be C layout");
    assert_equal(mla_string_get_memory_layout(hello), MLA_STRING_MEMORY_LAYOUT_C_STRING, "Substring should be C layout");

    assert_equal(mla_string_index_of(mla_str, hello, 1), (mla_int32_t)14,
                 "MlaString index of 'Hello' from index 1 should skip the first occurrence");
    assert_equal(mla_string_index_of(mla_str, hello, 14), (mla_int32_t)14,
                 "MlaString index of 'Hello' from the second occurrence should return 14");
    assert_equal(mla_string_index_of(mla_str, hello, 15), (mla_int32_t)-1,
                 "MlaString index of 'Hello' after the second occurrence should be -1");
    assert_equal(mla_string_index_of(mla_str, mla_string_const(""), 5), (mla_int32_t)5,
                 "Empty substring should match at the provided start index");
    assert_equal(mla_string_index_of(mla_str, hello, mla_string_length(mla_str)), (mla_int32_t)-1,
                 "Start index at the string length should return -1");
}

void IndexOfWithStartBufferLayoutTest() {
    mla_string_t mla_str = mla_string(mla_platform_pointer_to_managed_pointer("abcabcabc"), 9);

    assert_equal(mla_string_get_memory_layout(mla_str), MLA_STRING_MEMORY_LAYOUT_BUFFER, "MlaString should be buffer layout");
    assert_equal(mla_string_index_of(mla_str, mla_string_const("abc"), 1), (mla_int32_t)3,
                 "Buffer layout index of 'abc' from index 1 should return the second occurrence");
    assert_equal(mla_string_index_of(mla_str, mla_string_const("abc"), 3), (mla_int32_t)3,
                 "Buffer layout index of 'abc' from the second occurrence should return 3");
    assert_equal(mla_string_index_of(mla_str, mla_string_const("abc"), 4), (mla_int32_t)6,
                 "Buffer layout index of 'abc' from index 4 should return the third occurrence");
    assert_equal(mla_string_index_of(mla_str, mla_string_const("abc"), 7), (mla_int32_t)-1,
                 "Buffer layout index of 'abc' from index 7 should be -1 because the substring no longer fits");
}

void IndexOfWithStartEmbeddedLayoutTest() {
    mla_string_t mla_str = mla_string_const("abcabcabc");
    assert_true(mla_string_change_memory_layout(mla_str, MLA_STRING_MEMORY_LAYOUT_EMBEDDED),
                "MlaString should convert to embedded layout");

    assert_equal(mla_string_get_memory_layout(mla_str), MLA_STRING_MEMORY_LAYOUT_EMBEDDED, "MlaString should be embedded layout");
    assert_equal(mla_string_index_of(mla_str, mla_string_const("abc"), 0), (mla_int32_t)0,
                 "Embedded layout index of 'abc' from index 0 should return the first occurrence");
    assert_equal(mla_string_index_of(mla_str, mla_string_const("abc"), 1), (mla_int32_t)3,
                 "Embedded layout index of 'abc' from index 1 should return the second occurrence");
    assert_equal(mla_string_index_of(mla_str, mla_string_const("abc"), 6), (mla_int32_t)6,
                 "Embedded layout index of 'abc' from the last valid start should return 6");
    assert_equal(mla_string_index_of(mla_str, mla_string_const("abc"), 7), (mla_int32_t)-1,
                 "Embedded layout index of 'abc' from index 7 should be -1 because the substring no longer fits");
}

void IndexOfWithStartFastPathTest() {

    mla_string_t bufferStr = mla_string(mla_platform_pointer_to_managed_pointer("01234567890123456789"), 20);
    assert_equal(mla_string_get_memory_layout(bufferStr), MLA_STRING_MEMORY_LAYOUT_BUFFER,
                 "MlaString should be buffer layout");

    assert_equal(mla_string_index_of(bufferStr, mla_string_const("5"), 6), (mla_int32_t)15,
                 "Fast path with 1-char substring should respect the start index");
    assert_equal(mla_string_index_of(bufferStr, mla_string_const("56"), 6), (mla_int32_t)15,
                 "Fast path with 2-char substring should respect the start index");
    assert_equal(mla_string_index_of(bufferStr, mla_string_const("0"), 1), (mla_int32_t)10,
                 "Fast path should find the next matching character after the start index");
    assert_equal(mla_string_index_of(bufferStr, mla_string_const("56"), 16), (mla_int32_t)-1,
                 "Fast path with start index beyond the last valid match should return -1");
}

void IndexOfFastPathTest() {

    mla_string_t bufferStr = mla_string(mla_platform_pointer_to_managed_pointer("01234567890123456789"), 20);
    assert_equal(mla_string_get_memory_layout(bufferStr), MLA_STRING_MEMORY_LAYOUT_BUFFER,
                 "MlaString should be buffer layout");

    assert_equal(mla_string_index_of(bufferStr, mla_string_const("5")), (mla_int32_t)5,
                 "Fast path with 1-char substring should return first match");
    assert_equal(mla_string_index_of(bufferStr, mla_string_const("56")), (mla_int32_t)5,
                 "Fast path with 2-char substring should return first match");
    assert_equal(mla_string_index_of(bufferStr, mla_string_const("AA")), (mla_int32_t)-1,
                 "Fast path with 2-char substring should return -1 when not found");

    mla_string_t embeddedStr = mla_string("012345678");
    assert_equal(mla_string_get_memory_layout(embeddedStr), MLA_STRING_MEMORY_LAYOUT_EMBEDDED,
                 "MlaString should be embedded layout");

    assert_equal(mla_string_index_of(embeddedStr, mla_string_const("5")), (mla_int32_t)5,
                 "Embedded layout fast path with 1-char substring should return first match");
    assert_equal(mla_string_index_of(embeddedStr, mla_string_const("56")), (mla_int32_t)5,
                 "Embedded layout fast path with 2-char substring should return first match");
}

void LastIndexOfTest() {
    mla_string_t mla_str = mla_string("Hello, World! Hello, Universe!");
    assert_equal(mla_string_last_index_of(mla_str, mla_string("Hello")), (mla_int32_t)14,
                 "MlaString last index of 'Hello' should be 19");
    assert_equal(mla_string_last_index_of(mla_str, mla_string("world")), (mla_int32_t)-1,
                 "MlaString last index of 'world' should be -1 (not found)");
}

void ToCStringTest() {
    mla_string_t mla_str = mla_string("Hello, World!");
    mla_c_string_t mla_c_str = mla_string_to_cString(mla_str);

    mla_char_t* c_str = mla_pointer_get_data<mla_char_t>(mla_c_str.c_heap_str);

    assert_true(c_str != nullptr, "MlaString C-string should not be null");

    if (c_str != nullptr) {
        assert_equal(mla_strlen(c_str), (mla_uint32_t)13, "MlaString C-string length should be 13");
    } else {
        assert_fail("MlaString C-string conversion failed");
    }
}

void ToCStringFromBufferTest() {
    mla_string_t mla_str = mla_string_const("Hello, World!");
    mla_c_string_t mla_c_str = mla_string_to_cString(mla_str);

    mla_char_t* c_str = mla_pointer_get_data<mla_char_t>(mla_c_str.c_heap_str);

    assert_true(c_str != nullptr, "MlaString C-string should not be null");

    if (c_str != nullptr) {
        assert_equal(mla_strlen(c_str), (mla_uint32_t)13, "MlaString C-string length should be 13");
    } else {
        assert_fail("MlaString C-string conversion failed");
    }
}

void ToCStringFromBuffer_No_Force_CopyTest() {
    mla_string_t mla_str = mla_string_const("Hello, World!");
    mla_c_string_t mla_c_str = mla_string_to_cString(mla_str);

    mla_char_t* c_str = mla_pointer_get_data<mla_char_t>(mla_c_str.c_heap_str);

    if (c_str != nullptr) {
        assert_equal(mla_strlen(c_str), (mla_uint32_t)13, "MlaString C-string length should be 13");
    } else {
        assert_fail("MlaString C-string conversion failed");
    }

    mla_c_string_t mla_c_str2 = mla_string_to_cString(mla_str);

    mla_char_t* c_str2 = mla_pointer_get_data<mla_char_t>(mla_c_str.c_heap_str);

    if (c_str2 != nullptr) {
        assert_equal(mla_strlen(c_str2), (mla_uint32_t)13, "MlaString C-string length should be 13");
    } else {
        assert_fail("MlaString C-string conversion failed");
    }
}

void CStringDataAccessorTest() {
    mla_string_t str = mla_string("hello");
    mla_c_string_t cStr = mla_string_to_cString(str);
    const mla_char_t *cData = mla_c_string_data(cStr);

    assert_true(cData != nullptr, "C-string data accessor should return valid pointer");
    if (cData == nullptr) {
        return;
    }

    assert_equal(mla_strlen(cData), (mla_size_t)5, "C-string data should have expected length");
}

void StringFromCStringWithLengthTest() {
    mla_char_t sourceBuffer[sizeof("hello-world")] = "hello-world";
    mla_string_t str = mla_string_from_c_string(mla_platform_pointer_to_managed_pointer(sourceBuffer), 5);

    assert_equal(mla_string_length(str), (mla_size_t)5, "String length should match provided length");
    assert_equal(mla_string_get_memory_layout(str), MLA_STRING_MEMORY_LAYOUT_C_STRING, "String should use C-string layout");
    assert_true(mla_string_equals(str, mla_string("hello")), "String content should match the provided prefix");
}

void AccessCharTest() {
    mla_string_t mla_str = mla_string("Hello, World!");
    const mla_char_t* data = mla_string_data(mla_str);
    assert_equal(data[0], 'H', "First character of MlaString should be 'H'");
    assert_equal(data[7], 'W', "Eighth character of MlaString should be 'W'");
    assert_equal(data[12], '!', "Last character of MlaString should be '!'");
}

void ConcatTest() {
    mla_string_t mla_str1 = mla_string("Hello, ");
    mla_string_t mla_str2 = mla_string("World!");
    mla_string_t mla_result = mla_string_concat(mla_str1, mla_str2);

    assert_equal(mla_string_length(mla_result), (mla_uint32_t)13, "MlaString concatenated length should be 13");
    assert_true(mla_string_equals(mla_result, mla_string("Hello, World!")),
                "MlaString concatenated should equal 'Hello, World!'");
}

static mla_bool_t AutoMemoryManagementTest_Free_Called;

mla_bool_t AutoMemoryManagementTest_Malloc(mla_size_t size, mla_platform_pointer_t*out_ptr) {
    (void) size;
    (void) out_ptr;
    return false;
}

mla_bool_t AutoMemoryManagementTest_Free(mla_platform_pointer_t ptr) {
    (void) ptr;
    AutoMemoryManagementTest_Free_Called = true;
    return false;
}

void AutoMemoryManagementTest() {

    mla_memory_hook_t hook = mla_memory_hook_install(AutoMemoryManagementTest_Malloc, AutoMemoryManagementTest_Free); {
        mla_string_t datacopy = mla_string_empty(); {
            mla_string_t data = mla_string_concat(mla_string("Hello, "), mla_string("World!"),
                                                  mla_string(" This is a test of concatenation."));
            assert_equal(mla_string_length(data), (mla_uint32_t)46, "Concatenated string length should be 58");

            if (!mla_pointer_is_null(data.data_storage)) {
                assert_equal(mla_pointer_ref_count(data.data_storage), (mla_int32_t)1,
                             "Reference count should be 1 after concatenation");
                datacopy = data;
                assert_equal(mla_pointer_ref_count(data.data_storage), (mla_int32_t)2,
                             "Reference count should be 2 after copying");
                assert_equal(mla_pointer_ref_count(datacopy.data_storage), (mla_int32_t)2,
                             "Reference count should be 2 after copying");
                assert_equal(mla_pointer_get_platform_pointer(datacopy.data_storage), mla_pointer_get_platform_pointer(data.data_storage),
                             "Copied string data should match original");
            } else {
                assert_fail("Data buffer should not be null after concatenation");
            }

            data = mla_string_empty(); // Clear the original string
        }

        if (!mla_pointer_is_null(datacopy.data_storage)) {
            assert_equal(mla_pointer_ref_count(datacopy.data_storage), (mla_int32_t)1,
                         "Reference count should be 1 after clearing the original string");

            assert_equal(mla_string_length(datacopy), (mla_uint32_t)46, "Copied string length should still be 58");
            assert_true(mla_string_equals(datacopy, mla_string("Hello, World! This is a test of concatenation.")),
                        "Copied string should equal 'Hello, World! This is a test of concatenation.'");
        } else {
            assert_fail("Data buffer should not be null after clearing");
        }

        AutoMemoryManagementTest_Free_Called = false; // Reset the flag before clearing the copy
    }
    // Check ich the buffer was released
    assert_true(AutoMemoryManagementTest_Free_Called, "Free should have been called after clearing the copy");

    mla_memory_hook_uninstall(hook);
}

void SubStringTest() {
    mla_string_t mla_str = mla_string_concat(mla_string("Hello, "), mla_string("World, What are you doing today!"));
    mla_string_change_memory_layout(mla_str, MLA_STRING_MEMORY_LAYOUT_BUFFER);
    mla_string_t sub_str = mla_string_substr(mla_str, 7, 31); // "World"

    // Check memory managemant
    if (!mla_pointer_is_null(sub_str.data_storage)) {
        assert_equal(mla_pointer_ref_count(sub_str.data_storage), (mla_int32_t)2,
                     "Reference count should be 2 after creating substring");
    } else {
        assert_fail("Data buffer should not be null after concatenation");
    }

    assert_equal(mla_string_get_memory_layout(sub_str), MLA_STRING_MEMORY_LAYOUT_SUB_STRING, "Substring should have SUB_STRING layout");
    assert_equal(mla_string_length(sub_str), (mla_uint32_t)31, "Substring length should be 5");
    assert_true(mla_string_equals(sub_str, mla_string("World, What are you doing today")), "Substring should equal 'World, What are you doing today'");
}

void SubStringEmbeddedTest() {
    mla_string_t mla_str = mla_string_concat(mla_string("Hello, "), mla_string("World!"));
    assert_equal(mla_string_get_memory_layout(mla_str), MLA_STRING_MEMORY_LAYOUT_EMBEDDED, "Substring should have EMBEDDED layout");
    mla_string_t sub_str = mla_string_substr(mla_str, 7, 5); // "World"

    assert_equal(mla_string_get_memory_layout(sub_str), MLA_STRING_MEMORY_LAYOUT_EMBEDDED, "Substring should have EMBEDDED layout");
    assert_equal(mla_string_length(sub_str), (mla_uint32_t)5, "Substring length should be 5");
    assert_true(mla_string_equals(sub_str, mla_string("World")), "Substring should equal 'World'");
}

void MultiByteCharHandlingTest() {
    mla_string_t mla_str = mla_string("€ 100");
    assert_equal(mla_string_length(mla_str), (mla_uint32_t)7, "MlaString length should be 7 bytes");

    assert_equal(mla_string_multi_byte_char_count(mla_str), (mla_uint32_t)5,
                 "MlaString should have 4 multi-byte characters");

    mla_multi_byte_char_t mb_char = mla_string_multi_byte_char_at(mla_str, 0);
    assert_equal(mb_char.bytes[0], (mla_char_t)(mla_uint8_t)0xE2, "First byte of Euro sign should be 0xE2");
    assert_equal(mb_char.bytes[1], (mla_char_t)(mla_uint8_t)0x82, "Second byte of Euro sign should be 0x82");
    assert_equal(mb_char.bytes[2], (mla_char_t)(mla_uint8_t)0xAC, "Third byte of Euro sign should be 0xAC");
    assert_equal(mb_char.bytes[3], (mla_char_t)(mla_uint8_t)0x00, "Fourth byte of Euro sign should be null terminator");

    mb_char = mla_string_multi_byte_char_at(mla_str, 1);
    assert_equal(mb_char.bytes[0], (mla_char_t)(mla_uint8_t)' ', "First byte of space should be ' '");
    assert_equal(mb_char.bytes[1], (mla_char_t)(mla_uint8_t)0x00, "Second byte of space should be null terminator");
    assert_equal(mb_char.bytes[2], (mla_char_t)(mla_uint8_t)0x00, "Third byte of space should be null terminator");
    assert_equal(mb_char.bytes[3], (mla_char_t)(mla_uint8_t)0x00, "Third byte of space should be null terminator");

    mb_char = mla_string_multi_byte_char_at(mla_str, 2);
    assert_equal(mb_char.bytes[0], (mla_char_t)(mla_uint8_t)'1', "First byte of space should be '1'");
    assert_equal(mb_char.bytes[1], (mla_char_t)(mla_uint8_t)0x00, "Second byte of space should be null terminator");
    assert_equal(mb_char.bytes[2], (mla_char_t)(mla_uint8_t)0x00, "Third byte of space should be null terminator");
    assert_equal(mb_char.bytes[3], (mla_char_t)(mla_uint8_t)0x00, "Third byte of space should be null terminator");

    mb_char = mla_string_multi_byte_char_at(mla_str, 3);
    assert_equal(mb_char.bytes[0], (mla_char_t)(mla_uint8_t)'0', "First byte of space should be '0'");
    assert_equal(mb_char.bytes[1], (mla_char_t)(mla_uint8_t)0x00, "Second byte of space should be null terminator");
    assert_equal(mb_char.bytes[2], (mla_char_t)(mla_uint8_t)0x00, "Third byte of space should be null terminator");
    assert_equal(mb_char.bytes[3], (mla_char_t)(mla_uint8_t)0x00, "Third byte of space should be null terminator");

    mb_char = mla_string_multi_byte_char_at(mla_str, 4);
    assert_equal(mb_char.bytes[0], (mla_char_t)(mla_uint8_t)'0', "First byte of space should be '0'");
    assert_equal(mb_char.bytes[1], (mla_char_t)(mla_uint8_t)0x00, "Second byte of space should be null terminator");
    assert_equal(mb_char.bytes[2], (mla_char_t)(mla_uint8_t)0x00, "Third byte of space should be null terminator");
    assert_equal(mb_char.bytes[3], (mla_char_t)(mla_uint8_t)0x00, "Third byte of space should be null terminator");

    mb_char = mla_string_multi_byte_char_at(mla_str, 5);
    assert_equal(mb_char.bytes[0], (mla_char_t)(mla_uint8_t)0x00, "First byte of space should be null terminator");
    assert_equal(mb_char.bytes[1], (mla_char_t)(mla_uint8_t)0x00, "Second byte of space should be null terminator");
    assert_equal(mb_char.bytes[2], (mla_char_t)(mla_uint8_t)0x00, "Third byte of space should be null terminator");
    assert_equal(mb_char.bytes[3], (mla_char_t)(mla_uint8_t)0x00, "Third byte of space should be null terminator");
}

void ToUtf16AndFromUtf16Test() {
    mla_string_t baseString = mla_string("€ 100");
    mla_string_utf16_buffer_t utf16Buffer = mla_string_to_utf16_buffer(baseString);
    assert_equal(utf16Buffer.charCount, (mla_uint32_t)5, "UTF-16 buffer should have 5 characters");

    const mla_utf_16_char_t* utf16Buffer_data = mla_string_utf16_data(utf16Buffer);

    // Euro sign is U+20AC, which is 0x20AC in UTF-16
    if (utf16Buffer_data != nullptr) {
        assert_equal(utf16Buffer_data[0], (mla_utf_16_char_t)0x20AC, "First UTF-16 character should be Euro sign");
        assert_equal(utf16Buffer_data[1], (mla_utf_16_char_t)' ', "Second UTF-16 character should be space");
        assert_equal(utf16Buffer_data[2], (mla_utf_16_char_t)'1', "Third UTF-16 character should be '1'");
        assert_equal(utf16Buffer_data[3], (mla_utf_16_char_t)'0', "Fourth UTF-16 character should be '0'");
        assert_equal(utf16Buffer_data[4], (mla_utf_16_char_t)'0', "Fifth UTF-16 character should be '0'");
        assert_equal(utf16Buffer_data[5], (mla_utf_16_char_t)0x00, "Six UTF-16 character should be null terminator");
    } else {
        assert_fail("UTF-16 buffer is empty");
    }

    mla_string_t newBaseString = mla_string_from_utf16_buffer(utf16Buffer);
    assert_struct_equal(mla_string_t, baseString, newBaseString,
                        "New MlaString from UTF-16 buffer should equal original");

}

void ToUtf32AndFromUtf32Test() {
    mla_string_t baseString = mla_string("€ 100");
    mla_string_utf32_buffer_t utf32Buffer = mla_string_to_utf32_buffer(baseString);
    assert_equal(utf32Buffer.charCount, (mla_uint32_t)5, "UTF-32 buffer should have 5 characters");

    // Euro sign is U+20AC, which is 0x20AC in UTF-32
    const mla_utf_32_char_t* utf32Buffer_data = mla_string_utf32_data(utf32Buffer);

    if (utf32Buffer_data != nullptr) {
        assert_equal(utf32Buffer_data[0], (mla_utf_32_char_t)0x000020AC, "First UTF-32 character should be Euro sign");
        assert_equal(utf32Buffer_data[1], (mla_utf_32_char_t)' ', "Second UTF-32 character should be space");
        assert_equal(utf32Buffer_data[2], (mla_utf_32_char_t)'1', "Third UTF-32 character should be '1'");
        assert_equal(utf32Buffer_data[3], (mla_utf_32_char_t)'0', "Fourth UTF-32 character should be '0'");
        assert_equal(utf32Buffer_data[4], (mla_utf_32_char_t)'0', "Fifth UTF-32 character should be '0'");
        assert_equal(utf32Buffer_data[5], (mla_utf_32_char_t)0x00000000,
                     "Sixth UTF-32 character should be null terminator");
    } else {
        assert_fail("UTF-32 buffer is empty");
    }


    mla_string_t newBaseString = mla_string_from_utf32_buffer(utf32Buffer);
    assert_struct_equal(mla_string_t, baseString, newBaseString,
                        "New MlaString from UTF-32 buffer should equal original");
}

void ToUtf16AndFromUtf16_EmptyTest() {

    mla_string_t baseString = mla_string("");
    mla_string_utf16_buffer_t utf16Buffer = mla_string_to_utf16_buffer(baseString);
    assert_equal(utf16Buffer.charCount, (mla_uint32_t)0, "UTF-16 empty: charCount should be 0");

    const mla_utf_16_char_t* utf16Buffer_data = mla_string_utf16_data(utf16Buffer);

    assert_null(utf16Buffer_data, "UTF-16 empty: data shoud be null");
    mla_string_t roundTrip = mla_string_from_utf16_buffer(utf16Buffer);
    assert_true(mla_string_is_empty(roundTrip), "UTF-16 empty: round trip string should be empty");
}

void ToUtf16AndFromUtf16_AsciiTest() {
    mla_string_t baseString = mla_string("ABC");
    mla_string_utf16_buffer_t utf16Buffer = mla_string_to_utf16_buffer(baseString);
    assert_equal(utf16Buffer.charCount, (mla_uint32_t)3, "UTF-16 ASCII: charCount should be 3");

    const mla_utf_16_char_t* utf16Buffer_data = mla_string_utf16_data(utf16Buffer);

    if (utf16Buffer_data != nullptr) {
        assert_equal(utf16Buffer_data[0], (mla_utf_16_char_t)'A', "UTF-16 ASCII: data[0]=='A'");
        assert_equal(utf16Buffer_data[1], (mla_utf_16_char_t)'B', "UTF-16 ASCII: data[1]=='B'");
        assert_equal(utf16Buffer_data[2], (mla_utf_16_char_t)'C', "UTF-16 ASCII: data[2]=='C'");
        assert_equal(utf16Buffer_data[3], (mla_utf_16_char_t)0x00, "UTF-16 ASCII: null terminator");
    } else {
        assert_fail("UTF-16 buffer is empty");
    }

    mla_string_t roundTrip = mla_string_from_utf16_buffer(utf16Buffer);
    assert_struct_equal(mla_string_t, baseString, roundTrip, "UTF-16 ASCII round trip should match");

}

void ToUtf16AndFromUtf16_MixedBmpTest() {
    // Euro (U+20AC) Omega (U+03A9) Eszett (U+00DF)
    mla_string_t baseString = mla_string("€Ωß");
    mla_string_utf16_buffer_t utf16Buffer = mla_string_to_utf16_buffer(baseString);
    assert_equal(utf16Buffer.charCount, (mla_uint32_t)3, "UTF-16 Mixed BMP: charCount should be 3");

    const mla_utf_16_char_t* utf16Buffer_data = mla_string_utf16_data(utf16Buffer);

    if (utf16Buffer_data != nullptr) {
        assert_equal(utf16Buffer_data[0], (mla_utf_16_char_t)0x20AC, "UTF-16 Mixed BMP: Euro");
        assert_equal(utf16Buffer_data[1], (mla_utf_16_char_t)0x03A9, "UTF-16 Mixed BMP: Omega");
        assert_equal(utf16Buffer_data[2], (mla_utf_16_char_t)0x00DF, "UTF-16 Mixed BMP: Eszett");
        assert_equal(utf16Buffer_data[3], (mla_utf_16_char_t)0x00, "UTF-16 Mixed BMP: terminator");
    } else {
        assert_fail("UTF-16 buffer is empty");
    }

    mla_string_t roundTrip = mla_string_from_utf16_buffer(utf16Buffer);
    assert_struct_equal(mla_string_t, baseString, roundTrip, "UTF-16 Mixed BMP round trip should match");

}

void ToUtf32AndFromUtf32_EmptyTest() {
    mla_string_t baseString = mla_string("");
    mla_string_utf32_buffer_t utf32Buffer = mla_string_to_utf32_buffer(baseString);
    assert_equal(utf32Buffer.charCount, (mla_uint32_t)0, "UTF-32 empty: charCount should be 0");

    const mla_utf_32_char_t* utf32Buffer_data = mla_string_utf32_data(utf32Buffer);

    assert_null(utf32Buffer_data, "UTF-32 empty: data should be null");
    mla_string_t roundTrip = mla_string_from_utf32_buffer(utf32Buffer);
    assert_true(mla_string_is_empty(roundTrip), "UTF-32 empty: round trip string should be empty");

}

void ToUtf32AndFromUtf32_AsciiTest() {
    mla_string_t baseString = mla_string("ABC");
    mla_string_utf32_buffer_t utf32Buffer = mla_string_to_utf32_buffer(baseString);
    assert_equal(utf32Buffer.charCount, (mla_uint32_t)3, "UTF-32 ASCII: charCount should be 3");

    const mla_utf_32_char_t* utf32Buffer_data = mla_string_utf32_data(utf32Buffer);

    if (utf32Buffer_data != nullptr) {
        assert_equal(utf32Buffer_data[0], (mla_utf_32_char_t)'A', "UTF-32 ASCII: data[0]=='A'");
        assert_equal(utf32Buffer_data[1], (mla_utf_32_char_t)'B', "UTF-32 ASCII: data[1]=='B'");
        assert_equal(utf32Buffer_data[2], (mla_utf_32_char_t)'C', "UTF-32 ASCII: data[2]=='C'");
        assert_equal(utf32Buffer_data[3], (mla_utf_32_char_t)0x00000000, "UTF-32 ASCII: terminator");
    } else {
        assert_fail("UTF-32 buffer is empty");
    }

    mla_string_t roundTrip = mla_string_from_utf32_buffer(utf32Buffer);
    assert_struct_equal(mla_string_t, baseString, roundTrip, "UTF-32 ASCII round trip should match");
}

void ToUtf32AndFromUtf32_MixedBmpTest() {
    mla_string_t baseString = mla_string("€Ωß");
    mla_string_utf32_buffer_t utf32Buffer = mla_string_to_utf32_buffer(baseString);
    assert_equal(utf32Buffer.charCount, (mla_uint32_t)3, "UTF-32 Mixed BMP: charCount should be 3");

    const mla_utf_32_char_t* utf32Buffer_data = mla_string_utf32_data(utf32Buffer);

    if (utf32Buffer_data != nullptr) {
        assert_equal(utf32Buffer_data[0], (mla_utf_32_char_t)0x000020AC, "UTF-32 Mixed BMP: Euro");
        assert_equal(utf32Buffer_data[1], (mla_utf_32_char_t)0x000003A9, "UTF-32 Mixed BMP: Omega");
        assert_equal(utf32Buffer_data[2], (mla_utf_32_char_t)0x000000DF, "UTF-32 Mixed BMP: Eszett");
        assert_equal(utf32Buffer_data[3], (mla_utf_32_char_t)0x00000000, "UTF-32 Mixed BMP: terminator");
    } else {
        assert_fail("UTF-32 buffer is empty");
    }

    mla_string_t roundTrip = mla_string_from_utf32_buffer(utf32Buffer);
    assert_struct_equal(mla_string_t, baseString, roundTrip, "UTF-32 Mixed BMP round trip should match");
}

void StringFromInt8Test() {
    // Test positive value
    mla_string_t str = mla_string_from_int8(42);
    assert_true(mla_string_equals(str, mla_string("42")), "int8(42) should equal '42'");

    // Test negative value
    str = mla_string_from_int8(-42);
    assert_true(mla_string_equals(str, mla_string("-42")), "int8(-42) should equal '-42'");

    // Test min value
    str = mla_string_from_int8(-128);
    assert_true(mla_string_equals(str, mla_string("-128")), "int8 min should equal '-128'");

    // Test max value
    str = mla_string_from_int8(127);
    assert_true(mla_string_equals(str, mla_string("127")), "int8 max should equal '127'");

    // Test zero
    str = mla_string_from_int8(0);
    assert_true(mla_string_equals(str, mla_string("0")), "int8(0) should equal '0'");
}

void StringFromUInt8Test() {
    // Test typical value
    mla_string_t str = mla_string_from_uint8(42);
    assert_true(mla_string_equals(str, mla_string("42")), "uint8(42) should equal '42'");

    // Test max value
    str = mla_string_from_uint8(255);
    assert_true(mla_string_equals(str, mla_string("255")), "uint8 max should equal '255'");

    // Test zero
    str = mla_string_from_uint8(0);
    assert_true(mla_string_equals(str, mla_string("0")), "uint8(0) should equal '0'");
}

void StringFromInt16Test() {
    // Test positive value
    mla_string_t str = mla_string_from_int16(1234);
    assert_true(mla_string_equals(str, mla_string("1234")), "int16(1234) should equal '1234'");

    // Test negative value
    str = mla_string_from_int16(-1234);
    assert_true(mla_string_equals(str, mla_string("-1234")), "int16(-1234) should equal '-1234'");

    // Test min value
    str = mla_string_from_int16(-32768);
    assert_true(mla_string_equals(str, mla_string("-32768")), "int16 min should equal '-32768'");

    // Test max value
    str = mla_string_from_int16(32767);
    assert_true(mla_string_equals(str, mla_string("32767")), "int16 max should equal '32767'");
}

void StringFromUInt16Test() {
    // Test typical value
    mla_string_t str = mla_string_from_uint16(1234);
    assert_true(mla_string_equals(str, mla_string("1234")), "uint16(1234) should equal '1234'");

    // Test max value
    str = mla_string_from_uint16(65535);
    assert_true(mla_string_equals(str, mla_string("65535")), "uint16 max should equal '65535'");

    // Test zero
    str = mla_string_from_uint16(0);
    assert_true(mla_string_equals(str, mla_string("0")), "uint16(0) should equal '0'");
}

void StringFromInt32Test() {
    // Test positive value
    mla_string_t str = mla_string_from_int32(123456);
    assert_true(mla_string_equals(str, mla_string("123456")), "int32(123456) should equal '123456'");

    // Test negative value
    str = mla_string_from_int32(-123456);
    assert_true(mla_string_equals(str, mla_string("-123456")), "int32(-123456) should equal '-123456'");

    // Test min value
    str = mla_string_from_int32(mla_int32_min);
    assert_true(mla_string_equals(str, mla_string("-2147483648")), "int32 min should equal '-2147483648'");

    // Test max value
    str = mla_string_from_int32(mla_int32_max);
    assert_true(mla_string_equals(str, mla_string("2147483647")), "int32 max should equal '2147483647'");
}

void StringFromUInt32Test() {
    // Test typical value
    mla_string_t str = mla_string_from_uint32(123456);
    assert_true(mla_string_equals(str, mla_string("123456")), "uint32(123456) should equal '123456'");

    // Test max value
    str = mla_string_from_uint32(mla_uint32_max);
    assert_true(mla_string_equals(str, mla_string("4294967295")), "uint32 max should equal '4294967295'");

    // Test zero
    str = mla_string_from_uint32(0);
    assert_true(mla_string_equals(str, mla_string("0")), "uint32(0) should equal '0'");

}

void StringFromInt64Test() {
    // Test positive value
    mla_string_t str = mla_string_from_int64(123456789012LL);
    assert_true(mla_string_equals(str, mla_string("123456789012")), "int64(123456789012) should equal '123456789012'");

    // Test negative value
    str = mla_string_from_int64(-123456789012LL);
    assert_true(mla_string_equals(str, mla_string("-123456789012")),
                "int64(-123456789012) should equal '-123456789012'");

    // Test min value
    str = mla_string_from_int64(mla_int64_min);
    assert_true(mla_string_equals(str, mla_string("-9223372036854775808")),
                "int64 min should equal '-9223372036854775808'");

    // Test max value
    str = mla_string_from_int64(mla_int64_max);
    assert_true(mla_string_equals(str, mla_string("9223372036854775807")),
                "int64 max should equal '9223372036854775807'");
}

void StringFromUInt64Test() {
    // Test typical value
    mla_string_t str = mla_string_from_uint64(123456789012ULL);
    assert_true(mla_string_equals(str, mla_string("123456789012")), "uint64(123456789012) should equal '123456789012'");

    // Test max value
    str = mla_string_from_uint64(mla_uint64_max);
    assert_true(mla_string_equals(str, mla_string("18446744073709551615")),
                "uint64 max should equal '18446744073709551615'");

    // Test zero
    str = mla_string_from_uint64(0);
    assert_true(mla_string_equals(str, mla_string("0")), "uint64(0) should equal '0'");
}

void StringFromFloatTest() {
    // Test with 2 decimal places (Short result - likely SSO)
    mla_string_t str = mla_string_from_float(3.14F, 2);
    assert_true(mla_string_equals(str, mla_string("3.14")), "float(3.14, 2) should equal '3.14'");

    // Test negative with 3 decimal places (Short result - likely SSO)
    str = mla_string_from_float(-123.456F, 3);
    assert_true(mla_string_equals(str, mla_string("-123.456")), "float(-123.456, 3) should equal '-123.456'");

    // Test zero
    str = mla_string_from_float(0.0F, 1);
    assert_true(mla_string_equals(str, mla_string("0.0")), "float(0.0, 1) should equal '0.0'");

    // Test with 0 decimal places
    str = mla_string_from_float(42.789F, 0);
    assert_true(mla_string_equals(str, mla_string("43")), "float(42.789, 0) should round to '43'");

    // Test long string (Heap allocation > 14 chars)
    // 333333.34375 is perfectly representable in float (exact binary fraction)
    str = mla_string_from_float(333333.34375F, 10);
    // Expected length is 17 chars: "333333.3437500000"
    assert_true(mla_string_equals(str, mla_string("333333.3437500000")), "float(333333.34375, 10) should equal '333333.3437500000'");
}

void StringFromDoubleTest() {
    // Test with 2 decimal places (Short result - likely SSO)
    mla_string_t str = mla_string_from_double(3.14159265358979, 2);
    assert_true(mla_string_equals(str, mla_string("3.14")), "double(pi, 2) should equal '3.14'");

    // Test negative with 5 decimal places (Short result - likely SSO)
    str = mla_string_from_double(-123.456789, 5);
    assert_true(mla_string_equals(str, mla_string("-123.45679")), "double(-123.456789, 5) should equal '-123.45679'");

    // Test zero
    str = mla_string_from_double(0.0, 3);
    assert_true(mla_string_equals(str, mla_string("0.000")), "double(0.0, 3) should equal '0.000'");

    // Test with 0 decimal places
    str = mla_string_from_double(42.789, 0);
    assert_true(mla_string_equals(str, mla_string("43")), "double(42.789, 0) should round to '43'");

    // Test long string (Heap allocation > 14 chars)
    // 123456.789012345 is safely representable in double
    str = mla_string_from_double(123456.789012345, 9);
    // Expected length 16 chars: "123456.789012345"
    assert_true(mla_string_equals(str, mla_string("123456.789012345")), "double(large, 9) should equal '123456.789012345'");
}

void StringFromBoolTest() {
    // Test true
    mla_string_t str = mla_string_from_bool(true);
    assert_true(mla_string_equals(str, mla_string("true")), "bool(true) should equal 'true'");

    // Test false
    str = mla_string_from_bool(false);
    assert_true(mla_string_equals(str, mla_string("false")), "bool(false) should equal 'false'");

}

void StringCompareTest() {
    // Test equal strings
    mla_string_t str1 = mla_string("Hello");
    mla_string_t str2 = mla_string("Hello");
    assert_equal(mla_string_compare(str1, str2), (mla_int32_t)0, "Equal strings should return 0");

    // Test first string less than second
    str1 = mla_string("Apple");
    str2 = mla_string("Banana");
    assert_true(mla_string_compare(str1, str2) < 0, "Apple should be less than Banana");

    // Test first string greater than second
    str1 = mla_string("Zebra");
    str2 = mla_string("Apple");
    assert_true(mla_string_compare(str1, str2) > 0, "Zebra should be greater than Apple");

    // Test shorter string as prefix
    str1 = mla_string("Hello");
    str2 = mla_string("Hello, World!");
    assert_true(mla_string_compare(str1, str2) < 0, "Shorter prefix should be less");

    // Test longer string with same prefix
    str1 = mla_string("Hello, World!");
    str2 = mla_string("Hello");
    assert_true(mla_string_compare(str1, str2) > 0, "Longer string should be greater");

    // Test empty strings
    str1 = mla_string("");
    str2 = mla_string("");
    assert_equal(mla_string_compare(str1, str2), (mla_int32_t)0, "Empty strings should be equal");

    // Test empty vs non-empty
    str1 = mla_string("");
    str2 = mla_string("A");
    assert_true(mla_string_compare(str1, str2) < 0, "Empty should be less than non-empty");
}

void StringCompareIgnoreCaseTest() {
    // Test equal strings ignoring case
    mla_string_t str1 = mla_string("Hello");
    mla_string_t str2 = mla_string("hello");
    assert_equal(mla_string_compare_ignore_case(str1, str2), (mla_int32_t)0,
                 "Case-insensitive comparison should be equal");

    // Test mixed case
    str1 = mla_string("HeLLo WoRLd");
    str2 = mla_string("hello world");
    assert_equal(mla_string_compare_ignore_case(str1, str2), (mla_int32_t)0, "Mixed case should be equal");

    // Test first string less than second (ignoring case)
    str1 = mla_string("APPLE");
    str2 = mla_string("banana");
    assert_true(mla_string_compare_ignore_case(str1, str2) < 0, "APPLE should be less than banana (case-insensitive)");

    // Test first string greater than second (ignoring case)
    str1 = mla_string("zebra");
    str2 = mla_string("APPLE");
    assert_true(mla_string_compare_ignore_case(str1, str2) > 0,
                "zebra should be greater than APPLE (case-insensitive)");

    // Test shorter string as prefix (ignoring case)
    str1 = mla_string("HELLO");
    str2 = mla_string("hello, world!");
    assert_true(mla_string_compare_ignore_case(str1, str2) < 0, "Shorter prefix should be less (case-insensitive)");

    // Test empty strings
    str1 = mla_string("");
    str2 = mla_string("");
    assert_equal(mla_string_compare_ignore_case(str1, str2), (mla_int32_t)0,
                 "Empty strings should be equal (case-insensitive)");

    // Test case where only case differs at start
    str1 = mla_string("abc");
    str2 = mla_string("ABC");
    assert_equal(mla_string_compare_ignore_case(str1, str2), (mla_int32_t)0, "Only case difference should be equal");

    // Test different characters after same prefix
    str1 = mla_string("Test1");
    str2 = mla_string("TEST2");
    assert_true(mla_string_compare_ignore_case(str1, str2) < 0, "Test1 should be less than TEST2 (case-insensitive)");
}

void StringToLowerTest() {
    // Test mixed case string
    mla_string_t str = mla_string("Hello, WORLD!");
    mla_string_t lower = mla_string_to_lower(str);
    assert_true(mla_string_equals(lower, mla_string("hello, world!")),
                "Mixed case should convert to lowercase");

    // Test already lowercase string (should not allocate)
    str = mla_string("hello");
    lower = mla_string_to_lower(str);
    assert_true(mla_string_equals(lower, mla_string("hello")),
                "Already lowercase should remain unchanged");

    // Test all uppercase string
    str = mla_string("HELLO");
    lower = mla_string_to_lower(str);
    assert_true(mla_string_equals(lower, mla_string("hello")),
                "All uppercase should convert to lowercase");

    // Test empty string
    str = mla_string("");
    lower = mla_string_to_lower(str);
    assert_true(mla_string_equals(lower, mla_string("")),
                "Empty string should remain empty");

    // Test string with numbers and symbols
    str = mla_string("Test123!@#");
    lower = mla_string_to_lower(str);
    assert_true(mla_string_equals(lower, mla_string("test123!@#")),
                "Numbers and symbols should remain unchanged");

    // Test with UTF-8 characters (Euro sign should remain unchanged)
    str = mla_string("€HELLO");
    lower = mla_string_to_lower(str);
    assert_true(mla_string_equals(lower, mla_string("€hello")),
                "UTF-8 characters should be handled correctly");
}

void StringToUpperTest() {
    // Test mixed case string
    mla_string_t str = mla_string("Hello, world!");
    mla_string_t upper = mla_string_to_upper(str);
    assert_true(mla_string_equals(upper, mla_string("HELLO, WORLD!")),
                "Mixed case should convert to uppercase");

    // Test already uppercase string (should not allocate)
    str = mla_string("HELLO");
    upper = mla_string_to_upper(str);
    assert_true(mla_string_equals(upper, mla_string("HELLO")),
                "Already uppercase should remain unchanged");

    // Test all lowercase string
    str = mla_string("hello");
    upper = mla_string_to_upper(str);
    assert_true(mla_string_equals(upper, mla_string("HELLO")),
                "All lowercase should convert to uppercase");

    // Test empty string
    str = mla_string("");
    upper = mla_string_to_upper(str);
    assert_true(mla_string_equals(upper, mla_string("")),
                "Empty string should remain empty");

    // Test string with numbers and symbols
    str = mla_string("test123!@#");
    upper = mla_string_to_upper(str);
    assert_true(mla_string_equals(upper, mla_string("TEST123!@#")),
                "Numbers and symbols should remain unchanged");

    // Test with UTF-8 characters (Euro sign should remain unchanged)
    str = mla_string("€hello");
    upper = mla_string_to_upper(str);
    assert_true(mla_string_equals(upper, mla_string("€HELLO")),
                "UTF-8 characters should be handled correctly");
}


void ReplaceTest() {
    // Test basic replacement
    mla_string_t str = mla_string("Hello, World!");
    mla_string_t result = mla_string_replace(str, mla_string("World"), mla_string("Universe"));
    assert_true(mla_string_equals(result, mla_string("Hello, Universe!")),
                "Basic replacement should work");


    // Test basic replacement SSO Strings
    str = mla_string("Hello");
    result = mla_string_replace(str, mla_string("ll"), mla_string("uu"));
    assert_true(mla_string_equals(result, mla_string("Heuuo")),
                "Basic replacement should work");
    assert_equal(mla_string_get_memory_layout(result), MLA_STRING_MEMORY_LAYOUT_EMBEDDED, "Result should be SSO string");

    // Test multiple occurrences
    str = mla_string("foo bar foo baz foo");
    result = mla_string_replace(str, mla_string("foo"), mla_string("qux"));
    assert_true(mla_string_equals(result, mla_string("qux bar qux baz qux")),
                "Multiple occurrences should be replaced");

    // Test substring not found
    str = mla_string("Hello, World!");
    result = mla_string_replace(str, mla_string("Mars"), mla_string("Venus"));
    assert_true(mla_string_equals(result, str),
                "No replacement when substring not found");

    // Test empty old substring
    str = mla_string("Hello");
    result = mla_string_replace(str, mla_string(""), mla_string("X"));
    assert_true(mla_string_equals(result, str),
                "Empty old substring should return original");

    // Test replacement with empty string (deletion)
    str = mla_string("Hello, World!");
    result = mla_string_replace(str, mla_string(", "), mla_string(""));
    assert_true(mla_string_equals(result, mla_string("HelloWorld!")),
                "Replacing with empty string should delete substring");

    // Test replacing entire string
    str = mla_string("Test");
    result = mla_string_replace(str, mla_string("Test"), mla_string("Success"));
    assert_true(mla_string_equals(result, mla_string("Success")),
                "Replacing entire string should work");

    // Test replacement at start
    str = mla_string("Start Middle End");
    result = mla_string_replace(str, mla_string("Start"), mla_string("Begin"));
    assert_true(mla_string_equals(result, mla_string("Begin Middle End")),
                "Replacement at start should work");

    // Test replacement at end
    str = mla_string("Start Middle End");
    result = mla_string_replace(str, mla_string("End"), mla_string("Finish"));
    assert_true(mla_string_equals(result, mla_string("Start Middle Finish")),
                "Replacement at end should work");

    // Test replacement with longer string
    str = mla_string("Hi");
    result = mla_string_replace(str, mla_string("Hi"), mla_string("Hello World"));
    assert_true(mla_string_equals(result, mla_string("Hello World")),
                "Replacement with longer string should work");

    // Test empty string
    str = mla_string("");
    result = mla_string_replace(str, mla_string("X"), mla_string("Y"));
    assert_true(mla_string_equals(result, mla_string("")),
                "Empty string should remain empty");
}

void TrimTest() {
    // Test trimming whitespace from both ends
    mla_string_t str = mla_string("  Hello, World!  ");
    mla_string_t result = mla_string_trim(str);
    assert_true(mla_string_equals(result, mla_string("Hello, World!")),
                "Trimming whitespace from both ends should work");

    // Test trimming only from the left
    str = mla_string("  Leading whitespace");
    result = mla_string_trim(str);
    assert_true(mla_string_equals(result, mla_string("Leading whitespace")),
                "Trimming leading whitespace should work");

    // Test trimming only from the right
    str = mla_string("Trailing whitespace  ");
    result = mla_string_trim(str);
    assert_true(mla_string_equals(result, mla_string("Trailing whitespace")),
                "Trimming trailing whitespace should work");

    // Test with no whitespace to trim
    str = mla_string("NoWhitespace");
    result = mla_string_trim(str);
    assert_true(mla_string_equals(result, str),
                "String with no whitespace should remain unchanged");

    // Test with only whitespace
    str = mla_string("   ");
    result = mla_string_trim(str);
    assert_true(mla_string_equals(result, mla_string("")),
                "String with only whitespace should become empty");

    // Test empty string
    str = mla_string("");
    result = mla_string_trim(str);
    assert_true(mla_string_equals(result, mla_string("")),
                "Empty string should remain empty");
}

void SplitTest() {
    // Test basic split
    mla_string_t str = mla_string("apple,banana,cherry");
    mla_array_list_t<mla_init_struct(mla_string_t)> result = mla_string_split(str, mla_string(","));

    if (mla_array_list_size(result) == 3) {
        assert_true(mla_string_equals(mla_array_list_get_unsafe(result, 0), mla_string("apple")), "First substring should be 'apple'");
        assert_true(mla_string_equals(mla_array_list_get_unsafe(result, 1), mla_string("banana")), "Second substring should be 'banana'");
        assert_true(mla_string_equals(mla_array_list_get_unsafe(result, 2), mla_string("cherry")), "Third substring should be 'cherry'");
    } else {
        assert_fail("Split did not produce expected number of substrings");
    }

    // Test with a different delimiter
    str = mla_string("one;two;three");
    result = mla_string_split(str, mla_string(";"));
    assert_equal(mla_array_list_size(result), (mla_size_t)3, "Split with semicolon should produce 3 substrings");


    // Test with multiple character delimiter
    str = mla_string("alpha--beta--gamma");
    result = mla_string_split(str, mla_string("--"));
    assert_equal(mla_array_list_size(result), (mla_size_t)3, "Split with multi-char delimiter should work");


    // Test with no delimiter present
    str = mla_string("no_delimiter_here");
    result = mla_string_split(str, mla_string(","));
    assert_equal(mla_array_list_size(result), (mla_size_t)1, "Split with no delimiter should return the original string");


    // Test with delimiter at the start
    str = mla_string(",start");
    result = mla_string_split(str, mla_string(","));

    if (mla_array_list_size(result) == 2) {
        assert_true(mla_string_equals(mla_array_list_get_unsafe(result, 0), mla_string("")),
                    "First substring should be empty");
        assert_true(mla_string_equals(mla_array_list_get_unsafe(result, 1), mla_string("start")),
                    "Second substring should be 'start'");
    } else {
        assert_fail("Split with leading delimiter did not produce expected number of substrings");
    }

    // Test with delimiter at the end
    str = mla_string("end,");
    result = mla_string_split(str, mla_string(","));

    if (mla_array_list_size(result) == 2) {
        assert_true(mla_string_equals(mla_array_list_get_unsafe(result, 0), mla_string("end")),
                    "First substring should be 'end'");
        assert_true(mla_string_equals(mla_array_list_get_unsafe(result, 1), mla_string("")),
                    "Second substring should be empty");
    } else {
        assert_fail("Split with trailing delimiter did not produce expected number of substrings");
    }

    // Test empty string
    str = mla_string("");
    result = mla_string_split(str, mla_string(","));
    assert_equal(mla_array_list_size(result), (mla_size_t)1, "Splitting an empty string should result in one empty string");

}

void JoinTest() {
    mla_array_list_t<mla_init_struct(mla_string_t)> parts = mla_array_list<mla_init_struct(mla_string_t)>(3);
    mla_array_list_add(parts, mla_string("apple"));
    mla_array_list_add(parts, mla_string("banana"));
    mla_array_list_add(parts, mla_string("cherry"));

    mla_string_t joined = mla_string_join(parts, mla_string(","));
    assert_true(mla_string_equals(joined, mla_string("apple,banana,cherry")), "Join should concatenate with delimiter");

    mla_array_list_t<mla_init_struct(mla_string_t)> singlePart = mla_array_list<mla_init_struct(mla_string_t)>(1);
    mla_array_list_add(singlePart, mla_string("only"));
    mla_string_t singleJoin = mla_string_join(singlePart, mla_string(","));
    assert_true(mla_string_equals(singleJoin, mla_string("only")), "Join with one element should return the element unchanged");

    mla_array_list_t<mla_init_struct(mla_string_t)> emptyParts = mla_array_list<mla_init_struct(mla_string_t)>(0);
    mla_string_t emptyJoin = mla_string_join(emptyParts, mla_string(","));
    assert_true(mla_string_is_empty(emptyJoin), "Join on empty list should return empty string");
}

void RepeatTest() {
    // Test basic repetition
    mla_string_t str = mla_string("ab");
    mla_string_t repeated = mla_string_repeat(str, 3);
    assert_true(mla_string_equals(repeated, mla_string("ababab")), "Repeating 'ab' 3 times should equal 'ababab'");

    // Test repeating 0 times
    repeated = mla_string_repeat(str, 0);
    assert_true(mla_string_is_empty(repeated), "Repeating 0 times should result in empty string");

    // Test repeating 1 time (should basically be a copy)
    repeated = mla_string_repeat(str, 1);
    assert_true(mla_string_equals(repeated, str), "Repeating 1 time should equal original string");


    // Test repeating empty string
    str = mla_string_empty();
    repeated = mla_string_repeat(str, 5);
    assert_true(mla_string_is_empty(repeated), "Repeating empty string should result in empty string");

    // Test repeating multi-byte string
    str = mla_string("€");
    repeated = mla_string_repeat(str, 2);
    assert_true(mla_string_equals(repeated, mla_string("€€")), "Repeating '€' 2 times should equal '€€'");
}


void StringCopyTest() {
    // Test copy from char* (Short string -> SSO)
    mla_string_t copy1 = mla_string_copy("hello", 5);
    assert_true(mla_string_equals(copy1, mla_string("hello")), "Copy from char* failed");

    // Test copy from char* (Long string -> Heap)
    mla_string_t copy_long = mla_string_copy("this is a very long string that will definitely be on the heap", 62);
    assert_true(mla_string_equals(copy_long, mla_string("this is a very long string that will definitely be on the heap")),
                "Copy from long char* failed");

    // Test copy from mla_string_t (Short string -> SSO)
    mla_string_t orig = mla_string("world");
    mla_string_t copy2 = mla_string_copy(orig);
    assert_true(mla_string_equals(copy2, orig), "Copy from mla_string_t failed");

}

void StringContainsIgnoreCaseTest() {
    mla_string_t str = mla_string("Hello, World!");
    assert_true(mla_string_contains_ignore_case(str, mla_string("hello")), "Contains ignore case failed");
    assert_true(mla_string_contains_ignore_case(str, mla_string("WORLD")), "Contains ignore case failed");
    assert_false(mla_string_contains_ignore_case(str, mla_string("Mars")), "Should not contain Mars");
}

void StringStartsWithIgnoreCaseTest() {
    mla_string_t str = mla_string("Hello, World!");
    assert_true(mla_string_starts_with_ignore_case(str, mla_string("hello")), "Starts with ignore case failed");
    assert_false(mla_string_starts_with_ignore_case(str, mla_string("world")), "Should not start with world");
}

void StringEndsWithIgnoreCaseTest() {
    mla_string_t str = mla_string("Hello, World!");
    assert_true(mla_string_ends_with_ignore_case(str, mla_string("WORLD!")), "Ends with ignore case failed");
    assert_false(mla_string_ends_with_ignore_case(str, mla_string("hello")), "Should not end with hello");
}


void StringFromSizeTest() {
    mla_string_t str = mla_string_from_size(12345);
    assert_true(mla_string_equals(str, mla_string("12345")), "From size failed");

}

void StringFromHexTest() {
    mla_string_t str8 = mla_string_from_uint8_hex(0xAB);
    assert_true(mla_string_equals(str8, mla_string("0xAB")), "uint8 hex failed");

    mla_string_t str16 = mla_string_from_uint16_hex(0xABCD);
    assert_true(mla_string_equals(str16, mla_string("0xABCD")), "uint16 hex failed");

    mla_string_t str32 = mla_string_from_uint32_hex(0x12345678);
    assert_true(mla_string_equals(str32, mla_string("0x12345678")), "uint32 hex failed");

    mla_string_t str64 = mla_string_from_uint64_hex(0x1234567890ABCDEFULL);
    assert_true(mla_string_equals(str64, mla_string("0x1234567890ABCDEF")), "uint64 hex failed");

}

void StringFromHexShortTest() {
    mla_string_t str8 = mla_string_from_uint8_hex_short(0xAB);
    assert_true(mla_string_equals(str8, mla_string("AB")), "uint8 hex short failed");


    mla_string_t str16 = mla_string_from_uint16_hex_short(0xABCD);
    assert_true(mla_string_equals(str16, mla_string("ABCD")), "uint16 hex short failed");

    mla_string_t str32 = mla_string_from_uint32_hex_short(0x12345678);
    assert_true(mla_string_equals(str32, mla_string("12345678")), "uint32 hex short failed");

    mla_string_t str64 = mla_string_from_uint64_hex_short(0x1234567890ABCDEFULL);
    assert_true(mla_string_equals(str64, mla_string("1234567890ABCDEF")), "uint64 hex short failed");

}

void StringHashTest() {
    mla_string_t str = mla_string("hello");
    mla_size_t h = mla_string_hash_t::hash(str);
    assert_true(h != 0, "Hash should not be zero");
}

void StringHashRawApiTest() {
    mla_string_t str = mla_string("hello");
    mla_size_t rawHash = mla_string_hash(mla_string_data(str), mla_string_length(str));
    mla_size_t wrappedHash = mla_string_hash_t::hash(str);

    assert_true(rawHash != 0, "Raw hash should not be zero");
    assert_equal(rawHash, wrappedHash, "Raw hash API should match hash wrapper");
}

void StringConstTest() {
    mla_string_t str = mla_string_const("hello");
    assert_true(mla_string_equals(str, mla_string("hello")), "String const failed");
    assert_equal(mla_string_length(str), (mla_size_t)5, "String const length failed");
}

void StringEqualsConstTest() {
    mla_string_t str = mla_string("hello");
    assert_true(mla_string_equals_const(str, "hello"), "String equals const failed");
    assert_false(mla_string_equals_const(str, "world"), "String equals const should fail for different strings");
}


void StringEqualsEmbeddedLayoutTest() {
    // Test embedded strings (small strings that fit in the embedded storage)
    mla_string_t embedded1 = mla_string("test");
    mla_string_t embedded2 = mla_string("test");
    mla_string_t embedded3 = mla_string("differ");

    assert_equal(mla_string_get_memory_layout(embedded1), MLA_STRING_MEMORY_LAYOUT_EMBEDDED, "String should be embedded");
    assert_equal(mla_string_get_memory_layout(embedded2), MLA_STRING_MEMORY_LAYOUT_EMBEDDED, "String should be embedded");

    assert_true(mla_string_equals(embedded1, embedded2), "Two identical embedded strings should be equal");
    assert_false(mla_string_equals(embedded1, embedded3), "Two different embedded strings should not be equal");
}

void StringEqualsBufferLayoutTest() {
    // Test buffer-based strings (medium strings that require heap allocation)
    mla_string_t buffer1 = mla_string(mla_platform_pointer_to_managed_pointer("This is a longer test string for buffer layout"), 46);
    mla_string_t buffer2 = mla_string(mla_platform_pointer_to_managed_pointer("This is a longer test string for buffer layout"), 46);
    mla_string_t buffer3 = mla_string(mla_platform_pointer_to_managed_pointer("This is a different buffer string entirely"), 42);

    assert_equal(mla_string_get_memory_layout(buffer1), MLA_STRING_MEMORY_LAYOUT_BUFFER, "String should be buffer layout");
    assert_equal(mla_string_get_memory_layout(buffer2), MLA_STRING_MEMORY_LAYOUT_BUFFER, "String should be buffer layout");

    assert_true(mla_string_equals(buffer1, buffer2), "Two identical buffer strings should be equal");
    assert_false(mla_string_equals(buffer1, buffer3), "Two different buffer strings should not be equal");
}

void StringEqualsCStringLayoutTest() {
    // Test C-style strings (null-terminated format)
    mla_char_t cstr1[50] = "C-style string test";
    mla_char_t cstr2[50] = "C-style string test";
    mla_char_t cstr3[50] = "Different C string";

    mla_string_t cstring1 = mla_string(mla_platform_pointer_to_managed_pointer(cstr1));
    mla_string_t cstring2 = mla_string(mla_platform_pointer_to_managed_pointer(cstr2));
    mla_string_t cstring3 = mla_string(mla_platform_pointer_to_managed_pointer(cstr3));

    assert_equal(mla_string_get_memory_layout(cstring1), MLA_STRING_MEMORY_LAYOUT_C_STRING, "String should be C-string layout");
    assert_equal(mla_string_get_memory_layout(cstring2), MLA_STRING_MEMORY_LAYOUT_C_STRING, "String should be C-string layout");

    assert_true(mla_string_equals(cstring1, cstring2), "Two identical C-strings should be equal");
    assert_false(mla_string_equals(cstring1, cstring3), "Two different C-strings should not be equal");
}

void StringEqualsStrstrOptimizationTest() {
    // Test the strstr optimization path: both strings are C-style strings
    // When both strings are C-style strings, the equals function uses strstr for comparison
    mla_char_t cstr1[50] = "Hello World C-style";
    mla_char_t cstr2[50] = "Hello World C-style";

    mla_string_t cstring1 = mla_string(mla_platform_pointer_to_managed_pointer(cstr1));
    mla_string_t cstring2 = mla_string(mla_platform_pointer_to_managed_pointer(cstr2));

    assert_true(mla_string_equals(cstring1, cstring2), "C-strings should be equal using strstr optimization");
}

void StringEqualsMixedLayoutTest() {
    // Test equals with mixed memory layouts
    mla_string_t embedded = mla_string("test");
    mla_string_t buffer = mla_string_const("test");

    mla_char_t cstr[50] = "test";
    mla_string_t cstring = mla_string(mla_platform_pointer_to_managed_pointer(cstr));

    assert_true(mla_string_equals(embedded, buffer), "Embedded and buffer with same content should be equal");
    assert_true(mla_string_equals(buffer, cstring), "Buffer and C-string with same content should be equal");
    assert_true(mla_string_equals(embedded, cstring), "Embedded and C-string with same content should be equal");
}

void StringEqualsLengthMismatchTest() {
    // Test that equals returns false for strings with different lengths
    mla_string_t str1 = mla_string("short");
    mla_string_t str2 = mla_string("much longer string");

    assert_false(mla_string_equals(str1, str2), "Strings with different lengths should not be equal");
}

void StringEqualsEmptyStringsTest() {
    // Test equals with empty strings
    mla_string_t empty1 = mla_string("");
    mla_string_t empty2 = mla_string("");
    mla_string_t nonempty = mla_string("not empty");

    assert_true(mla_string_equals(empty1, empty2), "Two empty strings should be equal");
    assert_false(mla_string_equals(empty1, nonempty), "Empty and non-empty strings should not be equal");
}

void StringEqualsSamePointerTest() {
    // NOLINTBEGIN(performance-unnecessary-copy-initialization)

    // Test equals when both strings point to the same data
    mla_string_t str1 = mla_string("pointer test");
    mla_string_t str2 = str1;

    assert_true(mla_string_equals(str1, str2), "Same pointer strings should be equal");

    // NOLINTEND(performance-unnecessary-copy-initialization)
}

void StringEqualsMaxLoopCheckBelowThresholdTest() {
    // Test equals with strings below the max loop check threshold
    mla_string_t str1 = mla_string("below threshold");
    mla_string_t str2 = mla_string("below threshold");
    mla_string_t differ_at_start = mla_string("xbelow threshold");

    assert_true(mla_string_equals(str1, str2), "Equal strings below threshold should be equal");
    assert_false(mla_string_equals(str1, differ_at_start), "Different strings should not be equal");
}

void StringEqualsMaxLoopCheckAboveThresholdTest() {
    // Test equals with very long strings that exceed the max loop check threshold
    mla_string_t long_str1 = mla_string_concat(
        mla_string("very long string with lots of content"),
        mla_string(" and even more content"),
        mla_string(" to exceed threshold")
    );

    mla_string_t long_str2 = mla_string_concat(
        mla_string("very long string with lots of content"),
        mla_string(" and even more content"),
        mla_string(" to exceed threshold")
    );

    mla_string_t long_str_differ = mla_string_concat(
        mla_string("very long string with lots of content"),
        mla_string(" and MORE content"),
        mla_string(" to exceed threshold")
    );

    assert_true(mla_string_equals(long_str1, long_str2), "Identical long strings should be equal");
    assert_false(mla_string_equals(long_str1, long_str_differ), "Different long strings should not be equal");

}

void StringEqualsSubstringLayoutTest() {
    // Test equals with substring (view-based) memory layout
    mla_string_t original = mla_string_concat(mla_string("prefix:"), mla_string("actual content"));
    mla_string_t substring = mla_string_substr(original, 7, 14);
    mla_string_t target = mla_string("actual content");

    assert_equal(mla_string_get_memory_layout(substring), MLA_STRING_MEMORY_LAYOUT_EMBEDDED,
                 "Substring should have SUB_STRING layout");
    assert_true(mla_string_equals(substring, target), "Substring and regular string with same content should be equal");


}

void StringEqualsNullPointerDataTest() {
    // Test equals with empty/null pointer strings
    mla_string_t str1 = mla_string("");
    mla_string_t str2 = mla_string("");

    // Empty strings should still be equal
    assert_true(mla_string_equals(str1, str2), "Empty strings should be equal");
}


void RegisterStringTests(mla_test_executor_t &p_TestExecutor) {
    mla_test_t test = mla_test("SizeOf", test_category, SizeOfTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ContainsCLayout", test_category, ContainsCLayoutTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ContainsBufferLayout", test_category, ContainsBufferLayoutTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ContainsEmbeddedLayout", test_category, ContainsEmbeddedLayoutTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StartsWith", test_category, StartsWithTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Length", test_category, LengthTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("EndsWith", test_category, EndsWithTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Equals", test_category, EqualsTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("EqualsIgnoreCase", test_category, EqualsIgnoreCaseTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("IndexOf", test_category, IndexOfCLayoutTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("IndexOfBufferLayout", test_category, IndexOfBufferLayoutTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("IndexOfEmbeddedLayout", test_category, IndexOfEmbeddedLayoutTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("IndexOfWithStartCLayout", test_category, IndexOfWithStartCLayoutTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("IndexOfWithStartBufferLayout", test_category, IndexOfWithStartBufferLayoutTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("IndexOfWithStartEmbeddedLayout", test_category, IndexOfWithStartEmbeddedLayoutTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("IndexOfWithStartFastPath", test_category, IndexOfWithStartFastPathTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("IndexOfFastPath", test_category, IndexOfFastPathTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("LastIndexOf", test_category, LastIndexOfTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ToCString", test_category, ToCStringTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ToCStringFromBuffer", test_category, ToCStringFromBufferTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ToCStringFromBuffer_No_Force_Copy", test_category, ToCStringFromBuffer_No_Force_CopyTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("CStringDataAccessor", test_category, CStringDataAccessorTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("FromCStringWithLength", test_category, StringFromCStringWithLengthTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("AccessChar", test_category, AccessCharTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Concat", test_category, ConcatTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("AutoMemoryManagement", test_category, AutoMemoryManagementTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("SubString", test_category, SubStringTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("SubStringEmbedded", test_category, SubStringEmbeddedTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("MultiByteCharHandling", test_category, MultiByteCharHandlingTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ToUtf16AndFromUtf16", test_category, ToUtf16AndFromUtf16Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ToUtf32AndFromUtf32", test_category, ToUtf32AndFromUtf32Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ToUtf16AndFromUtf16_Empty", test_category, ToUtf16AndFromUtf16_EmptyTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ToUtf16AndFromUtf16_Ascii", test_category, ToUtf16AndFromUtf16_AsciiTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ToUtf16AndFromUtf16_MixedBmp", test_category, ToUtf16AndFromUtf16_MixedBmpTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ToUtf32AndFromUtf32_Empty", test_category, ToUtf32AndFromUtf32_EmptyTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ToUtf32AndFromUtf32_Ascii", test_category, ToUtf32AndFromUtf32_AsciiTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ToUtf32AndFromUtf32_MixedBmp", test_category, ToUtf32AndFromUtf32_MixedBmpTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("FromInt8", test_category, StringFromInt8Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("FromUInt8", test_category, StringFromUInt8Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("FromInt16", test_category, StringFromInt16Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("FromUInt16", test_category, StringFromUInt16Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("FromInt32", test_category, StringFromInt32Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("FromUInt32", test_category, StringFromUInt32Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("FromInt64", test_category, StringFromInt64Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("FromUInt64", test_category, StringFromUInt64Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("FromFloat", test_category, StringFromFloatTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("FromDouble", test_category, StringFromDoubleTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("FromBool", test_category, StringFromBoolTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Compare", test_category, StringCompareTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("CompareIgnoreCase", test_category, StringCompareIgnoreCaseTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ToLower", test_category, StringToLowerTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ToUpper", test_category, StringToUpperTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Replace", test_category, ReplaceTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Trim", test_category, TrimTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Split", test_category, SplitTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Join", test_category, JoinTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Repeat", test_category, RepeatTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Copy", test_category, StringCopyTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ContainsIgnoreCase", test_category, StringContainsIgnoreCaseTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StartsWithIgnoreCase", test_category, StringStartsWithIgnoreCaseTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("EndsWithIgnoreCase", test_category, StringEndsWithIgnoreCaseTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("FromSize", test_category, StringFromSizeTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("FromHex", test_category, StringFromHexTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("FromHexShort", test_category, StringFromHexShortTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Hash", test_category, StringHashTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("HashRawApi", test_category, StringHashRawApiTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Const", test_category, StringConstTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("EqualsConst", test_category, StringEqualsConstTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    // New tests for mla_string_equals with different memory layouts and optimization paths
    ///////////////////////////////////////////////////////////////
    test = mla_test("EqualsEmbeddedLayout", test_category, StringEqualsEmbeddedLayoutTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("EqualsBufferLayout", test_category, StringEqualsBufferLayoutTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("EqualsCStringLayout", test_category, StringEqualsCStringLayoutTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("EqualsStrstrOptimization", test_category, StringEqualsStrstrOptimizationTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("EqualsMixedLayout", test_category, StringEqualsMixedLayoutTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("EqualsLengthMismatch", test_category, StringEqualsLengthMismatchTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("EqualsEmptyStrings", test_category, StringEqualsEmptyStringsTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("EqualsSamePointer", test_category, StringEqualsSamePointerTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("EqualsMaxLoopCheckBelowThreshold", test_category, StringEqualsMaxLoopCheckBelowThresholdTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("EqualsMaxLoopCheckAboveThreshold", test_category, StringEqualsMaxLoopCheckAboveThresholdTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("EqualsSubstringLayout", test_category, StringEqualsSubstringLayoutTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("EqualsNullPointerData", test_category, StringEqualsNullPointerDataTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}




void StringConcatBenchmark() {
    mla_string_t str1 = mla_string("Hello, ");
    mla_string_t str2 = mla_string("World!");
    mla_string_t str3 = mla_string(" This is a test of concatenation.");

    mla_string_t result = mla_string_concat(str1, str2, str3);

    mla_size_t length = mla_string_length(result);
    (void) length; // Prevent unused variable warning

    result = mla_string_empty();
}

void StringConcatEmbeddedBenchmark() {
    mla_string_t str1 = mla_string("H ");
    mla_string_t str2 = mla_string("W ");
    mla_string_t str3 = mla_string("!");

    mla_string_t result = mla_string_concat(str1, str2, str3);

    mla_size_t length = mla_string_length(result);
    (void) length; // Prevent unused variable warning

    result = mla_string_empty();
}

void StringFromInt8Benchmark() {
    mla_int8_t value = 42;
    mla_string_t str = mla_string_from_int8(value);
    str = mla_string_empty();
}

void StringFromInt16Benchmark() {
    mla_int16_t value = 1234;
    mla_string_t str = mla_string_from_int16(value);
    str = mla_string_empty();
}

void StringFromInt32Benchmark() {
    mla_int32_t value = 123456;
    mla_string_t str = mla_string_from_int32(value);
    str = mla_string_empty();
}

void StringFromInt64Benchmark() {
    mla_int64_t value = 123456789012LL;
    mla_string_t str = mla_string_from_int64(value);
    str = mla_string_empty();
}

void StringFromUInt8Benchmark() {
    mla_uint8_t value = 42;
    mla_string_t str = mla_string_from_uint8(value);
    str = mla_string_empty();
}

void StringFromUInt16Benchmark() {
    mla_uint16_t value = 1234;
    mla_string_t str = mla_string_from_uint16(value);
    str = mla_string_empty();
}

void StringFromUInt32Benchmark() {
    mla_uint32_t value = 123456;
    mla_string_t str = mla_string_from_uint32(value);
    str = mla_string_empty();
}

void StringFromUInt64Benchmark() {
    mla_uint64_t value = 123456789012ULL;
    mla_string_t str = mla_string_from_uint64(value);
    str = mla_string_empty();
}

void StringFromFloatBenchmark() {
    mla_float_t value = 9995555553.14159F;
    mla_string_t str = mla_string_from_float(value, 5);
    str = mla_string_empty();
}

void StringFromFloatEmbeddedBenchmark() {
    mla_float_t value = 3.14159F;
    mla_string_t str = mla_string_from_float(value, 5);
    str = mla_string_empty();
}


void StringFromDoubleBenchmark() {
    mla_double_t value = 8888883.14159265358979;
    mla_string_t str = mla_string_from_double(value, 10);
    str = mla_string_empty();
}

void StringFromDoubleEmbeddedBenchmark() {
    mla_double_t value = 3.141592;
    mla_string_t str = mla_string_from_double(value, 6);
    str = mla_string_empty();
}

void StringFromBoolBenchmark() {
    mla_bool_t value = true;
    mla_string_t str = mla_string_from_bool(value);
    str = mla_string_empty();
}

void StringFromUtf16BufferBenchmark() {
    mla_utf_16_char_t data[] = {0x20AC, ' ', '1', '0', '0', 0x00};
    mla_string_utf16_buffer_t buffer = {mla_platform_pointer_to_managed_pointer(data), 5};
    mla_string_t str = mla_string_from_utf16_buffer(buffer);
    str = mla_string_empty();
}

void StringFromUtf32BufferBenchmark() {
    mla_utf_32_char_t data[] = {0x000020AC, ' ', '1', '0', '0', 0x00000000};
    mla_string_utf32_buffer_t buffer = {mla_platform_pointer_to_managed_pointer(data), 5};
    mla_string_t str = mla_string_from_utf32_buffer(buffer);
    str = mla_string_empty();
}


void StringContains_Buffer_LayoutBenchmark() {
    const mla_test_char_t *data = "Hello, World! This is a test string for benchmarking.";
    mla_test_int32_t length = mla_s_cast<mla_test_int32_t>(mla_test_strlen(data)); // Length of the string

    mla_string_t str = mla_string(mla_platform_pointer_to_managed_pointer(data), length);
    mla_string_t subString = mla_string("for");

    if (!mla_string_contains(str, subString)) {
        static_assert(true, "Substring not found in MlaStringContainsBenchmark");
    }
}

void StringContains_C_LayoutBenchmark() {
    mla_string_t str = mla_string("Hello, World! This is a test string for benchmarking.");
    mla_string_t subString = mla_string("for");


    if (!mla_string_contains(str, subString)) {
        static_assert(true, "Substring not found in MlaStringContainsBenchmark");
    }
}

void StringIndexOf_Buffer_LayoutBenchmark() {
    const mla_test_char_t *data = "Hello, World! This is a test string for benchmarking.";
    mla_test_int32_t length = mla_s_cast<mla_test_int32_t>(mla_test_strlen(data)); // Length of the string

    mla_string_t str = mla_string(mla_platform_pointer_to_managed_pointer(data), length);
    mla_string_t subString = mla_string("for");

    mla_test_int32_t index = mla_string_index_of(str, subString);
    if (index != 38) {
        static_assert(true, "Index of substring not found in MlaStringIndexOfBenchmark");
    }
}

void StringIndexOfShort_Buffer_LayoutBenchmark() {
    const mla_test_char_t *data = "Hello, World! This is a test string for benchmarking.";
    mla_test_int32_t length = mla_s_cast<mla_test_int32_t>(mla_test_strlen(data)); // Length of the string

    mla_string_t str = mla_string(mla_platform_pointer_to_managed_pointer(data), length);
    mla_string_t subString = mla_string("is");

    mla_test_int32_t index = mla_string_index_of(str, subString);
    if (index != 38) {
        static_assert(true, "Index of substring not found in MlaStringIndexOfBenchmark");
    }
}

void StringIndexOf_C_LayoutBenchmark() {
    mla_string_t str = mla_string_const("Hello, World! This is a test string for benchmarking.");
    mla_string_t subString = mla_string_const("for");

    mla_test_int32_t index = mla_string_index_of(str, subString);
    if (index != 38) {
        static_assert(true, "Index of substring not found in MlaStringIndexOfBenchmark");
    }
}

void StringEquals_C_LayoutBenchmark() {
    mla_char_t leftData[64] = "Hello, World! This is a test string for benchmarking.";
    mla_char_t rightData[64] = "Hello, World! This is a test string for benchmarking.";

    mla_string_t left = mla_string(mla_platform_pointer_to_managed_pointer(leftData));
    mla_string_t right = mla_string(mla_platform_pointer_to_managed_pointer(rightData));

    mla_test_bench_sink(mla_string_equals(left, right));
}

void StringEquals_Buffer_LayoutBenchmark() {

    mla_char_t leftData[64] = "Hello, World! This is a test string for benchmarking.";
    mla_char_t rightData[64] = "Hello, World! This is a test string for benchmarking.";

    mla_string_t left = mla_string(mla_platform_pointer_to_managed_pointer(leftData), 53);
    mla_string_t right = mla_string(mla_platform_pointer_to_managed_pointer(rightData), 53);

    mla_test_bench_sink(mla_string_equals(left, right));
}

void StringEquals_BufferShort_LayoutBenchmark() {
    mla_string_t left = mla_string(mla_platform_pointer_to_managed_pointer("Hello, World! This is a test"), 28);
    mla_string_t right = mla_string_const("Hello, World! This is a test");

    mla_test_bench_sink(mla_string_equals(left, right));
}

void StringEquals_Embedded_LayoutBenchmark() {
    mla_string_t left = mla_string("Hello, World");
    mla_string_t right = mla_string("Hello, World");

    mla_test_bench_sink(mla_string_equals(left, right));
}

void String_to_C_LayoutBenchmark() {

    mla_string_t str = mla_string("Hello, World! This is a test string for benchmarking.");
    mla_c_string_t cStr = mla_string_to_cString(str);
    mla_test_bench_sink(cStr);
}

void String_to_buffer_LayoutBenchmark() {
    mla_string_t str = mla_string(mla_platform_pointer_to_managed_pointer("Hello, World! This is a test string for benchmarking."), 53);
    mla_c_string_t cStr = mla_string_to_cString(str);
    mla_test_bench_sink(cStr);

}

void RefCountBenchmark() {
    mla_pointer_t data = mla_create_char_array(1);

    mla_string_t str1 = mla_string(data, 1);
    mla_string_t str2 = str1;
    mla_string_t str3 = str2;
    mla_string_t str4 = str3;
    mla_string_t str5 = str4;
    mla_string_t str6 = str5;
    mla_string_t str7 = str6;
    mla_string_t str8 = str7;
    mla_string_t str9 = str8;
    mla_string_t str10 = str9;
    mla_string_t str11 = str10;
    mla_string_t str12 = str11;
    mla_string_t str13 = str12;
    mla_string_t str14 = str13;
    mla_string_t str15 = str14;
    mla_string_t str16 = str15;
    mla_string_t str17 = str16;
    mla_string_t str18 = str17;
    mla_string_t str19 = str18;
    mla_string_t str20 = str19;
    mla_string_t str21 = str20;
    mla_string_t str22 = str21;
    mla_string_t str23 = str22;
    mla_string_t str24 = str23;
    mla_string_t str25 = str24;
    mla_string_t str26 = str25;
    mla_string_t str27 = str26;
    mla_string_t str28 = str27;
    mla_string_t str29 = str28;
    mla_string_t str30 = str29;
    str1 = mla_string_empty();
    str2 = mla_string_empty();
    str3 = mla_string_empty();
    str4 = mla_string_empty();
    str5 = mla_string_empty();
    str6 = mla_string_empty();
    str7 = mla_string_empty();
    str8 = mla_string_empty();
    str9 = mla_string_empty();
    str10 = mla_string_empty();
    str11 = mla_string_empty();
    str12 = mla_string_empty();
    str13 = mla_string_empty();
    str14 = mla_string_empty();
    str15 = mla_string_empty();
    str16 = mla_string_empty();
    str17 = mla_string_empty();
    str18 = mla_string_empty();
    str19 = mla_string_empty();
    str20 = mla_string_empty();
    str21 = mla_string_empty();
    str22 = mla_string_empty();
    str23 = mla_string_empty();
    str24 = mla_string_empty();
    str25 = mla_string_empty();
    str26 = mla_string_empty();
    str27 = mla_string_empty();
    str28 = mla_string_empty();
    str29 = mla_string_empty();
    str30 = mla_string_empty();
}

// Benchmarks for mla_string_substr
void SubstringEmbeddedBenchmark() {

    mla_string_t str = mla_string("Hello");
    mla_string_t result = mla_string_substr(str, 0, 3);
    mla_test_bench_sink(mla_string_length(result));
}

void SubstringBufferShortBenchmark() {

    mla_string_t str = mla_string(mla_platform_pointer_to_managed_pointer("Hello World!"), 12);
    mla_string_t result = mla_string_substr(str, 6, 5);
    mla_test_bench_sink(mla_string_length(result));
}

void SubstringBufferMediumBenchmark() {
    mla_string_t str = mla_string(mla_platform_pointer_to_managed_pointer("This is a medium length string for benchmarking substring operations"), 68);
    mla_string_t result = mla_string_substr(str, 10, 20);
    mla_test_bench_sink(mla_string_length(result));
}

void SubstringCStringBenchmark() {

    mla_char_t cstr1[100] = "This is a very long C-style string used for benchmarking the strstr optimization path";

    mla_string_t left = mla_string(mla_platform_pointer_to_managed_pointer(cstr1), 85);

    mla_string_t result = mla_string_substr(left, 10, 20);
    mla_test_bench_sink(mla_string_length(result));

}


void RegisterStringBenchmarks(mla_benchmark_executor_t &p_BenchmarkExecutor) {
    // Concat Benchmarks
    ////////////////////////////////////////////

    mla_benchmark_t benchmark = mla_benchmark("ConcatEmbedded", benchmark_category, StringConcatEmbeddedBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("Concat", benchmark_category, StringConcatBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    // From Benchmarks
    ////////////////////////////////////////////
    benchmark = mla_benchmark("FromInt8", benchmark_category, StringFromInt8Benchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("FromInt16", benchmark_category, StringFromInt16Benchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("FromInt32", benchmark_category, StringFromInt32Benchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("FromInt64", benchmark_category, StringFromInt64Benchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("FromUInt8", benchmark_category, StringFromUInt8Benchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("FromUInt16", benchmark_category, StringFromUInt16Benchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("FromUInt32", benchmark_category, StringFromUInt32Benchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("FromUInt64", benchmark_category, StringFromUInt64Benchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("FromFloat", benchmark_category, StringFromFloatBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("FromDouble", benchmark_category, StringFromDoubleBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("FromFloatEmbedded", benchmark_category, StringFromFloatEmbeddedBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("FromDoubleEmbedded", benchmark_category, StringFromDoubleEmbeddedBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("FromBool", benchmark_category, StringFromBoolBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("FromUtf16Buffer", benchmark_category, StringFromUtf16BufferBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("FromUtf32Buffer", benchmark_category, StringFromUtf32BufferBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    // Ref Count Benchmarks
    ////////////////////////////////////////////
    benchmark = mla_benchmark("RefCount", benchmark_category, RefCountBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    // Contains Benchmarks
    ////////////////////////////////////////////
    benchmark = mla_benchmark("Contains_Buffer", benchmark_category, StringContains_Buffer_LayoutBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
    benchmark = mla_benchmark("Contains_CStr", benchmark_category, StringContains_C_LayoutBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    // IndexOf Benchmarks
    ////////////////////////////////////////////
    benchmark = mla_benchmark("IndexOf_Buffer", benchmark_category, StringIndexOf_Buffer_LayoutBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
    benchmark = mla_benchmark("IndexOfShort_Buffer", benchmark_category, StringIndexOfShort_Buffer_LayoutBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
    benchmark = mla_benchmark("IndexOf_CStr", benchmark_category, StringIndexOf_C_LayoutBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    // Equal
    ///////////////////////////////////////////
    benchmark = mla_benchmark("Equals_CStr", benchmark_category, StringEquals_C_LayoutBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
    benchmark = mla_benchmark("Equals_Buffer", benchmark_category, StringEquals_Buffer_LayoutBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
    benchmark = mla_benchmark("Equals_BufferShort", benchmark_category, StringEquals_BufferShort_LayoutBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
    benchmark = mla_benchmark("Equals_Embedded", benchmark_category, StringEquals_Embedded_LayoutBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    // Substring Benchmarks
    ///////////////////////////////////////////
    benchmark = mla_benchmark("Substr_Embedded", benchmark_category, SubstringEmbeddedBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
    benchmark = mla_benchmark("Substr_Buffer_Short", benchmark_category, SubstringBufferShortBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
    benchmark = mla_benchmark("Substr_Buffer_Medium", benchmark_category, SubstringBufferMediumBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
    benchmark = mla_benchmark("Substr_C_String", benchmark_category, SubstringCStringBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    // To C String Benchmarks
    ////////////////////////////////////////////
    benchmark = mla_benchmark("C_String_From_C_Layout", benchmark_category, String_to_C_LayoutBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
    benchmark = mla_benchmark("C_String_From_buffer_Layout", benchmark_category, String_to_buffer_LayoutBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
}


#endif
