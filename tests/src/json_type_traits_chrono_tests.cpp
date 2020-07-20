// Copyright 2020 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <jsoncons/json.hpp>

using namespace jsoncons;

namespace ns {

    template <class J,class Rep>
    struct A
    {
        static void f(const J&)
        {
            static_assert(sizeof(Rep) == 0, "f not implemented");
        }
    };

    template<class J,class Rep>
    struct A<J,std::chrono::duration<Rep>>
    {
        static void f(const J&)
        {
            std::cout << "Hello duration\n";
        }
    };
}

TEST_CASE("test_chrono")
{
    std::string s;
    ns::A<std::string,std::chrono::seconds>::f(s);
    ns::A<std::string,std::chrono::duration<double>>::f(s);
} 

#if 0

TEST_CASE("json_type_traits chron tests")
{
    SECTION("test 1")
    {
        uint64_t time = 1000;

        json j(time, semantic_tag::epoch_time);

        auto val = j.as<std::chrono::seconds>(); 

        CHECK(val.count() == std::chrono::seconds::rep(time));
    }
    SECTION("test 2")
    {
        double time = 1000.100;

        json j(time, semantic_tag::epoch_time);

        auto val = j.as<std::chrono::duration<double>>();

        CHECK(val.count() == time);
    }
}
#endif
