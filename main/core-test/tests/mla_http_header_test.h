//
// Created by christian on 10/8/25.
//

#ifndef MLA_HTTP_HEADER_TEST_H
#define MLA_HTTP_HEADER_TEST_H

#include "../../lib/base-lib/core/http/mla_http_header.h"
#include "../../lib/base-lib/core/system/mla_string.h"
#include "../../lib/base-lib/test-support/mla_test_executor.h"

// Adds a single header and verifies retrieval via value and values API.
void HttpHeader_AddSingleTest() {

    auto headers = mla_array_list<mla_init_struct(mla_http_header_t)>();

    mla_http_headers_add(headers, mla_string_const("Content-Type"), mla_string_const("text/plain"));

    assert_equal((mla_test_int32_t)mla_array_list_size(headers), (mla_test_int32_t)1, "headers.size should be 1");

    mla_string_t v = mla_http_headers_get_value(headers, mla_string_const("Content-Type"));
    assert_true(mla_string_equals(v, mla_string_const("text/plain")), "get_value(Content-Type)==text/plain");

    auto vs = mla_http_headers_get_values(headers, mla_string_const("Content-Type"));

    if (mla_array_list_size(vs) == 1) {
        assert_true(mla_string_equals(mla_array_list_get_unsafe(vs, 0), mla_string_const("text/plain")), "values[0]==text/plain");
        mla_http_header_t* h = mla_array_list_get_ref(headers, 0);
        assert_equal((mla_test_int32_t)h->type, (mla_test_int32_t)MLA_HTTP_HEADER_TYPE_SINGLE, "header.type==SINGLE");
    } else {
        assert_fail("get_values(Content-Type) size!=1");
    }
}

// Adds a header twice and verifies it becomes multi-value and preserves both values.
void HttpHeader_DuplicateBecomesMultiTest() {

    auto headers = mla_array_list<mla_init_struct(mla_http_header_t)>();

    mla_http_headers_add(headers, mla_string_const("Set-Cookie"), mla_string_const("a=1"));
    mla_http_headers_add(headers, mla_string_const("Set-Cookie"), mla_string_const("b=2"));

    assert_equal((mla_test_int32_t)mla_array_list_size(headers), (mla_test_int32_t)1, "only one header entry");

    mla_http_header_t* h = mla_array_list_get_ref(headers, 0);

    if (h != nullptr) {
        assert_equal((mla_test_int32_t)h->type, (mla_test_int32_t)MLA_HTTP_HEADER_TYPE_MULTI, "header.type==MULTI");
        assert_equal((mla_test_int32_t)mla_array_list_size(h->values), (mla_test_int32_t)2, "multi values size==2");
        assert_true(mla_string_equals(mla_array_list_get_unsafe(h->values, 0), mla_string_const("a=1")), "values[0]==a=1");
        assert_true(mla_string_equals(mla_array_list_get_unsafe(h->values, 1), mla_string_const("b=2")), "values[1]==b=2");
    } else {
        assert_fail("header is null");
    }


    mla_string_t first = mla_http_headers_get_value(headers, mla_string_const("Set-Cookie"));
    assert_true(mla_string_equals(first, mla_string_const("a=1")), "get_value(Set-Cookie)==first value");

    auto vs = mla_http_headers_get_values(headers, mla_string_const("Set-Cookie"));
    assert_equal((mla_test_int32_t)mla_array_list_size(vs), (mla_test_int32_t)2, "get_values(Set-Cookie) size==2");
}

// Verifies case-insensitive matching for header names.
void HttpHeader_CaseInsensitiveNameTest() {

    auto headers = mla_array_list<mla_init_struct(mla_http_header_t)>();

    mla_http_headers_add(headers, mla_string_const("content-type"), mla_string_const("text/plain"));
    mla_http_headers_add(headers, mla_string_const("Content-Type"), mla_string_const("application/json"));

    assert_equal((mla_test_int32_t)mla_array_list_size(headers), (mla_test_int32_t)1, "only one header entry (case-insensitive match)");

    mla_http_header_t* h = mla_array_list_get_ref(headers, 0);

    if (h != nullptr) {
        assert_equal((mla_test_int32_t)h->type, (mla_test_int32_t)MLA_HTTP_HEADER_TYPE_MULTI, "header.type==MULTI after duplicate add");
        assert_equal((mla_test_int32_t)mla_array_list_size(h->values), (mla_test_int32_t)2, "values size==2");
        assert_true(mla_string_equals(mla_array_list_get_unsafe(h->values, 0), mla_string_const("text/plain")), "values[0]==text/plain");
        assert_true(mla_string_equals(mla_array_list_get_unsafe(h->values, 1), mla_string_const("application/json")), "values[1]==application/json");
    } else {
        assert_fail("header is null");
    }


    // Mixed case query
    mla_string_t v = mla_http_headers_get_value(headers, mla_string_const("CONTENT-TYPE"));
    assert_true(mla_string_equals(v, mla_string_const("text/plain")), "get_value(CONTENT-TYPE)==first value");
}

