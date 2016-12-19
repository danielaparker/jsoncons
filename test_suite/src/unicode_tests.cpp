// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <jsoncons/json.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <string>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(unicode_test_suite)

BOOST_AUTO_TEST_CASE( test_surrogate_pair )
{
    std::string input = "[\"\\u8A73\\u7D30\\u95B2\\u89A7\\uD800\\uDC01\\u4E00\"]";
    json value = json::parse(input);
    serialization_options format;
    format.escape_all_non_ascii(true);
    std::string output;
    value.dump(output,format);

    BOOST_CHECK_EQUAL(input,output);
}

BOOST_AUTO_TEST_CASE(test_skip_bom)
{
    std::string input = "\xEF\xBB\xBF[1,2,3]";
    json value = json::parse(input);
    BOOST_CHECK_EQUAL(true,value.is_array());
    BOOST_CHECK_EQUAL(3,value.size());
}

BOOST_AUTO_TEST_CASE(test_skip_bom2)
{
    std::wstring input = L"\xFEFF[1,2,3]";
    wjson value = wjson::parse(input);
    BOOST_CHECK_EQUAL(true,value.is_array());
    BOOST_CHECK_EQUAL(3,value.size());
}

BOOST_AUTO_TEST_CASE(test_wide_surrogate_pair)
{
    std::wstring input = L"[\"\\u8A73\\u7D30\\u95B2\\u89A7\\uD800\\uDC01\\u4E00\"]";
    wjson value = wjson::parse(input);
    wserialization_options format;
    format.escape_all_non_ascii(true);
    std::wstring output;
    value.dump(output,format);

    BOOST_CHECK(input == output);
}

BOOST_AUTO_TEST_CASE( test1 )
{
    std::istringstream is("{\"unicode_string_1\":\"\\uD800\\uDC00\"}");

    json root = json::parse_stream(is);
    BOOST_CHECK(root.is_object());
    BOOST_CHECK(root.is<json::object>());

    root["double_1"] = 10.0;

    json double_1 = root["double_1"];

    BOOST_CHECK_CLOSE(double_1.as<double>(), 10.0, 0.000001);

    BOOST_CHECK_CLOSE(double_1.as<double>(), 10.0, 0.000001);

    json copy(root);
}

BOOST_AUTO_TEST_SUITE_END()

