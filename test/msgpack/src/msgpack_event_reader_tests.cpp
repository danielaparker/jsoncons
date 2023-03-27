// Copyright 2013-2023 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <jsoncons_ext/msgpack/msgpack_event_reader.hpp>
#include <jsoncons_ext/msgpack/msgpack.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

TEST_CASE("msgpack_event_reader reputon test")
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
    msgpack::encode_msgpack(j, data);

    SECTION("test 1")
    {
        msgpack::msgpack_bytes_cursor2 cursor(data);

        CHECK(cursor.current().event_type() == staj2_event_type::begin_object);
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::string_value);  // key
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::string_value);  // key
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::begin_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::begin_object);
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::string_value);  // key
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::string_value);  // key
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::string_value);  // key
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::string_value);  // key
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::double_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::end_object);
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::end_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj2_event_type::end_object);
        cursor.next();
        CHECK(cursor.done());
    }
}

struct msgpack_bytes_cursor2_reset_test_traits
{
    using cursor_type = msgpack::msgpack_bytes_cursor2;
    using input_type = std::vector<uint8_t>;

    static void set_input(input_type& input, input_type bytes) {input = bytes;}
};

struct msgpack_stream_cursor2_reset_test_traits
{
    using cursor_type = msgpack::msgpack_stream_cursor2;

    // binary_stream_source::char_type is actually char, not uint8_t
    using input_type = std::istringstream;

    static void set_input(input_type& input, std::vector<uint8_t> bytes)
    {
        auto data = reinterpret_cast<const char*>(bytes.data());
        std::string s(data, bytes.size());
        input.str(s);
    }
};

TEMPLATE_TEST_CASE("msgpack_event_reader reset test", "",
                   msgpack_bytes_cursor2_reset_test_traits,
                   msgpack_stream_cursor2_reset_test_traits)
{
    using traits = TestType;
    using input_type = typename traits::input_type;
    using cursor_type = typename traits::cursor_type;
    using source_type = typename cursor_type::source_type;
    using event_type = staj2_event_type;

    SECTION("keeping same source")
    {
        std::error_code ec;
        input_type input;
        traits::set_input(input, {
            0xa3, 0x54, 0x6f, 0x6d, // str(3), "Tom"
            0xd0, 0x9c, // int8(-100)
            0xc0 // nil
        });
        source_type source(input);
        cursor_type cursor(std::move(source));

        REQUIRE_FALSE(cursor.done());
        CHECK(cursor.current().event_type() == event_type::string_value);
        CHECK(cursor.current().tag() == semantic_tag::none);
        CHECK(cursor.current().template get<std::string>() == std::string("Tom"));
        CHECK(cursor.current().template get<jsoncons::string_view>() ==
              jsoncons::string_view("Tom"));
        cursor.next();
        CHECK(cursor.done());

        cursor.reset();
        REQUIRE_FALSE(cursor.done());
        CHECK(cursor.current().event_type() == event_type::int64_value);
        CHECK(cursor.current().tag() == semantic_tag::none);
        CHECK(cursor.current().template get<int>() == -100);
        cursor.next();
        CHECK(cursor.done());

        cursor.reset(ec);
        REQUIRE_FALSE(ec);
        REQUIRE_FALSE(cursor.done());
        CHECK(cursor.current().event_type() == event_type::null_value);
        CHECK(cursor.current().tag() == semantic_tag::none);
        cursor.next(ec);
        REQUIRE_FALSE(ec);
        CHECK(cursor.done());
    }

    SECTION("with another source")
    {
        std::error_code ec;
        input_type input0;
        input_type input1;
        input_type input2;
        input_type input3;
        traits::set_input(input0, {});
        traits::set_input(input1, {0xa3, 0x54, 0x6f, 0x6d}); // str(3), "Tom"
        traits::set_input(input2, {0xc1}); // never used
        traits::set_input(input3, {0xd0, 0x9c}); // int8(-100)

        // Constructing cursor with blank input results in unexpected_eof
        // error because it eagerly parses the next event upon construction.
        cursor_type cursor(input0, ec);
        CHECK(ec == msgpack::msgpack_errc::unexpected_eof);
        CHECK_FALSE(cursor.done());

        // Reset to valid input1
        cursor.reset(input1);
        CHECK(cursor.current().event_type() == event_type::string_value);
        CHECK(cursor.current().tag() == semantic_tag::none);
        CHECK(cursor.current().template get<std::string>() == std::string("Tom"));
        CHECK(cursor.current().template get<jsoncons::string_view>() ==
              jsoncons::string_view("Tom"));
        ec = msgpack::msgpack_errc::success;
        REQUIRE_FALSE(cursor.done());
        cursor.next(ec);
        CHECK_FALSE(ec);
        CHECK(cursor.done());

        // Reset to invalid input2
        cursor.reset(input2, ec);
        CHECK(ec == msgpack::msgpack_errc::unknown_type);
        CHECK_FALSE(cursor.done());

        // Reset to valid input3
        ec = msgpack::msgpack_errc::success;
        cursor.reset(input3, ec);
        REQUIRE_FALSE(ec);
        CHECK(cursor.current().event_type() == event_type::int64_value);
        CHECK(cursor.current().tag() == semantic_tag::none);
        CHECK(cursor.current().template get<int>() == -100);
        REQUIRE_FALSE(cursor.done());
        cursor.next(ec);
        CHECK_FALSE(ec);
        CHECK(cursor.done());
    }
}
