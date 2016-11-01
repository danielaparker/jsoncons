// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <string>
#include "jsoncons/json.hpp"
#include "jsoncons/json_filter.hpp"

using namespace jsoncons;

class name_fix_up_filter : public json_filter
{
public:
    std::vector<std::string> items;

    name_fix_up_filter(json_output_handler& handler)
        : json_filter(handler)
    {
    }

private:
    void do_name(const char* p, size_t length) override
    {
        member_name_ = std::string(p, length);
        if (member_name_ != "name")
        {
            this->output_handler().name(p, length);
        }
    }

    void do_string_value(const char* p, size_t length) override
    {
        if (member_name_ == "name")
        {
            std::string value(p, length);
            size_t end_first = value.find_first_of(" \t");
            size_t start_last = value.find_first_not_of(" \t", end_first);
            this->output_handler().name("first-name");
            std::string first = value.substr(0, end_first);
            this->output_handler().value(first);
            if (start_last != std::string::npos)
            {
                this->output_handler().name("last-name");
                std::string last = value.substr(start_last);
                this->output_handler().value(last);
            }
            else
            {
                items.push_back(value);
            }
        }
        else
        {
            this->output_handler().value(p, length);
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

    json j;
    is >> j;

    json_serializer serializer(os, true);
    name_fix_up_filter filter(serializer);
    j.write(filter);
}

void name_fix_up_example2()
{
    std::string in_file = "input/address-book.json";
    std::string out_file = "output/new-address-book2.json";
    std::ifstream is(in_file);
    std::ofstream os(out_file);

    json_serializer serializer(os, true);
    name_fix_up_filter filter(serializer);
    json_reader reader(is, filter);
    reader.read_next();
}

void change_member_name_example()
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

void json_filter_examples()
{
    std::cout << "\njson_filter examples\n\n";
    name_fix_up_example1();
    name_fix_up_example2();
    change_member_name_example();

    std::cout << std::endl;
}

