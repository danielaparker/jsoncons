// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <map>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(json_literal_operator_tests)

BOOST_AUTO_TEST_CASE(json_literal_operator_test1)
{
    json j = R"(
{
    "StartDate" : "2017-03-01",
    "MaturityDate" : "2020-12-30",
    "Currency" : "USD",
    "DiscountCurve" : "USD-LIBOR",
    "FixedRate" : 0.01,
    "PayFrequency" : "6M",
    "DayCountBasis" : "ACT/360",
    "Notional" : 1000000          
}
)"_json;

    BOOST_CHECK(j["Currency"] == "USD");

}

BOOST_AUTO_TEST_CASE(ojson_literal_operator_test1)
{
    ojson j = R"(
{
    "StartDate" : "2017-03-01",
    "MaturityDate" : "2020-12-30",
    "Currency" : "USD",
    "DiscountCurve" : "USD-LIBOR",
    "FixedRate" : 0.01,
    "PayFrequency" : "6M",
    "DayCountBasis" : "ACT/360",
    "Notional" : 1000000          
}
)"_ojson;

    BOOST_CHECK(j["Currency"] == "USD");

}

BOOST_AUTO_TEST_CASE(json_literal_operator_test2)
{
    wjson j = LR"(
{
    "StartDate" : "2017-03-01",
    "MaturityDate" : "2020-12-30",
    "Currency" : "USD",
    "DiscountCurve" : "USD-LIBOR",
    "FixedRate" : 0.01,
    "PayFrequency" : "6M",
    "DayCountBasis" : "ACT/360",
    "Notional" : 1000000          
}
)"_json;

    BOOST_CHECK(j[L"Currency"] == L"USD");

}

BOOST_AUTO_TEST_CASE(ojson_literal_operator_test2)
{
    owjson j = LR"(
{
    "StartDate" : "2017-03-01",
    "MaturityDate" : "2020-12-30",
    "Currency" : "USD",
    "DiscountCurve" : "USD-LIBOR",
    "FixedRate" : 0.01,
    "PayFrequency" : "6M",
    "DayCountBasis" : "ACT/360",
    "Notional" : 1000000          
}
)"_ojson;

    BOOST_CHECK(j[L"Currency"] == L"USD");

}

BOOST_AUTO_TEST_SUITE_END()

