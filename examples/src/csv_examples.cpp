// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <string>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/csv/csv_reader.hpp>
#include <jsoncons_ext/csv/csv_serializer.hpp>

using namespace jsoncons;
using namespace jsoncons::csv;

void mapping_types()
{
    const std::string s = R"(Date,1Y,2Y,3Y,5Y
2017-01-09,0.0062,0.0075,0.0083,0.011
2017-01-08,0.0063,0.0076,0.0084,0.0112
2017-01-08,0.0063,0.0076,0.0084,0.0112
)";

    csv_serializing_options options;
    options.assume_header(true)
           .column_types("string,float,float,float,float");

    // mapping_type::n_objects
    options.mapping(mapping_type::n_objects);
    ojson j1 = decode_csv<ojson>(s,options);
    std::cout << "\n(1)\n"<< pretty_print(j1) << "\n";

    // mapping_type::n_rows
    options.mapping(mapping_type::n_rows);
    ojson j2 = decode_csv<ojson>(s,options);
    std::cout << "\n(2)\n"<< pretty_print(j2) << "\n";

    // mapping_type::m_columns
    options.mapping(mapping_type::m_columns);
    ojson j3 = decode_csv<ojson>(s,options);
    std::cout << "\n(3)\n" << pretty_print(j3) << "\n";
}

void csv_decode_without_type_inference()
{
    std::string s = R"(employee-no,employee-name,dept,salary
00000001,"Smith,Matthew",sales,150000.00
00000002,"Brown,Sarah",sales,89000.00
)";

    csv_serializing_options options;
    options.assume_header(true)
           .infer_types(false);
    ojson j = decode_csv<ojson>(s,options);

    std::cout << pretty_print(j) << std::endl;
}

void read_write_csv_tasks()
{
    std::ifstream is("./input/tasks.csv");

    json_decoder<ojson> decoder;
    csv_serializing_options options;
    options.assume_header(true)
           .trim(true)
           .ignore_empty_values(true) 
           .column_types("integer,string,string,string");
    csv_reader reader(is,decoder,options);
    reader.read();
    ojson tasks = decoder.get_result();

    std::cout << "(1)\n";
    std::cout << pretty_print(tasks) << "\n\n";

    std::cout << "(2)\n";
    csv_serializer serializer(std::cout);
    tasks.dump(serializer);
}

void serialize_array_of_arrays_to_comma_delimited()
{
    std::string in_file = "./input/countries.json";
    std::ifstream is(in_file);

    json countries;
    is >> countries;

    csv_serializer serializer(std::cout);
    countries.dump(serializer);
}

void serialize_to_tab_delimited_file()
{
    std::string in_file = "./input/employees.json";
    std::ifstream is(in_file);

    json employees;
    is >> employees;

    csv_serializing_options options;
    options.field_delimiter('\t');
    csv_serializer serializer(std::cout,options);

    employees.dump(serializer);
}

void serialize_books_to_csv_file()
{
    const json books = json::parse(R"(
    [
        {
            "title" : "Kafka on the Shore",
            "author" : "Haruki Murakami",
            "price" : 25.17
        },
        {
            "title" : "Women: A Novel",
            "author" : "Charles Bukowski",
            "price" : 12.00
        },
        {
            "title" : "Cutter's Way",
            "author" : "Ivan Passer"
        }
    ]
    )");

    csv_serializer serializer(std::cout);

    books.dump(serializer);
}

void serialize_books_to_csv_file_with_reorder()
{
    const json books = json::parse(R"(
    [
        {
            "title" : "Kafka on the Shore",
            "author" : "Haruki Murakami",
            "price" : 25.17
        },
        {
            "title" : "Women: A Novel",
            "author" : "Charles Bukowski",
            "price" : 12.00
        },
        {
            "title" : "Cutter's Way",
            "author" : "Ivan Passer"
        }
    ]
    )");

    csv_serializing_options options;
    options.column_names("author,title,price");

    csv_serializer serializer(std::cout, options);

    books.dump(serializer);
}

using jsoncons::ojson;
using jsoncons::csv::csv_serializing_options;
using jsoncons::csv::csv_reader;
using jsoncons::json_decoder;

void last_column_repeats()
{
    const std::string bond_yields = R"(Date,Yield
2017-01-09,0.0062,0.0075,0.0083,0.011,0.012
2017-01-08,0.0063,0.0076,0.0084,0.0112,0.013
2017-01-08,0.0063,0.0076,0.0084,0.0112,0.014
)";

    json_decoder<ojson> decoder1;
    csv_serializing_options options1;
    options1.header_lines(1);
    options1.column_types("string,float*");
    std::istringstream is1(bond_yields);
    csv_reader reader1(is1, decoder1, options1);
    reader1.read();
    ojson val1 = decoder1.get_result();
    std::cout << "\n(1)\n" << pretty_print(val1) << "\n";

    json_decoder<ojson> decoder2;
    csv_serializing_options options2;
    options2.assume_header(true);
    options2.column_types("string,[float*]");
    std::istringstream is2(bond_yields);
    csv_reader reader2(is2, decoder2, options2);
    reader2.read();
    ojson val2 = decoder2.get_result();
    std::cout << "\n(2)\n" << pretty_print(val2) << "\n";
}

