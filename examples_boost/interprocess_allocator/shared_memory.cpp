#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <cstdlib> //std::system
#include <jsoncons/json.hpp>
#include <scoped_allocator>

using namespace jsoncons;

typedef boost::interprocess::allocator<char,
        boost::interprocess::managed_shared_memory::segment_manager> shmem_allocator;

using MyScopedAllocator = std::scoped_allocator_adaptor<shmem_allocator>;

struct boost_sorted_policy 
{
    template <typename KeyT,typename Json>
    using object = sorted_json_object<KeyT,Json,boost::interprocess::vector>;

    template <typename Json>
    using array = json_array<Json,boost::interprocess::vector>;

    template <typename CharT,typename CharTraits,typename Allocator>
    using member_key = boost::interprocess::basic_string<CharT, CharTraits, Allocator>;
};

using cust_json = basic_json<char,boost_sorted_policy,MyScopedAllocator>;

int f1(int argc, char *argv[])
{
   if (argc == 1){  //Parent process
      //Remove shared memory on construction and destruction
      struct shm_remove
      {
         shm_remove() noexcept { boost::interprocess::shared_memory_object::remove("MySharedMemory"); }
         ~shm_remove() noexcept{ boost::interprocess::shared_memory_object::remove("MySharedMemory"); }
      } remover;

      //Construct managed shared memory
      boost::interprocess::managed_shared_memory segment(boost::interprocess::create_only, 
                                                         "MySharedMemory", 6553);

      //Initialize shared memory STL-compatible allocator
      const shmem_allocator alloc(segment.get_segment_manager());

      // Create json value with all dynamic allocations in shared memory

      cust_json* j = segment.construct<cust_json>("MyJson1")(json_array_arg, alloc);
      j->push_back(10);

      cust_json o(json_object_arg, alloc);
      o.insert_or_assign("category", "reference");
      o.insert_or_assign("author", "Nigel Rees");
      o.insert_or_assign("title", "Sayings of the Century");
      o.insert_or_assign("price", 8.95);

      j->push_back(o);

      //cust_json a = cust_json::array(2,cust_json(json_object_arg, semantic_tag::none, alloc),alloc);
      //a[0]["first"] = 1;

      //cust_json a(json_array_arg, semantic_tag::none, alloc);

      //j->push_back(a);

      std::pair<cust_json*, boost::interprocess::managed_shared_memory::size_type> res;
      res = segment.find<cust_json>("MyJson1");

      std::cout << "Parent:" << std::endl;
      std::cout << pretty_print(*(res.first)) << std::endl;

      //Launch child process
      std::string s(argv[0]); s += " child ";
      if (0 != std::system(s.c_str()))
         return 1;

      //Check child has destroyed all objects
      if (segment.find<cust_json>("MyJson1").first)
         return 1;
   }
   else{
      //Open managed shared memory
      boost::interprocess::managed_shared_memory segment(boost::interprocess::open_only, 
                                                         "MySharedMemory");

      std::pair<cust_json*, boost::interprocess::managed_shared_memory::size_type> res;
      res = segment.find<cust_json>("MyJson1");

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
      segment.destroy<cust_json>("MyJson1");
   }
   return 0;
} 

int f2(int argc, char *argv[])
{
   typedef std::pair<double, int> MyType;

   if (argc == 1){  //Parent process
      //Remove shared memory on construction and destruction
      struct shm_remove
      {
         shm_remove() { boost::interprocess::shared_memory_object::remove("MySharedMemory"); }
         ~shm_remove(){ boost::interprocess::shared_memory_object::remove("MySharedMemory"); }
      } remover;

      //Construct managed shared memory
      boost::interprocess::managed_shared_memory segment(boost::interprocess::create_only, 
                                                         "MySharedMemory", 100000);

      //Initialize shared memory STL-compatible allocator
      const MyScopedAllocator alloc(segment.get_segment_manager());

      // Create json value with all dynamic allocations in shared memory

      cust_json* j = segment.construct<cust_json>("MyJson2")(cust_json(json_array_arg, semantic_tag::none, alloc));
      j->push_back(10);
      j->push_back(20);
      j->push_back(30);
      j->push_back(cust_json(json_object_arg, semantic_tag::none, alloc));
      j->push_back(cust_json(json_array_arg, semantic_tag::none, alloc));
      j->at(4).push_back(40);

      std::pair<cust_json*, boost::interprocess::managed_shared_memory::size_type> res;
      res = segment.find<cust_json>("MyJson2");

      std::cout << "Parent:" << std::endl;
      std::cout << pretty_print(*(res.first)) << std::endl;

      //Launch child process
      std::string s(argv[0]); s += " child ";
      if (0 != std::system(s.c_str()))
         return 1;

      //Check child has destroyed all objects
      if (segment.find<cust_json>("MyJson2").first)
         return 1;
   }
   else{
      //Open managed shared memory
      boost::interprocess::managed_shared_memory segment(boost::interprocess::open_only, 
                                                         "MySharedMemory");

      std::pair<cust_json*, boost::interprocess::managed_shared_memory::size_type> res;
      res = segment.find<cust_json>("MyJson2");

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
      segment.destroy<cust_json>("MyJson2");
   }
   return 0;
} 

int main(int argc, char *argv[])
{
   return f2(argc, argv);
}

