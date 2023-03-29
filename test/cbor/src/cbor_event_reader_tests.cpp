// Copyright 2013-2023 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <jsoncons_ext/cbor/cbor_event_reader.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

TEST_CASE("cbor_event_reader reputon test")
{
    ojson j = ojson::parse(R"(
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

    std::vector<uint8_t> data;
    cbor::encode_cbor(j, data);

    SECTION("test 1")
    {
        cbor::cbor_bytes_event_reader event_reader(data);

        CHECK(event_reader.event_kind() == item_event_kind::begin_object);
        CHECK(event_reader.size() == 2);
        event_reader.next();
        CHECK(event_reader.event_kind() == item_event_kind::string_value);  // key
        event_reader.next();
        CHECK(event_reader.event_kind() == item_event_kind::string_value);
        event_reader.next();
        CHECK(event_reader.event_kind() == item_event_kind::string_value);  // key
        event_reader.next();
        CHECK(event_reader.event_kind() == item_event_kind::begin_array);
        CHECK(event_reader.size() == 1);
        event_reader.next();
        CHECK(event_reader.event_kind() == item_event_kind::begin_object);
        event_reader.next();
        CHECK(event_reader.event_kind() == item_event_kind::string_value);  // key
        event_reader.next();
        CHECK(event_reader.event_kind() == item_event_kind::string_value);
        event_reader.next();
        CHECK(event_reader.event_kind() == item_event_kind::string_value);  // key
        event_reader.next();
        CHECK(event_reader.event_kind() == item_event_kind::string_value);
        event_reader.next();
        CHECK(event_reader.event_kind() == item_event_kind::string_value);  // key
        event_reader.next();
        CHECK(event_reader.event_kind() == item_event_kind::string_value);
        event_reader.next();
        CHECK(event_reader.event_kind() == item_event_kind::string_value);  // key
        event_reader.next();
        CHECK(event_reader.event_kind() == item_event_kind::double_value);
        event_reader.next();
        CHECK(event_reader.event_kind() == item_event_kind::end_object);
        event_reader.next();
        CHECK(event_reader.event_kind() == item_event_kind::end_array);
        event_reader.next();
        CHECK(event_reader.event_kind() == item_event_kind::end_object);
        event_reader.next();
        CHECK(event_reader.done());
    }
}

struct cbor_bytes_cursor2_reset_test_traits
{
    using cursor_type = cbor::cbor_bytes_event_reader;
    using input_type = std::vector<uint8_t>;

    static void set_input(input_type& input, input_type bytes) {input = bytes;}
};

struct cbor_stream_cursor2_reset_test_traits
{
    using cursor_type = cbor::cbor_stream_event_reader;

    // binary_stream_source::char_type is actually char, not uint8_t
    using input_type = std::istringstream;

    static void set_input(input_type& input, std::vector<uint8_t> bytes)
    {
        auto data = reinterpret_cast<const char*>(bytes.data());
        std::string s(data, bytes.size());
        input.str(s);
    }
};

TEMPLATE_TEST_CASE("cbor_event_reader reset test", "",
                   cbor_bytes_cursor2_reset_test_traits,
                   cbor_stream_cursor2_reset_test_traits)
{
    using traits = TestType;
    using input_type = typename traits::input_type;
    using cursor_type = typename traits::cursor_type;
    using source_type = typename cursor_type::source_type;

    SECTION("keeping same source")
    {
        std::error_code ec;
        input_type input;
        traits::set_input(input, {
            0x63, 0x54, 0x6f, 0x6d, // text(3), "Tom"
            0x38, 0x63, // negative(99)
            0xf6 // null
        });
        source_type source(input);
        cursor_type event_reader(std::move(source));

        REQUIRE_FALSE(event_reader.done());
        CHECK(event_reader.event_kind() == item_event_kind::string_value);
        CHECK(event_reader.tag() == semantic_tag::none);
        CHECK(event_reader.template get<std::string>() == std::string("Tom"));
        CHECK(event_reader.template get<jsoncons::string_view>() ==
              jsoncons::string_view("Tom"));
        event_reader.next();
        CHECK(event_reader.done());

        event_reader.reset();
        REQUIRE_FALSE(event_reader.done());
        CHECK(event_reader.event_kind() == item_event_kind::int64_value);
        CHECK(event_reader.tag() == semantic_tag::none);
        CHECK(event_reader.template get<int>() == -100);
        event_reader.next();
        CHECK(event_reader.done());

        event_reader.reset(ec);
        REQUIRE_FALSE(ec);
        REQUIRE_FALSE(event_reader.done());
        CHECK(event_reader.event_kind() == item_event_kind::null_value);
        CHECK(event_reader.tag() == semantic_tag::none);
        event_reader.next(ec);
        REQUIRE_FALSE(ec);
        CHECK(event_reader.done());
    }

    SECTION("with another source")
    {
        std::error_code ec;
        input_type input0;
        input_type input1;
        input_type input2;
        input_type input3;
        traits::set_input(input0, {});
        traits::set_input(input1, {0x63, 0x54, 0x6f, 0x6d}); // text(3), "Tom"
        traits::set_input(input2, {0xe0}); // invalid special
        traits::set_input(input3, {0x38, 0x63}); // negative(99)

        // Constructing event_reader with blank input results in unexpected_eof
        // error because it eagerly parses the next event upon construction.
        cursor_type event_reader(input0, ec);
        CHECK(ec == cbor::cbor_errc::unexpected_eof);
        CHECK_FALSE(event_reader.done());

        // Reset to valid input1
        event_reader.reset(input1);
        CHECK(event_reader.event_kind() == item_event_kind::string_value);
        CHECK(event_reader.tag() == semantic_tag::none);
        CHECK(event_reader.template get<std::string>() == std::string("Tom"));
        CHECK(event_reader.template get<jsoncons::string_view>() ==
              jsoncons::string_view("Tom"));
        ec = cbor::cbor_errc::success;
        REQUIRE_FALSE(event_reader.done());
        event_reader.next(ec);
        CHECK_FALSE(ec);
        CHECK(event_reader.done());

        // Reset to invalid input2
        event_reader.reset(input2, ec);
        CHECK(ec == cbor::cbor_errc::unknown_type);
        CHECK_FALSE(event_reader.done());

        // Reset to valid input3
        ec = cbor::cbor_errc::success;
        event_reader.reset(input3, ec);
        REQUIRE_FALSE(ec);
        CHECK(event_reader.event_kind() == item_event_kind::int64_value);
        CHECK(event_reader.tag() == semantic_tag::none);
        CHECK(event_reader.template get<int>() == -100);
        REQUIRE_FALSE(event_reader.done());
        event_reader.next(ec);
        CHECK_FALSE(ec);
        CHECK(event_reader.done());
    }
}

