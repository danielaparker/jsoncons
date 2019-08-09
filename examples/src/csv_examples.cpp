// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons_ext/csv/csv.hpp>
#include <fstream>
#include <iomanip>
#include "example_types.hpp"

using namespace jsoncons;

namespace csv_examples {

void csv_source_to_json_value()
{
    const std::string s = R"(Date,1Y,2Y,3Y,5Y
2017-01-09,0.0062,0.0075,0.0083,0.011
2017-01-08,0.0063,0.0076,0.0084,0.0112
2017-01-08,0.0063,0.0076,0.0084,0.0112
)";

    csv::csv_options options;
    options.assume_header(true)
           .column_types("string,float,float,float,float");

    // mapping_type::n_objects
    options.mapping(csv::mapping_type::n_objects);
    ojson j1 = csv::decode_csv<ojson>(s,options);
    std::cout << "\n(1)\n"<< pretty_print(j1) << "\n";

    // mapping_type::n_rows
    options.mapping(csv::mapping_type::n_rows);
    ojson j2 = csv::decode_csv<ojson>(s,options);
    std::cout << "\n(2)\n"<< pretty_print(j2) << "\n";

    // mapping_type::m_columns
    options.mapping(csv::mapping_type::m_columns);
    ojson j3 = csv::decode_csv<ojson>(s,options);
    std::cout << "\n(3)\n" << pretty_print(j3) << "\n";
}

void csv_source_to_cpp_object()
{
    const std::string input = R"(Date,1Y,2Y,3Y,5Y
2017-01-09,0.0062,0.0075,0.0083,0.011
2017-01-08,0.0063,0.0076,0.0084,0.0112
2017-01-08,0.0063,0.0076,0.0084,0.0112
)";

    csv::csv_options ioptions;
    ioptions.header_lines(1)
           .mapping(csv::mapping_type::n_rows);

    typedef std::vector<std::tuple<std::string,double,double,double,double>> table_type;

    table_type table = csv::decode_csv<table_type>(input,ioptions);

    std::cout << "(1)\n";
    for (const auto& row : table)
    {
        std::cout << std::get<0>(row) << "," 
                  << std::get<1>(row) << "," 
                  << std::get<2>(row) << "," 
                  << std::get<3>(row) << "," 
                  << std::get<4>(row) << "\n";
    }
    std::cout << "\n";

    std::string output;

    csv::csv_options ooptions;
    ooptions.column_names("Date,1Y,2Y,3Y,5Y");
    csv::encode_csv<table_type>(table, output, ooptions);

    std::cout << "(2)\n";
    std::cout << output << "\n";
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
    ojson j = csv::decode_csv<ojson>(s,options);

    std::cout << pretty_print(j) << std::endl;
}

void read_write_csv_tasks()
{
    std::ifstream is("./input/tasks.csv");

    json_decoder<ojson> decoder;
    csv::csv_options options;
    options.assume_header(true)
           .trim(true)
           .ignore_empty_values(true) 
           .column_types("integer,string,string,string");
    csv::csv_reader reader(is,decoder,options);
    reader.read();
    ojson tasks = decoder.get_result();

    std::cout << "(1)\n";
    std::cout << pretty_print(tasks) << "\n\n";

    std::cout << "(2)\n";
    csv::csv_stream_encoder encoder(std::cout);
    tasks.dump(encoder);
}

void serialize_array_of_arrays_to_comma_delimited()
{
    std::string in_file = "./input/countries.json";
    std::ifstream is(in_file);

    json countries;
    is >> countries;

    csv::csv_stream_encoder encoder(std::cout);
    countries.dump(encoder);
}

void serialize_to_tab_delimited_file()
{
    std::string in_file = "./input/employees.json";
    std::ifstream is(in_file);

    json employees;
    is >> employees;

    csv::csv_options options;
    options.field_delimiter('\t');
    csv::csv_stream_encoder encoder(std::cout,options);

    employees.dump(encoder);
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

    csv::csv_stream_encoder encoder(std::cout);

    books.dump(encoder);
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

    csv::csv_options options;
    options.column_names("author,title,price");

    csv::csv_stream_encoder encoder(std::cout, options);

    books.dump(encoder);
}

