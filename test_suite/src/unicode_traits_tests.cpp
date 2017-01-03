// Copyright 2016 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include <jsoncons/unicode_traits.hpp>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(json_text_traits_tests)

BOOST_AUTO_TEST_CASE(test_utf8_to_utf8)
{
    std::string source = "Hello world";
    const char* p = source.data();
    std::string target;
    unicode_traits<char>::append_to_string(p,source.data()+source.length(),
                                    target, &p,
                                    uni_conversion_flags::strict);
    BOOST_CHECK(source == target);
}

BOOST_AUTO_TEST_CASE(test_utf16_to_utf8)
{
    std::u16string source = u"Hello world";
    const char16_t* p = source.data();
    std::string target;
    unicode_traits<char16_t>::append_to_string(p,source.data()+source.length(),
                                        target, &p,
                                        uni_conversion_flags::strict);
    BOOST_CHECK("Hello world" == target);
}

BOOST_AUTO_TEST_CASE(test_utf8_to_utf162)
{
    std::string source = "Hello world";
    const char* p = source.data();
    std::u16string target;
    unicode_traits<char>::append_to_string(p,source.data()+source.length(),
                                       target, &p,
                                       uni_conversion_flags::strict);
    BOOST_CHECK(u"Hello world" == target);
}
BOOST_AUTO_TEST_CASE(test_utf8_to_utf32)
{
    std::string source = "Hello world";
    const char* p = source.data();
    std::u32string target;
    unicode_traits<char>::append_to_string(p,source.data()+source.length(),
                                   target, &p,
                                   uni_conversion_flags::strict);
    BOOST_CHECK(U"Hello world" == target);
}
BOOST_AUTO_TEST_CASE(test_utf16_to_utf32)
{
    std::u16string source = u"Hello world";
    const char16_t* p = source.data();
    std::u32string target;
    unicode_traits<char16_t>::append_to_string(p,source.data()+source.length(),
                                   target, &p,
                                   uni_conversion_flags::strict);
    BOOST_CHECK(U"Hello world" == target);
}
BOOST_AUTO_TEST_CASE(test_utf32_to_utf16)
{
    std::u32string source = U"Hello world";
    const char32_t* p = source.data();
    std::u16string target;
    unicode_traits<char32_t>::append_to_string(p,source.data()+source.length(),
                                   target, &p,
                                   uni_conversion_flags::strict);
    BOOST_CHECK(u"Hello world" == target);
}

BOOST_AUTO_TEST_CASE(test_utf16_to_utf16)
{
    std::u16string source = u"Hello world";
    const char16_t* p = source.data();
    std::u16string target;
    unicode_traits<char16_t>::append_to_string(p,source.data()+source.length(),
                                   target, &p,
                                   uni_conversion_flags::strict);
    BOOST_CHECK(u"Hello world" == target);
}
BOOST_AUTO_TEST_CASE(test_utf32_to_utf8)
{
    std::u32string source = U"Hello world";
    const char32_t* p = source.data();
    std::string target;
    unicode_traits<char32_t>::append_to_string(p,source.data()+source.length(),
                                        target, &p,
                                        uni_conversion_flags::strict);
    BOOST_CHECK("Hello world" == target);
}

BOOST_AUTO_TEST_CASE(test_utf8_next_codepoint)
{
    std::string source = "Hello world";
    const char* p = source.data();
    char32_t codepoint;

    auto result = unicode_traits<char>::next_codepoint(source.data(),source.data()+source.length(),
                                           &codepoint,&p,
                                           uni_conversion_flags::strict);
    BOOST_CHECK(result == uni_conversion_result::ok);
    BOOST_CHECK(codepoint == 'H');
    BOOST_CHECK(p == source.data()+1);
}

BOOST_AUTO_TEST_CASE(test_utf8_next_codepoint2)
{
    std::string source{"\u0040\u0040\u0000\u0011",4};
    const char* p = source.data();
    char32_t codepoint;

    auto result1 = unicode_traits<char>::next_codepoint(source.data(),source.data()+source.length(),
                                           &codepoint,&p,
                                           uni_conversion_flags::strict);
    BOOST_CHECK(result1 == uni_conversion_result::ok);
    BOOST_CHECK(codepoint == '@');
    BOOST_CHECK(p == source.data()+1);

    auto result2 = unicode_traits<char>::next_codepoint(p, source.data() + source.length(),
        &codepoint,&p,
        uni_conversion_flags::strict);
    BOOST_CHECK(result2 == uni_conversion_result::ok);
    BOOST_CHECK(codepoint == '@');
    BOOST_CHECK(p == source.data() + 2);

    auto result3 = unicode_traits<char>::next_codepoint(p, source.data() + source.length(),
        &codepoint,&p,
        uni_conversion_flags::strict);
    BOOST_CHECK(result3 == uni_conversion_result::ok);
    //BOOST_CHECK(codepoint == '@');
    BOOST_CHECK(p == source.data() + 3);

    auto result4 = unicode_traits<char>::next_codepoint(p, source.data() + source.length(),
        &codepoint,&p,
        uni_conversion_flags::strict);
    BOOST_CHECK(result4 == uni_conversion_result::ok);
    //BOOST_CHECK(codepoint == '@');
    BOOST_CHECK(p == source.data() + 4);
}

BOOST_AUTO_TEST_CASE(test_utf8_next_codepoint3)
{
    std::string source{"\x7f\xdf\xbf\xe0\xa0\x80",6};
    const char* p = source.data();
    char32_t codepoint;

    auto result1 = unicode_traits<char>::next_codepoint(source.data(),source.data()+source.length(),
                                           &codepoint,&p,
                                           uni_conversion_flags::strict);
    BOOST_REQUIRE(result1 == uni_conversion_result::ok);
    //BOOST_CHECK(codepoint == '@');
    BOOST_CHECK(p == source.data()+1);

    auto result2 = unicode_traits<char>::next_codepoint(p,source.data()+source.length(),
                                           &codepoint,&p,
                                           uni_conversion_flags::strict);
    BOOST_REQUIRE(result2 == uni_conversion_result::ok);

    //BOOST_CHECK(codepoint == '@');
    BOOST_CHECK(p == source.data()+3);

    auto result3 = unicode_traits<char>::next_codepoint(p, source.data() + source.length(),
        &codepoint,&p,
        uni_conversion_flags::strict);
    BOOST_REQUIRE(result3 == uni_conversion_result::ok);

    //BOOST_CHECK(codepoint == '@');
    BOOST_CHECK(p == source.data() + 6);

}

BOOST_AUTO_TEST_CASE(test_utf16_next_codepoint)
{
    std::u16string source = u"Hello world";
    const char16_t* p = source.data();
    char32_t codepoint;

    auto result = unicode_traits<char16_t>::next_codepoint(source.data(),source.data()+source.length(),
                                           &codepoint,&p,
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

    auto result = unicode_traits<char32_t>::next_codepoint(source.data(),source.data()+source.length(),
                                           &codepoint,&p,
                                           uni_conversion_flags::strict);
    BOOST_CHECK(result == uni_conversion_result::ok);
    BOOST_CHECK(codepoint == 'H');
    BOOST_CHECK(p == source.data()+1);
}

BOOST_AUTO_TEST_SUITE_END()


