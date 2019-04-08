// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <string>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/csv/csv_reader.hpp>
#include <jsoncons_ext/csv/csv_encoder.hpp>
#include <fstream>

namespace jc = jsoncons;
namespace csv = jsoncons::csv;

void mapping_types()
{
    const std::string s = R"(Date,1Y,2Y,3Y,5Y
2017-01-09,0.0062,0.0075,0.0083,0.011
2017-01-08,0.0063,0.0076,0.0084,0.0112
2017-01-08,0.0063,0.0076,0.0084,0.0112
)";

    csv::csv_options options;
    options.assume_header(true)
           .column_types("string,float,float,float,float");

    // csv::mapping_type::n_objects
    options.mapping(csv::mapping_type::n_objects);
    jc::ojson j1 = csv::decode_csv<jc::ojson>(s,options);
    std::cout << "\n(1)\n"<< pretty_print(j1) << "\n";

    // csv::mapping_type::n_rows
    options.mapping(csv::mapping_type::n_rows);
    jc::ojson j2 = csv::decode_csv<jc::ojson>(s,options);
    std::cout << "\n(2)\n"<< pretty_print(j2) << "\n";

    // csv::mapping_type::m_columns
    options.mapping(csv::mapping_type::m_columns);
    jc::ojson j3 = csv::decode_csv<jc::ojson>(s,options);
    std::cout << "\n(3)\n" << pretty_print(j3) << "\n";
}

void mapping_types2()
{
    const std::string s = R"(Date,1Y,2Y,3Y,5Y
2017-01-09,0.0062,0.0075,0.0083,0.011
2017-01-08,0.0063,0.0076,0.0084,0.0112
2017-01-08,0.0063,0.0076,0.0084,0.0112
)";

    csv::csv_options options;
    options.header_lines(1)
           .mapping(csv::mapping_type::n_rows);

    typedef std::vector<std::tuple<std::string,double,double,double,double>> table_type;

    table_type table = csv::decode_csv<table_type>(s,options);

    for (const auto& row : table)
    {
        std::cout << std::get<0>(row) << "," 
                  << std::get<1>(row) << "," 
                  << std::get<2>(row) << "," 
                  << std::get<3>(row) << "," 
                  << std::get<4>(row) << "\n";
    }
}

void csv_decode_without_type_inference()
{
    std::string s = R"(employee-no,employee-name,dept,salary
00000001,"Smith,Matthew",sales,150000.00
00000002,"Brown,Sarah",sales,89000.00
)";

    csv::csv_options options;
    options.assume_header(true)
           .infer_types(false);
    jc::ojson j = csv::decode_csv<jc::ojson>(s,options);

    std::cout << pretty_print(j) << std::endl;
}

void read_write_csv_tasks()
{
    std::ifstream is("./input/tasks.csv");

    jc::json_decoder<jc::ojson> decoder;
    csv::csv_options options;
    options.assume_header(true)
           .trim(true)
           .ignore_empty_values(true) 
           .column_types("integer,string,string,string");
    csv::csv_reader reader(is,decoder,options);
    reader.read();
    jc::ojson tasks = decoder.get_result();

    std::cout << "(1)\n";
    std::cout << pretty_print(tasks) << "\n\n";

    std::cout << "(2)\n";
    csv::csv_encoder encoder(std::cout);
    tasks.dump(encoder);
}

void serialize_array_of_arrays_to_comma_delimited()
{
    std::string in_file = "./input/countries.json";
    std::ifstream is(in_file);

    jc::json countries;
    is >> countries;

    csv::csv_encoder encoder(std::cout);
    countries.dump(encoder);
}

void serialize_to_tab_delimited_file()
{
    std::string in_file = "./input/employees.json";
    std::ifstream is(in_file);

    jc::json employees;
    is >> employees;

    csv::csv_options options;
    options.field_delimiter('\t');
    csv::csv_encoder encoder(std::cout,options);

    employees.dump(encoder);
}

