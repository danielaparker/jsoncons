// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

TEST_CASE("test_byte_string_serialization")
{
    const uint8_t bs[] = {'H','e','l','l','o'};
    json j(byte_string_view(bs,sizeof(bs)));

    std::ostringstream os;
    os << j;

    std::string expected; 
    expected.push_back('\"');
    encode_base64url(bs,sizeof(bs),expected);
    expected.push_back('\"');

    //std::cout << expected << " " << os.str() << std::endl;

    CHECK(expected == os.str());
}

