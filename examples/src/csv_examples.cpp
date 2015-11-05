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

void read_csv_file1()
{
    string text = "employee-no,employee-name,dept,salary\n00000001,\"Smith,Matthew\",sales,150000.00\n00000002,\"Brown,Sarah\",sales,89000.00";

    std::istringstream is(text);

    json_deserializer handler;

    csv_parameters params;
    params.assume_header(true);
    params.data_types("string,string,string,float");
    csv_reader reader(is,handler,params);
    reader.read();
    json val = handler.get_result();

    std::cout << pretty_print(val) << std::endl;
}

void read_csv_file2()
{
    string text = 
"project_id, task_name, task_start, task_finish\n"
"4001,task1,01/01/2003,01/31/2003\n"
"4001,task2,02/01/2003,02/28/2003\n"
"4001,task3,03/01/2003,03/31/2003\n"
"4002,task1,04/01/2003,04/30/2003\n"
"4002,task2,05/01/2003,";

    std::istringstream is(text);

    json_deserializer handler;

    csv_parameters params;
    params.assume_header(true);
    params.trim(true);
    params.ignore_empty_values(true);
    params.data_types("integer,string,string,string");
    csv_reader reader(is,handler,params);
    reader.read();
    json val = handler.get_result();

    std::cout << pretty_print(val) << std::endl;
}

void read_csv_file()
{
    read_csv_file1();
    read_csv_file2();
}

void write_csv_file()
{
}

