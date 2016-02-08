If you want to use the json methods `is<T>`, `as<T>`, `add`, `set` and `operator=` to access or modify with a new type, you need to show json how to interact with that type, by extending `json_type_traits` in the `jsoncons` namespace.

For example, by including the header file `jsoncons_ext/boost/type_extensions.hpp`, you can access and modify `json` values with `boost::gregorian` dates.

    #include "jsoncons/json.hpp"
    #include "jsoncons_ext/boost/type_extensions.hpp"

    namespace my
    {
        using jsoncons::json;
        using boost::gregorian::date;

        json deal;
        deal["Maturity"] = date(2014,10,14);

        json observation_dates = make_array();
        observation_dates.add(date(2014,2,14));
        observation_dates.add(date(2014,2,21));

        deal["ObservationDates"] = std::move(observation_dates);

        date maturity = deal["Maturity"].as<date>();
        std::cout << "Maturity: " << maturity << std::endl << std::endl;

        std::cout << "Observation dates: " << std::endl << std::endl;
        json::array_iterator it = deal["ObservationDates"].elements().begin();
        json::array_iterator end = deal["ObservationDates"].end_elements();

        while (it != end)
        {
            date d = it->as<date>();
            std::cout << d << std::endl;
            ++it;
        }
        std::cout << std::endl;

        std::cout << pretty_print(deal) << std::endl;
    }

The output is

    Maturity: 2014-Oct-14

    Observation dates:

    2014-Feb-14
    2014-Feb-21

    {
        "Maturity":"2014-10-14",
        "ObservationDates":
        ["2014-02-14","2014-02-21"]
    }

You can look in the header file `jsoncons_ext/boost/type_extensions.hpp`
to see how the specialization of `json_type_traits` that supports
the conversions works. In this implementation the `boost` date values are stored in the `json` values as strings.

namespace jsoncons 
{
    template <typename JsonT>
    class json_type_traits<JsonT,boost::gregorian::date>
    {
    public:
        static bool is(const JsonT& val) 
        {
            if (!val.is_string())
            {
                return false;
            }
            std::string s = val.template as<std::string>();
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

        static boost::gregorian::date as(const JsonT& val)
        {
            std::string s = val.template as<std::string>();
            return boost::gregorian::from_simple_string(s);
        }

        static void assign(JsonT& lhs, boost::gregorian::date val)
        {
            lhs = to_iso_extended_string(val);
        }
    };
}

