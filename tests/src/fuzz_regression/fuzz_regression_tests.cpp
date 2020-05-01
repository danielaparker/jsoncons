// Copyright 2020 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <jsoncons_ext/ubjson/ubjson.hpp>
#include <jsoncons_ext/msgpack/msgpack.hpp>
#include <jsoncons_ext/bson/bson.hpp>
#include <jsoncons_ext/csv/csv.hpp>
#include <vector>
#include <utility>
#include <ctime>
#include <fstream>
#include <catch/catch.hpp>

using namespace jsoncons;

namespace {

    class diagnostics_visitor : public default_json_visitor
    {
        bool visit_begin_object(semantic_tag, const ser_context&, std::error_code&) override
        {
            std::cout << "visit_begin_object" << std::endl; 
            return true;
        }

        bool visit_begin_object(size_t length, semantic_tag, const ser_context&, std::error_code&) override
        {
            std::cout << "visit_begin_object " << length << std::endl; 
            return true;
        }

        bool visit_end_object(const ser_context&, std::error_code&) override
        {
            std::cout << "visit_end_object" << std::endl; 
            return true;
        }
        bool visit_begin_array(size_t length, semantic_tag, const ser_context&, std::error_code&) override
        {
            std::cout << "visit_begin_array " << length << std::endl; 
            return true;
        }

        bool visit_end_array(const ser_context&, std::error_code&) override
        {
            std::cout << "visit_end_array" << std::endl; 
            return true;
        }

        bool visit_key(const string_view_type& s, const ser_context&, std::error_code&) override
        {
            std::cout << "visit_key " << s << std::endl; 
            return true;
        }
        bool visit_string(const string_view_type& s, semantic_tag, const ser_context&, std::error_code&) override
        {
            std::cout << "visit_string " << s << std::endl; 
            return true;
        }
        bool visit_int64(int64_t val, semantic_tag, const ser_context&, std::error_code&) override
        {
            std::cout << "visit_int64 " << val << std::endl; 
            return true;
        }
        bool visit_uint64(uint64_t val, semantic_tag, const ser_context&, std::error_code&) override
        {
            std::cout << "visit_uint64 " << val << std::endl; 
            return true;
        }
        bool visit_bool(bool val, semantic_tag, const ser_context&, std::error_code&) override
        {
            std::cout << "visit_bool " << val << std::endl; 
            return true;
        }
        bool visit_null(semantic_tag, const ser_context&, std::error_code&) override
        {
            std::cout << "visit_null " << std::endl; 
            return true;
        }

        bool visit_typed_array(const span<const uint16_t>& s, 
                                    semantic_tag tag, 
                                    const ser_context&, 
                                    std::error_code&) override  
        {
            std::cout << "visit_typed_array uint16_t " << tag << std::endl; 
            for (auto val : s)
            {
                std::cout << val << "" << std::endl;
            }
            std::cout << "" << std::endl;
            return true;
        }

        bool visit_typed_array(half_arg_t, const span<const uint16_t>& s,
            semantic_tag tag,
            const ser_context&,
            std::error_code&) override
        {
            std::cout << "visit_typed_array half_arg_t uint16_t " << tag << "" << std::endl;
            for (auto val : s)
            {
                std::cout << val << "" << std::endl;
            }
            std::cout << "" << std::endl;
            return true;
        }
    };

    class diagnostics_visitor2 : public default_json_visitor2
    {
        bool visit_begin_object(semantic_tag, const ser_context&, std::error_code&) override
        {
            std::cout << "visit_begin_object" << std::endl; 
            return true;
        }

        bool visit_begin_object(size_t length, semantic_tag, const ser_context&, std::error_code&) override
        {
            std::cout << "visit_begin_object " << length << std::endl; 
            return true;
        }

        bool visit_end_object(const ser_context&, std::error_code&) override
        {
            std::cout << "visit_end_object" << std::endl; 
            return true;
        }
        bool visit_begin_array(size_t length, semantic_tag, const ser_context&, std::error_code&) override
        {
            std::cout << "visit_begin_array " << length << std::endl; 
            return true;
        }

