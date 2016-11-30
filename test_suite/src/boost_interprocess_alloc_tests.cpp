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

using namespace jsoncons;
using namespace boost::interprocess;

BOOST_AUTO_TEST_SUITE(boost_interprocess_alloc_test_suite)

typedef boost::interprocess::allocator<int,
        boost::interprocess::managed_shared_memory::segment_manager> shmem_allocator;
typedef basic_json<char,json_traits<char>,shmem_allocator> shm_json;

BOOST_AUTO_TEST_CASE(test_vector4)
{
    typedef std::vector<shm_json, shmem_allocator> ShmVector;
    typedef std::vector<ShmVector, shmem_allocator> ShmVector2;

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

    auto j1 = shm_json::array(allocator);
    j1.add(1);
    j1.add(2);
    shm_json::variant::array_data x(j1);

    auto j2 = shm_json::array(allocator);
    j2.add(true);
    j2.add(false);
    shm_json::variant::array_data y(j2);

    std::swap(x.data_,y.data_);
}
#if 0
BOOST_AUTO_TEST_CASE(test_vector3)
{
    typedef std::vector<shm_json, shmem_allocator> ShmVector;
    typedef std::vector<ShmVector, shmem_allocator> ShmVector2;

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

    shm_json j1{shm_json::array(allocator)};

    shm_json j2(2);

    j1.swap(j2);
}

BOOST_AUTO_TEST_CASE(test_vector1)
{
    typedef std::vector<shm_json, shmem_allocator> ShmVector;
    typedef std::vector<ShmVector, shmem_allocator> ShmVector2;

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

    shm_json j1 = shm_json::array(allocator);
    //j1.add(1);
    //j1.add(2);

    std::cout << "is_move_constructible " << std::boolalpha << std::is_move_constructible<shm_json>::value << std::endl;
    std::cout << "is_copy_constructible " << std::boolalpha << std::is_copy_constructible<shm_json>::value << std::endl;

    ShmVector vec(allocator);
    //vec.reserve(2);
    std::cout << "Check 1" << std::endl;
    vec.push_back(j1);
    std::cout << "Check 2" << std::endl;
    vec.push_back(shm_json(2));
    std::cout << "Check 3" << std::endl;
    //for (const auto& x : vec)
    //{
    //    std::cout << x << std::endl;
    //}
}

BOOST_AUTO_TEST_CASE(test_vector2)
{
    typedef std::vector<int, shmem_allocator> ShmVector;
    typedef std::vector<ShmVector, shmem_allocator> ShmVector2;

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

    ShmVector vec(allocator);
    vec.push_back(1);
    vec.push_back(2);

    ShmVector vec1(allocator);
    vec1.push_back(10);
    vec1.push_back(20);

    ShmVector2 vec2(allocator);
    vec2.emplace_back(vec);
    vec2.emplace_back(vec1);

    for (const auto& x : vec2)
    {
        for (const auto& y : x)
        {
            std::cout << y << " ";
        }
        std::cout << std::endl;
    }
}
#endif
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

    shm_json j{ shm_json::array(allocator) };
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

    //shm_json o{ shm_json::object(allocator) };
    //o.set("name too long for small string optimization", 10.0);
    //shm_json o2{ shm_json::object(allocator) };
    //o2.set("name", 10.0);

    //j.add(o);
    //j.add(o2);

    std::cout << v << std::endl;

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
