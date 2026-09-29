// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h"
#endif
#include <jsoncons_ext/cbor/cbor.hpp>
#include <jsoncons/json.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <limits>
#include <catch/catch.hpp>

namespace cbor = jsoncons::cbor;

namespace {

std::error_code parse_cbor_error(const std::vector<uint8_t>& v)
{
    std::error_code ec;
    jsoncons::json_decoder<jsoncons::json> decoder;
    cbor::cbor_bytes_reader reader(v, decoder);
    reader.read(ec);

    return ec;
}

} // namespace

TEST_CASE("cbor_test_floating_point")
{
    jsoncons::json j1;
    j1["max double"] = (std::numeric_limits<double>::max)();
    j1["max float"] = (std::numeric_limits<float>::max)();

    std::vector<uint8_t> v;
    cbor::encode_cbor(j1, v);

    jsoncons::json j2 = cbor::decode_cbor<jsoncons::json>(v);
    CHECK(j2 == j1);

    jsoncons::json j3 = cbor::decode_cbor<jsoncons::json>(v.begin(), v.end());
    CHECK(j3 == j1);
} 

TEST_CASE("cbor_test")
{
    jsoncons::json j1;
    j1["zero"] = 0;
    j1["one"] = 1;
    j1["two"] = 2;
    j1["null"] = jsoncons::null_type();
    j1["true"] = true;
    j1["false"] = false;
    j1["max int64_t"] = (std::numeric_limits<int64_t>::max)();
    j1["max uint64_t"] = (std::numeric_limits<uint64_t>::max)();
    j1["min int64_t"] = (std::numeric_limits<int64_t>::lowest)();
    j1["max int32_t"] = (std::numeric_limits<int32_t>::max)();
    j1["max uint32_t"] = (std::numeric_limits<uint32_t>::max)();
    j1["min int32_t"] = (std::numeric_limits<int32_t>::lowest)();
    j1["max int16_t"] = (std::numeric_limits<int16_t>::max)();
    j1["max uint16_t"] = (std::numeric_limits<uint16_t>::max)();
    j1["min int16_t"] = (std::numeric_limits<int16_t>::lowest)();
    j1["max int8_t"] = (std::numeric_limits<int8_t>::max)();
    j1["max uint8_t"] = (std::numeric_limits<uint8_t>::max)();
    j1["min int8_t"] = (std::numeric_limits<int8_t>::lowest)();
    j1["max double"] = (std::numeric_limits<double>::max)();
    j1["min double"] = (std::numeric_limits<double>::lowest)();
    j1["max float"] = (std::numeric_limits<float>::max)();
    j1["zero float"] = 0.0;
    j1["min float"] = (std::numeric_limits<float>::lowest)();
    j1["String too long for small string optimization"] = "String too long for small string optimization";

    jsoncons::json ja(jsoncons::json_array_arg);
    ja.push_back(0);
    ja.push_back(1);
    ja.push_back(2);
    ja.push_back(jsoncons::null_type());
    ja.push_back(true);
    ja.push_back(false);
    ja.push_back((std::numeric_limits<int64_t>::max)());
    ja.push_back((std::numeric_limits<uint64_t>::max)());
    ja.push_back((std::numeric_limits<int64_t>::lowest)());
    ja.push_back((std::numeric_limits<int32_t>::max)());
    ja.push_back((std::numeric_limits<uint32_t>::max)());
    ja.push_back((std::numeric_limits<int32_t>::lowest)());
    ja.push_back((std::numeric_limits<int16_t>::max)());
    ja.push_back((std::numeric_limits<uint16_t>::max)());
    ja.push_back((std::numeric_limits<int16_t>::lowest)());
    ja.push_back((std::numeric_limits<int8_t>::max)());
    ja.push_back((std::numeric_limits<uint8_t>::max)());
    ja.push_back((std::numeric_limits<int8_t>::lowest)());
    ja.push_back((std::numeric_limits<double>::max)());
    ja.push_back((std::numeric_limits<double>::lowest)());
    ja.push_back((std::numeric_limits<float>::max)());
    ja.push_back(0.0);
    ja.push_back((std::numeric_limits<float>::lowest)());
    ja.push_back("String too long for small string optimization");

    j1["An array"] = ja;

    std::vector<uint8_t> v;
    cbor::encode_cbor(j1, v);

    jsoncons::json j2 = cbor::decode_cbor<jsoncons::json>(v);
    CHECK(j2 == j1);

    jsoncons::json j3 = cbor::decode_cbor<jsoncons::json>(v.begin(), v.end());
    CHECK(j3 == j1);
} 

