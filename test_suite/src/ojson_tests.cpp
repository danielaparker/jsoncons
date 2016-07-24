// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include "jsoncons/json.hpp"
#include "jsoncons/json_serializer.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <new>
#include <boost/optional.hpp>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(ojson_test_suite)

BOOST_AUTO_TEST_CASE(test_object)
{
    ojson o = ojson::parse(R"(
    {
        "d" : 4,
        "a" : 1,
        "b" : 2
    }
    )");

    ojson o2 = o;
    BOOST_CHECK(o == o2);

    ojson o3 = o;
    o3["a"] = 2;
    //BOOST_CHECK_EQUAL(o,o3);

    std::cout << o << std::endl;

    BOOST_CHECK_EQUAL(1,o["a"].as<int>());
    BOOST_CHECK_EQUAL(2,o["b"].as<int>());
    BOOST_CHECK_EQUAL(4,o["d"].as<int>());

    o.set("c",3);

    auto it = o.find("b");
    BOOST_CHECK(it != o.members().end());
    o.set(it,"e",5);

    std::cout << o << std::endl;

    o.erase("d");

    std::cout << o << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()

