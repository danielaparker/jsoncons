// Copyright 2016 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <limits>

using namespace jsoncons;
using namespace jsoncons::cbor;

BOOST_AUTO_TEST_SUITE(cbor_tests)

void check_encode(const std::vector<uint8_t>& expected, const json& j)
{
    std::vector<uint8_t> result = encode_cbor(j);
    BOOST_TEST_INFO(j.to_string());
    BOOST_REQUIRE(expected.size() == result.size());
    for (size_t i = 0; i < expected.size(); ++i)
    {
        BOOST_REQUIRE_EQUAL(expected[i],result[i]);
    }
}

BOOST_AUTO_TEST_CASE(cbor_encoder_test)
{
    check_encode({'\x00'},json(0U));
    check_encode({'\x01'},json(1U));
    check_encode({'\x0a'},json(10U));
    check_encode({'\x17'},json(23U));
    check_encode({'\x18','\x18'},json(24U));

}

BOOST_AUTO_TEST_SUITE_END()

