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

        std::cout << "(1)\n";
        for (auto x : b)
        {
            std::cout << std::hex << (int)x;
        }
        std::cout << "\n\n";

        cbor::cbor_view bv = b; // a non-owning view of the CBOR bytes

        std::cout << "(2)\n";
        for (cbor::cbor_view row : bv.array_range())
        {
            std::cout << row << "\n";
        }
        std::cout << "\n";

        // Get element at position /0/1 using jsonpointer
        cbor::cbor_view element1 = jsonpointer::get(bv, "/0/1");
        std::cout << "(3) " << element1.as<std::string>() << "\n\n";

        std::cout << "(4)\n";
        std::cout << pretty_print(bv) << "\n\n";

        json_serializing_options options;
        options.byte_string_format(byte_string_chars_format::base64)
               .bignum_format(bignum_chars_format::base64url);
        std::cout << "(5)\n";
        std::cout << pretty_print(bv, options) << "\n\n";

        json j = cbor::decode_cbor<json>(bv);
        j[0].push_back(bignum("18446744073709551616"));
        j[0].insert(j[0].array_range().begin(),10.5);
        std::cout << "(6)\n";
        std::cout << pretty_print(j) << "\n\n";

        std::vector<uint8_t> u;
        cbor::encode_cbor(j, u);
        std::cout << "(7)\n";
        std::cout << pretty_print(cbor::cbor_view(u)) << "\n\n";

        csv::csv_serializing_options csv_options;
        csv_options.column_names("A,B,C,D,E");
        std::string csv;
        csv::encode_csv(j, csv, csv_options);
        std::cout << "(8)\n";
        std::cout << csv << "\n\n";
    }
}

void readme_examples()
{
    std::cout << "\nReadme examples\n\n";

    readme::example1();

    std::cout << std::endl;
}

