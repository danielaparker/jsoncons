// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include <boost/numeric/ublas/matrix.hpp>
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
using jsoncons::basic_json_reader;
using std::string;

BOOST_AUTO_TEST_CASE(test_construction_from_string)
{
    std::string input = "{\"first_name\":\"Jane\",\"last_name\":\"Roe\",\"events_attended\":10}";

    json val = json::parse_string(input);

    std::cout << val << std::endl;
}

BOOST_AUTO_TEST_CASE(test_construction_from_file)
{
    json val = json::parse_file("../../../examples/members.json");

    //output_format format(true);
    //val.to_stream(std::cout,format);

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
    json obj(json::an_object);
    obj["first_name"] = "Jane";
    obj["last_name"] = "Roe";
    obj["events_attended"] = 10;
    obj["accept_waiver_of_liability"] = true;

    // An array value with four elements
    json arr(json::an_array);
    arr.push_back(null_val);
    arr.push_back(flag);
    arr.push_back(number);
    arr.push_back(obj);

    output_format format(true);
    arr.to_stream(std::cout,format);
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
    json obj(json::an_object);
    obj["first_name"] = "Jane";
    obj["last_name"] = "Roe";
    obj["events_attended"] = 10;
    obj["accept_waiver_of_liability"] = true;

    std::string first_name = obj["first_name"].as_string();
    std::string last_name = obj.get("last_name").as_string();
    int events_attended = obj["events_attended"].as_int();
    bool accept_waiver_of_liability = obj["accept_waiver_of_liability"].as_bool();

    std::cout << first_name << " " << last_name << ", " << events_attended << ", " << accept_waiver_of_liability << std::endl;

}

BOOST_AUTO_TEST_CASE(test_value_not_found_and_defaults)
{
    json obj(json::an_object);
    obj["first_name"] = "Jane";
    obj["last_name"] = "Roe";

    try
    {
        std::string experience = obj["outdoor_experience"].as_string();
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }

    std::string experience = obj.has_member("outdoor_experience") ? obj["outdoor_experience"].as_string() : "";

    bool first_aid_certification = obj.get("first_aid_certification",false).as_bool();

    std::cout << "experience=" << experience << ", first_aid_certification=" << first_aid_certification << std::endl;
}

BOOST_AUTO_TEST_CASE(test_another_object_iterator)
{
    json obj(json::an_object);
    obj["first_name"] = "Jane";
    obj["last_name"] = "Roe";
    obj["events_attended"] = 10;
    obj["accept_waiver_of_liability"] = true;

    for (auto it = obj.begin_members(); it != obj.end_members(); ++it)
    {
        std::cout << "name=" << it->first << ", value=" << it->second.as_string() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_another_array_iterator)
{
    json arr(json::an_array);
    arr.push_back("Montreal");
    arr.push_back("Toronto");
    arr.push_back("Ottawa");
    arr.push_back("Vancouver");

    for (auto it = arr.begin_elements(); it != arr.end_elements(); ++it)
    {
        std::cout << it->as_string() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_integer_limits)
{
    const long long max_value = std::numeric_limits<long long>::max();

    const unsigned long long max_uvalue = std::numeric_limits<unsigned long long>::max();
    {
        std::ostringstream os;

        os << max_value;

        os << "{\"max_longlong\":-" << max_value << "}";
        json val = json::parse_string(os.str());
        std::cout << val << std::endl;
        BOOST_CHECK(val["max_longlong"].is_longlong());
    }
    {
        std::ostringstream os;

        os << max_value;

        os << "{\"max_longlong_overflow\":-" << max_value << "0}";
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
    std::cout << "size map=" << sizeof(std::vector<std::pair<std::string,json>>) << std::endl;
}

namespace jsoncons
{
    template <typename Char>
    std::basic_ostream<Char>& operator<<(std::basic_ostream<Char>& os, 
                                         const userdata<Char,matrix<double>>& o)
    {
        const matrix<double>& A = o.value_;

        os << '[';
        for (size_t i = 0; i < A.size1(); ++i)
        {
            if (i > 0)
            {
                os << ',';
            }
            os << '[';
            for (size_t j = 0; j < A.size2(); ++j)
            {
                if (j > 0)
                {
                    os << ',';
                }
                os << A(i,j);
            }
            os << ']';
        }
        os << ']';
        return os;
    }
}

BOOST_AUTO_TEST_CASE(test_userdata)
{
    std::cout << "Check 1" << std::endl;
    json obj(json::an_object);
    matrix<double> A(2,2);
    A(0,0) = 1;
    A(0,1) = 2;
    A(1,0) = 3;
    A(1,1) = 4;

    std::cout << A << std::endl;

    obj.set_userdata("mydata",A);

    obj.serialize(json_serializer(std::cout));
    std::cout << std::endl;

    matrix<double> B = obj["mydata"].as_userdata<matrix<double>>();

    for (size_t i = 0; i < B.size1(); ++i)
    {
        for (size_t j = 0; j < B.size2(); ++j)
        {
            if (j > 0)
            {
                std::cout << ',';
            }
            std::cout << B(i,j);
        }
        std::cout << '\n';
    }
}

