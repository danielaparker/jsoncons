## Support for Custom Allocators

Since 0.171.0, `basic_json` supports [std::uses_allocator](https://en.cppreference.com/w/cpp/memory/uses_allocator) construction.
The allocator template parameter may be a stateless allocator, a [std::pmr::polymorphic_allocator](https://en.cppreference.com/w/cpp/memory/polymorphic_allocator), or a [std::scoped_allocator_adaptor](https://en.cppreference.com/w/cpp/memory/scoped_allocator_adaptor).
Non-propagating stateful allocators, such as the [Boost.Interprocess allocators](https://www.boost.org/doc/libs/1_82_0/doc/html/interprocess/allocators_containers.html#interprocess.allocators_containers.allocator_introduction),
must be wrapped by a `std::scoped_allocator_adaptor`.

### Examples

#### Using `json` with boost stateless `fast_pool_allocator`
```cpp
#include <boost/pool/pool_alloc.hpp>
#include "jsoncons/json.hpp"

using bfp_json = jsoncons::basic_json<char, boost::fast_pool_allocator<void>>;

bfp_json j;

j.insert_or_assign("FirstName","Joe");
j.insert_or_assign("LastName","Smith");
```

#### Using `json` with stateful Boost.Interprocess allocators

```cpp
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <cstdlib> //std::system
#include <jsoncons/json.hpp>
#include <scoped_allocator>

using namespace jsoncons;

using shmem_allocator = boost::interprocess::allocator<int,
    boost::interprocess::managed_shared_memory::segment_manager>;

using MyScopedAllocator = std::scoped_allocator_adaptor<shmem_allocator>;

struct boost_sorted_policy 
{
    template <class KeyT,class Json>
    using object = sorted_json_object<KeyT,Json,boost::interprocess::vector>;

    template <class Json>
    using array = json_array<Json,boost::interprocess::vector>;

    template <class CharT, class CharTraits, class Allocator>
    using member_key = boost::interprocess::basic_string<CharT, CharTraits, Allocator>;
};

using shm_json = basic_json<char,boost_sorted_policy, MyScopedAllocator>;

int main(int argc, char *argv[])
{
   typedef std::pair<double, int> MyType;

   if (argc == 1){  //Parent process
      //Remove shared memory on construction and destruction
      struct shm_remove
      {
         shm_remove() { boost::interprocess::shared_memory_object::remove("MySharedMemory"); }
         ~shm_remove() noexcept { boost::interprocess::shared_memory_object::remove("MySharedMemory"); }
      } remover;

      //Construct managed shared memory
      boost::interprocess::managed_shared_memory segment(boost::interprocess::create_only, 
                                                         "MySharedMemory", 65536);

      //Initialize shared memory STL-compatible allocator
      const shmem_allocator alloc(segment.get_segment_manager());

      // Create json value with all dynamic allocations in shared memory

      shm_json* j = segment.construct<shm_json>("my json")(json_array_arg, alloc);
      j->push_back(10);

      shm_json o(json_object_arg, alloc);
      o.try_emplace("category", "reference");
      o.try_emplace("author", "Nigel Rees");
      o.insert_or_assign("title", "Sayings of the Century");
      o.insert_or_assign("price", 8.95);

      j->push_back(o);

      shm_json a = shm_json::array(2,shm_json::object(alloc),alloc);
      a[0]["first"] = 1;

      j->push_back(a);

      std::pair<shm_json*, boost::interprocess::managed_shared_memory::size_type> res;
      res = segment.find<shm_json>("my json");

      std::cout << "Parent:" << std::endl;
      std::cout << pretty_print(*(res.first)) << std::endl;

      //Launch child process
      std::string s(argv[0]); s += " child ";
      if (0 != std::system(s.c_str()))
         return 1;


      //Check child has destroyed all objects
      if (segment.find<MyType>("my json").first)
         return 1;
   }
   else{
      //Open managed shared memory
      boost::interprocess::managed_shared_memory segment(boost::interprocess::open_only, 
          "MySharedMemory");

      std::pair<shm_json*, boost::interprocess::managed_shared_memory::size_type> res;
      res = segment.find<shm_json>("my json");

      if (res.first != nullptr)
      {
          std::cout << "Child:" << std::endl;
          std::cout << pretty_print(*(res.first)) << std::endl;
      }
      else
      {
          std::cout << "Result is null" << std::endl;
      }

      //We're done, delete all the objects
      segment.destroy<shm_json>("my json");
   }
   return 0;
}

```
Output:
```
Parent:
[
    10,
    {
        "author": "Nigel Rees",
        "category": "reference",
        "price": 8.95,
        "title": "Sayings of the Century"
    },
    [
        {
            "first": 1
        },
        {}
    ]
]
Child:
[
    10,
    {
        "author": "Nigel Rees",
        "category": "reference",
        "price": 8.95,
        "title": "Sayings of the Century"
    },
    [
        {
            "first": 1
        },
        {}
    ]
]
```
