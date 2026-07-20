#include <jsoncons_ext/msgpack/msgpack.hpp>
#include <jsoncons/json.hpp>

#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <limits>

using namespace jsoncons;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, std::size_t size)
{
    try {
       json j2 = msgpack::decode_msgpack<json>(jsoncons::span<uint8_t>(data, size));
    }
    catch(const jsoncons::ser_error&) {}

    return 0;
}
