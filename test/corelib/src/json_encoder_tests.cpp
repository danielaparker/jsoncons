// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <iostream>

using namespace jsoncons;

TEST_CASE("test_byte_string_serialization")
{
    std::vector<uint8_t> bytes = {'H','e','l','l','o'};
    jsoncons::json j(jsoncons::byte_string_arg, bytes);

    std::ostringstream os;
    os << j;

    std::string expected; 
    expected.push_back('\"');
    bytes_to_base64url(bytes.begin(),bytes.end(),expected);
    expected.push_back('\"');

    //std::cout << expected << " " << os.str() << '\n';

    CHECK(expected == os.str());
}

struct json_string_encoder_reset_test_fixture
{
    std::string output1;
    std::string output2;
    json_string_encoder encoder;

    json_string_encoder_reset_test_fixture()
        : encoder(output1,
                  jsoncons::json_options().indent_size(0).new_line_chars("")
                                .spaces_around_comma(spaces_option::no_spaces))
    {}

    std::string string1() const {return output1;}
    std::string string2() const {return output2;}
};

struct json_stream_encoder_reset_test_fixture
{
    std::ostringstream output1;
    std::ostringstream output2;
    json_stream_encoder encoder;

    json_stream_encoder_reset_test_fixture()
        : encoder(output1,
                  jsoncons::json_options().indent_size(0).new_line_chars("")
                      .spaces_around_comma(spaces_option::no_spaces))
    {}

    std::string string1() const {return output1.str();}
    std::string string2() const {return output2.str();}
};

struct compact_json_string_encoder_reset_test_fixture
{
    std::string output1;
    std::string output2;
    compact_json_string_encoder encoder;

    compact_json_string_encoder_reset_test_fixture() : encoder(output1) {}
    std::string string1() const {return output1;}
    std::string string2() const {return output2;}
};

struct compact_json_stream_encoder_reset_test_fixture
{
    std::ostringstream output1;
    std::ostringstream output2;
    compact_json_stream_encoder encoder;

    compact_json_stream_encoder_reset_test_fixture() : encoder(output1) {}
    std::string string1() const {return output1.str();}
    std::string string2() const {return output2.str();}
};

TEMPLATE_TEST_CASE("test_json_encoder_reset", "",
                   json_string_encoder_reset_test_fixture,
                   json_stream_encoder_reset_test_fixture,
                   compact_json_string_encoder_reset_test_fixture,
                   compact_json_stream_encoder_reset_test_fixture)
{
    using fixture_type = TestType;
    fixture_type f;

    // Parially encode, reset, then fully encode to same sink
    f.encoder.begin_array();
    f.encoder.string_value("foo");
    f.encoder.uint64_value(42);
    f.encoder.flush();
    CHECK(f.string1() == R"(["foo",42)");
    f.encoder.reset();
    f.encoder.begin_array();
    f.encoder.string_value("foo");
    f.encoder.uint64_value(42);
    f.encoder.end_array();
    f.encoder.flush();
    CHECK(f.string1() == R"(["foo",42["foo",42])");

    // Reset and encode to different sink
    f.encoder.reset(f.output2);
    f.encoder.begin_array();
    f.encoder.string_value("foo");
    f.encoder.uint64_value(42);
    f.encoder.end_array();
    f.encoder.flush();
    CHECK(f.string2() == R"(["foo",42])");
}

namespace {

    // Fails at the first key and counts the events it receives after that
    class key_failing_visitor : public default_json_visitor
    {
    public:
        int events_after_error{0};
    private:
        bool failed_{false};

        JSONCONS_VISITOR_RETURN_TYPE visit_begin_object(semantic_tag, const ser_context&, std::error_code&) override
        {
            events_after_error += failed_ ? 1 : 0;
            JSONCONS_VISITOR_RETURN;
        }
        JSONCONS_VISITOR_RETURN_TYPE visit_end_object(const ser_context&, std::error_code&) override
        {
            events_after_error += failed_ ? 1 : 0;
            JSONCONS_VISITOR_RETURN;
        }
        JSONCONS_VISITOR_RETURN_TYPE visit_key(const string_view_type&, const ser_context&, std::error_code& ec) override
        {
            events_after_error += failed_ ? 1 : 0;
            failed_ = true;
            ec = std::make_error_code(std::errc::io_error);
            JSONCONS_VISITOR_RETURN;
        }
        JSONCONS_VISITOR_RETURN_TYPE visit_uint64(uint64_t, semantic_tag, const ser_context&, std::error_code&) override
        {
            events_after_error += failed_ ? 1 : 0;
            JSONCONS_VISITOR_RETURN;
        }
    };

} // namespace

TEST_CASE("dump stops at the first visitor error")
{
    auto j = json::parse(R"({"a":1,"b":{"c":2}})");

    SECTION("try_dump")
    {
        key_failing_visitor visitor;
        auto result = j.try_dump(visitor);
        REQUIRE_FALSE(result);
        CHECK(result.error() == std::errc::io_error);
        CHECK(visitor.events_after_error == 0);
    }
    SECTION("dump with error code")
    {
        key_failing_visitor visitor;
        std::error_code ec;
        j.dump(visitor, ec);
        CHECK(ec == std::errc::io_error);
        CHECK(visitor.events_after_error == 0);
    }
}

TEST_CASE("encode_json nesting deeper than max_nesting_depth")
{
    auto options = json_options{}.max_nesting_depth(2);
    for (auto text : {R"([[[[]]]])", R"({"a":{"a":{"a":{}}}})", R"({"a":[[[]]]})"})
    {
        std::string s;
        auto result = try_encode_json(json::parse(text), s, options);
        REQUIRE_FALSE(result);
        CHECK(result.error() == json_errc::max_nesting_depth_exceeded);
    }
}
