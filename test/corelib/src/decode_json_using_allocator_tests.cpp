// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>

#include <map>
#include <vector>
#include <set>
#include <string>
#include <forward_list>
#include <list>
#include <catch/catch.hpp>

using namespace jsoncons;

#if defined(JSONCONS_HAS_STATEFUL_ALLOCATOR) && JSONCONS_HAS_STATEFUL_ALLOCATOR == 1

#include <scoped_allocator>
#include <common/mock_stateful_allocator.hpp>

template <typename T>
using cust_allocator = std::scoped_allocator_adaptor<mock_stateful_allocator<T>>;

TEST_CASE("decode_json using allocator")
{
    using char_allocator_type = mock_stateful_allocator<char>;
    using cust_string = std::basic_string<char, std::char_traits<char>, char_allocator_type>;

    SECTION("decode string test")
    {
        using cust_allocator_type = cust_allocator<cust_string>;

        cust_allocator_type alloc(1);
        auto aset = make_alloc_set(alloc);

        cust_string s{aset.get_allocator()};

        std::string str = R"("Hello World")";

        auto result = jsoncons::try_decode_json<cust_string>(aset, str);
        REQUIRE(result);
        //REQUIRE(3 == result.size());
        //CHECK("1" == result[0].c_str());
    }

    SECTION("std::vector of string")
    {
        using cust_allocator_type = cust_allocator<cust_string>;

        cust_allocator_type alloc(1);
        auto aset = make_alloc_set(alloc);

        std::string str = R"(["1","2","3"])";

        auto result = jsoncons::try_decode_json<std::vector<cust_string,cust_allocator_type>>(aset, str);
        REQUIRE(result);
        REQUIRE(3 == result->size());
        //CHECK("1" == result[0].c_str());
    }

    SECTION("std::forward_list of string")
    {
        using cust_allocator_type = cust_allocator<cust_string>;
        using value_type = std::forward_list<cust_string,cust_allocator_type>;

        cust_allocator_type alloc(1);
        auto aset = make_alloc_set(alloc);

        std::string str = R"(["1","2","3"])";

        auto result = jsoncons::try_decode_json<value_type>(aset, str);
        REQUIRE(result);
        value_type& val(*result);
        REQUIRE(!val.empty());
        //CHECK("1" == result[0].c_str());
    }

    SECTION("std::list of string")
    {
        using cust_allocator_type = cust_allocator<cust_string>;
        using value_type = std::list<cust_string,cust_allocator_type>;

        cust_allocator_type alloc(1);
        auto aset = make_alloc_set(alloc);

        std::string str = R"(["1","2","3"])";

        auto result = jsoncons::try_decode_json<value_type>(aset, str);
        REQUIRE(result);
        value_type& val(*result);
        REQUIRE(!val.empty());
        //CHECK("1" == result[0].c_str());
    }

    SECTION("decode vector of char test")
    {
        using cust_allocator_type = cust_allocator<char>;

        cust_allocator_type alloc(1);
        auto aset = make_alloc_set(alloc);

        std::string str = R"([1,2,3])";

        auto result = jsoncons::try_decode_json<std::vector<char,cust_allocator_type>>(aset, str);
        REQUIRE(result);
        REQUIRE(3 == result->size());
        //CHECK("1" == result[0].c_str());
    }

    SECTION("decode vector of int test")
    {
        using cust_allocator_type = cust_allocator<int>;

        cust_allocator_type alloc(1);
        auto aset = make_alloc_set(alloc);

        std::string str = R"([1,2,3])";

        auto result = jsoncons::try_decode_json<std::vector<int, cust_allocator_type>>(aset, str);
        REQUIRE(result);
        REQUIRE(3 == result->size());
        //CHECK("1" == result[0].c_str());
    }

    SECTION("decode pair test")
    {
        using cust_allocator_type = cust_allocator<cust_string>;

        cust_allocator_type alloc(1);
        auto aset = make_alloc_set(alloc);

        std::string str = R"(["1",2])";

        auto result = jsoncons::try_decode_json<std::pair<cust_string, int>>(aset, str);
        REQUIRE(result);
        CHECK(std::string("1") == result->first.c_str());
        CHECK(2 == result->second);
    }

    SECTION("decode map string -> string test")
    {
        using cust_allocator_type = cust_allocator<std::pair<const cust_string,cust_string>>;

        cust_allocator_type alloc(1);
        auto aset = make_alloc_set(alloc);

        std::string str = R"({"1" : "1", "2" : "2", "3" : "3"})";

        auto result = jsoncons::try_decode_json<std::map<cust_string,cust_string,std::less<cust_string>,cust_allocator_type>>(aset, str);
        REQUIRE(result);
        REQUIRE(3 == result->size());
        //CHECK("1" == result[0].c_str());
    }

    /*SECTION("decode std::array of string")
    {
        using cust_allocator_type = cust_allocator<char>;

        cust_allocator_type alloc(1);
        auto aset = make_alloc_set(alloc);

        std::string str = R"([1,2,3])";

        auto result = jsoncons::try_decode_json<std::array<cust_string, 3>>(aset, str);
        REQUIRE(result);
        REQUIRE(3 == result->size());
        //CHECK("1" == result[0].c_str());
    }*/

    SECTION("decode set of string")
    {
        using cust_allocator_type = cust_allocator<cust_string>;

        cust_allocator_type alloc(1);
        auto aset = make_alloc_set(alloc);

        std::string str = R"(["1","2","3"])";

        auto result = jsoncons::try_decode_json<std::set<cust_string,std::less<cust_string>,cust_allocator_type>>(aset, str);
        REQUIRE(result);
        REQUIRE(3 == result->size());
        //CHECK("1" == result[0].c_str());
    }
}

#endif