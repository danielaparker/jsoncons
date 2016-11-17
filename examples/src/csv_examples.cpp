// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <string>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/csv/csv_reader.hpp>
#include <jsoncons_ext/csv/csv_serializer.hpp>

using namespace jsoncons;
using namespace jsoncons::csv;

void read_csv_file1()
{
    std::string text = R"(employee-no,employee-name,dept,salary
00000001,\"Smith,Matthew\",sales,150000.00
00000002,\"Brown,Sarah\",sales,89000.00
)";

    std::istringstream is(text);

    json_decoder<json> encoder;

    csv_parameters params;
    params.assume_header(true)
          .column_types({ "string","string","string","float" });
    csv_reader reader(is,encoder,params);
    reader.read();
    json val = encoder.get_result();

    std::cout << pretty_print(val) << std::endl;
}

void read_write_csv_tasks()
{
    std::ifstream is("input/tasks.csv");

    json_decoder<ojson> encoder;
    csv_parameters params;
    params.assume_header(true)
          .trim(true)
          .ignore_empty_values(true) 
          .column_types({"integer","string","string","string"});
    csv_reader reader(is,encoder,params);
    reader.read();
    ojson tasks = encoder.get_result();

    std::cout << "(1)\n";
    std::cout << pretty_print(tasks) << "\n\n";

    std::cout << "(2)\n";
    csv_serializer serializer(std::cout);
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
    read_write_csv_tasks();
    serialize_array_of_arrays_to_comma_delimited();
    serialize_to_tab_delimited_file();
    std::cout << std::endl;
}

