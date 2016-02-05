// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include "jsoncons_ext/csv/csv_reader.hpp"
#include "jsoncons_ext/csv/csv_serializer.hpp"
#include "jsoncons/json_reader.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;
using namespace jsoncons::csv;

BOOST_AUTO_TEST_SUITE(csv_test_suite)

BOOST_AUTO_TEST_CASE(csv_test1_array_1col_skip1)
{
    std::string text = "a\n1\n4";
    std::istringstream is(text);

    json_deserializer handler;

    csv_parameters params;
    params.header_lines(1);

    csv_reader reader(is,handler,params);
    reader.read();
    json val = handler.get_result();
    //std::cout << pretty_print(val) << std::endl; 

    BOOST_CHECK(val.size()==2);
    BOOST_CHECK(val[0].size()==1);
    BOOST_CHECK(val[1].size()==1);
    BOOST_CHECK(val[0][0]==json("1"));
    BOOST_CHECK(val[1][0]==json("4"));
}

BOOST_AUTO_TEST_CASE(csv_test1_array_1col)
{
    std::string text = "1\n4";
    std::istringstream is(text);

    json_deserializer handler;

    csv_parameters params;

    csv_reader reader(is,handler,params);
    reader.read();
    json val = handler.get_result();

    BOOST_CHECK(val.size()==2);
    BOOST_CHECK(val[0].size()==1);
    BOOST_CHECK(val[1].size()==1);
    BOOST_CHECK(val[0][0]==json("1"));
    BOOST_CHECK(val[1][0]==json("4"));
}

BOOST_AUTO_TEST_CASE(csv_test1_array_3cols)
{
    std::string text = "a,b,c\n1,2,3\n4,5,6";
    std::istringstream is(text);

    json_deserializer handler;

    csv_parameters params;

    csv_reader reader(is,handler,params);
    reader.read();
    json val = handler.get_result();

    BOOST_CHECK(val.size()==3);
    BOOST_CHECK(val[0].size()==3);
    BOOST_CHECK(val[1].size()==3);
    BOOST_CHECK(val[2].size()==3);
    BOOST_CHECK(val[0][0]==json("a"));
    BOOST_CHECK(val[0][1]==json("b"));
    BOOST_CHECK(val[0][2]==json("c"));
    BOOST_CHECK(val[1][0]==json("1"));
    BOOST_CHECK(val[1][1]==json("2"));
    BOOST_CHECK(val[1][2]==json("3"));
    BOOST_CHECK(val[2][0]==json("4"));
    BOOST_CHECK(val[2][1]==json("5"));
    BOOST_CHECK(val[2][2]==json("6"));
}

BOOST_AUTO_TEST_CASE(csv_test1_array_3cols_trim_leading)
{
    std::string text = "a ,b ,c \n 1, 2, 3\n 4 , 5 , 6 ";
    std::istringstream is(text);

    json_deserializer handler;

    csv_parameters params;
    params.trim_leading(true);

    csv_reader reader(is,handler,params);
    reader.read();
    json val = handler.get_result();

    BOOST_CHECK(val.size()==3);
    BOOST_CHECK(val[0].size()==3);
    BOOST_CHECK(val[1].size()==3);
    BOOST_CHECK(val[2].size()==3);
    BOOST_CHECK(val[0][0]==json("a "));
    BOOST_CHECK(val[0][1]==json("b "));
    BOOST_CHECK(val[0][2]==json("c "));
    BOOST_CHECK(val[1][0]==json("1"));
    BOOST_CHECK(val[1][1]==json("2"));
    BOOST_CHECK(val[1][2]==json("3"));
    BOOST_CHECK(val[2][0]==json("4 "));
    BOOST_CHECK(val[2][1]==json("5 "));
    BOOST_CHECK(val[2][2]==json("6 "));
}

BOOST_AUTO_TEST_CASE(csv_test1_array_3cols_trim_trailing)
{
    std::string text = "a ,b ,c \n 1, 2, 3\n 4 , 5 , 6 ";
    std::istringstream is(text);

    json_deserializer handler;

    csv_parameters params;
    params.trim_trailing(true);

    csv_reader reader(is,handler,params);
    reader.read();
    json val = handler.get_result();

    BOOST_CHECK(val.size()==3);
    BOOST_CHECK(val[0].size()==3);
    BOOST_CHECK(val[1].size()==3);
    BOOST_CHECK(val[2].size()==3);
    BOOST_CHECK(val[0][0]==json("a"));
    BOOST_CHECK(val[0][1]==json("b"));
    BOOST_CHECK(val[0][2]==json("c"));
    BOOST_CHECK(val[1][0]==json(" 1"));
    BOOST_CHECK(val[1][1]==json(" 2"));
    BOOST_CHECK(val[1][2]==json(" 3"));
    BOOST_CHECK(val[2][0]==json(" 4"));
    BOOST_CHECK(val[2][1]==json(" 5"));
    BOOST_CHECK(val[2][2]==json(" 6"));
}

