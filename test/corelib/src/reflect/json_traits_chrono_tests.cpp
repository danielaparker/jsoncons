// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <cmath>
#include <limits>
#include <catch/catch.hpp>

using namespace jsoncons;

TEST_CASE("json_traits chron tests")
{
    SECTION("test 1")
    {
        uint64_t time = 1000;

        jsoncons::json j(time, jsoncons::semantic_tag::epoch_second);

        auto val = j.as<std::chrono::seconds>(); 

        CHECK(val.count() == std::chrono::seconds::rep(time));
    }
    SECTION("test 2")
    {
        double time = 1000.100;

        jsoncons::json j(time, jsoncons::semantic_tag::epoch_second);

        auto val = j.as<std::chrono::duration<double>>();

        CHECK(val.count() == time);
    }
}

TEST_CASE("json_traits chrono out of range tests")
{
    SECTION("std::chrono::seconds")
    {
        auto result = json(1e300, semantic_tag::epoch_second).try_as<std::chrono::seconds>();
        REQUIRE_FALSE(result);
        CHECK(conv_errc::not_epoch == result.error().code());

        CHECK_FALSE(json(9223372036854775808.0, semantic_tag::epoch_second).try_as<std::chrono::seconds>());
        CHECK_FALSE(json(std::nan(""), semantic_tag::epoch_second).try_as<std::chrono::seconds>());
        CHECK_FALSE(json("9223372036854775808000", semantic_tag::epoch_milli).try_as<std::chrono::seconds>());
        CHECK_FALSE(json("9223372036854775808000000000", semantic_tag::epoch_nano).try_as<std::chrono::seconds>());
        CHECK_FALSE(json(1e300, semantic_tag::epoch_milli).try_as<std::chrono::seconds>());
        CHECK_FALSE(json(1e300, semantic_tag::epoch_nano).try_as<std::chrono::seconds>());
        CHECK_FALSE(json(1e300).try_as<std::chrono::seconds>());
        CHECK_FALSE(json(uint64_t(9223372036854775808ull), semantic_tag::epoch_second).try_as<std::chrono::seconds>());
        CHECK_FALSE(json(int64_t(2147483648000), semantic_tag::epoch_milli).try_as<std::chrono::duration<int32_t>>());
        CHECK_FALSE(json(int64_t(-1500), semantic_tag::epoch_milli).try_as<std::chrono::duration<uint32_t>>());

        CHECK(json(-9223372036854775808.0, semantic_tag::epoch_second).as<std::chrono::seconds>().count() == (std::numeric_limits<int64_t>::min)());
        CHECK(json(1500.0, semantic_tag::epoch_milli).as<std::chrono::seconds>().count() == 1);
        CHECK(json("9223372036854775807999", semantic_tag::epoch_milli).as<std::chrono::seconds>().count() == (std::numeric_limits<int64_t>::max)());
        CHECK(json(1e300, semantic_tag::epoch_second).as<std::chrono::duration<double>>().count() == 1e300);
        CHECK(json(int64_t(1500), semantic_tag::epoch_milli).as<std::chrono::duration<double>>().count() == 1.5);
        CHECK(json(uint64_t(10000000000000000000ull), semantic_tag::epoch_nano).as<std::chrono::seconds>().count() == 10000000000);
        CHECK(json(-9223372036854775808.0, semantic_tag::epoch_milli).as<std::chrono::seconds>().count() == -9223372036854775);
        CHECK(json(int64_t(1761436800123), semantic_tag::epoch_milli).as<std::chrono::duration<int32_t>>().count() == 1761436800);
        CHECK(json(uint64_t(18446744073709551615ull), semantic_tag::epoch_milli).as<std::chrono::seconds>().count() == 18446744073709551);
        CHECK(json(9223372036854775808.0, semantic_tag::epoch_milli).as<std::chrono::seconds>().count() == 9223372036854775);
    }
    SECTION("std::chrono::milliseconds")
    {
        auto result = json(int64_t(9223372036854776), semantic_tag::epoch_second).try_as<std::chrono::milliseconds>();
        REQUIRE_FALSE(result);
        CHECK(conv_errc::not_epoch == result.error().code());

        CHECK_FALSE(json(int64_t(-9223372036854776), semantic_tag::epoch_second).try_as<std::chrono::milliseconds>());
        CHECK_FALSE(json(1e300, semantic_tag::epoch_second).try_as<std::chrono::milliseconds>());
        CHECK_FALSE(json(1e300, semantic_tag::epoch_milli).try_as<std::chrono::milliseconds>());
        CHECK_FALSE(json(1e300, semantic_tag::epoch_nano).try_as<std::chrono::milliseconds>());
        CHECK_FALSE(json(1e300).try_as<std::chrono::milliseconds>());
        CHECK_FALSE(json("9223372036854776", semantic_tag::epoch_second).try_as<std::chrono::milliseconds>());
        CHECK_FALSE(json("9223372036854775808000000", semantic_tag::epoch_nano).try_as<std::chrono::milliseconds>());
        CHECK_FALSE(json(uint64_t(9223372036854775808ull), semantic_tag::epoch_milli).try_as<std::chrono::milliseconds>());

        CHECK(json(int64_t(9223372036854775), semantic_tag::epoch_second).as<std::chrono::milliseconds>().count() == 9223372036854775000);
        CHECK(json("-9223372036854775", semantic_tag::epoch_second).as<std::chrono::milliseconds>().count() == -9223372036854775000);
        CHECK(json(uint64_t(10000000000000000000ull), semantic_tag::epoch_nano).as<std::chrono::milliseconds>().count() == 10000000000000);
    }
    SECTION("std::chrono::nanoseconds")
    {
        auto result = json(int64_t(253402300799999), semantic_tag::epoch_milli).try_as<std::chrono::nanoseconds>();
        REQUIRE_FALSE(result);
        CHECK(conv_errc::not_epoch == result.error().code());

        CHECK_FALSE(json(int64_t(9223372037), semantic_tag::epoch_second).try_as<std::chrono::nanoseconds>());
        CHECK_FALSE(json(9.3e9, semantic_tag::epoch_second).try_as<std::chrono::nanoseconds>());
        CHECK_FALSE(json(9.3e12, semantic_tag::epoch_milli).try_as<std::chrono::nanoseconds>());
        CHECK_FALSE(json(1e300).try_as<std::chrono::nanoseconds>());
        CHECK_FALSE(json("9223372037", semantic_tag::epoch_second).try_as<std::chrono::nanoseconds>());
        CHECK_FALSE(json("9223372036855", semantic_tag::epoch_milli).try_as<std::chrono::nanoseconds>());
        CHECK_FALSE(json(1e300, semantic_tag::epoch_nano).try_as<std::chrono::nanoseconds>());
        CHECK_FALSE(json(uint64_t(9223372036854775808ull), semantic_tag::epoch_nano).try_as<std::chrono::nanoseconds>());

        CHECK(json(int64_t(9223372036), semantic_tag::epoch_second).as<std::chrono::nanoseconds>().count() == 9223372036000000000);
        CHECK(json(int64_t(9223372036854), semantic_tag::epoch_milli).as<std::chrono::nanoseconds>().count() == 9223372036854000000);
        CHECK(json(1.5, semantic_tag::epoch_second).as<std::chrono::nanoseconds>().count() == 1500000000);
        CHECK(json(5000000001.0, semantic_tag::epoch_second).as<std::chrono::nanoseconds>().count() == 5000000001000000000);
        CHECK(json(0.1, semantic_tag::epoch_second).as<std::chrono::duration<long double, std::nano>>().count() == static_cast<long double>(0.1) * 1000000000);
    }
}
