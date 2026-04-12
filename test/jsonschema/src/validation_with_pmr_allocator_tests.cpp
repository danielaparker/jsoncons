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

#if defined(JSONCONS_HAS_POLYMORPHIC_ALLOCATOR) && JSONCONS_HAS_POLYMORPHIC_ALLOCATOR == 1
#include <memory_resource> 

TEST_CASE("jsonschema pmr allocator tests")
{
    SECTION("Basic")
    {
        char buffer[1024] = {}; // a small buffer on the stack
        //char* last = buffer + sizeof(buffer);
        std::pmr::monotonic_buffer_resource pool{ std::data(buffer), std::size(buffer) };
        std::pmr::polymorphic_allocator<char> alloc(&pool);

        auto schema = jsoncons::pmr::json::parse(jsoncons::make_alloc_set(alloc), R"(
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

        auto instance = jsoncons::pmr::json::parse(jsoncons::make_alloc_set(alloc), R"(
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

        auto compiled = jsonschema::make_json_schema(schema);

        bool result = compiled.is_valid(instance);
        CHECK_FALSE(result);
    }
}

#endif

