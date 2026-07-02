// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <string>
#include <catch/catch.hpp>

using namespace jsoncons;

TEST_CASE("unicode_traits tests")
{
    SECTION("test_surrogate_pair")
    {
        std::string input = "[\"\\u8A73\\u7D30\\u95B2\\u89A7\\uD800\\uDC01\\u4E00\"]";
        json value = json::parse(input);
        auto options = json_options{}
            .escape_all_non_ascii(true);
        std::string output;
        value.dump(output,options);

        CHECK(input == output);
    }
    SECTION("test_wide_surrogate_pair")
    {
        std::wstring input = L"[\"\\u8A73\\u7D30\\u95B2\\u89A7\\uD800\\uDC01\\u4E00\"]";
        wjson value = wjson::parse(input);
        auto options = wjson_options{}
            .escape_all_non_ascii(true);
        std::wstring output;
        value.dump(output,options);

        CHECK(input == output);
    }
    SECTION("with double")
    {
        std::istringstream is("{\"unicode_string_1\":\"\\uD800\\uDC00\"}");

        json root = json::parse(is);
        CHECK(root.is_object());
        CHECK(root.is_object());

        root["double_1"] = 10.0;

        json double_1 = root["double_1"];

        CHECK(10.0 == Approx(double_1.as<double>()).epsilon(0.000001));

        CHECK(10.0 == Approx(double_1.as<double>()).epsilon(0.000001));
    }
}

TEST_CASE("unicode_traits utf8 tests")
{
    SECTION("Valid continuation bytes")
    {
        std::vector<uint8_t> seq1 = {0xC2, 0x80};
        auto result1 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::success == result1.ec);

        std::vector<uint8_t> seq2 = {0xC2, 0xBF};
        auto result2 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::success == result2.ec);

        std::vector<uint8_t> seq3 = {0xDF, 0xBF};
        auto result3 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::success == result3.ec);

        std::vector<uint8_t> seq4 = {0xE0, 0xA0, 0x80};
        auto result4 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::success == result4.ec);

        std::vector<uint8_t> seq5 = {0xEF, 0xBF, 0xBF};
        auto result5 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::success == result5.ec);

        std::vector<uint8_t> seq6 = {0xF0, 0x90, 0x80, 0x80};
        auto result6 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::success == result6.ec);

        std::vector<uint8_t> seq7 = {0xF4, 0x8F, 0xBF, 0xBF};
        auto result7 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::success == result7.ec);
    }
    SECTION("Boundary tests")
    {
        std::vector<uint8_t> seq1 = {0xC2, 0x80};
        auto result1 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::success == result1.ec);

        std::vector<uint8_t> seq2 = {0xC2, 0xBF};
        auto result2 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::success == result2.ec);

        std::vector<uint8_t> seq3 = {0xE0, 0xA0, 0x80};
        auto result3 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::success == result3.ec);

        std::vector<uint8_t> seq4 = {0xED, 0x9F, 0xBF};
        auto result4 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::success == result4.ec);

        std::vector<uint8_t> seq5 = {0xF0, 0x90, 0x80, 0x80};
        auto result5 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::success == result5.ec);

        std::vector<uint8_t> seq6 = {0xF4, 0x8F, 0xBF, 0xBF};
        auto result6 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::success == result6.ec);
    }
    SECTION("Invalid isolated bytes")
    {
        std::vector<uint8_t> seq1 = {0x80};
        auto result1 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::source_illegal == result1.ec);

        std::vector<uint8_t> seq2 = {0x81};
        auto result2 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::source_illegal == result2.ec);

        std::vector<uint8_t> seq3 = {0x90};
        auto result3 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::source_illegal == result3.ec);

        std::vector<uint8_t> seq4 = {0xA0};
        auto result4 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::source_illegal == result4.ec);

        std::vector<uint8_t> seq5 = {0xBF};
        auto result5 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::source_illegal == result5.ec);

        std::vector<uint8_t> seq6 = {0xC0};
        auto result6 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::source_illegal == result6.ec);

        std::vector<uint8_t> seq7 = {0xC1};
        auto result7 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::source_illegal == result7.ec);  

        for (int c = 0xF5; c <= int(0xFF); ++c)
        {
            std::vector<uint8_t> seq = {uint8_t(c)};
            auto result = unicode_traits::validate(seq1.data(), seq1.size());
            CHECK(unicode_traits::conv_errc::source_illegal == result.ec);  
        }
    }
    SECTION("Invalid multiple continuation bytes without a starter")
    {
        std::vector<uint8_t> seq1 = {0x80, 0x80};
        auto result1 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::source_illegal == result1.ec);

        std::vector<uint8_t> seq2 = {0xBF, 0xBF};
        auto result2 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::source_illegal == result2.ec);

        std::vector<uint8_t> seq3 = {0x80, 0xBF, 0x80};
        auto result3 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::source_illegal == result3.ec);

        std::vector<uint8_t> seq4 = {0x80, 0x81, 0x82, 0x83};
        auto result4 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::source_illegal == result4.ec);
    }
    SECTION("Invalid continuation after ASCII")
    {
        std::vector<uint8_t> seq1 = {0x41, 0x80};
        auto result1 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::source_illegal == result1.ec);

        std::vector<uint8_t> seq2 = {0x7F, 0xBF};
        auto result2 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::source_illegal == result2.ec);

        std::vector<uint8_t> seq3 = {0x20, 0x80};
        auto result3 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::source_illegal == result3.ec);
    }
    SECTION("Invalid too few continuation bytes")
    {
        std::vector<uint8_t> seq1 = {0xC2};
        auto result1 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::source_exhausted == result1.ec);

        std::vector<uint8_t> seq2 = {0xE2, 0x82};
        auto result2 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::source_exhausted == result2.ec);

        std::vector<uint8_t> seq3 = {0xF0, 0x90, 0x80};
        auto result3 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::source_exhausted == result3.ec);
    }
    SECTION("Invalid: wrong continuation byte")
    {
        std::vector<uint8_t> seq1 = {0xC2, 0x7F};
        auto result1 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::bad_continuation_byte == result1.ec);

        std::vector<uint8_t> seq2 = {0xC2, 0xC0};
        auto result2 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::bad_continuation_byte == result2.ec);

        std::vector<uint8_t> seq3 = {0xE2, 0x28, 0xA1};
        auto result3 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::bad_continuation_byte == result3.ec);

        std::vector<uint8_t> seq4 = {0xE2, 0xC0, 0x80};
        auto result4 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::bad_continuation_byte == result4.ec);

        std::vector<uint8_t> seq5 = {0xF0, 0x90, 0x41, 0x80};
        auto result5 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::bad_continuation_byte == result5.ec);

        std::vector<uint8_t> seq6 = {0xF0, 0x7F, 0x80, 0x80};
        auto result6 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::bad_continuation_byte == result6.ec);
    }
    SECTION("Invalid: extra continuation byte")
    {
        std::vector<uint8_t> seq1 = {0xC2, 0x80, 0x80};
        auto result1 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::source_illegal == result1.ec);

        std::vector<uint8_t> seq2 = {0xE2, 0x82, 0xAC, 0x80};
        auto result2 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::source_illegal == result2.ec);

        std::vector<uint8_t> seq3 = {0xF0, 0x90, 0x80, 0x80};
        auto result3 = unicode_traits::validate(seq1.data(), seq1.size());
        CHECK(unicode_traits::conv_errc::source_illegal == result3.ec);
    }
}


