// Copyright 2020 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <jsoncons/json.hpp>
#include <random>
#include <common/test_utilities.hpp>

using namespace jsoncons;

TEST_CASE("json std::bitset tests")
{
    SECTION("test 1")
    {
        std::random_device rd;
        std::mt19937 gen(rd());

        auto rng32  = [&](){return random_binary_string(gen, 32);};
        auto rng65  = [&](){return random_binary_string(gen, 65);};
        auto rng256 = [&](){return random_binary_string(gen, 256);};
        auto rng512 = [&](){return random_binary_string(gen, 512);};

        std::bset<32> i_bs32_low(0);
        auto o_bs32_low = decode_json<std::bset<32>>(s32_low);
        CHECK(o_bs32_low == i_bs32_low);

        for (std::size_t i = 0; i < 100; ++i)
        {
            std::bset<32> i_bs32(rng32());
            std::bset<65> i_bs65(rng65());
            std::bset<256> i_bs256(rng256());
            std::bset<512> i_bs512(rng512());

            std::string s32;
            encode_json(i_bs32, s32);
            std::string s65;
            encode_json(i_bs65, s65);
            std::string s256;
            encode_json(i_bs256, s256);
            std::string s512;
            encode_json(i_bs512, s512);

            auto o_bs32 = decode_json<std::bset<32>>(s32);
            auto o_bs65 = decode_json<std::bset<65>>(s65);
            auto o_bs256 = decode_json<std::bset<256>>(s256);
            auto o_bs512 = decode_json<std::bset<512>>(s512);
            CHECK(o_bs32 == i_bs32);
            CHECK(o_bs65 == i_bs65);
            CHECK(o_bs256 == i_bs256);
            CHECK(o_bs512 == i_bs512);
        }
    }
}

