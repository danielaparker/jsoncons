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

#if 0
TEST_CASE("even_odd_visitor indefinite length map")
{
    std::vector<uint8_t> input = {0xbf,               // Start indefinite-length map
                                  0x63,               // First key, UTF-8 string length 3
                                      0x46,0x75,0x6e,     // "Fun"
                                  0xf5,                 // First value, true
                                  0x63,              // Second key, UTF-8 string length 3
                                      0x41,0x6d,0x74, // "Amt"
                                  0x21,                 // -2
                                      0xff};            // "break"

    SECTION("test 1")
    {
        my_json_visitor destination;
        cbor::even_odd_to_json_visitor visitor{destination};

        cbor::basic_cbor_parser<bytes_source> parser{ bytes_source(input) };

        std::error_code ec;
        parser.parse(visitor, ec);
        std::cout << "\n";
    }
}

TEST_CASE("even_odd_visitor definite length map")
{
    std::vector<uint8_t> input = {0xa2,               // Start definite-length map
                                  0x63,               // First key, UTF-8 string length 3
                                      0x46,0x75,0x6e,     // "Fun"
                                  0xf5,                 // First value, true
                                  0x63,              // Second key, UTF-8 string length 3
                                      0x41,0x6d,0x74, // "Amt"
                                  0x21};                 // -2

    SECTION("test 1")
    {
        my_json_visitor destination;
        cbor::even_odd_to_json_visitor visitor{destination};

        cbor::basic_cbor_parser<bytes_source> parser{ bytes_source(input) };

        std::error_code ec;
        parser.parse(visitor, ec);
        std::cout << "\n";
    }
}

TEST_CASE("even_odd_visitor definite length map with definite length map key")
{
    std::vector<uint8_t> input = {0xa2,               // Start definite-length map
                                  0xa1,               // First key, map
                                      0x62,'o','c','\0', // {\"oc\": 0}
                                  0xf5,                 // First value, true
                                  0x63,              // Second key, UTF-8 string length 3
                                      0x41,0x6d,0x74, // "Amt"
                                  0x21};                 // -2

    SECTION("test 1")
    {
        my_json_visitor destination;
        cbor::even_odd_to_json_visitor visitor{destination};

        cbor::basic_cbor_parser<bytes_source> parser{ bytes_source(input) };

        std::error_code ec;
        parser.parse(visitor, ec);
        std::cout << "\n";
    }
}

TEST_CASE("even_odd_visitor definite length map with definite length array key")
{
    std::vector<uint8_t> input = {0xa2,               // Start definite-length map
                                  0x82,               // First key, array length 2
                                      '\0','\0',      // [0,0]         
                                  0xf5,               // First value, true
                                  0x63,               // Second key, UTF-8 string length 3
                                      0x41,0x6d,0x74, // "Amt"
                                  0x21};              // -2

    SECTION("test 1")
    {
        my_json_visitor destination;
        cbor::even_odd_to_json_visitor visitor{destination};

        cbor::basic_cbor_parser<bytes_source> parser{ bytes_source(input) };

        std::error_code ec;
        parser.parse(visitor, ec);
        std::cout << "\n";
    }
}

#endif
#if 0

TEST_CASE("even_odd_visitor definite length map with definite length array key with object")
{
    std::vector<uint8_t> input = {0xa2,               // Start definite-length map
                                  0x82,               // First key, array length 2
                                      0xa1,               // First key, map
                                        0x62,'o','c','\0',
                                        '\0',      // [{\"oc\": 0},0]         
                                  0xf5,               // First value, true
                                  0x63,               // Second key, UTF-8 string length 3
                                      0x41,0x6d,0x74, // "Amt"
                                  0x21};              // -2

    SECTION("test 1")
    {
        my_json_visitor destination;
        cbor::even_odd_to_json_visitor visitor{destination};

        cbor::basic_cbor_parser<bytes_source> parser{ bytes_source(input) };

        std::error_code ec;
        parser.parse(visitor, ec);
        std::cout << "\n";
    }
}

