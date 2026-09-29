// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons_ext/bson/bson.hpp>
#include <jsoncons/json.hpp>

#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <limits>
#include <catch/catch.hpp>

namespace bson = jsoncons::bson;

namespace {

std::error_code parse_error(const std::vector<uint8_t>& v)
{
    std::error_code ec;
    jsoncons::json_decoder<jsoncons::json> decoder;
    bson::bson_bytes_reader reader(v, decoder);
    reader.read(ec);

    return ec;
}

} // namespace

TEST_CASE("bson GLD.SerializerBenchmark tests")
{
    SECTION("bson 1.1")
    {
        CHECK(parse_error({0x0c,0x00,0x00,0x00,0x10,0x6e,0x00}) == bson::bson_errc::unexpected_eof);
        CHECK(parse_error({0x0b,0x00,0x00,0x00,0x10,0x6e,0x00,0x07,0x00,0x00,0x00}) == bson::bson_errc::unexpected_eof);
        CHECK(parse_error({0x08,0x00,0x00,0x00,0x14,0x78,0x00,0x00}) == bson::bson_errc::unknown_type);
        CHECK(parse_error({}) == bson::bson_errc::unexpected_eof);
        CHECK(parse_error({0x05,0x00}) == bson::bson_errc::unexpected_eof);
        CHECK(parse_error({0x04,0x00,0x00,0x00,0x00}) == bson::bson_errc::size_mismatch);
        CHECK(parse_error({0x0c,0x00,0x00,0x00,0x02,0x73,0x00,0x05,0x00,0x00,0x00,0x6b,0x00}) == bson::bson_errc::unexpected_eof);
        CHECK(parse_error({0x0a,0x00,0x00,0x00,0x08,0x6f,0x6b,0x00,0x02,0x00}) == bson::bson_errc::expected_boolean); //
    }
}
