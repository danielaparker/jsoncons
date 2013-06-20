// Copyright 2013 Daniel Parker
// Distributed under Boost license

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include "jsoncons/json.hpp"
#include "jsoncons/json_serializer.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using jsoncons::json_stream_writer;
using jsoncons::output_format;
using jsoncons::json;
using jsoncons::wjson;
using jsoncons::basic_json_parser;
using std::string;

BOOST_AUTO_TEST_CASE( test1 )
{
    std::istringstream is("{\"unicode_string_1\":\"\\uD800\\uDC00\"}");

    json root = json::parse(is);
    std::cout << "root type=" << root.type() << std::endl;

    root["double_1"] = json(10.0);

    json double_1 = root["double_1"];

    BOOST_CHECK_CLOSE(double_1.as_double(), 10.0, 0.000001);

    json copy = root;
}

BOOST_AUTO_TEST_CASE(test_assignment)
{
    json root(json::an_object);

	root["double_1"] = json(10.0);

    json double_1 = root["double_1"];

    BOOST_CHECK_CLOSE(double_1.as_double(), 10.0, 0.000001);

    root["myobject"] = json::an_object;
    root["myobject"]["double_2"] = json(7.0);
    root["myobject"]["bool_2"] = json(true);
    root["myobject"]["int_2"] = json(long long(0));
    root["myobject"]["string_2"] = json("my string");
    root["myarray"] = json::an_array;

    json double_2 = root["myobject"]["double_2"];
    json int_2 = root["myobject"]["double_2"];

    BOOST_CHECK_CLOSE(double_2.as_double(), 7.0, 0.000001);
    BOOST_CHECK_CLOSE(int_2.as_int(), 7.0, 0);

	std::cout << root << std::endl;

    //json double_2_value = root["double_2"];

    //BOOST_CHECK(double_2_value.is_null());
}

