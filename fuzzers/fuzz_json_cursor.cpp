#include <jsoncons/json_parser.hpp>
#include <jsoncons/json_cursor.hpp>
#include <jsoncons/json.hpp>

using namespace jsoncons;
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
        std::string s(reinterpret_cast<const char*>(data), size);
        std::istringstream is(s);

        std::error_code ec;
        json_cursor reader(is, ec);
        while (reader.done() == 0 && ec == 0)
        {
                const auto& event = cursor.current();
                std::string s2 = event.as<std::string>();
                reader.next(ec);
        }

        return 0;
}
