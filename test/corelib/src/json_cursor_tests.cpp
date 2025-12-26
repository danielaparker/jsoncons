// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <jsoncons/json_cursor.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

TEST_CASE("json_cursor eof test")
{
    SECTION("empty string 1")
    {
        std::string data = "";

        json_string_cursor cursor(data);
        CHECK(cursor.eof());
    }
    SECTION("string with ws 1")
    {
        std::string data = " ";

        json_string_cursor cursor(data);
        CHECK(cursor.eof());
    }
    SECTION("string source eof 2")
    {
        std::string data = "";

        json_string_cursor cursor(data, json_options{});
        CHECK(cursor.eof());
    }
    SECTION("string source eof 3")
    {
        std::string data = "";

        std::error_code ec;
        json_string_cursor cursor(data, ec);
        CHECK(cursor.eof());
    }
    SECTION("stream source eof test")
    {
        std::string data = "";
        std::istringstream is(data);

        std::error_code ec;
        json_stream_cursor cursor(is, ec);
        CHECK(cursor.eof());
    }
    SECTION("null stream source eof test")
    {   
        std::string data = "";

        std::error_code ec;
        json_string_cursor cursor(data, ec);
        CHECK(cursor.eof());
    }
}

TEST_CASE("json_cursor string_value test")
{
    std::string s = R"("Tom")";
    std::istringstream is(s);

    json_stream_cursor cursor(is);
    REQUIRE_FALSE(cursor.done());

    SECTION("test 1")
    {
        CHECK(staj_event_type::string_value == cursor.current().event_type());
        CHECK(std::string("Tom") == cursor.current().get<std::string>());
        CHECK((cursor.current().get<jsoncons::string_view>() == jsoncons::string_view("Tom")));
        cursor.next();
        CHECK(cursor.done());
    }
}

TEST_CASE("json_cursor string_value as<int> test")
{
    std::string s = R"("-100")";
    std::istringstream is(s);

    json_stream_cursor cursor(is);

    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::string_value == cursor.current().event_type());
    CHECK(-100 == cursor.current().get<int>());
    cursor.next();
    CHECK(cursor.done());
}

TEST_CASE("json_cursor string_value as<unsigned> test")
{
    std::string s = R"("100")";
    std::istringstream is(s);

    json_stream_cursor cursor(is);

    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::string_value == cursor.current().event_type());
    CHECK(100 == cursor.current().get<int>());
    CHECK(100 == cursor.current().get<unsigned>());
    cursor.next();
    CHECK(cursor.done());
}

TEST_CASE("json_cursor null_value test")
{
    std::string s = "null";
    std::istringstream is(s);

    json_stream_cursor cursor(is);

    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::null_value == cursor.current().event_type());
    cursor.next();
    CHECK(cursor.done());
}

TEST_CASE("json_cursor bool_value test")
{
    std::string s = "false";
    std::istringstream is(s);

    json_stream_cursor cursor(is);

    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::bool_value == cursor.current().event_type());
    cursor.next();
    CHECK(cursor.done());
}

TEST_CASE("json_cursor int64_value test")
{
    std::string s = "-100";
    std::istringstream is(s);

    std::error_code ec;
    json_stream_cursor cursor(is, ec);
    REQUIRE_FALSE(ec);

    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::int64_value == cursor.current().event_type());
    CHECK(-100 == cursor.current().get<int>());
    cursor.next(ec);
    REQUIRE_FALSE(ec);
    CHECK(cursor.done());
}

TEST_CASE("json_cursor uint64_value test")
{
    std::string s = "100";
    std::istringstream is(s);

    json_stream_cursor cursor(is);

    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::uint64_value == cursor.current().event_type());
    CHECK(100 == cursor.current().get<int>());
    CHECK(100 == cursor.current().get<unsigned>());
    cursor.next();
    CHECK(cursor.done());
}

TEST_CASE("json_cursor string_value as bignum test")
{
    std::string s = "-18446744073709551617";
    std::istringstream is("\""+s+"\"");

    json_stream_cursor cursor(is);

    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::string_value == cursor.current().event_type());
    CHECK(s == cursor.current().get<std::string>());
    cursor.next();
    CHECK(cursor.done());
}

