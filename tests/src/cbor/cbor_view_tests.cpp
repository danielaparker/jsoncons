// Copyright 2016 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <limits>

using namespace jsoncons;
using namespace jsoncons::cbor;

BOOST_AUTO_TEST_SUITE(cbor_view_tests)
/*
BOOST_AUTO_TEST_CASE(cbor_view_test)
{
    ojson j1 = ojson::parse(R"(
    {
       "application": "hiking",
       "reputons": [
       {
           "rater": "HikingAsylum.example.com",
           "assertion": "is-good",
           "rated": "sk",
           "rating": 0.90
         }
       ]
    }
    )");
 
    std::vector<uint8_t> c;
    encode_cbor(j1, c);

    cbor_view v(c); 
    BOOST_CHECK(v.is_object());
    BOOST_CHECK(!v.is_array());

    cbor_view reputons = v.at("reputons");

    cbor_view reputons_0 = reputons.at(0);

    cbor_view reputons_0_rated = reputons_0.at("rated");
}
*/
BOOST_AUTO_TEST_CASE(jsonpointer_test)
{
    ojson j = ojson::parse(R"(
    {
       "application": "hiking",
       "reputons": [
       {
           "rater": "HikingAsylum.example.com",
           "assertion": "is-good",
           "rated": "sk",
           "rating": 0.90
         }
       ]
    }
    )");

    std::vector<uint8_t> buffer;
    encode_cbor(j, buffer);

    cbor_view v(buffer); 

    std::error_code ec;
    cbor_view application = jsonpointer::get(v, "/application", ec);
    BOOST_CHECK(!ec);

    json j2 = decode_cbor<json>(application);
    std::cout << pretty_print(j2) << std::endl;

    cbor_view reputons_0_rated = jsonpointer::get(v, "/reputons", ec);
    BOOST_CHECK(!ec);

    json j3 = decode_cbor<json>(reputons_0_rated);
    std::cout << pretty_print(j3) << std::endl;

}

BOOST_AUTO_TEST_SUITE_END()