TEST_CASE("cbor_test2")
{
    jsoncons::wjson j1;
    j1[L"zero"] = 0;
    j1[L"one"] = 1;
    j1[L"two"] = 2;
    j1[L"null"] = jsoncons::null_type();
    j1[L"true"] = true;
    j1[L"false"] = false;
    j1[L"max int64_t"] = (std::numeric_limits<int64_t>::max)();
    j1[L"max uint64_t"] = (std::numeric_limits<uint64_t>::max)();
    j1[L"min int64_t"] = (std::numeric_limits<int64_t>::lowest)();
    j1[L"max int32_t"] = (std::numeric_limits<int32_t>::max)();
    j1[L"max uint32_t"] = (std::numeric_limits<uint32_t>::max)();
    j1[L"min int32_t"] = (std::numeric_limits<int32_t>::lowest)();
    j1[L"max int16_t"] = (std::numeric_limits<int16_t>::max)();
    j1[L"max uint16_t"] = (std::numeric_limits<uint16_t>::max)();
    j1[L"min int16_t"] = (std::numeric_limits<int16_t>::lowest)();
    j1[L"max int8_t"] = (std::numeric_limits<int8_t>::max)();
    j1[L"max uint8_t"] = (std::numeric_limits<uint8_t>::max)();
    j1[L"min int8_t"] = (std::numeric_limits<int8_t>::lowest)();
    j1[L"max double"] = (std::numeric_limits<double>::max)();
    j1[L"min double"] = (std::numeric_limits<double>::lowest)();
    j1[L"max float"] = (std::numeric_limits<float>::max)();
    j1[L"zero float"] = 0.0;
    j1[L"min float"] = (std::numeric_limits<float>::lowest)();
    j1[L"S"] = L"S";
    j1[L"String too long for small string optimization"] = L"String too long for small string optimization";

    jsoncons::wjson ja(jsoncons::json_array_arg);
    ja.push_back(0);
    ja.push_back(1);
    ja.push_back(2);
    ja.push_back(jsoncons::null_type());
    ja.push_back(true);
    ja.push_back(false);
    ja.push_back((std::numeric_limits<int64_t>::max)());
    ja.push_back((std::numeric_limits<uint64_t>::max)());
    ja.push_back((std::numeric_limits<int64_t>::lowest)());
    ja.push_back((std::numeric_limits<int32_t>::max)());
    ja.push_back((std::numeric_limits<uint32_t>::max)());
    ja.push_back((std::numeric_limits<int32_t>::lowest)());
    ja.push_back((std::numeric_limits<int16_t>::max)());
    ja.push_back((std::numeric_limits<uint16_t>::max)());
    ja.push_back((std::numeric_limits<int16_t>::lowest)());
    ja.push_back((std::numeric_limits<int8_t>::max)());
    ja.push_back((std::numeric_limits<uint8_t>::max)());
    ja.push_back((std::numeric_limits<int8_t>::lowest)());
    ja.push_back((std::numeric_limits<double>::max)());
    ja.push_back((std::numeric_limits<double>::lowest)());
    ja.push_back((std::numeric_limits<float>::max)());
    ja.push_back(0.0);
    ja.push_back((std::numeric_limits<float>::lowest)());
    ja.push_back(L"S");
    ja.push_back(L"String too long for small string optimization");

    j1[L"An array"] = ja;

    std::vector<uint8_t> v;
    cbor::encode_cbor(j1, v);

    jsoncons::wjson j2 = cbor::decode_cbor<jsoncons::wjson>(v);
    CHECK(j2 == j1);

    jsoncons::wjson j3 = cbor::decode_cbor<jsoncons::wjson>(v.begin(), v.end());
    CHECK(j3 == j1);
}

TEST_CASE("cbor_reputon_test")
{
    jsoncons::ojson j1 = jsoncons::ojson::parse(R"(
{
   "application": "hiking",
   "reputons": [
   {
       "rater": "HikingAsylum",
       "assertion": "advanced",
       "rated": "Marilyn C",
       "rating": 0.90
     }
   ]
}
)");

    std::vector<uint8_t> v;
    cbor::encode_cbor(j1, v);

    auto j2 = cbor::decode_cbor<jsoncons::ojson>(v);
    CHECK(j2 == j1);

    auto j3 = cbor::decode_cbor<jsoncons::ojson>(v.begin(), v.end());
    CHECK(j3 == j1);
}

