// Copyright 2016 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <cstddef>
#include <memory>

using namespace jsoncons;
using namespace boost::interprocess;

BOOST_AUTO_TEST_SUITE(boost_interprocess_alloc_test_suite)

typedef boost::interprocess::allocator<int,
        boost::interprocess::managed_shared_memory::segment_manager> shmem_allocator;
typedef basic_json<char,json_traits<char>,shmem_allocator> shm_json;

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

    shm_json j = shm_json::array(allocator);
    j.add(0);
    j.add(1);
    j.add(2);
    j.add(3);
    j.resize(5);
    j[4] = 4;

    shm_json a{ shm_json::array(allocator) };
    a.add(true);
    a.add(false);
    a.add(null_type());

    j.add(a);
    shm_json v{ shm_json::array(allocator) };
    v.add(j);
    v.add(a);

    shm_json o = shm_json::object(allocator);
    o.set("name too long for small string optimization", "value too long for small string optimization");
    o["another name too long for small string optimization"] = 10;
    std::cout << o << std::endl;

    //shm_json o2{ shm_json::object(allocator) };
    //o2.set("name", 10.0);

    //j.add(o);
    //j.add(o2);

    //std::cout << v << std::endl;

    //std::cout << j << std::endl;

    //shm_json j2{ shm_json::array(allocator),allocator };

    //segment.destroy_ptr(j);

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
