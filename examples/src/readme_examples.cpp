// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <string>
#include <vector>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor_serializer.hpp>
#include <jsoncons_ext/cbor/cbor_view.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/csv/csv_serializer.hpp>

namespace readme
{
    using namespace jsoncons;    

    void example1()
    {
        // Construct some CBOR using the streaming API
        std::vector<uint8_t> b;
        cbor::cbor_bytes_serializer bserializer(b);
        bserializer.begin_document();
        bserializer.begin_array(); // indefinite length array
        bserializer.begin_array(3); // fixed length array
        bserializer.string_value("Toronto");
        bserializer.byte_string_value({'H','e','l','l','o'});
        bserializer.bignum_value("-18446744073709551617");
        bserializer.end_array();
        bserializer.end_array();
        bserializer.end_document();

        // Print bytes
        std::cout << "(1)\n";
        for (auto x : b)
        {
            std::cout << std::hex << (int)x;
        }
        std::cout << "\n\n";

        cbor::cbor_view bv = b; // a non-owning view of the CBOR bytes

        // Loop over the outer array elements
        std::cout << "(2)\n";
        for (cbor::cbor_view row : bv.array_range())
        {
            std::cout << row << "\n";
        }
        std::cout << "\n";

        // Get element at position /0/1 using jsonpointer (must be by value)
        cbor::cbor_view val = jsonpointer::get(bv, "/0/1");
        std::cout << "(3) " << val.as<std::string>() << "\n\n";

        // Print JSON representation with default options
        std::cout << "(4)\n";
        std::cout << pretty_print(bv) << "\n\n";

        // Print JSON representation with different options
        json_serializing_options options;
        options.byte_string_format(byte_string_chars_format::base64)
               .bignum_format(bignum_chars_format::base64url);
        std::cout << "(5)\n";
        std::cout << pretty_print(bv, options) << "\n\n";

        // Unpack bytes into a json variant like structure, and add some more elements
        json j = cbor::decode_cbor<json>(bv);
        j[0].push_back(bignum("18446744073709551616"));
        j[0].insert(j[0].array_range().begin(),10.5);
        std::cout << "(6)\n";
        std::cout << pretty_print(j) << "\n\n";

        // Get element at position /0/0 using jsonpointer (by reference)
        json& ref = jsonpointer::get(j, "/0/0");
        std::cout << "(7) " << ref.as<double>() << "\n\n";

        // Repack bytes
        std::vector<uint8_t> b2;
        cbor::encode_cbor(j, b2);
        std::cout << "(8)\n";
        cbor::cbor_view b2v = b2;
        std::cout << pretty_print(b2v) << "\n\n";

        // Serialize to CSV
        csv::csv_serializing_options csv_options;
        csv_options.column_names("A,B,C,D,E");

        std::string from_unpacked;
        csv::encode_csv(j, from_unpacked, csv_options);
        std::cout << "(9)\n";
        std::cout << from_unpacked << "\n\n";

        std::string from_packed;
        csv::encode_csv(b2v, from_packed, csv_options);
        std::cout << "(10)\n";
        std::cout << from_packed << "\n\n";
    }
}

void readme_examples()
{
    std::cout << "\nReadme examples\n\n";

    readme::example1();

    std::cout << std::endl;
}

