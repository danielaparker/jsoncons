// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json_stream_reader.hpp>
#include <string>
#include <sstream>

using namespace jsoncons;

void json_stream_reader_example()
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
            "mark" : 95              
        },
        {
            "enrollmentNo" : 102,
            "firstName" : "William",
            "lastName" : "Skeleton",
            "mark" : 60              
        }
    ]
    )";

    std::istringstream is(s);

    json_stream_reader reader(is);

    for (; !reader.done(); reader.next())
    {
        const auto& event = reader.current();
        switch (event.event_type())
        {
            case stream_event_type::name:
                // Returned data is string, so can use as<jsoncons::string_view>()>()
                std::cout << event.as<jsoncons::string_view>() << ": ";
                break;
            case stream_event_type::string_value:
                // Can use as<std::string_view>() if your compiler supports it
                std::cout << event.as<jsoncons::string_view>() << "\n";
                break;
            case stream_event_type::int64_value:
            case stream_event_type::uint64_value:
                // Converts integer value to std::string
                std::cout << event.as<std::string>() << "\n";
                break;
        }
    }
}

class first_name_filter : public stream_filter
{
    bool accept_next_ = false;
public:
    bool accept(const stream_event& event) override
    {
        if (event.event_type()  == stream_event_type::name &&
            event.as<jsoncons::string_view>() == "firstName")
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

void stream_filter_example()
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
            "mark" : 95              
        },
        {
            "enrollmentNo" : 102,
            "firstName" : "William",
            "lastName" : "Skeleton",
            "mark" : 60              
        }
    ]
    )";

    std::istringstream is(s);

    first_name_filter filter;
    json_stream_reader reader(is, filter);

    for (; !reader.done(); reader.next())
    {
        const auto& event = reader.current();
        switch (event.event_type())
        {
            case stream_event_type::string_value:
                std::cout << event.as<jsoncons::string_view>() << "\n";
                break;
        }
    }
}

void pull_parser_examples()
{
    std::cout << "\nPull parser examples\n\n";

    json_stream_reader_example();
    std::cout << "\n";
    stream_filter_example();

    std::cout << "\n";
}