void serialize_books_to_csv_file()
{
    const jc::json books = jc::json::parse(R"(
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

    csv::csv_encoder encoder(std::cout);

    books.dump(encoder);
}

void serialize_books_to_csv_file_with_reorder()
{
    const jc::json books = jc::json::parse(R"(
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

    csv::csv_options options;
    options.column_names("author,title,price");

    csv::csv_encoder encoder(std::cout, options);

    books.dump(encoder);
}

void last_column_repeats()
{
    const std::string bond_yields = R"(Date,Yield
2017-01-09,0.0062,0.0075,0.0083,0.011,0.012
2017-01-08,0.0063,0.0076,0.0084,0.0112,0.013
2017-01-08,0.0063,0.0076,0.0084,0.0112,0.014
)";

    jc::json_decoder<jc::ojson> decoder1;
    csv::csv_options options1;
    options1.header_lines(1);
    options1.column_types("string,float*");
    std::istringstream is1(bond_yields);
    csv::csv_reader reader1(is1, decoder1, options1);
    reader1.read();
    jc::ojson val1 = decoder1.get_result();
    std::cout << "\n(1)\n" << pretty_print(val1) << "\n";

    jc::json_decoder<jc::ojson> decoder2;
    csv::csv_options options2;
    options2.assume_header(true);
    options2.column_types("string,[float*]");
    std::istringstream is2(bond_yields);
    csv::csv_reader reader2(is2, decoder2, options2);
    reader2.read();
    jc::ojson val2 = decoder2.get_result();
    std::cout << "\n(2)\n" << pretty_print(val2) << "\n";
}

void last_two_columns_repeat()
{
    const std::string holidays = R"(1,CAD,2,UK,3,EUR,4,US
38719,2-Jan-2006,40179,1-Jan-2010,38719,2-Jan-2006,39448,1-Jan-2008
38733,16-Jan-2006,40270,2-Apr-2010,38733,16-Jan-2006,39468,21-Jan-2008
)";

    // array of arrays
    jc::json_decoder<jc::ojson> decoder1;
    csv::csv_options options1;
    options1.column_types("[integer,string]*");
    std::istringstream is1(holidays);
    csv::csv_reader reader1(is1, decoder1, options1);
    reader1.read();
    jc::ojson val1 = decoder1.get_result();
    std::cout << "(1)\n" << pretty_print(val1) << "\n";

    // array of objects
    jc::json_decoder<jc::ojson> decoder2;
    csv::csv_options options2;
    options2.header_lines(1);
    options2.column_names("CAD,UK,EUR,US");
    options2.column_types("[integer,string]*");
    std::istringstream is2(holidays);
    csv::csv_reader reader2(is2, decoder2, options2);
    reader2.read();
    jc::ojson val2 = decoder2.get_result();
    std::cout << "(2)\n" << pretty_print(val2) << "\n";
}

void decode_csv_string()
{
    std::string s = R"(employee-no,employee-name,dept,salary
00000001,\"Smith,Matthew\",sales,150000.00
00000002,\"Brown,Sarah\",sales,89000.00
)";

    csv::csv_options options;
    options.assume_header(true)
           .column_types("string,string,string,float");
    jc::json j = csv::decode_csv<jc::json>(s,options);

    std::cout << pretty_print(j) << std::endl;
}

void decode_csv_stream()
{
    const std::string bond_yields = R"(Date,1Y,2Y,3Y,5Y
2017-01-09,0.0062,0.0075,0.0083,0.011
2017-01-08,0.0063,0.0076,0.0084,0.0112
2017-01-08,0.0063,0.0076,0.0084,0.0112
)";

    csv::csv_options options;
    options.assume_header(true)
           .column_types("string,float,float,float,float");

    std::istringstream is(bond_yields);

    jc::ojson j = csv::decode_csv<jc::ojson>(is,options);

    std::cout << pretty_print(j) << std::endl;
}

void encode_csv_file_from_books()
{
    const jc::json books = jc::json::parse(R"(
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

    csv::encode_csv(books, std::cout);
}

void decode_encode_csv_tasks()
{
    std::ifstream is("./input/tasks.csv");

    csv::csv_options options;
    options.assume_header(true)
           .trim(true)
           .ignore_empty_values(true) 
           .column_types("integer,string,string,string");
    jc::ojson tasks = csv::decode_csv<jc::ojson>(is, options);

    std::cout << "(1)\n" << pretty_print(tasks) << "\n\n";

    std::cout << "(2)\n";
    csv::encode_csv(tasks, std::cout);
}

void csv_parser_type_inference()
{
    csv::csv_options options;
    options.assume_header(true)
           .mapping(csv::mapping_type::n_objects);

    std::ifstream is1("input/sales.csv");
    jc::ojson j1 = csv::decode_csv<jc::ojson>(is1,options);
    std::cout << "\n(1)\n"<< pretty_print(j1) << "\n";

    options.mapping(csv::mapping_type::n_rows);
    std::ifstream is2("input/sales.csv");
    jc::ojson j2 = csv::decode_csv<jc::ojson>(is2,options);
    std::cout << "\n(2)\n"<< pretty_print(j2) << "\n";

    options.mapping(csv::mapping_type::m_columns);
    std::ifstream is3("input/sales.csv");
    jc::ojson j3 = csv::decode_csv<jc::ojson>(is3,options);
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
    jc::json_options print_options;
    print_options.array_array_line_splits(jc::line_split_kind::same_line);

    csv::csv_options options1;
    options1.assume_header(true)
           .subfield_delimiter(';');

    jc::json j1 = csv::decode_csv<jc::json>(s,options1);
    std::cout << "(1)\n" << pretty_print(j1,print_options) << "\n\n";

    csv::csv_options options2;
    options2.mapping(csv::mapping_type::n_rows)
           .subfield_delimiter(';');

    jc::json j2 = csv::decode_csv<jc::json>(s,options2);
    std::cout << "(2)\n" << pretty_print(j2,print_options) << "\n\n";

    csv::csv_options options3;
    options3.assume_header(true)
           .mapping(csv::mapping_type::m_columns)
           .subfield_delimiter(';');

    jc::json j3 = csv::decode_csv<jc::json>(s,options3);
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
    mapping_types2();
    std::cout << std::endl;
}

