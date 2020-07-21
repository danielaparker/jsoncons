// Copyright 2020 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <jsoncons/json.hpp>

using namespace jsoncons;

namespace jsoncons {

    template <class Json,class T,class Enable=void>
    struct A
    {
        static T as(const Json&)
        {
            static_assert(sizeof(T) == 0, "as not implemented");
        }
    };

    template<class Json,class Rep>
    struct A<Json,std::chrono::duration<Rep>>
    {
        using duration_type = std::chrono::duration<Rep>;

        static duration_type as(const Json& j)
        {
            std::cout << "Hello duration\n";
            return duration_type(j.template as<Rep>());
        }
    };
}

TEST_CASE("test_chrono")
{
    json j(1000, semantic_tag::epoch_time);
    auto val1 = jsoncons::A<json,std::chrono::seconds>::as(j);
    auto val2 = jsoncons::A<json,std::chrono::duration<double>>::as(j);

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
