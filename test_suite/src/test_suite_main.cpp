// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <jsoncons/json.hpp>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(test_suite_main)

BOOST_AUTO_TEST_CASE(test)
{
    json a;
    a["key1"] = "value1";

    json b;
    b["key2"] = json();
    b["key2"]["key3"] = std::move(a);

    json expected;
    expected["key1"] = "value1";
    BOOST_CHECK(expected == b["key2"]["key3"]);

    std::cout << a << std::endl;
    std::cout << (int)a.type_id() << std::endl;

    BOOST_CHECK(!(a.is_object() || a.is_array() || a.is_string()));
}

BOOST_AUTO_TEST_SUITE_END()

