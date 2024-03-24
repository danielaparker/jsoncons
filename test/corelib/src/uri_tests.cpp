// Copyright 2013-2024 Daniel Parker
// Distributed under Boost license

#include <jsoncons/uri.hpp>
#include <catch/catch.hpp>
#include <iostream>

TEST_CASE("uri tests (https://en.wikipedia.org/wiki/Uniform_Resource_Identifier)")
{
    SECTION("https://john.doe@www.example.com:123/forum/questions/?tag=networking&order=newest#top")
    {
        std::string s = "https://john.doe@www.example.com:123/forum/questions/?tag=networking&order=newest#top";

        jsoncons::uri uri(s); 

        CHECK(uri.scheme() == jsoncons::string_view("https"));
        CHECK(uri.encoded_authority() == jsoncons::string_view("john.doe@www.example.com:123"));
        CHECK(uri.userinfo() == jsoncons::string_view("john.doe"));
        CHECK(uri.host() == jsoncons::string_view("www.example.com"));
        CHECK(uri.port() == jsoncons::string_view("123"));
        CHECK(uri.path() == jsoncons::string_view("/forum/questions/"));
        CHECK(uri.encoded_query() == jsoncons::string_view("tag=networking&order=newest"));
        CHECK(uri.encoded_fragment() == jsoncons::string_view("top"));
        CHECK(uri.base().string() == "https://john.doe@www.example.com:123/forum/questions/");
        CHECK(uri.is_absolute());
    }
    SECTION("ldap://[2001:db8::7]/c=GB?objectClass?one")
    {
        std::string s = "ldap://[2001:db8::7]/c=GB?objectClass?one";

        jsoncons::uri uri(s); 

        CHECK(uri.scheme() == jsoncons::string_view("ldap"));
        CHECK(uri.encoded_authority() == jsoncons::string_view("2001:db8::7"));
        CHECK(uri.userinfo() == jsoncons::string_view(""));
        CHECK(uri.host() == jsoncons::string_view("2001:db8::7"));
        CHECK(uri.port() == jsoncons::string_view(""));
        CHECK(uri.encoded_path() == jsoncons::string_view("/c=GB"));
        CHECK(uri.encoded_query() == jsoncons::string_view("objectClass?one"));
        CHECK(uri.encoded_fragment() == jsoncons::string_view(""));
        CHECK(uri.is_absolute());
    }
    SECTION("mailto:John.Doe@example.com")
    {
        std::string s = "mailto:John.Doe@example.com";

        jsoncons::uri uri(s); 

        CHECK(uri.scheme() == jsoncons::string_view("mailto"));
        CHECK(uri.encoded_authority() == jsoncons::string_view(""));
        CHECK(uri.userinfo() == jsoncons::string_view(""));
        CHECK(uri.host() == jsoncons::string_view(""));
        CHECK(uri.port() == jsoncons::string_view(""));
        CHECK(uri.encoded_path() == jsoncons::string_view("John.Doe@example.com"));
        CHECK(uri.encoded_query() == jsoncons::string_view(""));
        CHECK(uri.encoded_fragment() == jsoncons::string_view(""));
        CHECK(uri.is_absolute());
    }
    SECTION("news:comp.infosystems.www.servers.unix")
    {
        std::string s = "news:comp.infosystems.www.servers.unix";

        jsoncons::uri uri(s); 

        CHECK(uri.scheme() == jsoncons::string_view("news"));
        CHECK(uri.encoded_authority() == jsoncons::string_view(""));
        CHECK(uri.userinfo() == jsoncons::string_view(""));
        CHECK(uri.host() == jsoncons::string_view(""));
        CHECK(uri.port() == jsoncons::string_view(""));
        CHECK(uri.encoded_path() == jsoncons::string_view("comp.infosystems.www.servers.unix"));
        CHECK(uri.encoded_query() == jsoncons::string_view(""));
        CHECK(uri.encoded_fragment() == jsoncons::string_view(""));
        CHECK(uri.is_absolute());
    }
    SECTION("tel:+1-816-555-1212")
    {
        std::string s = "tel:+1-816-555-1212";

        jsoncons::uri uri(s); 

        CHECK(uri.scheme() == jsoncons::string_view("tel"));
        CHECK(uri.encoded_authority() == jsoncons::string_view(""));
        CHECK(uri.userinfo() == jsoncons::string_view(""));
        CHECK(uri.host() == jsoncons::string_view(""));
        CHECK(uri.port() == jsoncons::string_view(""));
        CHECK(uri.encoded_path() == jsoncons::string_view("+1-816-555-1212"));
        CHECK(uri.encoded_query() == jsoncons::string_view(""));
        CHECK(uri.encoded_fragment() == jsoncons::string_view(""));
        CHECK(uri.is_absolute());
    }
    SECTION("telnet://192.0.2.16:80/")
    {
        std::string s = "telnet://192.0.2.16:80/";

        jsoncons::uri uri(s); 

        CHECK(uri.scheme() == jsoncons::string_view("telnet"));
        CHECK(uri.encoded_authority() == jsoncons::string_view("192.0.2.16:80"));
        CHECK(uri.userinfo() == jsoncons::string_view(""));
        CHECK(uri.host() == jsoncons::string_view("192.0.2.16"));
        CHECK(uri.port() == jsoncons::string_view("80"));
        CHECK(uri.encoded_path() == jsoncons::string_view("/"));
        CHECK(uri.encoded_query() == jsoncons::string_view(""));
        CHECK(uri.encoded_fragment() == jsoncons::string_view(""));
        CHECK(uri.is_absolute());
    }
    SECTION("urn:oasis:names:specification:docbook:dtd:xml:4.1.2")
    {
        std::string s = "urn:oasis:names:specification:docbook:dtd:xml:4.1.2";

        jsoncons::uri uri(s); 

        CHECK(uri.scheme() == jsoncons::string_view("urn"));
        CHECK(uri.encoded_authority() == jsoncons::string_view(""));
        CHECK(uri.userinfo() == jsoncons::string_view(""));
        CHECK(uri.host() == jsoncons::string_view(""));
        CHECK(uri.port() == jsoncons::string_view(""));
        CHECK(uri.encoded_path() == jsoncons::string_view("oasis:names:specification:docbook:dtd:xml:4.1.2"));
        CHECK(uri.encoded_query() == jsoncons::string_view(""));
        CHECK(uri.encoded_fragment() == jsoncons::string_view(""));
        CHECK(uri.is_absolute());
    }
    SECTION("urn:example:foo-bar-baz-qux?+CCResolve:cc=uk")
    {
        std::string s = "urn:example:foo-bar-baz-qux?+CCResolve:cc=uk";

        jsoncons::uri uri(s); 

        CHECK(uri.scheme() == jsoncons::string_view("urn"));
        CHECK(uri.encoded_authority() == jsoncons::string_view(""));
        CHECK(uri.userinfo() == jsoncons::string_view(""));
        CHECK(uri.host() == jsoncons::string_view(""));
        CHECK(uri.port() == jsoncons::string_view(""));
        CHECK(uri.encoded_path() == jsoncons::string_view("example:foo-bar-baz-qux"));
        CHECK(uri.encoded_query() == jsoncons::string_view("+CCResolve:cc=uk"));
        CHECK(uri.encoded_fragment() == jsoncons::string_view(""));
        CHECK(uri.is_absolute());

        jsoncons::uri relative("#/defs/bar");
        jsoncons::uri new_uri = relative.resolve(uri);
        std::cout << "new_uri: " << new_uri.string() << "\n"; 
    }
}

