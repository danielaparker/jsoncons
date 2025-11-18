#include <jsoncons/utility/bigint.hpp>
#include <string>

using namespace jsoncons;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, std::size_t size) 
{
	std::string input(reinterpret_cast<const char*>(data), size);
	try {
		bigint a("56654250564056135415631554531554513813");
		bigint b(input);
		bigint c = a % b;
		bigint d = b % a;
	}
	catch (const std::runtime_error&) {}

	return 0;
}