BOOST_AUTO_TEST_CASE(csv_test1_array_3cols_trim)
{
    std::string text = "a ,, \n 1, 2, 3\n 4 , 5 , 6 ";
    std::istringstream is(text);

    json_deserializer handler;

    csv_parameters params;
    params.trim(true);
    params.unquoted_empty_value_is_null(true);

    csv_reader reader(is,handler,params);
    reader.read();
    json val = handler.get_result();

    BOOST_CHECK(val.size()==3);
    BOOST_CHECK(val[0].size()==3);
    BOOST_CHECK(val[1].size()==3);
    BOOST_CHECK(val[2].size()==3);
    BOOST_CHECK(val[0][0]==json("a"));
    BOOST_CHECK(val[0][1]==json::null_type());
    BOOST_CHECK(val[0][2]==json::null_type());
    BOOST_CHECK(val[1][0]==json("1"));
    BOOST_CHECK(val[1][1]==json("2"));
    BOOST_CHECK(val[1][2]==json("3"));
    BOOST_CHECK(val[2][0]==json("4"));
    BOOST_CHECK(val[2][1]==json("5"));
    BOOST_CHECK(val[2][2]==json("6"));
}

BOOST_AUTO_TEST_CASE(csv_test1_array_3cols_comment)
{
    std::string text = "a,b,c\n#1,2,3\n4,5,6";
    std::istringstream is(text);

    json_deserializer handler;

    csv_parameters params;
    params.comment_starter('#');

    csv_reader reader(is,handler,params);
    reader.read();
    json val = handler.get_result();

    BOOST_CHECK(val.size()==2);
    BOOST_CHECK(val[0].size()==3);
    BOOST_CHECK(val[1].size()==3);
    BOOST_CHECK(val[0][0]==json("a"));
    BOOST_CHECK(val[0][1]==json("b"));
    BOOST_CHECK(val[0][2]==json("c"));
    BOOST_CHECK(val[1][0]==json("4"));
    BOOST_CHECK(val[1][1]==json("5"));
    BOOST_CHECK(val[1][2]==json("6"));
}

BOOST_AUTO_TEST_CASE(csv_test1_object_1col)
{
    std::string text = "a\n1\n4";
    std::istringstream is(text);

    json_deserializer handler;

    csv_parameters params;
	params.assume_header(true);

    csv_reader reader(is,handler,params);
    reader.read();
    json val = handler.get_result();

    BOOST_CHECK(val.size()==2);
    BOOST_CHECK(val[0].size()==1);
    BOOST_CHECK(val[1].size()==1);
    BOOST_CHECK(val[0]["a"]==json("1"));
    BOOST_CHECK(val[1]["a"]==json("4"));
}

BOOST_AUTO_TEST_CASE(csv_test1_object_3cols)
{
    std::string text = "a,b,c\n1,2,3\n4,5,6";
    std::istringstream is(text);

    json_deserializer handler;

    csv_parameters params;
	params.assume_header(true);

    csv_reader reader(is,handler,params);
    reader.read();
    json val = handler.get_result();

    BOOST_CHECK(val.size()==2);
    BOOST_CHECK(val[0].size()==3);
    BOOST_CHECK(val[1].size()==3);
    BOOST_CHECK(val[0]["a"]==json("1"));
    BOOST_CHECK(val[0]["b"]==json("2"));
    BOOST_CHECK(val[0]["c"]==json("3"));
    BOOST_CHECK(val[1]["a"]==json("4"));
    BOOST_CHECK(val[1]["b"]==json("5"));
    BOOST_CHECK(val[1]["c"]==json("6"));
}

