// Copyright 2020 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <jsoncons/json.hpp>

using namespace jsoncons;

namespace ns {

    template <class J,class T>
    struct A
    {
        static T f(const J&)
        {
            static_assert(sizeof(T) == 0, "f not implemented");
        }
    };

    template<class J,class Rep>
    struct A<J,std::chrono::duration<Rep>>
    {
        using duration_type = std::chrono::duration<Rep>;

        static duration_type f(const J&)
        {
            std::cout << "Hello duration\n";
            return duration_type{};
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
