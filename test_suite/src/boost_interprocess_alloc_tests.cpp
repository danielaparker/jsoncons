// Copyright 2016 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <cstddef>
#include <memory>
#include <scoped_allocator>

using namespace jsoncons;
using namespace boost::interprocess;

BOOST_AUTO_TEST_SUITE(boost_interprocess_alloc_test_suite)

typedef boost::interprocess::allocator<int,
        boost::interprocess::managed_shared_memory::segment_manager> shmem_allocator;
typedef basic_json<char,json_traits<char>,shmem_allocator> shm_json;
typedef std::vector<int, shmem_allocator> ShmVector;

BOOST_AUTO_TEST_CASE(test_boost_interprocess_allocator)
{
    struct shm_remove
    {
        shm_remove() { boost::interprocess::shared_memory_object::remove("MySharedMemory"); }
        ~shm_remove(){ boost::interprocess::shared_memory_object::remove("MySharedMemory"); }
    } remover;

    //Create a new segment with given name and size
    boost::interprocess::managed_shared_memory segment(boost::interprocess::create_only,
            "MySharedMemory", 65536);

    //Initialize shared memory STL-compatible allocator
    const shmem_allocator allocator(segment.get_segment_manager());

    shm_json* j = segment.construct<shm_json>("shm_json")(allocator);

    ShmVector* v = segment.construct<ShmVector>("ShmVector")(allocator);

    /*typedef basic_json<char,json_traits<char>,allocator_type> my_json;

    managed_shared_memory segment(create_only, "MySharedMemory", 65536);
    allocator_type alloc(segment.get_segment_manager());

    my_json::array an_array = my_json::array(alloc);
    my_json::object an_object = my_json::object(alloc);

    my_json root = an_object;
    root.reserve(1);*/

    /*root.set("field1", 10.0);
    root.set("field2", 20.0);

    auto it = root.find("field1");

    std::cout << it->key() << std::endl;
    if (it != root.object_range().end())
    {
        my_json& val = it->value();
    }*/
   
    //BOOST_CHECK_EQUAL(10.0,root["field1"]);
    //BOOST_CHECK_EQUAL(20.0,root.get("field1",0));
    //std::cout << root << std::endl; 
}
BOOST_AUTO_TEST_SUITE_END()
