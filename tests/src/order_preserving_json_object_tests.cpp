// Copyright 2019 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <map>
#include <assert.h>

using namespace jsoncons;

TEST_CASE("order preserving insert")
{
    json_object<std::string, ojson> o;

    typedef std::pair<ojson::key_storage_type,ojson> item_type;
    std::vector<item_type> items;
    items.emplace_back("b", 1);
    items.emplace_back("a", 2);
    items.emplace_back("c", 3);
    items.emplace_back("a", 4);
    items.emplace_back("a", 5);
    items.emplace_back("d", 6);
    items.emplace_back("a", 7);

    o.insert(std::make_move_iterator(items.begin()), std::make_move_iterator(items.end()), 
             [](item_type&& item){return ojson::key_value_type(std::move(item.first),std::move(item.second));});

    SECTION("iterate")
    {
        REQUIRE(o.size() == 4);

        auto it = o.begin();
        CHECK(it->key() == std::string("b"));
        CHECK(it->value().as<int>() == 1);
        CHECK((++it)->key() == std::string("a"));
        CHECK(it->value().as<int>() == 2);
        CHECK((++it)->key() == std::string("c"));
        CHECK(it->value().as<int>() == 3);
        CHECK((++it)->key() == std::string("d"));
        CHECK(it->value().as<int>() == 6);
    }

    SECTION("find")
    {
        auto it1 = o.find("a");
        REQUIRE(bool(it1 != o.end()));
        CHECK(it1->value().as<int>() == 2);

        auto it2 = o.find("b");
        REQUIRE(bool(it2 != o.end()));
        CHECK(it2->value().as<int>() == 1);

        auto it3 = o.find("c");
        REQUIRE(bool(it3 != o.end()));
        CHECK(it3->value().as<int>() == 3);

        auto it4 = o.find("d");
        REQUIRE(bool(it4 != o.end()));
        CHECK(it4->value().as<int>() == 6);
    }
}

TEST_CASE("order preserving insert_or_assign")
{
    json_object<std::string, ojson> o;

    o.insert_or_assign("b", 1);
    o.insert_or_assign("a", 2);
    o.insert_or_assign("c", 3);
    o.insert_or_assign("a", 4);
    o.insert_or_assign("a", 5);

    SECTION("insert_or_assign")
    {
        REQUIRE(o.size() == 3);

        auto it = o.find("a");
        REQUIRE(bool(it != o.end()));
        CHECK(it->value().as<int>() == 5);

        auto it2 = o.begin();
        CHECK(it2->key() == std::string("b"));
        CHECK(it2->value().as<int>() == 1);
        CHECK((++it2)->key() == std::string("a"));
        CHECK(it2->value().as<int>() == 5);
        CHECK((++it2)->key() == std::string("c"));
        CHECK(it2->value().as<int>() == 3);
    }

    SECTION("insert_or_assign at pos")
    {
        auto it = o.find("a");
        auto it2 = o.insert_or_assign(it,"d",3);

        auto it3 = o.begin();
        CHECK(it3->key() == std::string("b"));
        CHECK(it3->value().as<int>() == 1);
        CHECK((++it3)->key() == std::string("d"));
        CHECK(it3->value().as<int>() == 3);
        CHECK((++it3)->key() == std::string("a"));
        CHECK(it3->value().as<int>() == 5);
        CHECK((++it3)->key() == std::string("c"));
        CHECK(it3->value().as<int>() == 3);

        //for (auto kv : o)
        //{
        //    std::cout << kv.key() << ": " << kv.value() << "\n";
        //}
    }

    SECTION("try_emplace")
    {
        REQUIRE(o.size() == 3);

        auto it = o.find("a");
        o.try_emplace("d",7);
        o.try_emplace("d",8);

        auto it3 = o.begin();
        CHECK(it3->key() == std::string("b"));
        CHECK(it3->value().as<int>() == 1);
        CHECK((++it3)->key() == std::string("a"));
        CHECK(it3->value().as<int>() == 5);
        CHECK((++it3)->key() == std::string("c"));
        CHECK(it3->value().as<int>() == 3);
        CHECK((++it3)->key() == std::string("d"));
        CHECK(it3->value().as<int>() == 7);
    }

    SECTION("try_emplace at pos")
    {
        auto it = o.find("a");
        auto it2 = o.try_emplace(it,"d",7);
        o.try_emplace(it2, "d", 8);

        auto it3 = o.begin();
        CHECK(it3->key() == std::string("b"));
        CHECK(it3->value().as<int>() == 1);
        CHECK((++it3)->key() == std::string("d"));
        CHECK(it3->value().as<int>() == 7);
        CHECK((++it3)->key() == std::string("a"));
        CHECK(it3->value().as<int>() == 5);
        CHECK((++it3)->key() == std::string("c"));
        CHECK(it3->value().as<int>() == 3);
    }

    SECTION("erase")
    {
        REQUIRE(o.size() == 3);

        o.erase("a");
        REQUIRE(o.size() == 2);

        auto it2 = o.begin();
        CHECK(it2->key() == std::string("b"));
        CHECK(it2->value().as<int>() == 1);
        CHECK((++it2)->key() == std::string("c"));
        CHECK(it2->value().as<int>() == 3);
    }

    SECTION("erase range")
    {
        REQUIRE(o.size() == 3);

        o.erase(o.begin(),o.begin()+2);
        REQUIRE(o.size() == 1);

        auto it2 = o.begin();
        CHECK(it2->key() == std::string("c"));
        CHECK(it2->value().as<int>() == 3);
    }
}

