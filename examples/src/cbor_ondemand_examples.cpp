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

    auto parsed = cbor::ondemand::read_item(jsoncons::span<const uint8_t>(data));
    if (!parsed)
    {
        std::cout << "error " << parsed.error().message() << "\n";
        return 1;
    }

    bool is_key = true;
    std::string_view key;
    for (const cbor::ondemand::item child : parsed.value().children())
    {
        if (is_key)
        {
            key = child.get_string().value();
        }
        else if (child.type() == cbor::ondemand::item_type::array)
        {
            std::cout << key << ":";
            for (const cbor::ondemand::item element : child.children())
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

int main()
{
    std::cout << "\ncbor examples\n\n";

    cbor_ondemand_example();
}

