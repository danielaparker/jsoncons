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

BOOST_AUTO_TEST_SUITE(message_pack_tests)

BOOST_AUTO_TEST_CASE(message_pack_test)
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
    ja.add(0);
    ja.add(1);
    ja.add(2);
    ja.add(null_type());
    ja.add(true);
    ja.add(false);
    ja.add((std::numeric_limits<int64_t>::max)());
    ja.add((std::numeric_limits<uint64_t>::max)());
    ja.add((std::numeric_limits<int64_t>::min)());
    ja.add((std::numeric_limits<int32_t>::max)());
    ja.add((std::numeric_limits<uint32_t>::max)());
    ja.add((std::numeric_limits<int32_t>::min)());
    ja.add((std::numeric_limits<int16_t>::max)());
    ja.add((std::numeric_limits<uint16_t>::max)());
    ja.add((std::numeric_limits<int16_t>::min)());
    ja.add((std::numeric_limits<int8_t>::max)());
    ja.add((std::numeric_limits<uint8_t>::max)());
    ja.add((std::numeric_limits<int8_t>::min)());
    ja.add((std::numeric_limits<double>::max)());
    ja.add(-(std::numeric_limits<double>::max)());
    ja.add((std::numeric_limits<float>::max)());
    ja.add(0.0);
    ja.add(-(std::numeric_limits<float>::max)());
    ja.add("String too long for small string optimization");

    j1["An array"] = ja;

    size_t calculated_size = Encode_message_pack_<json>::calculate_size(j1);
    std::vector<uint8_t> v = encode_msgpack(j1);
    BOOST_CHECK(calculated_size == v.size());
    BOOST_CHECK(calculated_size == v.capacity());

    json j2 = decode_msgpack<json>(v);

    BOOST_CHECK_EQUAL(j1,j2);
} 

BOOST_AUTO_TEST_CASE(message_pack_test2)
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
    ja.add(0);
    ja.add(1);
    ja.add(2);
    ja.add(null_type());
    ja.add(true);
    ja.add(false);
    ja.add((std::numeric_limits<int64_t>::max)());
    ja.add((std::numeric_limits<uint64_t>::max)());
    ja.add((std::numeric_limits<int64_t>::min)());
    ja.add((std::numeric_limits<int32_t>::max)());
    ja.add((std::numeric_limits<uint32_t>::max)());
    ja.add((std::numeric_limits<int32_t>::min)());
    ja.add((std::numeric_limits<int16_t>::max)());
    ja.add((std::numeric_limits<uint16_t>::max)());
    ja.add((std::numeric_limits<int16_t>::min)());
    ja.add((std::numeric_limits<int8_t>::max)());
    ja.add((std::numeric_limits<uint8_t>::max)());
    ja.add((std::numeric_limits<int8_t>::min)());
    ja.add((std::numeric_limits<double>::max)());
    ja.add(-(std::numeric_limits<double>::max)());
    ja.add((std::numeric_limits<float>::max)());
    ja.add(0.0);
    ja.add(-(std::numeric_limits<float>::max)());
    ja.add(L"S");
    ja.add(L"String too long for small string optimization");

    j1[L"An array"] = ja;

    size_t calculated_size = Encode_message_pack_<wjson>::calculate_size(j1);
    std::vector<uint8_t> v = encode_msgpack(j1);
    BOOST_CHECK(calculated_size == v.size());
    BOOST_CHECK(calculated_size == v.capacity());

    wjson j2 = decode_msgpack<wjson>(v);

    BOOST_CHECK(j1 == j2);
}

BOOST_AUTO_TEST_SUITE_END()

