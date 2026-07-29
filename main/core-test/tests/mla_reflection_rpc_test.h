//
// Created by chris on 12/19/2025.
//

#ifndef MLA_REFLECTION_RPC_TEST_H
#define MLA_REFLECTION_RPC_TEST_H

#include "../../lib/base-lib/core/reflection/mla_reflection_rpc.h"
#include "../../lib/base-lib/core/reflection/mla_reflection.h"
#include "../../lib/base-lib/core/serializer/mla_serializer.h"
#include "../../lib/base-lib/test-support/mla_test_executor.h"

// Test struct for testing reflection RPC
struct test_rpc_simple_struct_t {
    mla_string_t name;
    mla_uint32_t id;
    mla_bool_t active;

    static test_rpc_simple_struct_t init() {
        return { mla_string_empty(), 0, false };
    }

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const test_rpc_simple_struct_t* test_struct = mla_pointer_get_data<const test_rpc_simple_struct_t>(obj);

        if (test_struct == nullptr) {
            return false;
        }

        mla_serializer_write_string(serializer, mla_string_const("name"), test_struct->name);
        mla_serializer_write_uint32(serializer, mla_string_const("id"), test_struct->id);
        mla_serializer_write_bool(serializer, mla_string_const("active"), test_struct->active);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        test_rpc_simple_struct_t* test_struct = mla_pointer_get_data<test_rpc_simple_struct_t>(obj);

        if (test_struct == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "name")) {
            mla_deserializer_read_string(deserializer, test_struct->name);
        } else if (mla_string_equals_const(property_name, "id")) {
            mla_deserializer_read_uint32(deserializer, test_struct->id);
        } else if (mla_string_equals_const(property_name, "active")) {
            mla_deserializer_read_bool(deserializer, test_struct->active);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }

    static mla_reflection_struct_metadata_t metadata() {
        mla_reflection_struct_metadata_t meta = mla_reflection_struct_metadata(test_rpc_simple_struct_t);
        mla_reflection_struct_field_string(meta, test_rpc_simple_struct_t, name);
        mla_reflection_struct_field_uint32(meta, test_rpc_simple_struct_t, id);
        mla_reflection_struct_field_bool(meta, test_rpc_simple_struct_t, active);
        mla_reflection_struct_metadata_freeze(meta);
        return meta;
    }
};

struct test_rpc_nested_struct_t {
    test_rpc_simple_struct_t inner;
    mla_int32_t value;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const test_rpc_nested_struct_t* test_struct = mla_pointer_get_data<const test_rpc_nested_struct_t>(obj);

        if (test_struct == nullptr) {
            return false;
        }

        mla_serializer_write_struct(serializer, mla_string_const("inner"), test_struct->inner, test_rpc_simple_struct_t);
        mla_serializer_write_int32(serializer, mla_string_const("value"), test_struct->value);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        test_rpc_nested_struct_t* test_struct = mla_pointer_get_data<test_rpc_nested_struct_t>(obj);

        if (test_struct == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "inner")) {
            mla_deserializer_read_struct(deserializer, test_struct->inner, test_rpc_simple_struct_t);
        } else if (mla_string_equals_const(property_name, "value")) {
            mla_deserializer_read_int32(deserializer, test_struct->value);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }

    static mla_reflection_struct_metadata_t metadata() {
        mla_reflection_struct_metadata_t meta = mla_reflection_struct_metadata(test_rpc_nested_struct_t);
        mla_reflection_struct_field_struct(meta, test_rpc_nested_struct_t, inner, test_rpc_simple_struct_t);
        mla_reflection_struct_field_int32(meta, test_rpc_nested_struct_t, value);
        mla_reflection_struct_metadata_freeze(meta);
        return meta;
    }
};

struct test_rpc_with_list_struct_t {
    mla_array_list_t<mla_init_struct(test_rpc_simple_struct_t)> items;
    mla_uint32_t count;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const test_rpc_with_list_struct_t* test_struct = mla_pointer_get_data<const test_rpc_with_list_struct_t>(obj);

        if (test_struct == nullptr) {
            return false;
        }