#if (defined(__GNUC__) || defined(__clang__)) && defined(JSONCONS_HAS_INT128) 
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
TEST_CASE("cbor json constructor __int64 tests")
{
    SECTION("test 1")
    {
        jsoncons::json j1("-18446744073709551617", jsoncons::semantic_tag::bigint);

        __int128 val1 = j1.as<__int128>();

        std::vector<uint8_t> expected = {0xc3,0x49,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
        std::vector<uint8_t> data;
        cbor::encode_cbor(val1, data);
        CHECK(expected == data);

        auto val2 = cbor::decode_cbor<__int128>(data);

        CHECK((val2 == val1));
    }
}
TEST_CASE("cbor json constructor unsigned __int64 tests")
{
    SECTION("test 1")
    {
        jsoncons::json j1("18446744073709551616", jsoncons::semantic_tag::bigint);

        auto val1 = j1.as<unsigned __int128>();

        std::vector<uint8_t> expected = {0xc2,0x49,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
        std::vector<uint8_t> data;
        cbor::encode_cbor(val1, data);
        CHECK(expected == data);

        auto val2 = cbor::decode_cbor<unsigned __int128>(data);

        CHECK((val2 == val1));
    }
}
#pragma GCC diagnostic pop
#endif

TEST_CASE("cbor GLD.SerializerBenchmark tests")
{
    SECTION("test 1")
    {
        CHECK(parse_cbor_error({0x18}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_cbor_error({0x19}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_cbor_error({0x19,0x00}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_cbor_error({0x1a}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_cbor_error({0x1a,0x00}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_cbor_error({0x1a,0x00,0x00}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_cbor_error({0x1a,0x00,0x00,0x00}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_cbor_error({0x1b,0x00,0x00,0x00}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_cbor_error({0x1c}) == cbor::cbor_errc::reserved_additional_info_value); // bad
        CHECK(parse_cbor_error({0x1d}) == cbor::cbor_errc::reserved_additional_info_value); // bad
        CHECK(parse_cbor_error({0x1e}) == cbor::cbor_errc::reserved_additional_info_value); // bad
        CHECK(parse_cbor_error({0xfc}) == cbor::cbor_errc::reserved_additional_info_value);
        CHECK(parse_cbor_error({0xfd}) == cbor::cbor_errc::reserved_additional_info_value);
        CHECK(parse_cbor_error({0xfe}) == cbor::cbor_errc::reserved_additional_info_value);
        CHECK(parse_cbor_error({0x44,01,02,03}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_cbor_error({0x5f}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_cbor_error({0x5f,0x01,0xff}) == cbor::cbor_errc::illegal_chunked_string);
        CHECK(parse_cbor_error({0x64,0x49,0x45,0x54}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_cbor_error({0x74,0x32,0x30,0x31,0x33}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_cbor_error({0x7f,0x01,0xff}) == cbor::cbor_errc::illegal_chunked_string);
        CHECK(parse_cbor_error({0x7f,0x65,0x73,0x74,0x72,0x65,0x61,0x64,0x6d,0x69,0x6e}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_cbor_error({0x62,0xc0,0xae}) == cbor::cbor_errc::invalid_utf8_text_string);
        CHECK(parse_cbor_error({0x81}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_cbor_error({0x82,0x01}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_cbor_error({0x81,0x81,0x81,0x81,0x81}) == cbor::cbor_errc::unexpected_eof);

        CHECK(parse_cbor_error({
            0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
            0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
            0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
            0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
            0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
            0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
            0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
            0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
            0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
            0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81
        }) == cbor::cbor_errc::unexpected_eof);

        CHECK(parse_cbor_error({0x81,0xFE}) == cbor::cbor_errc::reserved_additional_info_value);
        CHECK(parse_cbor_error({0x9f}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_cbor_error({0x9f,0x01}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_cbor_error({0x9f,0xFE,0xff}) == cbor::cbor_errc::reserved_additional_info_value);
        CHECK(parse_cbor_error({0x91,0xff}) == cbor::cbor_errc::unknown_type);
        CHECK(parse_cbor_error({0xa1}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_cbor_error({0xa1,0xfe,0x01}) == cbor::cbor_errc::reserved_additional_info_value);
        CHECK(parse_cbor_error({0xa1,0x61,0x61}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_cbor_error({0xa1,0x61,0x61,0xfe}) == cbor::cbor_errc::reserved_additional_info_value);
        CHECK(parse_cbor_error({0xa2,0x01,0x02}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_cbor_error({0xbf}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_cbor_error({0xbf,0x00,0x01,0x03,0xff}) == cbor::cbor_errc::unknown_type);
        CHECK(parse_cbor_error({0xbf,0x61,0x61}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_cbor_error({0xbf,0x61,0x61,0x01}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_cbor_error({0xbf,0xFE,0x01}) == cbor::cbor_errc::reserved_additional_info_value);
        CHECK(parse_cbor_error({0xbf,0x01,0xFE}) == cbor::cbor_errc::reserved_additional_info_value);
        CHECK(parse_cbor_error({0xa1,0xff}) == cbor::cbor_errc::unknown_type);
        CHECK(parse_cbor_error({0xa1,0x00,0xff}) == cbor::cbor_errc::unknown_type);
        CHECK(parse_cbor_error({0xff}) == cbor::cbor_errc::unknown_type);
        //CHECK(parse_cbor_error({0xc1,0xa1,0x61,0x61,0x00}) == cbor::cbor_errc::unexpected_eof); // tag is epoch, but value is map
        //CHECK(parse_cbor_error({0xc0,0xa1,0x61,0x61,0x00}) == cbor::cbor_errc::unexpected_eof); // tag is text string, but value is map
        CHECK(parse_cbor_error({0x18}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_cbor_error({0xff}) == cbor::cbor_errc::unknown_type);
    }
}
