// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include "jsoncons/json.hpp"
#include "jsoncons/json_serializer.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <boost/pool/pool_alloc.hpp>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(allocator_test_suite)

typedef jsoncons::basic_json<char, boost::fast_pool_allocator<void>> myjson;

BOOST_AUTO_TEST_CASE(test_allocator)
{
	// Works but leaves memory leaks, memory needs to be released
    
    /*myjson o;
	
    o.set("field1",10.0);
    o.set("field2",20.0);
    o.set("field333","Too large for small string value");

	std::string s = o["field333"].as<std::string>();

    std::cout << o << std::endl; 
    */ 
	
    //boost::singleton_pool<boost::pool_allocator_tag, sizeof(myjson::string_wrapper)>::release_memory();
}
BOOST_AUTO_TEST_SUITE_END()
