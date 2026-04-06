// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons_ext/toon/decode_toon.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <iostream>

namespace toon = jsoncons::toon;

TEST_CASE("decode_toon tests")
{
    SECTION("test 1")
    {
        auto expected = jsoncons::ojson::parse(R"({
    "context": {
        "task": "Our favorite hikes together",
        "location": "Boulder",
        "season": "spring_2025"
    },
    "friends": [
        "ana",
        "luis",
        "sam"
    ],
    "hikes": [
        {
            "id": 1,
            "name": "Blue Lake Trail",
            "distanceKm": 7.5,
            "elevationGain": 320,
            "companion": "ana",
            "wasSunny": true
        },
        {
            "id": 2,
            "name": "Ridge Overlook",
            "distanceKm": 9.2,
            "elevationGain": 540,
            "companion": "luis",
            "wasSunny": false
        },
        {
            "id": 3,
            "name": "Wildflower Loop",
            "distanceKm": 5.1,
            "elevationGain": 180,
            "companion": "sam",
            "wasSunny": true
        }
    ]
})");

        std::string toon_str = R"(context:
  task: Our favorite hikes together
  location: Boulder
  season: spring_2025
friends[3]: ana,luis,sam
hikes[3]{id,name,distanceKm,elevationGain,companion,wasSunny}:
  1,Blue Lake Trail,7.5,320,ana,true
  2,Ridge Overlook,9.2,540,luis,false
  3,Wildflower Loop,5.1,180,sam,true)";

        auto result = toon::try_decode_toon<jsoncons::ojson>(toon_str);
        REQUIRE(result);
        CHECK(expected == *result);
        //std::cout << pretty_print(*result) << "\n";
    }
    SECTION("test 2")
    {
        auto expected = jsoncons::ojson::parse(R"({
    "orders": [
        {
            "orderId": "ORD-001",
            "customer": {
                "name": "Alice Chen",
                "email": "alice@example.com"
            },
            "items": [
                {
                    "sku": "WIDGET-A",
                    "quantity": 2,
                    "price": 29.99
                },
                {
                    "sku": "GADGET-B",
                    "quantity": 1,
                    "price": 49.99
                }
            ],
            "total": 109.97,
            "status": "shipped"
        },
        {
            "orderId": "ORD-002",
            "customer": {
                "name": "Bob Smith",
                "email": "bob@example.com"
            },
            "items": [
                {
                    "sku": "THING-C",
                    "quantity": 3,
                    "price": 15
                }
            ],
            "total": 45,
            "status": "delivered"
        }
    ]
})");

        std::string toon_str = R"(orders[2]:
  - orderId: ORD-001
    customer:
      name: Alice Chen
      email: alice@example.com
    items[2]{sku,quantity,price}:
      WIDGET-A,2,29.99
      GADGET-B,1,49.99
    total: 109.97
    status: shipped
  - orderId: ORD-002
    customer:
      name: Bob Smith
      email: bob@example.com
    items[1]{sku,quantity,price}:
      THING-C,3,15
    total: 45
    status: delivered)";

        std::stringstream is(toon_str);
        auto result = toon::try_decode_toon<jsoncons::ojson>(is);
        REQUIRE(result);
        CHECK(expected == *result);
    }
}
