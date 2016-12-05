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

BOOST_AUTO_TEST_CASE(example)
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

     shm_json* o = segment.construct<shm_json>("shm_json")(allocator);

    //o->set("category", "reference");
    //o->set("author", "Nigel Rees");
    //o->set("title", "Sayings of the Century");
    o->set("price", 8.95);

    std::cout << pretty_print(*o) << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()
