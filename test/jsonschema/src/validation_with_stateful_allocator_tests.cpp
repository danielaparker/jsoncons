// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#include <jsoncons_ext/jsonschema/jsonschema.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/utility/byte_string.hpp>

#include <fstream>
#include <iostream>
#include <regex>
#include <catch/catch.hpp>

namespace jsonschema = jsoncons::jsonschema;

#if defined(JSONCONS_HAS_STATEFUL_ALLOCATOR) && JSONCONS_HAS_STATEFUL_ALLOCATOR == 1

#include <common/mock_stateful_allocator.hpp>
#include <scoped_allocator>

template <typename T>
using MyScopedAllocator = std::scoped_allocator_adaptor<mock_stateful_allocator<T>>;

using cust_json = jsoncons::basic_json<char,jsoncons::sorted_policy,MyScopedAllocator<char>>;

TEST_CASE("jsonschema validator tests")
{
    SECTION("Basic")
    {
        auto schema = jsoncons::json::parse(R"(
{
  "$id": "https://example.com/polygon",
  "$schema": "http://json-schema.org/draft-07/schema#",
  "$defs": {
    "point": {
      "type": "object",
      "properties": {
        "x": { "type": "number" },
        "y": { "type": "number" }
      },
      "additionalProperties": false,
      "required": [ "x", "y" ]
    }
  },
  "type": "array",
  "items": { "$ref": "#/$defs/point" },
  "minItems": 3,
  "maxItems": 1
}

        )");

        MyScopedAllocator<char> alloc(1);

        auto instance = cust_json::parse(jsoncons::make_alloc_set(alloc), R"(
[
  {
    "x": 2.5,
    "y": 1.3
  },
  {
    "x": 1,
    "z": 6.7
  }
]
        )");

        jsonschema::json_schema<jsoncons::json> compiled = jsonschema::make_json_schema(schema);

        bool result = compiled.is_valid(instance);
        CHECK(result);
    }
}

#endif

