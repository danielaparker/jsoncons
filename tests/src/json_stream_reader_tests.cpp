// Copyright 2018 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <jsoncons/json_stream_reader.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

TEST_CASE("json_stream_reader string_value test")
{
    std::string s = R"("Tom")";
    std::istringstream is(s);

    json_stream_reader reader(is);

    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::string_value);
    CHECK(reader.current().as<std::string>() == std::string("Tom"));
    CHECK(reader.current().as<jsoncons::string_view>() == jsoncons::string_view("Tom"));
    reader.next();
    CHECK(reader.done());
}

TEST_CASE("json_stream_reader string_value as<int> test")
{
    std::string s = R"("-100")";
    std::istringstream is(s);

    json_stream_reader reader(is);

    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::string_value);
    CHECK(reader.current().as<int>() == -100);
    reader.next();
    CHECK(reader.done());
}

TEST_CASE("json_stream_reader string_value as<unsigned> test")
{
    std::string s = R"("100")";
    std::istringstream is(s);

    json_stream_reader reader(is);

    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::string_value);
    CHECK(reader.current().as<int>() == 100);
    CHECK(reader.current().as<unsigned>() == 100);
    reader.next();
    CHECK(reader.done());
}

TEST_CASE("json_stream_reader null_value test")
{
    std::string s = "null";
    std::istringstream is(s);

    json_stream_reader reader(is);

    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::null_value);
    reader.next();
    CHECK(reader.done());
}

TEST_CASE("json_stream_reader bool_value test")
{
    std::string s = "false";
    std::istringstream is(s);

    json_stream_reader reader(is);

    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::bool_value);
    reader.next();
    CHECK(reader.done());
}

TEST_CASE("json_stream_reader int64_value test")
{
    std::string s = "-100";
    std::istringstream is(s);

    json_stream_reader reader(is);

    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::int64_value);
    CHECK(reader.current().as<int>() == -100);
    reader.next();
    CHECK(reader.done());
}

TEST_CASE("json_stream_reader uint64_value test")
{
    std::string s = "100";
    std::istringstream is(s);

    json_stream_reader reader(is);

    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::uint64_value);
    CHECK(reader.current().as<int>() == 100);
    CHECK(reader.current().as<unsigned>() == 100);
    reader.next();
    CHECK(reader.done());
}

TEST_CASE("json_stream_reader string_value as bignum test")
{
    std::string s = "-18446744073709551617";
    std::istringstream is("\""+s+"\"");

    json_stream_reader reader(is);

    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::string_value);
    CHECK(s == reader.current().as<std::string>());
    bignum c = reader.current().as<bignum>();
    CHECK(bignum("-18446744073709551617") == c);
    reader.next();
    CHECK(reader.done());
}

TEST_CASE("json_stream_reader bignum_value as bignum")
{
    std::string s = "-18446744073709551617";
    std::istringstream is(s);

    json_stream_reader reader(is);

    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::bignum_value);
    bignum c = reader.current().as<bignum>();
    CHECK(bignum(s) == c);
    reader.next();
    CHECK(reader.done());
}

TEST_CASE("json_stream_reader double_value test")
{
    std::string s = "100.0";
    std::istringstream is(s);

    json_stream_reader reader(is);

    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::double_value);
    reader.next();
    CHECK(reader.done());
}

TEST_CASE("json_stream_reader array_value test")
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

    json_stream_reader reader(is);

    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::begin_array);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::begin_object);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::uint64_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::string_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::string_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::uint64_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::end_object);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::begin_object);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::uint64_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::string_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::string_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::uint64_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::end_object);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::begin_object);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::uint64_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::string_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::string_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::uint64_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::end_object);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::end_array);
    reader.next();
    CHECK(reader.done());
}

TEST_CASE("json_stream_reader object_value test")
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

    json_stream_reader reader(is);

    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::begin_object);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::uint64_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::string_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::string_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::name);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::uint64_value);
    reader.next();
    REQUIRE_FALSE(reader.done());
    CHECK(reader.current().event_type() == stream_event_type::end_object);
    reader.next();
    CHECK(reader.done());
}