BOOST_AUTO_TEST_CASE(csv_test1_object_3cols_header)
{
    std::string text = "a,b,c\n1,2,3\n4,5,6";
    std::istringstream is(text);

    json_deserializer handler;

    csv_parameters params;
	params.header("x,y,z");
    params.header_lines(1);

    csv_reader reader(is,handler,params);
    reader.read();
    json val = handler.get_result();

    BOOST_CHECK(val.size()==2);
    BOOST_CHECK(val[0].size()==3);
    BOOST_CHECK(val[1].size()==3);
    BOOST_CHECK(val[0]["x"]==json("1"));
    BOOST_CHECK(val[0]["y"]==json("2"));
    BOOST_CHECK(val[0]["z"]==json("3"));
    BOOST_CHECK(val[1]["x"]==json("4"));
    BOOST_CHECK(val[1]["y"]==json("5"));
    BOOST_CHECK(val[1]["z"]==json("6"));
}

BOOST_AUTO_TEST_CASE(csv_test1_object_3cols_bool)
{
    std::string text = "a,b,c\n1,0,1\ntrue,FalSe,TrUe";
    std::istringstream is(text);

    json_deserializer handler;

    csv_parameters params;
	params.header("x,y,z");
	params.data_types("boolean,boolean,boolean");
    params.header_lines(1);

    csv_reader reader(is,handler,params);
    reader.read();
    json val = handler.get_result();

    BOOST_CHECK(val.size()==2);
    BOOST_CHECK(val[0].size()==3);
    BOOST_CHECK(val[1].size()==3);
    BOOST_CHECK(val[0]["x"]==json(true));
    BOOST_CHECK(val[0]["y"]==json(false));
    BOOST_CHECK(val[0]["z"]==json(true));
    BOOST_CHECK(val[1]["x"]==json(true));
    BOOST_CHECK(val[1]["y"]==json(false));
    BOOST_CHECK(val[1]["z"]==json(true));
}

BOOST_AUTO_TEST_CASE(csv_test1_object_1col_quoted)
{
    std::string text = "a\n\"1\"\n\"4\"";
    std::istringstream is(text);

    json_deserializer handler;

    csv_parameters params;
	params.assume_header(true);

    csv_reader reader(is,handler,params);
    reader.read();
    json val = handler.get_result();

    std::cout << pretty_print(val) << std::endl;

    BOOST_CHECK(val.size()==2);
    BOOST_CHECK(val[0].size()==1);
    BOOST_CHECK(val[1].size()==1);
    BOOST_CHECK(val[0]["a"]==json("1"));
    BOOST_CHECK(val[1]["a"]==json("4"));
}

BOOST_AUTO_TEST_CASE(csv_test1_object_3cols_quoted)
{
    std::string text = "a,b,c\n\"1\",\"2\",\"3\"\n4,5,\"6\"";
    std::istringstream is(text);

    json_deserializer handler;

    csv_parameters params;
	params.assume_header(true);

    csv_reader reader(is,handler,params);
    reader.read();
    json val = handler.get_result();

    std::cout << pretty_print(val) << std::endl;

    BOOST_CHECK(val.size()==2);
    BOOST_CHECK(val[0].size()==3);
    BOOST_CHECK(val[1].size()==3);
    BOOST_CHECK(val[0]["a"]==json("1"));
    BOOST_CHECK(val[0]["b"]==json("2"));
    BOOST_CHECK(val[0]["c"]==json("3"));
    BOOST_CHECK(val[1]["a"]==json("4"));
    BOOST_CHECK(val[1]["b"]==json("5"));
    BOOST_CHECK(val[1]["c"]==json("6"));
}

BOOST_AUTO_TEST_CASE(csv_test1_array_1col_crlf)
{
    std::string text = "1\r\n4";
    std::istringstream is(text);

    json_deserializer handler;

    csv_parameters params;

    csv_reader reader(is,handler,params);
    reader.read();
    json val = handler.get_result();

    BOOST_CHECK(val.size()==2);
    BOOST_CHECK(val[0].size()==1);
    BOOST_CHECK(val[1].size()==1);
    BOOST_CHECK(val[0][0]==json("1"));
    BOOST_CHECK(val[1][0]==json("4"));
}

