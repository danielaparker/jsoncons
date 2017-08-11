// Copyright 2017 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include <jsoncons/json_stream_traits.hpp>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <ctime>
#include <cstdint>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(json_stream_traits_tests)

BOOST_AUTO_TEST_CASE(test_uinteger)
{
    std::ostringstream oss;

    dump(10,oss);
    std::cout << oss.str() << std::endl;
}

BOOST_AUTO_TEST_CASE(test_integer)
{
    std::ostringstream oss;

    dump(-10,oss);
    std::cout << oss.str() << std::endl;
}

BOOST_AUTO_TEST_CASE(test_bool)
{
    std::ostringstream oss;

    dump(true,oss);
    std::cout << oss.str() << std::endl;
}

BOOST_AUTO_TEST_CASE(test_double)
{
    std::ostringstream oss;

    dump(10.5,oss);
    std::cout << oss.str() << std::endl;
}

BOOST_AUTO_TEST_CASE(test_vector)
{
    std::ostringstream oss;

    std::vector<double> v = {1,2,3,4,5,6};
    dump(v,oss);

    std::cout << oss.str() << std::endl;
}

BOOST_AUTO_TEST_CASE(test_array)
{
    std::ostringstream oss;

    std::array<double,4> v{1,2,3,4};
    dump(v,oss);

    std::cout << oss.str() << std::endl;
}

BOOST_AUTO_TEST_CASE(test_vector_of_vector)
{
    std::ostringstream oss;

    std::vector<std::vector<double>> v = {std::vector<double>{1,2,3,4,5,6}};
    dump(v,oss);

    std::cout << oss.str() << std::endl;
}

BOOST_AUTO_TEST_CASE(test_map)
{
    std::ostringstream oss;

    std::map<std::string,double> m = {{"a",1},{"b",2}};
    dump(m,oss);

    std::cout << oss.str() << std::endl;
}

BOOST_AUTO_TEST_CASE(test_map_string)
{
    std::ostringstream oss;

    std::map<std::string,std::string> m = {{"a","1"},{"b","2"}};
    dump(m,oss);

    std::cout << oss.str() << std::endl;
}

BOOST_AUTO_TEST_CASE(test_tuple_bool_string_integer)
{
    std::ostringstream oss;

    auto val = std::make_tuple(false,std::string("foo"),1);
    dump(val,oss);

    std::cout << oss.str() << std::endl;
}

BOOST_AUTO_TEST_CASE(test_tuple_bool_charptr_integer)
{
    std::ostringstream oss;

    auto val = std::make_tuple(false,"foo",1);
    dump(val,oss);

    std::cout << oss.str() << std::endl;
}

BOOST_AUTO_TEST_CASE(test_pair)
{
    std::ostringstream oss;

    auto val = std::make_pair(false,"foo");
    dump(val,oss);

    std::cout << oss.str() << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()


