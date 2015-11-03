// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include "jsoncons/json.hpp"
#include "jsoncons/json_serializer.hpp"
#include "jsoncons/json_filter.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <new>
#include <boost/optional.hpp>

using jsoncons::parsing_context;
using jsoncons::json_serializer;
using jsoncons::output_format;
using jsoncons::json;
using jsoncons::json_reader;
using jsoncons::json_input_handler;
using jsoncons::json_output_handler;
using std::string;
using jsoncons::json_filter;
using jsoncons::pretty_print;
using jsoncons::json_deserializer;
using jsoncons::json_parse_exception;

BOOST_AUTO_TEST_CASE(test_small_string)
{
	json s("ABCD");
	BOOST_CHECK(s.type() == jsoncons::value_types::small_string_t);
	BOOST_CHECK(s.small_string_length_ == 4);
	BOOST_CHECK(std::string(s.value_.small_string_value_,s.small_string_length_) == std::string("ABCD"));

	json t(s);
	BOOST_CHECK(t.type() == jsoncons::value_types::small_string_t);
	BOOST_CHECK(t.small_string_length_ == 4);
	BOOST_CHECK(std::string(t.value_.small_string_value_,t.small_string_length_) == std::string("ABCD"));

	json q;
	q = s;
	BOOST_CHECK(q.type() == jsoncons::value_types::small_string_t);
	BOOST_CHECK(q.small_string_length_ == 4);
	BOOST_CHECK(std::string(q.value_.small_string_value_,q.small_string_length_) == std::string("ABCD"));
}
