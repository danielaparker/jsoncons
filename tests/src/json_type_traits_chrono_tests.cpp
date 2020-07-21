// Copyright 2020 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <jsoncons/json.hpp>

using namespace jsoncons;

namespace jsoncons {
#if 0
    template <class Json,class T,class Enable=void>
    struct A
    {
        using allocator_type = typename Json::allocator_type;

        static constexpr bool is_compatible = false;

        static T as(const Json&)
        {
            static_assert(sizeof(T) == 0, "as not implemented");
        }

        static Json to_json(const T&, const allocator_type& = allocator_type())
        {
            static_assert(sizeof(T) == 0, "to_json not implemented");
        }
    };
#endif
    template<class Json,class Rep>
    struct json_type_traits<Json,std::chrono::duration<Rep>>
    {
        using duration_type = std::chrono::duration<Rep>;

        using allocator_type = typename Json::allocator_type;

        static bool is(const Json& j) noexcept
        {
            return (j.tag() == semantic_tag::epoch_time);
        }

        static duration_type as(const Json& j)
        {
            if (j.is_number())
            {
                return duration_type{j.template as<Rep>()};
            }
            else
            {
                return duration_type{};
            }
        }
        static Json to_json(const duration_type& val, allocator_type = allocator_type())
        {
            return Json(val.count(), semantic_tag::epoch_time);
        }
    };
}

TEST_CASE("test_chrono")
{
    json j1(1000, semantic_tag::epoch_time);
    json j2(1000.10, semantic_tag::epoch_time);
    auto val1 = jsoncons::json_type_traits<json,std::chrono::seconds>::as(j1);
    auto val2 = jsoncons::json_type_traits<json,std::chrono::duration<double>>::as(j2);

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
