// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <jsoncons/uri.hpp>
#include <catch/catch.hpp>

TEST_CASE("uri tests (https://en.wikipedia.org/wiki/Uniform_Resource_Identifier)")
{
    SECTION("https://john.doe@www.example.com:123/forum/questions/?tag=networking&order=newest#top")
    {
        std::string s = "https://john.doe@www.example.com:123/forum/questions/?tag=networking&order=newest#top";

        jsoncons::uri uri(s); 

        CHECK(uri.scheme() == jsoncons::string_view("https"));
        CHECK(uri.authority() == jsoncons::string_view("john.doe@www.example.com:123"));
        CHECK(uri.userinfo() == jsoncons::string_view("john.doe"));
        CHECK(uri.host() == jsoncons::string_view("www.example.com"));
        CHECK(uri.port() == jsoncons::string_view("123"));
        CHECK(uri.path() == jsoncons::string_view("/forum/questions/"));
        CHECK(uri.query() == jsoncons::string_view("tag=networking&order=newest"));
        CHECK(uri.fragment() == jsoncons::string_view("top"));
        CHECK(uri.base() == "https://john.doe@www.example.com:123/forum/questions/");
        CHECK(uri.is_absolute());
    }
    SECTION("ldap://[2001:db8::7]/c=GB?objectClass?one")
    {
        std::string s = "ldap://[2001:db8::7]/c=GB?objectClass?one";

        jsoncons::uri uri(s); 

        CHECK(uri.scheme() == jsoncons::string_view("ldap"));
        CHECK(uri.authority() == jsoncons::string_view("2001:db8::7"));
        CHECK(uri.userinfo() == jsoncons::string_view(""));
        CHECK(uri.host() == jsoncons::string_view("2001:db8::7"));
        CHECK(uri.port() == jsoncons::string_view(""));
        CHECK(uri.path() == jsoncons::string_view("/c=GB"));
        CHECK(uri.query() == jsoncons::string_view("objectClass?one"));
        CHECK(uri.fragment() == jsoncons::string_view(""));
        CHECK(uri.is_absolute());
    }
    SECTION("mailto:John.Doe@example.com")
    {
        std::string s = "mailto:John.Doe@example.com";

        jsoncons::uri uri(s); 

        CHECK(uri.scheme() == jsoncons::string_view("mailto"));
        CHECK(uri.authority() == jsoncons::string_view(""));
        CHECK(uri.userinfo() == jsoncons::string_view(""));
        CHECK(uri.host() == jsoncons::string_view(""));
        CHECK(uri.port() == jsoncons::string_view(""));
        CHECK(uri.path() == jsoncons::string_view("John.Doe@example.com"));
        CHECK(uri.query() == jsoncons::string_view(""));
        CHECK(uri.fragment() == jsoncons::string_view(""));
        CHECK(uri.is_absolute());
    }
    SECTION("news:comp.infosystems.www.servers.unix")
    {
        std::string s = "news:comp.infosystems.www.servers.unix";

        jsoncons::uri uri(s); 

        CHECK(uri.scheme() == jsoncons::string_view("news"));
        CHECK(uri.authority() == jsoncons::string_view(""));
        CHECK(uri.userinfo() == jsoncons::string_view(""));
        CHECK(uri.host() == jsoncons::string_view(""));
        CHECK(uri.port() == jsoncons::string_view(""));
        CHECK(uri.path() == jsoncons::string_view("comp.infosystems.www.servers.unix"));
        CHECK(uri.query() == jsoncons::string_view(""));
        CHECK(uri.fragment() == jsoncons::string_view(""));
        CHECK(uri.is_absolute());
    }
    SECTION("tel:+1-816-555-1212")
    {
        std::string s = "tel:+1-816-555-1212";

        jsoncons::uri uri(s); 

        CHECK(uri.scheme() == jsoncons::string_view("tel"));
        CHECK(uri.authority() == jsoncons::string_view(""));
        CHECK(uri.userinfo() == jsoncons::string_view(""));
        CHECK(uri.host() == jsoncons::string_view(""));
        CHECK(uri.port() == jsoncons::string_view(""));
        CHECK(uri.path() == jsoncons::string_view("+1-816-555-1212"));
        CHECK(uri.query() == jsoncons::string_view(""));
        CHECK(uri.fragment() == jsoncons::string_view(""));
        CHECK(uri.is_absolute());
    }
    SECTION("telnet://192.0.2.16:80/")
    {
        std::string s = "telnet://192.0.2.16:80/";

        jsoncons::uri uri(s); 

        CHECK(uri.scheme() == jsoncons::string_view("telnet"));
        CHECK(uri.authority() == jsoncons::string_view("192.0.2.16:80"));
        CHECK(uri.userinfo() == jsoncons::string_view(""));
        CHECK(uri.host() == jsoncons::string_view("192.0.2.16"));
        CHECK(uri.port() == jsoncons::string_view("80"));
        CHECK(uri.path() == jsoncons::string_view("/"));
        CHECK(uri.query() == jsoncons::string_view(""));
        CHECK(uri.fragment() == jsoncons::string_view(""));
        CHECK(uri.is_absolute());
    }
    SECTION("urn:oasis:names:specification:docbook:dtd:xml:4.1.2")
    {
        std::string s = "urn:oasis:names:specification:docbook:dtd:xml:4.1.2";

        jsoncons::uri uri(s); 

        CHECK(uri.scheme() == jsoncons::string_view("urn"));
        CHECK(uri.authority() == jsoncons::string_view(""));
        CHECK(uri.userinfo() == jsoncons::string_view(""));
        CHECK(uri.host() == jsoncons::string_view(""));
        CHECK(uri.port() == jsoncons::string_view(""));
        CHECK(uri.path() == jsoncons::string_view("oasis:names:specification:docbook:dtd:xml:4.1.2"));
        CHECK(uri.query() == jsoncons::string_view(""));
        CHECK(uri.fragment() == jsoncons::string_view(""));
        CHECK(uri.is_absolute());
    }
}

