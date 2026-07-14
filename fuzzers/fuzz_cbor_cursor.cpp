#include <jsoncons_ext/cbor/cbor.hpp>
#include <jsoncons_ext/cbor/cbor_cursor.hpp>

#include <jsoncons/json.hpp>

#include <sstream>

using namespace jsoncons;
using namespace jsoncons::cbor;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, std::size_t size)
{
    std::string s(reinterpret_cast<const char*>(data), size);
    std::istringstream is(s);

    std::error_code ec;
    cbor_stream_cursor cursor(is, ec);
    while (!cursor.done() && !ec)
    {
        const auto& event = cursor.current();
        std::string s2 = event.get<std::string>(ec);
        if (!ec)
        {
            cursor.next(ec);
        }
    }

    return 0;
}