TEST_CASE("json_cursor bigint value as bignum")
{
    std::string s = "-18446744073709551617";
    std::istringstream is(s);

    json_stream_cursor cursor(is);

    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::string_value == cursor.current().event_type());
    CHECK(semantic_tag::bigint == cursor.current().tag());
    cursor.next();
    CHECK(cursor.done());
}

TEST_CASE("json_cursor double_value test")
{
    std::string s = "100.0";
    std::istringstream is(s);

    json_stream_cursor cursor(is);

    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::double_value == cursor.current().event_type());
    cursor.next();
    CHECK(cursor.done());
}


TEST_CASE("json_cursor array_value test")
{
    std::string s = R"(
    [
        {
            "enrollmentNo" : 100,
            "firstName" : "Tom",
            "lastName" : "Cochrane",
            "mark" : 55              
        },
        {
            "enrollmentNo" : 101,
            "firstName" : "Catherine",
            "lastName" : "Smith",
            "mark" : 95},
        {
            "enrollmentNo" : 102,
            "firstName" : "William",
            "lastName" : "Skeleton",
            "mark" : 60              
        }
    ]
    )";

    std::istringstream is(s);

    json_stream_cursor cursor(is);

    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::begin_array == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::begin_object == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::key == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::uint64_value == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::key == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::string_value == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::key == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::string_value == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::key == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::uint64_value == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::end_object == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::begin_object == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::key == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::uint64_value == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::key == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::string_value == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::key == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::string_value == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::key == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::uint64_value == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::end_object == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::begin_object == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::key == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::uint64_value == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::key == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::string_value == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::key == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::string_value == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::key == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::uint64_value == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::end_object == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::end_array == cursor.current().event_type());
    cursor.next();
    CHECK(cursor.done());
}

TEST_CASE("json_cursor object_value test")
{
    std::string s = R"(
        {
            "enrollmentNo" : 100,
            "firstName" : "Tom",
            "lastName" : "Cochrane",
            "mark" : 55              
        }
    )";

    json_string_cursor cursor(s);

    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::begin_object == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::key == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::uint64_value == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::key == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::string_value == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::key == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::string_value == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::key == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::uint64_value == cursor.current().event_type());
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(staj_event_type::end_object == cursor.current().event_type());
    cursor.next();
    CHECK(cursor.done());
}

struct remove_mark_filter
{
    bool reject_next_ = false;

    bool operator()(const staj_event& event, const ser_context&) 
    {
        if (event.event_type()  == staj_event_type::key &&
            event.get<jsoncons::string_view>() == "mark")
        {
            reject_next_ = true;
            return false;
        }
        else if (reject_next_)
        {
            reject_next_ = false;
            return false;
        }
        else
        {
            return true;
        }
    }
};

