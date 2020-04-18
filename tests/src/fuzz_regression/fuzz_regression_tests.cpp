// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <jsoncons_ext/ubjson/ubjson.hpp>
#include <jsoncons_ext/csv/csv.hpp>
#include <vector>
#include <utility>
#include <ctime>
#include <fstream>
#include <catch/catch.hpp>

using namespace jsoncons;

TEST_CASE("oss-fuzz issues")
{
    SECTION("issue 21589")
    {
        std::string pathname = "input/fuzz/clusterfuzz-testcase-minimized-fuzz_parse-5763671533027328";
        json_options options;
        options.max_nesting_depth(std::numeric_limits<int>::max());

        std::ifstream is(pathname, std::ios_base::in | std::ios_base::binary);
        REQUIRE_THROWS_WITH(json::parse(is, options), Catch::Matchers::Contains(json_error_category_impl().message((int)json_errc::expected_comma_or_right_bracket).c_str()));
    }
    SECTION("issue 21709")
    {
        std::string pathname = "input/fuzz/clusterfuzz-testcase-minimized-fuzz_cbor-5740910806827008.fuzz";

        std::ifstream is(pathname, std::ios_base::in | std::ios_base::binary);
        cbor::cbor_options options;
        options.max_nesting_depth(10000);
        REQUIRE_THROWS_WITH(cbor::decode_cbor<json>(is,options), Catch::Matchers::Contains(cbor::cbor_error_category_impl().message((int)cbor::cbor_errc::max_nesting_depth_exceeded).c_str()));
    }
    SECTION("issue 21710")
    {
        std::string pathname = "input/fuzz/clusterfuzz-testcase-fuzz_cbor-5141282369568768";

        std::ifstream is(pathname, std::ios_base::in | std::ios_base::binary);
        CHECK(is);

        cbor::cbor_options options;
        options.max_nesting_depth(10000);

        REQUIRE_THROWS_WITH(cbor::decode_cbor<json>(is,options), Catch::Matchers::Contains(cbor::cbor_error_category_impl().message((int)cbor::cbor_errc::max_nesting_depth_exceeded).c_str()));
    }

    SECTION("issue 21710b")
    {
        std::string pathname = "input/fuzz/clusterfuzz-testcase-fuzz_cbor-5141282369568768";

        std::ifstream is(pathname, std::ios_base::in | std::ios_base::binary);
        CHECK(is);

        cbor::cbor_options options;
        options.max_nesting_depth(std::numeric_limits<int>::max());

        default_json_visitor visitor;
        cbor::cbor_stream_reader reader(is,visitor,options);

        std::error_code ec;
        REQUIRE_NOTHROW(reader.read(ec));
        CHECK_FALSE(ec);
    }
    SECTION("issue 21697")
    {
        std::string pathname = "input/fuzz/clusterfuzz-testcase-minimized-fuzz_ubjson-5737197673381888";

        std::ifstream is(pathname, std::ios_base::in | std::ios_base::binary);
        CHECK(is);

        json_decoder<json> visitor;

        ubjson::ubjson_stream_reader reader(is,visitor);
        std::error_code ec;
        REQUIRE_NOTHROW(reader.read(ec));
        CHECK(ec == ubjson::ubjson_errc::key_expected);
    }

    SECTION("issue 21663")
    {
        std::string pathname = "input/fuzz/clusterfuzz-testcase-minimized-fuzz_csv-5762751990595584";

        std::ifstream is(pathname, std::ios_base::in | std::ios_base::binary);
        CHECK(is);

        json_decoder<json> visitor;

        csv::csv_options options;
        options.assume_header(true);
        options.mapping(csv::mapping_kind::n_rows);
        csv::csv_reader reader(is,visitor,options);

        std::error_code ec;
        reader.read(ec);
        REQUIRE_NOTHROW(reader.read(ec));
        CHECK_FALSE(ec);

        //std::cout << visitor.get_result() << "\n";
    }

    SECTION("issue 21663b")
    {
        std::string s = "-6.6E6";
        auto result = jsoncons::detail::to_integer<int64_t>(s.data(),s.size());
        CHECK_FALSE(result);
    }

    SECTION("issue 21619")
    {
        std::string pathname = "input/fuzz/clusterfuzz-testcase-minimized-fuzz_cbor-5171679883165696";

        std::ifstream is(pathname, std::ios_base::in | std::ios_base::binary);
        CHECK(is);

        cbor::cbor_options options;
        options.max_nesting_depth(std::numeric_limits<int>::max());

        json_decoder<json> visitor;

        cbor::cbor_stream_reader reader(is,visitor,options);

        std::error_code ec;
        REQUIRE_NOTHROW(reader.read(ec));
        CHECK(ec.value() == (int)cbor::cbor_errc::unexpected_eof);
    }
}

