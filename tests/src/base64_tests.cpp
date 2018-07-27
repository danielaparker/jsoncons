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
#include <jsoncons/json.hpp>
#include <jsoncons/jsoncons_utilities.hpp>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(base64_tests)

// https://tools.ietf.org/html/rfc4648#section-4 test vectors

void check_encode_base64(const std::vector<uint8_t>& input, const std::string& expected)
{
    std::string result;
    encode_base64(input.data(),input.size(),result);
    BOOST_REQUIRE(result.size() == expected.size());
    for (size_t i = 0; i < result.size(); ++i)
    {
        BOOST_CHECK(result[i] == expected[i]);
    }

    std::vector<uint8_t> output = decode_base64(result);
    BOOST_REQUIRE(output.size() == input.size());
    for (size_t i = 0; i < output.size(); ++i)
    {
        BOOST_CHECK(output[i] == input[i]);
    }
}

BOOST_AUTO_TEST_CASE(test_base64_conversion)
{
    check_encode_base64({}, "");
    check_encode_base64({'f'}, "Zg==");
    check_encode_base64({'f','o'}, "Zm8=");
    check_encode_base64({'f','o','o'}, "Zm9v");
    check_encode_base64({'f','o','o','b'}, "Zm9vYg==");
    check_encode_base64({'f','o','o','b','a'}, "Zm9vYmE=");
    check_encode_base64({'f','o','o','b','a','r'}, "Zm9vYmFy");
}

BOOST_AUTO_TEST_SUITE_END()




