// Copyright 2018 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <jsoncons/json_event_reader.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

TEST_CASE("json_event_reader string_value test")
{
    std::string s = R"("Tom")";
    std::istringstream is(s);

    json_event_reader reader(is);

    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::begin_document);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::string_value);
    reader.next();
    CHECK(reader.current().event_type() == json_event_type::end_document);
    CHECK(reader.done());
}

TEST_CASE("json_event_reader null_value test")
{
    std::string s = "null";
    std::istringstream is(s);

    json_event_reader reader(is);

    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::begin_document);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::null_value);
    reader.next();
    CHECK(reader.current().event_type() == json_event_type::end_document);
    CHECK(reader.done());
}

TEST_CASE("json_event_reader bool_value test")
{
    std::string s = "false";
    std::istringstream is(s);

    json_event_reader reader(is);

    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::begin_document);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::bool_value);
    reader.next();
    CHECK(reader.current().event_type() == json_event_type::end_document);
    CHECK(reader.done());
}

TEST_CASE("json_event_reader int64_value test")
{
    std::string s = "-100";
    std::istringstream is(s);

    json_event_reader reader(is);

    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::begin_document);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::int64_value);
    reader.next();
    CHECK(reader.current().event_type() == json_event_type::end_document);
    CHECK(reader.done());
}

TEST_CASE("json_event_reader uint64_value test")
{
    std::string s = "100";
    std::istringstream is(s);

    json_event_reader reader(is);

    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::begin_document);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::uint64_value);
    reader.next();
    CHECK(reader.current().event_type() == json_event_type::end_document);
    CHECK(reader.done());
}

TEST_CASE("json_event_reader bignum_value test")
{
    std::string s = "-18446744073709551617";
    std::istringstream is(s);

    json_event_reader reader(is);

    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::begin_document);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::bignum_value);
    reader.next();
    CHECK(reader.current().event_type() == json_event_type::end_document);
    CHECK(reader.done());
}

TEST_CASE("json_event_reader double_value test")
{
    std::string s = "100.0";
    std::istringstream is(s);

    json_event_reader reader(is);

    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::begin_document);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::double_value);
    reader.next();
    CHECK(reader.current().event_type() == json_event_type::end_document);
    CHECK(reader.done());
}

TEST_CASE("json_event_reader array_value test")
{
    std::string s = R"(
    [
        {
            "enrollmentNo" : 100,
            "firstName" : "Tom",
            "lastName" : "Cochrane",
            "mark" : 55              
        },
        {
            "enrollmentNo" : 101,
            "firstName" : "Catherine",
            "lastName" : "Smith",
            "mark" : 95},
        {
            "enrollmentNo" : 102,
            "firstName" : "William",
            "lastName" : "Skeleton",
            "mark" : 60              
        }
    ]
    )";

    std::istringstream is(s);

    json_event_reader reader(is);

    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::begin_document);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::begin_array);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::begin_object);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::uint64_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::string_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::string_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::uint64_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::end_object);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::begin_object);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::uint64_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::string_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::string_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::uint64_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::end_object);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::begin_object);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::uint64_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::string_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::string_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::uint64_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::end_object);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::end_array);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::end_document);
    reader.next();
    CHECK(reader.done());
}

TEST_CASE("json_event_reader object_value test")
{
    std::string s = R"(
        {
            "enrollmentNo" : 100,
            "firstName" : "Tom",
            "lastName" : "Cochrane",
            "mark" : 55              
        },
        {
            "enrollmentNo" : 101,
            "firstName" : "Catherine",
            "lastName" : "Smith",
            "mark" : 95},
        {
            "enrollmentNo" : 102,
            "firstName" : "William",
            "lastName" : "Skeleton",
            "mark" : 60              
        }
    )";

    std::istringstream is(s);

    json_event_reader reader(is);

    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::begin_document);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::begin_object);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::uint64_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::string_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::string_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::uint64_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::end_object);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == json_event_type::end_document);
    reader.next();
    CHECK(reader.done());
}




