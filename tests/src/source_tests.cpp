// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/source.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <new>

TEST_CASE("stream_source tests")
{
    std::string data = "012345678";
    std::istringstream is(data);
    jsoncons::stream_source<char> source(is);

    SECTION("test get and peek")
    {
        auto p0 = source.peek();
        CHECK_FALSE(source.eof());
        CHECK(p0.value() == '0');
        CHECK(source.position() == 0);
        auto g0 = source.get();
        CHECK_FALSE(source.eof());
        CHECK(g0.value() == '0');
        CHECK(source.position() == 1);
        auto p1 = source.peek();
        CHECK_FALSE(source.eof());
        CHECK(p1.value() == '1');
        CHECK(source.position() == 1);
        auto g1 = source.get();
        CHECK_FALSE(source.eof());
        CHECK(g1.value() == '1');
        CHECK(source.position() == 2);
        auto g2 = source.get();
        CHECK_FALSE(source.eof());
        CHECK(g2.value() == '2');
        CHECK(source.position() == 3);
        auto g3 = source.get();
        CHECK_FALSE(source.eof());
        CHECK(g3.value() == '3');
        CHECK(source.position() == 4);

        auto p4 = source.peek();
        CHECK_FALSE(source.eof());
        CHECK(p4.value() == '4');
        CHECK(source.position() == 4);
        auto g4 = source.get();
        CHECK_FALSE(source.eof());
        CHECK(g4.value() == '4');
        CHECK(source.position() == 5);
        auto p5 = source.peek();
        CHECK_FALSE(source.eof());
        CHECK(p5.value() == '5');
        CHECK(source.position() == 5);
        auto g5 = source.get();
        CHECK_FALSE(source.eof());
        CHECK(g5.value() == '5');
        CHECK(source.position() == 6);
        auto g6 = source.get();
        CHECK_FALSE(source.eof());
        CHECK(g6.value() == '6');
        CHECK(source.position() == 7);
        auto g7 = source.get();
        CHECK_FALSE(source.eof());
        CHECK(g7.value() == '7');
        CHECK(source.position() == 8);
        auto g8 = source.get();
        CHECK_FALSE(source.eof());
        CHECK(g8.value() == '8');
        CHECK(source.position() == 9);
        /*auto g9 = */source.get();
        CHECK(source.eof());
    }
}

