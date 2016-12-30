// Copyright 2016 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include <jsoncons/json_text_traits.hpp>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(json_text_traits_tests)

BOOST_AUTO_TEST_CASE(test_utf8_to_utf8)
{
    std::string source = "Hello world";
    const char* p = source.data();
    std::string target;
    json_text_traits<char>::to_utf8(&p,source.data()+source.length(),
                                    target,
                                    uni_conversion_flags::strict);
    BOOST_CHECK(source == target);
}

BOOST_AUTO_TEST_CASE(test_utf8_from_utf8)
{
    std::string source = "Hello world";
    const char* p = source.data();
    std::string target;
    json_text_traits<char>::from_utf8(&p,source.data()+source.length(),
                                      target,
                                      uni_conversion_flags::strict);
    BOOST_CHECK(source == target);
}

BOOST_AUTO_TEST_CASE(test_utf16_to_utf8)
{
    std::u16string source = u"Hello world";
    const char16_t* p = source.data();
    std::string target;
    json_text_traits<char16_t>::to_utf8(&p,source.data()+source.length(),
                                        target,
                                        uni_conversion_flags::strict);
    BOOST_CHECK("Hello world" == target);
}

BOOST_AUTO_TEST_CASE(test_utf8_to_utf16)
{
    std::string source = "Hello world";
    const char* p = source.data();
    std::u16string target;
    json_text_traits<char16_t>::from_utf8(&p,source.data()+source.length(),
                                        target,
                                        uni_conversion_flags::strict);
    BOOST_CHECK(u"Hello world" == target);
}

BOOST_AUTO_TEST_CASE(test_utf32_to_utf8)
{
    std::u32string source = U"Hello world";
    const char32_t* p = source.data();
    std::string target;
    json_text_traits<char32_t>::to_utf8(&p,source.data()+source.length(),
                                        target,
                                        uni_conversion_flags::strict);
    BOOST_CHECK("Hello world" == target);
}

BOOST_AUTO_TEST_CASE(test_utf8_to_utf32)
{
    std::string source = "Hello world";
    const char* p = source.data();
    std::u32string target;
    json_text_traits<char32_t>::from_utf8(&p,source.data()+source.length(),
                                        target,
                                        uni_conversion_flags::strict);
    BOOST_CHECK(U"Hello world" == target);
}

BOOST_AUTO_TEST_CASE(test_utf8_next_codepoint)
{
    std::string source = "Hello world";
    const char* p = source.data();
    char32_t codepoint;

    auto result = json_text_traits<char>::next_codepoint(&p,source.data()+source.length(),
                                           &codepoint,
                                           uni_conversion_flags::strict);
    BOOST_CHECK(result == uni_conversion_result::ok);
    BOOST_CHECK(codepoint == 'H');
    BOOST_CHECK(p == source.data()+1);
}

BOOST_AUTO_TEST_CASE(test_utf16_next_codepoint)
{
    std::u16string source = u"Hello world";
    const char16_t* p = source.data();
    char32_t codepoint;

    auto result = json_text_traits<char16_t>::next_codepoint(&p,source.data()+source.length(),
                                           &codepoint,
                                           uni_conversion_flags::strict);
    BOOST_CHECK(result == uni_conversion_result::ok);
    BOOST_CHECK(codepoint == 'H');
    BOOST_CHECK(p == source.data()+1);
}

BOOST_AUTO_TEST_CASE(test_utf32_next_codepoint)
{
    std::u32string source = U"Hello world";
    const char32_t* p = source.data();
    char32_t codepoint;

    auto result = json_text_traits<char32_t>::next_codepoint(&p,source.data()+source.length(),
                                           &codepoint,
                                           uni_conversion_flags::strict);
    BOOST_CHECK(result == uni_conversion_result::ok);
    BOOST_CHECK(codepoint == 'H');
    BOOST_CHECK(p == source.data()+1);
}

BOOST_AUTO_TEST_SUITE_END()


