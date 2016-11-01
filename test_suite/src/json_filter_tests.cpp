// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <new>
#include "jsoncons/json_serializer.hpp"
#include "jsoncons/json_filter.hpp"
#include "jsoncons/json_reader.hpp"
#include "jsoncons/json.hpp"

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(json_filter_test_suite)

struct item
{
    item(std::string n, size_t l, size_t c)
        : name(n),line(l),column(c)
    {
    }
    std::string name;
    size_t line;
    size_t column;
};

class my_json_filter : public json_filter
{
public:
    std::vector<std::string> items;

    my_json_filter(json_output_handler& handler)
        : json_filter(handler)
    {
    }

private:
    void do_name(const char* p, size_t length) override
    {
        member_name_ = std::string(p, length);
        if (member_name_ != "name")
        {
            parent_handler().name(p, length);
        }
    }

    void do_string_value(const char* p, size_t length) override
    {
        if (member_name_ == "name")
        {
            std::string value(p, length);
            size_t end_first = value.find_first_of(" \t");
            size_t start_last = value.find_first_not_of(" \t", end_first);
            parent_handler().name("first-name");
            std::string first = value.substr(0, end_first);
            parent_handler().value(first);
            if (start_last != std::string::npos)
            {
                parent_handler().name("last-name");
                std::string last = value.substr(start_last);
                parent_handler().value(last);
            }
            else
            {
                items.push_back(value);
            }
        }
        else
        {
            parent_handler().value(p, length);
        }
    }

    std::string member_name_;
};

BOOST_AUTO_TEST_CASE(test_filter)
{
    std::string in_file = "input/address-book.json";
    std::string out_file = "output/address-book-new.json";
    std::ifstream is(in_file, std::ofstream::binary);
    std::ofstream os(out_file);

    json_serializer serializer(os, true);
    my_json_filter filter(serializer);
    json_reader reader(is, filter);
    reader.read_next();

    BOOST_CHECK_EQUAL(1,filter.items.size());
    BOOST_CHECK_EQUAL("John", filter.items[0]);
}

class my_parsing_context : public parsing_context
{
    size_t do_line_number() const override
    {
        return 1;
    }

    size_t do_column_number() const override
    {
        return 1;
    }

    char do_current_char() const override
    {
        return '0';
    }
};

BOOST_AUTO_TEST_CASE(test_output_input_adapter)
{
    std::string input = "\"String\"";
    std::istringstream is(input);

    json_deserializer handler;
    basic_json_output_input_adapter<char> adapter(handler);
    try
    {
        json_reader reader(is,handler);
        reader.read_next();
    }
    catch (const std::exception&)
    {
    }
}

BOOST_AUTO_TEST_CASE(test_rename_name)
{
    json j;
    try
    {
        j = json::parse(R"(
{"store":
{"book": [
{"category": "reference",
"author": "Margaret Weis",
"title": "Dragonlance Series",
"price": 31.96}, {"category": "reference",
"author": "Brent Weeks",
"title": "Night Angel Trilogy",
"price": 14.70
}]}}
)");
    }
    catch (const parse_exception& e)
    {
        std::cout << e.what() << std::endl;
    }

    std::cout << ("1\n") << pretty_print(j) << std::endl;

    json_serializer serializer(std::cout, true);

    rename_name_filter filter("price","price2",serializer);
    j.write(filter);

}

BOOST_AUTO_TEST_SUITE_END()
