#include <jsoncons_ext/ubjson/ubjson.hpp>
#include <jsoncons_ext/ubjson/ubjson_reader.hpp>
#include <jsoncons/json.hpp>
#include <sstream>
#include <cassert>

using namespace jsoncons;
using namespace ubjson;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, std::size_t size)
{
    std::string s(reinterpret_cast<const char*>(data), size);
    std::istringstream is(s);

    default_json_visitor visitor;

    ubjson_stream_reader reader(is, visitor);
    std::error_code ec;
    reader.read(ec);

    return 0;
}
