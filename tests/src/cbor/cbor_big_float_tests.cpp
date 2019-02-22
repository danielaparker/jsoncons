// Copyright 2019 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h"
#endif
#include <jsoncons/json.hpp>
#include <jsoncons/bignum.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <limits>
#include <cmath>
#include <catch/catch.hpp>

using namespace jsoncons;
using namespace jsoncons::cbor;

TEST_CASE("cbor bigfloat tests")
{
    std::vector<uint8_t> v = {0xc5, // Tag 5 
                              0x82, // Array of length 2
                              0x21, // -2 
                              0x19, 0x6a, 0xb3 // 27315 
                             };
    std::error_code ec;

    jsoncons::json_decoder<json> decoder;
    cbor_buffer_reader parser(v, decoder);
    parser.read(ec);

    json j = decoder.get_result();

    SECTION("Test 1")
    {
        bignum integer = 3;
        int64_t exp = -1;
        int64_t newExp;

        bignum five(5);
        if (exp > 0)
        {
            newExp = static_cast<int64_t>(std::floor(exp*std::log(2)/std::log(10)));
            bignum fivePower = power(five,(unsigned)newExp);
            uint64_t binShift = exp - newExp;
            integer = ((integer) << (unsigned)binShift)/fivePower;
        }
        else
        {
            newExp = static_cast<int64_t>(std::ceil(-exp*std::log(2)/std::log(10)));
            bignum fivePower = power(five,(unsigned)newExp);
            uint64_t binShift = -exp - newExp;
            integer = (integer*fivePower) >> (unsigned)binShift;
        }

        std::string s;
        std::string result;
        integer.dump(s);
        jsoncons::detail::prettify_string(s.c_str(),s.length(),-newExp,6,100, result);

        std::cout << "result: " << result << "\n"; 
    }

    SECTION("Test 2")
    {
        bignum integer = 2;
        int64_t exp = 3;
        int64_t newExp;

        bignum five(5);
        if (exp > 0)
        {
            newExp = static_cast<int64_t>(std::floor(exp*std::log(2)/std::log(10)));
            bignum fivePower = power(five,newExp);
            uint64_t binShift = exp - newExp;
            integer = ((integer) << (unsigned)binShift)/fivePower;
        }
        else
        {
            newExp = static_cast<int64_t>(std::ceil(-exp*std::log(2)/std::log(10)));
            bignum fivePower = power(five,newExp);
            uint64_t binShift = -exp - newExp;
            integer = (integer*fivePower) >> (unsigned)binShift;
        }

        std::string s;
        std::string result;
        integer.dump(s);
        jsoncons::detail::prettify_string(s.c_str(),s.length(),(int)-newExp,6,(std::numeric_limits<int>::max)(), result);

        std::cout << "result: " << result << "\n"; 
    }
} 

