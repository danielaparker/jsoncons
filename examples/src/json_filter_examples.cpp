// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <string>
#include <sstream>
#include <jsoncons/json.hpp>
#include <jsoncons/json_filter.hpp>

using namespace jsoncons;

class name_fix_up_filter : public json_filter
{
public:
    name_fix_up_filter(json_content_handler& handler)
        : json_filter(handler)
    {
    }

private:
    void do_name(const string_view_type& name, 
                 const serializing_context& context) override
    {
        member_name_ = std::string(name);
        if (member_name_ != "name")
        {
            this->downstream_handler().name(name, context);
        }
    }

    void do_string_value(const string_view_type& s, 
                         const serializing_context& context) override
    {
        if (member_name_ == "name")
        {
            size_t end_first = s.find_first_of(" \t");
            size_t start_last = s.find_first_not_of(" \t", end_first);
            this->downstream_handler().name("first-name", context);
            string_view_type first = s.substr(0, end_first);
            this->downstream_handler().string_value(first, context);
            if (start_last != string_view_type::npos)
            {
                this->downstream_handler().name("last-name", context);
                string_view_type last = s.substr(start_last);
                this->downstream_handler().string_value(last, context);
            }
            else
            {
                std::cerr << "Incomplete name \"" << s
                   << "\" at line " << context.line_number()
                   << " and column " << context.column_number() << std::endl;
            }
        }
        else
        {
            this->downstream_handler().string_value(s, context);
        }
    }

    std::string member_name_;
};

void name_fix_up_example1()
{
    std::string in_file = "./input/address-book.json";
    std::string out_file = "./output/new-address-book1.json";
    std::ifstream is(in_file);
    std::ofstream os(out_file);

    json_serializer serializer(os, jsoncons::indenting::indent);
    name_fix_up_filter filter(serializer);
    json_reader reader(is, filter);
    reader.read_next();
}

void name_fix_up_example2()
{
    std::string in_file = "./input/address-book.json";
    std::string out_file = "./output/new-address-book2.json";
    std::ifstream is(in_file);
    std::ofstream os(out_file);

    json j;
    is >> j;

    json_serializer serializer(os, jsoncons::indenting::indent);
    name_fix_up_filter filter(serializer);
    j.dump(filter);
}

void change_member_name_example()
{
    std::string s = R"({"first":1,"second":2,"fourth":3,"fifth":4})";    

    json_serializer serializer(std::cout);

    // Filters can be chained
    rename_object_member_filter filter2("fifth", "fourth", serializer);
    rename_object_member_filter filter1("fourth", "third", filter2);

    // A filter can be passed to any function that takes
    // a json_content_handler ...
    std::cout << "(1) ";
    std::istringstream is(s);
    json_reader reader(is, filter1);
    reader.read();
    std::cout << std::endl;

    // or a json_content_handler    
    std::cout << "(2) ";
    ojson j = ojson::parse(s);
    j.dump(filter1);
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

