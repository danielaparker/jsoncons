// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <cstddef>

using namespace jsoncons;

typedef basic_json<char,sorted_policy,boost::pool_allocator<char>> my_json;

#define NO_MEMORY_LEAK

#ifndef NO_MEMORY_LEAK
TEST_CASE("test_pool_allocator")
{
    jsoncons::json_decoder<json,boost::pool_allocator<char>> decoder;
    basic_json_parser<char,boost::pool_allocator<char>> parser(decoder);

    parser.reset();

    static std::string s("[1,2,3,4,5,6]");

    parser.update(s.data(),s.length());
    parser.parse_some();
    CHECK(parser.done());

    parser.end_parse();

    json j = decoder.get_result();

    std::cout << j << std::endl;
}

TEST_CASE("test_json_reader_with_allocator")
{
    jsoncons::json_decoder<json,boost::pool_allocator<char>> decoder;
    static std::string s("[1,2,3,4,5,6]");
    std::istringstream is(s);

    basic_json_reader<char,boost::pool_allocator<char>> reader(is,decoder); 
    reader.read();

    json j = decoder.get_result();

    std::cout << j << std::endl;
}
#endif

