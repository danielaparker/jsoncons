// Copyright 2016 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <jsoncons_ext/cbor/cbor_encoder.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <limits>

using namespace jsoncons;
using namespace jsoncons::cbor;

BOOST_AUTO_TEST_SUITE(cbor_encoder_tests)

BOOST_AUTO_TEST_CASE(test_array)
{
    std::vector<uint8_t> v;
    cbor_byte_string_encoder encoder(v);
    encoder.begin_json();
    //encoder.begin_object(1);
    encoder.begin_array(3);
    encoder.bool_value(true);
    encoder.bool_value(false);
    encoder.null_value();
    encoder.end_array();
    //encoder.end_object();
    encoder.end_json();

    for (auto c: v)
    {
        std::cout << std::hex << (int)c << std::endl;
    }

    try
    {
        json result = decode_cbor<json>(v);
        std::cout << result << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
} 

BOOST_AUTO_TEST_CASE(test_indefinite_length_array)
{
    std::vector<uint8_t> v;
    cbor_byte_string_encoder encoder(v);
    encoder.begin_json();
    encoder.begin_array();
    encoder.begin_array(4);
    encoder.bool_value(true);
    encoder.bool_value(false);
    encoder.null_value();
    encoder.string_value("Hello");
    encoder.end_array();
    encoder.end_array();
    encoder.end_json();

    for (auto c: v)
    {
        std::cout << std::hex << (int)c << std::endl;
    }

    try
    {
        json result = decode_cbor<json>(v);
        std::cout << result << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
} 

BOOST_AUTO_TEST_SUITE_END()

