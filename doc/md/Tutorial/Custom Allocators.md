## Examples

### Using `json` with boost Boost.Interprocess allocators

```c++
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <cstdlib> //std::system
#include <jsoncons/json.hpp>

using namespace jsoncons;
using namespace boost::interprocess;

typedef boost::interprocess::allocator<int,
        boost::interprocess::managed_shared_memory::segment_manager> shmem_allocator;

template<class CharT>
struct b_json_traits : public json_traits<char>
{
   template <class T,class Allocator>
   using object_container = boost::interprocess::vector<T,Allocator>;

   template <class T, class Allocator>
   using array_container = boost::interprocess::vector<T,Allocator>;

   template <class Allocator>
   using key_container = boost::interprocess::vector<CharT,Allocator>;

   template <class Allocator>
   using string_container = boost::interprocess::vector<CharT, Allocator>;
};

typedef basic_json<char,b_json_traits<char>,shmem_allocator> shm_json;

int main(int argc, char *argv[])
{
   typedef std::pair<double, int> MyType;

   if(argc == 1){  //Parent process
      //Remove shared memory on construction and destruction
      struct shm_remove
      {
         shm_remove() { shared_memory_object::remove("MySharedMemory"); }
         ~shm_remove(){ shared_memory_object::remove("MySharedMemory"); }
      } remover;

      //Construct managed shared memory
      managed_shared_memory segment(create_only, "MySharedMemory", 65536);

      //Initialize shared memory STL-compatible allocator
      const shmem_allocator allocator(segment.get_segment_manager());

      // Create json value with all dynamic allocations in shared memory

      shm_json* j = segment.construct<shm_json>("my json")(shm_json::array(allocator));
      j->add(10);

      shm_json o(allocator);
      o.set("category", "reference");
      o.set("author", "Nigel Rees");
      o.set("title", "Sayings of the Century");
      o.set("price", 8.95);

      j->add(o);

      std::pair<shm_json*,managed_shared_memory::size_type> res;
      res = segment.find<shm_json>("my json");

      std::cout << pretty_print(*(res.first)) << std::endl;

      //Launch child process
      std::string s(argv[0]); s += " child ";
      if(0 != std::system(s.c_str()))
         return 1;


      //Check child has destroyed all objects
      if(segment.find<MyType>("my json").first)
         return 1;
   }
   else{
      //Open managed shared memory
      managed_shared_memory segment(open_only, "MySharedMemory");

      std::pair<shm_json*,managed_shared_memory::size_type> res;
      res = segment.find<shm_json>("my json");

      if (res.first != nullptr)
      {
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