void last_column_repeats()
{
    const std::string bond_yields = R"(Date,Yield
2017-01-09,0.0062,0.0075,0.0083,0.011,0.012
2017-01-08,0.0063,0.0076,0.0084,0.0112,0.013
2017-01-08,0.0063,0.0076,0.0084,0.0112,0.014
)";

    json_decoder<ojson> decoder1;
    csv::csv_options options1;
    options1.header_lines(1);
    options1.column_types("string,float*");
    std::istringstream is1(bond_yields);
    csv::csv_reader reader1(is1, decoder1, options1);
    reader1.read();
    ojson val1 = decoder1.get_result();
    std::cout << "\n(1)\n" << pretty_print(val1) << "\n";

    json_decoder<ojson> decoder2;
    csv::csv_options options2;
    options2.assume_header(true);
    options2.column_types("string,[float*]");
    std::istringstream is2(bond_yields);
    csv::csv_reader reader2(is2, decoder2, options2);
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
    csv::csv_options options1;
    options1.column_types("[integer,string]*");
    std::istringstream is1(holidays);
    csv::csv_reader reader1(is1, decoder1, options1);
    reader1.read();
    ojson val1 = decoder1.get_result();
    std::cout << "(1)\n" << pretty_print(val1) << "\n";

    // array of objects
    json_decoder<ojson> decoder2;
    csv::csv_options options2;
    options2.header_lines(1);
    options2.column_names("CAD,UK,EUR,US");
    options2.column_types("[integer,string]*");
    std::istringstream is2(holidays);
    csv::csv_reader reader2(is2, decoder2, options2);
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

    csv::csv_options options;
    options.assume_header(true)
           .column_types("string,string,string,float");
    json j = csv::decode_csv<json>(s,options);

    std::cout << pretty_print(j) << std::endl;
}

void decode_csv_stream()
{
    const std::string bond_yields = R"(Date,1Y,2Y,3Y,5Y
2017-01-09,0.0062,0.0075,0.0083,0.011
2017-01-08,0.0063,0.0076,0.0084,0.0112
2017-01-07,0.0063,0.0076,0.0084,0.0112
)";

    csv::csv_options options;
    options.assume_header(true)
           .column_types("string,float,float,float,float");

    std::istringstream is(bond_yields);

    ojson j = csv::decode_csv<ojson>(is,options);

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
    ojson tasks = csv::decode_csv<ojson>(is, options);

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
    ojson j1 = csv::decode_csv<ojson>(is1,options);
    std::cout << "\n(1)\n"<< pretty_print(j1) << "\n";

    options.mapping(csv::mapping_type::n_rows);
    std::ifstream is2("input/sales.csv");
    ojson j2 = csv::decode_csv<ojson>(is2,options);
    std::cout << "\n(2)\n"<< pretty_print(j2) << "\n";

    options.mapping(csv::mapping_type::m_columns);
    std::ifstream is3("input/sales.csv");
    ojson j3 = csv::decode_csv<ojson>(is3,options);
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
    csv::csv_options options1;
    options1.assume_header(true)
            .subfield_delimiter(';');

    json j1 = csv::decode_csv<json>(s,options1);

    json_options print_options;
    print_options.array_array_line_splits(line_split_kind::same_line)
                 .float_format(float_chars_format::fixed);

    std::cout << "(1)\n" << pretty_print(j1,print_options) << "\n\n";

    csv::csv_options options2;
    options2.mapping(csv::mapping_type::n_rows)
           .subfield_delimiter(';');

    json j2 = csv::decode_csv<json>(s,options2);
    std::cout << "(2)\n" << pretty_print(j2,print_options) << "\n\n";

    csv::csv_options options3;
    options3.assume_header(true)
           .mapping(csv::mapping_type::m_columns)
           .subfield_delimiter(';');

    json j3 = csv::decode_csv<json>(s,options3);
    std::cout << "(3)\n" << pretty_print(j3,print_options) << "\n\n";
}

    const std::string data = R"(index_id,observation_date,rate
EUR_LIBOR_06M,2015-10-23,0.0000214
EUR_LIBOR_06M,2015-10-26,0.0000143
EUR_LIBOR_06M,2015-10-27,0.0000001
)";

void as_a_variant_like_structure()
{
    csv::csv_options options;
    options.assume_header(true);

    // Parse the CSV data into an ojson value
    ojson j = csv::decode_csv<ojson>(data, options);

    // Pretty print
    json_options print_options;
    print_options.float_format(float_chars_format::fixed);
    std::cout << "(1)\n" << pretty_print(j, print_options) << "\n\n";

    // Iterate over the rows
    std::cout << std::fixed << std::setprecision(7);
    std::cout << "(2)\n";
    for (const auto& row : j.array_range())
    {
        // Access rated as string and rating as double
        std::cout << row["index_id"].as<std::string>() << ", " 
                  << row["observation_date"].as<std::string>() << ", " 
                  << row["rate"].as<double>() << "\n";
    }
}

