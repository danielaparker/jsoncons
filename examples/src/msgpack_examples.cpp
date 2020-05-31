// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <string>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/msgpack/msgpack.hpp>

using namespace jsoncons;

namespace {

    void example1()
    {
    ojson j1 = ojson::parse(R"(
    [
      { "category": "reference",
        "author": "Nigel Rees",
        "title": "Sayings of the Century",
        "price": 8.95
      },
      { "category": "fiction",
        "author": "Evelyn Waugh",
        "title": "Sword of Honour",
        "price": 12.99
      }
    ]
    )");

        std::vector<uint8_t> v;
        msgpack::encode_msgpack(j1, v);

        ojson j2 = msgpack::decode_msgpack<ojson>(v);

        std::cout << pretty_print(j2) << std::endl;

        json j3 = msgpack::decode_msgpack<json>(v);

        std::cout << pretty_print(j3) << std::endl;

        std::cout << std::endl;

        //wjson j4 = msgpack::decode_msgpack<wjson>(v);

        //std::wcout << pretty_print(j4) << std::endl;

        //std::cout << std::endl;
    }

    void example2()
    {
        ojson j1;
        j1["zero"] = 0;
        j1["one"] = 1;
        j1["two"] = 2;
        j1["null"] = null_type();
        j1["true"] = true;
        j1["false"] = false;
        j1["max int64_t"] = (std::numeric_limits<int64_t>::max)();
        j1["max uint64_t"] = (std::numeric_limits<uint64_t>::max)();
        j1["min int64_t"] = (std::numeric_limits<int64_t>::lowest)();
        j1["max int32_t"] = (std::numeric_limits<int32_t>::max)();
        j1["max uint32_t"] = (std::numeric_limits<uint32_t>::max)();
        j1["min int32_t"] = (std::numeric_limits<int32_t>::lowest)();
        j1["max int16_t"] = (std::numeric_limits<int16_t>::max)();
        j1["max uint16_t"] = (std::numeric_limits<uint16_t>::max)();
        j1["min int16_t"] = (std::numeric_limits<int16_t>::lowest)();
        j1["max int8_t"] = (std::numeric_limits<int8_t>::max)();
        j1["max uint8_t"] = (std::numeric_limits<uint8_t>::max)();
        j1["min int8_t"] = (std::numeric_limits<int8_t>::lowest)();
        j1["max double"] = (std::numeric_limits<double>::max)();
        j1["min double"] = (std::numeric_limits<double>::lowest)();
        j1["max float"] = (std::numeric_limits<float>::max)();
        j1["zero float"] = 0.0;
        j1["min float"] = (std::numeric_limits<float>::lowest)();
        j1["Key too long for small string optimization"] = "String too long for small string optimization";

        std::vector<uint8_t> v;
        msgpack::encode_msgpack(j1, v);

        ojson j2 = msgpack::decode_msgpack<ojson>(v);

        std::cout << pretty_print(j2) << std::endl;

        std::cout << std::endl;
    }

    void ext_example()
    {
        std::vector<uint8_t> input = {

            0x82, // map, length 2
              0xa5, // string, length 5
                'H','e','l','l','o',
              0xa5, // string, length 5
                'W','o','r','l','d',
              0xa4, // string, length 4
                 'D','a','t','a',
              0xc7, // ext8 format code
                0x06, // length 6
                0x07, // type
                  'f','o','o','b','a','r'
        };

        ojson j = msgpack::decode_msgpack<ojson>(input);

        std::cout << "(1)\n" << pretty_print(j) << "\n\n";
        std::cout << "(2) " << j["Data"].tag() << "("  << j["Data"].ext_tag() << ")\n\n";
        
        // Get ext value as a std::vector<uint8_t>
        auto v = j["Data"].as<std::vector<uint8_t>>(); 

        std::cout << "(3)\n";
        std::cout << byte_string_view(v) << "\n\n";

        std::vector<uint8_t> output;
        msgpack::encode_msgpack(j,output);
        assert(output == input);
    }

} // namespace

void msgpack_examples()
{
    std::cout << "\nmsgpack examples\n\n";
    example1();
    example2();
    ext_example();
    std::cout << std::endl;
}

