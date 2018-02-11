// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <jsoncons/json.hpp>
#include <jsoncons/detail/heap_only_string.hpp>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(heap_only_string_tests)

BOOST_AUTO_TEST_CASE(test_heap_only_string)
{
    std::string input = "Hello World";
    auto s = detail::heap_only_string_factory<char, std::allocator<char>>::create(input.data(), input.size());

    //std::cout << s->c_str() << std::endl;
    BOOST_CHECK(input == std::string(s->c_str()));

    detail::heap_only_string_factory<char,std::allocator<char>>::destroy(s);
}

BOOST_AUTO_TEST_CASE(test_heap_only_string_wchar_t)
{
    std::wstring input = L"Hello World";
    auto s = detail::heap_only_string_factory<wchar_t, std::allocator<wchar_t>>::create(input.data(), input.size());

    //std::wcout << s->c_str() << std::endl;

    BOOST_CHECK(input == std::wstring(s->c_str()));

    detail::heap_only_string_factory<wchar_t,std::allocator<wchar_t>>::destroy(s);
}

BOOST_AUTO_TEST_SUITE_END()


