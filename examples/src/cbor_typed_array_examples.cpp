// Copyright 2017 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonpath/json_query.hpp>
#include "sample_types.hpp"
#include <string>
#include <iomanip>
#include <cassert>

using namespace jsoncons;

namespace cbor_typed_array_examples {

void float64_big_endian_example()
{
    const std::vector<uint8_t> input = {
      0xd8,0x52, // Tag 82 (float64 big endian Typed Array)
        0x50,    // Byte string value of length 16
            0xff, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0x7f, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };

    auto j = cbor::decode_cbor<json>(input);
    std::cout << "(1)\n" << pretty_print(j) << "\n\n";

    auto v = cbor::decode_cbor<std::vector<double>>(input);
    std::cout << "(2)\n";
    for (auto item : v)
    {
        std::cout << std::defaultfloat << item << "\n";
    }
}

void mult_dim_row_major_example()
{
    const std::vector<uint8_t> input = {
      0xd8,0x28,     // Tag 40 (multi-dimensional row major array)
        0x82,        // array(2)
          0x82,      // array(2)
            0x02,    // unsigned(2) 1st Dimension
            0x03,    // unsigned(3) 2nd Dimension
          0xd8,0x41,     // Tag 65 (uint16 big endian Typed Array)
            0x4c,        // byte string(12)
              0x00,0x02, // unsigned(2)
              0x00,0x04, // unsigned(4)
              0x00,0x08, // unsigned(8)
              0x00,0x04, // unsigned(4)
              0x00,0x10, // unsigned(16)
              0x01,0x00  // unsigned(256)
    };

    json j = cbor::decode_cbor<json>(input);

    std::cout << "Tag: " << j.tag() << "\n\n";
    std::cout << pretty_print(j) << "\n";
}

}; // cbor_typed_array_examples

void run_cbor_typed_array_examples()
{
    std::cout << "\ncbor typed array examples\n\n";
    cbor_typed_array_examples::float64_big_endian_example();
    cbor_typed_array_examples::mult_dim_row_major_example();
    std::cout << "\n\n";
}