TEST_CASE("uri fragment tests")
{
    SECTION("#/definitions/nonNegativeInteger")
    {
        std::string s = "#/definitions/nonNegativeInteger";

        jsoncons::uri uri(s); 

        CHECK(uri.scheme().empty());
        CHECK(uri.authority().empty());
        CHECK(uri.userinfo().empty());
        CHECK(uri.host().empty());
        CHECK(uri.port().empty());
        CHECK(uri.path().empty());
        CHECK(uri.query().empty());
        CHECK(uri.fragment() == jsoncons::string_view("/definitions/nonNegativeInteger"));
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
        CHECK(uri.authority() == jsoncons::string_view("json-schema.org"));
        CHECK(uri.userinfo().empty());
        CHECK(uri.host() == jsoncons::string_view("json-schema.org"));
        CHECK(uri.port().empty());
        CHECK(uri.path() == jsoncons::string_view("/draft-07/schema"));
        CHECK(uri.query().empty());
        CHECK(uri.fragment().empty());
        CHECK(uri.is_absolute());
    }
    SECTION("folder/")
    {
        std::string s = "folder/";

        jsoncons::uri uri(s); 

        CHECK(uri.scheme().empty());
        CHECK(uri.authority().empty());
        CHECK(uri.userinfo().empty());
        CHECK(uri.host().empty());
        CHECK(uri.port().empty());
        CHECK(uri.path() == jsoncons::string_view("folder/"));
        CHECK(uri.query().empty());
        CHECK(uri.fragment().empty());
        CHECK(!uri.is_absolute());
    }
    SECTION("name.json#/definitions/orNull")
    {
        std::string s = "name.json#/definitions/orNull";

        jsoncons::uri uri(s); 

        CHECK(uri.scheme().empty());
        CHECK(uri.authority().empty());
        CHECK(uri.userinfo().empty());
        CHECK(uri.host().empty());
        CHECK(uri.port().empty());
        CHECK(uri.path() == jsoncons::string_view("name.json"));
        CHECK(uri.query().empty());
        CHECK(uri.fragment() == jsoncons::string_view("/definitions/orNull"));
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
        CHECK(uri.authority() == jsoncons::string_view("localhost:1234"));
        CHECK(uri.userinfo().empty());
        CHECK(uri.host() == jsoncons::string_view("localhost"));
        CHECK(uri.port() == jsoncons::string_view("1234"));
        CHECK(uri.path() == jsoncons::string_view("/folder/"));
        CHECK(uri.query().empty());
        CHECK(uri.fragment().empty());
        CHECK(uri.is_absolute());
    }
    SECTION("folderInteger.json")
    {
        jsoncons::uri base_uri("http://localhost:1234/folder/"); 
        jsoncons::uri relative_uri("folderInteger.json");

        jsoncons::uri uri =  relative_uri.resolve(base_uri);

        CHECK(uri.scheme() == jsoncons::string_view("http"));
        CHECK(uri.authority() == jsoncons::string_view("localhost:1234"));
        CHECK(uri.userinfo().empty());
        CHECK(uri.host() == jsoncons::string_view("localhost"));
        CHECK(uri.port() == jsoncons::string_view("1234"));
        CHECK(uri.path() == jsoncons::string_view("/folder/folderInteger.json"));
        CHECK(uri.query().empty());
        CHECK(uri.fragment().empty());
        CHECK(uri.is_absolute());
    }
}
