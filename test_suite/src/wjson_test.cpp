// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include "jsoncons/json.hpp"
#include "jsoncons/json_serializer.hpp"
#include "jsoncons/json_filter.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include "my_custom_data.hpp"

using jsoncons::parsing_context;
using jsoncons::json_serializer;
using jsoncons::output_format;
using jsoncons::json;
using jsoncons::wjson;
using jsoncons::json_reader;
using jsoncons::json_input_handler;
using jsoncons::json_output_handler;
using std::string;
using std::wstring;
using jsoncons::json_filter;
using jsoncons::pretty_print;

BOOST_AUTO_TEST_CASE(test_wjson )
{
    wjson root;
    root[L"field1"] = L"test";
    root[L"field2"] = 3.9;
    root[L"field3"] = true;
    std::wcout << root << L"\n";
    //wjson root = wjson::parse(L"{}");
}

BOOST_AUTO_TEST_CASE(test_wjson_escape_u)
{
    wstring input = L"[\"\\uABCD\"]";
    std::wistringstream is(input);

    wjson root = wjson::parse(is);

    wstring s = root[0].as<wstring>();
    BOOST_CHECK_EQUAL( s.length(), 1 );
    BOOST_CHECK_EQUAL( s[0], 0xABCD );
}

BOOST_AUTO_TEST_CASE(test_wjson_escape_u2)
{
    wstring input = L"[\"\\u007F\\u07FF\\u0800\"]";
    std::wistringstream is(input);

    wjson root = wjson::parse(is);

    wstring s = root[0].as<wstring>();

    std::wofstream os("output/xxx.txt");

    os << s << L"\n";

    std::wcout << s << L"\n\n";
    //BOOST_CHECK_EQUAL( s.length(), 1 );
    //BOOST_CHECK_EQUAL( s[0], 0xABCD );
}

BOOST_AUTO_TEST_CASE(test_json_escape_u2)
{
    string input = "[\"\\u007F\\u07FF\\u0800\"]";
    std::istringstream is(input);

    json root = json::parse(is);

    string s = root[0].as<string>();

    std::ofstream os("output/yyy.txt");

    os << s << "\n";

    std::cout << s << "\n";
    //BOOST_CHECK_EQUAL( s.length(), 1 );
    //BOOST_CHECK_EQUAL( s[0], 0xABCD );
}