TEST_CASE("uri fragment tests")
{
    SECTION("#/definitions/nonNegativeInteger")
    {
        std::string s = "#/definitions/nonNegativeInteger";

        jsoncons::uri uri(s); 

        CHECK(uri.scheme().empty());
        CHECK(uri.encoded_authority().empty());
        CHECK(uri.userinfo().empty());
        CHECK(uri.host().empty());
        CHECK(uri.port().empty());
        CHECK(uri.encoded_path().empty());
        CHECK(uri.encoded_query().empty());
        CHECK(uri.encoded_fragment() == jsoncons::string_view("/definitions/nonNegativeInteger"));
        CHECK(!uri.is_absolute());
    }
}

TEST_CASE("uri base tests")
{
    SECTION("http://json-schema.org/draft-07/schema#")
    {
        std::string s = "http://json-schema.org/draft-07/schema#";

        jsoncons::uri uri(s); 

        CHECK(uri.scheme() == jsoncons::string_view("http"));
        CHECK(uri.encoded_authority() == jsoncons::string_view("json-schema.org"));
        CHECK(uri.userinfo().empty());
        CHECK(uri.host() == jsoncons::string_view("json-schema.org"));
        CHECK(uri.port().empty());
        CHECK(uri.encoded_path() == jsoncons::string_view("/draft-07/schema"));
        CHECK(uri.encoded_query().empty());
        CHECK(uri.encoded_fragment().empty());
        CHECK(uri.is_absolute());
    }
    SECTION("folder/")
    {
        std::string s = "folder/";

        jsoncons::uri uri(s); 

        CHECK(uri.scheme().empty());
        CHECK(uri.encoded_authority().empty());
        CHECK(uri.userinfo().empty());
        CHECK(uri.host().empty());
        CHECK(uri.port().empty());
        CHECK(uri.encoded_path() == jsoncons::string_view("folder/"));
        CHECK(uri.encoded_query().empty());
        CHECK(uri.encoded_fragment().empty());
        CHECK(!uri.is_absolute());
    }
    SECTION("name.json#/definitions/orNull")
    {
        std::string s = "name.json#/definitions/orNull";

        jsoncons::uri uri(s); 

        CHECK(uri.scheme().empty());
        CHECK(uri.encoded_authority().empty());
        CHECK(uri.userinfo().empty());
        CHECK(uri.host().empty());
        CHECK(uri.port().empty());
        CHECK(uri.encoded_path() == jsoncons::string_view("name.json"));
        CHECK(uri.encoded_query().empty());
        CHECK(uri.encoded_fragment() == jsoncons::string_view("/definitions/orNull"));
        CHECK(!uri.is_absolute());
    }
}

