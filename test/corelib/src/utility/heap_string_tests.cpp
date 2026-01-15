// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/utility/heap_string.hpp>

#include <ctime>
#include <sstream>
#include <utility>
#include <vector>

#include <catch/catch.hpp>

using namespace jsoncons;

TEST_CASE("heap_string test")
{
    using heap_string_factory_type = jsoncons::heap::heap_string_factory<char, null_type, std::allocator<char>>;
    using pointer = typename heap_string_factory_type::pointer;

    std::string s("Hello World");

    pointer ptr = heap_string_factory_type::create(s.data(), s.length(), null_type(), std::allocator<char>());
 
    CHECK(s == ptr->c_str());
    CHECK(s.length() == ptr->length_);

    heap_string_factory_type::destroy(ptr);
}

#if defined(JSONCONS_HAS_POLYMORPHIC_ALLOCATOR) && JSONCONS_HAS_POLYMORPHIC_ALLOCATOR == 1
#include <memory_resource> 

class checked_resource : public std::pmr::memory_resource
{
public:
   explicit checked_resource(std::pmr::memory_resource* upstream = std::pmr::get_default_resource())
      : upstream_(upstream)
   {
   }

   std::make_signed<size_t>::type allocated = 0;

protected:
   void* do_allocate(std::size_t bytes, std::size_t alignment) override
   {
      allocated += bytes;
      return upstream_->allocate(bytes, alignment);
   }

   void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override
   {
      allocated -= bytes;
      upstream_->deallocate(p, bytes, alignment);
   }

   bool do_is_equal(const memory_resource& other) const noexcept override { return this == &other; }

private:
   std::pmr::memory_resource* upstream_;
};


TEST_CASE("heap_string with polymorphic allocator test")
{
    using heap_string_factory_type = jsoncons::heap::heap_string_factory<char, null_type, std::pmr::polymorphic_allocator<char>>;
    using pointer = typename heap_string_factory_type::pointer;

    char buffer[1024] = {}; // a small buffer on the stack
    std::pmr::monotonic_buffer_resource pool1{ std::data(buffer), std::size(buffer) };
    checked_resource checked(&pool1);
    std::pmr::polymorphic_allocator<char> alloc(&checked);

    std::string s1("Hello World 1");
    pointer ptr1 = heap_string_factory_type::create(s1.data(), s1.length(), null_type(), alloc);
    CHECK(s1 == ptr1->c_str());
    CHECK(s1.length() == ptr1->length_);

    std::string s2("Hello 2");
    pointer ptr2 = heap_string_factory_type::create(s2.data(), s2.length(), null_type(), alloc);
    CHECK(s2 == ptr2->c_str());
    CHECK(s2.length() == ptr2->length_);

    heap_string_factory_type::destroy(ptr1);
    heap_string_factory_type::destroy(ptr2);

    CHECK(checked.allocated == 0);
}


#endif