void last_two_columns_repeat()
{
    const std::string holidays = R"(1,CAD,2,UK,3,EUR,4,US
38719,2-Jan-2006,40179,1-Jan-2010,38719,2-Jan-2006,39448,1-Jan-2008
38733,16-Jan-2006,40270,2-Apr-2010,38733,16-Jan-2006,39468,21-Jan-2008
)";

    // array of arrays
    json_decoder<ojson> decoder1;
    csv_serializing_options options1;
    options1.column_types("[integer,string]*");
    std::istringstream is1(holidays);
    csv_reader reader1(is1, decoder1, options1);
    reader1.read();
    ojson val1 = decoder1.get_result();
    std::cout << "(1)\n" << pretty_print(val1) << "\n";

    // array of objects
    json_decoder<ojson> decoder2;
    csv_serializing_options options2;
    options2.header_lines(1);
    options2.column_names("CAD,UK,EUR,US");
    options2.column_types("[integer,string]*");
    std::istringstream is2(holidays);
    csv_reader reader2(is2, decoder2, options2);
    reader2.read();
    ojson val2 = decoder2.get_result();
    std::cout << "(2)\n" << pretty_print(val2) << "\n";
}

void decode_csv_string()
{
    std::string s = R"(employee-no,employee-name,dept,salary
00000001,\"Smith,Matthew\",sales,150000.00
00000002,\"Brown,Sarah\",sales,89000.00
)";

    csv_serializing_options options;
    options.assume_header(true)
           .column_types("string,string,string,float");
    json j = decode_csv<json>(s,options);

    std::cout << pretty_print(j) << std::endl;
}

void decode_csv_stream()
{
    const std::string bond_yields = R"(Date,1Y,2Y,3Y,5Y
2017-01-09,0.0062,0.0075,0.0083,0.011
2017-01-08,0.0063,0.0076,0.0084,0.0112
2017-01-08,0.0063,0.0076,0.0084,0.0112
)";

    csv_serializing_options options;
    options.assume_header(true)
           .column_types("string,float,float,float,float");

    std::istringstream is(bond_yields);

    ojson j = decode_csv<ojson>(is,options);

    std::cout << pretty_print(j) << std::endl;
}

void encode_csv_file_from_books()
{
    const json books = json::parse(R"(
    [
        {
            "title" : "Kafka on the Shore",
            "author" : "Haruki Murakami",
            "price" : 25.17
        },
        {
            "title" : "Women: A Novel",
            "author" : "Charles Bukowski",
            "price" : 12.00
        },
        {
            "title" : "Cutter's Way",
            "author" : "Ivan Passer"
        }
    ]
    )");

    encode_csv(books, std::cout);
}

void decode_encode_csv_tasks()
{
    std::ifstream is("./input/tasks.csv");

    csv_serializing_options options;
    options.assume_header(true)
           .trim(true)
           .ignore_empty_values(true) 
           .column_types("integer,string,string,string");
    ojson tasks = decode_csv<ojson>(is, options);

    std::cout << "(1)\n" << pretty_print(tasks) << "\n\n";

    std::cout << "(2)\n";
    encode_csv(tasks, std::cout);
}

void csv_parser_type_inference()
{
    csv_serializing_options options;
    options.assume_header(true)
           .mapping(mapping_type::n_objects);

    std::ifstream is1("input/sales.csv");
    ojson j1 = decode_csv<ojson>(is1,options);
    std::cout << "\n(1)\n"<< pretty_print(j1) << "\n";

    options.mapping(mapping_type::n_rows);
    std::ifstream is2("input/sales.csv");
    ojson j2 = decode_csv<ojson>(is2,options);
    std::cout << "\n(2)\n"<< pretty_print(j2) << "\n";

    options.mapping(mapping_type::m_columns);
    std::ifstream is3("input/sales.csv");
    ojson j3 = decode_csv<ojson>(is3,options);
    std::cout << "\n(3)\n"<< pretty_print(j3) << "\n";
}
 
// Examples with subfields
 

void decode_csv_with_subfields()
{
    const std::string s = R"(calculationPeriodCenters,paymentCenters,resetCenters
NY;LON,TOR,LON
NY,LON,TOR;LON
"NY";"LON","TOR","LON"
"NY","LON","TOR";"LON"
)";
    json_serializing_options print_options;
    print_options.array_array_split_lines(line_split_kind::same_line);

    csv_serializing_options options1;
    options1.assume_header(true)
           .subfield_delimiter(';');

    json j1 = decode_csv<json>(s,options1);
    std::cout << "(1)\n" << pretty_print(j1,print_options) << "\n\n";

    csv_serializing_options options2;
    options2.mapping(mapping_type::n_rows)
           .subfield_delimiter(';');

    json j2 = decode_csv<json>(s,options2);
    std::cout << "(2)\n" << pretty_print(j2,print_options) << "\n\n";

    csv_serializing_options options3;
    options3.assume_header(true)
           .mapping(mapping_type::m_columns)
           .subfield_delimiter(';');

    json j3 = decode_csv<json>(s,options3);
    std::cout << "(3)\n" << pretty_print(j3,print_options) << "\n\n";
}

void csv_examples()
{
    std::cout << "\nCSV examples\n\n";
    read_write_csv_tasks();
    serialize_to_tab_delimited_file();
    serialize_array_of_arrays_to_comma_delimited();
    serialize_books_to_csv_file();
    serialize_books_to_csv_file_with_reorder();
    last_column_repeats();
    last_two_columns_repeat();
    decode_csv_string();
    decode_csv_stream();
    encode_csv_file_from_books();
    decode_encode_csv_tasks();

    csv_decode_without_type_inference();
    mapping_types();
    csv_parser_type_inference();

    decode_csv_with_subfields();
    std::cout << std::endl;
}

