// Copyright 2013 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h"
#endif
#include <jsoncons/bignum.hpp>
#include <jsoncons/json.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <catch/catch.hpp>

using namespace jsoncons;

TEST_CASE("test_positive_bignum")
{
    std::string expected = "18446744073709551616";
    std::vector<uint8_t> v = {1,0,0,0,0,0,0,0,0};
    bignum x = bignum::from_bytes_be(sign_t::plus, v.data(),v.size());

    std::string sx;
    x.dump(sx);
    CHECK(sx == expected);

    bignum y(x);
    std::string sy;
    y.dump(sy);
    CHECK(sy == expected);

    bignum z;
    z = x;
    std::string sz;
    y.dump(sz);
    CHECK(sz == expected);

    SECTION("dump_hex_string")
    {
        std::string exp = "10000000000000000";
        std::string s;
        x.dump_hex_string(s);
        CHECK(s == exp);
    }

}

TEST_CASE("bignums are equal")
{
    std::string s = "18446744073709551616";
    bignum x = bignum::from_string(s);
    bignum y = bignum::from_string(s);

    bool test = x == y;
    CHECK(test);
}

TEST_CASE("test_negative_bignum")
{
    std::string expected = "-18446744073709551617";
    std::vector<uint8_t> b = {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    bignum x = bignum::from_bytes_be(sign_t::plus, b.data(),b.size());
    x = -1 - x;

    std::string sx;
    x.dump(sx);
    CHECK(sx == expected);

    bignum y(x);
    std::string sy;
    y.dump(sy);
    CHECK(sy == expected);

    bignum z;
    z = x;
    std::string sz;
    y.dump(sz);
    CHECK(sz == expected);

    int signum;
    std::vector<uint8_t> v;
    x.dump(signum, v);

    REQUIRE(v.size() == b.size());
    for (std::size_t i = 0; i < v.size(); ++i)
    {
        REQUIRE(v[i] == b[i]);
    }

    SECTION("dump_hex_string")
    {
        std::string exp = "-10000000000000001";
        std::string s;
        x.dump_hex_string(s);
        //std::cout << "bignum: " << expected << ", s: " << s << "\n";
        CHECK(s == exp);
    }
}

TEST_CASE("test_longlong")
{
    long long n = (std::numeric_limits<long long>::max)();

    bignum val(n);

    //std::cout << "long long " << n << " == " << val << std::endl;
    //std::cout << val.to_string(16) << std::endl;
}

TEST_CASE("test_bignum2")
{
    std::string v = "10000000000000000";
    bignum val = bignum::from_string(v.data());

    //std::cout << val << std::endl;
}

TEST_CASE("test_logical_operations")
{
    bignum x = bignum::from_string( "888888888888888888" );
    bignum y = bignum::from_string( "888888888888888888" );

    bignum z = x & y;

    bool test = z == x;
    CHECK(test);
}

TEST_CASE("test_addition")
{
    bignum x = bignum::from_string( "4444444444444444444444444444444" );
    bignum y = bignum::from_string( "4444444444444444444444444444444" );
    bignum a = bignum::from_string( "8888888888888888888888888888888" );

    bignum z = x + y;
    bool test = z == a;
    CHECK(test);
}

TEST_CASE("test_multiplication")
{
    bignum x = bignum::from_string( "4444444444444444444444444444444" );
    bignum a = bignum::from_string( "8888888888888888888888888888888" );

    bignum z = 2*x;
    bool test = z == a;
    CHECK(test);

    z = x*2;

    test = z == a;
    CHECK(test);
}

TEST_CASE("test_conversion_0")
{
    bignum x;

    json j(x);

    bignum y = j.as<bignum>();
    CHECK(bool(x == y));

    std::string s;
    y.dump(s);

    CHECK(s == "0");
}

TEST_CASE("test_traits1")
{
    std::vector<uint8_t> data = {0x01,0x00};
    bignum x = bignum::from_bytes_be(sign_t::plus, data.data(), data.size());

    json j(x);

    bignum y = j.as<bignum>();
    CHECK(bool(x == y));

    std::string s;
    y.dump(s);

    CHECK(s == "256");
}

TEST_CASE("test_traits2")
{
    std::vector<uint8_t> data = {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    bignum x = bignum::from_bytes_be(sign_t::plus, data.data(), data.size());

    json j(x);

    bignum y = j.as<bignum>();
    CHECK(bool(x == y));

    std::string s;
    y.dump(s);

    CHECK(s == "18446744073709551616");
}

TEST_CASE("test_traits3")
{
    std::vector<uint8_t> data = {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    bignum x = bignum::from_bytes_be(sign_t::plus, data.data(), data.size());

    x = -1 - x;

    int signum;
    std::vector<uint8_t> v;
    x.dump(signum,v);

    REQUIRE(signum == -1);
    //for (auto c : v)
    //{
    //    //std::cout << std::hex << (int)c;
    //}
    //std::cout << std::endl;

    json j(x);

    bignum y = j.as<bignum>();
    CHECK(bool(x == y));

    std::string s;
    y.dump(s);

    CHECK(s == "-18446744073709551617");
}

TEST_CASE("test shift left")
{
    SECTION("n << 1")
    {
        bignum n = bignum::from_string("1");
        bignum x = n << 1;
        std::string s;
        x.dump(s);
        CHECK(s == "2");
    }
    SECTION("n << 100")
    {
        bignum n(1);
        bignum x = n << 100;
        std::string s;
        x.dump(s);
        CHECK(s == "1267650600228229401496703205376");
    }
    SECTION("n << 100, += 1")
    {
        bignum n(1);
        bignum x = n << 100;
        x += 1;
        std::string s;
        x.dump(s);
        CHECK(s == "1267650600228229401496703205377");
    }
}

TEST_CASE("times 10")
{
    SECTION("1")
    {
        bignum n = bignum::from_string("1234");
        bignum m = n * 10;
        std::string s;
        m.dump(s);
        CHECK(s == "12340");
    }
    SECTION("31")
    {
        std::string expected("1234");
        bignum n = bignum::from_string(expected);

        for (std::size_t i = 0; i < 31; ++i)
        {
            n *= (uint64_t)10;
            expected.push_back('0');
        }
        std::string s;
        n.dump(s);
        CHECK(s == expected);
        //std::cout << "x31: " << s << "\n";
    }
    SECTION("32")
    {
        std::string expected("1234");
        bignum n = bignum::from_string(expected);
        for (std::size_t i = 0; i < 32; ++i)
        {
            n *= (uint64_t)10;
            expected.push_back('0');
        }
        std::string s;
        n.dump(s);
        CHECK(s == expected);
        //std::cout << "x31: " << s << "\n";
    }
}

TEST_CASE("bignum div")
{
#if defined(_MSC_VER) && _MSC_VER >= 1910
    SECTION("bignum")
    {
        bignum big_pos = bignum::from_string("18364494661702398480");
        bignum small_pos = bignum::from_string("65535");
        bignum res_pos = bignum::from_string("280224226164681");
        bignum big_neg = -big_pos;
        bignum small_neg = -small_pos;
        bignum res_neg = -res_pos;

        CHECK((big_neg / big_neg) == bignum(1));
        CHECK((big_neg / small_neg) == res_pos);
        CHECK((big_neg / small_pos) == res_neg);
        CHECK((big_neg / big_pos) == bignum(-1));

        CHECK((small_neg / big_neg) == bignum(0));
        CHECK((small_neg / small_neg) == bignum(1));
        CHECK((small_neg / small_pos) == bignum(-1));
        CHECK((small_neg / big_pos) == bignum(0));

        CHECK((small_pos / big_neg) == bignum(0));
        CHECK((small_pos / small_neg) == bignum(-1));
        CHECK((small_pos / small_pos) == bignum(1));
        CHECK((small_pos / big_pos) == bignum(0));

        CHECK((big_pos / big_neg) == bignum(-1));
        CHECK((big_pos / small_neg) == res_neg);
        CHECK((big_pos / small_pos) == res_pos);
        CHECK((big_pos / big_pos) == bignum(1));
    }
#endif
}

// Source: https://github.com/justmoon/node-bignum/blob/master/test/big.js

TEST_CASE("bignum operations")
{
    SECTION("plus")
    {
        bignum a = bignum::from_string("20178175244496647895629245678926563358862835685868092718528786189298896755892724096350318132354654969715294305656279188466948605121492948268400884893722767401972695174353441");
        bignum b = bignum::from_string("93976986297275963857794534313022836860642008364607162222395304627737845003599751108876721426146679370149373711095582235633733294240624814097369771481147215472578762824607080");
        bignum c = a + b;
        bignum expected = bignum::from_string("114155161541772611753423779991949400219504844050475254940924090817036741759492475205227039558501334339864668016751861424100681899362117762365770656374869982874551457998960521");

        CHECK((c == expected));
    }

    SECTION("minus")
    {
        bignum a = bignum::from_string("63584976221895260406245934266037944699776129516216688813405106853198139417759498415735161100030933326522675347686646219695144553808051168706779408804756208386011014197185296");
        bignum b = bignum::from_string("75761734353628069683913529566109295493116360791340046058510920764479664838827482335858563500856417188227416490721063436557647698896399869016678013515043471880323279258685478");
        bignum c = a - b;
        bignum expected = bignum::from_string("-12176758131732809277667595300071350793340231275123357245105813911281525421067983920123402400825483861704741143034417216862503145088348700309898604710287263494312265061500182");

        CHECK((c == expected));
    }

    SECTION("mult")
    {
        bignum a = bignum::from_string("43359329001059048967113581928625959342654930666632400867978208429224464941890190751598229305718587280094852374898291386268967561418738958337632249177044975686477011571044266");
        bignum b = bignum::from_string("1277902648419017187919156692641295109476255233737630537760832794503886212911067061184379695097643279217271150419129022856601771338794256383410400076210073482253089544155377");
        bignum c = a * b;
        bignum expected = bignum::from_string("55409001364124857587521411422210474638575227552776047085010157327559891765943209923363557763419730972781537530948429788352886919273214132899346769031695550850320602049507618052164677667378189154076988316301237199538599598044906690917691500474146296751848053320011822980888915807952984822080201739642211593661864443811046346990267512628848918282");

        CHECK((c == expected));
    }

    SECTION("div")
    {
        bignum a = bignum::from_string("43359329001059048967113581928625959342654930666632400867978208429224464941890190751598229305718587280094852374898291386268967561418738958337632249177044975686477011571044266");
        bignum b = bignum::from_string("1277902648419017187919156692641295109476255233737630537760832794503886212911067061184379695097643279217271150419129022856601771338794256383410400076210073482253089544155377");
        bignum c = a / b;
        bignum expected = bignum::from_string("33");

        CHECK((c == expected));
    }
}

