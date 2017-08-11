// Copyright 2017 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <jsoncons/json_stream_traits.hpp>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <ctime>
#include <cstdint>

using boost::numeric::ublas::matrix;

namespace jsoncons
{
    template <>
    struct json_stream_traits<char,matrix<double>>
    {
        static void encode(const matrix<double>& val, json_output_handler& handler)
        {
            handler.begin_array();
            for (size_t i = 0; i < val.size1(); ++i)
            {
                handler.begin_array();
                for (size_t j = 0; j < val.size2(); ++j)
                {
                    handler.double_value(val(i, j),0);
                }
                handler.end_array();
            }
            handler.end_array();
        }
    };
};

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(json_stream_traits_tests)

BOOST_AUTO_TEST_CASE(test_matrix)
{
    std::ostringstream oss;

    matrix<double> A(2, 2);
    A(0, 0) = 1;
    A(0, 1) = 2;
    A(1, 0) = 3;
    A(1, 1) = 4;

    dump(A,oss,true);

    std::cout << oss.str() << std::endl;
}

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


