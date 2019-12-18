// Copyright 2017 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonpath/json_query.hpp>
#include "sample_types.hpp"
#include <string>
#include <iomanip>
#include <cassert>

using namespace jsoncons;

namespace cbor_typed_array_examples {

void decode_float64_big_endian_array()
{
    const std::vector<uint8_t> input = {
      0xd8,0x52, // Tag 82 (float64 big endian Typed Array)
        0x50,    // Byte string value of length 16
            0xff, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0x7f, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };

    auto j = cbor::decode_cbor<json>(input);
    std::cout << "(1)\n" << pretty_print(j) << "\n\n";

    auto v = cbor::decode_cbor<std::vector<double>>(input);
    std::cout << "(2)\n";
    for (auto item : v)
    {
        std::cout << std::defaultfloat << item << "\n";
    }
    std::cout << "\n";

    std::vector<uint8_t> output1;
    cbor::encode_cbor(v, output1);

    // output1 contains a classical CBOR array
    std::cout << "(3)\n" << byte_string_view(output1.data(), output1.size()) << "\n\n";

    std::vector<uint8_t> output2;
    cbor::cbor_options options;
    options.enable_typed_arrays(true);
    cbor::encode_cbor(v, output2, options);

    // output2 contains a float64, native endian, Typed Array 
    std::cout << "(4)\n" << byte_string_view(output2.data(), output2.size()) << "\n\n";
}

void decode_mult_dim_row_major()
{
    const std::vector<uint8_t> input = {
      0xd8,0x28,     // Tag 40 (multi-dimensional row major array)
        0x82,        // array(2)
          0x82,      // array(2)
            0x02,    // unsigned(2) 1st Dimension
            0x03,    // unsigned(3) 2nd Dimension
          0xd8,0x41,     // Tag 65 (uint16 big endian Typed Array)
            0x4c,        // byte string(12)
              0x00,0x02, // unsigned(2)
              0x00,0x04, // unsigned(4)
              0x00,0x08, // unsigned(8)
              0x00,0x04, // unsigned(4)
              0x00,0x10, // unsigned(16)
              0x01,0x00  // unsigned(256)
    };

    json j = cbor::decode_cbor<json>(input);

    std::cout << j.tag() << "\n";
    std::cout << pretty_print(j) << "\n";
}

void encode_mult_dim_array()
{
    std::vector<uint8_t> v;

    cbor::cbor_bytes_encoder encoder(v);
    std::vector<std::size_t> shape = { 2,3 };
    encoder.begin_multi_dim(shape, semantic_tag::multi_dim_column_major);
    encoder.begin_array(6);
    encoder.uint64_value(2);
    encoder.uint64_value(4);
    encoder.uint64_value(8);
    encoder.uint64_value(4);
    encoder.uint64_value(16);
    encoder.uint64_value(256);
    encoder.end_array();
    encoder.end_multi_dim();

    std::cout << "(1)\n" << byte_string_view(v.data(), v.size()) << "\n\n";

    auto j = cbor::decode_cbor<json>(v);
    std::cout << "(2) " << j.tag() << "\n";
    std::cout << pretty_print(j) << "\n\n";
}

void encode_half_array()
{
    std::vector<uint8_t> buffer;

    cbor::cbor_options options;
    options.enable_typed_arrays(true);
    cbor::cbor_bytes_encoder encoder(buffer, options);

    std::vector<uint16_t> values = {0x3bff,0x3c00,0x3c01,0x3555};
    encoder.typed_array(half_arg, values);

    // buffer contains a half precision floating-point, native endian, Typed Array 
    std::cout << "(1)\n" << byte_string_view(buffer.data(), buffer.size()) << "\n\n";

    auto j = cbor::decode_cbor<json>(buffer);

    std::cout << "(2)\n";
    for (auto item : j.array_range())
    {
        std::cout << std::boolalpha << item.is_half() 
                  << " " << std::hex << (int)item.as<uint16_t>() 
                  << " " << std::defaultfloat << item.as<double>() << "\n";
    }
    std::cout << "\n";

    std::cout << "(3)\n" << pretty_print(j) << "\n\n";
}

void cursor_example_multi_dim_row_major_typed_array()
{
    const std::vector<uint8_t> input = {
      0xd8,0x28,  // Tag 40 (multi-dimensional row major array)
        0x82,     // array(2)
          0x82,   // array(2)
            0x02,    // unsigned(2) 1st Dimension
            0x03,    // unsigned(3) 2nd Dimension
        0xd8,0x41,     // Tag 65 (uint16 big endian Typed Array)
          0x4c,        // byte string(12)
            0x00,0x02, // unsigned(2)
            0x00,0x04, // unsigned(4)
            0x00,0x08, // unsigned(8)
            0x00,0x04, // unsigned(4)
            0x00,0x10, // unsigned(16)
            0x01,0x00  // unsigned(256)
    };

    cbor::cbor_bytes_cursor cursor(input);
    for (; !cursor.done(); cursor.next())
    {
        const auto& event = cursor.current();
        switch (event.event_type())
        {
            case staj_event_type::begin_array:
                std::cout << event.event_type() 
                          << " " << "(" << event.tag() << ")\n";
                break;
            case staj_event_type::end_array:
                std::cout << event.event_type() 
                          << " " << "(" << event.tag() << ")\n";
                break;
            case staj_event_type::uint64_value:
                std::cout << event.event_type() 
                          << ": " << event.get<uint64_t>() << " " << "(" << event.tag() << ")\n";
                break;
            default:
                std::cout << "Unhandled event type " << event.event_type() 
                          << " " << "(" << event.tag() << ")\n";
                break;
        }
    }
}

void cursor_example_multi_dim_column_major_classical_cbor_array()
{
    const std::vector<uint8_t> input = {
      0xd9,0x04,0x10,  // Tag 1040 (multi-dimensional column major array)
        0x82,     // array(2)
          0x82,   // array(2)
            0x02,    // unsigned(2) 1st Dimension
            0x03,    // unsigned(3) 2nd Dimension
          0x86,   // array(6)
            0x02,           // unsigned(2)   
            0x04,           // unsigned(4)   
            0x08,           // unsigned(8)   
            0x04,           // unsigned(4)   
            0x10,           // unsigned(16)  
            0x19,0x01,0x00  // unsigned(256) 
    };

    cbor::cbor_bytes_cursor cursor(input);
    for (; !cursor.done(); cursor.next())
    {
        const auto& event = cursor.current();
        switch (event.event_type())
        {
            case staj_event_type::begin_array:
                std::cout << event.event_type() 
                          << " " << "(" << event.tag() << ")\n";
                break;
            case staj_event_type::end_array:
                std::cout << event.event_type() 
                          << " " << "(" << event.tag() << ")\n";
                break;
            case staj_event_type::uint64_value:
                std::cout << event.event_type() 
                          << ": " << event.get<uint64_t>() << " " << "(" << event.tag() << ")\n";
                break;
            default:
                std::cout << "Unhandled event type " << event.event_type() 
                          << " " << "(" << event.tag() << ")\n";
                break;
        }
    }
}

struct my_cbor_content_handler : public cbor::default_cbor_content_handler
{
    std::vector<double> v;
private:
    bool do_typed_array(const span<const double>& data,  
                        semantic_tag,
                        const ser_context&,
                        std::error_code&) override
    {
        v = std::vector<double>(data.begin(),data.end());
        return false;
    }
};

void read_to_cbor_content_handler()
{
    std::vector<double> v{10.0,20.0,30.0,40.0};

    std::vector<uint8_t> buffer;
    cbor::cbor_options options;
    options.enable_typed_arrays(true);
    cbor::encode_cbor(v, buffer, options);

    std::cout << "(1)\n";
    std::cout << byte_string_view(buffer.data(),buffer.size()) << "\n\n";
/*
    0xd8, // Tag
        0x56, // Tag 86, float64, little endian, Typed Array
    0x58,0x20, // Byte string value of length 32 
        0x00,0x00,0x00,0x00,0x00,0x00,0x24,0x40,
        0x00,0x00,0x00,0x00,0x00,0x00,0x34,0x40, 
        0x00,0x00,0x00,0x00,0x00,0x00,0x3e,0x40, 
        0x00,0x00,0x00,0x00,0x00,0x00,0x44,0x40
*/

    cbor::cbor_bytes_cursor cursor(buffer);
    assert(cursor.current().event_type() == staj_event_type::begin_array);
    assert(cursor.is_typed_array());

    my_cbor_content_handler handler;
    cursor.read(handler);
    std::cout << "(2)\n";
    for (auto item : handler.v)
    {
        std::cout << item << "\n";
    }
    std::cout << "\n";
}

} // cbor_typed_array_examples

void run_cbor_typed_array_examples()
{
    std::cout << "\ncbor typed array examples\n\n";
    cbor_typed_array_examples::decode_float64_big_endian_array();
    cbor_typed_array_examples::decode_mult_dim_row_major();
    cbor_typed_array_examples::encode_mult_dim_array();
    cbor_typed_array_examples::encode_half_array();
    cbor_typed_array_examples::cursor_example_multi_dim_row_major_typed_array();
    cbor_typed_array_examples::cursor_example_multi_dim_column_major_classical_cbor_array();
    cbor_typed_array_examples::read_to_cbor_content_handler();

    std::cout << "\n\n";
}

