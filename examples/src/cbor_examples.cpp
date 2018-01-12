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
    std::cout << std::endl;
}

void cbor_examples()
{
    std::cout << "\ncbor examples\n\n";
    decode_cbor_byte_string();
    encode_cbor_byte_string();
    std::cout << std::endl;
}

