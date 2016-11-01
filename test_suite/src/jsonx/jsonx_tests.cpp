// Copyright 2016 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <jsoncons/json.hpp>
#include <boost/test/unit_test.hpp>
#include "jsoncons_ext/jsonx/jsonx_serializer.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <fstream>

using namespace jsoncons;
using namespace jsoncons::jsonx;

BOOST_AUTO_TEST_SUITE(jsonx_test_suite)

BOOST_AUTO_TEST_CASE(serialize_jsonx)
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

BOOST_AUTO_TEST_CASE(serialize_special_characters)
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

BOOST_AUTO_TEST_SUITE_END()

