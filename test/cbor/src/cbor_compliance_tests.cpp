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

std::error_code parse_error(const std::vector<uint8_t>& v)
{
    std::error_code ec;
    jsoncons::json_decoder<jsoncons::json> decoder;
    cbor::cbor_bytes_reader reader(v, decoder);
    reader.read(ec);

    return ec;
}

} // namespace

TEST_CASE("cbor GLD.SerializerBenchmark tests")
{
    SECTION("RFC 8949")
    {
        CHECK(parse_error({0x18}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_error({0x19}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_error({0x19,0x00}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_error({0x1a}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_error({0x1a,0x00}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_error({0x1a,0x00,0x00}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_error({0x1a,0x00,0x00,0x00}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_error({0x1b,0x00,0x00,0x00}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_error({0x1c}) == cbor::cbor_errc::reserved_additional_info); // bad
        CHECK(parse_error({0x1d}) == cbor::cbor_errc::reserved_additional_info); // bad
        CHECK(parse_error({0x1e}) == cbor::cbor_errc::reserved_additional_info); // bad
        CHECK(parse_error({0x1f}) == cbor::cbor_errc::unknown_type);
        CHECK(parse_error({0x3f}) == cbor::cbor_errc::unknown_type);
        CHECK(parse_error({0xdf}) == cbor::cbor_errc::unknown_type);
        CHECK(parse_error({0xfc}) == cbor::cbor_errc::reserved_additional_info);
        CHECK(parse_error({0xfd}) == cbor::cbor_errc::reserved_additional_info);
        CHECK(parse_error({0xfe}) == cbor::cbor_errc::reserved_additional_info);
        CHECK(parse_error({0x44,01,02,03}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_error({0x5f}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_error({0x5f,0x01,0xff}) == cbor::cbor_errc::illegal_chunked_string);
        CHECK(parse_error({0x64,0x49,0x45,0x54}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_error({0x74,0x32,0x30,0x31,0x33}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_error({0x7f,0x01,0xff}) == cbor::cbor_errc::illegal_chunked_string);
        CHECK(parse_error({0x5f,0x5e}) == cbor::cbor_errc::reserved_additional_info);
        CHECK(parse_error({0x7f,0x7e}) == cbor::cbor_errc::reserved_additional_info);
        CHECK(parse_error({0xc4,0x82,0x3b,0x7f,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x06}) == cbor::cbor_errc::invalid_bigdecimal);
        CHECK(parse_error({0x7f,0x65,0x73,0x74,0x72,0x65,0x61,0x64,0x6d,0x69,0x6e}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_error({0x62,0xc0,0xae}) == cbor::cbor_errc::invalid_utf8_text_string);
        CHECK(parse_error({0x81}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_error({0x82,0x01}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_error({0x81,0x81,0x81,0x81,0x81}) == cbor::cbor_errc::unexpected_eof);

        CHECK(parse_error({
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

        CHECK(parse_error({0x81,0xFE}) == cbor::cbor_errc::reserved_additional_info);
        CHECK(parse_error({0x9f}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_error({0x9f,0x01}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_error({0x9f,0xFE,0xff}) == cbor::cbor_errc::reserved_additional_info);
        CHECK(parse_error({0x91,0xff}) == cbor::cbor_errc::unknown_type);
        CHECK(parse_error({0xa1}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_error({0xa1,0xfe,0x01}) == cbor::cbor_errc::reserved_additional_info);
        CHECK(parse_error({0xa1,0x61,0x61}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_error({0xa1,0x61,0x61,0xfe}) == cbor::cbor_errc::reserved_additional_info);
        CHECK(parse_error({0xa2,0x01,0x02}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_error({0xbf}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_error({0xbf,0x00,0x01,0x03,0xff}) == cbor::cbor_errc::unknown_type);
        CHECK(parse_error({0xbf,0x61,0x61}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_error({0xbf,0x61,0x61,0x01}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_error({0xbf,0xFE,0x01}) == cbor::cbor_errc::reserved_additional_info);
        CHECK(parse_error({0xbf,0x01,0xFE}) == cbor::cbor_errc::reserved_additional_info);
        CHECK(parse_error({0xa1,0xff}) == cbor::cbor_errc::unknown_type);
        CHECK(parse_error({0xa1,0x00,0xff}) == cbor::cbor_errc::unknown_type);
        CHECK(parse_error({0xff}) == cbor::cbor_errc::unknown_type);
        //CHECK(parse_error({0xc1,0xa1,0x61,0x61,0x00}) == cbor::cbor_errc::unexpected_eof); // tag is epoch, but value is map
        //CHECK(parse_error({0xc0,0xa1,0x61,0x61,0x00}) == cbor::cbor_errc::unexpected_eof); // tag is text string, but value is map
        CHECK(parse_error({0x18}) == cbor::cbor_errc::unexpected_eof);
        CHECK(parse_error({0xff}) == cbor::cbor_errc::unknown_type);
    }
}
