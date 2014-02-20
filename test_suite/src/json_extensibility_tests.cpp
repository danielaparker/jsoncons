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
    boost::gregorian::date d(boost::gregorian::day_clock::local_day());
    a.add(d);
    boost::gregorian::date val = a[0].as<boost::gregorian::date>();
    BOOST_CHECK_EQUAL(d,val);
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

