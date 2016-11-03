// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <string>
#include <sstream>
#include "jsoncons/json.hpp"
#include "jsoncons/json_filter.hpp"

using namespace jsoncons;

class name_fix_up_filter : public json_filter
{
public:
    name_fix_up_filter(json_output_handler& handler)
        : json_filter(handler)
    {
    }

private:
    void do_name(const char* p, size_t length, 
                 const parsing_context& context) override
    {
        member_name_ = std::string(p, length);
        if (member_name_ != "name")
        {
            this->downstream_handler().name(p, length, context);
        }
    }

    void do_string_value(const char* p, size_t length, 
                         const parsing_context& context) override
    {
        if (member_name_ == "name")
        {
            std::string value(p, length);
            size_t end_first = value.find_first_of(" \t");
            size_t start_last = value.find_first_not_of(" \t", end_first);
            this->downstream_handler().name("first-name", context);
            std::string first = value.substr(0, end_first);
            this->downstream_handler().value(first, context);
            if (start_last != std::string::npos)
            {
                this->downstream_handler().name("last-name", context);
                std::string last = value.substr(start_last);
                this->downstream_handler().value(last, context);
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
            this->downstream_handler().value(p, length, context);
        }
    }

    std::string member_name_;
};

void name_fix_up_example1()
{
    std::string in_file = "input/address-book.json";
    std::string out_file = "output/new-address-book1.json";
    std::ifstream is(in_file);
    std::ofstream os(out_file);

    json_serializer serializer(os, true);
    name_fix_up_filter filter(serializer);
    json_reader reader(is, filter);
    reader.read_next();
}

void name_fix_up_example2()
{
    std::string in_file = "input/address-book.json";
    std::string out_file = "output/new-address-book2.json";
    std::ifstream is(in_file);
    std::ofstream os(out_file);

    json j;
    is >> j;

    json_serializer serializer(os, true);
    name_fix_up_filter filter(serializer);
    j.write(filter);
}

void change_member_name_example()
{
    std::string s = R"({"first":1,"second":2,"fourth":3,"fifth":4})";    

    json_serializer serializer(std::cout);

    // Filters can be chained
    rename_name_filter filter2("fifth", "fourth", serializer);
    rename_name_filter filter1("fourth", "third", filter2);

    // A filter can be passed to any function that takes
    // a json_input_handler ...
    std::cout << "(1) ";
    std::istringstream is(s);
    json_reader reader(is, filter1);
    reader.read();
    std::cout << std::endl;

    // or a json_output_handler    
    std::cout << "(2) ";
    ojson j = ojson::parse(s);
    j.write(filter1);
    std::cout << std::endl;
}

void json_filter_examples()
{
    std::cout << "\njson_filter examples\n\n";
    name_fix_up_example1();
    name_fix_up_example2();
    change_member_name_example();

    std::cout << std::endl;
}

