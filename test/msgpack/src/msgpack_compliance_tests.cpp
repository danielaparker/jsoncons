// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif

#include <jsoncons_ext/msgpack/msgpack.hpp>

#include <jsoncons/json.hpp>

#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <limits>
#include <catch/catch.hpp>

namespace msgpack = jsoncons::msgpack;

namespace {

std::error_code parse_error(const std::vector<uint8_t>& v)
{
    std::error_code ec;
    jsoncons::json_decoder<jsoncons::json> decoder;
    msgpack::msgpack_bytes_reader reader(v, decoder);
    reader.read(ec);

    return ec;
}

} // namespace

TEST_CASE("msgpack GLD.SerializerBenchmark tests")
{
    SECTION("MessagePack (2013 str/bin/ext)")
    {
        CHECK(parse_error({0xd9,0x01}) == msgpack::msgpack_errc::unexpected_eof);
        CHECK(parse_error({0xc1}) == msgpack::msgpack_errc::unknown_type);
    }
    SECTION("MessagePack (2017 timestamp_ext)")
    {
        CHECK(parse_error({0xd6,0xff,0x00,0x00}) == msgpack::msgpack_errc::unexpected_eof);
    }
}

