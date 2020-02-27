#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <jsoncons/config/jsoncons_config.hpp>


TEST_CASE("This here test")
{
#if defined(JSONCONS_HAS_STD_OPTIONAL)
std::cout << "Has std::optional" << "\n";
std::optional<int64_t> end{};
std::optional<int64_t> end2{};

end = std::optional<int64_t>();

#else
std::cout << "Hasn't std::optional" << "\n";
#endif
}
