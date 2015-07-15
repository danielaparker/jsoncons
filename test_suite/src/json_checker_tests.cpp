// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include "jsoncons/json_parser.hpp"
#include "jsoncons/json.hpp"
#include "jsoncons/json_deserializer.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using jsoncons::json_deserializer;
using jsoncons::json;
using jsoncons::output_format;
using jsoncons::json_reader;
using jsoncons::pretty_print;
using std::string;
using jsoncons::json_exception;
using jsoncons::json_parse_exception;

using namespace boost::filesystem;

/*
using jsoncons::json_parser;
BOOST_AUTO_TEST_CASE(test_json_parser)
{
    std::string in_file = "input/JSON_checker/pass1.json";
    std::ifstream is(in_file);
    json_deserializer handler;
    json_parser jc(is,handler,20);
    jc.parse();

    std::cout << "JSON CHECKER 1" << std::endl;
    json val = std::move(handler.root());
    std::cout << val << std::endl;
}

BOOST_AUTO_TEST_CASE(test_json_parser2)
{
    std::string in_file = "input/JSON_checker/pass1.json";
    std::ifstream is(in_file);

    output_format format;
    format.escape_all_non_ascii(true);
    json_deserializer handler;
    json_reader jc(is,handler);
    jc.read();

    std::cout << "JSON CHECKER 1" << std::endl;
    json val = std::move(handler.root());
    std::cout << jsoncons::pretty_print(val,format) << std::endl;

}
*/

