// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <string>
#include <catch/catch.hpp>

namespace {

    class my_in_place_updater : public jsoncons::default_json_visitor
    {
        char* data_;
        std::size_t length_;
        std::string from_;
        std::string to_;
    public:
        using jsoncons::default_json_visitor::string_view_type;

        my_in_place_updater(char* data, std::size_t length, 
                            const std::string& from, const std::string& to)
            : data_(data), length_(length), 
              from_(from), to_(to)
        {
            (void)length_;
        }
        JSONCONS_VISITOR_RETURN_TYPE visit_string(const string_view_type& value,
                          jsoncons::semantic_tag,
                          const jsoncons::ser_context& context,
                          std::error_code&) override
        {
            if (value == from_)
            {
                //std::copy(to_.begin(), to_.end(), data_ + context.position() + 1);
                auto p = data_ + context.position() + 1;
                for (auto it = to_.begin(); it != to_.end(); ++it)
                {
                    *p++ = *it;
                }
            }
            JSONCONS_VISITOR_RETURN;
        }
    };

}

TEST_CASE("json in-place update tests")
{
    SECTION("test compact\n")
    {
        const std::string str1 = R"({"items": [{"id":1, "name" : "abc", "expiry" : "0420"}, { "id":2,"name" : ")";
        const std::string str2 = R"(","expiry" : "0720" }] })";

        std::string input = str1 + "id" + str2;
        std::string expected = str1 + "ab" + str2;

        my_in_place_updater updater(&input[0], input.size(), "id", "ab");
        jsoncons::json_string_reader reader(input, updater);
        reader.read();
        CHECK(std::string(input) == std::string(expected));
    }
    SECTION("test \n")
    {
        const std::string str1 = "{\"items\"\n:\n \n[\n{\"id\"\n:\n1\n,\n \"name\" \n:\n \"abc\"\n,\n \"expiry\" \n:\n \"0420\"\n}\n\n,\n { \"id\"\n:\n2\n,\n\"name\" \n:\n \"";
        const std::string str2 = "\"\n,\n\"expiry\" \n:\n \"0720\" \n}\n\n]\n \n}";
        std::string format = str1 + "%s" + str2;

        std::string input = str1 + "id" + str2;
        std::string expected = str1 + "ab" + str2;

        my_in_place_updater updater(&input[0], input.size(), "id", "ab");
        jsoncons::json_string_reader reader(input, updater);
        reader.read();
        CHECK(std::string(input) == std::string(expected));
    }
    SECTION("test \r\n")
    {
        //std::string format = "{\"items\"\r\n:\r\n \r\n[\r\n{\"id\"\r\n:\r\n1\r\n,\r\n \"name\" \r\n:\r\n \"abc\"\r\n,\r\n \"foo\" \r\n:\r\n 1000.0e-50\r\n,\r\n \"expiry\" \r\n:\r\n \"0420\"\r\n}\r\n\r\n,\r\n { \"id\"\r\n:\r\n2\r\n,\r\n\"name\" \r\n:\r\n \"%s\"\r\n,\r\n\"expiry\" \r\n:\r\n \"0720\" \r\n}\r\n\r\n]\r\n \r\n}";
        const std::string str1 = "{\"items\"\r\n:\r\n \r\n[\r\n{\"id\"\r\n:\r\n1\r\n,\r\n \"name\" \r\n:\r\n \"abc\"\r\n,\r\n \"foo\" \r\n:\r\n 1000.0e-50\r\n,\r\n \"expiry\" \r\n:\r\n \"0420\"\r\n}\r\n\r\n,\r\n { \"id\"\r\n:\r\n2\r\n,\r\n\"name\" \r\n:\r\n \"";
        const std::string str2 = "\"\r\n,\r\n\"expiry\" \r\n:\r\n \"0720\" \r\n}\r\n\r\n]\r\n \r\n}";
        std::string format = str1 + "%s" + str2;

        std::string input = str1 + "id" + str2;
        std::string expected = str1 + "ab" + str2;

        my_in_place_updater updater(&input[0], input.size(), "id", "ab");
        jsoncons::json_string_reader reader(input, updater);
        reader.read();
        CHECK(std::string(input) == std::string(expected));
    }
    SECTION("test \r")
    {
        //std::string format = "{\"items\"\r:\r \r[\r{\"id\"\r:\r1\r,\r \"name\" \r:\r \"abc\"\r,\r \"expiry\" \r:\r \"0420\"\r}\r\r,\r { \"id\"\r:\r2\r,\r\"name\" \r:\r \"%s\"\r,\r\"expiry\" \r:\r \"0720\" \r}\r\r]\r \r}";
        const std::string str1 = "{\"items\"\r:\r \r[\r{\"id\"\r:\r1\r,\r \"name\" \r:\r \"abc\"\r,\r \"expiry\" \r:\r \"0420\"\r}\r\r,\r { \"id\"\r:\r2\r,\r\"name\" \r:\r \"";
        const std::string str2 = "\"\r,\r\"expiry\" \r:\r \"0720\" \r}\r\r]\r \r}";
        std::string format = str1 + "%s" + str2;

        std::string input = str1 + "id" + str2;
        std::string expected = str1 + "ab" + str2;

        my_in_place_updater updater(&input[0], input.size(), "id", "ab");
        jsoncons::json_string_reader reader(input, updater);
        reader.read();
        CHECK(std::string(input) == std::string(expected));
    }
    SECTION("test /**/ \n")
    {
        //std::string format = "/*\n       \n        \n*/\"%s\"";
        const std::string str1 = "/*\n       \n        \n*/\"";
        const std::string str2 = "\"";
        std::string format = str1 + "%s" + str2;

        std::string input = str1 + "id" + str2;
        std::string expected = str1 + "ab" + str2;

        my_in_place_updater updater(&input[0], input.size(), "id", "ab");
        jsoncons::json_string_reader reader(input, updater);
        reader.read();
        CHECK(std::string(input) == std::string(expected));
    }
    SECTION("test /**/ \r")
    {
        //std::string format = "/*\r       \r        \r*/\"%s\"";
        const std::string str1 = "/*\r       \r        \r*/\"";
        const std::string str2 = "\"";
        std::string format = str1 + "%s" + str2;

        std::string input = str1 + "id" + str2;
        std::string expected = str1 + "ab" + str2;

        my_in_place_updater updater(&input[0], input.size(), "id", "ab");
        jsoncons::json_string_reader reader(input, updater);
        reader.read();
        CHECK(std::string(input) == std::string(expected));
    }
    SECTION("test /**/ \r\n")
    {
        //std::string format = "/*\r\n       \r\n        \r\n*/\"%s\"";
        const std::string str1 = "/*\r\n       \r\n        \r\n*/\"";
        const std::string str2 = "\"";
        std::string format = str1 + "%s" + str2;

        std::string input = str1 + "id" + str2;
        std::string expected = str1 + "ab" + str2;

        my_in_place_updater updater(&input[0], input.size(), "id", "ab");
        jsoncons::json_string_reader reader(input, updater);
        reader.read();
        CHECK(std::string(input) == std::string(expected));
    }
    SECTION("test // \n")
    {
        //std::string format = "//               \n\"%s\"";
        const std::string str1 = "//               \n\"";
        const std::string str2 = "\"";
        std::string format = str1 + "%s" + str2;

        std::string input = str1 + "id" + str2;
        std::string expected = str1 + "ab" + str2;

        my_in_place_updater updater(&input[0], input.size(), "id", "ab");
        jsoncons::json_string_reader reader(input, updater);
        reader.read();
        CHECK(std::string(input) == std::string(expected));
    }
    SECTION("test // \r")
    {
        //std::string format = "//               \r\"%s\"";
        const std::string str1 = "//               \r\"";
        const std::string str2 = "\"";
        std::string format = str1 + "%s" + str2;

        std::string input = str1 + "id" + str2;
        std::string expected = str1 + "ab" + str2;

        my_in_place_updater updater(&input[0], input.size(), "id", "ab");
        jsoncons::json_string_reader reader(input, updater);
        reader.read();
        CHECK(std::string(input) == std::string(expected));
    }
    SECTION("test // \r\n")
    {
        //std::string format = "//               \r\n\"%s\"";
        const std::string str1 = "//               \r\n\"";
        const std::string str2 = "\"";
        std::string format = str1 + "%s" + str2;

        std::string input = str1 + "id" + str2;
        std::string expected = str1 + "ab" + str2;

        my_in_place_updater updater(&input[0], input.size(), "id", "ab");
        jsoncons::json_string_reader reader(input, updater);
        reader.read();
        CHECK(std::string(input) == std::string(expected));
    }
}

