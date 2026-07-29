//
// Created by chris on 12/15/2025.
//

#ifndef MLA_REFLECTION_TEST_H
#define MLA_REFLECTION_TEST_H

#include "../../lib/base-lib/core/reflection/mla_reflection.h"
#include "../../lib/base-lib/test-support/mla_test_executor.h"

// Test struct definitions
struct test_simple_struct_t {
    mla_bool_t bool_field;
    mla_int32_t int_field;
    mla_float_t float_field;
    mla_string_t string_field;

    static mla_reflection_struct_metadata_t metadata() {
        mla_reflection_struct_metadata_t meta = mla_reflection_struct_metadata(test_simple_struct_t);
        mla_reflection_struct_field_bool(meta, test_simple_struct_t, bool_field);
        mla_reflection_struct_field_int32(meta, test_simple_struct_t, int_field);
        mla_reflection_struct_field_float(meta, test_simple_struct_t, float_field);
        mla_reflection_struct_field_string(meta, test_simple_struct_t, string_field);
        mla_reflection_struct_metadata_freeze(meta);
        return meta;
    }
};

struct test_nested_struct_t {
    mla_uint64_t id;
    test_simple_struct_t nested;
    mla_double_t value;

    static mla_reflection_struct_metadata_t metadata() {
        mla_reflection_struct_metadata_t meta = mla_reflection_struct_metadata(test_nested_struct_t);
        mla_reflection_struct_field_uint64(meta, test_nested_struct_t, id);
        mla_reflection_struct_field_struct(meta, test_nested_struct_t, nested, test_simple_struct_t);
        mla_reflection_struct_field_double(meta, test_nested_struct_t, value);
        mla_reflection_struct_metadata_freeze(meta);
        return meta;
    }
};

struct test_list_struct_t {

    mla_array_list_t<mla_int32_t> int_list;
    mla_array_list_t<mla_init_struct(mla_string_t)> string_list;

    static mla_reflection_struct_metadata_t metadata() {
        mla_reflection_struct_metadata_t meta = mla_reflection_struct_metadata(test_list_struct_t);
        mla_reflection_struct_field_int32_list(meta, test_list_struct_t, int_list);
        mla_reflection_struct_field_string_list(meta, test_list_struct_t, string_list);
        mla_reflection_struct_metadata_freeze(meta);
        return meta;
    }
};

inline void CreateStructMetadataTest() {
    mla_reflection_struct_metadata_t metadata = mla_reflection_struct_metadata_for(mla_string_const("TestStruct"), 128);

    assert_true(mla_string_equals(metadata.name, mla_string_const("TestStruct")), "Metadata name should match");
    assert_equal(metadata.size, (mla_size_t)128, "Metadata size should match");
    assert_false(metadata.frozen, "Metadata should not be frozen initially");
    assert_equal(mla_array_list_size(metadata.fields), (mla_size_t)0, "Fields list should be empty initially");
}

inline void AddFieldsToMetadataTest() {
    mla_reflection_struct_metadata_t metadata = mla_reflection_struct_metadata(test_simple_struct_t);

    mla_reflection_struct_field_bool(metadata, test_simple_struct_t, bool_field);
    mla_reflection_struct_field_int32(metadata, test_simple_struct_t, int_field);

    if (mla_array_list_size(metadata.fields) == 2) {
        mla_reflection_struct_field_t field1 = mla_array_list_get_unsafe(metadata.fields, 0);
        assert_true(mla_string_equals(field1.name, mla_string_const("bool_field")), "First field name should match");
        assert_equal(field1.type, MLA_REFLECTION_TYPE_BOOL, "First field type should be BOOL");

        mla_reflection_struct_field_t field2 = mla_array_list_get_unsafe(metadata.fields, 1);
        assert_true(mla_string_equals(field2.name, mla_string_const("int_field")), "Second field name should match");
        assert_equal(field2.type, MLA_REFLECTION_TYPE_INT32, "Second field type should be INT32");
    } else {
        assert_fail("Fields list size should be 2 after adding fields");
    }
}

inline void FreezeMetadataTest() {
    mla_reflection_struct_metadata_t metadata = mla_reflection_struct_metadata_for(mla_string_const("TestStruct"), 64);

    assert_false(metadata.frozen, "Metadata should not be frozen initially");

    mla_reflection_struct_metadata_freeze(metadata);

    assert_true(metadata.frozen, "Metadata should be frozen after freeze call");
}

inline void StructMetadataProviderTest() {
    mla_reflection_struct_metadata_t metadata = test_simple_struct_t::metadata();

    assert_true(mla_string_equals(metadata.name, mla_string_const("test_simple_struct_t")), "Struct name should match");
    assert_equal(metadata.size, (mla_size_t)sizeof(test_simple_struct_t), "Struct size should match");
    assert_true(metadata.frozen, "Metadata should be frozen");
    assert_equal(mla_array_list_size(metadata.fields), (mla_size_t)4, "Should have 4 fields");
}

inline void NestedStructMetadataTest() {
    mla_reflection_struct_metadata_t metadata = test_nested_struct_t::metadata();

    if (mla_array_list_size(metadata.fields) == 3) {
        mla_reflection_struct_field_t nested_field = mla_array_list_get_unsafe(metadata.fields, 1);
        assert_equal(nested_field.type, MLA_REFLECTION_TYPE_STRUCT, "Nested field should be STRUCT type");
        assert_true(mla_string_equals(nested_field.struct_provider().name, mla_string_const("test_simple_struct_t")), "Nested struct name should match");
    } else {
        assert_fail("Fields list size should be 3 after adding fields");
    }

}

