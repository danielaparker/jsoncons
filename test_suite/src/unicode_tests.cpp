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

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(unicode_test_suite)

BOOST_AUTO_TEST_CASE( test_surrogate_pair )
{
    std::string input = "[\"\\u8A73\\u7D30\\u95B2\\u89A7\\uD800\\uDC01\\u4E00\"]";
    json value = json::parse(input);
    output_format format;
    format.escape_all_non_ascii(true);
    std::string output = value.to_string(format);

    BOOST_CHECK_EQUAL(input,output);
}
#if 0
BOOST_AUTO_TEST_CASE(test_wide_surrogate_pair)
{
    wstring input = L"[\"\\u8A73\\u7D30\\u95B2\\u89A7\\uD800\\uDC01\\u4E00\"]";
    wjson value = wjson::parse(input);
    woutput_format format;
    format.escape_all_non_ascii(true);
    wstring output = value.to_string(format);

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
#endif

BOOST_AUTO_TEST_SUITE_END()

