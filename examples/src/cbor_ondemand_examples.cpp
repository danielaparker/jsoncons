// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons_ext/cbor/cbor_ondemand.hpp>
#include <iostream>
#include <vector>

namespace cbor = jsoncons::cbor;

int cbor_ondemand_example()
{
    // {"id": 42, "scores": [1, 2]}
    const std::vector<uint8_t> data = {
        0xa2,
        0x62, 'i', 'd', 0x18, 0x2a,
        0x66, 's', 'c', 'o', 'r', 'e', 's', 0x82, 0x01, 0x02
    };

    auto item = cbor::ondemand::read_item(jsoncons::span<const uint8_t>(data));
    if (!item)
    {
        std::cout << "error " << item.error().message() << "\n";
        return 1;
    }

    bool is_key = true;
    std::string_view key;
    for (const cbor::ondemand::cbor_item child : item.value().children())
    {
        if (is_key)
        {
            key = child.get_string().value();
        }
        else if (child.type() == cbor::ondemand::cbor_type::array)
        {
            std::cout << key << ":";
            for (const cbor::ondemand::cbor_item element : child.children())
            {
                uint64_t value = element.get_uint64().value();
                std::cout << " " << value;
            }
            std::cout << "\n";
        }
        else
        {
            uint64_t value = child.get_uint64().value();
            std::cout << key << ": " << value << "\n";
        }
        is_key = !is_key;
    }
    return 0;
}

int definite_length_string_invalid_utf8()
{
    std::vector<uint8_t> data = {
        0x63,             // CBOR text string, length 3
        0x61, 0xFF, 0x62  // 'a', invalid UTF-8 byte, 'b'
    };

    auto r = cbor::ondemand::read_item(data);
    std::string buf;
    auto str = (*r).get_string(buf);
    if (str)
    {
        std::cout << buf << "\n";
    }
    else
    {
        std::cout << str.error().message() << "\n";
    }
    return 0;
}

int indefinite_length_string_invalid_utf8()
{
    std::vector<uint8_t> data = {
        0x7F,             // Indefinite-length CBOR text string
        0x62, 0x61, 0xC3, // Chunk 1: 'a' + first byte of UTF-8 sequence
        0x62, 0xA9, 0x62, // Chunk 2: second byte of UTF-8 sequence + 'b'
        0xFF              // Break
    };

    auto r = cbor::ondemand::read_item(data);
    std::string buf;
    auto str = (*r).get_string(buf);
    if (str)
    {
        std::cout << buf << "\n";
    }
    else
    {
        std::cout << str.error().message() << "\n";
    }
    return 0;
}

int main()
{
    std::cout << "cbor::ondemand examples\n\n";

    cbor_ondemand_example();
    std::cout << "\n";
    definite_length_string_invalid_utf8();
    std::cout << "\n";
    indefinite_length_string_invalid_utf8();
}

