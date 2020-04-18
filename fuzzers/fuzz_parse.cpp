#include <jsoncons/json_parser.hpp>
#include <jsoncons/json.hpp>

using namespace jsoncons;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	std::string s(reinterpret_cast<const char*>(data), size);
	std::istringstream is(s);
	json_string_encoder encoder(s);
	json_reader reader(is, encoder);

	std::error_code ec;
	reader.read(ec);

	return 0;
}