TEST_CASE("uri resolve tests")
{
    SECTION("folder/")
    {
        jsoncons::uri base_uri("http://localhost:1234/scope_change_defs2.json"); 
        jsoncons::uri relative_uri("folder/");
        
        jsoncons::uri uri =  relative_uri.resolve(base_uri);

        CHECK(uri.scheme() == jsoncons::string_view("http"));
        CHECK(uri.encoded_authority() == jsoncons::string_view("localhost:1234"));
        CHECK(uri.userinfo().empty());
        CHECK(uri.host() == jsoncons::string_view("localhost"));
        CHECK(uri.port() == jsoncons::string_view("1234"));
        CHECK(uri.encoded_path() == jsoncons::string_view("/folder/"));
        CHECK(uri.encoded_query().empty());
        CHECK(uri.encoded_fragment().empty());
        CHECK(uri.is_absolute());
    }
    SECTION("folderInteger.json")
    {
        jsoncons::uri base_uri("http://localhost:1234/folder/"); 
        jsoncons::uri relative_uri("folderInteger.json");

        jsoncons::uri uri =  relative_uri.resolve(base_uri);

        CHECK(uri.scheme() == jsoncons::string_view("http"));
        CHECK(uri.encoded_authority() == jsoncons::string_view("localhost:1234"));
        CHECK(uri.userinfo().empty());
        CHECK(uri.host() == jsoncons::string_view("localhost"));
        CHECK(uri.port() == jsoncons::string_view("1234"));
        CHECK(uri.encoded_path() == jsoncons::string_view("/folder/folderInteger.json"));
        CHECK(uri.encoded_query().empty());
        CHECK(uri.encoded_fragment().empty());
        CHECK(uri.is_absolute());
    }
}

TEST_CASE("uri part decode tests")
{
    SECTION("test 1")
    {
        std::string raw = "%7e";
        std::string expected = "~";

        std::string decoded = jsoncons::uri::decode_part(raw);
        CHECK(decoded == expected);
    }
    SECTION("test 2")
    {
        std::string raw = "%25";
        std::string expected = "%";

        std::string decoded = jsoncons::uri::decode_part(raw);
        CHECK(decoded == expected);
    }
    SECTION("test 3")
    {
        std::string raw = "foo%25bar%7ebaz";
        std::string expected = "foo%bar~baz";

        std::string decoded = jsoncons::uri::decode_part(raw);
        CHECK(decoded == expected);
    }
}

TEST_CASE("uri part encode tests")
{
    SECTION("test 1")
    {
        std::string part = "/@_-!.~'()*azAZ09,;:$&+=%3F%ae";
        std::string expected = part;

        std::string encoded;
        jsoncons::uri::encode_path(part, encoded);
        CHECK(encoded == expected);
    }
    
    SECTION("test 2")
    {
        std::string part = "%?/[]@,;:$&+=";
        std::string expected = "%25%3F/%5B%5D@,;:$&+=";

        std::string encoded;
        jsoncons::uri::encode_path(part, encoded);
        CHECK(encoded == expected);
    }
}

TEST_CASE("uri part encode illegal characters tests")
{
    SECTION("test 1")
    {
        std::string part = "_-!.~'()*azAZ09?/[]@,;:$&+=%3F%ae";
        std::string expected = part;

        std::string encoded;
        jsoncons::uri::encode_illegal_characters(part, encoded);
        CHECK(encoded == expected);
    }
}

TEST_CASE("uri constructors")
{
    SECTION("test 1")
    {
        jsoncons::uri x{"http://localhost:4242/draft2019-09/recursiveRef6/base.json"};

        jsoncons::uri y{ x, jsoncons::uri_fragment_part, "/anyOf" };

        jsoncons::uri expected{"http://localhost:4242/draft2019-09/recursiveRef6/base.json#/anyOf"};

        CHECK(expected == y);       
    }
}