        mla_serializer_write_list_struct(serializer, mla_string_const("items"), test_struct->items, test_rpc_simple_struct_t);
        mla_serializer_write_uint32(serializer, mla_string_const("count"), test_struct->count);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        test_rpc_with_list_struct_t* test_struct = mla_pointer_get_data<test_rpc_with_list_struct_t>(obj);

        if (test_struct == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "items")) {
            mla_deserializer_read_list_struct(deserializer, test_struct->items, test_rpc_simple_struct_t);
        } else if (mla_string_equals_const(property_name, "count")) {
            mla_deserializer_read_uint32(deserializer, test_struct->count);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }

    static mla_reflection_struct_metadata_t metadata() {
        mla_reflection_struct_metadata_t meta = mla_reflection_struct_metadata(test_rpc_with_list_struct_t);
        mla_reflection_struct_field_struct_list(meta, test_rpc_with_list_struct_t, items, test_rpc_simple_struct_t);
        mla_reflection_struct_field_uint32(meta, test_rpc_with_list_struct_t, count);
        mla_reflection_struct_metadata_freeze(meta);
        return meta;
    }
};

// Test: Invalid struct metadata initialization
inline void ReflectionRpcStructMetadataInvalidTest() {
    mla_reflection_rpc_struct_metadata_t metadata = mla_reflection_rpc_struct_metadata_invalid();

    assert_true(mla_string_is_empty(metadata.name), "Invalid metadata should have empty name");
    assert_equal(mla_array_list_size(metadata.fields), (mla_size_t)0, "Invalid metadata should have no fields");
}

// Test: Get metadata for unregistered struct
inline void ReflectionRpcGetUnregisteredStructTest() {
    mla_reflection_struct_metadata_request_t request = mla_reflection_struct_metadata_request_empty();
    request.struct_name = mla_string_const("NonExistentStruct");

    mla_reflection_struct_metadata_response_t response = mla_reflection_struct_metadata_response_empty();
    response.found = true;  // Set to true to verify it changes
    response.meta_data = mla_reflection_rpc_struct_metadata_invalid();

    mla_bool_t result = mla_reflection_metadata_rpc_get_metadata(&request, &response);

    assert_true(result, "Function should return true even if struct not found");
    assert_false(response.found, "Response should indicate struct was not found");
}

// Test: Get metadata for registered simple struct
inline void ReflectionRpcGetSimpleStructMetadataTest() {
    // Register the test struct
    mla_reflection_register_struct<test_rpc_simple_struct_t>();

    mla_reflection_struct_metadata_request_t request = mla_reflection_struct_metadata_request_empty();
    request.struct_name = mla_string_const("test_rpc_simple_struct_t");

    mla_reflection_struct_metadata_response_t response = mla_reflection_struct_metadata_response_empty();
    response.found = false;
    response.meta_data = mla_reflection_rpc_struct_metadata_invalid();

    mla_bool_t result = mla_reflection_metadata_rpc_get_metadata(&request, &response);

    assert_true(result, "Function should return true");
    assert_true(response.found, "Response should indicate struct was found");
    assert_true(mla_string_equals_const(response.meta_data.name, "test_rpc_simple_struct_t"), "Metadata name should match");
    assert_equal(mla_array_list_size(response.meta_data.fields), (mla_size_t)3, "Should have 3 fields");
}

