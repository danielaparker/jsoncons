// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>

#include <map>
#include <vector>
#include <catch/catch.hpp>

using namespace jsoncons;

#if defined(JSONCONS_HAS_STATEFUL_ALLOCATOR) && JSONCONS_HAS_STATEFUL_ALLOCATOR == 1

#include <scoped_allocator>
#include <common/mock_stateful_allocator.hpp>

using char_allocator_type = mock_stateful_allocator<char>;
template <typename T>
using cust_allocator = std::scoped_allocator_adaptor<mock_stateful_allocator<T>>;
using cust_string = std::basic_string<char, std::char_traits<char>, char_allocator_type>;

using cust_allocator_type = cust_allocator<cust_string>;

TEST_CASE("decode_json using allocator")
{
    cust_allocator_type alloc(1);
    auto aset = make_alloc_set(alloc);

    SECTION("decode string test")
    {
        cust_string s{aset.get_allocator()};

        std::string str = R"("Hello World")";

        auto result = jsoncons::try_decode_json<cust_string>(aset, str);
        REQUIRE(result);
        //REQUIRE(3 == result.size());
        //CHECK("1" == result[0].c_str());
    }

    /*SECTION("convert_vector_test")
    {
        std::vector<cust_string, cust_allocator_type> v{aset.get_allocator()};
        using element_type = std::vector<cust_string, cust_allocator_type>::value_type;

        cust_allocator_type a = aset.get_allocator();
        v.push_back(element_type(a));

        std::string str = R"(["1","2","3"])";

        //auto result = jsoncons::try_decode_json<std::vector<cust_string_type,cust_allocator_type>>(aset, str);
        //REQUIRE(result);
        //REQUIRE(3 == result.size());
        //CHECK("1" == result[0].c_str());
    }*/
}

#endif