BOOST_AUTO_TEST_CASE(test_array)
{
    json root(json::an_object);

    root["addresses"];

    std::vector<json> addresses;
    json address1(json::an_object);
    address1["city"] = json("San Francisco");
    address1["state"] = json("CA");
    address1["zip"] = json("94107");
    address1["country"] = json("USA");
    addresses.push_back(address1);

    json address2(json::an_object);
    address2["city"] = json("Sunnyvale");
    address2["state"] = json("CA");
    address2["zip"] = json("94085");
    address2["country"] = json("USA");
    addresses.push_back(address2);
    
    root["addresses"] = json(addresses.begin(),addresses.end());

    std::cout << root << std::endl;

    BOOST_CHECK(root["addresses"].size() == 2);

    std::cout << "size=" << root["addresses"].size() << std::endl;
    for (size_t i = 0; i < root["addresses"].size(); ++i)
    {
        std::cout << root["addresses"][i].to_string() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(example)
{
    std::string in = "\"getValuesReturn\" : {\"return\" : \"true\",\"TextTag\" : \"Text!\",\"String\" : [\"First item\",\"Second item\",\"Third item\"],\"TagWithAttrsAndText\" : {\"content\" : \"Text!\",\"attr3\" : \"value3\",\"attr2\" : \"value2\",\"attr1\" : \"value1\"},\"EmptyTag\" : true,\"attribute\" : {\"attrValue\" : \"value\"},\"TagWithAttrs\" : {\"attr3\" : \"value3\",\"attr2\" : \"value2\",\"attr1\" : \"value1\"}}}";
    std::istringstream is(in);

    json root = json::parse(is);

    std::cout << root << std::endl;
}

BOOST_AUTO_TEST_CASE(test_null)
{
    json nullval = json::null;
    BOOST_CHECK(nullval.is_null());

    json obj(json::an_object);
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
    BOOST_CHECK(!root["bool1"].as_bool());
    BOOST_CHECK(root["bool2"].as_bool());
    BOOST_CHECK(root["integer"].as_int() == 12345678);
    BOOST_CHECK(root["integer"].as_uint() == 12345678);
    BOOST_CHECK(root["neg-integer"].as_int() == -87654321);
    BOOST_CHECK_CLOSE(root["double"].as_double(), 123456.01, 0.0000001);
    BOOST_CHECK(root["escaped-string"].as_string() == std::string("\\\n"));
}

BOOST_AUTO_TEST_CASE(test_serialize)
{
    std::string input = "{\"city\":\"Toronto\", \"number\":100.5}";

    json o = json::parse_string(input);

    std::ostringstream os;

    json_stream_writer serializer(os,output_format(true));
    o.serialize(serializer);
    std::cout << os.str() << std::endl;
}

BOOST_AUTO_TEST_CASE(test_array2)
{
    std::vector<int> v;
    v.push_back(100);
    v.push_back(200);
    v.push_back(300);

    json a(v.begin(),v.end());
    a.push_back(400);

    std::cout << a << std::endl;

}

BOOST_AUTO_TEST_CASE(test_nan_replacement)
{
    json obj(json::an_object);
    obj["field1"] = std::sqrt(-1.0);
    obj["field2"] = 1.79e308*1000;
    obj["field3"] = -1.79e308*1000;
    std::cout << obj << std::endl;
}

BOOST_AUTO_TEST_CASE(test_custom_nan_replacement)
{
    json obj(json::an_object);
    obj["field1"] = std::sqrt(-1.0);
    obj["field2"] = 1.79e308*1000;
    obj["field3"] = -1.79e308*1000;

    output_format format;
    format.nan_replacement("null");
    format.pos_inf_replacement("1e9999");
    format.neg_inf_replacement("-1e9999");

    obj.to_stream(std::cout,format);
    std::cout << std::endl;
}

BOOST_AUTO_TEST_CASE(test_no_nan_replacement)
{
    json obj(json::an_object);
    obj["field1"] = std::sqrt(-1.0);
    obj["field2"] = 1.79e308*1000;
    obj["field3"] = -1.79e308*1000;

    output_format format;
    format.replace_nan(false);
    format.replace_inf(false);

    obj.to_stream(std::cout,format);
    std::cout << std::endl;
}

BOOST_AUTO_TEST_CASE(test_object_iterator)
{
    json obj(json::an_object);
    obj["city"] = "Toronto";
    obj["province"] = "Ontario";
    obj["country"] = "Canada";

    for (auto it = obj.begin_members(); it != obj.end_members(); ++it)
    {
        std::cout << it->name() << "=" << it->value().as_string() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_array_iterator)
{
    json arr(json::an_array);
    arr.push_back("Toronto");
    arr.push_back("Vancouver");
    arr.push_back("Montreal");

    for (auto it = arr.begin_elements(); it != arr.end_elements(); ++it)
    {
        std::cout << it->as_string() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_u0000)
{
    string inputStr("[\"\\u0040\\u0040\\u0000\\u0011\"]");
    std::cout << "Input:    " << inputStr << std::endl;
    json arr = json::parse_string(inputStr);

    std::string str = arr[0].as_string();
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
    json obj = json::parse_file("../../../examples/persons.json");
    std::cout << obj << std::endl;
}

BOOST_AUTO_TEST_CASE(test_uHHHH)
{
    string inputStr("[\"\\u007F\\u07FF\\u0800\"]");
    std::cout << "Input:    " << inputStr << std::endl;
    json arr = json::parse_string(inputStr);

    std::string s = arr[0].as_string();
    std::cout << "Hex dump: [";
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (i != 0)
            std::cout << " ";
        unsigned int u(s[i] >= 0 ? s[i] : 256 + s[i] );
        std::cout << "0x"  << std::hex<< std::setfill('0') << std::setw(2) << u;
    }
    std::cout << "]" << std::endl;
    
    std::ostringstream os;
    output_format format;
    format.escape_all_non_ascii(true);
    arr.to_stream(os,format);
    std::string outputStr = os.str();
    std::cout << "Output:   " << os.str() << std::endl;

	json arr2 = json::parse_string(outputStr);
    std::string s2 = arr2[0].as_string();
    std::cout << "Hex dump: [";
    for (size_t i = 0; i < s2.size(); ++i)
    {
        if (i != 0)
            std::cout << " ";
        unsigned int u(s2[i] >= 0 ? s2[i] : 256 + s2[i] );
        std::cout << "0x"  << std::hex<< std::setfill('0') << std::setw(2) << u;
    }
    std::cout << "]" << std::endl;

}

BOOST_AUTO_TEST_CASE(constructing_structures)
{
	json root = json(json::an_object);

    root["persons"] = json(json::an_array);

    json person(json::an_object);
    person["first_name"] = "John";
    person["last_name"] = "Smith";
    person["birth_date"] = "1972-01-30";
    json address(json::an_object);
    address["city"] = "Toronto";
    address["country"] = "Canada";
    person["address"] = std::move(address);

    root["persons"].push_back(std::move(person));

    output_format format(true);
    root.to_stream(std::cout,format);
    std::cout << std::endl;
}

BOOST_AUTO_TEST_CASE(test_defaults)
{
    json obj(json::an_object);

    obj["field1"] = 1;
    obj["field3"] = "Toronto";

    double x1 = obj.has_member("field1") ? obj["field1"].as_double() : 10.0;
    double x2 = obj.has_member("field2") ? obj["field2"].as_double() : 20.0;

    std::cout << "x1=" << x1 << std::endl;
    std::cout << "x2=" << x2 << std::endl;

    std::string x3 = obj.get("field3","Montreal").as_string();
    std::string x4 = obj.get("field4","San Francisco").as_string();

    std::cout << "x3=" << x3 << std::endl;
    std::cout << "x4=" << x4 << std::endl;
}

BOOST_AUTO_TEST_CASE(test_wjson)
{
    wjson root(wjson::an_object);
    //root[L"field1"] = L"test";
    //wjson root = wjson::parse(L"{}");
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
    std::ofstream os("test.json",std::ofstream::binary);

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

    output_format format(false);
    json_stream_writer writer(os, format);

    std::clock_t t = std::clock();

    writer.begin_array();
    for (size_t i = 0; i < 100000; ++i)
    {
        writer.begin_element();
        writer.begin_object();
        writer.begin_member(&person[0],person.length());
        writer.begin_object();
        writer.begin_member(&first_name[0],first_name.length());
        writer.value(&john_first_name[0],john_first_name.length());
        writer.end_member();
        writer.begin_member(&last_name[0],last_name.length());
        writer.value(&john_last_name[0],john_last_name.length());
        writer.end_member();
        writer.begin_member(&birthdate[0],birthdate.length());
        writer.value(&john_birthdate[0],john_birthdate.length());
        writer.end_member();
        writer.begin_member(&sex[0],sex.length());
        writer.value(&john_sex[0],john_sex.length());
        writer.end_member();
        writer.begin_member(&salary[0],salary.length());
        writer.value((long long)70000);
        writer.end_member();
        writer.begin_member(&interests[0],interests.length());
        writer.begin_array();
        writer.begin_element();
        writer.value(&reading[0],reading.length());
        writer.end_element();
        writer.begin_element();
        writer.value(&mountain_biking[0],mountain_biking.length());
        writer.end_element();
        writer.begin_element();
        writer.value(&hacking[0],hacking.length());
        writer.end_element();
        writer.end_array();
        writer.end_member();
        writer.begin_member(&favorites[0],favorites.length());
        writer.begin_object();
        writer.begin_member(&color[0],color.length());
        writer.value(&john_color[0],john_color.length());
        writer.end_member();
        writer.begin_member(&sport[0],sport.length());
        writer.value(&john_sport[0],john_sport.length());
        writer.end_member();
        writer.begin_member(&food[0],food.length());
        writer.value(&john_food[0],john_food.length());
        writer.end_member();
        writer.end_object();
        writer.end_member();

        writer.end_object();
        writer.end_member();
        writer.end_object();
        writer.end_element();
    }
    writer.end_array();
    os.flush();
    std::clock_t s = std::clock() - t;
    std::cout << "It took " << (((double)s)/CLOCKS_PER_SEC) << " seconds to write.\n";

    std::ifstream is("test.json",std::ofstream::binary);
    t = std::clock();

    json root = json::parse(is);
    s = std::clock() - t;
    std::cout << "It took " << (((double)s)/CLOCKS_PER_SEC) << " seconds.\n";
 
}

