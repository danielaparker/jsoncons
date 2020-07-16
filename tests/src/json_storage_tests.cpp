// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json_exception.hpp>
#include <jsoncons/json.hpp>
#include <catch/catch.hpp>
#include <new>
#include <string>

using namespace jsoncons;

TEST_CASE("json storage tests")
{
    json var1(int64_t(-100), semantic_tag::none);
    CHECK(storage_kind::int64_value == var1.storage());
    json var2(uint64_t(100), semantic_tag::none);
    CHECK(storage_kind::uint64_value == var2.storage());
    json var3("Small string", 12, semantic_tag::none);
    CHECK(storage_kind::short_string_value == var3.storage());
    json var4("Too long to fit in small string", 31, semantic_tag::none);
    CHECK(storage_kind::long_string_value == var4.storage());
    json var5(true, semantic_tag::none);
    CHECK(storage_kind::bool_value == var5.storage());
    json var6(semantic_tag::none);
    CHECK(storage_kind::empty_object_value == var6.storage());
    json var7{ null_type(), semantic_tag::none };
    CHECK(storage_kind::null_value == var7.storage());
    json var8{ json::object(json::allocator_type()), semantic_tag::none };
    CHECK(storage_kind::object_value == var8.storage());
    json var9(123456789.9, semantic_tag::none);
    CHECK(storage_kind::double_value == var9.storage());
}

