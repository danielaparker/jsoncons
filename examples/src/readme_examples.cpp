// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <string>
#include <jsoncons/json.hpp>
#include <deque>
#include <map>
#include <unordered_map>
#include <jsoncons_ext/cbor/cbor_serializer.hpp>
#include <jsoncons_ext/cbor/cbor_view.hpp>

namespace readme
{
    void example1()
    {
        // Use streaming to construct some CBOR
        std::vector<uint8_t> b;
        jsoncons::cbor::cbor_bytes_serializer serializer(b);
        serializer.begin_document();
        serializer.begin_array(3);
        serializer.bool_value(true);
        serializer.bool_value(false);
        serializer.null_value();
        serializer.end_array();
        serializer.end_document();

        std::cout << "(1) ";
        for (auto b : b)
        {
            std::cout << std::hex << (int)b;
        }
        std::cout << "\n\n";

        // jsoncons::cbor::cbor_view returns a non-owning view of the CBOR bytes
        jsoncons::cbor::cbor_view bv = b;

        for (auto element : bv.array_range())
        {
            std::string s;
            //element.dump(s);
            //std::cout << s << std::endl;
        }
    }
}

void readme_examples()
{
    std::cout << "\nReadme examples\n\n";

    readme::example1();

    std::cout << std::endl;
}

