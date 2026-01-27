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

using namespace jsoncons;

TEST_CASE("toon_reader tests")
{
    SECTION("test 1")
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

      json_decoder<json> decoder;
      toon::toon_string_reader reader(data, decoder);
      reader.read();  

      REQUIRE(0 == reader.blank_lines().size());
      REQUIRE(9 == reader.lines().size());
      CHECK(0 == reader.lines()[0].indent);
      CHECK("context:" == reader.lines()[0].content);
      CHECK(2 == reader.lines()[1].indent);
      CHECK("task: Our favorite hikes together" == reader.lines()[1].content);
      CHECK(2 == reader.lines()[2].indent);
      CHECK(2 == reader.lines()[3].indent);
      CHECK("season: spring_2025" == reader.lines()[3].content);
      CHECK(0 == reader.lines()[4].indent);
      CHECK(0 == reader.lines()[5].indent);
      CHECK(2 == reader.lines()[6].indent);
      CHECK(2 == reader.lines()[7].indent);
      CHECK(2 == reader.lines()[8].indent);

    }
}
