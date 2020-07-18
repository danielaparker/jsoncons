// Copyright 2016 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons_ext/bson/bson.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <fstream>
#include <catch/catch.hpp>

using namespace jsoncons;

namespace
{
    std::vector<char> read_bytes(const std::string& filename)
    {
        std::vector<char> bytes;
        std::string in_file = "./bson/input/test34.bson";
        std::ifstream is(filename, std::ifstream::binary);
        REQUIRE(is);
        is.seekg (0, is.end);
        std::streamoff length = is.tellg();
        is.seekg (0, is.beg);
        bytes.resize(length);
        is.read(bytes.data(), length);
        is.close();
        return bytes;
    }
}

// From https://github.com/mongodb/libbson/tree/master/tests

TEST_CASE("bson c test suite")
{
    SECTION("int32")
    {
        std::string in_file = "./bson/input/test33.bson";
        std::vector<char> bytes = read_bytes(in_file);

        std::vector<char> bytes2;
        ojson j(json_object_arg);
        j.try_emplace("a", -123);
        j.try_emplace("c", 0);
        j.try_emplace("b", 123);
        REQUIRE_NOTHROW(bson::encode_bson(j, bytes2));
        CHECK(bytes2 == bytes);

        auto j2 = bson::decode_bson<ojson>(bytes);
        CHECK(j2 == j);
    }
    SECTION("int64")
    {
        std::string in_file = "./bson/input/test34.bson";
        std::vector<char> bytes = read_bytes(in_file);

        bytes_source source(bytes);

        uint8_t buf[sizeof(int64_t)]; 

        source.read(buf, sizeof(int32_t));
        auto doc_size = jsoncons::detail::little_to_native<int32_t>(buf, sizeof(buf));
        REQUIRE(doc_size == 16);
        auto result = source.get_character();
        REQUIRE(result);
        REQUIRE(result.value() == 0x12); // 64-bit integer
        std::string s;
        for (result = source.get_character();
             result && result.value() != 0; result = source.get_character())
        {
            s.push_back(result.value());
        }
        REQUIRE(s == std::string("a"));
        source.read(buf, sizeof(int64_t));
        auto val = jsoncons::detail::little_to_native<int64_t>(buf, sizeof(int64_t));
        CHECK(val == 100000000000000ULL);
        result = source.get_character();
        REQUIRE(result);
        CHECK(result.value() == 0);
        CHECK(source.eof());

        std::vector<char> bytes2;
        std::map<std::string, int64_t> m{ {"a", val} };
        REQUIRE_NOTHROW(bson::encode_bson(m, bytes2));
        CHECK(bytes2 == bytes);

        auto m2 = bson::decode_bson<std::map<std::string, int64_t>>(bytes);
        CHECK(m2 == m);
    }
    SECTION("double")
    {
        std::string in_file = "./bson/input/test20.bson";
        std::vector<char> bytes = read_bytes(in_file);

        std::vector<char> bytes2;
        std::map<std::string, double> m = { {"double", 123.4567} };
        REQUIRE_NOTHROW(bson::encode_bson(m, bytes2));
        CHECK(bytes2 == bytes);

        auto m2 = bson::decode_bson<std::map<std::string, double>>(bytes);
        CHECK(m2 == m);
    }
    SECTION("bool")
    {
        std::string in_file = "./bson/input/test19.bson";
        std::vector<char> bytes = read_bytes(in_file);

        std::vector<char> bytes2;
        std::map<std::string, bool> m = { {"bool", true} };
        REQUIRE_NOTHROW(bson::encode_bson(m, bytes2));
        CHECK(bytes2 == bytes);

        auto m2 = bson::decode_bson<std::map<std::string, bool>>(bytes);
        CHECK(m2 == m);
    }
    SECTION("array")
    {
        std::string in_file = "./bson/input/test23.bson";
        std::vector<char> bytes = read_bytes(in_file);

        std::vector<char> bytes2;
        ojson a(json_array_arg);
        a.push_back("hello");
        a.push_back("world");

        ojson b;
        b["array"] = std::move(a);

        REQUIRE_NOTHROW(bson::encode_bson(b, bytes2));
        CHECK(bytes2 == bytes);

        auto b2 = bson::decode_bson<ojson>(bytes);
        CHECK(b2 == b);
    }
    SECTION("binary")
    {
        std::string in_file = "./bson/input/test24.bson";
        std::vector<char> bytes = read_bytes(in_file);

        std::vector<char> bytes2;
        std::vector<uint8_t> bstr = {'1', '2', '3', '4'};

        json b;
        b.try_emplace("binary", byte_string_arg, bstr, 0x80);

        REQUIRE_NOTHROW(bson::encode_bson(b, bytes2));
        //std::cout << byte_string_view(bytes2) << "\n\n";
        //std::cout << byte_string_view(bytes) << "\n\n";
        CHECK(bytes2 == bytes);

        auto b2 = bson::decode_bson<json>(bytes);
        CHECK(b2 == b);
    }
    SECTION("binary (jsoncons default)")
    {
        std::string in_file = "./bson/input/test24.bson";
        std::vector<char> bytes = read_bytes(in_file);

        std::vector<char> bytes2;
        std::vector<uint8_t> bstr = {'1', '2', '3', '4'};

        json b;
        b.try_emplace("binary", byte_string_arg, bstr); // default subtype is user defined

        REQUIRE_NOTHROW(bson::encode_bson(b, bytes2));
        //std::cout << byte_string_view(bytes2) << "\n\n";
        //std::cout << byte_string_view(bytes) << "\n\n";
        CHECK(bytes2 == bytes);

        auto b2 = bson::decode_bson<json>(bytes);
        CHECK(b2 == b);
    }
    SECTION("null")
    {
        std::string in_file = "./bson/input/test18.bson";
        std::vector<char> bytes = read_bytes(in_file);

        json j;
        j.try_emplace("hello", null_type()); 

        std::vector<char> bytes2;
        REQUIRE_NOTHROW(bson::encode_bson(j, bytes2));
        //std::cout << byte_string_view(bytes2) << "\n\n";
        //std::cout << byte_string_view(bytes) << "\n\n";
        CHECK(bytes2 == bytes);

        auto b2 = bson::decode_bson<json>(bytes);
        CHECK(b2 == j);
    }
    SECTION("utf8")
    {
        std::string in_file = "./bson/input/test11.bson";
        std::vector<char> bytes = read_bytes(in_file);

        json j;
        j.try_emplace("hello", "world"); 

        std::vector<char> bytes2;
        REQUIRE_NOTHROW(bson::encode_bson(j, bytes2));
        //std::cout << byte_string_view(bytes2) << "\n\n";
        //std::cout << byte_string_view(bytes) << "\n\n";
        CHECK(bytes2 == bytes);

        auto b2 = bson::decode_bson<json>(bytes);
        CHECK(b2 == j);
    }
    SECTION("document")
    {
        std::string in_file = "./bson/input/test21.bson";
        std::vector<char> bytes = read_bytes(in_file);

        json b;
        b.try_emplace("document", json()); 

        std::vector<char> bytes2;
        REQUIRE_NOTHROW(bson::encode_bson(b, bytes2));
        //std::cout << byte_string_view(bytes2) << "\n\n";
        //std::cout << byte_string_view(bytes) << "\n\n";
        CHECK(bytes2 == bytes);

        auto b2 = bson::decode_bson<json>(bytes);
        CHECK(b2 == b);
    }
}

