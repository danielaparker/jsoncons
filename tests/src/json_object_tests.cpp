// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <map>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(json_object_tests)

BOOST_AUTO_TEST_CASE(as_test)
{
    json j;
    std::string s = j.as<std::string>();
    BOOST_CHECK_EQUAL("{}",s);
}

BOOST_AUTO_TEST_CASE(as_test2)
{
    try
    {
        json j;
        std::string s = j["empty"].as<std::string>();
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_last_wins_unique_sequence_1)
{
    std::vector<std::string> u = {"a","c","a"};
    auto it = last_wins_unique_sequence(u.begin(),u.end(),
                              [](const std::string& a, const std::string& b){return a.compare(b);});
    std::vector<std::string> expected = { "c","a"};
    size_t count = std::distance(u.begin(),it);

    BOOST_REQUIRE(expected.size() == count);
    for (size_t i = 0; i < count; ++i)
    {
        BOOST_CHECK (expected[i] == u[i]);
    }
}

BOOST_AUTO_TEST_CASE(test_last_wins_unique_sequence_2)
{
    std::vector<std::string> u = {"a","c","a","c"};
    auto it = last_wins_unique_sequence(u.begin(),u.end(),
                              [](const std::string& a, const std::string& b){return a.compare(b);});
    std::vector<std::string> expected = { "a","c"};
    size_t count = std::distance(u.begin(),it);

    BOOST_REQUIRE(expected.size() == count);
    for (size_t i = 0; i < count; ++i)
    {
        BOOST_CHECK (expected[i] == u[i]);
    }
}
BOOST_AUTO_TEST_CASE(test_last_wins_unique_sequence_3)
{
    std::vector<std::string> u = {"a","c","a","d","e"};
    auto it = last_wins_unique_sequence(u.begin(),u.end(),
                              [](const std::string& a, const std::string& b){return a.compare(b);});
    std::vector<std::string> expected = { "c","a","d","e"};
    size_t count = std::distance(u.begin(),it);

    BOOST_REQUIRE(expected.size() == count);
    for (size_t i = 0; i < count; ++i)
    {
        BOOST_CHECK (expected[i] == u[i]);
    }
}
BOOST_AUTO_TEST_CASE(test_last_wins_unique_sequence_4)
{
    std::vector<std::string> u = { "a","c","a","d","e","e","f","a" };
    auto it = last_wins_unique_sequence(u.begin(),u.end(),
                              [](const std::string& a, const std::string& b){return a.compare(b);});
    std::vector<std::string> expected = { "c","d","e","f","a" };
    size_t count = std::distance(u.begin(),it);

    BOOST_REQUIRE(expected.size() == count);
    for (size_t i = 0; i < count; ++i)
    {
        BOOST_CHECK (expected[i] == u[i]);
    }
}
BOOST_AUTO_TEST_CASE(test_last_wins_unique_sequence_5)
{
    std::vector<std::string> u = { "a","b","f","e","c","d"};
    auto it = last_wins_unique_sequence(u.begin(),u.end(),
                              [](const std::string& a, const std::string& b){return a.compare(b);});
    std::vector<std::string> expected = { "a","b","f","e","c","d" };
    size_t count = std::distance(u.begin(),it);

    BOOST_REQUIRE(expected.size() == count);
    for (size_t i = 0; i < count; ++i)
    {
        BOOST_CHECK (expected[i] == u[i]);
    }
}

BOOST_AUTO_TEST_CASE(parse_duplicate_names)
{
    json j1 = json::parse(R"({"first":1,"second":2,"third":3})");
    BOOST_CHECK_EQUAL(3,j1.size());
    BOOST_CHECK_EQUAL(1,j1["first"].as<int>());
    BOOST_CHECK_EQUAL(2,j1["second"].as<int>());
    BOOST_CHECK_EQUAL(3,j1["third"].as<int>());

    json j2 = json::parse(R"({"first":1,"second":2,"first":3})");
    BOOST_CHECK_EQUAL(2,j2.size());
    BOOST_CHECK_EQUAL(3,j2["first"].as<int>());
    BOOST_CHECK_EQUAL(2,j2["second"].as<int>());

    ojson oj1 = ojson::parse(R"({"first":1,"second":2,"third":3})");
    BOOST_CHECK_EQUAL(3,oj1.size());
    BOOST_CHECK_EQUAL(1,oj1["first"].as<int>());
    BOOST_CHECK_EQUAL(2,oj1["second"].as<int>());
    BOOST_CHECK_EQUAL(3,oj1["third"].as<int>());

    ojson oj2 = ojson::parse(R"({"first":1,"second":2,"first":3})");
    BOOST_CHECK_EQUAL(2,oj2.size());
    BOOST_CHECK_EQUAL(3,oj2["first"].as<int>());
    BOOST_CHECK_EQUAL(2,oj2["second"].as<int>());
}

BOOST_AUTO_TEST_CASE(test_erase_member)
{
    json o;
    o["key"] = "Hello";

    BOOST_CHECK(o.size() == 1);
    o.erase("key");
    BOOST_CHECK(o.size() == 0);

    json a;
    json b = json::object();
    b["input-file"] = "config_file";
    json b_copy = b;

    a["b"] = std::move(b);

    BOOST_CHECK_EQUAL(true,a["b"].is_object());
    BOOST_CHECK_EQUAL(a["b"],b_copy);
}

BOOST_AUTO_TEST_CASE(test_object_erase_range)
{
    json o;
    o["key1"] = "value1";
    o["key2"] = "value2";
    o["key3"] = "value3";
    o["key4"] = "value4";

    auto first = o.find("key2");
    auto last = o.find("key4");

    o.erase(first,last);
    
    BOOST_CHECK_EQUAL(2,o.size());
    BOOST_CHECK_EQUAL(1,o.count("key1"));
    BOOST_CHECK_EQUAL(1,o.count("key4"));
}

BOOST_AUTO_TEST_CASE(test_empty_object)
{
    json a;
    BOOST_CHECK(a.size() == 0);
    BOOST_CHECK(a.is_object());
    BOOST_CHECK(a.is<json::object>());

    json::object_iterator begin = a.object_range().begin();
    json::object_iterator end = a.object_range().end();

    for (json::object_iterator it = begin; it != end; ++it)
    {
        BOOST_CHECK(false);
    }

    a["key"] = "Hello";
    BOOST_CHECK(a.size() == 1);
    BOOST_CHECK(a.is_object());
    BOOST_CHECK(a.is<json::object>());
}

BOOST_AUTO_TEST_CASE(test_const_empty_object)
{
    const json b;
    BOOST_CHECK(b.size() == 0);
    BOOST_CHECK(b.is_object());
    BOOST_CHECK(b.is<json::object>());

    json::const_object_iterator begin = b.object_range().begin();
    json::const_object_iterator end = b.object_range().end();

    for (json::const_object_iterator it = begin; it != end; ++it)
    {
        BOOST_CHECK(false);
    }
}

BOOST_AUTO_TEST_CASE(test_empty_object_reserve)
{
    json c;
    BOOST_CHECK(c.size() == 0);
    BOOST_CHECK(c.is_object());
    BOOST_CHECK(c.is<json::object>());
    c.reserve(100);
    BOOST_CHECK(c.capacity() == 100);
    c["key"] = "Hello";
    BOOST_CHECK(c.size() == 1);
    BOOST_CHECK(c.is_object());
    BOOST_CHECK(c.is<json::object>());
    BOOST_CHECK(c.capacity() == 100);
}

BOOST_AUTO_TEST_CASE(test_empty_object_copy)
{
    json a;
    BOOST_CHECK(a.size() == 0);
    BOOST_CHECK(a.is_object());
    BOOST_CHECK(a.is<json::object>());

    json b = a;
    BOOST_CHECK(b.size() == 0);
    BOOST_CHECK(b.is_object());
    BOOST_CHECK(b.is<json::object>());
}

BOOST_AUTO_TEST_CASE(test_empty_object_assignment)
{
    json a;
    BOOST_CHECK(a.size() == 0);
    BOOST_CHECK(a.is_object());
    BOOST_CHECK(a.is<json::object>());

    json b = json::make_array<1>(10);
    BOOST_CHECK(b.size() == 10);
    BOOST_CHECK(b.is_array());
    BOOST_CHECK(b.is<json::array>());

    b = a;
    BOOST_CHECK(b.size() == 0);
    BOOST_CHECK(b.is_object());
    BOOST_CHECK(b.is<json::object>());

    json c;
    c["key"] = "value";
    BOOST_CHECK(c.size() == 1);
    BOOST_CHECK(c.is_object());
    BOOST_CHECK(c.is<json::object>());
    c = a;
    BOOST_CHECK(c.size() == 0);
    BOOST_CHECK(c.is_object());
    BOOST_CHECK(c.is<json::object>());
}

BOOST_AUTO_TEST_CASE(test_get)
{
    json a;

    a["field1"] = "value1";

    std::string s1 = a.at("field1").as<std::string>();
    std::string s1a = a.at("field1").as<std::string>();
    std::string s2 = a.get_with_default("field2","null");
    BOOST_REQUIRE_THROW(a.at("field2"), std::out_of_range);

    BOOST_CHECK(s1 == std::string("value1"));
    BOOST_CHECK(s1a == std::string("value1"));

    //std::cout << "s2=" << s2 << std::endl;
    BOOST_CHECK_EQUAL(std::string("null"),s2);
}

BOOST_AUTO_TEST_CASE(test_proxy_get)
{
    json a;

    a["object1"] = json();
    a["object1"]["field1"] = "value1";

    std::string s1 = a["object1"].at("field1").as<std::string>();
    std::string s1a = a["object1"].at("field1").as<std::string>();
    std::string s2 = a["object1"].get("field2",json::null()).as<std::string>();
    a["object1"].get("field2", json::null()).is_null();
    //std::cout << s2 << std::endl;
    BOOST_REQUIRE_THROW(a["object1"].at("field2").as<std::string>(), std::out_of_range);

    BOOST_CHECK_EQUAL(std::string("value1"),s1);
    BOOST_CHECK_EQUAL(std::string("value1"),s1a);
    BOOST_CHECK_EQUAL(std::string("null"),s2);
}

BOOST_AUTO_TEST_CASE(test_proxy_get_with_default)
{
    json a;

    a["object1"] = json();
    a["object1"]["field1"] = "3.7";
    a["object1"]["field2"] = 1.5;

    std::string s1 = a["object1"].get_with_default("field1","default");
    std::string s2 = a["object1"].get_with_default("field2","1.0");
    std::string s3 = a["object1"].get_with_default("field3","1.0");
    std::string s4 = a["object1"].get_with_default<std::string>("field2","1.0");
    std::string s5 = a["object1"].get_with_default<std::string>("field3","1.0");
    double d1 = a["object1"].get_with_default("field1",1.0);
    double d2 = a["object1"].get_with_default("field2",1.0);
    double d3 = a["object1"].get_with_default("field3",1.0);

    BOOST_CHECK_EQUAL(std::string("3.7"),s1);
    BOOST_CHECK_EQUAL(std::string("1.5"),s2);
    BOOST_CHECK_EQUAL(std::string("1.0"),s3);
    BOOST_CHECK_EQUAL(std::string("1.5"),s4);
    BOOST_CHECK_EQUAL(std::string("1.0"),s5);
    BOOST_CHECK_EQUAL(3.7,d1);
    BOOST_CHECK_EQUAL(1.5,d2);
    BOOST_CHECK_EQUAL(1,d3);
}

BOOST_AUTO_TEST_CASE(test_set_and_proxy_set)
{
    json a;

    a.insert_or_assign("object1",json());
    a.insert_or_assign("field1","value1");
    a["object1"].insert_or_assign("field2","value2");

    BOOST_CHECK_EQUAL(std::string("value1"),a["field1"].as<std::string>());
    BOOST_CHECK_EQUAL(std::string("value2"),a["object1"]["field2"].as<std::string>());
}

BOOST_AUTO_TEST_CASE(test_emplace_and_proxy_set)
{
    json a;

    a.try_emplace("object1",json());
    a.try_emplace("field1","value1");
    a["object1"].try_emplace("field2","value2");

    BOOST_CHECK_EQUAL(std::string("value1"),a["field1"].as<std::string>());
    BOOST_CHECK_EQUAL(std::string("value2"),a["object1"]["field2"].as<std::string>());
}

BOOST_AUTO_TEST_CASE(test_const_member_read)
{
    json a;

    a["field1"] = 10;

    a["field2"];

    const json b(a);

    int val1 = b["field1"].as<int>();
    BOOST_CHECK(val1 == 10);
    BOOST_REQUIRE_THROW(b["field2"], std::out_of_range);
}

BOOST_AUTO_TEST_CASE(test_proxy_const_member_read)
{
    json a;

    a["object1"] = json();
    a["object1"]["field1"] = "value1";
    a["object1"]["field2"]; // No throw yet

    const json b(a);

    std::string s1 = b["object1"]["field1"].as<std::string>();
    BOOST_REQUIRE_THROW(b["object1"]["field2"], std::out_of_range);

    BOOST_CHECK(s1 == std::string("value1"));
}

BOOST_AUTO_TEST_CASE(test_object_equals)
{
    json a;
    a["field1"] = "value1";

    json b;
    b["field1"] = "value1";

    BOOST_CHECK(a == b);

    json c;
    c["field1"] = 10;

    BOOST_CHECK(a != c);
}

BOOST_AUTO_TEST_CASE(test_json_object_iterator_1)
{
    json a;
    a["name1"] = "value1";
    a["name2"] = "value2";
    a["name3"] = "value3";

    json::object_iterator it = a.object_range().begin();
    BOOST_CHECK((*it).key() == "name1");
    BOOST_CHECK((*it).value() == json("value1"));
    ++it;
    BOOST_CHECK((*it).key() == "name2");
    BOOST_CHECK((*it).value() == json("value2"));

    BOOST_CHECK((*(it++)).key() == "name2");
    BOOST_CHECK((*it).key() == "name3");
    BOOST_CHECK((*it).value() == json("value3"));

    BOOST_CHECK((*(it--)).key() == "name3");
    BOOST_CHECK((*it).value() == json("value2"));
    BOOST_CHECK((*(--it)).value() == json("value1"));

    json::key_value_pair_type member = *it;
    BOOST_CHECK(member.key() == "name1");
    BOOST_CHECK(member.value() == json("value1"));
}

BOOST_AUTO_TEST_CASE(test_json_object_iterator_2)
{
    json a;
    a["name1"] = "value1";
    a["name2"] = "value2";
    a["name3"] = "value3";

    json::const_object_iterator it = a.object_range().begin();
    BOOST_CHECK((*it).key() == "name1");
    BOOST_CHECK((*it).value() == json("value1"));
    ++it;
    BOOST_CHECK((*it).key() == "name2");
    BOOST_CHECK((*it).value() == json("value2"));

    BOOST_CHECK((*(it++)).key() == "name2");
    BOOST_CHECK((*it).key() == "name3");
    BOOST_CHECK((*it).value() == json("value3"));

    BOOST_CHECK((*(it--)).key() == "name3");
    BOOST_CHECK((*it).value() == json("value2"));

    BOOST_CHECK((*(--it)).value() == json("value1"));

    json::key_value_pair_type member = *it;
    BOOST_CHECK(member.key() == "name1");
    BOOST_CHECK(member.value() == json("value1"));
}

BOOST_AUTO_TEST_CASE(test_json_object_iterator_3)
{
    json a;
    a["name1"] = "value1";
    a["name2"] = "value2";
    a["name3"] = "value3";

    json::const_object_iterator it = static_cast<const json&>(a).object_range().begin();
    BOOST_CHECK(it == a.object_range().begin());
    BOOST_CHECK(it != a.object_range().end());
    BOOST_CHECK((*it).key() == "name1");
    BOOST_CHECK((*it).value() == json("value1"));
    ++it;
    BOOST_CHECK(it != a.object_range().end());
    BOOST_CHECK((*it).key() == "name2");
    BOOST_CHECK((*it).value() == json("value2"));

    BOOST_CHECK((*(it++)).key() == "name2");
    BOOST_CHECK(it != a.object_range().end());
    BOOST_CHECK((*it).key() == "name3");
    BOOST_CHECK((*it).value() == json("value3"));

    BOOST_CHECK((*(it--)).key() == "name3");
    BOOST_CHECK((*it).value() == json("value2"));

    BOOST_CHECK((*(--it)).value() == json("value1"));
    BOOST_CHECK(it == a.object_range().begin());

    json::key_value_pair_type member = *it;
    BOOST_CHECK(member.key() == "name1");
    BOOST_CHECK(member.value() == json("value1"));

    //*it = member; // Don't want this to compile
}

BOOST_AUTO_TEST_CASE(test_object_key_proxy)
{
    json a;
    a["key1"] = "value1";

    json b;
    b["key2"] = json();
    b["key2"]["key3"] = std::move(a);

    BOOST_CHECK(!(a.is_object() || a.is_array() || a.is_string()));
}

// accessor tests


BOOST_AUTO_TEST_CASE(test_get_with_string_default)
{
    json example;

    std::string s("too long string for short string");
    std::string result = example.get_with_default("test", s);
    BOOST_CHECK_EQUAL(s,result);
}

BOOST_AUTO_TEST_CASE(test_compare_with_string)
{
    json a;
    a["key"] = "value";
    a["key1"] = "value1";
    a["key2"] = "value2";
    BOOST_CHECK(a["key"] == a["key"]);
    BOOST_CHECK(!(a["key"] == a["key1"]));
    BOOST_CHECK(!(a["key"] == a["key2"]));
}

BOOST_AUTO_TEST_CASE(test_count)
{
    json a;
    a["key1"] = "value1";
    a["key2"] = "value2";

    BOOST_CHECK_EQUAL(1, a.count("key1"));
    BOOST_CHECK_EQUAL(1, a.count("key2"));
    BOOST_CHECK_EQUAL(0, a.count("key3"));

    json b = json::parse(
        "{\"key1\":\"a value\",\"key1\":\"another value\"}"
        );
    BOOST_CHECK_EQUAL(1, b.count("key1"));
}

BOOST_AUTO_TEST_CASE(test_find)
{
    json obj;

    json::object_iterator it = obj.find("key");
    BOOST_CHECK(it == obj.object_range().end());

    obj["key1"] = 10;
    obj["key2"] = true;
    obj["key3"] = 'c';
    obj["key4"] = "value4";

    json::object_iterator it2 =  obj.find("key");
    BOOST_CHECK(it2 == obj.object_range().end());

    json::object_iterator it3 =  obj.find("key4");
    BOOST_CHECK(it3 != obj.object_range().end());
    BOOST_CHECK_EQUAL(std::string("value4"), it3->value().as<std::string>());
}

BOOST_AUTO_TEST_CASE(test_as)
{
    json obj;
    obj["field1"] = 10;
    obj["field2"] = true;
    obj["char_field"] = 'c';
    obj["string_field"] = "char";

    std::string s = obj["field1"].as<std::string>();
    BOOST_CHECK_EQUAL(std::string("10"),s);
    int int_val = obj["field2"].as<int>();
    BOOST_CHECK_EQUAL(1,int_val);
    int short_val = obj["field2"].as<short>();
    BOOST_CHECK_EQUAL(short_val,1);
    int ushort_val = obj["field2"].as<unsigned short>();
    BOOST_CHECK_EQUAL(ushort_val,static_cast<unsigned short>(1));
    char char_val = obj["field2"].as<char>();
    BOOST_CHECK_EQUAL(int(char_val),1);

    BOOST_CHECK(obj["char_field"].is<char>());
    BOOST_CHECK(!obj["string_field"].is<char>());

    json parent;
    parent["child"] = obj;
    s = parent["child"]["field1"].as<std::string>();
    BOOST_CHECK_EQUAL(s,std::string("10"));
    int_val = parent["child"]["field2"].as<int>();
    BOOST_CHECK_EQUAL(int_val,1);
    short_val = parent["child"]["field2"].as<short>();
    BOOST_CHECK_EQUAL(short_val,1);

    //json::object x = parent["child"].as<json::object>();
    // Compile time error, "as<Json::object> not supported"

    json empty;
    BOOST_CHECK(empty.is_object());
    BOOST_CHECK(empty.empty());

    //json::object y = empty.as<json::object>();
    // Compile time error, "as<Json::object> not supported"
}

BOOST_AUTO_TEST_CASE(test_as2)
{
    json obj;
    obj["field1"] = "10";
    obj["field2"] = "-10";
    obj["field3"] = "10.1";

    BOOST_CHECK_EQUAL(10,obj["field1"].as<int>());
    BOOST_CHECK_EQUAL(-10,obj["field2"].as<int>());
    BOOST_CHECK_EQUAL(10.1,obj["field3"].as<double>());
}

BOOST_AUTO_TEST_CASE(test_is)
{
    json obj;
    obj["field1"] = 10;
    obj["field2"] = -10;
    obj["field3"] = 10U;

    BOOST_CHECK(obj["field1"].major_type() == jsoncons::json_major_type::integer_t);
    BOOST_CHECK(obj["field2"].major_type() == jsoncons::json_major_type::integer_t);
    BOOST_CHECK(obj["field3"].major_type() == jsoncons::json_major_type::uinteger_t);

    BOOST_CHECK(!obj["field1"].is<std::string>());
    BOOST_CHECK(obj["field1"].is<short>());
    BOOST_CHECK(obj["field1"].is<int>());
    BOOST_CHECK(obj["field1"].is<long>());
    BOOST_CHECK(obj["field1"].is<long long>());
    BOOST_CHECK(obj["field1"].is<unsigned int>());
    BOOST_CHECK(obj["field1"].is<unsigned long>());
    BOOST_CHECK(obj["field1"].is<unsigned long long>());
    BOOST_CHECK(!obj["field1"].is<double>());

    BOOST_CHECK(!obj["field2"].is<std::string>());
    BOOST_CHECK(obj["field2"].is<short>());
    BOOST_CHECK(obj["field2"].is<int>());
    BOOST_CHECK(obj["field2"].is<long>());
    BOOST_CHECK(obj["field2"].is<long long>());
    BOOST_CHECK(!obj["field2"].is<unsigned short>());
    BOOST_CHECK(!obj["field2"].is<unsigned int>());
    BOOST_CHECK(!obj["field2"].is<unsigned long>());
    BOOST_CHECK(!obj["field2"].is<unsigned long long>());
    BOOST_CHECK(!obj["field2"].is<double>());

    BOOST_CHECK(!obj["field3"].is<std::string>());
    BOOST_CHECK(obj["field3"].is<short>());
    BOOST_CHECK(obj["field3"].is<int>());
    BOOST_CHECK(obj["field3"].is<long>());
    BOOST_CHECK(obj["field3"].is<long long>());
    BOOST_CHECK(obj["field3"].is<unsigned int>());
    BOOST_CHECK(obj["field3"].is<unsigned long>());
    BOOST_CHECK(obj["field3"].is<unsigned long long>());
    BOOST_CHECK(!obj["field3"].is<double>());
}

BOOST_AUTO_TEST_CASE(test_is2)
{
    json obj = json::parse("{\"field1\":10}");

    BOOST_CHECK(obj["field1"].major_type() == jsoncons::json_major_type::uinteger_t);

    BOOST_CHECK(!obj["field1"].is<std::string>());
    BOOST_CHECK(obj["field1"].is<int>());
    BOOST_CHECK(obj["field1"].is<long>());
    BOOST_CHECK(obj["field1"].is<long long>());
    BOOST_CHECK(obj["field1"].is<unsigned int>());
    BOOST_CHECK(obj["field1"].is<unsigned long>());
    BOOST_CHECK(obj["field1"].is<unsigned long long>());
    BOOST_CHECK(!obj["field1"].is<double>());
}

BOOST_AUTO_TEST_CASE(test_is_type)
{
    json obj;
    BOOST_CHECK(obj.is_object());
    BOOST_CHECK(obj.is<json::object>());

    // tests for proxy is_type methods
    obj["string"] = "val1";

    BOOST_CHECK(obj.is_object());
    BOOST_CHECK(obj.is<json::object>());

    BOOST_CHECK(obj["string"].is_string());
    BOOST_CHECK(obj["string"].is<std::string>());

    obj["double"] = 10.7;
    BOOST_CHECK(obj["double"].is_double());
    BOOST_CHECK(obj["double"].is<double>());

    obj["int"] = -10;
    BOOST_CHECK(obj["int"].is_integer());
    BOOST_CHECK(obj["int"].is<long long>());

    obj["uint"] = 10u;
    BOOST_CHECK(obj["uint"].is_uinteger());
    BOOST_CHECK(obj["uint"].is<unsigned long long>());

    obj["long"] = static_cast<long>(10);
    BOOST_CHECK(obj["long"].is_integer());
    BOOST_CHECK(obj["long"].is<long long>());

    obj["ulong"] = static_cast<unsigned long>(10);
    BOOST_CHECK(obj["ulong"].is_uinteger());
    BOOST_CHECK(obj["ulong"].is<unsigned long long>());

    obj["longlong"] = static_cast<long long>(10);
    BOOST_CHECK(obj["longlong"].is_integer());
    BOOST_CHECK(obj["longlong"].is<long long>());

    obj["ulonglong"] = static_cast<unsigned long long>(10);
    BOOST_CHECK(obj["ulonglong"].is_uinteger());
    BOOST_CHECK(obj["ulonglong"].is<unsigned long long>());

    obj["true"] = true;
    BOOST_CHECK(obj["true"].is_bool());
    BOOST_CHECK(obj["true"].is<bool>());

    obj["false"] = false;
    BOOST_CHECK(obj["false"].is_bool());
    BOOST_CHECK(obj["false"].is<bool>());

    obj["null1"] = json::null();
    BOOST_CHECK(obj["null1"].is_null());

    obj["object"] = json();
    BOOST_CHECK(obj["object"].is_object());
    BOOST_CHECK(obj["object"].is<json::object>());

    obj["array"] = json::array();
    BOOST_CHECK(obj["array"].is_array());
    BOOST_CHECK(obj["array"].is<json::array>());

    // tests for json is_type methods

    json str = obj["string"];
    BOOST_CHECK(str.is<std::string>());
    BOOST_CHECK(str.is<std::string>());
}

BOOST_AUTO_TEST_CASE(test_as_vector_of_double)
{
    std::string s("[0,1.1,2,3.1]");
    json val = json::parse(s);

    std::vector<double> v = val.as<std::vector<double>>(); 
    BOOST_CHECK(v.size() == 4);
    BOOST_CHECK_CLOSE(v[0],0.0,0.0000000001);
    BOOST_CHECK_CLOSE(v[1],1.1,0.0000000001);
    BOOST_CHECK_CLOSE(v[2],2.0,0.0000000001);
    BOOST_CHECK_CLOSE(v[3],3.1,0.0000000001);
}

BOOST_AUTO_TEST_CASE(test_as_vector_of_bool)
{
    std::string s("[true,false,true]");
    json val = json::parse(s);

    std::vector<bool> v = val.as<std::vector<bool>>(); 
    BOOST_CHECK(v.size() == 3);
    BOOST_CHECK_EQUAL(v[0],true);
    BOOST_CHECK_EQUAL(v[1],false);
    BOOST_CHECK_EQUAL(v[2],true);
}

BOOST_AUTO_TEST_CASE(test_as_vector_of_string)
{
    std::string s("[\"Hello\",\"World\"]");
    json val = json::parse(s);

    std::vector<std::string> v = val.as<std::vector<std::string>>(); 
    BOOST_CHECK(v.size() == 2);
    BOOST_CHECK(v[0] == "Hello");
    BOOST_CHECK(v[1] == "World");
}

BOOST_AUTO_TEST_CASE(test_as_vector_of_char)
{
    std::string s("[20,30]");
    json val = json::parse(s);

    std::vector<char> v = val.as<std::vector<char>>(); 
    BOOST_CHECK(v.size() == 2);
    BOOST_CHECK(v[0] == 20);
    BOOST_CHECK(v[1] == 30);
}

BOOST_AUTO_TEST_CASE(test_as_vector_of_int)
{
    std::string s("[0,1,2,3]");
    json val = json::parse(s);

    std::vector<int> v = val.as<std::vector<int>>(); 
    BOOST_CHECK(v.size() == 4);
    BOOST_CHECK(v[0]==0);
    BOOST_CHECK(v[1]==1);
    BOOST_CHECK(v[2]==2);
    BOOST_CHECK(v[3]==3);

    std::vector<unsigned int> v1 = val.as<std::vector<unsigned int>>(); 
    BOOST_CHECK(v1.size() == 4);
    BOOST_CHECK(v1[0]==0);
    BOOST_CHECK(v1[1]==1);
    BOOST_CHECK(v1[2]==2);
    BOOST_CHECK(v1[3]==3);

    std::vector<long> v2 = val.as<std::vector<long>>(); 
    BOOST_CHECK(v2.size() == 4);
    BOOST_CHECK(v2[0]==0);
    BOOST_CHECK(v2[1]==1);
    BOOST_CHECK(v2[2]==2);
    BOOST_CHECK(v2[3]==3);

    std::vector<unsigned long> v3 = val.as<std::vector<unsigned long>>(); 
    BOOST_CHECK(v3.size() == 4);
    BOOST_CHECK(v3[0]==0);
    BOOST_CHECK(v3[1]==1);
    BOOST_CHECK(v3[2]==2);
    BOOST_CHECK(v3[3]==3);

    std::vector<long long> v4 = val.as<std::vector<long long>>(); 
    BOOST_CHECK(v4.size() == 4);
    BOOST_CHECK(v4[0]==0);
    BOOST_CHECK(v4[1]==1);
    BOOST_CHECK(v4[2]==2);
    BOOST_CHECK(v4[3]==3);

    std::vector<unsigned long long> v5 = val.as<std::vector<unsigned long long>>(); 
    BOOST_CHECK(v5.size() == 4);
    BOOST_CHECK(v5[0]==0);
    BOOST_CHECK(v5[1]==1);
    BOOST_CHECK(v5[2]==2);
    BOOST_CHECK(v5[3]==3);
}

BOOST_AUTO_TEST_CASE(test_as_vector_of_int_on_proxy)
{
    std::string s("[0,1,2,3]");
    json val = json::parse(s);
    json root;
    root["val"] = val;
    std::vector<int> v = root["val"].as<std::vector<int>>();
    BOOST_CHECK(v.size() == 4);
    BOOST_CHECK(v[0]==0);
    BOOST_CHECK(v[1]==1);
    BOOST_CHECK(v[2]==2);
    BOOST_CHECK(v[3]==3);
}

BOOST_AUTO_TEST_CASE(test_defaults)
{
    json obj;

    obj["field1"] = 1;
    obj["field3"] = "Toronto";

    double x1 = obj.count("field1") ? obj["field1"].as<double>() : 10.0;
    double x2 = obj.count("field2") ? obj["field2"].as<double>() : 20.0;


    BOOST_CHECK(x1 == 1.0);
    BOOST_CHECK(x2 == 20.0);

    std::string s1 = obj.get_with_default("field3", "Montreal");
    std::string s2 = obj.get_with_default("field4", "San Francisco");

    BOOST_CHECK_EQUAL(s1,"Toronto");
    BOOST_CHECK_EQUAL(s2,"San Francisco");
}

BOOST_AUTO_TEST_CASE(test_accessing)
{
    json obj;
    obj["first_name"] = "Jane";
    obj["last_name"] = "Roe";
    obj["events_attended"] = 10;
    obj["accept_waiver_of_liability"] = true;

    BOOST_CHECK(obj["first_name"].as<std::string>() == "Jane");
    BOOST_CHECK(obj.at("last_name").as<std::string>() == "Roe");
    BOOST_CHECK(obj["events_attended"].as<int>() == 10);
    BOOST_CHECK(obj["accept_waiver_of_liability"].as<bool>());
}

BOOST_AUTO_TEST_CASE(test_value_not_found_and_defaults)
{
    json obj;
    obj["first_name"] = "Jane";
    obj["last_name"] = "Roe";

    BOOST_CHECK_EXCEPTION(obj["outdoor_experience"].as<std::string>(),
                          std::out_of_range,
                          [](const std::exception& ex ) { return ex.what() == std::string("Key 'outdoor_experience' not found"); });

    std::string experience = obj.count("outdoor_experience") > 0 ? obj["outdoor_experience"].as<std::string>() : "";

    BOOST_CHECK(experience == "");

    BOOST_CHECK_EXCEPTION(obj["first_aid_certification"].as<std::string>(),
                          std::out_of_range,
                          [](const std::exception& ex ) { return ex.what() == std::string("Key 'first_aid_certification' not found"); });
}

BOOST_AUTO_TEST_CASE(test_set_override)
{
    json obj;
    obj["first_name"] = "Jane";
    obj["height"] = 0.9;

    obj["first_name"] = "Joe";
    obj["height"] = "0.3";

    BOOST_CHECK(obj["first_name"] == "Joe");
    BOOST_CHECK_CLOSE(obj["height"].as<double>(),0.3,0.00000000001);
}

// merge tests

BOOST_AUTO_TEST_CASE(test_json_merge)
{
json j = json::parse(R"(
{
    "a" : 1,
    "b" : 2
}
)");
json j2 = j;

const json source = json::parse(R"(
{
    "a" : 2,
    "c" : 3
}
)");

const json expected = json::parse(R"(
{
    "a" : 1,
    "b" : 2,
    "c" : 3
}
)");

    j.merge(source);
    BOOST_CHECK(j.size() == 3);
    BOOST_CHECK_EQUAL(expected,j);

    j2.merge(j2.object_range().begin()+1,source);
    BOOST_CHECK(j2.size() == 3);
    BOOST_CHECK_EQUAL(expected,j2);

    //std::cout << j << std::endl;
}

BOOST_AUTO_TEST_CASE(test_ojson_merge)
{
ojson j = ojson::parse(R"(
{
    "a" : 1,
    "b" : 2
}
)");

ojson j2 = j;

const ojson source = ojson::parse(R"(
{
    "a" : 2,
    "c" : 3
}
)");
const ojson expected = ojson::parse(R"(
{
    "a" : 1,
    "b" : 2,
    "c" : 3
}
)");

    j.merge(source);
    BOOST_CHECK(j.size() == 3);
    BOOST_CHECK_EQUAL(expected,j);

    j2.merge(j2.object_range().begin()+1,source);
    BOOST_CHECK(j2.size() == 3);
    BOOST_CHECK_EQUAL(expected,j2);

    //std::cout << j << std::endl;
}

BOOST_AUTO_TEST_CASE(test_json_merge_move)
{
json j = json::parse(R"(
{
    "a" : "1",
    "b" : [1,2,3]
}
)");
    json j2 = j;

json source = json::parse(R"(
{
    "a" : "2",
    "c" : [4,5,6]
}
)");

json expected = json::parse(R"(
{
    "a" : "1",
    "b" : [1,2,3],
    "c" : [4,5,6]
}
)");

    json source2 = source;

    j.merge(std::move(source));
    BOOST_CHECK(j.size() == 3);
    BOOST_CHECK_EQUAL(expected,j);
    std::cout << pretty_print(j) << std::endl;

    j2.merge(std::move(source2));
    BOOST_CHECK(j2.size() == 3);
    BOOST_CHECK_EQUAL(expected,j2);

    //std::cout << source << std::endl;
}

BOOST_AUTO_TEST_CASE(test_ojson_merge_move)
{
ojson j = ojson::parse(R"(
{
    "a" : "1",
    "d" : [1,2,3]
}
)");
ojson j2 = j;

ojson source = ojson::parse(R"(
{
    "a" : "2",
    "c" : [4,5,6]
}
)");

ojson source2 = source;

ojson expected = ojson::parse(R"(
{
    "d" : [1,2,3],
    "a" : "1",
    "c" : [4,5,6]
}
)");

    j.merge(std::move(source));
    BOOST_CHECK(j.size() == 3);
    BOOST_CHECK_EQUAL(expected,j);

    j2.merge(j2.object_range().begin(),std::move(source2));
    BOOST_CHECK(j2.size() == 3);
    BOOST_CHECK_EQUAL(expected,j2);

    //std::cout << "(1)\n" << j << std::endl;
    //std::cout << "(2)\n" << source << std::endl;
}

// merge_or_update tests

BOOST_AUTO_TEST_CASE(test_json_merge_or_update)
{
json j = json::parse(R"(
{
    "a" : 1,
    "b" : 2
}
)");
json j2 = j;

const json source = json::parse(R"(
{
    "a" : 2,
    "c" : 3
}
)");

const json expected = json::parse(R"(
{
    "a" : 2,
    "b" : 2,
    "c" : 3
}
)");

    j.merge_or_update(source);
    BOOST_CHECK(j.size() == 3);
    BOOST_CHECK_EQUAL(expected,j);

    j2.merge_or_update(j2.object_range().begin()+1,source);
    BOOST_CHECK(j2.size() == 3);
    BOOST_CHECK_EQUAL(expected,j2);

    //std::cout << j << std::endl;
}

BOOST_AUTO_TEST_CASE(test_ojson_merge_or_update)
{
ojson j = ojson::parse(R"(
{
    "a" : 1,
    "b" : 2
}
)");

ojson j2 = j;

const ojson source = ojson::parse(R"(
{
    "a" : 2,
    "c" : 3
}
)");
const ojson expected = ojson::parse(R"(
{
    "a" : 2,
    "b" : 2,
    "c" : 3
}
)");

    j.merge_or_update(source);
    BOOST_CHECK(j.size() == 3);
    BOOST_CHECK_EQUAL(expected,j);

    j2.merge_or_update(j2.object_range().begin()+1,source);
    BOOST_CHECK(j2.size() == 3);
    BOOST_CHECK_EQUAL(expected,j2);

    //std::cout << j << std::endl;
}

BOOST_AUTO_TEST_CASE(test_json_merge_or_update_move)
{
json j = json::parse(R"(
{
    "a" : "1",
    "b" : [1,2,3]
}
)");
    json j2 = j;

json source = json::parse(R"(
{
    "a" : "2",
    "c" : [4,5,6]
}
)");

json expected = json::parse(R"(
{
    "a" : "2",
    "b" : [1,2,3],
    "c" : [4,5,6]
}
)");

    json source2 = source;

    j.merge_or_update(std::move(source));
    BOOST_CHECK(j.size() == 3);
    BOOST_CHECK_EQUAL(expected,j);
    std::cout << pretty_print(j) << std::endl;

    j2.merge_or_update(std::move(source2));
    BOOST_CHECK(j2.size() == 3);
    BOOST_CHECK_EQUAL(expected,j2);

    //std::cout << source << std::endl;
}

BOOST_AUTO_TEST_CASE(test_ojson_merge_or_update_move)
{
ojson j = ojson::parse(R"(
{
    "a" : "1",
    "d" : [1,2,3]
}
)");
ojson j2 = j;

ojson source = ojson::parse(R"(
{
    "a" : "2",
    "c" : [4,5,6]
}
)");

ojson source2 = source;

ojson expected = ojson::parse(R"(
{
    "d" : [1,2,3],
    "a" : "2",
    "c" : [4,5,6]
}
)");

    j.merge_or_update(std::move(source));
    BOOST_CHECK(j.size() == 3);
    BOOST_CHECK_EQUAL(expected,j);

    j2.merge_or_update(j2.object_range().begin(),std::move(source2));
    BOOST_CHECK(j2.size() == 3);
    BOOST_CHECK_EQUAL(expected,j2);

    //std::cout << "(1)\n" << j << std::endl;
    //std::cout << "(2)\n" << source << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()

