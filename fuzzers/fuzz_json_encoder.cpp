#include <jsoncons/json_parser.hpp>
#include <jsoncons/json.hpp>

using namespace jsoncons;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, std::size_t size)
{
    std::string s(reinterpret_cast<const char*>(data), size);
    std::istringstream is(s);

    try
    {
        std::string s2;
        json_string_encoder visitor(s2);
        json_reader reader(is, visitor);
        std::error_code ec;
        reader.read(ec);
        std::cout << ec.message() << "\n";
    }
    catch (const std::excption& e)
    {
        std::cout << e.what() << "\n";
    }

    return 0;
}
