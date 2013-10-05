// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include "jsoncons/json.hpp"
#include "jsoncons/json_serializer.hpp"
#include "jsoncons/csv_reader.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using jsoncons::json_deserializer;
using jsoncons::output_format;
using jsoncons::json;
using jsoncons::csv_reader;
using jsoncons::pretty_print;
using std::string;

BOOST_AUTO_TEST_CASE(test_comma_delimited_file)
{
    std::cout << "###test_csv_reader" << std::endl;

    std::string in_file = "../../../examples/countries.csv";
    std::ifstream is(in_file);

    json_deserializer handler;
    json params(json::an_object);
    params["field_separator"] = ",";

    csv_reader reader(params,is,handler);
    reader.read();

    std::cout << pretty_print(handler.root()) << std::endl;
}

BOOST_AUTO_TEST_CASE(test_tab_delimited_file)
{
    std::cout << "###test_csv_reader" << std::endl;

    std::string in_file = "../../../examples/employees.txt";
    std::ifstream is(in_file);

    json_deserializer handler;
    json params(json::an_object);
    params["field_separator"] = "\t";

    csv_reader reader(params,is,handler);
    reader.read();

    std::cout << pretty_print(handler.root()) << std::endl;
}

