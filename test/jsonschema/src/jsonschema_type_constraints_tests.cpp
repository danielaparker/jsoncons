// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>
#include <common/mock_stateful_allocator.hpp>
#include <catch/catch.hpp>
#include <scoped_allocator>
#include <string>
#include <utility>

namespace
{
    namespace jsonschema = jsoncons::jsonschema;

    template <typename Json, typename... Args>
    using make_schema_result = decltype(jsonschema::make_json_schema(
        std::declval<Json>(), std::declval<Args>()...));

    template <typename Json>
    void check_schema_overloads(bool supported)
    {
        using resolver_type = Json (*)(const jsoncons::uri&);
        using options_type = jsonschema::evaluation_options;
        CHECK((jsoncons::ext_traits::is_detected<make_schema_result, Json>::value == supported));
        CHECK((jsoncons::ext_traits::is_detected<make_schema_result, Json, options_type>::value == supported));
        CHECK((jsoncons::ext_traits::is_detected<make_schema_result, Json, std::string>::value == supported));
        CHECK((jsoncons::ext_traits::is_detected<make_schema_result, Json, std::string, options_type>::value == supported));
        CHECK((jsoncons::ext_traits::is_detected<make_schema_result, Json, resolver_type>::value == supported));
        CHECK((jsoncons::ext_traits::is_detected<make_schema_result, Json, resolver_type, options_type>::value == supported));
        CHECK((jsoncons::ext_traits::is_detected<make_schema_result, Json, std::string, resolver_type>::value == supported));
        CHECK((jsoncons::ext_traits::is_detected<make_schema_result, Json, std::string, resolver_type, options_type>::value == supported));
    }

    template <typename Json>
    void check_schema_validation()
    {
        auto root = Json::parse(R"({"$ref":"https://example.com/string"})");
        auto resolver = [](const jsoncons::uri&) { return Json::parse(R"({"type":"string"})"); };
        auto resolved = jsonschema::make_json_schema(root, resolver);
        CHECK(resolved.is_valid(Json("value")));
        CHECK_FALSE(resolved.is_valid(Json(42)));
        auto retrieved = jsonschema::make_json_schema(root, "https://example.com/root", resolver);
        CHECK(retrieved.is_valid(Json("value")));
        CHECK_FALSE(retrieved.is_valid(Json(42)));

        root = Json::parse(R"({"type":"string"})");
        auto local = jsonschema::make_json_schema(root);
        CHECK(local.is_valid(Json("value")));
        CHECK_FALSE(local.is_valid(Json(42)));
        auto local_retrieved = jsonschema::make_json_schema(root, "https://example.com/root");
        CHECK(local_retrieved.is_valid(Json("value")));
        CHECK_FALSE(local_retrieved.is_valid(Json(42)));
    }
}

TEST_CASE("jsonschema supported JSON types")
{
    check_schema_overloads<jsoncons::json>(true);
    check_schema_overloads<jsoncons::ojson>(true);
    check_schema_validation<jsoncons::json>();
    check_schema_validation<jsoncons::ojson>();
    using scoped_json = jsoncons::basic_json<char, jsoncons::sorted_policy,
        std::scoped_allocator_adaptor<std::allocator<char>, std::allocator<char>>>;
    check_schema_overloads<scoped_json>(true);

#if defined(JSONCONS_HAS_POLYMORPHIC_ALLOCATOR) && JSONCONS_HAS_POLYMORPHIC_ALLOCATOR == 1
    check_schema_overloads<jsoncons::pmr::json>(true);
    check_schema_overloads<jsoncons::pmr::ojson>(true);
    check_schema_validation<jsoncons::pmr::json>();
    check_schema_validation<jsoncons::pmr::ojson>();
#endif
}

TEST_CASE("jsonschema unsupported JSON types")
{
    check_schema_overloads<jsoncons::wjson>(false);
    check_schema_overloads<jsoncons::wojson>(false);
    using stateful_json = jsoncons::basic_json<char, jsoncons::sorted_policy,
        std::scoped_allocator_adaptor<mock_stateful_allocator<char>>>;
    check_schema_overloads<stateful_json>(false);
    using inner_stateful_json = jsoncons::basic_json<char, jsoncons::sorted_policy,
        std::scoped_allocator_adaptor<std::allocator<char>, mock_stateful_allocator<char>>>;
    check_schema_overloads<inner_stateful_json>(false);
}