        bool visit_end_array(const ser_context&, std::error_code&) override
        {
            std::cout << "visit_end_array" << std::endl; 
            return true;
        }

        bool visit_string(const string_view_type& s, semantic_tag, const ser_context&, std::error_code&) override
        {
            std::cout << "visit_string " << s << std::endl; 
            return true;
        }
        bool visit_int64(int64_t val, semantic_tag, const ser_context&, std::error_code&) override
        {
            std::cout << "visit_int64 " << val << std::endl; 
            return true;
        }
        bool visit_uint64(uint64_t val, semantic_tag, const ser_context&, std::error_code&) override
        {
            std::cout << "visit_uint64 " << val << std::endl; 
            return true;
        }
        bool visit_bool(bool val, semantic_tag, const ser_context&, std::error_code&) override
        {
            std::cout << "visit_bool " << val << std::endl; 
            return true;
        }
        bool visit_null(semantic_tag, const ser_context&, std::error_code&) override
        {
            std::cout << "visit_null " << std::endl; 
            return true;
        }

        bool visit_typed_array(const span<const uint16_t>& s, 
                                    semantic_tag tag, 
                                    const ser_context&, 
                                    std::error_code&) override  
        {
            std::cout << "visit_typed_array uint16_t " << tag << std::endl; 
            for (auto val : s)
            {
                std::cout << val << "" << std::endl;
            }
            std::cout << "" << std::endl;
            return true;
        }

        bool visit_typed_array(half_arg_t, const span<const uint16_t>& s,
            semantic_tag tag,
            const ser_context&,
            std::error_code&) override
        {
            std::cout << "visit_typed_array half_arg_t uint16_t " << tag << "" << std::endl;
            for (auto val : s)
            {
                std::cout << val << "" << std::endl;
            }
            std::cout << "" << std::endl;
            return true;
        }
    };
} // namespace

