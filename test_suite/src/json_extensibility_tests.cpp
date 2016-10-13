// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include "jsoncons/json.hpp"
#include "jsoncons/json_serializer.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include "boost/date_time/gregorian/gregorian.hpp"
#include <boost/numeric/ublas/matrix.hpp>

namespace jsoncons 
{
    template <class Json>
    struct json_type_traits<Json,boost::gregorian::date>
    {
        static bool is(const Json& val) JSONCONS_NOEXCEPT
        {
            if (!val.is_string())
            {
                return false;
            }
            std::string s = val.template as<std::string>();
            try
            {
                boost::gregorian::from_simple_string(s);
                return true;
            }
            catch (...)
            {
                return false;
            }
        }

        static boost::gregorian::date as(const Json& val)
        {
            std::string s = val.template as<std::string>();
            return boost::gregorian::from_simple_string(s);
        }

        static Json to_json(boost::gregorian::date val)
        {
            return Json::make_string(to_iso_extended_string(val));
        }

        static Json to_json(boost::gregorian::date val, typename Json::allocator_type allocator)
        {
            return Json::make_string(to_iso_extended_string(val),allocator);
        }
    };

    template <class Json,class T>
    struct json_type_traits<Json,boost::numeric::ublas::matrix<T>>
    {
        static bool is(const Json& val) JSONCONS_NOEXCEPT
        {
            if (!val.is_array())
            {
                return false;
            }
            if (val.size() > 0)
            {
                size_t n = val[0].size();
                for (const auto& a: val.elements())
                {
                    if (!(a.is_array() && a.size() == n))
                    {
                        return false;
                    }
                    for (auto x: a.elements())
                    {
                        if (!x.is<T>())
                        {
                            return false;
                        }
                    }
                }
            }
            return true;
        }

        static boost::numeric::ublas::matrix<T> as(const Json& val)
        {
            if (val.is_array() && val.size() > 0)
            {
                size_t m = val.size();
                size_t n = 0;
                for (const auto& a : val.elements())
                {
                    if (a.size() > n)
                    {
                        n = a.size();
                    }
                }

                boost::numeric::ublas::matrix<T> A(m,n,T());
                for (size_t i = 0; i < m; ++i)
                {
                    const auto& a = val[i];
                    for (size_t j = 0; j < a.size(); ++j)
                    {
                        A(i,j) = a[j].as<T>();
                    }
                }
                return A;
            }
            else
            {
                boost::numeric::ublas::matrix<T> A;
                return A;
            }
        }

        static Json to_json(const boost::numeric::ublas::matrix<T>& val)
        {
            Json a = Json::make_array<2>(val.size1(), val.size2(),T());
            for (size_t i = 0; i < val.size1(); ++i)
            {
                for (size_t j = 0; j < val.size1(); ++j)
                {
                    a[i][j] = val(i,j);
                }
            }
            return a;
        }
    };
}

using namespace jsoncons;
using boost::numeric::ublas::matrix;

BOOST_AUTO_TEST_SUITE(json_extensibility_test_suite)

BOOST_AUTO_TEST_CASE(test_add_extensibility)
{
    json a = json::make_array();
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

    json deal;
    deal["maturity"] = boost::gregorian::date(2015,1,1);
    json observation_dates = json::make_array();
    observation_dates.add(boost::gregorian::date(2013,10,21));
    observation_dates.add(boost::gregorian::date(2013,10,28));
    deal["observation_dates"] = std::move(observation_dates);
    std::cout << pretty_print(deal) << std::endl;
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

        json observation_dates = json::make_array();
        observation_dates.add(date(2014,2,14));
        observation_dates.add(date(2014,2,21));

        deal["ObservationDates"] = std::move(observation_dates);

        date maturity = deal["Maturity"].as<date>();
        std::cout << "Maturity: " << maturity << std::endl << std::endl;

        std::cout << "Observation dates: " << std::endl << std::endl;
        json::array_iterator it = deal["ObservationDates"].elements().begin();
        json::array_iterator end = deal["ObservationDates"].elements().end();

        while (it != end)
        {
            date d = it->as<date>();
            std::cout << d << std::endl;
            ++it;
        }
        std::cout << std::endl;

        std::cout << pretty_print(deal) << std::endl;
}

BOOST_AUTO_TEST_CASE(test_boost_matrix)
{
    matrix<double> A(2, 2);
    A(0, 0) = 1.1;
    A(0, 1) = 2.1;
    A(1, 0) = 3.1;
    A(1, 1) = 4.1;

    json a = A;

    std::cout << "(1) " << std::boolalpha << a.is<matrix<double>>() << "\n\n";

    std::cout << "(2) " << std::boolalpha << a.is<matrix<int>>() << "\n\n";

    std::cout << "(3) \n\n";

    std::cout << pretty_print(a) << "\n\n";

    BOOST_CHECK(a.is<matrix<double>>());
    BOOST_CHECK(!a.is<matrix<int>>());

    BOOST_CHECK_EQUAL(a[0][0].as<double>(),A(0,0));
    BOOST_CHECK_EQUAL(a[0][1].as<double>(),A(0,1));
    BOOST_CHECK_EQUAL(a[1][0].as<double>(),A(1,0));
    BOOST_CHECK_EQUAL(a[1][1].as<double>(),A(1,1));

    matrix<double> B = a.as<matrix<double>>();

    std::cout << "(4) \n\n";
    for (size_t i = 0; i < B.size1(); ++i)
    {
        for (size_t j = 0; j < B.size2(); ++j)
        {
            if (j > 0)
            {
                std::cout << ",";
            }
            std::cout << B(i, j);
        }
        std::cout << "\n";
    }
    std::cout << "\n\n";

    BOOST_CHECK_EQUAL(B.size1(),a.size());
    BOOST_CHECK_EQUAL(B.size2(),a[0].size());

    BOOST_CHECK_EQUAL(a[0][0].as<double>(),B(0,0));
    BOOST_CHECK_EQUAL(a[0][1].as<double>(),B(0,1));
    BOOST_CHECK_EQUAL(a[1][0].as<double>(),B(1,0));
    BOOST_CHECK_EQUAL(a[1][1].as<double>(),B(1,1));
}

BOOST_AUTO_TEST_SUITE_END()

