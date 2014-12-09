// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include "jsoncons_ext/csv/csv_reader.hpp"
#include "jsoncons_ext/csv/csv_serializer.hpp"
#include "jsoncons/json_reader.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using jsoncons::json_deserializer;
using jsoncons_ext::csv::csv_serializer;
using jsoncons::json;
using jsoncons_ext::csv::csv_reader;
using jsoncons::json_reader;
using jsoncons::pretty_print;
using std::string;

BOOST_AUTO_TEST_CASE(read_comma_delimited_file)
{
    std::string in_file = "input/countries.csv";
    std::ifstream is(in_file);

    json_deserializer handler;

    csv_reader reader(is,handler);
    reader.read();
    json countries = std::move(handler.root());

    std::cout << pretty_print(countries) << std::endl;
}

BOOST_AUTO_TEST_CASE(serialize_comma_delimited_file)
{
    std::string in_file = "input/countries.json";
    std::ifstream is(in_file);

    json_deserializer handler;
    json_reader reader(is,handler);
    reader.read();
    json countries = std::move(handler.root());

    csv_serializer serializer(std::cout);

    countries.to_stream(serializer);
}

BOOST_AUTO_TEST_CASE(test_tab_delimited_file)
{
    std::string in_file = "input/employees.txt";
    std::ifstream is(in_file);

    json_deserializer handler;
    json params;
    params["field_delimiter"] = "\t";
    params["has_header"] = true;

    csv_reader reader(is,handler,params);
    reader.read();
    json employees = std::move(handler.root());

    std::cout << pretty_print(employees) << std::endl;
}

BOOST_AUTO_TEST_CASE(serialize_tab_delimited_file)
{
    std::string in_file = "input/employees.json";
    std::ifstream is(in_file);

    json_deserializer handler;
    json params;
    params["field_delimiter"] = "\t";

    json_reader reader(is,handler);
    reader.read();
    json employees = std::move(handler.root());

    csv_serializer serializer(std::cout,params);

    employees.to_stream(serializer);
}
