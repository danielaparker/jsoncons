// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <new>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/json_query.hpp>

using namespace jsoncons;
using namespace jsoncons::jsonpath;

struct jsonpath_fixture
{
    static const char* store_text()
    {
        static const char* text = "{ \"store\": {\"book\": [ { \"category\": \"reference\",\"author\": \"Nigel Rees\",\"title\": \"Sayings of the Century\",\"price\": 8.95},{ \"category\": \"fiction\",\"author\": \"Evelyn Waugh\",\"title\": \"Sword of Honour\",\"price\": 12.99},{ \"category\": \"fiction\",\"author\": \"Herman Melville\",\"title\": \"Moby Dick\",\"isbn\": \"0-553-21311-3\",\"price\": 8.99},{ \"category\": \"fiction\",\"author\": \"J. R. R. Tolkien\",\"title\": \"The Lord of the Rings\",\"isbn\": \"0-395-19395-8\",\"price\": 22.99}],\"bicycle\": {\"color\": \"red\",\"price\": 19.95}}}";
        return text;
    }
    static const char* store_text_empty_isbn()
    {
        static const char* text = "{ \"store\": {\"book\": [ { \"category\": \"reference\",\"author\": \"Nigel Rees\",\"title\": \"Sayings of the Century\",\"price\": 8.95},{ \"category\": \"fiction\",\"author\": \"Evelyn Waugh\",\"title\": \"Sword of Honour\",\"price\": 12.99},{ \"category\": \"fiction\",\"author\": \"Herman Melville\",\"title\": \"Moby Dick\",\"isbn\": \"0-553-21311-3\",\"price\": 8.99},{ \"category\": \"fiction\",\"author\": \"J. R. R. Tolkien\",\"title\": \"The Lord of the Rings\",\"isbn\": \"\",\"price\": 22.99}],\"bicycle\": {\"color\": \"red\",\"price\": 19.95}}}";
        return text;
    }
    static const char* book_text()
    {
        static const char* text = "{ \"category\": \"reference\",\"author\": \"Nigel Rees\",\"title\": \"Sayings of the Century\",\"price\": 8.95}";
        return text;
    }

    json book()
    {
        json root = json::parse(jsonpath_fixture::store_text());
        json book = root["store"]["book"];
        return book;
    }

    json bicycle()
    {
        json root = json::parse(jsonpath_fixture::store_text());
        json bicycle = root["store"]["bicycle"];
        return bicycle;
    }
};

void test_error_code(const json& root, const std::string& path, int value, const std::error_category& category, size_t line, size_t column)
{
    REQUIRE_THROWS_AS(json_query(root,path),parse_error);
    try
    {
        json result = json_query(root,path);
    }
    catch (const parse_error& e)
    {
        CHECK((e.code().value() == value && e.code().category() == category));
        CHECK(e.line_number() == line);
        CHECK(e.column_number() == column);
    }
}

void test_error_code(const json& root, const std::string& path, std::error_code value, size_t line, size_t column)
{
    REQUIRE_THROWS_AS(json_query(root,path),parse_error);
    try
    {
        json result = json_query(root,path);
    }
    catch (const parse_error& e)
    {
        CHECK(e.code() == value);
        CHECK(e.line_number() == line);
        CHECK(e.column_number() == column);
    }
}

TEST_CASE("test_root_error")
{
    json root = json::parse(jsonpath_fixture::store_text());
    test_error_code(root, "..*", jsonpath_parser_errc::expected_root,1,1);
}

TEST_CASE("test_right_bracket_error")
{

    json root = json::parse(jsonpath_fixture::store_text());
    test_error_code(root, "$['store']['book'[*]", jsonpath_parser_errc::expected_right_bracket,1,18);
}

TEST_CASE("test_dot_dot_dot")
{

    json root = json::parse(jsonpath_fixture::store_text());
    test_error_code(root, "$.store...price", jsonpath_parser_errc::expected_name,1,10);
}

TEST_CASE("test_dot_star_name")
{

    json root = json::parse(jsonpath_fixture::store_text());
    test_error_code(root, "$.store.*price", jsonpath_parser_errc::expected_separator,1,10);
}

TEST_CASE("test_filter_error")
{
    json root = json::parse(jsonpath_fixture::store_text());
    test_error_code(root, "$..book[?(.price<10)]", json_parse_errc::invalid_json_text,1,17);
}




