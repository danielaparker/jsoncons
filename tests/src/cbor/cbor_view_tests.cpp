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

    json jv = decode_cbor<json>(v);
    std::cout << pretty_print(jv) << std::endl;

    cbor_view reputons = v.at("reputons");
    BOOST_CHECK(reputons.is_array());

    cbor_view reputons_0 = reputons.at(0);

    cbor_view reputons_0_rated = reputons_0.at("rated");

    cbor_view rating = reputons_0.at("rating");
    BOOST_CHECK(rating.as_double() == 0.90);

    for (const auto& member : v.object_range())
    {
        const auto& key = member.key();
        const auto& val = member.value();
        json jval = decode_cbor<json>(val);

        std::cout << key << ": " << jval << std::endl;
    }
    std::cout << std::endl;

    for (auto element : reputons.array_range())
    {
        json j = decode_cbor<json>(element);
        std::cout << j << std::endl;
    }
    std::cout << std::endl;
}

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

    std::error_code ec;
    cbor_view application = jsonpointer::get(cbor_view(buffer), "/application", ec);
    BOOST_CHECK(!ec);

    json j2 = decode_cbor<json>(application);
    std::cout << pretty_print(j2) << std::endl;

    cbor_view reputons_0_rated = jsonpointer::get(cbor_view(buffer), "/reputons", ec);
    BOOST_CHECK(!ec);

    json j3 = decode_cbor<json>(reputons_0_rated);
    std::cout << pretty_print(j3) << std::endl;

}

const json store = json::parse(R"(
{
    "store": {
        "book": [
            {
                "category": "reference",
                "author": "Nigel Rees",
                "title": "Sayings of the Century",
                "price": 8.95
            },
            {
                "category": "fiction",
                "author": "Evelyn Waugh",
                "title": "Sword of Honour",
                "price": 12.99
            },
            {
                "category": "fiction",
                "author": "Herman Melville",
                "title": "Moby Dick",
                "isbn": "0-553-21311-3",
                "price": 8.99
            },
            {
                "category": "fiction",
                "author": "J. R. R. Tolkien",
                "title": "The Lord of the Rings",
                "isbn": "0-395-19395-8",
                "price": 22.99
            }
        ],
        "bicycle": {
            "color": "red",
            "price": 19.95
        }
    }
}
)");

BOOST_AUTO_TEST_SUITE_END()