// Verifies get\_value and get\_values for a missing header.
void HttpHeader_MissingHeaderLookupTest() {

    auto headers = mla_array_list<mla_init_struct(mla_http_header_t)>();

    mla_string_t v = mla_http_headers_get_value(headers, mla_string_const("X-Missing"));
    assert_true(mla_string_equals(v, mla_string_empty()), "get_value(missing)==empty");

    auto vs = mla_http_headers_get_values(headers, mla_string_const("X-Missing"));
    assert_equal((mla_test_int32_t)mla_array_list_size(vs), (mla_test_int32_t)0, "get_values(missing) size==0");
}

// Verifies values maintain insertion order for multi-value headers.
void HttpHeader_PreserveInsertionOrderTest() {

    auto headers = mla_array_list<mla_init_struct(mla_http_header_t)>();

    mla_http_headers_add(headers, mla_string_const("X-Order"), mla_string_const("v1"));
    mla_http_headers_add(headers, mla_string_const("X-Order"), mla_string_const("v2"));
    mla_http_headers_add(headers, mla_string_const("X-Order"), mla_string_const("v3"));

    auto vs = mla_http_headers_get_values(headers, mla_string_const("X-Order"));

    if (mla_array_list_size(vs) == 3) {
        assert_true(mla_string_equals(mla_array_list_get_unsafe(vs, 0), mla_string_const("v1")), "values[0]==v1");
        assert_true(mla_string_equals(mla_array_list_get_unsafe(vs, 1), mla_string_const("v2")), "values[1]==v2");
        assert_true(mla_string_equals(mla_array_list_get_unsafe(vs, 2), mla_string_const("v3")), "values[2]==v3");
    } else {
        assert_fail("get_values(X-Order) size!=3");
    }
}

// Verifies get\_values on a single-value header returns a one-element list.
void HttpHeader_GetValuesFromSingleTest() {

    auto headers = mla_array_list<mla_init_struct(mla_http_header_t)>();

    mla_http_headers_add(headers, mla_string_const("Server"), mla_string_const("core"));
    auto vs = mla_http_headers_get_values(headers, mla_string_const("Server"));

    if (mla_array_list_size(vs) == 1) {
        assert_true(mla_string_equals(mla_array_list_get_unsafe(vs, 0), mla_string_const("core")), "values[0]==core");
    }  else {
        assert_fail("failed to get single value from single-value header");
    }


}

void HttpHeader_HasValueMultiTest() {


    auto headers = mla_array_list<mla_init_struct(mla_http_header_t)>();

    mla_http_headers_add(headers, mla_string_const("X-Order"), mla_string_const("v1"));
    mla_http_headers_add(headers, mla_string_const("X-Order"), mla_string_const("v2"));
    mla_http_headers_add(headers, mla_string_const("X-Order"), mla_string_const("v3"));

    assert_true(mla_http_headers_has_header_value(headers, mla_string_const("X-Order"), mla_string_const("v2")), "has_header_value should find existing value");
    assert_false(mla_http_headers_has_header_value(headers, mla_string_const("X-Order"), mla_string_const("v4")), "has_header_value should not find non-existing value");

}

void HttpHeader_HasValueSingleTest() {

    auto headers = mla_array_list<mla_init_struct(mla_http_header_t)>();

    mla_http_headers_add(headers, mla_string_const("X-Order"), mla_string_const("v1"));

    assert_true(mla_http_headers_has_header_value(headers, mla_string_const("X-Order"), mla_string_const("v1")), "has_header_value should find existing value");
    assert_false(mla_http_headers_has_header_value(headers, mla_string_const("X-Order"), mla_string_const("v4")), "has_header_value should not find non-existing value");


}

// Registration
void RegisterHttpHeaderTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("AddSingle", test_category, HttpHeader_AddSingleTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("DuplicateBecomesMulti", test_category, HttpHeader_DuplicateBecomesMultiTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("CaseInsensitiveName", test_category, HttpHeader_CaseInsensitiveNameTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("MissingLookup", test_category, HttpHeader_MissingHeaderLookupTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("PreserveInsertionOrder", test_category, HttpHeader_PreserveInsertionOrderTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("HasValueMulti", test_category, HttpHeader_HasValueMultiTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("HasValueSingle", test_category, HttpHeader_HasValueSingleTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}

#endif