TEST_CASE("json_cursor with filter tests")
{
    std::string s = R"(
    [
        {
            "enrollmentNo" : 100,
            "firstName" : "Tom",
            "lastName" : "Cochrane",
            "mark" : 55              
        },
        {
            "enrollmentNo" : 101,
            "firstName" : "Catherine",
            "lastName" : "Smith",
            "mark" : 95},
        {
            "enrollmentNo" : 102,
            "firstName" : "William",
            "lastName" : "Skeleton",
            "mark" : 60              
        }
    ]
    )";

    json_string_cursor cursor(s);

    auto filtered_c = cursor | remove_mark_filter();

    REQUIRE_FALSE(filtered_c.done());
    CHECK(staj_event_type::begin_array == filtered_c.current().event_type());
    filtered_c.next();
    REQUIRE_FALSE(filtered_c.done());
    CHECK(staj_event_type::begin_object == filtered_c.current().event_type());
    filtered_c.next();
    REQUIRE_FALSE(filtered_c.done());
    CHECK(staj_event_type::key == filtered_c.current().event_type());
    filtered_c.next();
    REQUIRE_FALSE(filtered_c.done());
    CHECK(staj_event_type::uint64_value == filtered_c.current().event_type());
    filtered_c.next();
    REQUIRE_FALSE(filtered_c.done());
    CHECK(staj_event_type::key == filtered_c.current().event_type());
    filtered_c.next();
    REQUIRE_FALSE(filtered_c.done());
    CHECK(staj_event_type::string_value == filtered_c.current().event_type());
    filtered_c.next();
    REQUIRE_FALSE(filtered_c.done());
    CHECK(staj_event_type::key == filtered_c.current().event_type());
    filtered_c.next();
    REQUIRE_FALSE(filtered_c.done());
    CHECK(staj_event_type::string_value == filtered_c.current().event_type());
    filtered_c.next();
    REQUIRE_FALSE(filtered_c.done());
    CHECK(staj_event_type::end_object == filtered_c.current().event_type());
    filtered_c.next();
    REQUIRE_FALSE(filtered_c.done());
    CHECK(staj_event_type::begin_object == filtered_c.current().event_type());
    filtered_c.next();
    REQUIRE_FALSE(filtered_c.done());
    CHECK(staj_event_type::key == filtered_c.current().event_type());
    filtered_c.next();
    REQUIRE_FALSE(filtered_c.done());
    CHECK(staj_event_type::uint64_value == filtered_c.current().event_type());
    filtered_c.next();
    REQUIRE_FALSE(filtered_c.done());
    CHECK(staj_event_type::key == filtered_c.current().event_type());
    filtered_c.next();
    REQUIRE_FALSE(filtered_c.done());
    CHECK(staj_event_type::string_value == filtered_c.current().event_type());
    filtered_c.next();
    REQUIRE_FALSE(filtered_c.done());
    CHECK(staj_event_type::key == filtered_c.current().event_type());
    filtered_c.next();
    REQUIRE_FALSE(filtered_c.done());
    CHECK(staj_event_type::string_value == filtered_c.current().event_type());
    filtered_c.next();
    REQUIRE_FALSE(filtered_c.done());
    CHECK(staj_event_type::end_object == filtered_c.current().event_type());
    filtered_c.next();
    REQUIRE_FALSE(filtered_c.done());
    CHECK(staj_event_type::begin_object == filtered_c.current().event_type());
    filtered_c.next();
    REQUIRE_FALSE(filtered_c.done());
    CHECK(staj_event_type::key == filtered_c.current().event_type());
    filtered_c.next();
    REQUIRE_FALSE(filtered_c.done());
    CHECK(staj_event_type::uint64_value == filtered_c.current().event_type());
    filtered_c.next();
    REQUIRE_FALSE(filtered_c.done());
    CHECK(staj_event_type::key == filtered_c.current().event_type());
    filtered_c.next();
    REQUIRE_FALSE(filtered_c.done());
    CHECK(staj_event_type::string_value == filtered_c.current().event_type());
    filtered_c.next();
    REQUIRE_FALSE(filtered_c.done());
    CHECK(staj_event_type::key == filtered_c.current().event_type());
    filtered_c.next();
    REQUIRE_FALSE(filtered_c.done());
    CHECK(staj_event_type::string_value == filtered_c.current().event_type());
    filtered_c.next();
    REQUIRE_FALSE(filtered_c.done());
    CHECK(staj_event_type::end_object == filtered_c.current().event_type());
    filtered_c.next();
    REQUIRE_FALSE(filtered_c.done());
    CHECK(staj_event_type::end_array == filtered_c.current().event_type());
    filtered_c.next();
    CHECK(filtered_c.done());
}

namespace {
namespace ns {

    struct book
    {
        std::string author;
        std::string title;
        double price{0};

        book() = default;

        book(const std::string& author, 
             const std::string& title, 
             double price) : 
            author(author), title(title), price(price)
        {
        }
    };

} // namespace ns
} // namespace

JSONCONS_ALL_MEMBER_TRAITS(ns::book,author,title,price)

