// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include "jsoncons/json.hpp"
#include "jsoncons/json_serializer.hpp"
#include "jsoncons/json_filter.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <new>
#include <boost/optional.hpp>

using jsoncons::parsing_context;
using jsoncons::json_serializer;
using jsoncons::output_format;
using jsoncons::json;
using jsoncons::json_reader;
using jsoncons::json_input_handler;
using jsoncons::json_output_handler;
using std::string;
using jsoncons::json_filter;
using jsoncons::pretty_print;

BOOST_AUTO_TEST_CASE(test_boost_optional)
{
    boost::optional<jsoncons::json> opt_json;
    opt_json = jsoncons::json(jsoncons::json::an_object);
}

BOOST_AUTO_TEST_CASE(test_for_each_value)
{
    string input = "{\"A\":\"Jane\", \"B\":\"Roe\",\"C\":10}";
    json val = json::parse_string(input);

    json::object_iterator it = val.begin_members();

    BOOST_CHECK(it->value().type() == jsoncons::value_type::string_t);
    ++it;
    BOOST_CHECK(it->value().type() == jsoncons::value_type::string_t);
    ++it;
    BOOST_CHECK(it->value().type() == jsoncons::value_type::ulonglong_t);
    ++it;
    BOOST_CHECK(it == val.end_members());
}

BOOST_AUTO_TEST_CASE(test_assignment)
{
    json root;

    root["double_1"] = 10.0;

    json double_1 = root["double_1"];

    BOOST_CHECK_CLOSE(double_1.as<double>(), 10.0, 0.000001);

    root["myobject"] = json();
    root["myobject"]["double_2"] = 7.0;
    root["myobject"]["bool_2"] = true;
    root["myobject"]["int_2"] = 0LL;
    root["myobject"]["string_2"] = "my string";
    root["myarray"] = json::an_array;

    json double_2 = root["myobject"]["double_2"];

    BOOST_CHECK_CLOSE(double_2.as<double>(), 7.0, 0.000001);
    BOOST_CHECK(double_2.as<int>() == 7);
    BOOST_CHECK(root["myobject"]["bool_2"].as<bool>());
    BOOST_CHECK(root["myobject"]["int_2"].as_longlong() == 0);
    BOOST_CHECK(root["myobject"]["string_2"].as<std::string>() == std::string("my string"));

    BOOST_CHECK(root["myobject"]["bool_2"].as<bool>());
    BOOST_CHECK(root["myobject"]["int_2"].as<long long>() == 0);
    BOOST_CHECK(root["myobject"]["string_2"].as<std::string>() == std::string("my string"));

    //std::cout << root << std::endl;

    //json double_2_value = root["double_2"];

    //BOOST_CHECK(double_2_value.is_null());
}

BOOST_AUTO_TEST_CASE(test_array)
{
    json root;

    root["addresses"];

    std::vector<json> addresses;
    json address1;
    address1["city"] = "San Francisco";
    address1["state"] = "CA";
    address1["zip"] = "94107";
    address1["country"] = "USA";
    addresses.push_back(address1);

    json address2;
    address2["city"] = "Sunnyvale";
    address2["state"] = "CA";
    address2["zip"] = "94085";
    address2["country"] = "USA";
    addresses.push_back(address2);

    root["addresses"] = json(addresses.begin(), addresses.end());

    BOOST_CHECK(root["addresses"].size() == 2);

}

BOOST_AUTO_TEST_CASE(example)
{
    std::cout << "EXAMPLE" << std::endl;
    std::string in = "{\"getValuesReturn\" : {\"return\" : \"true\",\"TextTag\" : \"Text!\",\"String\" : [\"First item\",\"Second item\",\"Third item\"],\"TagWithAttrsAndText\" : {\"content\" : \"Text!\",\"attr3\" : \"value3\",\"attr2\" : \"value2\",\"attr1\" : \"value1\"},\"EmptyTag\" : true,\"attribute\" : {\"attrValue\" : \"value\"},\"TagWithAttrs\" : {\"attr3\" : \"value3\",\"attr2\" : \"value2\",\"attr1\" : \"value1\"}}}";

    std::cout << in << std::endl;
    std::istringstream is(in);

    json root = json::parse(is);

    std::cout << root << std::endl;
}

BOOST_AUTO_TEST_CASE(test_null)
{
    json nullval = json::null;
    BOOST_CHECK(nullval.is_null());
    BOOST_CHECK(nullval.is<json::null_type>());

    json obj;
    obj["field"] = json::null;
    std::cout << obj << std::endl;
}

