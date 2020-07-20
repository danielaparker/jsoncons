// Copyright 2020 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <jsoncons/json.hpp>
#include <chrono>

using namespace jsoncons;

namespace {

    template <class T, class = void>
    struct A
    {
        static void f()
        {
            static_assert(std::false_type<T>::value, "f not implemented");
        }
    };

    template<class Rep>
    struct A<std::chrono::duration<Rep>>
    {
        static void f()
        {
        }
    };

}

TEST_CASE("std::chrono::duration test")
{
    A<std::chrono::seconds>::f();
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
