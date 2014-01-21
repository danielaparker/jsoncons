// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include "jsoncons/json.hpp"
#include "jsoncons/json_serializer.hpp"
#include "my_custom_data.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using jsoncons::json_serializer;
using jsoncons::output_format;
using jsoncons::json;
using jsoncons::wjson;
using jsoncons::basic_json_reader;
using std::string;
using boost::numeric::ublas::matrix;

BOOST_AUTO_TEST_CASE(test_is_type)
{
    json obj;
    BOOST_CHECK(obj.is_object());
    BOOST_CHECK(obj.is<json::object_type>());
    std::cout << "HERE!!!"  << ", type=" << obj.type() <<  std::endl;

    // tests for proxy is_type methods
    obj["string"] = "val1";
    std::cout << "type=" << obj.type() << std::endl;

    BOOST_CHECK(obj.is_object());
    BOOST_CHECK(obj.is<json::object_type>());

    BOOST_CHECK(obj["string"].is_string());
    BOOST_CHECK(obj["string"].is<std::string>());

    obj["double"] = 10.7;
    BOOST_CHECK(obj["double"].is_double());
    BOOST_CHECK(obj["double"].is<double>());

    obj["int"] = -10;
    BOOST_CHECK(obj["int"].is_longlong());
    BOOST_CHECK(obj["int"].is<long long>());

    obj["uint"] = 10u;
    BOOST_CHECK(obj["uint"].is_ulonglong());
    BOOST_CHECK(obj["uint"].is<unsigned long long>());

    obj["long"] = static_cast<long>(10);
    BOOST_CHECK(obj["long"].is_longlong());
    BOOST_CHECK(obj["long"].is<long long>());

    obj["ulong"] = static_cast<unsigned long>(10);
    BOOST_CHECK(obj["ulong"].is_ulonglong());
    BOOST_CHECK(obj["ulong"].is<unsigned long long>());

    obj["longlong"] = static_cast<long long>(10);
    BOOST_CHECK(obj["longlong"].is_longlong());
    BOOST_CHECK(obj["longlong"].is<long long>());

    obj["ulonglong"] = static_cast<unsigned long long>(10);
    BOOST_CHECK(obj["ulonglong"].is_ulonglong());
    BOOST_CHECK(obj["ulonglong"].is<unsigned long long>());

    obj["true"] = true;
    BOOST_CHECK(obj["true"].is_bool());
    BOOST_CHECK(obj["true"].is<bool>());

    obj["false"] = false;
    BOOST_CHECK(obj["false"].is_bool());
    BOOST_CHECK(obj["false"].is<bool>());

    obj["null1"] = json::null;
    BOOST_CHECK(obj["null1"].is_null());

    obj["object"] = json::an_object;
    BOOST_CHECK(obj["object"].is_object());
    BOOST_CHECK(obj["object"].is<json::object_type>());

    obj["array"] = json::an_array;
    BOOST_CHECK(obj["array"].is_array());
    BOOST_CHECK(obj["array"].is<json::array_type>());

    matrix<double> A;
    obj.set_custom_data("custom",A);
    BOOST_CHECK(obj["custom"].is_custom());

    // tests for json is_type methods

    json str = obj["string"];
    BOOST_CHECK(str.is_string());
    BOOST_CHECK(str.is<std::string>());
}

BOOST_AUTO_TEST_CASE(test_as_vector_of_double)
{
    std::string s("[0,1.1,2,3.1]");
    json val = json::parse_string(s);

    std::vector<double> v = val.as_vector<double>(); 
    BOOST_CHECK(v.size() == 4);
    BOOST_CHECK_CLOSE(v[0],0.0,0.0000000001);
    BOOST_CHECK_CLOSE(v[1],1.1,0.0000000001);
    BOOST_CHECK_CLOSE(v[2],2.0,0.0000000001);
    BOOST_CHECK_CLOSE(v[3],3.1,0.0000000001);
}

BOOST_AUTO_TEST_CASE(test_as_vector_of_string)
{
    std::string s("[\"Hello\",\"World\"]");
    json val = json::parse_string(s);

    std::vector<string> v = val.as_vector<std::string>(); 
    BOOST_CHECK(v.size() == 2);
    BOOST_CHECK(v[0] == "Hello");
    BOOST_CHECK(v[1] == "World");
}

BOOST_AUTO_TEST_CASE(test_as_vector_of_char)
{
    std::string s("[\"H\",\"W\"]");
    json val = json::parse_string(s);

    std::vector<char> v = val.as_vector<char>(); 
    BOOST_CHECK(v.size() == 2);
    BOOST_CHECK(v[0] == 'H');
    BOOST_CHECK(v[1] == 'W');
}

BOOST_AUTO_TEST_CASE(test_as_vector_of_bool)
{
    std::string s("[true,false]");
    json val = json::parse_string(s);

    std::vector<bool> v = val.as_vector<bool>(); 
    BOOST_CHECK(v.size() == 2);
    BOOST_CHECK(v[0]);
    BOOST_CHECK(!v[1]);
}

BOOST_AUTO_TEST_CASE(test_as_vector_of_int)
{
    std::string s("[0,1,2,3]");
    json val = json::parse_string(s);

    std::vector<int> v = val.as_vector<int>(); 
    BOOST_CHECK(v.size() == 4);
    BOOST_CHECK(v[0]==0);
    BOOST_CHECK(v[1]==1);
    BOOST_CHECK(v[2]==2);
    BOOST_CHECK(v[3]==3);

    std::vector<unsigned int> v1 = val.as_vector<unsigned int>(); 
    BOOST_CHECK(v1.size() == 4);
    BOOST_CHECK(v1[0]==0);
    BOOST_CHECK(v1[1]==1);
    BOOST_CHECK(v1[2]==2);
    BOOST_CHECK(v1[3]==3);

    std::vector<long> v2 = val.as_vector<long>(); 
    BOOST_CHECK(v2.size() == 4);
    BOOST_CHECK(v2[0]==0);
    BOOST_CHECK(v2[1]==1);
    BOOST_CHECK(v2[2]==2);
    BOOST_CHECK(v2[3]==3);

    std::vector<unsigned long> v3 = val.as_vector<unsigned long>(); 
    BOOST_CHECK(v3.size() == 4);
    BOOST_CHECK(v3[0]==0);
    BOOST_CHECK(v3[1]==1);
    BOOST_CHECK(v3[2]==2);
    BOOST_CHECK(v3[3]==3);

    std::vector<long long> v4 = val.as_vector<long long>(); 
    BOOST_CHECK(v4.size() == 4);
    BOOST_CHECK(v4[0]==0);
    BOOST_CHECK(v4[1]==1);
    BOOST_CHECK(v4[2]==2);
    BOOST_CHECK(v4[3]==3);

    std::vector<unsigned long long> v5 = val.as_vector<unsigned long long>(); 
    BOOST_CHECK(v5.size() == 4);
    BOOST_CHECK(v5[0]==0);
    BOOST_CHECK(v5[1]==1);
    BOOST_CHECK(v5[2]==2);
    BOOST_CHECK(v5[3]==3);
}

BOOST_AUTO_TEST_CASE(test_as_vector_of_int_on_proxy)
{
    std::string s("[0,1,2,3]");
    json val = json::parse_string(s);
    json root;
    root["val"] = val;
    std::vector<int> v = root["val"].as_vector<int>();
    BOOST_CHECK(v.size() == 4);
    BOOST_CHECK(v[0]==0);
    BOOST_CHECK(v[1]==1);
    BOOST_CHECK(v[2]==2);
    BOOST_CHECK(v[3]==3);
}

