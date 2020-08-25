// Copyright 2016 Daniel Parker
// Distributed under Boost license

#include <cassert>
#include <string>
#include <climits>
#include <iostream>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>

using namespace jsoncons;

namespace {

    void json_example()
    {
         std::bitset<70> bs1(ULLONG_MAX);

         std::string s;
         encode_json(bs1, s);
         std::cout << s << "\n\n";

         auto bs2 = decode_json<std::bitset<70>>(s);

         assert(bs2 == bs1);
    }

    void cbor_example()
    {
        std::bitset<8> bs1(42);

        std::vector<uint8_t> data;
        cbor::encode_cbor(bs1, data);
        std::cout << byte_string_view(data) << "\n\n";
        /*
          0xd7, // Expected conversion to base16
            0x41, // Byte string value of length 1 
              0x54
        */

        auto bs2 = cbor::decode_cbor<std::bitset<8>>(data);

        assert(bs2 == bs1);
    }

} // namespace

void json_traits_bitset_examples()
{
    std::cout << "\njson traits bitset examples\n\n";

    json_example();
    cbor_example();

    std::cout << std::endl;
}

