// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <string>
#include "jsoncons/json.hpp"
#include "jsoncons_ext/csv/csv_reader.hpp"

using jsoncons::json;
using jsoncons::pretty_print;
using jsoncons::output_format;
using jsoncons::json_deserializer;
using jsoncons::csv::csv_reader;
using jsoncons::csv::csv_parameters;
using std::string;

void read_csv_file()
{
    string text = "employee-no,employee-name,dept,salary\n00000001,\"Smith,Matthew\",sales,150000.00\n00000002,\"Brown,Sarah\",sales,89000.00";

    std::istringstream is(text);

    json_deserializer handler;

    csv_parameters params;
    params.assume_header(true);
    params.data_types("string,string,string,float");
    csv_reader reader(is,handler,params);
    reader.read();
    json val = std::move(handler.root());

    std::cout << pretty_print(val) << std::endl;
}

void write_csv_file()
{
}

