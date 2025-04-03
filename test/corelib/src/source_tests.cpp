// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/source.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <new>
#include <catch/catch.hpp>

TEST_CASE("basic_null_istream tests")
{
    SECTION("test1")
    {
        jsoncons::basic_null_istream<char> is{ jsoncons::basic_null_istream<char>() };
    }
}

TEST_CASE("string_source tests")
{
    std::string data = "012345678";
    jsoncons::string_source<char> source(data);

    SECTION("test get and peek")
    {
        char b;

        auto p0 = source.peek();
        CHECK_FALSE(source.eof());
        CHECK(p0.value == '0');
        CHECK(0 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK_FALSE(source.eof());
        CHECK(b == '0');
        CHECK(1 == source.position());
        auto p1 = source.peek();
        CHECK_FALSE(source.eof());
        CHECK(p1.value == '1');
        CHECK(1 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK_FALSE(source.eof());
        CHECK(b == '1');
        CHECK(2 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK_FALSE(source.eof());
        CHECK(b == '2');
        CHECK(3 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK_FALSE(source.eof());
        CHECK(b == '3');
        CHECK(4 == source.position());

        auto p4 = source.peek();
        CHECK_FALSE(source.eof());
        CHECK(p4.value == '4');
        CHECK(4 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK_FALSE(source.eof());
        CHECK(b == '4');
        CHECK(5 == source.position());
        auto p5 = source.peek();
        CHECK_FALSE(source.eof());
        CHECK(p5.value == '5');
        CHECK(5 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK_FALSE(source.eof());
        CHECK(b == '5');
        CHECK(6 == source.position());
        CHECK(1 == source.read(&b,1));

        CHECK_FALSE(source.eof());
        CHECK(b == '6');
        CHECK(7 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK_FALSE(source.eof());
        CHECK(b == '7');
        CHECK(8 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK(source.eof());
        CHECK(b == '8');
        CHECK(9 == source.position());
        CHECK(0 == source.read(&b,1));
        CHECK(source.eof());
    }

    SECTION("read_buffer")
    {
        auto s = source.read_buffer();
        CHECK(source.eof());
        CHECK(9 == s.size());
        CHECK(std::equal(s.begin(), s.end(), data.begin()));
        CHECK(9 == source.position());
    }
}

TEST_CASE("byte_source tests")
{
    std::string data = "012345678";
    jsoncons::bytes_source source(data);

    SECTION("test get and peek")
    {
        uint8_t b;

        auto p0 = source.peek();
        CHECK_FALSE(source.eof());
        CHECK(p0.value == '0');
        CHECK(0 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK_FALSE(source.eof());
        CHECK(b == '0');
        CHECK(1 == source.position());
        auto p1 = source.peek();
        CHECK_FALSE(source.eof());
        CHECK(p1.value == '1');
        CHECK(1 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK_FALSE(source.eof());
        CHECK(b == '1');
        CHECK(2 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK_FALSE(source.eof());
        CHECK(b == '2');
        CHECK(3 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK_FALSE(source.eof());
        CHECK(b == '3');
        CHECK(4 == source.position());

        auto p4 = source.peek();
        CHECK_FALSE(source.eof());
        CHECK(p4.value == '4');
        CHECK(4 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK_FALSE(source.eof());
        CHECK(b == '4');
        CHECK(5 == source.position());
        auto p5 = source.peek();
        CHECK_FALSE(source.eof());
        CHECK(p5.value == '5');
        CHECK(5 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK_FALSE(source.eof());
        CHECK(b == '5');
        CHECK(6 == source.position());
        CHECK(1 == source.read(&b,1));

        CHECK_FALSE(source.eof());
        CHECK(b == '6');
        CHECK(7 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK_FALSE(source.eof());
        CHECK(b == '7');
        CHECK(8 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK(source.eof());
        CHECK(b == '8');
        CHECK(9 == source.position());
        CHECK(0 == source.read(&b,1));
        CHECK(source.eof());
    }

    SECTION("read_buffer")
    {
        auto s = source.read_buffer();
        CHECK(source.eof());
        CHECK(9 == s.size());
        CHECK(std::equal(s.begin(), s.end(), data.begin()));
        CHECK(9 == source.position());
    }
}

TEST_CASE("stream_source tests")
{
    std::string data = "012345678";
    std::istringstream is(data);
    jsoncons::stream_source<char> source(is);

    SECTION("test get and peek")
    {
        char b;

        auto p0 = source.peek();
        CHECK_FALSE(source.eof());
        CHECK(p0.value == '0');
        CHECK(0 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK_FALSE(source.eof());
        CHECK(b == '0');
        CHECK(1 == source.position());
        auto p1 = source.peek();
        CHECK_FALSE(source.eof());
        CHECK(p1.value == '1');
        CHECK(1 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK_FALSE(source.eof());
        CHECK(b == '1');
        CHECK(2 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK_FALSE(source.eof());
        CHECK(b == '2');
        CHECK(3 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK_FALSE(source.eof());
        CHECK(b == '3');
        CHECK(4 == source.position());

        auto p4 = source.peek();
        CHECK_FALSE(source.eof());
        CHECK(p4.value == '4');
        CHECK(4 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK_FALSE(source.eof());
        CHECK(b == '4');
        CHECK(5 == source.position());
        auto p5 = source.peek();
        CHECK_FALSE(source.eof());
        CHECK(p5.value == '5');
        CHECK(5 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK_FALSE(source.eof());
        CHECK(b == '5');
        CHECK(6 == source.position());
        CHECK(1 == source.read(&b,1));

        CHECK_FALSE(source.eof());
        CHECK(b == '6');
        CHECK(7 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK_FALSE(source.eof());
        CHECK(b == '7');
        CHECK(8 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK(source.eof());
        CHECK(b == '8');
        CHECK(9 == source.position());
        CHECK(0 == source.read(&b,1));
        CHECK(source.eof());
    }
}

TEST_CASE("wide stream source tests")
{
    std::wstring data = L"012345678";
    std::wistringstream is(data);
    jsoncons::stream_source<wchar_t> source(is);
}

TEST_CASE("binary_stream_source tests")
{
    std::string data = "012345678";
    std::istringstream is(data);
    jsoncons::binary_stream_source source(is,4);

    SECTION("get and peek")
    {
        uint8_t b;

        auto p0 = source.peek();
        CHECK_FALSE(source.eof());
        CHECK(p0.value == '0');
        CHECK(0 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK_FALSE(source.eof());
        CHECK(b == '0');
        CHECK(1 == source.position());
        auto p1 = source.peek();
        CHECK_FALSE(source.eof());
        CHECK(p1.value == '1');
        CHECK(1 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK_FALSE(source.eof());
        CHECK(b == '1');
        CHECK(2 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK_FALSE(source.eof());
        CHECK(b == '2');
        CHECK(3 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK_FALSE(source.eof());
        CHECK(b == '3');
        CHECK(4 == source.position());

        auto p4 = source.peek();
        CHECK_FALSE(source.eof());
        CHECK(p4.value == '4');
        CHECK(4 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK_FALSE(source.eof());
        CHECK(b == '4');
        CHECK(5 == source.position());
        auto p5 = source.peek();
        CHECK_FALSE(source.eof());
        CHECK(p5.value == '5');
        CHECK(5 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK_FALSE(source.eof());
        CHECK(b == '5');
        CHECK(6 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK_FALSE(source.eof());
        CHECK(b == '6');
        CHECK(7 == source.position());
        CHECK(1 == source.read(&b,1));
        CHECK_FALSE(source.eof());
        CHECK(b == '7');
        CHECK(8 == source.position());

        jsoncons::binary_stream_source source2(std::move(source));
        CHECK(1 == source2.read(&b,1));
        CHECK(source2.eof());
        CHECK(b == '8');
        CHECK(9 == source2.position());
        CHECK(0 == source2.read(&b,1));
        CHECK(source2.eof());
    }

    SECTION("ignore")
    {
        source.ignore(1);
        CHECK_FALSE(source.eof());
        CHECK(1 == source.position());
        auto p1 = source.peek();
        CHECK_FALSE(source.eof());
        CHECK(p1.value == '1');
        CHECK(1 == source.position());
        source.ignore(7);
        CHECK_FALSE(source.eof());
        CHECK(8 == source.position());
        source.ignore(2);
        CHECK(source.eof());
        CHECK(9 == source.position());
    }

    SECTION("read 1, read1")
    {
        std::vector<uint8_t> v(10);
        std::vector<uint8_t> expected = {'0','1','2','3','4','5','6','7','8'};

        std::size_t len = source.read(v.data(),1);
        CHECK_FALSE(source.eof());
        CHECK(1 == len);
        CHECK(std::equal(expected.begin(),expected.begin()+1,v.begin()));
        CHECK(1 == source.position());

        len = source.read(v.data(),1);
        CHECK_FALSE(source.eof());
        CHECK(1 == len);
        CHECK(std::equal(expected.begin()+1,expected.begin()+2,v.begin()));
        CHECK(2 == source.position());
    }

    SECTION("read 3, read 4, read 3")
    {
        std::vector<uint8_t> v(10);
        std::vector<uint8_t> expected = {'0','1','2','3','4','5','6','7','8'};

        std::size_t len = source.read(v.data(),3);
        CHECK_FALSE(source.eof());
        CHECK(3 == len);
        CHECK(std::equal(expected.begin(),expected.begin()+3,v.begin()));
        CHECK(3 == source.position());

        len = source.read(v.data(),4);
        CHECK_FALSE(source.eof());
        CHECK(4 == len);
        CHECK(std::equal(expected.begin()+3,expected.begin()+7,v.begin()));
        CHECK(7 == source.position());

        len = source.read(v.data(),3);
        CHECK(source.eof());
        CHECK(2 == len);
        CHECK(std::equal(expected.begin()+7,expected.begin()+9,v.begin()));
        CHECK(9 == source.position());
    }

    SECTION("read 9")
    {
        std::vector<uint8_t> v(10);
        std::vector<uint8_t> expected = {'0','1','2','3','4','5','6','7','8'};

        std::size_t len = source.read(v.data(),9);
        CHECK_FALSE(source.eof());
        CHECK(9 == len);
        CHECK(std::equal(expected.begin(),expected.end(),v.begin()));
        CHECK(9 == source.position());
    }

    SECTION("read 10")
    {
        std::vector<uint8_t> v(10);
        std::vector<uint8_t> expected = { '0','1','2','3','4','5','6','7','8'};

        std::size_t len = source.read(v.data(), 10);
        CHECK(source.eof());
        CHECK(9 == len);
        CHECK(std::equal(expected.begin(), expected.end(), v.begin()));
        CHECK(9 == source.position());
    }

    SECTION("read_buffer")
    {
        auto s = source.read_buffer();
        CHECK_FALSE(source.eof());
        CHECK(4 == s.size());
        CHECK(std::equal(s.begin(), s.end(), data.begin()));
        CHECK(4 == source.position());

        s = source.read_buffer();
        CHECK_FALSE(source.eof());
        CHECK(4 == s.size());
        CHECK(std::equal(s.begin(), s.end(), data.begin()+4));
        CHECK(8 == source.position());

        s = source.read_buffer();
        CHECK(source.eof());
        CHECK(1 == s.size());
        CHECK(std::equal(s.begin(), s.end(), data.begin()+8));
        CHECK(9 == source.position());
    }
}

TEST_CASE("random access iterator iterator_stream source tests")
{
    std::string data = "012345678";
    jsoncons::iterator_source<std::string::iterator> source(data.begin(), data.end(), 4);

    SECTION("read 3")
    {
        std::vector<char> v(3);
        source.read(v.data(), 3);
        CHECK_FALSE(source.eof());
        CHECK(std::equal(v.begin(), v.begin()+3, data.begin()));
        CHECK(3 == source.position());
    }

    SECTION("read_buffer")
    {
        auto s = source.read_buffer();
        CHECK_FALSE(source.eof());
        CHECK(4 == s.size());
        CHECK(std::equal(s.begin(), s.end(), data.begin()));
        CHECK(4 == source.position());

        s = source.read_buffer();
        CHECK_FALSE(source.eof());
        CHECK(4 == s.size());
        CHECK(std::equal(s.begin(), s.end(), data.begin()+4));
        CHECK(8 == source.position());

        s = source.read_buffer();
        CHECK(source.eof());
        CHECK(1 == s.size());
        CHECK(std::equal(s.begin(), s.end(), data.begin()+8));
        CHECK(9 == source.position());
    }
}

TEST_CASE("forward iterator iterator_stream source tests")
{
    std::string data = "012345678";
    std::istringstream is(data);
    std::istream_iterator<char> iter(is);
    jsoncons::iterator_source<std::istream_iterator<char>> source(iter, std::istream_iterator<char>(), 4);

    SECTION("read 3")
    {
        std::vector<char> v(3);
        source.read(v.data(), 3);
        CHECK_FALSE(source.eof());
        CHECK(std::equal(v.begin(), v.begin()+3, data.begin()));
        CHECK(3 == source.position());
    }

    SECTION("read_buffer")
    {
        auto s = source.read_buffer();
        CHECK_FALSE(source.eof());
        CHECK(4 == s.size());
        CHECK(std::equal(s.begin(), s.end(), data.begin()));
        CHECK(4 == source.position());

        s = source.read_buffer();
        CHECK_FALSE(source.eof());
        CHECK(4 == s.size());
        CHECK(std::equal(s.begin(), s.end(), data.begin()+4));
        CHECK(8 == source.position());

        s = source.read_buffer();
        CHECK(source.eof());
        CHECK(1 == s.size());
        CHECK(std::equal(s.begin(), s.end(), data.begin()+8));
        CHECK(9 == source.position());
    }
}

TEST_CASE("binary_iterator_stream source tests")
{
    std::vector<uint8_t> data = { 0,1,2,3,4,5,6,7,8 };
    jsoncons::binary_iterator_source<std::vector<uint8_t>::iterator> source(data.begin(), data.end(), 4);

    SECTION("read 3")
    {
        std::vector<uint8_t> v(3);
        source.read(v.data(), 3);
        CHECK_FALSE(source.eof());
        CHECK(std::equal(v.begin(), v.begin()+3, data.begin()));
        CHECK(3 == source.position());
    }

    SECTION("read_buffer")
    {
        auto s = source.read_buffer();
        CHECK_FALSE(source.eof());
        CHECK(4 == s.size());
        CHECK(std::equal(s.begin(), s.end(), data.begin()));
        CHECK(4 == source.position());

        s = source.read_buffer();
        CHECK_FALSE(source.eof());
        CHECK(4 == s.size());
        CHECK(std::equal(s.begin(), s.end(), data.begin()+4));
        CHECK(8 == source.position());

        s = source.read_buffer();
        CHECK(source.eof());
        CHECK(1 == s.size());
        CHECK(std::equal(s.begin(), s.end(), data.begin()+8));
        CHECK(9 == source.position());
    }
}
