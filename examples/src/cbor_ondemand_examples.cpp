// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons_ext/cbor/cbor_ondemand.hpp>
#include <jsoncons/utility/byte_string.hpp>
#include <iostream>
#include <vector>
#include <string>

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
    for (const auto child : item.value().children())
    {
        if (is_key)
        {
            key = child.get_string().value();
        }
        else if (child.type() == cbor::ondemand::cbor_type::array)
        {
            std::cout << key << ":";
            for (const auto element : child.children())
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

int indefinite_length_byte_string()
{
    std::vector<uint8_t> data = {
        0x5F,                   // Start indefinite-length byte string
        0x42, 0x01, 0x02,       // Chunk 1: 2 bytes
        0x43, 0x03, 0x04, 0x05, // Chunk 2: 3 bytes
        0xFF                    // Break
    };

    auto item = cbor::ondemand::read_item(data);
    if (!item)
    {
        return 1;
    }
    if (auto bstr = (*item).get_byte_string(); bstr)
    {
        std::cout << "(1) ";
        jsoncons::print_bytes(*bstr, std::cout);
        std::cout << "\n";
    }
    else
    {
        std::cout << "(1) " << bstr.error().message() << "\n";
    }
    std::cout << "\n";

    std::vector<uint8_t> buf;
    if (auto bstr = (*item).get_byte_string(buf); bstr)
    {
        std::cout << "(2) ";
        jsoncons::print_bytes(buf, std::cout);
        std::cout << "\n";
    }
    else
    {
        std::cout << "(2) " << bstr.error().message() << "\n";
    }
    return 0;
}

int definite_length_string_invalid_utf8()
{
    std::vector<uint8_t> data = {
        0x63,             // CBOR text string, length 3
        0x61, 0xFF, 0x62  // 'a', invalid UTF-8 byte, 'b'
    };

    auto item = cbor::ondemand::read_item(data);
    if (!item)
    {
        return 1;
    }
    std::string buf;
    if (auto str = (*item).get_string(buf); str)
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

    auto item = cbor::ondemand::read_item(data);
    if (!item)
    {
        return 1;
    }
    std::string buf;
    if (auto str = (*item).get_string(buf); str)
    {
        std::cout << buf << "\n";
    }
    else
    {
        std::cout << str.error().message() << "\n";
    }
    return 0;
}

int mapped_item_by_string_key()
{
    const std::vector<uint8_t> data = {
0xA3,                         // Map with 3 key-value pairs
0x63, 0x66, 0x6F, 0x6F,       // Key: "foo"
0x01,                         // Value: 1
0x63, 0x62, 0x61, 0x72,       // Key: "bar"
0x02,                         // Value: 2
0x63, 0x62, 0x61, 0x7A,       // Key: "baz"
0x03                          // Value: 3
    };

    auto item = cbor::ondemand::read_item(data);   
    if (!item)
    {
        return 1;
    }
    if (auto bar = (*item).at("bar"); bar)
    {
        if (auto val = (*bar).get_uint64(); val)
        {
            std::cout << "(1) " << *val << "\n";
        }
    }
    else
    {
        std::cout << "(1) " << bar.error().message() << "\n";
    }
    if (auto qux = (*item).at("qux"); qux)
    {
        if (auto val = (*qux).get_uint64(); val)
        {
            std::cout << "(2) " << *val << "\n";
        }
    }
    else
    {
        std::cout << "(2) " << qux.error().message() << "\n";
    }
    return 0;
}

int mapped_item_by_unsigned_integer_key()
{
    const std::vector<uint8_t> data = {
0xA3,       // Map with 3 key-value pairs
0x01,       // Key: unsigned integer 1
0x0A,       // Value: 10
0x02,       // Key: unsigned integer 2
0x14,       // Value: 20
0x03,       // Key: unsigned integer 3
0x18, 0x1E  // Value: 30
    };

    auto item = cbor::ondemand::read_item(data);   
    if (!item)
    {
        return 1;
    }
    if (auto bar = (*item).at(1u); bar)
    {
        if (auto val = (*bar).get_uint64(); val)
        {
            std::cout << "(1) " << *val << "\n";
        }
    }
    else
    {
        std::cout << "(1) " << bar.error().message() << "\n";
    }
    if (auto qux = (*item).at(4u); qux)
    {
        if (auto val = (*qux).get_uint64(); val)
        {
            std::cout << "(2) " << *val << "\n";
        }
    }
    else
    {
        std::cout << "(2) " << qux.error().message() << "\n";
    }
    return 0;
}

int main()
{
    std::cout << "cbor::ondemand examples\n\n";

    cbor_ondemand_example();
    std::cout << "\n";
    indefinite_length_byte_string();
    std::cout << "\n";
    definite_length_string_invalid_utf8();
    std::cout << "\n";
    indefinite_length_string_invalid_utf8();
    std::cout << "\n";
    mapped_item_by_string_key();
    std::cout << "\n";
    mapped_item_by_unsigned_integer_key();
}