// Test: Verify field metadata details
inline void ReflectionRpcFieldMetadataDetailsTest() {
    // Register the test struct
    mla_reflection_register_struct<test_rpc_simple_struct_t>();

    mla_reflection_struct_metadata_request_t request = mla_reflection_struct_metadata_request_empty();
    request.struct_name = mla_string_const("test_rpc_simple_struct_t");

    mla_reflection_struct_metadata_response_t response = mla_reflection_struct_metadata_response_empty();
    response.found = false;
    response.meta_data = mla_reflection_rpc_struct_metadata_invalid();

    mla_reflection_metadata_rpc_get_metadata(&request, &response);

    assert_true(response.found, "Struct should be found");

    // Check individual fields
    mla_bool_t found_name = false;
    mla_bool_t found_id = false;
    mla_bool_t found_active = false;

    for (mla_size_t i = 0; i < mla_array_list_size(response.meta_data.fields); ++i) {
        const mla_reflection_rpc_struct_metadata_field_t& field = mla_array_list_get_unsafe(response.meta_data.fields, i);

        if (mla_string_equals_const(field.name, "name")) {
            found_name = true;
            assert_equal((mla_uint8_t)field.data_type, (mla_uint8_t)MLA_REFLECTION_TYPE_STRING, "name field should be STRING type");
        } else if (mla_string_equals_const(field.name, "id")) {
            found_id = true;
            assert_equal((mla_uint8_t)field.data_type, (mla_uint8_t)MLA_REFLECTION_TYPE_UINT32, "id field should be UINT32 type");
        } else if (mla_string_equals_const(field.name, "active")) {
            found_active = true;
            assert_equal((mla_uint8_t)field.data_type, (mla_uint8_t)MLA_REFLECTION_TYPE_BOOL, "active field should be BOOL type");
        }
    }

    assert_true(found_name, "Should find 'name' field");
    assert_true(found_id, "Should find 'id' field");
    assert_true(found_active, "Should find 'active' field");
}

// Test: Get metadata for nested struct
inline void ReflectionRpcGetNestedStructMetadataTest() {
    // Register both structs
    mla_reflection_register_struct<test_rpc_simple_struct_t>();
    mla_reflection_register_struct<test_rpc_nested_struct_t>();

    mla_reflection_struct_metadata_request_t request = mla_reflection_struct_metadata_request_empty();
    request.struct_name = mla_string_const("test_rpc_nested_struct_t");

    mla_reflection_struct_metadata_response_t response = mla_reflection_struct_metadata_response_empty();
    response.found = false;
    response.meta_data = mla_reflection_rpc_struct_metadata_invalid();

    mla_bool_t result = mla_reflection_metadata_rpc_get_metadata(&request, &response);

    assert_true(result, "Function should return true");
    assert_true(response.found, "Response should indicate struct was found");
    assert_equal(mla_array_list_size(response.meta_data.fields), (mla_size_t)2, "Should have 2 fields");

    // Check that nested struct field has struct_name populated
    mla_bool_t found_struct_field = false;
    for (mla_size_t i = 0; i < mla_array_list_size(response.meta_data.fields); ++i) {
        const mla_reflection_rpc_struct_metadata_field_t& field = mla_array_list_get_unsafe(response.meta_data.fields, i);

        if (mla_string_equals_const(field.name, "inner")) {
            found_struct_field = true;
            assert_equal((mla_uint8_t)field.data_type, (mla_uint8_t)MLA_REFLECTION_TYPE_STRUCT, "inner field should be STRUCT type");
            assert_true(mla_string_equals_const(field.struct_name, "test_rpc_simple_struct_t"), "struct_name should be set for nested struct");
        }
    }

    assert_true(found_struct_field, "Should find nested struct field");
}

// Test: Get metadata for struct with list
inline void ReflectionRpcGetStructWithListMetadataTest() {
    // Register structs
    mla_reflection_register_struct<test_rpc_simple_struct_t>();
    mla_reflection_register_struct<test_rpc_with_list_struct_t>();

    mla_reflection_struct_metadata_request_t request = mla_reflection_struct_metadata_request_empty();
    request.struct_name = mla_string_const("test_rpc_with_list_struct_t");

    mla_reflection_struct_metadata_response_t response = mla_reflection_struct_metadata_response_empty();
    response.found = false;
    response.meta_data = mla_reflection_rpc_struct_metadata_invalid();

    mla_bool_t result = mla_reflection_metadata_rpc_get_metadata(&request, &response);

    assert_true(result, "Function should return true");
    assert_true(response.found, "Response should indicate struct was found");

    // Check list field
    mla_bool_t found_list_field = false;
    for (mla_size_t i = 0; i < mla_array_list_size(response.meta_data.fields); ++i) {
        const mla_reflection_rpc_struct_metadata_field_t& field = mla_array_list_get_unsafe(response.meta_data.fields, i);

        if (mla_string_equals_const(field.name, "items")) {
            found_list_field = true;
            assert_equal((mla_uint8_t)field.data_type, (mla_uint8_t)MLA_REFLECTION_TYPE_LIST, "items field should be LIST type");
            assert_equal((mla_uint8_t)field.element_type, (mla_uint8_t)MLA_REFLECTION_TYPE_STRUCT, "List element should be STRUCT type");
            assert_true(mla_string_equals_const(field.struct_name, "test_rpc_simple_struct_t"), "struct_name should be set for list element");
        }
    }

    assert_true(found_list_field, "Should find list field");
}

