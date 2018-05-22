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
#include <jsoncons/json_serializer.hpp>
#include <jsoncons/json_filter.hpp>
#include <jsoncons/json_reader.hpp>
#include <jsoncons/json.hpp>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(json_filter_tests)

struct warning
{
    std::string name;
    size_t line_number;
    size_t column_number;
};

class name_fix_up_filter : public json_filter
{
public:
    std::vector<warning> warnings;

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
            this->downstream_handler().name(name,context);
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
                warnings.push_back(warning{std::string(s),
                                   context.line_number(),
                                   context.column_number()});
            }
        }
        else
        {
            this->downstream_handler().string_value(s,context);
        }
    }

    std::string member_name_;
};

BOOST_AUTO_TEST_CASE(test_filter)
{
    std::string in_file = "./input/address-book.json";
    std::string out_file = "./output/address-book-new.json";
    std::ifstream is(in_file, std::ofstream::binary);
    std::ofstream os(out_file);

    json_serializer serializer(os, jsoncons::indenting::indent);
    name_fix_up_filter filter(serializer);
    json_reader reader(is, filter);
    reader.read_next();

    BOOST_CHECK_EQUAL(1,filter.warnings.size());
    BOOST_CHECK_EQUAL("John", filter.warnings[0].name);
    BOOST_CHECK_EQUAL(9, filter.warnings[0].line_number);
    BOOST_CHECK_EQUAL(26, filter.warnings[0].column_number);
}

BOOST_AUTO_TEST_CASE(test_filter2)
{
    std::string in_file = "./input/address-book.json";
    std::string out_file = "./output/address-book-new.json";
    std::ifstream is(in_file, std::ofstream::binary);
    std::ofstream os(out_file);

    json_serializer serializer(os, jsoncons::indenting::indent);

    name_fix_up_filter filter2(serializer);

    rename_object_member_filter filter1("email","email2",filter2);

    json_reader reader(is, filter1);
    reader.read_next();

    BOOST_CHECK_EQUAL(1,filter2.warnings.size());
    BOOST_CHECK_EQUAL("John", filter2.warnings[0].name);
    BOOST_CHECK_EQUAL(9, filter2.warnings[0].line_number);
    BOOST_CHECK_EQUAL(26, filter2.warnings[0].column_number);
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
    catch (const parse_error& e)
    {
        std::cout << e.what() << std::endl;
    }
    BOOST_CHECK_CLOSE(31.96,j["store"]["book"][0]["price"].as<double>(),0.001);

    std::stringstream ss;
    json_serializer serializer(ss);
    rename_object_member_filter filter("price","price2",serializer);
    j.dump(filter);

    json j2 = json::parse(ss);
    BOOST_CHECK_CLOSE(31.96,j2["store"]["book"][0]["price2"].as<double>(),0.001);
}

BOOST_AUTO_TEST_CASE(test_chained_filters)
{
    ojson j = ojson::parse(R"({"first":1,"second":2,"fourth":3,"fifth":4})");

    json_decoder<ojson> decoder;

    rename_object_member_filter filter2("fifth", "fourth", decoder);
    rename_object_member_filter filter1("fourth", "third", filter2);

    j.dump(filter1);
    ojson j2 = decoder.get_result();
    BOOST_CHECK(j2.size() == 4);
    BOOST_CHECK(j2["first"] == 1);
    BOOST_CHECK(j2["second"] == 2);
    BOOST_CHECK(j2["third"] == 3);
    BOOST_CHECK(j2["fourth"] == 4);
}

BOOST_AUTO_TEST_SUITE_END()
