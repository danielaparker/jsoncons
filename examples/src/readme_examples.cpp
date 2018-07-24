// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <string>
#include <vector>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor_serializer.hpp>
#include <jsoncons_ext/cbor/cbor_view.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

namespace readme
{
    using namespace jsoncons;    

    void example1()
    {
        // Construct some CBOR using the streaming API
        std::vector<uint8_t> b;
        cbor::cbor_bytes_serializer serializer(b);
        serializer.begin_document();
        serializer.begin_array(3); // omit size for indefinite length array
        serializer.string_value("Toronto");
        serializer.byte_string_value({'H','e','l','l','o'});
        serializer.bignum_value("-18446744073709551617");
        serializer.end_array();
        serializer.end_document();

        std::cout << "(1)\n";
        for (auto b : b)
        {
            std::cout << std::hex << (int)b;
        }
        std::cout << "\n\n";

        cbor::cbor_view bv = b; // a non-owning view of the CBOR bytes

        std::cout << "(2)\n";
        for (auto element : bv.array_range())
        {
            std::cout << element.as_string() << "\n";
        }
        std::cout << "\n";

        // Get element at position 1 using jsonpointer
        cbor::cbor_view element1 = jsonpointer::get(bv, "/1");
        std::cout << "(3) " << element1.as_string() << "\n\n";

        std::cout << "(4)\n";
        std::cout << pretty_print(bv) << "\n\n";

        json_serializing_options options;
        options.byte_string_format(byte_string_chars_format::base64)
               .bignum_format(bignum_chars_format::base64url);
        std::cout << "(5)\n";
        std::cout << pretty_print(bv, options) << "\n\n";

        json j = cbor::decode_cbor<json>(bv);
        j.push_back(bignum("18446744073709551616"));
        j.insert(j.array_range().begin(),10.5);
        std::cout << "(6)\n";
        std::cout << pretty_print(j) << "\n\n";

        std::vector<uint8_t> u;
        cbor::encode_cbor(j, u);
        std::cout << "(7)\n";
        std::cout << pretty_print(cbor::cbor_view(u)) << "\n\n";

    }
}

void readme_examples()
{
    std::cout << "\nReadme examples\n\n";

    readme::example1();

    std::cout << std::endl;
}

