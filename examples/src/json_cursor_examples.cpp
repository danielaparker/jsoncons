// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json_cursor.hpp>
#include <string>
#include <sstream>

using namespace jsoncons;

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

    json_cursor reader(is);

    for (; !reader.done(); reader.next())
    {
        const auto& event = reader.current();
        switch (event.event_type())
        {
            case staj_event_type::begin_array:
                std::cout << "begin_array\n";
                break;
            case staj_event_type::end_array:
                std::cout << "end_array\n";
                break;
            case staj_event_type::begin_object:
                std::cout << "begin_object\n";
                break;
            case staj_event_type::end_object:
                std::cout << "end_object\n";
                break;
            case staj_event_type::name:
                // Or std::string_view, if supported
                std::cout << "name: " << event.get<jsoncons::string_view>() << "\n";
                break;
            case staj_event_type::string_value:
                // Or std::string_view, if supported
                std::cout << "string_value: " << event.get<jsoncons::string_view>() << "\n";
                break;
            case staj_event_type::null_value:
                std::cout << "null_value: " << "\n";
                break;
            case staj_event_type::bool_value:
                std::cout << "bool_value: " << std::boolalpha << event.get<bool>() << "\n";
                break;
            case staj_event_type::int64_value:
                std::cout << "int64_value: " << event.get<int64_t>() << "\n";
                break;
            case staj_event_type::uint64_value:
                std::cout << "uint64_value: " << event.get<uint64_t>() << "\n";
                break;
            case staj_event_type::double_value:
                std::cout << "double_value: " << event.get<double>() << "\n";
                break;
            default:
                std::cout << "Unhandled event type\n";
                break;
        }
    }
}

class author_filter : public staj_filter
{
    bool accept_next_ = false;
public:
    bool accept(const staj_event& event, const ser_context&) override
    {
        if (event.event_type()  == staj_event_type::name &&
            event.get<jsoncons::string_view>() == "author")
        {
            accept_next_ = true;
            return false;
        }
        else if (accept_next_)
        {
            accept_next_ = false;
            return true;
        }
        else
        {
            accept_next_ = false;
            return false;
        }
    }
};

// Filtering the stream
void filtering_a_json_stream()
{
    json_cursor cursor(example);

    author_filter filter;
    filtered_staj_reader reader(cursor, filter);

    for (; !reader.done(); reader.next())
    {
        const auto& event = reader.current();
        switch (event.event_type())
        {
            case staj_event_type::string_value:
                std::cout << event.get<jsoncons::string_view>() << "\n";
                break;
        }
    }
}

void json_cursor_examples()
{
    std::cout << "\njson_cursor examples\n\n";

    std::cout << "\n";
    filtering_a_json_stream();
    reading_a_json_stream();

    std::cout << "\n";
}