BOOST_AUTO_TEST_CASE(test_to_string)
{
    std::ostringstream os;
    os << "{"
       << "\"string\":\"value\""
       << ",\"null\":null"
       << ",\"bool1\":false"
       << ",\"bool2\":true"
       << ",\"integer\":12345678"
       << ",\"neg-integer\":-87654321"
       << ",\"double\":123456.01"
       << ",\"neg-double\":-654321.01"
       << ",\"exp\":2.00600e+03"
       << ",\"minus-exp\":1.00600e-010"
       << ",\"escaped-string\":\"\\\\\\n\""
       << "}";
    std::cout << "test_to_string" << std::endl;
    std::cout << os.str() << std::endl;


    json root = json::parse_string(os.str());
    std::cout << root << std::endl;

    BOOST_CHECK(root["null"].is_null());
    BOOST_CHECK(root["null"].is<json::null_type>());
    BOOST_CHECK(!root["bool1"].as<bool>());
    BOOST_CHECK(root["bool2"].as<bool>());
    BOOST_CHECK(root["integer"].as<int>() == 12345678);
    BOOST_CHECK(root["integer"].as<unsigned int>() == 12345678);
    BOOST_CHECK(root["neg-integer"].as<int>() == -87654321);
    BOOST_CHECK_CLOSE(root["double"].as<double>(), 123456.01, 0.0000001);
    BOOST_CHECK(root["escaped-string"].as<std::string>() == std::string("\\\n"));

    BOOST_CHECK(!root["bool1"].as<bool>());
    BOOST_CHECK(root["bool2"].as<bool>());
    BOOST_CHECK(root["integer"].as<int>() == 12345678);
    BOOST_CHECK(root["integer"].as<unsigned int>() == 12345678);
    BOOST_CHECK(root["neg-integer"].as<int>() == -87654321);
    BOOST_CHECK_CLOSE(root["double"].as<double>(), 123456.01, 0.0000001);
    BOOST_CHECK(root["escaped-string"].as<std::string>() == std::string("\\\n"));
}

BOOST_AUTO_TEST_CASE(test_serialize)
{
    std::string input = "{\"city\":\"Toronto\", \"number\":100.5}";

    json o = json::parse_string(input);

    std::ostringstream os;

    json_serializer serializer(os, true);
    o.to_stream(serializer);
    std::cout << os.str() << std::endl;
}

BOOST_AUTO_TEST_CASE(test_array2)
{
    std::vector<int> v;
    v.push_back(100);
    v.push_back(200);
    v.push_back(300);

    json a(v.begin(), v.end());
    a.add(400);

    std::cout << a << std::endl;

}

BOOST_AUTO_TEST_CASE(test_nan_replacement)
{
    json obj;
    obj["field1"] = std::sqrt(-1.0);
    obj["field2"] = 1.79e308 * 1000;
    obj["field3"] = -1.79e308 * 1000;
    std::cout << obj << std::endl;
}

BOOST_AUTO_TEST_CASE(test_custom_nan_replacement)
{
    json obj;
    obj["field1"] = std::sqrt(-1.0);
    obj["field2"] = 1.79e308 * 1000;
    obj["field3"] = -1.79e308 * 1000;

    output_format format;
    format.nan_replacement("null");
    format.pos_inf_replacement("1e9999");
    format.neg_inf_replacement("-1e9999");

    obj.to_stream(std::cout, format);
    std::cout << std::endl;
}

BOOST_AUTO_TEST_CASE(test_no_nan_replacement)
{
    json obj;
    obj["field1"] = std::sqrt(-1.0);
    obj["field2"] = 1.79e308 * 1000;
    obj["field3"] = -1.79e308 * 1000;

    output_format format;
    format.replace_nan(false);
    format.replace_inf(false);

    obj.to_stream(std::cout, format);
    std::cout << std::endl;
}