BOOST_AUTO_TEST_CASE(test_fail1)
{
    std::string in_file = "input/JSON_checker/fail1.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
        BOOST_CHECK_MESSAGE(0 != 0, in_file);
    }
    catch (...)
    {
        //std::cout << in_file << " " << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_fail2)
{
    std::string in_file = "input/JSON_checker/fail2.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
        BOOST_CHECK_MESSAGE(0 != 0, in_file);
    }
    catch (...)
    {
        //std::cout << in_file << " " << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_fail3)
{
    std::string in_file = "input/JSON_checker/fail3.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
        BOOST_CHECK_MESSAGE(0 != 0, in_file);
    }
    catch (...)
    {
        //std::cout << in_file << " " << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_fail4)
{
    std::string in_file = "input/JSON_checker/fail4.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
        BOOST_CHECK_MESSAGE(0 != 0, in_file);
    }
    catch (...)
    {
        //std::cout << in_file << " " << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_fail5)
{
    std::string in_file = "input/JSON_checker/fail5.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
        BOOST_CHECK_MESSAGE(0 != 0, in_file);
    }
    catch (...)
    {
        //std::cout << in_file << " " << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_fail6)
{
    std::string in_file = "input/JSON_checker/fail6.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
        BOOST_CHECK_MESSAGE(0 != 0, in_file);
    }
    catch (...)
    {
        //std::cout << in_file << " " << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_fail7)
{
    //std::string in_file = "input/JSON_checker/fail7.json";
    //std::ifstream is(in_file);

    //try
    //{
    //    json_deserializer handler;
    //    json_reader reader(is,handler);
    //    reader.read();
    //    BOOST_CHECK_MESSAGE(0 != 0, in_file);
    //}
    //catch (...)
    //{
        //std::cout << in_file << " " << e.what() << std::endl;
    //}
}

BOOST_AUTO_TEST_CASE(test_fail8)
{
    //std::string in_file = "input/JSON_checker/fail8.json";
    //std::ifstream is(in_file);

    //try
    //{
    //    json_deserializer handler;
    //    json_reader reader(is,handler);
    //    reader.read();
    //    BOOST_CHECK_MESSAGE(0 != 0, in_file);
    //}
    //catch (...)
    //{
    //    //std::cout << in_file << " " << e.what() << std::endl;
    //}
}

BOOST_AUTO_TEST_CASE(test_fail9)
{
    std::string in_file = "input/JSON_checker/fail9.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
        BOOST_CHECK_MESSAGE(0 != 0, in_file);
    }
    catch (...)
    {
        //std::cout << in_file << " " << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_fail10)
{
    //std::string in_file = "input/JSON_checker/fail10.json";
    //std::ifstream is(in_file);

    //try
    //{
    //    json_deserializer handler;
    //    json_reader reader(is,handler);
    //    reader.read();
    //    BOOST_CHECK_MESSAGE(0 != 0, in_file);
    //}
    //catch (...)
    //{
        //std::cout << in_file << " " << e.what() << std::endl;
    //}
}

BOOST_AUTO_TEST_CASE(test_fail11)
{
    std::string in_file = "input/JSON_checker/fail11.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
        BOOST_CHECK_MESSAGE(0 != 0, in_file);
    }
    catch (...)
    {
        //std::cout << in_file << " " << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_fail12)
{
    std::string in_file = "input/JSON_checker/fail12.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
        BOOST_CHECK_MESSAGE(0 != 0, in_file);
    }
    catch (...)
    {
        //std::cout << in_file << " " << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_fail13)
{
    std::string in_file = "input/JSON_checker/fail13.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
        BOOST_CHECK_MESSAGE(0 != 0, in_file);
    }
    catch (...)
    {
        //std::cout << in_file << " " << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_fail14)
{
    std::string in_file = "input/JSON_checker/fail14.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
        BOOST_CHECK_MESSAGE(0 != 0, in_file);
    }
    catch (...)
    {
        //std::cout << in_file << " " << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_fail15)
{
    std::string in_file = "input/JSON_checker/fail15.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
        BOOST_CHECK_MESSAGE(0 != 0, in_file);
    }
    catch (...)
    {
        //std::cout << in_file << " " << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_fail16)
{
    std::string in_file = "input/JSON_checker/fail16.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
        BOOST_CHECK_MESSAGE(0 != 0, in_file);
    }
    catch (...)
    {
        //std::cout << in_file << " " << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_fail17)
{
    std::string in_file = "input/JSON_checker/fail17.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        while (!reader.eof())
            reader.read();
        BOOST_CHECK_MESSAGE(0 != 0, in_file);
    }
    catch (...)
    {
        //std::cout << in_file << " " << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_fail18)
{
    // Not an issue

    //std::string in_file = "input/JSON_checker/fail18.json";
    //std::ifstream is(in_file);

    //try
    //{
    //    json_deserializer handler;
    //    json_reader reader(is,handler);
    //    reader.read();
    //    BOOST_CHECK_MESSAGE(0 != 0, in_file);
    //}
    //catch (...)
    //{
    //    //std::cout << in_file << " " << e.what() << std::endl;
    //}
}

BOOST_AUTO_TEST_CASE(test_fail19)
{
    std::string in_file = "input/JSON_checker/fail19.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
        BOOST_CHECK_MESSAGE(0 != 0, in_file);
    }
    catch (...)
    {
        //std::cout << in_file << " " << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_fail20)
{
    std::string in_file = "input/JSON_checker/fail20.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
        BOOST_CHECK_MESSAGE(0 != 0, in_file);
    }
    catch (...)
    {
        //std::cout << in_file << " " << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_fail21)
{
    std::string in_file = "input/JSON_checker/fail21.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
        BOOST_CHECK_MESSAGE(0 != 0, in_file);
    }
    catch (...)
    {
        //std::cout << in_file << " " << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_fail22)
{
    std::string in_file = "input/JSON_checker/fail22.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
        BOOST_CHECK_MESSAGE(0 != 0, in_file);
    }
    catch (...)
    {
        //std::cout << in_file << " " << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_fail23)
{
    std::string in_file = "input/JSON_checker/fail23.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
        BOOST_CHECK_MESSAGE(0 != 0, in_file);
    }
    catch (...)
    {
        //std::cout << in_file << " " << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_fail24)
{
    std::string in_file = "input/JSON_checker/fail24.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
        BOOST_CHECK_MESSAGE(0 != 0, in_file);
    }
    catch (...)
    {
        //std::cout << in_file << " " << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_fail25)
{
    std::string in_file = "input/JSON_checker/fail25.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
        BOOST_CHECK_MESSAGE(0 != 0, in_file);
    }
    catch (...)
    {
        //std::cout << in_file << " " << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_fail26)
{
    std::string in_file = "input/JSON_checker/fail26.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
        BOOST_CHECK_MESSAGE(0 != 0, in_file);
    }
    catch (...)
    {
        //std::cout << in_file << " " << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_fail27)
{
    std::string in_file = "input/JSON_checker/fail27.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
        BOOST_CHECK_MESSAGE(0 != 0, in_file);
    }
    catch (...)
    {
        //std::cout << in_file << " " << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_fail28)
{
    std::string in_file = "input/JSON_checker/fail28.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
        BOOST_CHECK_MESSAGE(0 != 0, in_file);
    }
    catch (...)
    {
        //std::cout << in_file << " " << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_fail29)
{
    std::string in_file = "input/JSON_checker/fail29.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
        BOOST_CHECK_MESSAGE(0 != 0, in_file);
    }
    catch (...)
    {
        //std::cout << in_file << " " << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_fail30)
{
    std::string in_file = "input/JSON_checker/fail30.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
        BOOST_CHECK_MESSAGE(0 != 0, in_file);
    }
    catch (...)
    {
        //std::cout << in_file << " " << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_fail31)
{
    std::string in_file = "input/JSON_checker/fail31.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
        BOOST_CHECK_MESSAGE(0 != 0, in_file);
    }
    catch (...)
    {
        //std::cout << in_file << " " << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_fail32)
{
    std::string in_file = "input/JSON_checker/fail32.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
        BOOST_CHECK_MESSAGE(0 != 0, in_file);
    }
    catch (...)
    {
        //std::cout << in_file << " " << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_fail33)
{
    std::string in_file = "input/JSON_checker/fail33.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
        BOOST_CHECK_MESSAGE(0 != 0, in_file);
    }
    catch (...)
    {
        //std::cout << in_file << " " << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_pass1)
{
    std::string in_file = "input/JSON_checker/pass1.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
    }
    catch (const json_parse_exception& e)
    {
        std::cout << in_file << " " << e.what() << std::endl;
        throw;
    }
}

BOOST_AUTO_TEST_CASE(test_pass2)
{
    std::string in_file = "input/JSON_checker/pass2.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
    }
    catch (const json_parse_exception& e)
    {
        std::cout << in_file << " " << e.what() << std::endl;
        throw;
    }
}

BOOST_AUTO_TEST_CASE(test_pass3)
{
    std::string in_file = "input/JSON_checker/pass3.json";
    std::ifstream is(in_file);

    try
    {
        json_deserializer handler;
        json_reader reader(is,handler);
        reader.read();
    }
    catch (const json_parse_exception& e)
    {
        std::cout << in_file << " " << e.what() << std::endl;
        throw;
    }
}

