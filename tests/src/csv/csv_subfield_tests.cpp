// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
//#include <jsoncons_ext/csv/csv_parameters.hpp>
#include <jsoncons_ext/csv/csv_reader.hpp>
#include <jsoncons_ext/csv/csv_serializer.hpp>
#include <jsoncons/json_reader.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <iostream>

using namespace jsoncons;
using namespace jsoncons::csv;

BOOST_AUTO_TEST_SUITE(csv_subfield_tests)
/*
BOOST_AUTO_TEST_CASE(test_n_objects)
{
    const std::string s = R"(calculationPeriodCenters,paymentCenters,resetCenters
NY;LON,TOR,LON
NY,LON,TOR;LON
"NY";"LON","TOR","LON"
"NY","LON","TOR";"LON"
)";
    csv_parameters params;
    params.assume_header(true)
          .subfield_delimiter(';');

    try
    {
        json j = decode_csv<json>(s,params);

        std::cout << "\nserialize" << std::endl;
        std::cout << pretty_print(j) << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}
*/
BOOST_AUTO_TEST_CASE(test_m_columns)
{
    const std::string s = R"(calculationPeriodCenters,paymentCenters,resetCenters
NY;LON,TOR,LON
NY,LON,TOR;LON
"NY";"LON","TOR","LON"
"NY","LON","TOR";"LON"
)";
    csv_parameters params;
    params.assume_header(true)
          .mapping(mapping_type::m_columns)
          .subfield_delimiter(';');

    try
    {
        json j = decode_csv<json>(s,params);

        std::cout << "\nserialize" << std::endl;
        std::cout << pretty_print(j) << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_SUITE_END()