BOOST_AUTO_TEST_CASE(test_object_iterator)
{
    json obj;
    obj["city"] = "Toronto";
    obj["province"] = "Ontario";
    obj["country"] = "Canada";

    for (auto it = obj.begin_members(); it != obj.end_members(); ++it){
        std::cout << it->name() << "=" << it->value().as<std::string>() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_array_iterator)
{
    json arr = json::an_array;
    arr.add("Toronto");
    arr.add("Vancouver");
    arr.add("Montreal");

    for (auto it = arr.begin_elements(); it != arr.end_elements(); ++it){
        std::cout << it->as<std::string>() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_u0000)
{
    string inputStr("[\"\\u0040\\u0040\\u0000\\u0011\"]");
    std::cout << "Input:    " << inputStr << std::endl;
    json arr = json::parse_string(inputStr);

    std::string str = arr[0].as<std::string>();
    std::cout << "Hex dump: [";
    for (size_t i = 0; i < str.size(); ++i)
    {
        unsigned int val = static_cast<unsigned int>(str[i]);
        if (i != 0)
        {
            std::cout << " ";
        }
        std::cout << "0x" << std::setfill('0') << std::setw(2) << std::hex << val;
    }
    std::cout << "]" << std::endl;

    std::ostringstream os;
    os << arr;
    std::cout << "Output:   " << os.str() << std::endl;

}

BOOST_AUTO_TEST_CASE(parse_file)
{
    json obj = json::parse_file("input/persons.json");
    std::cout << obj << std::endl;
}

BOOST_AUTO_TEST_CASE(test_uHHHH)
{
    string inputStr("[\"\\u007F\\u07FF\\u0800\"]");
    std::cout << "Input:    " << inputStr << std::endl;
    json arr = json::parse_string(inputStr);

    std::string s = arr[0].as<std::string>();
    std::cout << "Hex dump: [";
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (i != 0)
            std::cout << " ";
        unsigned int u(s[i] >= 0 ? s[i] : 256 + s[i]);
        std::cout << "0x"  << std::hex << std::setfill('0') << std::setw(2) << u;
    }
    std::cout << "]" << std::endl;

    std::ostringstream os;
    output_format format;
    format.escape_all_non_ascii(true);
    arr.to_stream(os, format);
    std::string outputStr = os.str();
    std::cout << "Output:   " << os.str() << std::endl;

    json arr2 = json::parse_string(outputStr);
    std::string s2 = arr2[0].as<std::string>();
    std::cout << "Hex dump: [";
    for (size_t i = 0; i < s2.size(); ++i)
    {
        if (i != 0)
            std::cout << " ";
        unsigned int u(s2[i] >= 0 ? s2[i] : 256 + s2[i]);
        std::cout << "0x"  << std::hex << std::setfill('0') << std::setw(2) << u;
    }
    std::cout << "]" << std::endl;

}

BOOST_AUTO_TEST_CASE(constructing_structures)
{
    json root;

    root["persons"] = json::an_array;

    json person;
    person["first_name"] = "John";
    person["last_name"] = "Smith";
    person["birth_date"] = "1972-01-30";
    json address;
    address["city"] = "Toronto";
    address["country"] = "Canada";
    person["address"] = std::move(address);

    root["persons"].add(std::move(person));

    std::cout << pretty_print(root) << std::endl;
}

BOOST_AUTO_TEST_CASE(test_defaults)
{
    json obj;

    obj["field1"] = 1;
    obj["field3"] = "Toronto";

    double x1 = obj.has_member("field1") ? obj["field1"].as<double>() : 10.0;
    double x2 = obj.has_member("field2") ? obj["field2"].as<double>() : 20.0;

    std::cout << "x1=" << x1 << std::endl;
    std::cout << "x2=" << x2 << std::endl;

    std::string x3 = obj.get("field3", "Montreal").as<std::string>();
    std::string x4 = obj.get("field4", "San Francisco").as<std::string>();

    std::cout << "x3=" << x3 << std::endl;
    std::cout << "x4=" << x4 << std::endl;
}

BOOST_AUTO_TEST_CASE(test_exception)
{
    try
    {
        std::string input("{\"field1\":\n\"value}");
        std::cout << input << std::endl;
        json obj = json::parse_string(input);
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_big_file)
{
    std::ofstream os("output/test.json", std::ofstream::binary);

    std::string person("person");
    std::string first_name("first_name");
    std::string last_name("last_name");
    std::string birthdate("birthdate");
    std::string sex("sex");
    std::string salary("salary");
    std::string interests("interests");
    std::string favorites("favorites");
    std::string color("color");
    std::string sport("sport");
    std::string food("food");

    std::string john_first_name("john");
    std::string john_last_name("doe");
    std::string john_birthdate("1998-05-13");
    std::string john_sex("m");
    std::string reading("Reading");
    std::string mountain_biking("Mountain biking");
    std::string hacking("Hacking");
    std::string john_color("blue");
    std::string john_sport("soccer");
    std::string john_food("spaghetti");

    output_format format;
    json_serializer handler(os, format, true);

    std::clock_t t = std::clock();

    handler.begin_array();
    for (size_t i = 0; i < 100; ++i)
    {
        handler.begin_object();
        handler.name(person);
        handler.begin_object();
        handler.name(first_name);
        handler.value(john_first_name);
        handler.name(last_name);
        handler.value(john_last_name);
        handler.name(birthdate);
        handler.value(john_birthdate);
        handler.name(sex);
        handler.value(john_sex);
        handler.name(salary);
        handler.value(70000);
        handler.name(interests);
        handler.begin_array();
        handler.value(reading);
        handler.value(mountain_biking);
        handler.value(hacking);
        handler.end_array();
        handler.name(favorites);
        handler.begin_object();
        handler.name(color);
        handler.value(john_color);
        handler.name(sport);
        handler.value(john_sport);
        handler.name(food);
        handler.value(john_food);
        handler.end_object();

        handler.end_object();
        handler.end_object();
    }
    handler.end_array();
    os.flush();
    std::clock_t s = std::clock() - t;
    std::cout << "It took " << (((double)s) / CLOCKS_PER_SEC) << " seconds to write.\n";

    std::ifstream is("output/test.json", std::ofstream::binary);
    t = std::clock();

    json root = json::parse(is);
    s = std::clock() - t;
    std::cout << "It took " << (((double)s) / CLOCKS_PER_SEC) << " seconds.\n";

}

BOOST_AUTO_TEST_CASE(test_multiline_comments)
{
    json obj = json::parse_file("input/json-multiline-comment.json");
    BOOST_CHECK(obj.is_array());
    BOOST_CHECK(obj.is<json::array>());
    BOOST_CHECK_EQUAL(obj.size(), 0);
}

