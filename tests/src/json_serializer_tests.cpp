// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;
using boost::numeric::ublas::matrix;

BOOST_AUTO_TEST_SUITE(json_serializer_tests)

BOOST_AUTO_TEST_CASE(test_byte_string_serialization)
{
    const uint8_t bs[] = {'H','e','l','l','o'};
    json j(byte_string_view(bs,sizeof(bs)));

    std::ostringstream os;
    os << j;

    std::string expected; 
    expected.push_back('\"');
    encode_base64url(bs,sizeof(bs),expected);
    expected.push_back('\"');

    //std::cout << expected << " " << os.str() << std::endl;

    BOOST_CHECK(expected == os.str());
}

BOOST_AUTO_TEST_CASE(test_direct_serialization)
{
    matrix<double> A(2, 2);
    A(0, 0) = 1;
    A(0, 1) = 2;
    A(1, 0) = 3;
    A(1, 1) = 4;

    json_serializing_options options;

    std::ostringstream os1;

    json_serializer os(os1, options, jsoncons::indenting::indent); // pretty printing
    os.begin_document();
    os.begin_array();
    for (size_t i = 0; i < A.size1(); ++i)
    {
        os.begin_array();
        for (size_t j = 0; j < A.size2(); ++j)
        {
            os.double_value(A(i, j));
        }
        os.end_array();
    }
    os.end_array();
    os.end_document();

    std::string expected1 = R"([
    [1.0,2.0],
    [3.0,4.0]
])";
    BOOST_CHECK_EQUAL(expected1,os1.str());
}

BOOST_AUTO_TEST_SUITE_END()


