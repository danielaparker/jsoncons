// Copyright 2013-2023 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <common/FreeListAllocator.hpp>
#include <scoped_allocator>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <ctime>
#include <cstdint>

using namespace jsoncons;

#if defined(JSONCONS_HAS_STATEFUL_ALLOCATOR)

template<typename T>
using ScopedTestAllocator = std::scoped_allocator_adaptor<FreeListAllocator<T>>;

TEST_CASE("decode_json with work allocator")
{
    ScopedTestAllocator<char> alloc(1);

    SECTION("convert_vector_test")
    {
        std::vector<double> v = {1,2,3,4,5,6};

        std::string json_text;
        jsoncons::encode_json(v,json_text);

        auto result = jsoncons::decode_json<std::vector<double>>(
            wrap_allocators(work_allocator_arg, alloc), 
            json_text);

        REQUIRE(v.size() == result.size());
        for (std::size_t i = 0; i < result.size(); ++i)
        {
            CHECK(v[i] == result[i]);
        }
    }

    SECTION("convert_map_test")
    {
        std::map<std::string,double> m = {{"a",1},{"b",2}};

        std::string json_text;
        jsoncons::encode_json(m,json_text);
        auto result = jsoncons::decode_json<std::map<std::string,double>>(
            wrap_allocators(work_allocator_arg, alloc), 
            json_text);
        REQUIRE(result.size() == m.size());
        CHECK(m["a"] == result["a"]);
        CHECK(m["b"] == result["b"]);
    }

    SECTION("convert vector of vector test")
    {
        std::vector<double> u{1,2,3,4};
        std::vector<std::vector<double>> v{u,u};

        std::string json_text;
        jsoncons::encode_json(v,json_text);
        auto result = jsoncons::decode_json<std::vector<std::vector<double>>>(
            wrap_allocators(work_allocator_arg, alloc), 
            json_text);
        REQUIRE(result.size() == v.size());
        for (const auto& item : result)
        {
            REQUIRE(item.size() == u.size());
            CHECK(item[0] == 1);
            CHECK(item[1] == 2);
            CHECK(item[2] == 3);
            CHECK(item[3] == 4);
        }
    }

    #if !(defined(__GNUC__) && __GNUC__ <= 5)

    SECTION("convert_tuple_test")
    {
        using employee_collection = std::map<std::string,std::tuple<std::string,std::string,double>>;

        employee_collection employees = 
        { 
            {"John Smith",{"Hourly","Software Engineer",10000}},
            {"Jane Doe",{"Commission","Sales",20000}}
        };

        std::string json_text;
        jsoncons::encode_json_pretty(employees, json_text);
        auto employees2 = jsoncons::decode_json<employee_collection>(
            wrap_allocators(work_allocator_arg, alloc), 
            json_text);
        REQUIRE(employees2.size() == employees.size());
        CHECK(employees2 == employees);
    }

    #endif
}

#endif // JSONCONS_HAS_STATEFUL_ALLOCATOR
