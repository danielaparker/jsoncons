// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json_reader.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_decoder.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <fstream>

using namespace jsoncons;

TEST_CASE("json_reader exception tests")
{
    SECTION("filename invalid")
    {
        std::string in_file = "./corelib/input/json-exception--1.json";
        std::ifstream is(in_file, std::ios::binary);
    
        jsoncons::json_decoder<jsoncons::json> decoder;
    
        json_stream_reader reader(is,decoder);
        REQUIRE_THROWS(reader.read_next());

        CHECK(false == decoder.is_valid());
    }
    SECTION("test_exception_left_brace")
    {
        std::string in_file = "./corelib/input/json-exception-1.json";
        std::ifstream is(in_file);
        REQUIRE(is);
    
        jsoncons::json_decoder<jsoncons::json> decoder;
        JSONCONS_TRY
        {
            json_stream_reader reader(is,decoder);
            reader.read_next();
        }
        JSONCONS_CATCH (const ser_error& e)
        {
            CHECK(e.code() == json_errc::expected_comma_or_rbracket);
            CHECK(14 == e.line());
            CHECK(30 == e.column());
        }
        CHECK(false == decoder.is_valid());
    }
    SECTION("test_exception_rbrace")
    {
        std::string in_file = "./corelib/input/json-exception-2.json";
        std::ifstream is(in_file);
        REQUIRE(is);
    
        jsoncons::json_decoder<jsoncons::json> decoder;
        JSONCONS_TRY
        {
            json_stream_reader reader(is,decoder);
            reader.read_next(); 
            CHECK(false);
        }
        JSONCONS_CATCH (const ser_error& e)
        {
            //std::cout << e.what() << '\n';
            CHECK(e.code() == json_errc::expected_comma_or_rbrace);
            CHECK(17 == e.line());
            CHECK(6 == e.column());
        }
        CHECK(false == decoder.is_valid());
    }
    
    SECTION("test_exception_array_eof")
    {
        std::istringstream is("[100");
    
        jsoncons::json_decoder<jsoncons::json> decoder;
        JSONCONS_TRY
        {
            json_stream_reader reader(is,decoder);
            reader.read_next(); 
            CHECK(false);
        }
        JSONCONS_CATCH (const ser_error& e)
        {
            CHECK(e.code() == json_errc::unexpected_eof);
            CHECK(1 == e.line());
            CHECK(5 == e.column());
        }
        CHECK(false == decoder.is_valid());
    }
    
    SECTION("test_exception_unicode_eof")
    {
        std::istringstream is("[\"\\u");
    
        jsoncons::json_decoder<jsoncons::json> decoder;
        JSONCONS_TRY
        {
            json_stream_reader reader(is,decoder);
            reader.read_next(); 
            CHECK(false);
        }
        JSONCONS_CATCH (const ser_error& e)
        {
            //std::cout << e.what() << '\n';
            CHECK(e.code() == json_errc::unexpected_eof);
            CHECK(1 == e.line());
            CHECK(5 == e.column());
        }
        CHECK(false == decoder.is_valid());
    }
    
    SECTION("test_exception_tru_eof")
    {
        std::istringstream is("[tru");
    
        jsoncons::json_decoder<jsoncons::json> decoder;
        JSONCONS_TRY
        {
            json_stream_reader reader(is,decoder);
            reader.read_next(); 
            CHECK(false);
        }
        JSONCONS_CATCH (const ser_error& e)
        {
            //std::cout << e.what() << '\n';
            CHECK(e.code() == json_errc::unexpected_eof);
            CHECK(1 == e.line());
            CHECK(5 == e.column());
        }
        CHECK(false == decoder.is_valid());
    }
    
    SECTION("test_exception_fals_eof")
    {
        std::istringstream is("[fals");
    
        jsoncons::json_decoder<jsoncons::json> decoder;
        JSONCONS_TRY
        {
            json_stream_reader reader(is,decoder);
            reader.read_next(); 
            CHECK(false);
        }
        JSONCONS_CATCH (const ser_error& e)
        {
            //std::cout << e.what() << '\n';
            CHECK(e.code() == json_errc::unexpected_eof);
            CHECK(1 == e.line());
            CHECK(6 == e.column());
        }
        CHECK(false == decoder.is_valid());
    }
    
    SECTION("test_exception_nul_eof")
    {
        std::istringstream is("[nul");
    
        jsoncons::json_decoder<jsoncons::json> decoder;
        JSONCONS_TRY
        {
            json_stream_reader reader(is,decoder);
            reader.read_next(); 
            CHECK(false);
        }
        JSONCONS_CATCH (const ser_error& e)
        {
            //std::cout << e.what() << '\n';
            CHECK(e.code() == json_errc::unexpected_eof);
            CHECK(1 == e.line());
            CHECK(5 == e.column());
        }
        CHECK(false == decoder.is_valid());
    }
    
    SECTION("json_errc::unexpected_eof false true")
    {
        std::istringstream is("[true");
    
        jsoncons::json_decoder<jsoncons::json> decoder;
        JSONCONS_TRY
        {
            json_stream_reader reader(is,decoder);
            reader.read_next(); 
            CHECK(false);
        }
        JSONCONS_CATCH (const ser_error& e)
        {
            CHECK(e.code() == json_errc::unexpected_eof);
            CHECK(1 == e.line());
            CHECK(6 == e.column());
        }
        CHECK(false == decoder.is_valid());
    }
    
    SECTION("json_errc::unexpected_eof false")
    {
        std::istringstream is("[false");
    
        jsoncons::json_decoder<jsoncons::json> decoder;
        JSONCONS_TRY
        {
            json_stream_reader reader(is,decoder);
            reader.read_next(); 
            CHECK(false);
        }
        JSONCONS_CATCH (const ser_error& e)
        {
            CHECK(e.code() == json_errc::unexpected_eof);
            CHECK(1 == e.line());
            CHECK(7 == e.column());
        }
        CHECK(false == decoder.is_valid());
    }
    
    SECTION("json_errc::unexpected_eof null")
    {
        std::istringstream is("[null");
    
        jsoncons::json_decoder<jsoncons::json> decoder;
        JSONCONS_TRY
        {
            json_stream_reader reader(is,decoder);
            reader.read_next(); 
            CHECK(false);
        }
        JSONCONS_CATCH (const ser_error& e)
        {
            CHECK(e.code() == json_errc::unexpected_eof);
            CHECK(1 == e.line());
            CHECK(6 == e.column());
        }
        CHECK(false == decoder.is_valid());
    }
    
    SECTION("unexpected_eof quote char")
    {
        std::string input("{\"field1\":\n\"value}");
        REQUIRE_THROWS_AS(jsoncons::json::parse(input),ser_error);
        JSONCONS_TRY
        {
            jsoncons::json::parse(input);
            CHECK(false);
        }
        JSONCONS_CATCH (const ser_error& e)
        {
            CHECK(jsoncons::json_errc::unexpected_eof == e.code());
            CHECK(2 == e.line());
            CHECK(8 == e.column());
        }
    }
}


