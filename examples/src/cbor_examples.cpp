// Copyright 2017 Daniel Parker
// Distributed under Boost license

#include <string>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>

using namespace jsoncons;

void decode_cbor_byte_string()
{
    // byte string for five bytes
    std::vector<uint8_t> bs = {0x45,'H','e','l','l','o'};
    json j = cbor::decode_cbor<json>(bs);

    auto bs = j.as<byte_string>();
    std::cout << "(1) ";
    for (auto b : bs)
    {
        std::cout << (char)b;
    }
    std::cout << std::endl;

    // byte string value to json becomes base64url
    std::cout << "(2) " << j << std::endl;
}

void encode_cbor_byte_string()
{
    // construct byte string value
    json j(byte_string("Hello"));

    std::vector<uint8_t> bs = cbor::encode_cbor(j);
    std::cout << "(1) ";

    std::cout << std::hex << (int)bs[0];
    for (size_t i = 1; i < bs.size(); ++i)
    {
        std::cout << (char)bs[i];
    }
    std::cout << std::endl;

    // byte string value to json becomes base64url
    std::cout << "(2) " << j << std::endl;
}

void cbor_examples()
{
    std::cout << "\ncbor examples\n\n";
    decode_cbor_byte_string();
    encode_cbor_byte_string();
    std::cout << std::endl;
}