TEST_CASE("binary_stream_source tests")
{
    std::string data = "012345678";
    std::istringstream is(data);
    jsoncons::binary_stream_source source(is,4);

    SECTION("get and peek")
    {
        auto p0 = source.peek();
        CHECK_FALSE(source.eof());
        CHECK(p0.value() == '0');
        CHECK(source.position() == 0);
        auto g0 = source.get();
        CHECK_FALSE(source.eof());
        CHECK(g0.value() == '0');
        CHECK(source.position() == 1);
        auto p1 = source.peek();
        CHECK_FALSE(source.eof());
        CHECK(p1.value() == '1');
        CHECK(source.position() == 1);
        auto g1 = source.get();
        CHECK_FALSE(source.eof());
        CHECK(g1.value() == '1');
        CHECK(source.position() == 2);
        auto g2 = source.get();
        CHECK_FALSE(source.eof());
        CHECK(g2.value() == '2');
        CHECK(source.position() == 3);
        auto g3 = source.get();
        CHECK_FALSE(source.eof());
        CHECK(g3.value() == '3');
        CHECK(source.position() == 4);

        auto p4 = source.peek();
        CHECK_FALSE(source.eof());
        CHECK(p4.value() == '4');
        CHECK(source.position() == 4);
        auto g4 = source.get();
        CHECK_FALSE(source.eof());
        CHECK(g4.value() == '4');
        CHECK(source.position() == 5);
        auto p5 = source.peek();
        CHECK_FALSE(source.eof());
        CHECK(p5.value() == '5');
        CHECK(source.position() == 5);
        auto g5 = source.get();
        CHECK_FALSE(source.eof());
        CHECK(g5.value() == '5');
        CHECK(source.position() == 6);
        auto g6 = source.get();
        CHECK_FALSE(source.eof());
        CHECK(g6.value() == '6');
        CHECK(source.position() == 7);
        auto g7 = source.get();
        CHECK_FALSE(source.eof());
        CHECK(g7.value() == '7');
        CHECK(source.position() == 8);

        jsoncons::binary_stream_source source2(std::move(source));
        auto g8 = source2.get();
        CHECK_FALSE(source2.eof());
        CHECK(g8.value() == '8');
        CHECK(source2.position() == 9);
        /*auto g9 = */source2.get();
        CHECK(source2.eof());
    }

    SECTION("ignore")
    {
        source.ignore(1);
        CHECK_FALSE(source.eof());
        CHECK(source.position() == 1);
        auto p1 = source.peek();
        CHECK_FALSE(source.eof());
        CHECK(p1.value() == '1');
        CHECK(source.position() == 1);
        source.ignore(7);
        CHECK_FALSE(source.eof());
        CHECK(source.position() == 8);
        source.ignore(2);
        CHECK(source.eof());
        CHECK(source.position() == 9);
    }

    SECTION("read 1, read1")
    {
        std::vector<uint8_t> v(10);
        std::vector<uint8_t> expected = {'0','1','2','3','4','5','6','7','8'};

        std::size_t len = source.read(v.data(),1);
        CHECK_FALSE(source.eof());
        CHECK(len == 1);
        CHECK(std::equal(expected.begin(),expected.begin()+1,v.begin()));
        CHECK(source.position() == 1);

        len = source.read(v.data(),1);
        CHECK_FALSE(source.eof());
        CHECK(len == 1);
        CHECK(std::equal(expected.begin()+1,expected.begin()+2,v.begin()));
        CHECK(source.position() == 2);
    }

    SECTION("read 3, read 4, read 3")
    {
        std::vector<uint8_t> v(10);
        std::vector<uint8_t> expected = {'0','1','2','3','4','5','6','7','8'};

        std::size_t len = source.read(v.data(),3);
        CHECK_FALSE(source.eof());
        CHECK(len == 3);
        CHECK(std::equal(expected.begin(),expected.begin()+3,v.begin()));
        CHECK(source.position() == 3);
        //for (size_t i = 0; i < 3; ++i)
        //{
        //    std::cout << i << ": " << v[i] << " ";
        //}
        //std::cout << std::endl;

        len = source.read(v.data(),4);
        CHECK_FALSE(source.eof());
        CHECK(len == 4);
        CHECK(std::equal(expected.begin()+3,expected.begin()+7,v.begin()));
        CHECK(source.position() == 7);

        len = source.read(v.data(),3);
        CHECK_FALSE(source.eof());
        CHECK(len == 2);
        CHECK(std::equal(expected.begin()+7,expected.begin()+9,v.begin()));
        CHECK(source.position() == 9);
    }

    SECTION("read 9")
    {
        std::vector<uint8_t> v(10);
        std::vector<uint8_t> expected = {'0','1','2','3','4','5','6','7','8'};

        std::size_t len = source.read(v.data(),9);
        CHECK_FALSE(source.eof());
        CHECK(len == 9);
        CHECK(std::equal(expected.begin(),expected.end(),v.begin()));
        CHECK(source.position() == 9);
    }

    SECTION("read 10")
    {
        std::vector<uint8_t> v(10);
        std::vector<uint8_t> expected = { '0','1','2','3','4','5','6','7','8'};

        std::size_t len = source.read(v.data(), 10);
        CHECK_FALSE(source.eof());
        CHECK(len == 9);
        CHECK(std::equal(expected.begin(), expected.end(), v.begin()));
        CHECK(source.position() == 9);
    }
}


