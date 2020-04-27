// Copyright 2016 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h"
#endif
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor_reader.hpp>
#include <jsoncons/even_odd_visitor.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <limits>

using namespace jsoncons;

class my_json_visitor : public default_json_visitor
{
    bool visit_begin_object(semantic_tag, const ser_context&, std::error_code&) override
    {
        std::cout << "visit_begin_object\n"; 
        return true;
    }

    bool visit_end_object(const ser_context&, std::error_code&) override
    {
        std::cout << "visit_end_object\n"; 
        return true;
    }
    bool visit_begin_array(semantic_tag, const ser_context&, std::error_code&) override
    {
        std::cout << "visit_begin_array\n"; 
        return true;
    }

    bool visit_end_array(const ser_context&, std::error_code&) override
    {
        std::cout << "visit_end_array\n"; 
        return true;
    }

    bool visit_key(const string_view_type& s, const ser_context&, std::error_code&) override
    {
        std::cout << "visit_key " << s << "\n"; 
        return true;
    }
    bool visit_string(const string_view_type& s, semantic_tag, const ser_context&, std::error_code&) override
    {
        std::cout << "visit_string " << s << "\n"; 
        return true;
    }
    bool visit_int64(int64_t val, semantic_tag, const ser_context&, std::error_code&) override
    {
        std::cout << "visit_int64 " << val << "\n"; 
        return true;
    }
    bool visit_uint64(uint64_t val, semantic_tag, const ser_context&, std::error_code&) override
    {
        std::cout << "visit_uint64 " << val << "\n"; 
        return true;
    }
    bool visit_bool(bool val, semantic_tag, const ser_context&, std::error_code&) override
    {
        std::cout << "visit_bool " << val << "\n"; 
        return true;
    }

    bool visit_typed_array(const span<const uint16_t>& s, 
                                semantic_tag tag, 
                                const ser_context&, 
                                std::error_code&) override  
    {
        std::cout << "visit_typed_array uint16_t " << tag << "\n"; 
        for (auto val : s)
        {
            std::cout << val << "\n";
        }
        std::cout << "\n";
        return true;
    }

    bool visit_typed_array(half_arg_t, const span<const uint16_t>& s,
        semantic_tag tag,
        const ser_context&,
        std::error_code&) override
    {
        std::cout << "visit_typed_array half_arg_t uint16_t " << tag << "\n";
        for (auto val : s)
        {
            std::cout << val << "\n";
        }
        std::cout << "\n";
        return true;
    }
};

TEST_CASE("even_odd_visitor cbor 1")
{
    std::vector<uint8_t> input = {0xa2,
                                      0xa1, // object (1), key
                                          0x62,'o','c', // string, key
                                          0x81,0, // array(1), value
                                      0x61, 'a', // string(1), value
                                      0xa0, // object(0), key
                                      0 // value
    };

    json expected = json::parse(R"(
        {"{\"oc\":[0]}":"a","{}":0}
    )");

    SECTION("test 1")
    {
        json_decoder<json> destination;
        even_odd_to_json_visitor visitor{destination};

        cbor::basic_cbor_parser<bytes_source> parser{ bytes_source(input) };

        std::error_code ec;
        parser.parse(visitor, ec);
        CHECK(destination.get_result() == expected);
    }
}

TEST_CASE("even_odd_visitor cbor 2")
{
    std::vector<uint8_t> input = {0xa2,
                                      0xa2, // object (2), key
                                          0x62,'a','a', // string, key
                                          0x81,0, // array(1), value
                                          0x62,'b','b', // string, key
                                          0x81,0, // array(1), value
                                      0x61, 'a', // string(1), value
                                      0xa0, // object(0), key
                                      0 // value
    };

    json expected = json::parse(R"(
        {"{\"oc\":[0]}":"a","{}":0}
    )");

    SECTION("test 1")
    {
        my_json_visitor destination;
        even_odd_to_json_visitor visitor{destination};

        cbor::basic_cbor_parser<bytes_source> parser{ bytes_source(input) };

        std::error_code ec;
        parser.parse(visitor, ec);
        //std::cout << destination.get_result() << "\n";
        //CHECK(destination.get_result() == expected);
    }
}


