#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <jsoncons_ext/cbor/cbor_reader.hpp>
#include <catch/catch.hpp>
#include <sstream>

using namespace jsoncons;
using namespace jsoncons::cbor;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
        std::string input(reinterpret_cast<const char*>(data), size);
        std::istringstream is(input);
        try {
                json j2 = decode_cbor<json>(is);
        }
        catch(jsoncons::ser_error e) {}

        return 0;
}
