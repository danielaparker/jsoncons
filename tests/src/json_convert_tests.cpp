// Copyright 2017 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <jsoncons/json_convert.hpp>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <ctime>
#include <cstdint>

BOOST_AUTO_TEST_SUITE(json_convert_tests)

BOOST_AUTO_TEST_CASE(convert_pair_test)
{
    auto val = std::make_pair(false,std::string("foo"));
    std::string s;

    jsoncons::json_convert<std::pair<bool,std::string>>::encode(val,s);

    auto result = jsoncons::json_convert<std::pair<bool,std::string>>::decode(s);

    BOOST_CHECK(val == result);
}

BOOST_AUTO_TEST_CASE(convert_vector_test)
{
    std::vector<double> v = {1,2,3,4,5,6};

    std::string s;
    jsoncons::json_convert<std::vector<double>>::encode(v,s);

    auto result = jsoncons::json_convert<std::vector<double>>::decode(s);

    BOOST_REQUIRE(v.size() == result.size());
    for (size_t i = 0; i < result.size(); ++i)
    {
        BOOST_CHECK_EQUAL(v[i],result[i]);
    }
}

BOOST_AUTO_TEST_CASE(convert_map_test)
{
    std::map<std::string,double> m = {{"a",1},{"b",2}};

    std::string s;
    jsoncons::json_convert<std::map<std::string,double>>::encode(m,s);

    std::cout << s << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()


