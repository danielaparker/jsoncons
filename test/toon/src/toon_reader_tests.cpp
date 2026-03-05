// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons_ext/toon/toon_reader.hpp>
#include <jsoncons/json_decoder.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <iostream>

namespace toon = jsoncons::toon;

#if 0

TEST_CASE("toon_reader util tests")
{
    SECTION("find_unquoted_char")
    {
        std::size_t pos;

        pos = toon::find_unquoted_char(R"(context:)", 't');
        CHECK(3 == pos);
        pos = toon::find_unquoted_char(R"(a:b"c:d"e)", ':');
        CHECK(1 == pos);
        pos = toon::find_unquoted_char(R"(a"b:c"d:e)", ':',0);
        CHECK(7 == pos);
        pos = toon::find_unquoted_char(R"("a:b":c)", ':',0);
        CHECK(5 == pos);
    }
    SECTION("find_first_unquoted")
    {
        std::array<char,2> chars = {':',','};
        auto r1 = toon::find_first_unquoted(R"(a:b,c)", chars);
        CHECK(r1.first == 1);
        CHECK(r1.second == ':');
        auto r2 = toon::find_first_unquoted(R"(a"b:c",d)", chars);
        CHECK(r2.first == 6);
        CHECK(r2.second == ',');
    }
}

TEST_CASE("toon_reader read_lines tests")
{
    /*SECTION("read lines")
    {
        std::string data = R"(context:
  task: Our favorite hikes together
  location: Boulder
  season: spring_2025
friends[3]: ana,luis,sam
hikes[3]{id,name,distanceKm,elevationGain,companion,wasSunny}:
  1,Blue Lake Trail,7.5,320,ana,true
  2,Ridge Overlook,9.2,540,luis,false
  3,Wildflower Loop,5.1,180,sam,true)";

      jsoncons::json_decoder<json> decoder;
      toon::toon_string_reader reader(data, decoder);
      reader.read();  

      REQUIRE(0 == reader.blank_lines().size());
      REQUIRE(9 == reader.lines().size());
      CHECK(0 == reader.lines()[0].indent);
      CHECK(0 == reader.lines()[0].depth);
      CHECK("context:" == reader.lines()[0].content);
      CHECK(2 == reader.lines()[1].indent);
      CHECK(1 == reader.lines()[1].depth);
      CHECK("task: Our favorite hikes together" == reader.lines()[1].content);
      CHECK(2 == reader.lines()[2].indent);
      CHECK(1 == reader.lines()[2].depth);
      CHECK(2 == reader.lines()[3].indent);
      CHECK(1 == reader.lines()[3].depth);
      CHECK("season: spring_2025" == reader.lines()[3].content);
      CHECK(0 == reader.lines()[4].indent);
      CHECK(0 == reader.lines()[4].depth);
      CHECK(0 == reader.lines()[5].indent);
      CHECK(0 == reader.lines()[5].depth);
      CHECK(2 == reader.lines()[6].indent);
      CHECK(1 == reader.lines()[6].depth);
      CHECK(2 == reader.lines()[7].indent);
      CHECK(1 == reader.lines()[7].depth);
      CHECK(2 == reader.lines()[8].indent);
      CHECK(1 == reader.lines()[8].depth);

    }
    SECTION("with trailing spaces")
    {
        std::string data = R"(context: 
  task: Our favorite hikes together  
  location: Boulder   
  season: spring_2025 
friends[3]: ana,luis,sam   
hikes[3]{id,name,distanceKm,elevationGain,companion,wasSunny}:   
  1,Blue Lake Trail,7.5,320,ana,true  
  2,Ridge Overlook,9.2,540,luis,false  
  3,Wildflower Loop,5.1,180,sam,true    )";

        json_decoder<json> decoder;
        toon::toon_string_reader reader(data, decoder);
        reader.read();

        REQUIRE(0 == reader.blank_lines().size());
        REQUIRE(9 == reader.lines().size());
        CHECK(0 == reader.lines()[0].indent);
        CHECK(0 == reader.lines()[0].depth);
        CHECK("context:" == reader.lines()[0].content);
        CHECK(2 == reader.lines()[1].indent);
        CHECK(1 == reader.lines()[1].depth);
        CHECK("task: Our favorite hikes together" == reader.lines()[1].content);
        CHECK(2 == reader.lines()[2].indent);
        CHECK(1 == reader.lines()[2].depth);
        CHECK(2 == reader.lines()[3].indent);
        CHECK(1 == reader.lines()[3].depth);
        CHECK("season: spring_2025" == reader.lines()[3].content);
        CHECK(0 == reader.lines()[4].indent);
        CHECK(0 == reader.lines()[4].depth);
        CHECK(0 == reader.lines()[5].indent);
        CHECK(0 == reader.lines()[5].depth);
        CHECK(2 == reader.lines()[6].indent);
        CHECK(1 == reader.lines()[6].depth);
        CHECK(2 == reader.lines()[7].indent);
        CHECK(1 == reader.lines()[7].depth);
        CHECK(2 == reader.lines()[8].indent);
        CHECK(1 == reader.lines()[8].depth);

    }

    SECTION("parse_key")
    {
        std::string key;
        toon::parse_key(R"(" foo")", key);
        CHECK(R"( foo)" == key);
    }*/
}

