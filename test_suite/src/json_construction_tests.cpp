// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include <boost/numeric/ublas/io.hpp>
#include "jsoncons/json.hpp"
#include "jsoncons/json_serializer.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using jsoncons::json_serializer;
using jsoncons::output_format;
using jsoncons::json;
using jsoncons::pretty_print;
using jsoncons::wjson;
using jsoncons::json_reader;
using jsoncons::json_exception;
using std::string;

BOOST_AUTO_TEST_CASE(test_construction_from_string)
{
    std::string input = "{\"first_name\":\"Jane\",\"last_name\":\"Roe\",\"events_attended\":10}";

    json val = json::parse_string(input);

    std::cout << val << std::endl;
}

BOOST_AUTO_TEST_CASE(test_construction_from_file)
{
    json val = json::parse_file("input/members.json");

    std::cout << pretty_print(val) << std::endl;
}

BOOST_AUTO_TEST_CASE(test_construction_in_code)
{
    // A null value
    json null_val;

    // A boolean value
    json flag(true);

    // A numeric value
    json number(10.5);

    // An object value with four members
    json obj;
    obj["first_name"] = "Jane";
    obj["last_name"] = "Roe";
    obj["events_attended"] = 10;
    obj["accept_waiver_of_liability"] = true;

    // An array value with four elements
    json arr(json::an_array);
    arr.add(null_val);
    arr.add(flag);
    arr.add(number);
    arr.add(obj);

    output_format format;
    arr.to_stream(std::cout,format,true);
    std::cout << std::endl;
}

BOOST_AUTO_TEST_CASE(test_from_container)
{
    std::vector<int> vec;
    vec.push_back(10);
    vec.push_back(20);
    vec.push_back(30);

    json val1(vec.begin(), vec.end());
    std::cout << val1 << std::endl;

    std::list<double> list;
    list.push_back(10.5);
    list.push_back(20.5);
    list.push_back(30.5);

    json val2(list.begin(), list.end());
    std::cout << val2 << std::endl;
}

BOOST_AUTO_TEST_CASE(test_accessing)
{
    json obj;
    obj["first_name"] = "Jane";
    obj["last_name"] = "Roe";
    obj["events_attended"] = 10;
    obj["accept_waiver_of_liability"] = true;

    std::string first_name = obj["first_name"].as<std::string>();
    std::string last_name = obj.at("last_name").as<std::string>();
    int events_attended = obj["events_attended"].as<int>();
    bool accept_waiver_of_liability = obj["accept_waiver_of_liability"].as<bool>();

    std::cout << first_name << " " << last_name << ", " << events_attended << ", " << accept_waiver_of_liability << std::endl;

}

BOOST_AUTO_TEST_CASE(test_value_not_found_and_defaults)
{
    json obj;
    obj["first_name"] = "Jane";
    obj["last_name"] = "Roe";

    try
    {
        std::string experience = obj["outdoor_experience"].as<std::string>();
    }
    catch (const json_exception& e)
    {
        std::cout << e.what() << std::endl;
    }

    std::string experience = obj.has_member("outdoor_experience") ? obj["outdoor_experience"].as<std::string>() : "";

    bool first_aid_certification = obj.get("first_aid_certification",false).as<bool>();

    std::cout << "experience=" << experience << ", first_aid_certification=" << first_aid_certification << std::endl;
}

BOOST_AUTO_TEST_CASE(test_another_object_iterator)
{
    json obj;
    obj["first_name"] = "Jane";
    obj["last_name"] = "Roe";
    obj["events_attended"] = 10;
    obj["accept_waiver_of_liability"] = true;

    for (auto it = obj.begin_members(); it != obj.end_members(); ++it)
    {
        std::cout << "name=" << it->name() << ", value=" << it->value().as<std::string>() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_another_array_iterator)
{
    json arr(json::an_array);
    arr.add("Montreal");
    arr.add("Toronto");
    arr.add("Ottawa");
    arr.add("Vancouver");

    for (auto it = arr.begin_elements(); it != arr.end_elements(); ++it)
    {
        std::cout << it->as<std::string>() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_integer_limits)
{
    const long long max_value = std::numeric_limits<long long>::max();

    const unsigned long long max_uvalue = std::numeric_limits<unsigned long long>::max();
    {
        std::ostringstream os;

        //os << max_value;

        os << "{\"max_longlong\":-" << max_value << "}";
        json val = json::parse_string(os.str());
        std::cout << val << std::endl;
        BOOST_CHECK(val["max_longlong"].is_longlong());
    }
    {
        std::ostringstream os;

        //os << max_value;

        std::cout << "TEST LIMITS" << std::endl;
        os << "{\"max_longlong_overflow\":-" << max_value << "0}";
        std::cout << os.str() << std::endl;


        json val = json::parse_string(os.str());
        std::cout << val << std::endl;
        BOOST_CHECK(val["max_longlong_overflow"].is_double());
    }
    {
        std::ostringstream os;

        os << "{\"max_ulonglong\":" << max_uvalue << "}";
        json val = json::parse_string(os.str());
        std::cout << val << std::endl;
        BOOST_CHECK(val["max_ulonglong"].is_ulonglong());
    }
    {
        std::ostringstream os;

        os << "{\"max_ulonglong_overflow\":" << max_uvalue << "0}";
        json val = json::parse_string(os.str());
        std::cout << val << std::endl;
        BOOST_CHECK(val["max_ulonglong_overflow"].is_double());
    }

    std::cout << "size json=" << sizeof(json) << std::endl;
    std::cout << "size string=" << sizeof(string) << std::endl;
    std::cout << "size array=" << sizeof(std::vector<json>) << std::endl;
    std::cout << "size map=" << sizeof(std::vector<json::member_type>) << std::endl;
}

BOOST_AUTO_TEST_CASE(test_multiple)
{
    std::string in="{\"a\":1,\"b\":2,\"c\":3}{\"a\":4,\"b\":5,\"c\":6}";
    std::cout << in << std::endl;

    std::istringstream is(in);

    jsoncons::json_deserializer handler;
    json_reader reader(is,handler);

    while (!reader.eof())
    {
        reader.read();
        if (!reader.eof())
        {
            json val = std::move(handler.root());
            std::cout << val << std::endl;
        }
    }
}

