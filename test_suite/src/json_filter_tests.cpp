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

using std::string;

using jsoncons::parsing_context;
using jsoncons::json_serializer;
using jsoncons::json_reader;
using jsoncons::json_filter;
using jsoncons::json_output_handler;

class my_json_filter : public json_filter
{
public:
    my_json_filter(json_output_handler& handler)
       : json_filter(handler)
    {
    }

private:
    void do_name(const char* p, size_t length, const parsing_context& context) override
    {
        property_name_ = string(p,length);
        if (property_name_ != "name")
        {
            input_handler().name(p, length, context);
        }
    }

    void do_string_value(const char* p, size_t length, const parsing_context& context) override
    {
        if (property_name_ == "name")
        {
            string value(p,length);
            size_t end_first = value.find_first_of(" \t");
            size_t start_last = value.find_first_not_of(" \t", end_first);
            input_handler().name("first-name", context);
            std::string first = value.substr(0, end_first);
            input_handler().value(first, context); 
            if (start_last != std::string::npos)
            {
                input_handler().name("last-name", context);
                std::string last = value.substr(start_last);
                input_handler().value(last, context); 
            }
            else
            {
                std::cerr << "Incomplete name \"" << value
                   << "\" at line " << context.line_number()
                   << " and column " << context.column_number() << std::endl;
            }
        }
        else
        {
            input_handler().value(p, length, context);
        }
    }

    std::string property_name_;
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
    reader.read();
}