TEST_CASE("staj event as object")
{
    std::vector<ns::book> books;
    books.emplace_back("Haruki Murakami",
                       "Kafka on the Shore",
                       25.17);
    books.emplace_back("Charles Bukowski",
                       "Women: A Novel",
                       12.0);

    std::string buffer;
    encode_json(books, buffer, indenting::indent);

    SECTION("test 1")
    {
        json_string_cursor cursor(buffer);
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_event_type::begin_array == cursor.current().event_type());
        cursor.next();
        CHECK(staj_event_type::begin_object == cursor.current().event_type());
        cursor.next();
        CHECK(staj_event_type::key == cursor.current().event_type());
        cursor.next();
        CHECK(staj_event_type::string_value == cursor.current().event_type());
        cursor.next();
        CHECK(staj_event_type::key == cursor.current().event_type());
        cursor.next();
        CHECK(staj_event_type::string_value == cursor.current().event_type());
        cursor.next();
        CHECK(staj_event_type::key == cursor.current().event_type());
        cursor.next();
        CHECK(staj_event_type::double_value == cursor.current().event_type());
        cursor.next();
        CHECK(staj_event_type::end_object == cursor.current().event_type());
        cursor.next();
        CHECK(staj_event_type::begin_object == cursor.current().event_type());
        cursor.next();
        CHECK(staj_event_type::key == cursor.current().event_type());
        cursor.next();
        CHECK(staj_event_type::string_value == cursor.current().event_type());
        cursor.next();
        CHECK(staj_event_type::key == cursor.current().event_type());
        cursor.next();
        CHECK(staj_event_type::string_value == cursor.current().event_type());
        cursor.next();
        CHECK(staj_event_type::key == cursor.current().event_type());
        cursor.next();
        CHECK(staj_event_type::double_value == cursor.current().event_type());
        cursor.next();
        CHECK(staj_event_type::end_object == cursor.current().event_type());
        cursor.next();
        CHECK(staj_event_type::end_array == cursor.current().event_type());
        cursor.next();
        CHECK(cursor.done());
    }
    SECTION("test read_to")
    {
        json document = json::parse(buffer);

        json_string_cursor cursor(buffer);
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_event_type::begin_array == cursor.current().event_type());
        cursor.next();
        CHECK(staj_event_type::begin_object == cursor.current().event_type());

        json_decoder<json> decoder;
        cursor.read_to(decoder);
        CHECK(staj_event_type::end_object == cursor.current().event_type());
        json j0 = decoder.get_result();
        CHECK((j0 == document[0]));

        cursor.next();

        json_decoder<json> decoder2;
        cursor.read_to(decoder2);
        CHECK(staj_event_type::end_object == cursor.current().event_type());
        json j1 = decoder2.get_result();
        CHECK((j1 == document[1]));
    }
}

TEMPLATE_TEST_CASE("json_cursor reset test", "",
                   (std::pair<json_string_cursor, std::string>),
                   (std::pair<json_stream_cursor, std::istringstream>))
{
    using cursor_type = typename TestType::first_type;
    using input_type = typename TestType::second_type;

    SECTION("keeping same source")
    {
        input_type input(R"("Tom" -100 null)");
        cursor_type cursor(input);
        std::error_code ec;

        CHECK(staj_event_type::string_value == cursor.current().event_type());
        CHECK(std::string("Tom") == cursor.current().template get<std::string>());
        CHECK(cursor.current().template get<jsoncons::string_view>() ==
              jsoncons::string_view("Tom"));
        REQUIRE_FALSE(cursor.done());
        cursor.reset();
        CHECK(staj_event_type::int64_value == cursor.current().event_type());
        CHECK(-100 == cursor.current().template get<int>());
        cursor.reset(ec);
        REQUIRE_FALSE(ec);
        CHECK(staj_event_type::null_value == cursor.current().event_type());
        CHECK_FALSE(cursor.done());
        cursor.next(ec);
        REQUIRE_FALSE(ec);
    }

    SECTION("with another source")
    {
        input_type input0;
        input_type input1(R"("Tom")");
        input_type input2("bad");
        input_type input3("-100");
        cursor_type cursor(input0);
        std::error_code ec;

        REQUIRE(cursor.done());
        cursor.reset(input1);
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_event_type::string_value == cursor.current().event_type());
        CHECK(std::string("Tom") == cursor.current().template get<std::string>());
        CHECK(cursor.current().template get<jsoncons::string_view>() ==
              jsoncons::string_view("Tom"));
        cursor.next();
        CHECK(cursor.done());

        cursor.reset(input2, ec);
        CHECK(ec == json_errc::syntax_error);
        CHECK(cursor.done()); //REVISIT Changed behavior

        // Check that cursor can reused be upon reset following an error.
        ec = json_errc::success;
        cursor.reset(input3, ec);
        REQUIRE_FALSE(ec);
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_event_type::int64_value == cursor.current().event_type());
        CHECK(-100 == cursor.current().template get<int>());
        cursor.next(ec);
        REQUIRE_FALSE(ec);
        CHECK(cursor.done());
    }
}

