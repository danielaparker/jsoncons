#include <jsoncons/json_parser.hpp>
#include <jsoncons/json.hpp>

using namespace jsoncons;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, std::size_t size) 
{
	std::string input(reinterpret_cast<const char*>(data), size);
	try{
		json val = json::parse(input);
	}
	catch(jsoncons::ser_error e) {}

	return 0;
}
