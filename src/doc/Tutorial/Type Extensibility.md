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
        template <typename Alloc>
        class json_type_traits<char,Alloc,boost::gregorian::date>
        {
        public:
            bool is(const basic_json<char,Alloc>& val) const
            {
                if (!val.is<std::string>())
                {
                    return false;
                }
                std::string s = val.as<std::string>();
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
            boost::gregorian::date as(const basic_json<char,Alloc>& val) const
            {
                std::string s = val.as<std::string>();
                return boost::gregorian::from_simple_string(s);
            }
            void assign(basic_json<char,Alloc>& self, boost::gregorian::date val)
            {
                std::string s(to_iso_extended_string(val));
                self = s;
            }
        };
    };