TEST_CASE("oss-fuzz issues")
{
#if 0
    // Fuzz target: fuzz_parse
    // Issue: Stack-overflow
    // Diagnosis: During basic_json destruction, an internal compiler stack error occurred in std::vector 
    //            destructor after reaching a certain nesting depth,
    //            approximately 270 with visual studio on windows.
    // Resolution: 
    // - Implement destructors for json_array and json_object that flatten std::vector elements
    // - max_nesting_depth option for all parsers and encoders (default 1024)
    SECTION("issue 21589")
    {
        std::string pathname = "input/fuzz/clusterfuzz-testcase-minimized-fuzz_parse-5763671533027328";
        json_options options;
        options.max_nesting_depth(std::numeric_limits<int>::max());

        std::ifstream is(pathname, std::ios_base::in | std::ios_base::binary);
        REQUIRE_THROWS_WITH(json::parse(is, options), Catch::Matchers::Contains(json_error_category_impl().message((int)json_errc::expected_comma_or_right_bracket).c_str()));
    }

    // Fuzz target: fuzz_cbo
    // Issue: Abrt in __cxxabiv1::failed_throw
    // Diagnosis: Huge length field in binary data formats
    // Resolution: Read from source in chunks, to avoid bad_alloc, and fail with unexpected_eof
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
        CHECK((ec.value() == (int)cbor::cbor_errc::unexpected_eof ||  // x64 arch
               ec.value() == (int)cbor::cbor_errc::number_too_large)); // x86 arch  
    }

    // Fuzz target: fuzz_cbor
    // Issue: Timeout in fuzz_cbor
    SECTION("issue 21631")
    {
        std::string pathname = "input/fuzz/clusterfuzz-testcase-minimized-fuzz_cbor-5639265590706176";

        std::ifstream is(pathname, std::ios_base::in | std::ios_base::binary);
        CHECK(is);

        default_json_visitor visitor;

        cbor::cbor_options options;
        options.max_nesting_depth(std::numeric_limits<int>::max());

        cbor::cbor_stream_reader reader(is,visitor,options);

        std::error_code ec;
        REQUIRE_NOTHROW(reader.read(ec));
        CHECK((ec == cbor::cbor_errc::unknown_type || // x64 arch
               ec == cbor::cbor_errc::number_too_large)); // x86 arch
    }

    // Fuzz target: fuzz_csv
    // Issue: Integer-overflow
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

        //std::cout << visitor.get_result() << "" << std::endl;
    }

    SECTION("issue 21663b")
    {
        std::string s = "-6.6E6";
        auto result = jsoncons::detail::to_integer<int64_t>(s.data(),s.size());
        CHECK_FALSE(result);
    }

    // Fuzz target: fuzz_ubjson
    // Issue: Out-of-memory
    // Diagnosis:  Issue with huge length for a strongly typed array of no-op, null, false, or true,
    // e.g. [[][$][T][#][I][9223372036854775807]
    // Resolution: limit number of items to max_items set in options
    SECTION("issue 21667")
    {
        std::string pathname = "input/fuzz/clusterfuzz-testcase-minimized-fuzz_ubjson-5738905124208640";

        std::ifstream is(pathname, std::ios_base::in | std::ios_base::binary);
        CHECK(is);

        default_json_visitor visitor;

        ubjson::ubjson_stream_reader reader(is,visitor);
        std::error_code ec;
        REQUIRE_NOTHROW(reader.read(ec));
        CHECK((ec == ubjson::ubjson_errc::max_items_exceeded || // x64 arch
               ec == ubjson::ubjson_errc::number_too_large)); // x86 arch
    }

    // Fuzz target: fuzz_ubjson
    // Issue: Timeout 
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

    // Fuzz target: fuzz_cbor
    // Issue: Stack overflow
    SECTION("issue 21709")
    {
        std::string pathname = "input/fuzz/clusterfuzz-testcase-minimized-fuzz_cbor-5740910806827008.fuzz";

        std::ifstream is(pathname, std::ios_base::in | std::ios_base::binary);
        cbor::cbor_options options;
        options.max_nesting_depth(10000);
        REQUIRE_THROWS_WITH(cbor::decode_cbor<json>(is,options), Catch::Matchers::Contains(cbor::cbor_error_category_impl().message((int)cbor::cbor_errc::max_nesting_depth_exceeded).c_str()));
    }

    // Fuzz target: fuzz_cbor
    // Issue: Stack overflow
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
        CHECK(ec.value() == (int)cbor::cbor_errc::unknown_type);
    }

    // Fuzz target: fuzz_msgpack
    // Issue: Out of memory
    SECTION("issue 21801")
    {
        std::string pathname = "input/fuzz/clusterfuzz-testcase-minimized-fuzz_msgpack-5651190114418688";

        std::ifstream is(pathname, std::ios_base::in | std::ios_base::binary);
        CHECK(is);

        json_decoder<json> visitor;

        msgpack::msgpack_options options;
        //options.max_nesting_depth(std::numeric_limits<int>::max());

        msgpack::msgpack_stream_reader reader(is,visitor);
        std::error_code ec;
        REQUIRE_NOTHROW(reader.read(ec));
        CHECK(ec == msgpack::msgpack_errc::unexpected_eof);
    }

    // Fuzz target: fuzz_cbor
    // Issue: Stack overflow
    SECTION("issue 21805")
    {
        std::string pathname = "input/fuzz/clusterfuzz-testcase-fuzz_cbor-5687592176844800";

        std::ifstream is(pathname, std::ios_base::in | std::ios_base::binary);
        CHECK(is);

        default_json_visitor visitor;

        cbor::cbor_options options;

        cbor::cbor_stream_reader reader(is,visitor,options);

        std::error_code ec;
        //reader.read(ec);
        //std::cout << ec.message() << "" << std::endl;
        REQUIRE_NOTHROW(reader.read(ec));
        CHECK((ec == cbor::cbor_errc::max_nesting_depth_exceeded)); 
    }

    // Fuzz target: fuzz_msgpack
    // Issue: Timeout
    SECTION("issue 21813")
    {
        std::string pathname = "input/fuzz/clusterfuzz-testcase-fuzz_msgpack-5727715157344256";

        std::ifstream is(pathname, std::ios_base::in | std::ios_base::binary);
        CHECK(is);

        json_decoder<json> visitor;

        msgpack::msgpack_options options;
        options.max_nesting_depth(std::numeric_limits<int>::max());

        msgpack::msgpack_stream_reader reader(is,visitor,options);
        std::error_code ec;
        REQUIRE_NOTHROW(reader.read(ec));
        CHECK(ec == msgpack::msgpack_errc::unexpected_eof);
    }

    // Fuzz target: fuzz_ubjson
    // Issue: Timeout
    SECTION("issue 21865")
    {
        std::string pathname = "input/fuzz/clusterfuzz-testcase-fuzz_bson-5637264110780416";

        std::ifstream is(pathname, std::ios_base::in | std::ios_base::binary);
        CHECK(is);

        json_decoder<json> visitor;
        //diagnostics_visitor visitor;

        bson::bson_options options;
        options.max_nesting_depth(std::numeric_limits<int>::max());

        bson::bson_stream_reader reader(is,visitor,options);
        std::error_code ec;
        REQUIRE_NOTHROW(reader.read(ec));
        CHECK(ec == bson::bson_errc::unexpected_eof);
    }

    // Fuzz target: fuzz_cbor_encoder
    // Issue: failed_throw
    // Resolution: change assert to illegal_chunked_string error code
    SECTION("issue 21902")
    {
        std::string pathname = "input/fuzz/clusterfuzz-testcase-fuzz_cbor_encoder-5665976638242816";

        std::ifstream is(pathname, std::ios_base::in | std::ios_base::binary);
        CHECK(is);

        try
        {
            std::vector<uint8_t> buf;
            cbor::cbor_bytes_encoder encoder(buf);
            cbor::cbor_stream_reader reader(is, encoder);

            std::error_code ec;
            REQUIRE_NOTHROW(reader.read(ec));
            CHECK(ec == cbor::cbor_errc::illegal_chunked_string);
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << "" << std::endl;
        }
    }

    // Fuzz target: fuzz_csv_encoder
    // Issue: Failed throw
    // Resolution: check if csv_parser is still in start state when no more input
    SECTION("issue 21912")
    {
        std::string pathname = "input/fuzz/clusterfuzz-testcase-minimized-fuzz_csv_encoder-5202115606872064.fuzz";

        std::ifstream is(pathname, std::ios_base::in | std::ios_base::binary);
        CHECK(is);

        std::string s2;
        csv::csv_string_encoder visitor(s2);

        csv::csv_reader reader(is, visitor);
        std::error_code ec;
        reader.read(ec);

        REQUIRE_NOTHROW(reader.read(ec));
        CHECK((ec == csv::csv_errc::source_error));

        //std::cout << visitor.get_result() << "" << std::endl;
    }
#endif
    // Fuzz target: fuzz_cbor
    // Issue: failed_throw
    // Notes: visit_key {,{}:{null:null,null:null,null:null,0:}}
    SECTION("issue 21948")
    {
        std::string pathname = "input/fuzz/clusterfuzz-testcase-minimized-fuzz_cbor-5743359164678144";

        std::ifstream is(pathname, std::ios_base::in | std::ios_base::binary);
        CHECK(is);

        //diagnostics_visitor visitor0;
        //json_decoder<json> visitor1;
        //json_tee_visitor visitor(visitor0, visitor1);

        diagnostics_visitor2 visitor;

        //json_decoder<json> visitor;
        cbor::cbor_options options;

        cbor::cbor_stream_reader reader(is,visitor,options);

        std::error_code ec;
        reader.read(ec);
        std::cout << ec.message() << "" << std::endl;
        //REQUIRE_NOTHROW(reader.read(ec));
        //CHECK((ec == cbor::cbor_errc::max_nesting_depth_exceeded)); 
    }
}

