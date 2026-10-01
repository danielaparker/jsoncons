// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json_exception.hpp>
#include <jsoncons/json.hpp>
#include <catch/catch.hpp>
#include <new>
#include <string>

using namespace jsoncons;

TEST_CASE("test json_storage_kind")
{
    SECTION("is_primitive")
    {
        CHECK(is_primitive_storage(jsoncons::json_storage_kind::null));
        CHECK(is_primitive_storage(jsoncons::json_storage_kind::boolean));
        CHECK(is_primitive_storage(jsoncons::json_storage_kind::uint64));
        CHECK(is_primitive_storage(jsoncons::json_storage_kind::int64));
        CHECK(is_primitive_storage(jsoncons::json_storage_kind::half_float));
        CHECK(is_primitive_storage(jsoncons::json_storage_kind::short_str));
        CHECK(is_primitive_storage(jsoncons::json_storage_kind::empty_object));
        CHECK_FALSE(is_primitive_storage(jsoncons::json_storage_kind::const_json_ref));
        CHECK_FALSE(is_primitive_storage(jsoncons::json_storage_kind::json_ref));
        CHECK_FALSE(is_primitive_storage(jsoncons::json_storage_kind::long_str));
        CHECK_FALSE(is_primitive_storage(jsoncons::json_storage_kind::byte_str));
        CHECK_FALSE(is_primitive_storage(jsoncons::json_storage_kind::array));
        CHECK_FALSE(is_primitive_storage(jsoncons::json_storage_kind::object));
    }
    SECTION("is_trivial_storage")
    {
        CHECK(is_trivial_storage(jsoncons::json_storage_kind::null));
        CHECK(is_trivial_storage(jsoncons::json_storage_kind::boolean));
        CHECK(is_trivial_storage(jsoncons::json_storage_kind::uint64));
        CHECK(is_trivial_storage(jsoncons::json_storage_kind::int64));
        CHECK(is_trivial_storage(jsoncons::json_storage_kind::half_float));
        CHECK(is_trivial_storage(jsoncons::json_storage_kind::short_str));
        CHECK(is_trivial_storage(jsoncons::json_storage_kind::empty_object));
        CHECK(is_trivial_storage(jsoncons::json_storage_kind::const_json_ref));
        CHECK(is_trivial_storage(jsoncons::json_storage_kind::json_ref));
        CHECK_FALSE(is_trivial_storage(jsoncons::json_storage_kind::long_str));
        CHECK_FALSE(is_trivial_storage(jsoncons::json_storage_kind::byte_str));
        CHECK_FALSE(is_trivial_storage(jsoncons::json_storage_kind::array));
        CHECK_FALSE(is_trivial_storage(jsoncons::json_storage_kind::object));
    }
    SECTION("is_string_storage")
    {
        CHECK_FALSE(is_string_storage(jsoncons::json_storage_kind::null));
        CHECK_FALSE(is_string_storage(jsoncons::json_storage_kind::boolean));
        CHECK_FALSE(is_string_storage(jsoncons::json_storage_kind::uint64));
        CHECK_FALSE(is_string_storage(jsoncons::json_storage_kind::int64));
        CHECK_FALSE(is_string_storage(jsoncons::json_storage_kind::half_float));
        CHECK(is_string_storage(jsoncons::json_storage_kind::short_str));
        CHECK_FALSE(is_string_storage(jsoncons::json_storage_kind::empty_object));
        CHECK_FALSE(is_string_storage(jsoncons::json_storage_kind::const_json_ref));
        CHECK_FALSE(is_string_storage(jsoncons::json_storage_kind::json_ref));
        CHECK(is_string_storage(jsoncons::json_storage_kind::long_str));
        CHECK_FALSE(is_string_storage(jsoncons::json_storage_kind::byte_str));
        CHECK_FALSE(is_string_storage(jsoncons::json_storage_kind::array));
        CHECK_FALSE(is_string_storage(jsoncons::json_storage_kind::object));
    }
}

TEST_CASE("test jsoncons::semantic_tag")
{
    SECTION("is_number")
    {
        CHECK_FALSE(is_number_tag(jsoncons::semantic_tag::none));
        CHECK_FALSE(is_number_tag(jsoncons::semantic_tag::undefined));
        CHECK_FALSE(is_number_tag(jsoncons::semantic_tag::datetime));
        CHECK_FALSE(is_number_tag(jsoncons::semantic_tag::epoch_second));
        CHECK_FALSE(is_number_tag(jsoncons::semantic_tag::epoch_milli));
        CHECK_FALSE(is_number_tag(jsoncons::semantic_tag::epoch_nano));
        CHECK_FALSE(is_number_tag(jsoncons::semantic_tag::base64));
        CHECK_FALSE(is_number_tag(jsoncons::semantic_tag::base64url));
        CHECK_FALSE(is_number_tag(jsoncons::semantic_tag::uri));
        CHECK_FALSE(is_number_tag(jsoncons::semantic_tag::clamped));
        CHECK_FALSE(is_number_tag(jsoncons::semantic_tag::multi_dim_row_major));
        CHECK_FALSE(is_number_tag(jsoncons::semantic_tag::multi_dim_column_major));
        CHECK(is_number_tag(jsoncons::semantic_tag::bigint));
        CHECK(is_number_tag(jsoncons::semantic_tag::bigdec));
        CHECK(is_number_tag(jsoncons::semantic_tag::bigfloat));
        CHECK(is_number_tag(jsoncons::semantic_tag::float128));
        CHECK_FALSE(is_number_tag(jsoncons::semantic_tag::ext));
        CHECK_FALSE(is_number_tag(jsoncons::semantic_tag::id));
        CHECK_FALSE(is_number_tag(jsoncons::semantic_tag::regex));
        CHECK_FALSE(is_number_tag(jsoncons::semantic_tag::code));
    }
}

TEST_CASE("json storage tests")
{
    jsoncons::json var1(int64_t(-100), jsoncons::semantic_tag::none);
    CHECK(jsoncons::json_storage_kind::int64 == var1.storage_kind());
    jsoncons::json var2(uint64_t(100), jsoncons::semantic_tag::none);
    CHECK(jsoncons::json_storage_kind::uint64 == var2.storage_kind());
    jsoncons::json var3("Small string", 12, jsoncons::semantic_tag::none);
    CHECK(jsoncons::json_storage_kind::short_str == var3.storage_kind());
    jsoncons::json var4("Too long to fit in small string", 31, jsoncons::semantic_tag::none);
    CHECK(jsoncons::json_storage_kind::long_str == var4.storage_kind());
    jsoncons::json var5(true, jsoncons::semantic_tag::none);
    CHECK(jsoncons::json_storage_kind::boolean == var5.storage_kind());
    jsoncons::json var6(jsoncons::semantic_tag::none);
    CHECK(jsoncons::json_storage_kind::empty_object == var6.storage_kind());
    jsoncons::json var7{ jsoncons::null_type(), jsoncons::semantic_tag::none };
    CHECK(jsoncons::json_storage_kind::null == var7.storage_kind());
    jsoncons::json var8{ jsoncons::json::object(jsoncons::json::allocator_type()), jsoncons::semantic_tag::none };
    CHECK(jsoncons::json_storage_kind::object == var8.storage_kind());
    jsoncons::json var9(123456789.9, jsoncons::semantic_tag::none);
    CHECK(jsoncons::json_storage_kind::float64 == var9.storage_kind());
}

