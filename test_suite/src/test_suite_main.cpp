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
    std::allocator<char> allocator;
    json o = json::object(allocator);
    o.set("name too long for small string optimization", "value too long for small string optimization");
    std::cout << o << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()

