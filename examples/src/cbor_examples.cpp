// Copyright 2017 Daniel Parker
// Distributed under Boost license

#include <string>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using namespace jsoncons;

void cbor_reputon_example()
{
    ojson j1 = ojson::parse(R"(
    {
       "application": "hiking",
       "reputons": [
       {
           "rater": "HikingAsylum.example.com",
           "assertion": "is-good",
           "rated": "sk",
           "rating": 0.90
         }
       ]
    }
    )");

    // Encoding an unpacked (json) value to a packed CBOR value
    std::vector<uint8_t> data;
    cbor::encode_cbor(j1, data);

    // Decoding a packed CBOR value to an unpacked (json) value
    ojson j2 = cbor::decode_cbor<ojson>(data);
    std::cout << "(1)\n" << pretty_print(j2) << "\n\n";

    // Iterating over and accessing the nested data items of a packed CBOR value
    cbor::cbor_view datav{data};    
    cbor::cbor_view reputons = datav.at("reputons");    

    std::cout << "(2)\n";
    for (auto element : reputons.array_range())
    {
        std::cout << element.at("rated").as_string() << ", ";
        std::cout << element.at("rating").as_double() << "\n";
    }
    std::cout << std::endl;

    // Querying a packed CBOR value for a nested data item with jsonpointer
    std::error_code ec;
    cbor::cbor_view rated = jsonpointer::get(datav, "/reputons/0/rated", ec);
    if (!ec)
    {
        std::cout << "(3) " << rated.as_string() << "\n";
    }

    std::cout << std::endl;
}

void decode_cbor_byte_string()
{
    // byte string for five bytes
    std::vector<uint8_t> buf = {0x45,'H','e','l','l','o'};
    json j = cbor::decode_cbor<json>(buf);

    auto bs = j.as<byte_string>();

    // byte_string to ostream displays as hex
    std::cout << "(1) "<< bs << "\n\n";

    // byte string value to JSON text becomes base64url
    std::cout << "(2) " << j << std::endl;
}

void encode_cbor_byte_string()
{
    // construct byte string value
    json j(byte_string("Hello"));

    std::vector<uint8_t> buf;
    cbor::encode_cbor(j, buf);

    std::cout << std::hex << std::showbase << (int)buf[0];
    for (size_t i = 1; i < buf.size(); ++i)
    {
        std::cout << (char)buf[i];
    }
    std::cout << std::dec << std::endl;
}

void cbor_view_object_range()
{
    // {\"foo\": [0, 1, 2],\"bar\": [3,4,5]}

    std::vector<uint8_t> packed = {0xa2,0x63,'f','o','o',0x83,'\0','\1','\2',0x63,'b','a','r',0x83,'\3','\4','\5'};

    cbor::cbor_view v{packed};
    for (auto member : v.object_range())
    {
        std::cout << member.key() << ":" << std::endl;
        for (auto element : member.value().array_range())
        {
            std::cout << element.as_integer() << std::endl;
        }
    }
}

void cbor_view_array_range()
{
    // [\"Toronto\",\"Vancouver\",\"Montreal\"]

    std::vector<uint8_t> packed = {0x83,0x67,'T','o','r','o','n','t','o',0x69,'V','a','n','c','o','u','v','e','r',0x68,'M','o','n','t','r','e','a','l'};

    cbor::cbor_view v{packed};
    for (auto element : v.array_range())
    {
        std::cout << element.as_string() << std::endl;
    }
}

void cbor_examples()
{
    std::cout << "\ncbor examples\n\n";
    decode_cbor_byte_string();
    encode_cbor_byte_string();
    cbor_view_object_range();
    cbor_view_array_range();
    cbor_reputon_example();
    std::cout << std::endl;
}