BOOST_AUTO_TEST_CASE(csv_test1_array_3cols_crlf)
{
    std::string text = "a,b,c\r\n1,2,3\r\n4,5,6";
    std::istringstream is(text);

    json_deserializer handler;

    csv_parameters params;

    csv_reader reader(is,handler,params);
    reader.read();
    json val = handler.get_result();

    BOOST_CHECK(val.size()==3);
    BOOST_CHECK(val[0].size()==3);
    BOOST_CHECK(val[1].size()==3);
    BOOST_CHECK(val[2].size()==3);
    BOOST_CHECK(val[0][0]==json("a"));
    BOOST_CHECK(val[0][1]==json("b"));
    BOOST_CHECK(val[0][2]==json("c"));
    BOOST_CHECK(val[1][0]==json("1"));
    BOOST_CHECK(val[1][1]==json("2"));
    BOOST_CHECK(val[1][2]==json("3"));
    BOOST_CHECK(val[2][0]==json("4"));
    BOOST_CHECK(val[2][1]==json("5"));
    BOOST_CHECK(val[2][2]==json("6"));
}

BOOST_AUTO_TEST_CASE(csv_test1_object_1col_crlf)
{
    std::string text = "a\r\n1\r\n4";
    std::istringstream is(text);

    json_deserializer handler;

    csv_parameters params;
	params.assume_header(true);

    csv_reader reader(is,handler,params);
    reader.read();
    json val = handler.get_result();

    BOOST_CHECK(val.size()==2);
    BOOST_CHECK(val[0].size()==1);
    BOOST_CHECK(val[1].size()==1);
    BOOST_CHECK(val[0]["a"]==json("1"));
    BOOST_CHECK(val[1]["a"]==json("4"));
}

BOOST_AUTO_TEST_CASE(csv_test1_object_3cols_crlf)
{
    std::string text = "a,b,c\r\n1,2,3\r\n4,5,6";
    std::istringstream is(text);

    json_deserializer handler;

    csv_parameters params;
	params.assume_header(true);

    csv_reader reader(is,handler,params);
    reader.read();
    json val = handler.get_result();

    BOOST_CHECK(val.size()==2);
    BOOST_CHECK(val[0].size()==3);
    BOOST_CHECK(val[1].size()==3);
    BOOST_CHECK(val[0]["a"]==json("1"));
    BOOST_CHECK(val[0]["b"]==json("2"));
    BOOST_CHECK(val[0]["c"]==json("3"));
    BOOST_CHECK(val[1]["a"]==json("4"));
    BOOST_CHECK(val[1]["b"]==json("5"));
    BOOST_CHECK(val[1]["c"]==json("6"));
}

BOOST_AUTO_TEST_CASE(read_comma_delimited_file)
{
    std::string in_file = "input/countries.csv";
    std::ifstream is(in_file);

    json_deserializer handler;

    csv_parameters params;
	params.assume_header(true);

    csv_reader reader(is,handler,params);
    reader.read();
    json countries = handler.get_result();

    std::cout << pretty_print(countries) << std::endl;
}

BOOST_AUTO_TEST_CASE(read_comma_delimited_file_header)
{
    std::string in_file = "input/countries.csv";
    std::ifstream is(in_file);

    json_deserializer handler;

    csv_parameters params;
    params.header("Country Code,Name");
    params.header_lines(1);

    csv_reader reader(is,handler,params);
    reader.read();
    json countries = handler.get_result();

    std::cout << pretty_print(countries) << std::endl;
}

BOOST_AUTO_TEST_CASE(serialize_comma_delimited_file)
{
    std::string in_file = "input/countries.json";
    std::ifstream is(in_file);

    json_deserializer handler;
    json_reader reader(is,handler);
    reader.read_next();
    json countries = handler.get_result();

    csv_serializer serializer(std::cout);

    countries.to_stream(serializer);
}

BOOST_AUTO_TEST_CASE(test_tab_delimited_file)
{
    std::string in_file = "input/employees.txt";
    std::ifstream is(in_file);

    json_deserializer handler;
    csv_parameters params;
    params.field_delimiter('\t');
    params.assume_header(true);

    csv_reader reader(is,handler,params);
    reader.read();
    json employees = handler.get_result();

    std::cout << pretty_print(employees) << std::endl;
}

BOOST_AUTO_TEST_CASE(serialize_tab_delimited_file)
{
    std::string in_file = "input/employees.json";
    std::ifstream is(in_file);

    json_deserializer handler;
    csv_parameters params;
    params.field_delimiter('\t');

    json_reader reader(is,handler);
    reader.read_next();
    json employees = handler.get_result();

    csv_serializer serializer(std::cout,params);

    employees.to_stream(serializer);
}

BOOST_AUTO_TEST_SUITE_END()