TEST_CASE("toon_reader parse_delimited_values tests")
{
    SECTION("test 1")
    {
        std::string line = R"(a,b,c)";
        std::vector<jsoncons::string_view> values;
        toon::parse_delimited_values(line, ',', values);

        REQUIRE(3 == values.size());
        CHECK("a" == values[0]);
        CHECK("b" == values[1]);
        CHECK("c" == values[2]);
    }
    SECTION("test 2")
    {
        std::string line = R"(a,"b,c",d)";
        std::vector<jsoncons::string_view> values;
        toon::parse_delimited_values(line, ',', values);

        REQUIRE(3 == values.size());
        CHECK("a" == values[0]);
        CHECK("b,c" == values[1]);
        CHECK("d" == values[2]);
    }
    SECTION("test 3")
    {
        std::string line = R"("a,b",c)";
        std::vector<jsoncons::string_view> values;
        toon::parse_delimited_values(line, ',', values);

        REQUIRE(2 == values.size());
        CHECK("a,b" == values[0]);
        CHECK("c" == values[1]);

        /*for (const auto& value : values)
        {
            std::cout << value << "\n";
        }*/
    }
}
TEST_CASE("toon_reader parse_header tests")
{
    SECTION("test 1")
    {
        std::vector<toon::parsed_line> lines;
        std::vector<toon::blank_line_info> blank_lines;

        std::string raw = R"([3]{id,name,distanceKm,elevationGain,companion,wasSunny}:
  1,Blue Lake Trail,7.5,320,ana,true
  2,Ridge Overlook,9.2,540,luis,false
  3,Wildflower Loop,5.1,180,sam,true)";
        std::error_code ec;
        toon::read_lines(raw, 2, true, lines, blank_lines, ec);

        REQUIRE(4 == lines.size());
        
        auto result = toon::parse_header(lines[0].content);
        REQUIRE(result);
        REQUIRE(*result);
        jsoncons::optional<std::string>& key((*result)->key);
        std::size_t& length{(*result)->length};
        char& delimiter{(*result)->delimiter};
        std::vector<jsoncons::string_view>& fields{(*result)->fields};

        CHECK_FALSE(key);
        CHECK(',' == delimiter);
        REQUIRE(3 == length);
        REQUIRE(6 == fields.size());
        CHECK("id" == fields[0]);
        CHECK("name" == fields[1]);
        CHECK("distanceKm" == fields[2]);
        CHECK("elevationGain" == fields[3]);
        CHECK("companion" == fields[4]);
        CHECK("wasSunny" == fields[5]);

        /*for (const auto& field : fields)
        {
            std::cout << field << "\n";
        }*/

    }
}
#endif

