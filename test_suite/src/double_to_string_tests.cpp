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

