// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <new>
#include <boost/optional.hpp>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(jsoncons_tests)

BOOST_AUTO_TEST_CASE(test_1)
{
    basic_json<char32_t> j; 

    std::basic_ostringstream<char32_t> os;

    std::cout << sizeof(json) << std::endl;

    //os << j << U"\n";
}

BOOST_AUTO_TEST_CASE(test_shrink_to_fit)
{
    json val = json::make_array(3);
    val.reserve(100);
    val[0].reserve(100);
    val[0]["key"] = "value";
    val.shrink_to_fit();
    BOOST_CHECK_EQUAL(3,val.size());
    BOOST_CHECK_EQUAL(1,val[0].size());
}

BOOST_AUTO_TEST_CASE(test_boost_optional)
{
    boost::optional<jsoncons::json> opt_json;
    opt_json = json::object();
}

BOOST_AUTO_TEST_CASE(test_for_each_value)
{
    std::string input = "{\"A\":\"Jane\", \"B\":\"Roe\",\"C\":10}";
    json val = json::parse(input);

    json::object_iterator it = val.object_range().begin();

    BOOST_CHECK(it->value().is_string());
    ++it;
    BOOST_CHECK(it->value().is_string());
    ++it;
    BOOST_CHECK(it->value().major_type() == jsoncons::json_major_type::uinteger_t);
    ++it;
    BOOST_CHECK(it == val.object_range().end());
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
    root["myarray"] = json::array();

    json double_2 = root["myobject"]["double_2"];

    BOOST_CHECK_CLOSE(double_2.as<double>(), 7.0, 0.000001);
    BOOST_CHECK(double_2.as<int>() == 7);
    BOOST_CHECK(root["myobject"]["bool_2"].as<bool>());
    BOOST_CHECK(root["myobject"]["int_2"].as_integer() == 0);
    BOOST_CHECK(root["myobject"]["string_2"].as<std::string>() == std::string("my string"));

    BOOST_CHECK(root["myobject"]["bool_2"].as<bool>());
    BOOST_CHECK(root["myobject"]["int_2"].as<long long>() == 0);
    BOOST_CHECK(root["myobject"]["string_2"].as<std::string>() == std::string("my string"));

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

    root["addresses"] = addresses;

    BOOST_CHECK(root["addresses"].size() == 2);

}

BOOST_AUTO_TEST_CASE(test_null)
{
    json nullval = json::null();
    BOOST_CHECK(nullval.is_null());
    BOOST_CHECK(nullval.is<jsoncons::null_type>());

    json obj;
    obj["field"] = json::null();
    BOOST_CHECK(obj["field"] == json::null());
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


    json root = json::parse(os.str());

    BOOST_CHECK(root["null"].is_null());
    BOOST_CHECK(root["null"].is<jsoncons::null_type>());
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

BOOST_AUTO_TEST_CASE(test_u0000)
{
    std::string inputStr("[\"\\u0040\\u0040\\u0000\\u0011\"]");
    //std::cout << "Input:    " << inputStr << std::endl;
    json arr = json::parse(inputStr);

    std::string s = arr[0].as<std::string>();
    BOOST_REQUIRE(4 == s.length());
    BOOST_CHECK(static_cast<uint8_t>(s[0]) == 0x40);
    BOOST_CHECK(static_cast<uint8_t>(s[1]) == 0x40);
    BOOST_CHECK(static_cast<uint8_t>(s[2]) == 0x00);
    BOOST_CHECK(static_cast<uint8_t>(s[3]) == 0x11);

    std::ostringstream os;
    os << arr;
    std::string s2 = os.str();

    //std::cout << std::hex << "Output:   " << os.str() << std::endl;

}

BOOST_AUTO_TEST_CASE(test_uHHHH)
{
    std::string inputStr("[\"\\u007F\\u07FF\\u0800\"]");
    json arr = json::parse(inputStr);

    std::string s = arr[0].as<std::string>();
    BOOST_REQUIRE(s.length() == 6);
    BOOST_CHECK(static_cast<uint8_t>(s[0]) == 0x7f);
    BOOST_CHECK(static_cast<uint8_t>(s[1]) == 0xdf);
    BOOST_CHECK(static_cast<uint8_t>(s[2]) == 0xbf);
    BOOST_CHECK(static_cast<uint8_t>(s[3]) == 0xe0);
    BOOST_CHECK(static_cast<uint8_t>(s[4]) == 0xa0);
    BOOST_CHECK(static_cast<uint8_t>(s[5]) == 0x80);

    std::ostringstream os;
    json_serializing_options options;
    options.escape_all_non_ascii(true);
    arr.dump(os, options);
    std::string outputStr = os.str();

    json arr2 = json::parse(outputStr);
    std::string s2 = arr2[0].as<std::string>();
    BOOST_REQUIRE(s2.length() == 6);
    BOOST_CHECK(static_cast<uint8_t>(s2[0]) == 0x7f);
    BOOST_CHECK(static_cast<uint8_t>(s2[1]) == 0xdf);
    BOOST_CHECK(static_cast<uint8_t>(s2[2]) == 0xbf);
    BOOST_CHECK(static_cast<uint8_t>(s2[3]) == 0xe0);
    BOOST_CHECK(static_cast<uint8_t>(s2[4]) == 0xa0);
    BOOST_CHECK(static_cast<uint8_t>(s2[5]) == 0x80);

}

BOOST_AUTO_TEST_CASE(test_multiline_comments)
{
    std::string path = "./input/json-multiline-comment.json";
    std::fstream is(path);
    if (!is)
    {
        std::cout << "Cannot open " << path << std::endl;
        return;
    }
    json j = json::parse(is);

    BOOST_CHECK(j.is_array());
    BOOST_CHECK(j.is<json::array>());
    BOOST_CHECK_EQUAL(j.size(), 0);
}

BOOST_AUTO_TEST_SUITE_END()