TEST_CASE("toon_reader tests")
{
    /*SECTION("array of primitives")
    {
        auto expected = jsoncons::ojson::parse(R"([" foo", "baz" ,"bar ",1,true,false,null])");
        std::vector<toon::parsed_line> lines;
        std::vector<toon::blank_line_info> blank_lines;

        std::string data = R"([7]: " foo", baz ,"bar ",1,true,false,null)";
        std::error_code ec;

        jsoncons::json_decoder<jsoncons::ojson> decoder;
        toon::toon_string_reader reader(data, decoder);
        reader.read();
        REQUIRE(decoder.is_valid());
        auto result = decoder.get_result();
        CHECK(expected == result);
        //std::cout << pretty_print(result) << "\n";
    }
    SECTION("tabular array")
    {
        auto expected = jsoncons::ojson::parse(R"([
  { "id": 1, "name": "Alice", "role": "admin" },
  { "id": 2, "name": "Bob", "role": "user" }
])");
        std::vector<toon::parsed_line> lines;
        std::vector<toon::blank_line_info> blank_lines;

        std::string data = R"([2]{id,name,role}:
  1,Alice,admin
  2,Bob,user)";
        std::error_code ec;

        jsoncons::json_decoder<jsoncons::ojson> decoder;
        toon::toon_string_reader reader(data, decoder);
        reader.read();
        REQUIRE(decoder.is_valid());
        auto result = decoder.get_result();
        CHECK(expected == result);
        //std::cout << pretty_print(result) << "\n";
    }
    SECTION("list format with hyphen markers")
    {
            auto expected = jsoncons::ojson::parse(R"([
  1,
  { "a": 1 },
  "text"
])");
            std::vector<toon::parsed_line> lines;
            std::vector<toon::blank_line_info> blank_lines;

            std::string data = R"([3]:
  - 1
  - a: 1
  - text)";
            std::error_code ec;

            jsoncons::json_decoder<jsoncons::ojson> decoder;
            toon::toon_string_reader reader(data, decoder);
            reader.read();
            REQUIRE(decoder.is_valid());
            auto result = decoder.get_result();
            CHECK(expected == result);
    }
    SECTION("object")
    {
            auto expected = jsoncons::ojson::parse(R"({
    "task": "Our favorite hikes together",
    "location": "Boulder",
    "season": "spring_2025"
})");
            std::vector<toon::parsed_line> lines;
            std::vector<toon::blank_line_info> blank_lines;

            std::string data = R"(task: Our favorite hikes together
location: Boulder
season: spring_2025)";
            std::error_code ec;

            jsoncons::json_decoder<jsoncons::ojson> decoder;
            toon::toon_string_reader reader(data, decoder);
            reader.read();
            REQUIRE(decoder.is_valid());
            auto result = decoder.get_result();
            CHECK(expected == result);
            //std::cout << pretty_print(result) << "\n";
    }
    SECTION("nested objects")
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
        std::vector<toon::parsed_line> lines;
        std::vector<toon::blank_line_info> blank_lines;

        std::string data = R"(orders[2]:
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
        std::error_code ec;

        jsoncons::json_decoder<jsoncons::ojson> decoder;
        toon::toon_string_reader reader(data, decoder);
        reader.read();
        REQUIRE(decoder.is_valid());
        auto result = decoder.get_result();
        //CHECK(expected == result);
        std::cout << pretty_print(result) << "\n";
    }*/
    SECTION("nested objects 2")
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
    }
  ]
})");
        std::vector<toon::parsed_line> lines;
        std::vector<toon::blank_line_info> blank_lines;

        std::string data = R"(orders[1]:
  - orderId: ORD-001
    total: 109.97
    status: shipped)";

        jsoncons::json_decoder<jsoncons::ojson> decoder;
        toon::toon_string_reader reader(data, decoder);
        reader.read();
        REQUIRE(decoder.is_valid());
        auto result = decoder.get_result();
        //CHECK(expected == result);
        std::cout << pretty_print(result) << "\n";
    }
}