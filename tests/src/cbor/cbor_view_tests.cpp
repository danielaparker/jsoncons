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

    auto c = encode_cbor(j1);

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

    auto buffer = encode_cbor(j);

    cbor_view v(buffer); 

    cbor_view application;
    jsonpointer::jsonpointer_errc ec;

    std::tie(application,ec) = jsonpointer::get(v,"/application");
    BOOST_CHECK_EQUAL(ec,jsonpointer::jsonpointer_errc());

    json j2 = decode_cbor<json>(application);
    std::cout << pretty_print(j2) << std::endl;

    cbor_view reputons_0_rated;
    std::tie(reputons_0_rated,ec) = jsonpointer::get(v,"/reputons");
    BOOST_CHECK_EQUAL(ec,jsonpointer::jsonpointer_errc());

    json j3 = decode_cbor<json>(reputons_0_rated);
    std::cout << pretty_print(j3) << std::endl;

}

BOOST_AUTO_TEST_SUITE_END()