void as_a_strongly_typed_cpp_structure()
{
    csv::csv_options options;
    options.assume_header(true)
           .float_format(float_chars_format::fixed);

    // Decode the CSV data into a c++ structure
    std::vector<ns::fixing> v = csv::decode_csv<std::vector<ns::fixing>>(data, options);

    // Iterate over values
    std::cout << std::fixed << std::setprecision(7);
    std::cout << "(1)\n";
    for (const auto& item : v)
    {
        std::cout << item.index_id() << ", " << item.observation_date() << ", " << item.rate() << "\n";
    }

    // Encode the c++ structure into CSV data
    std::string s;
    csv::encode_csv(v, s, options);
    std::cout << "(2)\n";
    std::cout << s << "\n";
}

void as_a_stream_of_json_events()
{
    csv::csv_options options;
    options.assume_header(true);

    csv::csv_cursor cursor(data, options);

    for (; !cursor.done(); cursor.next())
    {
        const auto& event = cursor.current();
        switch (event.event_type())
        {
            case staj_event_type::begin_array:
                std::cout << event.event_type() << " " << "\n";
                break;
            case staj_event_type::end_array:
                std::cout << event.event_type() << " " << "\n";
                break;
            case staj_event_type::begin_object:
                std::cout << event.event_type() << " " << "\n";
                break;
            case staj_event_type::end_object:
                std::cout << event.event_type() << " " << "\n";
                break;
            case staj_event_type::name:
                // Or std::string_view, if supported
                std::cout << event.event_type() << ": " << event.get<jsoncons::string_view>() << "\n";
                break;
            case staj_event_type::string_value:
                // Or std::string_view, if supported
                std::cout << event.event_type() << ": " << event.get<jsoncons::string_view>() << "\n";
                break;
            case staj_event_type::null_value:
                std::cout << event.event_type() << "\n";
                break;
            case staj_event_type::bool_value:
                std::cout << event.event_type() << ": " << std::boolalpha << event.get<bool>() << "\n";
                break;
            case staj_event_type::int64_value:
                std::cout << event.event_type() << ": " << event.get<int64_t>() << "\n";
                break;
            case staj_event_type::uint64_value:
                std::cout << event.event_type() << ": " << event.get<uint64_t>() << "\n";
                break;
            case staj_event_type::double_value:
                std::cout << event.event_type() << ": " << event.get<double>() << "\n";
                break;
            default:
                std::cout << "Unhandled event type: " << event.event_type() << " " << "\n";;
                break;
        }
    }
}

void grouped_into_basic_json_records()
{
    csv::csv_options options;
    options.assume_header(true);

    csv::csv_cursor cursor(data, options);

    auto it = make_array_iterator<ojson>(cursor);
    auto end = jsoncons::end(it);

    json_options print_options;
    print_options.float_format(float_chars_format::fixed);
    while (it != end)
    {
        std::cout << pretty_print(*it, print_options) << "\n";
        ++it;
    }
}

void grouped_into_strongly_typed_records()
{
    typedef std::tuple<std::string,std::string,double> record_type;

    csv::csv_options options;
    options.assume_header(true);
    csv::csv_cursor cursor(data, options);

    auto it = make_array_iterator<record_type>(cursor);

    std::cout << std::fixed << std::setprecision(7);
    for (const auto& record : it)
    {
        std::cout << std::get<0>(record) << ", " << std::get<1>(record) << ", " << std::get<2>(record) << "\n";
    }
}


} // csv_examples

void run_csv_examples()
{
    std::cout << "\nCSV examples\n\n";
    csv_examples::read_write_csv_tasks();
    csv_examples::serialize_to_tab_delimited_file();
    csv_examples::serialize_array_of_arrays_to_comma_delimited();
    csv_examples::serialize_books_to_csv_file();
    csv_examples::serialize_books_to_csv_file_with_reorder();
    csv_examples::last_column_repeats();
    csv_examples::last_two_columns_repeat();
    csv_examples::decode_csv_string();
    csv_examples::decode_csv_stream();
    csv_examples::encode_csv_file_from_books();
    csv_examples::decode_encode_csv_tasks();

    csv_examples::csv_decode_without_type_inference();
    csv_examples::csv_parser_type_inference();

    csv_examples::decode_csv_with_subfields();
    csv_examples::csv_source_to_json_value();
    csv_examples::csv_source_to_cpp_object();

    std::cout << "\n";
    csv_examples::as_a_variant_like_structure();
    std::cout << "\n";
    csv_examples::as_a_strongly_typed_cpp_structure();
    std::cout << "\n";
    csv_examples::as_a_stream_of_json_events();
    std::cout << "\n";
    csv_examples::grouped_into_basic_json_records();
    std::cout << "\n";
    csv_examples::grouped_into_strongly_typed_records();
    std::cout << "\n";

    std::cout << std::endl;
}

