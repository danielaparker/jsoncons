// Copyright 2016 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/msgpack/msgpack.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <limits>

using namespace jsoncons;
using namespace jsoncons::msgpack;

BOOST_AUTO_TEST_SUITE(msgpack_tests)

BOOST_AUTO_TEST_CASE(msgpack_test)
{
    json j1;
    j1["zero"] = 0;
    j1["one"] = 1;
    j1["two"] = 2;
    j1["null"] = null_type();
    j1["true"] = true;
    j1["false"] = false;
    j1["max int64_t"] = (std::numeric_limits<int64_t>::max)();
    j1["max uint64_t"] = (std::numeric_limits<uint64_t>::max)();
    j1["min int64_t"] = (std::numeric_limits<int64_t>::min)();
    j1["max int32_t"] = (std::numeric_limits<int32_t>::max)();
    j1["max uint32_t"] = (std::numeric_limits<uint32_t>::max)();
    j1["min int32_t"] = (std::numeric_limits<int32_t>::min)();
    j1["max int16_t"] = (std::numeric_limits<int16_t>::max)();
    j1["max uint16_t"] = (std::numeric_limits<uint16_t>::max)();
    j1["min int16_t"] = (std::numeric_limits<int16_t>::min)();
    j1["max int8_t"] = (std::numeric_limits<int8_t>::max)();
    j1["max uint8_t"] = (std::numeric_limits<uint8_t>::max)();
    j1["min int8_t"] = (std::numeric_limits<int8_t>::min)();
    j1["max double"] = (std::numeric_limits<double>::max)();
    j1["min double"] = -(std::numeric_limits<double>::max)();
    j1["max float"] = (std::numeric_limits<float>::max)();
    j1["zero float"] = 0.0;
    j1["min float"] = -(std::numeric_limits<float>::max)();
    j1["String too long for small string optimization"] = "String too long for small string optimization";

    json ja = json::array();
    ja.push_back(0);
    ja.push_back(1);
    ja.push_back(2);
    ja.push_back(null_type());
    ja.push_back(true);
    ja.push_back(false);
    ja.push_back((std::numeric_limits<int64_t>::max)());
    ja.push_back((std::numeric_limits<uint64_t>::max)());
    ja.push_back((std::numeric_limits<int64_t>::min)());
    ja.push_back((std::numeric_limits<int32_t>::max)());
    ja.push_back((std::numeric_limits<uint32_t>::max)());
    ja.push_back((std::numeric_limits<int32_t>::min)());
    ja.push_back((std::numeric_limits<int16_t>::max)());
    ja.push_back((std::numeric_limits<uint16_t>::max)());
    ja.push_back((std::numeric_limits<int16_t>::min)());
    ja.push_back((std::numeric_limits<int8_t>::max)());
    ja.push_back((std::numeric_limits<uint8_t>::max)());
    ja.push_back((std::numeric_limits<int8_t>::min)());
    ja.push_back((std::numeric_limits<double>::max)());
    ja.push_back(-(std::numeric_limits<double>::max)());
    ja.push_back((std::numeric_limits<float>::max)());
    ja.push_back(0.0);
    ja.push_back(-(std::numeric_limits<float>::max)());
    ja.push_back("String too long for small string optimization");

    j1["An array"] = ja;

    size_t calculated_size = msgpack_Encoder_<json>::calculate_size(j1);
    std::vector<uint8_t> v;
    encode_msgpack(j1, v);
    BOOST_CHECK(calculated_size == v.size());
    BOOST_CHECK(calculated_size == v.capacity());

    json j2 = decode_msgpack<json>(v);

    BOOST_CHECK_EQUAL(j1,j2);
} 

BOOST_AUTO_TEST_CASE(msgpack_test2)
{
    wjson j1;
    j1[L"zero"] = 0;
    j1[L"one"] = 1;
    j1[L"two"] = 2;
    j1[L"null"] = null_type();
    j1[L"true"] = true;
    j1[L"false"] = false;
    j1[L"max int64_t"] = (std::numeric_limits<int64_t>::max)();
    j1[L"max uint64_t"] = (std::numeric_limits<uint64_t>::max)();
    j1[L"min int64_t"] = (std::numeric_limits<int64_t>::min)();
    j1[L"max int32_t"] = (std::numeric_limits<int32_t>::max)();
    j1[L"max uint32_t"] = (std::numeric_limits<uint32_t>::max)();
    j1[L"min int32_t"] = (std::numeric_limits<int32_t>::min)();
    j1[L"max int16_t"] = (std::numeric_limits<int16_t>::max)();
    j1[L"max uint16_t"] = (std::numeric_limits<uint16_t>::max)();
    j1[L"min int16_t"] = (std::numeric_limits<int16_t>::min)();
    j1[L"max int8_t"] = (std::numeric_limits<int8_t>::max)();
    j1[L"max uint8_t"] = (std::numeric_limits<uint8_t>::max)();
    j1[L"min int8_t"] = (std::numeric_limits<int8_t>::min)();
    j1[L"max double"] = (std::numeric_limits<double>::max)();
    j1[L"min double"] = -(std::numeric_limits<double>::max)();
    j1[L"max float"] = (std::numeric_limits<float>::max)();
    j1[L"zero float"] = 0.0;
    j1[L"min float"] = -(std::numeric_limits<float>::max)();
    j1[L"S"] = L"S";
    j1[L"String too long for small string optimization"] = L"String too long for small string optimization";

    wjson ja = wjson::array();
    ja.push_back(0);
    ja.push_back(1);
    ja.push_back(2);
    ja.push_back(null_type());
    ja.push_back(true);
    ja.push_back(false);
    ja.push_back((std::numeric_limits<int64_t>::max)());
    ja.push_back((std::numeric_limits<uint64_t>::max)());
    ja.push_back((std::numeric_limits<int64_t>::min)());
    ja.push_back((std::numeric_limits<int32_t>::max)());
    ja.push_back((std::numeric_limits<uint32_t>::max)());
    ja.push_back((std::numeric_limits<int32_t>::min)());
    ja.push_back((std::numeric_limits<int16_t>::max)());
    ja.push_back((std::numeric_limits<uint16_t>::max)());
    ja.push_back((std::numeric_limits<int16_t>::min)());
    ja.push_back((std::numeric_limits<int8_t>::max)());
    ja.push_back((std::numeric_limits<uint8_t>::max)());
    ja.push_back((std::numeric_limits<int8_t>::min)());
    ja.push_back((std::numeric_limits<double>::max)());
    ja.push_back(-(std::numeric_limits<double>::max)());
    ja.push_back((std::numeric_limits<float>::max)());
    ja.push_back(0.0);
    ja.push_back(-(std::numeric_limits<float>::max)());
    ja.push_back(L"S");
    ja.push_back(L"String too long for small string optimization");

    j1[L"An array"] = ja;

    size_t calculated_size = msgpack_Encoder_<wjson>::calculate_size(j1);
    std::vector<uint8_t> v;
    encode_msgpack(j1, v);
    BOOST_CHECK(calculated_size == v.size());
    BOOST_CHECK(calculated_size == v.capacity());

    wjson j2 = decode_msgpack<wjson>(v);

    BOOST_CHECK(j1 == j2);
}

BOOST_AUTO_TEST_SUITE_END()

