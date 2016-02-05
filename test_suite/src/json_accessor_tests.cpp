// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include "jsoncons/json.hpp"
#include "jsoncons/json_serializer.hpp"
#include "my_any_specializations.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;
using boost::numeric::ublas::matrix;


BOOST_AUTO_TEST_SUITE(json_accessor_test_suite)

BOOST_AUTO_TEST_CASE(test_get_with_string_default)
{
  json example;
  std::string result = example.get("test", std::string("too long string for short string")).as<std::string>();
  std::cout << "result:" << result << std::endl;
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

BOOST_AUTO_TEST_CASE(test_object_key_proxy)
{
    json a;
    a["key1"] = "value1";

    json b;
    b["key2"] = json();
    b["key2"]["key3"] = std::move(a);

	std::cout << a.type() << std::endl;
    BOOST_CHECK(a.is_null());
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
	BOOST_CHECK_EQUAL(2, b.count("key1"));
}

BOOST_AUTO_TEST_CASE(test_find)
{
    json obj;

    json::object_iterator it = obj.find("key");
    BOOST_CHECK(it == obj.members().end());

    obj["key1"] = 10;
    obj["key2"] = true;
    obj["key3"] = 'c';
    obj["key4"] = "value4";

    json::object_iterator it2 =  obj.find("key");
    BOOST_CHECK(it2 == obj.members().end());

    json::object_iterator it3 =  obj.find("key4");
    BOOST_CHECK(it3 != obj.members().end());
    BOOST_CHECK_EQUAL("value4",it3->value().as_cstring());
	BOOST_CHECK_EQUAL("value4", it3->value().as<const char*>());
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

    json::object x = parent["child"].as<json::object>();

    json empty;
    BOOST_CHECK(empty.is_object());
    BOOST_CHECK(empty.empty());
    json::object y = empty.as<json::object>();
}

BOOST_AUTO_TEST_CASE(test_is)
{
    json obj;
    obj["field1"] = 10;
    obj["field2"] = -10;
    obj["field3"] = 10U;

    BOOST_CHECK(obj["field1"].type() == jsoncons::value_types::integer_t);
    BOOST_CHECK(obj["field2"].type() == jsoncons::value_types::integer_t);
    BOOST_CHECK(obj["field3"].type() == jsoncons::value_types::uinteger_t);

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

    BOOST_CHECK(obj["field1"].type() == jsoncons::value_types::uinteger_t);

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

    obj["null1"] = json::null_type();
    BOOST_CHECK(obj["null1"].is_null());

    obj["object"] = json();
    BOOST_CHECK(obj["object"].is_object());
    BOOST_CHECK(obj["object"].is<json::object>());

    obj["array"] = json::array();
    BOOST_CHECK(obj["array"].is_array());
    BOOST_CHECK(obj["array"].is<json::array>());

    matrix<double> A;
    obj.set("my-any",json::any(A));
    BOOST_CHECK(obj["my-any"].is<json::any>());

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

BOOST_AUTO_TEST_CASE(test_as_vector_of_bool)
{
    std::string s("[true,false]");
    json val = json::parse(s);

    std::vector<bool> v = val.as<std::vector<bool>>(); 
    BOOST_CHECK(v.size() == 2);
    BOOST_CHECK(v[0]);
    BOOST_CHECK(!v[1]);
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

BOOST_AUTO_TEST_SUITE_END()