// Test: Request with empty struct name
inline void ReflectionRpcEmptyStructNameTest() {
     mla_reflection_struct_metadata_request_t request = mla_reflection_struct_metadata_request_empty();
    request.struct_name = mla_string_empty();

    mla_reflection_struct_metadata_response_t response = mla_reflection_struct_metadata_response_empty();
    response.found = true;
    response.meta_data = mla_reflection_rpc_struct_metadata_invalid();

    mla_bool_t result = mla_reflection_metadata_rpc_get_metadata(&request, &response);

    assert_true(result, "Function should return true");
    assert_false(response.found, "Response should indicate struct was not found for empty name");
}

// Test: Request with null pointer
inline void ReflectionRpcNullRequestTest() {
    mla_reflection_struct_metadata_response_t response = mla_reflection_struct_metadata_response_empty();

    mla_bool_t result = mla_reflection_metadata_rpc_get_metadata(nullptr, &response);

    assert_false(result, "Function should return false for null request");
}

// Test: Multiple metadata requests
inline void ReflectionRpcMultipleRequestsTest() {
    // Register multiple structs
    mla_reflection_register_struct<test_rpc_simple_struct_t>();
    mla_reflection_register_struct<test_rpc_nested_struct_t>();

    // Request first struct
    mla_reflection_struct_metadata_request_t request1 = mla_reflection_struct_metadata_request_empty();
    request1.struct_name = mla_string_const("test_rpc_simple_struct_t");

    mla_reflection_struct_metadata_response_t response1 = mla_reflection_struct_metadata_response_empty();
    response1.found = false;
    response1.meta_data = mla_reflection_rpc_struct_metadata_invalid();

    mla_bool_t result1 = mla_reflection_metadata_rpc_get_metadata(&request1, &response1);

    // Request second struct
    mla_reflection_struct_metadata_request_t request2 = mla_reflection_struct_metadata_request_empty();
    request2.struct_name = mla_string_const("test_rpc_nested_struct_t");

    mla_reflection_struct_metadata_response_t response2 = mla_reflection_struct_metadata_response_empty();
    response2.found = false;
    response2.meta_data = mla_reflection_rpc_struct_metadata_invalid();

    mla_bool_t result2 = mla_reflection_metadata_rpc_get_metadata(&request2, &response2);

    assert_true(result1, "First request should succeed");
    assert_true(result2, "Second request should succeed");
    assert_true(response1.found, "First struct should be found");
    assert_true(response2.found, "Second struct should be found");
    assert_true(mla_string_equals_const(response1.meta_data.name, "test_rpc_simple_struct_t"), "First struct name should match");
    assert_true(mla_string_equals_const(response2.meta_data.name, "test_rpc_nested_struct_t"), "Second struct name should match");
}

inline void RegisterReflectionRpcTests(mla_test_executor_t &p_TestExecutor) {
    mla_test_t test = mla_test("ReflectionRpc_StructMetadataInvalid", test_category, ReflectionRpcStructMetadataInvalidTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ReflectionRpc_GetUnregisteredStruct", test_category, ReflectionRpcGetUnregisteredStructTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ReflectionRpc_GetSimpleStructMetadata", test_category, ReflectionRpcGetSimpleStructMetadataTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ReflectionRpc_FieldMetadataDetails", test_category, ReflectionRpcFieldMetadataDetailsTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ReflectionRpc_GetNestedStructMetadata", test_category, ReflectionRpcGetNestedStructMetadataTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ReflectionRpc_GetStructWithListMetadata", test_category, ReflectionRpcGetStructWithListMetadataTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ReflectionRpc_EmptyStructName", test_category, ReflectionRpcEmptyStructNameTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ReflectionRpc_NullRequest", test_category, ReflectionRpcNullRequestTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ReflectionRpc_MultipleRequests", test_category, ReflectionRpcMultipleRequestsTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}

#endif