inline void ListFieldMetadataTest() {
    mla_reflection_struct_metadata_t metadata = test_list_struct_t::metadata();

    if (mla_array_list_size(metadata.fields) == 2) {
        mla_reflection_struct_field_t int_list_field = mla_array_list_get_unsafe(metadata.fields, 0);
        assert_equal(int_list_field.type, MLA_REFLECTION_TYPE_LIST, "Int list field should be LIST type");
        assert_equal(int_list_field.element_type, MLA_REFLECTION_TYPE_INT32, "Element type should be INT32");

        mla_reflection_struct_field_t string_list_field = mla_array_list_get_unsafe(metadata.fields, 1);
        assert_equal(string_list_field.type, MLA_REFLECTION_TYPE_LIST, "String list field should be LIST type");
        assert_equal(string_list_field.element_type, MLA_REFLECTION_TYPE_STRING, "Element type should be STRING");
    } else {
        assert_fail("Fields list size should be 2 after adding fields");
    }

}

inline void RegisterStructTest() {
    mla_bool_t result = mla_reflection_register_struct<test_simple_struct_t>();
    assert_true(result, "Should successfully register struct");
}

inline void AllTypeFieldsTest() {
    struct all_types_t {
        mla_bool_t bool_val;
        mla_int8_t int8_val;
        mla_int16_t int16_val;
        mla_int32_t int32_val;
        mla_int64_t int64_val;
        mla_uint8_t uint8_val;
        mla_uint16_t uint16_val;
        mla_uint32_t uint32_val;
        mla_uint64_t uint64_val;
        mla_float_t float_val;
        mla_double_t double_val;
        mla_string_t string_val;
    };


    mla_reflection_struct_metadata_t metadata = mla_reflection_struct_metadata(all_types_t);
    mla_reflection_struct_field_bool(metadata, all_types_t, bool_val);
    mla_reflection_struct_field_int8(metadata, all_types_t, int8_val);
    mla_reflection_struct_field_int16(metadata, all_types_t, int16_val);
    mla_reflection_struct_field_int32(metadata, all_types_t, int32_val);
    mla_reflection_struct_field_int64(metadata, all_types_t, int64_val);
    mla_reflection_struct_field_uint8(metadata, all_types_t, uint8_val);
    mla_reflection_struct_field_uint16(metadata, all_types_t, uint16_val);
    mla_reflection_struct_field_uint32(metadata, all_types_t, uint32_val);
    mla_reflection_struct_field_uint64(metadata, all_types_t, uint64_val);
    mla_reflection_struct_field_float(metadata, all_types_t, float_val);
    mla_reflection_struct_field_double(metadata, all_types_t, double_val);
    mla_reflection_struct_field_string(metadata, all_types_t, string_val);


    assert_equal(mla_array_list_size(metadata.fields), (mla_size_t)12, "Should have 12 fields for all basic types");
}


inline void IsStructRegisteredTest() {
    // Test with unregistered struct
    mla_bool_t is_registered = mla_reflection_is_struct_registered(mla_string_const("NonExistentStruct"));
    assert_false(is_registered, "Non-existent struct should not be registered");

    // Register a struct
    mla_reflection_register_struct<test_simple_struct_t>();

    // Test with registered struct
    is_registered = mla_reflection_is_struct_registered(mla_string_const("test_simple_struct_t"));
    assert_true(is_registered, "test_simple_struct_t should be registered");

    // Test with different struct name
    is_registered = mla_reflection_is_struct_registered(mla_string_const("AnotherNonExistent"));
    assert_false(is_registered, "Another non-existent struct should not be registered");
}

inline void GetStructMetadataTest() {
    mla_reflection_struct_metadata_t metadata = mla_reflection_struct_metadata_invalid();

    // Test with unregistered struct
    mla_bool_t result = mla_reflection_get_struct_metadata(mla_string_const("UnknownStruct"), metadata);
    assert_false(result, "Should fail to get metadata for unregistered struct");

    // Register a struct
    mla_reflection_register_struct<test_nested_struct_t>();

    // Test with registered struct
    result = mla_reflection_get_struct_metadata(mla_string_const("test_nested_struct_t"), metadata);
    assert_true(result, "Should successfully get metadata for registered struct");
    assert_true(mla_string_equals(metadata.name, mla_string_const("test_nested_struct_t")), "Retrieved metadata name should match");
    assert_equal(metadata.size, (mla_size_t)sizeof(test_nested_struct_t), "Retrieved metadata size should match");
    assert_true(metadata.frozen, "Retrieved metadata should be frozen");
    assert_equal(mla_array_list_size(metadata.fields), (mla_size_t)3, "Retrieved metadata should have 3 fields");

    // Verify field details
    if (mla_array_list_size(metadata.fields) == 3) {
        mla_reflection_struct_field_t field = mla_array_list_get_unsafe(metadata.fields, 0);
        assert_true(mla_string_equals(field.name, mla_string_const("id")), "First field name should be 'id'");
        assert_equal(field.type, MLA_REFLECTION_TYPE_UINT64, "First field type should be UINT64");
    }
}


void RegisterReflectionTests(mla_test_executor_t &p_TestExecutor) {
    mla_test_t test = mla_test("CreateStructMetadata", test_category, CreateStructMetadataTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("AddFieldsToMetadata", test_category, AddFieldsToMetadataTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("FreezeMetadata", test_category, FreezeMetadataTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StructMetadataProvider", test_category, StructMetadataProviderTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("NestedStructMetadata", test_category, NestedStructMetadataTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ListFieldMetadata", test_category, ListFieldMetadataTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("RegisterStruct", test_category, RegisterStructTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("AllTypeFields", test_category, AllTypeFieldsTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("IsStructRegistered", test_category, IsStructRegisteredTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("GetStructMetadata", test_category, GetStructMetadataTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}

#endif
