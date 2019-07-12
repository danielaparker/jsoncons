// Copyright 2017 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <ctime>
#include <cstdint>
#include "data_structures.hpp"

using namespace jsoncons;

TEST_CASE("encode and decode json")
{
    json j(std::make_pair(false,std::string("foo")));

    SECTION("string test")
    {
        std::string s;
        encode_json(j, s);
        json result = decode_json<json>(s);
        CHECK(result == j);
    }

    SECTION("stream test")
    {
        std::stringstream ss;
        encode_json(j, ss);
        json result = decode_json<json>(ss);
        CHECK(result == j);
    }
}

TEST_CASE("encode and decode wjson")
{
    wjson j(std::make_pair(false,std::wstring(L"foo")));

    SECTION("string test")
    {
        std::wstring s;
        encode_json(j, s);
        wjson result = decode_json<wjson>(s);
        CHECK(result == j);
    }

    SECTION("stream test")
    {
        std::wstringstream ss;
        encode_json(j, ss);
        wjson result = decode_json<wjson>(ss);
        CHECK(result == j);
    }
}

TEST_CASE("convert_pair_test")
{
    auto val = std::make_pair(false,std::string("foo"));
    std::string s;

    jsoncons::encode_json(val, s);

    auto result = jsoncons::decode_json<std::pair<bool,std::string>>(s);

    CHECK(val == result);
}

TEST_CASE("convert_vector_test")
{
    std::vector<double> v = {1,2,3,4,5,6};

    std::string s;
    jsoncons::encode_json(v,s);

    auto result = jsoncons::decode_json<std::vector<double>>(s);

    REQUIRE(v.size() == result.size());
    for (size_t i = 0; i < result.size(); ++i)
    {
        CHECK(v[i] == result[i]);
    }
}

TEST_CASE("convert_map_test")
{
    std::map<std::string,double> m = {{"a",1},{"b",2}};

    std::string s;
    jsoncons::encode_json(m,s);
    auto result = jsoncons::decode_json<std::map<std::string,double>>(s);

    REQUIRE(result.size() == m.size());
    CHECK(m["a"] == result["a"]);
    CHECK(m["b"] == result["b"]);
}

TEST_CASE("convert_array_test")
{
    std::array<double,4> v{1,2,3,4};

    std::string s;
    jsoncons::encode_json(v,s);
    std::array<double, 4> result = jsoncons::decode_json<std::array<double,4>>(s);
    REQUIRE(result.size() == v.size());
    for (size_t i = 0; i < result.size(); ++i)
    {
        CHECK(v[i] == result[i]);
    }
}

#if !(defined(__GNUC__) && __GNUC__ <= 5)
TEST_CASE("convert_tuple_test")
{
    typedef std::map<std::string,std::tuple<std::string,std::string,double>> employee_collection;

    employee_collection employees = 
    { 
        {"John Smith",{"Hourly","Software Engineer",10000}},
        {"Jane Doe",{"Commission","Sales",20000}}
    };

    std::string s;
    jsoncons::encode_json(employees, s, jsoncons::indenting::indent);
    std::cout << "(1)\n" << s << std::endl;
    auto employees2 = jsoncons::decode_json<employee_collection>(s);
    REQUIRE(employees2.size() == employees.size());

    std::cout << "\n(2)\n";
    for (const auto& pair : employees2)
    {
        std::cout << pair.first << ": " << std::get<1>(pair.second) << std::endl;
    }
}
#endif
    
TEST_CASE("book_conversion_test")
{
    ns::book book_list{"Haruki Murakami", "Kafka on the Shore", 25.17};

    std::string s;
    encode_json(book_list, s, indenting::indent);

    std::cout << "s: " << s << "\n";

}

TEST_CASE("hiking_reputation")
{
    ns::hiking_reputation val("hiking", { ns::hiking_reputon{"HikingAsylum",ns::hiking_experience::advanced,"Marilyn C",0.9} });

    SECTION("1")
    {
        std::string s;
        encode_json(val, s);
        auto val2 = decode_json<ns::hiking_reputation>(s);
        CHECK(val2 == val);
    }

    SECTION("2")
    {
        std::string s;
        encode_json(val, s, indenting::indent);
        auto val2 = decode_json<ns::hiking_reputation>(s);
        CHECK(val2 == val);
    }

    SECTION("3")
    {
        std::string s;
        json_options options;
        encode_json(val, s, options, indenting::indent);
        auto val2 = decode_json<ns::hiking_reputation>(s, options);
        CHECK(val2 == val);
    }

    SECTION("4")
    {
        std::string s;
        encode_json(ojson(), val, s);
        auto val2 = decode_json<ns::hiking_reputation>(ojson(), s);
        CHECK(val2 == val);
    }

    SECTION("5")
    {
        std::string s;
        encode_json(ojson(), val, s, indenting::indent);
        auto val2 = decode_json<ns::hiking_reputation>(ojson(), s);
        CHECK(val2 == val);
    }

    SECTION("6")
    {
        std::string s;
        json_options options;
        encode_json(ojson(), val, s, options, indenting::indent);
        auto val2 = decode_json<ns::hiking_reputation>(ojson(), s, options);
        CHECK(val2 == val);
    }

    SECTION("os 1")
    {
        std::stringstream os;
        encode_json(val, os);
        auto val2 = decode_json<ns::hiking_reputation>(os);
        CHECK(val2 == val);
    }

    SECTION("os 2")
    {
        std::stringstream os;
        encode_json(val, os, indenting::indent);
        auto val2 = decode_json<ns::hiking_reputation>(os);
        CHECK(val2 == val);
    }

    SECTION("os 3")
    {
        std::stringstream os;
        json_options options;
        encode_json(val, os, options, indenting::indent);
        auto val2 = decode_json<ns::hiking_reputation>(os, options);
        CHECK(val2 == val);
    }

    SECTION("os 4")
    {
        std::stringstream os;
        encode_json(ojson(), val, os);
        auto val2 = decode_json<ns::hiking_reputation>(ojson(), os);
        CHECK(val2 == val);
    }

    SECTION("os 5")
    {
        std::stringstream os;
        encode_json(ojson(), val, os, indenting::indent);
        auto val2 = decode_json<ns::hiking_reputation>(ojson(), os);
        CHECK(val2 == val);
    }

    SECTION("os 6")
    {
        std::stringstream os;
        json_options options;
        encode_json(ojson(), val, os, options, indenting::indent);
        auto val2 = decode_json<ns::hiking_reputation>(ojson(), os, options);
        CHECK(val2 == val);
    }
}



