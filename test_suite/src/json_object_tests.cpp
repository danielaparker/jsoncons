// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include "jsoncons/json.hpp"
#include "jsoncons/json_serializer.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <map>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(json_object_test_suite)

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

BOOST_AUTO_TEST_CASE(test_empty_object)
{
    json a;
    BOOST_CHECK(a.size() == 0);
    BOOST_CHECK(a.is_object());
    BOOST_CHECK(a.is<json::object>());

    json::object_iterator begin = a.members().begin();
    json::object_iterator end = a.members().end();

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

    json::const_object_iterator begin = b.members().begin();
    json::const_object_iterator end = b.members().end();

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
    std::string s2 = a.get("field2","null").as<std::string>();
    BOOST_REQUIRE_THROW(a.at("field2"), json_exception);

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
    std::string s2 = a["object1"].get("field2",json::null_type()).as<std::string>();
    a["object1"].get("field2", json::null_type()).is_null();
    //std::cout << s2 << std::endl;
    BOOST_REQUIRE_THROW(a["object1"].at("field2").as<std::string>(), std::out_of_range);

    BOOST_CHECK_EQUAL(std::string("value1"),s1);
    BOOST_CHECK_EQUAL(std::string("value1"),s1a);
    BOOST_CHECK_EQUAL(std::string("null"),s2);
}

BOOST_AUTO_TEST_CASE(test_const_member_read)
{
    json a;

    a["field1"] = 10;

    a["field2"];

    const json b(a);

    int val1 = b["field1"].as<int>();
    BOOST_CHECK(val1 == 10);
    BOOST_REQUIRE_THROW(b["field2"], json_exception);
}

BOOST_AUTO_TEST_CASE(test_proxy_const_member_read)
{
    json a;

    a["object1"] = json();
    a["object1"]["field1"] = "value1";
    a["object1"]["field2"]; // No throw yet

    const json b(a);

    std::string s1 = b["object1"]["field1"].as<std::string>();
    BOOST_REQUIRE_THROW(b["object1"]["field2"], json_exception);

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

    json::object_iterator it = a.members().begin();
    BOOST_CHECK((*it).name() == "name1");
    BOOST_CHECK((*it).value() == json("value1"));
    ++it;
    BOOST_CHECK((*it).name() == "name2");
    BOOST_CHECK((*it).value() == json("value2"));

    BOOST_CHECK((*(it++)).name() == "name2");
    BOOST_CHECK((*it).name() == "name3");
    BOOST_CHECK((*it).value() == json("value3"));

    BOOST_CHECK((*(it--)).name() == "name3");
    BOOST_CHECK((*it).value() == json("value2"));
    BOOST_CHECK((*(--it)).value() == json("value1"));

    json::member_type member = *it;
    BOOST_CHECK(member.name() == "name1");
    BOOST_CHECK(member.value() == json("value1"));
}

BOOST_AUTO_TEST_CASE(test_json_object_iterator_2)
{
    json a;
    a["name1"] = "value1";
    a["name2"] = "value2";
    a["name3"] = "value3";

    json::const_object_iterator it = a.members().begin();
    BOOST_CHECK((*it).name() == "name1");
    BOOST_CHECK((*it).value() == json("value1"));
    ++it;
    BOOST_CHECK((*it).name() == "name2");
    BOOST_CHECK((*it).value() == json("value2"));

    BOOST_CHECK((*(it++)).name() == "name2");
    BOOST_CHECK((*it).name() == "name3");
    BOOST_CHECK((*it).value() == json("value3"));

    BOOST_CHECK((*(it--)).name() == "name3");
    BOOST_CHECK((*it).value() == json("value2"));

    BOOST_CHECK((*(--it)).value() == json("value1"));

    json::member_type member = *it;
    BOOST_CHECK(member.name() == "name1");
    BOOST_CHECK(member.value() == json("value1"));
}

BOOST_AUTO_TEST_CASE(test_json_object_iterator_3)
{
    json a;
    a["name1"] = "value1";
    a["name2"] = "value2";
    a["name3"] = "value3";

    json::const_object_iterator it = static_cast<const json&>(a).members().begin();
    BOOST_CHECK(it == a.members().begin());
    BOOST_CHECK(it != a.members().end());
    BOOST_CHECK((*it).name() == "name1");
    BOOST_CHECK((*it).value() == json("value1"));
    ++it;
    BOOST_CHECK(it != a.members().end());
    BOOST_CHECK((*it).name() == "name2");
    BOOST_CHECK((*it).value() == json("value2"));

    BOOST_CHECK((*(it++)).name() == "name2");
    BOOST_CHECK(it != a.members().end());
    BOOST_CHECK((*it).name() == "name3");
    BOOST_CHECK((*it).value() == json("value3"));

    BOOST_CHECK((*(it--)).name() == "name3");
    BOOST_CHECK((*it).value() == json("value2"));

    BOOST_CHECK((*(--it)).value() == json("value1"));
    BOOST_CHECK(it == a.members().begin());

    json::member_type member = *it;
    BOOST_CHECK(member.name() == "name1");
    BOOST_CHECK(member.value() == json("value1"));

    //*it = member; // Don't want this to compile
}
BOOST_AUTO_TEST_SUITE_END()

