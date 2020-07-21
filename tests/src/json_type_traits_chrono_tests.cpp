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

        static duration_type f(const J& j)
        {
            std::cout << "Hello duration\n";
            return duration_type{j.as<Rep>()};
        }
    };
}

TEST_CASE("test_chrono")
{
    json j(1000, semantic_tag::epoch_time);
    auto val1 = ns::A<json,std::chrono::seconds>::f(j);
    auto val2 = ns::A<json,std::chrono::duration<double>>::f(j);

    std::cout << val1.count() << ", " << val2.count() << "\n";
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
