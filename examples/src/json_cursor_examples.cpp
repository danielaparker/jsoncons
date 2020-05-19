// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json_cursor.hpp>
#include <jsoncons/json.hpp> // json_decoder and json
#include <string>
#include <sstream>

using namespace jsoncons;

namespace {
namespace ns {

    struct book
    {
        std::string author;
        std::string title;
        std::string isbn;
        std::string publisher;
        std::string date;
        double price;
    };

} // namespace ns
} // namespace

JSONCONS_ALL_MEMBER_TRAITS(ns::book,author,title,isbn,publisher,date,price);

namespace {

    // Example JSON text
    const std::string example = R"(
    [ 
      { 
          "author" : "Haruki Murakami",
          "title" : "Hard-Boiled Wonderland and the End of the World",
          "isbn" : "0679743464",
          "publisher" : "Vintage",
          "date" : "1993-03-02",
          "price": 18.90
      },
      { 
          "author" : "Graham Greene",
          "title" : "The Comedians",
          "isbn" : "0099478374",
          "publisher" : "Vintage Classics",
          "date" : "2005-09-21",
          "price": 15.74
      }
    ]
    )";

    // In the example, the application pulls the next event in the 
    // JSON input stream by calling next().

    void reading_a_json_stream()
    {
        std::istringstream is(example);

        json_cursor cursor(is);

        for (; !cursor.done(); cursor.next())
        {
            const auto& event = cursor.current();
            switch (event.event_type())
            {
                case staj_event_type::begin_array:
                    std::cout << event.event_type() << " " << "\n";
                    break;
                case staj_event_type::end_array:
                    std::cout << event.event_type() << " " << "\n";
                    break;
                case staj_event_type::begin_object:
                    std::cout << event.event_type() << " " << "\n";
                    break;
                case staj_event_type::end_object:
                    std::cout << event.event_type() << " " << "\n";
                    break;
                case staj_event_type::key:
                    // Or std::string_view, if supported
                    std::cout << event.event_type() << ": " << event.get<jsoncons::string_view>() << "\n";
                    break;
                case staj_event_type::string_value:
                    // Or std::string_view, if supported
                    std::cout << event.event_type() << ": " << event.get<jsoncons::string_view>() << "\n";
                    break;
                case staj_event_type::null_value:
                    std::cout << event.event_type() << "\n";
                    break;
                case staj_event_type::bool_value:
                    std::cout << event.event_type() << ": " << std::boolalpha << event.get<bool>() << "\n";
                    break;
                case staj_event_type::int64_value:
                    std::cout << event.event_type() << ": " << event.get<int64_t>() << "\n";
                    break;
                case staj_event_type::uint64_value:
                    std::cout << event.event_type() << ": " << event.get<uint64_t>() << "\n";
                    break;
                case staj_event_type::double_value:
                    std::cout << event.event_type() << ": " << event.get<double>() << "\n";
                    break;
                default:
                    std::cout << "Unhandled event type: " << event.event_type() << " " << "\n";
                    break;
            }
        }
    }

    // Filtering the stream
    void filtering_a_json_stream()
    {
        bool author_next = false;
        auto pred = [&](const staj_event& event, const ser_context&) -> bool
        {
            if (event.event_type() == staj_event_type::key &&
                event.get<jsoncons::string_view>() == "author")
            {
                author_next = true;
                return false;
            }
            if (author_next)
            {
                author_next = false;
                return true;
            }
            return false;
        };

        json_cursor cursor(example);
        auto filtered_c = cursor | pred;

        for (; !filtered_c.done(); filtered_c.next())
        {
            const auto& event = filtered_c.current();
            switch (event.event_type())
            {
                case staj_event_type::string_value:
                    std::cout << event.get<jsoncons::string_view>() << "\n";
                    break;
                default:
                    std::cout << "Unhandled event type: " << event.event_type() << " " << "\n";
                    break;
            }
        }
    }

    void read_nested_objects_to_basic_json()
    {
        json_cursor cursor(example);

        json_decoder<json> decoder;
        for (; !cursor.done(); cursor.next())
        {
            const auto& event = cursor.current();
            switch (event.event_type())
            {
                case staj_event_type::begin_array:
                {
                    std::cout << event.event_type() << " " << "\n";
                    break;
                }
                case staj_event_type::end_array:
                {
                    std::cout << event.event_type() << " " << "\n";
                    break;
                }
                case staj_event_type::begin_object:
                {
                    std::cout << event.event_type() << " " << "\n";
                    cursor.read_to(decoder);
                    json j = decoder.get_result();
                    std::cout << pretty_print(j) << "\n";
                    break;
                }
                default:
                {
                    std::cout << "Unhandled event type: " << event.event_type() << " " << "\n";
                    break;
                }
            }
        }
    }

    void iterate_over_complete_objects1()
    {
        json_cursor cursor(example);

        auto view = staj_array<json>(cursor);
        for (const auto& j : view)
        {
            std::cout << pretty_print(j) << "\n";
        }
    }

    void iterate_over_complete_objects2()
    {
        json_cursor cursor(example);

        auto view = staj_array<ns::book>(cursor);
        for (const auto& book : view)
        {
            std::cout << book.author << ", " << book.title << "\n";
        }
    }

} // namespace

void json_cursor_examples()
{
    std::cout << "\njson_cursor examples\n\n";

    std::cout << "\n";
    filtering_a_json_stream();
    reading_a_json_stream();
    read_nested_objects_to_basic_json();
    iterate_over_complete_objects1();
    iterate_over_complete_objects2();

    std::cout << "\n";
}

