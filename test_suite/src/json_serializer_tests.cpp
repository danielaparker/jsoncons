// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include "jsoncons/json.hpp"
#include "jsoncons/json_serializer.hpp"
#include "my_custom_data.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using jsoncons::json_serializer;
using jsoncons::output_format;
using jsoncons::json;
using jsoncons::wjson;
using jsoncons::basic_json_reader;
using std::string;
using boost::numeric::ublas::matrix;

BOOST_AUTO_TEST_CASE(test_direct_serialization)
{
    matrix<double> A(2, 2);
    A(0, 0) = 1;
    A(0, 1) = 2;
    A(1, 0) = 3;
    A(1, 1) = 4;

    output_format format;
    json_serializer os(std::cout, format, true); // pretty printing
    os.begin_array();
    for (size_t i = 0; i < A.size1(); ++i)
    {
        os.begin_array();
        for (size_t j = 0; j < A.size2(); ++j)
        {
            os.value(A(i, j));
        }
        os.end_array();
    }
    os.end_array();
}

BOOST_AUTO_TEST_CASE(test_double_to_string)
{
    double x = 1.0e100;
    std::string s = jsoncons::double_to_string<char>(x, 16);
    BOOST_CHECK(s == std::string("1.0e+100") || s == std::string("1.0e100"));

    x = 1.0e-100;
    s = jsoncons::double_to_string<char>(x, 16);
    BOOST_CHECK(s == std::string("1.0e-100"));

    x = -1.0e+100;
    s = jsoncons::double_to_string<char>(x, 16);
    BOOST_CHECK(s == std::string("-1.0e+100") || s == std::string("-1.0e100"));

    x = -1.0e-100;
    s = jsoncons::double_to_string<char>(x, 16);
    BOOST_CHECK(s == std::string("-1.0e-100"));

    x = 0;
    s = jsoncons::double_to_string<char>(x, 16);
    BOOST_CHECK(s == std::string("0.0"));

    x = -0;
    s = jsoncons::double_to_string<char>(x, 16);
    BOOST_CHECK(s == std::string("0.0"));

    x = 1;
    s = jsoncons::double_to_string<char>(x, 16);
    BOOST_CHECK(s == std::string("1.0"));

    x = -1;
    s = jsoncons::double_to_string<char>(x, 16);
    BOOST_CHECK(s == std::string("-1.0"));

    x = 10;
    s = jsoncons::double_to_string<char>(x, 16);
    BOOST_CHECK(s == std::string("10.0"));

    x = -10;
    s = jsoncons::double_to_string<char>(x, 16);
    BOOST_CHECK(s == std::string("-10.0"));

    x = 11;
    s = jsoncons::double_to_string<char>(x, 16);
    BOOST_CHECK(s == std::string("11.0"));

    x = -11;
    s = jsoncons::double_to_string<char>(x, 16);
    BOOST_CHECK(s == std::string("-11.0"));
}

BOOST_AUTO_TEST_CASE(test_double_to_wstring)
{
    double x = 1.0e100;
    std::wstring s = jsoncons::double_to_string<wchar_t>(x, 16);
    BOOST_CHECK(s == std::wstring(L"1.0e+100") || s == std::wstring(L"1.0e100"));

    x = 1.0e-100;
    s = jsoncons::double_to_string<wchar_t>(x, 16);
    BOOST_CHECK(s == std::wstring(L"1.0e-100"));

    x = -1.0e+100;
    s = jsoncons::double_to_string<wchar_t>(x, 16);
    BOOST_CHECK(s == std::wstring(L"-1.0e+100") || s == std::wstring(L"-1.0e100"));

    x = -1.0e-100;
    s = jsoncons::double_to_string<wchar_t>(x, 16);
    BOOST_CHECK(s == std::wstring(L"-1.0e-100"));

    x = 0;
    s = jsoncons::double_to_string<wchar_t>(x, 16);
    BOOST_CHECK(s == std::wstring(L"0.0"));

    x = -0;
    s = jsoncons::double_to_string<wchar_t>(x, 16);
    BOOST_CHECK(s == std::wstring(L"0.0"));

    x = 1;
    s = jsoncons::double_to_string<wchar_t>(x, 16);
    BOOST_CHECK(s == std::wstring(L"1.0"));

    x = -1;
    s = jsoncons::double_to_string<wchar_t>(x, 16);
    BOOST_CHECK(s == std::wstring(L"-1.0"));

    x = 10;
    s = jsoncons::double_to_string<wchar_t>(x, 16);
    BOOST_CHECK(s == std::wstring(L"10.0"));

    x = -10;
    s = jsoncons::double_to_string<wchar_t>(x, 16);
    BOOST_CHECK(s == std::wstring(L"-10.0"));

    x = 11;
    s = jsoncons::double_to_string<wchar_t>(x, 16);
    BOOST_CHECK(s == std::wstring(L"11.0"));

    x = -11;
    s = jsoncons::double_to_string<wchar_t>(x, 16);
    BOOST_CHECK(s == std::wstring(L"-11.0"));
}

