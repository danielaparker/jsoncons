// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <string>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonx/jsonx_serializer.hpp>

using namespace jsoncons;
using namespace jsoncons::jsonx;

void serialize_to_jsonx()
{
    json person = json::parse(R"(
{
  "name":"John Smith",
  "address": {
    "streetAddress": "21 2nd Street",
    "city": "New York",
    "state": "NY",
    "postalCode": 10021
  },
  "phoneNumbers": [
    "212 555-1111",
    "212 555-2222"
  ],
  "additionalInfo": null,
  "remote": false,
  "height": 62.4,
  "ficoScore": " > 640"
}
    )");

    jsonx_serializer serializer(std::cout,true);

    person.write(serializer);
}

void serialize_special_characters_to_jsonx()
{
    ojson special_chars = ojson::parse(R"(
{
  "&":"&",
  "<":"<",
  ">":">",
  "\"":"\"",
  "'":"'",
  "\u00A9":""
}
    )");

    jsonx_serializer serializer(std::cout,true);

    special_chars.write(serializer);
}

void jsonx_examples()
{
    std::cout << "\njsonx examples\n\n";
    serialize_to_jsonx();
    serialize_special_characters_to_jsonx();
    std::cout << std::endl;
}

