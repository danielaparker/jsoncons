#include <jsoncons/json_parser.hpp>
#include <jsoncons/json_cursor.hpp>
#include <jsoncons/json.hpp>

using namespace jsoncons;
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
        std::string s(reinterpret_cast<const char*>(data), size);
        std::istringstream is(s);

        json_cursor reader(is);
        while (reader.done() == 0)
                reader.next();

        return 0;
}
