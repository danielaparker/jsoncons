// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <vector>
#include <utility>
#include <ctime>
#include <fstream>
#include <catch/catch.hpp>

using namespace jsoncons;

TEST_CASE("clusterfuzz issues")
{
    SECTION("issue 21589")
    {
        std::string pathname = "input/clusterfuzz/clusterfuzz-testcase-minimized-fuzz_parse-5763671533027328";
        json_options options;
        options.max_nesting_depth(std::numeric_limits<int>::max());

        std::ifstream is(pathname);
        REQUIRE_THROWS_WITH(json::parse(is, options), Catch::Matchers::Contains(json_error_category_impl().message((int)json_errc::expected_comma_or_right_bracket).c_str()));
    }
    SECTION("issue 21709")
    {
        std::string pathname = "input/clusterfuzz/clusterfuzz-testcase-minimized-fuzz_cbor-5740910806827008.fuzz";

        std::ifstream is(pathname);
        cbor::cbor_options options;
        options.max_nesting_depth(std::numeric_limits<int>::max());
        REQUIRE_THROWS_WITH(cbor::decode_cbor<json>(is, options), Catch::Matchers::Contains(cbor::cbor_error_category_impl().message((int)cbor::cbor_errc::invalid_utf8_text_string).c_str()));
    }

}

