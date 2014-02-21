If you want to use the json methods `is<T>`, `as<T>`, `add`, `set` and `operator=` to access or modify with a new type, you need to show json how to interact with that type, by extending `value_adapter` in the `jsoncons` namespace.

Below is an example for extending `value_adapter` to access and modify with `boost::gregorian::date` values, while internally storing the values as strings.

First, you need to specialize `value_adapter` in namespace `jsoncons`

    #include "jsoncons/json.hpp"
    #include "boost/date_time/gregorian/gregorian.hpp"

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

You can now access and modify json members and elements with boost dates.

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