TEST_CASE("even_odd_visitor definite length map with definite length array key with object")
{
    std::vector<uint8_t> input = {0x82,               // Start definite-length array
                                  0x81,               // Array length 2
                                      0x82,               // Array length 2
                                        0x62,'o','c','\0',
                                  0xf5};              // -2

    SECTION("test 1")
    {
        my_json_visitor destination;
        cbor::even_odd_to_json_visitor visitor{destination};

        cbor::basic_cbor_parser<bytes_source> parser{ bytes_source(input) };

        std::error_code ec;
        parser.parse(visitor, ec);
        std::cout << "\n";
    }
}
#endif
#if 0
TEST_CASE("even_odd_visitor definite length map with definite length array key with object")
{
    std::vector<uint8_t> input = {0xa1,               // Start definite-length map
                                  0x81,               // First key, array length 1
                                      0xa1,               // First key, map
                                        0x62,'o','c','\0',
                                  0xf5               // First value, true
                                  }; 

    SECTION("test 1")
    {
        my_json_visitor destination;
        cbor::even_odd_to_json_visitor visitor{destination};

        cbor::basic_cbor_parser<bytes_source> parser{ bytes_source(input) };

        std::error_code ec;
        parser.parse(visitor, ec);
        std::cout << "\n";
    }
}

TEST_CASE("even_odd_visitor definite length map with definite length array key with object")
{
    std::vector<uint8_t> input = {0xa1,               // Start definite-length map
                                  0xa1,               // First key, map length 1
                                      0xa1,               // First key, map
                                        0x62,'o','c','\0',
                                        '\0',
                                  0xf5               // First value, true
                                  }; 

    SECTION("test 1")
    {
        my_json_visitor destination;
        cbor::even_odd_to_json_visitor visitor{destination};

        cbor::basic_cbor_parser<bytes_source> parser{ bytes_source(input) };

        std::error_code ec;
        parser.parse(visitor, ec);
        std::cout << "\n";
    }
}

TEST_CASE("even_odd_visitor definite length map with definite length array key with array")
{
    std::vector<uint8_t> input = {0xa1,0x62,'o','c',0x81,'\0'}; 

    SECTION("test 1")
    {
        my_json_visitor destination;
        cbor::even_odd_to_json_visitor visitor{destination};

        cbor::basic_cbor_parser<bytes_source> parser{ bytes_source(input) };

        std::error_code ec;
        parser.parse(visitor, ec);
        std::cout << "\n";
    }
}

TEST_CASE("even_odd_visitor indefinite length map")
{
    std::vector<uint8_t> input = {0xbf,0x64,'N','a','m','e',0xbf,0xff,0xff}; 

    SECTION("test 1")
    {
        my_json_visitor destination;
        cbor::even_odd_to_json_visitor visitor{destination};

        cbor::basic_cbor_parser<bytes_source> parser{ bytes_source(input) };

        std::error_code ec;
        parser.parse(visitor, ec);
        std::cout << "\n";
    }
}
TEST_CASE("even_odd_visitor definite length map")
{
    std::vector<uint8_t> input = {0xa1,0x64,'N','a','m','e',0xa0}; 

    SECTION("test 1")
    {
        my_json_visitor destination;
        cbor::even_odd_to_json_visitor visitor{destination};

        cbor::basic_cbor_parser<bytes_source> parser{ bytes_source(input) };

        std::error_code ec;
        parser.parse(visitor, ec);
        std::cout << "\n";
    }
}
#endif
TEST_CASE("even_odd_visitor definite length map")
{
    const std::vector<uint8_t> input = {
        0xd8, // Tag
            0x50, // Tag 80, float16, big endian, Typed Array
        0x48, // Byte string value of length 8
            0x00,0x01,
            0x03,0xff,
            0x04,0x00,
            0x7b,0xff
    };

    SECTION("test 1")
    {
        my_json_visitor destination;
        cbor::even_odd_to_json_visitor visitor{destination};

        cbor::basic_cbor_parser<bytes_source> parser{ bytes_source(input) };

        std::error_code ec;
        parser.parse(visitor, ec);
        std::cout << "\n";
    }
}



