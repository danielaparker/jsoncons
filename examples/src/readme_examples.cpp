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
        cbor::cbor_bytes_serializer writer(b);
        writer.begin_array(); // indefinite length array containing rows
        writer.begin_array(3); // a row, fixed length array
        writer.string_value("foo");
        writer.byte_string_value({'b','a','r'});
        writer.bignum_value("-18446744073709551617");
        writer.end_array();
        writer.end_array();
        writer.flush();

        // Print bytes
        std::cout << "(1)\n";
        for (auto c : b)
        {
            std::cout << std::hex << std::setprecision(2) << std::setw(2)
                      << std::setfill('0') << static_cast<int>(c);
        }
        std::cout << "\n\n";
/*
        9f -- Start indefinte length array
          83 -- Array of length 3
            63 -- String value of length 3
              666f6f -- "foo" 
            43 -- Byte string value of length 3
              626172 -- 'b''a''r'
            c3 -- Bignum
              49 -- Byte string value of length 9
              010000000000000000 -- Bytes content
          ff -- "break" 
*/
        cbor::cbor_view bv = b; // a non-owning view of the CBOR bytes

        // Loop over the rows
        std::cout << "(2)\n";
        for (cbor::cbor_view row : bv.array_range())
        {
            std::cout << row << "\n";
        }
        std::cout << "\n";

        // Get element at position 0/2 using jsonpointer (must be by value)
        cbor::cbor_view v = jsonpointer::get(bv, "/0/2");
        std::cout << "(3) " << v.as<std::string>() << "\n\n";

        // Print JSON representation with default options
        std::cout << "(4)\n";
        std::cout << pretty_print(bv) << "\n\n";

        // Print JSON representation with different options
        json_serializing_options options;
        options.byte_string_format(byte_string_chars_format::base64)
               .bignum_format(bignum_chars_format::base64url);
        std::cout << "(5)\n";
        std::cout << pretty_print(bv, options) << "\n\n";

        // Unpack bytes into a json variant value, and add some more elements
        json j = cbor::decode_cbor<json>(bv);

        json another_row = json::array(); 
        another_row.emplace_back(byte_string({'q','u','x'}));
        another_row.emplace_back("273.15", semantic_tag_type::decimal);
        another_row.emplace(another_row.array_range().begin(),"baz");

        j.push_back(std::move(another_row));
        std::cout << "(6)\n";
        std::cout << pretty_print(j) << "\n\n";

        // Get element at position /1/2 using jsonpointer (can be by reference)
        json& ref = jsonpointer::get(j, "/1/2");
        std::cout << "(7) " << ref.as<std::string>() << "\n\n";

#if (defined(__GNUC__) || defined(__clang__)) && (!defined(__STRICT_ANSI__) && defined(_GLIBCXX_USE_INT128))
        // If code compiled with GCC and std=gnu++11 (rather than std=c++11)
        __int128 i = j[1][2].as<__int128>();
#endif

        // Repack bytes
        std::vector<uint8_t> b2;
        cbor::encode_cbor(j, b2);

        // Print the repacked bytes
        std::cout << "(8)\n";
        for (auto c : b2)
        {
            std::cout << std::hex << std::setprecision(2) << std::setw(2)
                      << std::setfill('0') << static_cast<int>(c);
        }
        std::cout << "\n\n";
/*
        82 -- Array of length 2
          83 -- Array of length 3
            63 -- String value of length 3
              666f6f -- "foo" 
            43 -- Byte string value of length 3
              626172 -- 'b''a''r'
            c3 -- Bignum
            49 -- Byte string value of length 9
              010000000000000000 -- Bytes content
          83 -- Another array of length 3
          63 -- String value of length 3
            62617a -- "baz" 
          43 -- Byte string value of length 3
            717578 -- 'q''u''x'
          c4 - Tag 4 (decimal fraction)
            82 - Array of length 2
              21 -- -2
              19 6ab3 -- 27315
*/
        std::cout << "(9)\n";
        cbor::cbor_view bv2 = b2;
        std::cout << pretty_print(bv2) << "\n\n";

        // Serialize to CSV
        csv::csv_serializing_options csv_options;
        csv_options.column_names("Column 1,Column 2,Column 3");

        std::string csv_j;
        csv::encode_csv(j, csv_j, csv_options);
        std::cout << "(10)\n";
        std::cout << csv_j << "\n\n";

        std::string csv_bv2;
        csv::encode_csv(bv2, csv_bv2, csv_options);
        std::cout << "(11)\n";
        std::cout << csv_bv2 << "\n\n";
    }
}

void readme_examples()
{
    std::cout << "\nReadme examples\n\n";

    readme::example1();

    std::cout << std::endl;
}

