// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include "jsoncons/json.hpp"
#include "jsoncons/json_serializer.hpp"
#include "my_custom_data.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include "boost/date_time/gregorian/gregorian.hpp"

using jsoncons::json_serializer;
using jsoncons::output_format;
using jsoncons::json;
using jsoncons::wjson;
using jsoncons::basic_json_reader;
using std::string;
using boost::numeric::ublas::matrix;

namespace jsoncons
{
    template <typename Allocator>
    class value_adapter<char,Allocator,boost::gregorian::date>
    {
    public:
        bool is(const basic_json<char,Allocator>& val) const
        {
            if (!val.is_string())
            {
                return false;
            }
            std::string s = val.as_string();
            try
            {
                boost::gregorian::date_from_iso_string(s);
                return true;
            }
            catch (...)
            {
                return false;
            }
        }
        boost::gregorian::date as(const basic_json<char,Allocator>& val) const
        {
            std::string s = val.as_string();
            return boost::gregorian::from_simple_string(s);
        }
        void assign(basic_json<char,Allocator>& self, boost::gregorian::date val)
        {
            std::string s(to_iso_extended_string(val));
            self.assign_string(s);
        }
    };
};

BOOST_AUTO_TEST_CASE(test_add_extensibility)
{
    json a(json::an_array);
    a.add(boost::gregorian::date(2013,10,14));
    auto d = a[0].as<boost::gregorian::date>();
    BOOST_CHECK_EQUAL(boost::gregorian::date(2013,10,14),d);

    json o;
    o["ObservationDates"] = std::move(a);
    o["ObservationDates"].add(boost::gregorian::date(2013,10,21));
    d = o["ObservationDates"][0].as<boost::gregorian::date>();
    auto d2 = o["ObservationDates"][1].as<boost::gregorian::date>();

    BOOST_CHECK_EQUAL(boost::gregorian::date(2013,10,14),d);
    BOOST_CHECK_EQUAL(boost::gregorian::date(2013,10,21),d2);
}

BOOST_AUTO_TEST_CASE(test_set_extensibility)
{
    json o;
    boost::gregorian::date d(boost::gregorian::day_clock::local_day());
    o.set("today",d);
    boost::gregorian::date val = o["today"].as<boost::gregorian::date>();
    BOOST_CHECK_EQUAL(d,val);
}

BOOST_AUTO_TEST_CASE(test_assignment_extensibility)
{
    json o;
    boost::gregorian::date d(boost::gregorian::day_clock::local_day());
    o["today"] = d;
    boost::gregorian::date val = o["today"].as<boost::gregorian::date>();
    BOOST_CHECK_EQUAL(d,val);
}

BOOST_AUTO_TEST_CASE(test_example)
{
        using jsoncons::json;
        using boost::gregorian::date;

        json deal;
        deal["Maturity"] = date(2014,10,14);

        json observation_dates(json::an_array);
        observation_dates.add(date(2014,2,14));
        observation_dates.add(date(2014,2,21));

		deal["ObservationDates"] = std::move(observation_dates);

        date maturity = deal["Maturity"].as<date>();
        std::cout << "Maturity: " << maturity << std::endl << std::endl;

        std::cout << "Observation dates: " << std::endl << std::endl;
        json::array_iterator it = deal["ObservationDates"].begin_elements();
        json::array_iterator end = deal["ObservationDates"].end_elements();

        while (it != end)
        {
            std::cout << *it << std::endl;
			++it;
        }
        std::cout << std::endl;

        std::cout << pretty_print(deal) << std::endl;
}

