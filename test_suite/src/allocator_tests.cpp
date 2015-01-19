// Copyright 2013 Daniel Parker
// Distributed under Boost license

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

using jsoncons::basic_json;
using std::string;
typedef jsoncons::basic_json<char, boost::fast_pool_allocator<void>> myjson;

BOOST_AUTO_TEST_CASE(test_allocator)
{
	// Works but leaves memory leaks
    
    /*myjson o;
	
    o.set("field1",10.0);
    o.set("field2",20.0);
    o.set("field3","Three");

	string s = o["field3"].as<string>();

    std::cout << o << std::endl;
	*/
    //boost::singleton_pool<boost::pool_allocator_tag, sizeof(myjson::string_wrapper)>::release_memory();
}

