// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <ctime>
#include <new>
#include <codecvt>
#include <jsoncons/json.hpp>
#include <jsoncons/jsoncons_utilities.hpp>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(base64_tests)

// https://tools.ietf.org/html/rfc4648#section-4 test vectors

void check_encode_base64(const std::string& input, const std::string& expected)
{
    std::string result;
    encode_base64(input.begin(),input.end(),result);
    BOOST_CHECK_EQUAL(result, expected);
}

BOOST_AUTO_TEST_CASE(test_encode_base64)
{
    check_encode_base64("", "");
    check_encode_base64("f", "Zg==");
    check_encode_base64("fo", "Zm8=");
    check_encode_base64("foo", "Zm9v");
    check_encode_base64("foob", "Zm9vYg==");
    check_encode_base64("fooba", "Zm9vYmE=");
    check_encode_base64("foobar", "Zm9vYmFy");
}

BOOST_AUTO_TEST_CASE(test_decode_base64)
{
    BOOST_CHECK_EQUAL("", decode_base64(""));
    BOOST_CHECK_EQUAL("f", decode_base64("Zg=="));
    BOOST_CHECK_EQUAL("fo", decode_base64("Zm8="));
    BOOST_CHECK_EQUAL("foo", decode_base64("Zm9v"));
    BOOST_CHECK_EQUAL("foob", decode_base64("Zm9vYg=="));
    BOOST_CHECK_EQUAL("fooba", decode_base64("Zm9vYmE="));
    BOOST_CHECK_EQUAL("foobar", decode_base64("Zm9vYmFy"));
}

BOOST_AUTO_TEST_SUITE_END()




