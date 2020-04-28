// Copyright 2016 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h"
#endif
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor_reader.hpp>
#include <jsoncons/json_visitor2.hpp>
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

TEST_CASE("json_visitor2 cbor 1")
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
        json_visitor2_to_json_visitor visitor{destination};

        cbor::basic_cbor_parser<bytes_source> parser{ bytes_source(input) };

        std::error_code ec;
        parser.parse(visitor, ec);
        CHECK(destination.get_result() == expected);
    }
}

TEST_CASE("json_visitor2 cbor 2")
{
    std::vector<uint8_t> input = {0xa2,
                                      0xa2, // object (2), key
                                          0x62,'a','a', // string, key
                                          0x81,0, // array(1), value
                                          0x62,'b','b', // string, key
                                          0x81,1, // array(1), value
                                      0x61, 'a', // string(1), value
                                      0xa0, // object(0), key
                                      0 // value
    };

    json expected = json::parse(R"(
        {"{\"aa\":[0],\"bb\":[1]}":"a","{}":0}
    )");

    SECTION("test 1")
    {
        json_decoder<json> destination;
        json_visitor2_to_json_visitor visitor{destination};

        cbor::basic_cbor_parser<bytes_source> parser{ bytes_source(input) };

        std::error_code ec;
        parser.parse(visitor, ec);
        //std::cout << destination.get_result() << "\n";
        CHECK(destination.get_result() == expected);
    }
}

TEST_CASE("json_visitor2 cbor 3")
{
    std::vector<uint8_t> input = {0xa2,
                                      0xa2, // object (2), key
                                          0x62,'a','a', // string, key
                                          0x81,0, // array(1), value
                                          0xa0, // string, key
                                          0x81,1, // array(1), value
                                      0x61, 'a', // string(1), value
                                      0xa0, // object(0), key
                                      0 // value
    };

    json expected = json::parse(R"(
        {"{\"aa\":[0],{}:[1]}":"a","{}":0}
    )");

    SECTION("test 1")
    {
        json_decoder<json> destination;
        json_visitor2_to_json_visitor visitor{destination};

        cbor::basic_cbor_parser<bytes_source> parser{ bytes_source(input) };

        std::error_code ec;
        parser.parse(visitor, ec);
        //std::cout << destination.get_result() << "\n";
        CHECK(destination.get_result() == expected);
    }
}

TEST_CASE("json_visitor2 cbor 4")
{
    std::vector<uint8_t> input = {0xa2,
                                      0xa2, // object (2), key
                                          0x62,'a','a', // string, key
                                          0x81,0, // array(1), value
                                          0x80, // array, key
                                          0x81,1, // array(1), value
                                      0x61, 'a', // string(1), value
                                      0xa0, // object(0), key
                                      0 // value
    };

    json expected = json::parse(R"(
        {"{\"aa\":[0],[]:[1]}":"a","{}":0}
    )");

    SECTION("test 1")
    {
        json_decoder<json> destination;
        json_visitor2_to_json_visitor visitor{destination};

        cbor::basic_cbor_parser<bytes_source> parser{ bytes_source(input) };

        std::error_code ec;
        parser.parse(visitor, ec);
        //std::cout << destination.get_result() << "\n";
        CHECK(destination.get_result() == expected);
    }
}

TEST_CASE("json_visitor2 cbor 5")
{
    std::vector<uint8_t> input = {0xa2,
                                      0x84, // array(4), key
                                         0,
                                         1,
                                         2,
                                         3,
                                      0x61, 'a', // string(1), value
                                      0x80, // array(0), key
                                      0 // value
    };

    json expected = json::parse(R"(
        {"[0,1,2,3]":"a","[]":0}
    )");

    SECTION("test 1")
    {
        json_decoder<json> destination;
        json_visitor2_to_json_visitor visitor{destination};

        cbor::basic_cbor_parser<bytes_source> parser{ bytes_source(input) };

        std::error_code ec;
        parser.parse(visitor, ec);
        //std::cout << destination.get_result() << "\n";
        CHECK(destination.get_result() == expected);
    }
}


