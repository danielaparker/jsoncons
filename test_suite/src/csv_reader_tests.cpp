// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include "jsoncons_ext/csv/csv_reader.hpp"
#include "jsoncons/json_reader.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using jsoncons::json_deserializer;
using jsoncons::json;
using jsoncons_ext::csv::csv_reader;
using jsoncons::json_reader;
using jsoncons::pretty_print;
using std::string;

BOOST_AUTO_TEST_CASE(test_comma_delimited_file)
{
    std::cout << "###test_csv_reader" << std::endl;

    std::string in_file = "input/countries.csv";
    std::ifstream is(in_file);

    json_deserializer handler;

    csv_reader reader(is,handler);
    reader.read();
    json countries = std::move(handler.root());

    std::cout << pretty_print(countries) << std::endl;
} 

BOOST_AUTO_TEST_CASE(test_tab_delimited_file)
{
    std::cout << "###test_csv_reader" << std::endl;

    std::string in_file = "input/employees.txt";
    std::ifstream is(in_file);

    json_deserializer handler;
    json params(json::an_object);
    params["field_delimiter"] = "\t";
    params["has_header"] = true;

    csv_reader reader(is,handler,params);
    reader.read();
    json employees = std::move(handler.root());

    std::cout << pretty_print(employees) << std::endl;
}

