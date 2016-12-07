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

BOOST_AUTO_TEST_SUITE(boost_interprocess_alloc_test_suite)

typedef boost::interprocess::allocator<int,
        boost::interprocess::managed_shared_memory::segment_manager> shmem_allocator;
typedef basic_json<char,json_traits<char>,shmem_allocator> shm_json;
typedef basic_json<char,o_json_traits<char>,shmem_allocator> shm_ojson;

BOOST_AUTO_TEST_CASE(json_shared_memory_test)
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

    // Create json value with all dynamic allocations in shared memory

    shm_json* j = segment.construct<shm_json>("my json")(shm_json::array(allocator));

    shm_json o(allocator);
    o.set("category", "reference");
    o.set("author", "Nigel Rees");
    o.set("title", "Sayings of the Century");
    o.set("price", 8.95);

    j->add(o);

    std::pair<shm_json*, boost::interprocess::managed_shared_memory::size_type> res;
    res = segment.find<shm_json>("my json");

    std::cout << pretty_print(*(res.first)) << std::endl;
}

BOOST_AUTO_TEST_CASE(ojson_shared_memory_test)
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

    // Create json value with all dynamic allocations in shared memory

    shm_ojson::array a(allocator);
    shm_ojson* j = segment.construct<shm_ojson>("shm_ojson")(a,allocator);

    shm_ojson o(allocator);
    o.set("category", "reference");
    o.set("author", "Nigel Rees");
    o.set("title", "Sayings of the Century");
    o.set("price", 8.95);

    j->add(o);

    std::cout << pretty_print(*j) << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()
