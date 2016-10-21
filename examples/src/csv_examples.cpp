// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <string>
#include "jsoncons/json.hpp"
#include "jsoncons_ext/csv/csv_reader.hpp"
#include "jsoncons_ext/csv/csv_serializer.hpp"

using namespace jsoncons;
using namespace jsoncons::csv;

void read_csv_file1()
{
    std::string text = R"(employee-no,employee-name,dept,salary
00000001,Smith,Matthew,sales,150000.00
00000002,Brown,Sarah,sales,89000.00
)";

    std::istringstream is(text);

    json_deserializer handler;

    csv_parameters params;
    params.assume_header(true)
          .column_types({ "string","string","string","float" });
    csv_reader reader(is,handler,params);
    reader.read();
    json val = handler.get_result();

    std::cout << pretty_print(val) << std::endl;
}

void read_csv_tasks_file()
{
    std::ifstream is("input/tasks.csv");

    ojson_deserializer handler;

    csv_parameters params;
    params.assume_header(true)
          .trim(true)
          .ignore_empty_values(true) 
          .column_types({"integer","string","string","string"});
    csv_reader reader(is,handler,params);
    reader.read();
    ojson val = handler.get_result();

    std::ofstream os("output/tasks.json");
    os << pretty_print(val);
}

void write_csv_tasks_file()
{
    std::ifstream is("output/tasks.json");

    ojson tasks;
    is >> tasks;

    std::ofstream os("output/tasks.csv");
    csv_serializer serializer(os);
    tasks.write(serializer);
}

void serialize_array_of_arrays_to_comma_delimited()
{
    std::string in_file = "input/countries.json";
    std::ifstream is(in_file);

    json countries;
    is >> countries;

    csv_serializer serializer(std::cout);
    countries.write(serializer);
}

void serialize_to_tab_delimited_file()
{
    std::string in_file = "input/employees.json";
    std::ifstream is(in_file);

    json employees;
    is >> employees;

    csv_parameters params;
    params.field_delimiter('\t');
    csv_serializer serializer(std::cout,params);

    employees.write(serializer);
}

void csv_examples()
{
    std::cout << "\nCSV examples\n\n";
    read_csv_file1();
    read_csv_tasks_file();
    write_csv_tasks_file();
    serialize_array_of_arrays_to_comma_delimited();
    serialize_to_tab_delimited_file();
    std::cout << std::endl;
}

