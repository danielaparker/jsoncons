// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(double_to_string_tests)

template<class CharT>
std::basic_string<CharT> float_to_string(double val, uint8_t precision)
{
    std::basic_ostringstream<CharT> ss;
    ss.imbue(std::locale::classic());
    {
        buffered_output<CharT> os(ss);
        print_double<CharT> print(precision);
        print(val, precision, os);
    }
    return ss.str();
}

const serialization_options format;

BOOST_AUTO_TEST_CASE(test_double_to_string)
{
    double x = 1.0e100;
    std::string s = float_to_string<char>(x, format.precision());
    //std::cout << x << ": " << s << std::endl;
    BOOST_CHECK(s == std::string("1.0e+100") || s == std::string("1.0e100"));

    x = 1.0e-100;
    s = float_to_string<char>(x, format.precision());
    BOOST_CHECK(s == std::string("1.0e-100"));

    x = 0.123456789e-100;
    s = float_to_string<char>(x, format.precision());
    BOOST_CHECK(s == std::string("1.23456789e-101"));

    x = 0.123456789e100;
    s = float_to_string<char>(x, format.precision());
    BOOST_CHECK(s == std::string("1.23456789e+99") || s == std::string("1.23456789e+099"));

    x = 1234563;
    s = float_to_string<char>(x, 6);
    BOOST_CHECK(s == std::string("1.23456e+6") || s == std::string("1.23456e+06") || s == std::string("1.23456e+006") || s == std::string("1234560"));

    x = 0.0000001234563;
    s = float_to_string<char>(x, 6);
    BOOST_CHECK(s == std::string("1.23456e-7") || s == std::string("1.23456e-07") || s == std::string("1.23456e-007"));

    x = -1.0e+100;
    s = float_to_string<char>(x, format.precision());
    BOOST_CHECK(s == std::string("-1.0e+100") || s == std::string("-1.0e100"));

    x = -1.0e-100;
    s = float_to_string<char>(x, format.precision());
    BOOST_CHECK(s == std::string("-1.0e-100"));

    x = 0;
    s = float_to_string<char>(x, format.precision());
    BOOST_CHECK(s == std::string("0.0"));

    x = -0;
    s = float_to_string<char>(x, format.precision());
    BOOST_CHECK(s == std::string("0.0"));

    x = 1;
    s = float_to_string<char>(x, format.precision());
    BOOST_CHECK(s == std::string("1.0"));

    x = 0.1;
    s = float_to_string<char>(x, format.precision());
    BOOST_CHECK(s == std::string("0.1"));

    x = 1.1;
    s = float_to_string<char>(x, 17);
    BOOST_CHECK(s == std::string("1.1000000000000001"));

    x = -1;
    s = float_to_string<char>(x, format.precision());
    BOOST_CHECK(s == std::string("-1.0"));

    x = 10;
    s = float_to_string<char>(x, format.precision());
    BOOST_CHECK(s == std::string("10.0"));

    x = -10;
    s = float_to_string<char>(x, format.precision());
    BOOST_CHECK(s == std::string("-10.0"));

    x = 11;
    s = float_to_string<char>(x, format.precision());
    BOOST_CHECK(s == std::string("11.0"));

    x = -11;
    s = float_to_string<char>(x, format.precision());
    BOOST_CHECK(s == std::string("-11.0"));
}

#if defined(_MSC_VER)
BOOST_AUTO_TEST_CASE(test_locale)
{
    wchar_t * loc = _wsetlocale(LC_ALL, L"de-DE");
    BOOST_CHECK(loc != nullptr);

    double x = 123456789.0123;
    std::wstring s = float_to_string<wchar_t>(x, 13);
    //std::wcout << std::setprecision(13) << x << L": " << s << std::endl;
    BOOST_CHECK(std::wstring(L"123456789.0123") == s);
    _wsetlocale(LC_ALL, L"C");
}
#endif

BOOST_AUTO_TEST_CASE(test_double_to_wstring)
{
    double x = 1.0e100;
    std::wstring s = float_to_string<wchar_t>(x, format.precision());
    std::wcout << x << L":" << s << std::endl;
    BOOST_CHECK(s == std::wstring(L"1.0e+100") || s == std::wstring(L"1.0e100"));

    x = 1.0e-100;
    s = float_to_string<wchar_t>(x, format.precision());
    BOOST_CHECK(s == std::wstring(L"1.0e-100"));

    x = -1.0e+100;
    s = float_to_string<wchar_t>(x, format.precision());
    BOOST_CHECK(s == std::wstring(L"-1.0e+100") || s == std::wstring(L"-1.0e100"));

    x = -1.0e-100;
    s = float_to_string<wchar_t>(x, format.precision());
    BOOST_CHECK(s == std::wstring(L"-1.0e-100"));

    x = 0;
    s = float_to_string<wchar_t>(x, format.precision());
    BOOST_CHECK(s == std::wstring(L"0.0"));

    x = -0;
    s = float_to_string<wchar_t>(x, format.precision());
    BOOST_CHECK(s == std::wstring(L"0.0"));

    x = 1;
    s = float_to_string<wchar_t>(x, format.precision());
    BOOST_CHECK(s == std::wstring(L"1.0"));

    x = -1;
    s = float_to_string<wchar_t>(x, format.precision());
    BOOST_CHECK(s == std::wstring(L"-1.0"));

    x = 10;
    s = float_to_string<wchar_t>(x, format.precision());
    BOOST_CHECK(s == std::wstring(L"10.0"));

    x = -10;
    s = float_to_string<wchar_t>(x, format.precision());
    BOOST_CHECK(s == std::wstring(L"-10.0"));

    x = 11;
    s = float_to_string<wchar_t>(x, format.precision());
    BOOST_CHECK(s == std::wstring(L"11.0"));

    x = -11;
    s = float_to_string<wchar_t>(x, format.precision());
    BOOST_CHECK(s == std::wstring(L"-11.0"));
}
BOOST_AUTO_TEST_SUITE_END()

