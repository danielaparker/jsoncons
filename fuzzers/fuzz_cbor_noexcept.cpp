#include <jsoncons_ext/cbor/cbor.hpp>
#include <jsoncons_ext/cbor/cbor_cursor.hpp>
#include <jsoncons_ext/cbor/decode_cbor.hpp>

#include <jsoncons/json.hpp>

#include <sstream>

using namespace jsoncons;
using namespace jsoncons::cbor;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, std::size_t size)
{
    std::string s(reinterpret_cast<const char*>(data), size);

    // try_decode_cbor via istringstream (error_code returned in result)
    {
        std::istringstream is(s);
        auto result = try_decode_cbor<json>(is);
        (void)result;
    }

    // cbor_stream_reader + default_json_visitor with error_code
    {
        std::istringstream is(s);
        default_json_visitor visitor;
        cbor_stream_reader reader(is, visitor);
        std::error_code ec;
        reader.read(ec);
    }

    // cbor_bytes_cursor with error_code constructor
    {
        jsoncons::span<const uint8_t> input(data, size);
        std::error_code ec;
        cbor_bytes_cursor cursor(input, ec);
        while (!cursor.done() && !ec)
        {
            cursor.next(ec);
        }
    }

    // read_to(visitor, ec), including failure paths on malformed input
    {
        jsoncons::span<const uint8_t> input(data, size);
        std::error_code ec;
        cbor_bytes_cursor cursor(input, ec);
        default_json_visitor visitor;
        while (!cursor.done() && !ec)
        {
            cursor.read_to(visitor, ec);
            if (!ec)
            {
                cursor.next(ec);
            }
        }
    }

    // cbor_stream_cursor with a tiny buffer size
    {
        std::istringstream is(s);
        std::error_code ec;
        cbor_stream_cursor cursor(jsoncons::binary_stream_source(is, 7), ec);
        while (!cursor.done() && !ec)
        {
            cursor.next(ec);
        }
    }

    return 0;
}
