// Copyright 2017 Daniel Parker
// Distributed under Boost license

#include <string>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>

using namespace jsoncons;

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
    std::cout << std::endl;
}

