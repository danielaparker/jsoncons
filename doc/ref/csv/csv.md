### csv extension

The csv extension implements decode from and encode to the [CSV format](https://www.rfc-editor.org/rfc/rfc4180.txt)
You can either parse into or serialize from a variant-like structure, [basic_json](../basic_json.md), or your own
data structures, using [json_type_traits](../json_type_traits.md).

[decode_csv](decode_csv.md)

[encode_csv](encode_csv.md)

[csv_options](csv_options.md)

[csv_reader](csv_reader.md)

[basic_csv_encoder](basic_csv_encoder.md)

### Working with CSV data

For the examples below you need to include some header files and construct a string of CSV data:

```c++
#include <iomanip>
#include <iostream>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/csv/csv.hpp>

    const std::string data = R"(index_id,observation_date,rate
EUR_LIBOR_06M,2015-10-23,0.0000214
EUR_LIBOR_06M,2015-10-26,0.0000143
EUR_LIBOR_06M,2015-10-27,0.0000001
)";
```

jsoncons allows you to work with the CSV data similarly to JSON data:

- As a variant-like structure, [basic_json](../basic_json.md) 

- As a strongly typed C++ data structure

- As a stream of parse events


#### As a variant-like structure

```c++
int main()
{
    csv::csv_options options;
    options.assume_header(true);

    // Parse the CSV data into an ojson value
    ojson j = csv::decode_csv<ojson>(data, options);

    // Pretty print
    std::cout << "(1)\n" << pretty_print(j) << "\n\n";

    // Iterate over the rows
    std::cout << "(2)\n";
    for (const auto& row : j.array_range())
    {
        // Access rated as string and rating as double
        std::cout << row["index_id"].as<std::string>() << ", " 
                  << row["observation_date"].as<std::string>() << ", " 
                  << row["rate"].as<double>() << "\n";
    }
}
```
Output:
```
(1)
[
    {
        "index_id": "EUR_LIBOR_06M",
        "observation_date": "2015-10-23",
        "rate": 2.14e-05
    },
    {
        "index_id": "EUR_LIBOR_06M",
        "observation_date": "2015-10-26",
        "rate": 1.43e-05
    },
    {
        "index_id": "EUR_LIBOR_06M",
        "observation_date": "2015-10-27",
        "rate": 1e-07
    }
]

(2)
EUR_LIBOR_06M, 2015-10-23, 2.1e-05
EUR_LIBOR_06M, 2015-10-26, 1.4e-05
EUR_LIBOR_06M, 2015-10-27, 1e-07
```

#### As a strongly typed C++ data structure

```c++
int main()
{
    csv::csv_options options;
    options.assume_header(true);

    // Decode the CSV data into a c++ structure
    std::vector<ns::fixing> v = csv::decode_csv<std::vector<ns::fixing>>(data, options);

    // Iterate over values
    std::cout << std::fixed << std::setprecision(7);
    std::cout << "(1)\n";
    for (const auto& item : v)
    {
        std::cout << item.index_id() << ", " << item.observation_date() << ", " << item.rate() << "\n";
    }

    // Encode the c++ structure into CSV data
    std::string s;
    csv::encode_csv(v, s);
    std::cout << "(2)\n";
    std::cout << s << "\n";
}
```
Output:
```
(1)
EUR_LIBOR_06M, 2015-10-23, 0.0000214
EUR_LIBOR_06M, 2015-10-26, 0.0000143
EUR_LIBOR_06M, 2015-10-27, 0.0000001
(2)
index_id,observation_date,rate
EUR_LIBOR_06M,2015-10-23,2.14e-05
EUR_LIBOR_06M,2015-10-26,1.43e-05
EUR_LIBOR_06M,2015-10-27,1e-07
```

#### As a stream of parse events

```c++
int main()
{
    csv::csv_options options;
    options.assume_header(true);
    csv::csv_cursor cursor(data, options);

    for (; !cursor.done(); cursor.next())
    {
        const auto& event = cursor.current();
        switch (event.event_type())
        {
            case staj_event_type::begin_array:
                std::cout << event.event_type() << " " << "\n";
                break;
            case staj_event_type::end_array:
                std::cout << event.event_type() << " " << "\n";
                break;
            case staj_event_type::begin_object:
                std::cout << event.event_type() << " " << "\n";
                break;
            case staj_event_type::end_object:
                std::cout << event.event_type() << " " << "\n";
                break;
            case staj_event_type::name:
                // Or std::string_view, if supported
                std::cout << event.event_type() << ": " << event.get<jsoncons::string_view>() << "\n";
                break;
            case staj_event_type::string_value:
                // Or std::string_view, if supported
                std::cout << event.event_type() << ": " << event.get<jsoncons::string_view>() << "\n";
                break;
            case staj_event_type::null_value:
                std::cout << event.event_type() << "\n";
                break;
            case staj_event_type::bool_value:
                std::cout << event.event_type() << ": " << std::boolalpha << event.get<bool>() << "\n";
                break;
            case staj_event_type::int64_value:
                std::cout << event.event_type() << ": " << event.get<int64_t>() << "\n";
                break;
            case staj_event_type::uint64_value:
                std::cout << event.event_type() << ": " << event.get<uint64_t>() << "\n";
                break;
            case staj_event_type::double_value:
                std::cout << event.event_type() << ": " << event.get<double>() << "\n";
                break;
            default:
                std::cout << "Unhandled event type: " << event.event_type() << " " << "\n";;
                break;
        }
    }
}
```
Output:
```
begin_array
begin_object
name: index_id
string_value: EUR_LIBOR_06M
name: observation_date
string_value: 2015-10-23
name: rate
double_value: 2.1e-05
end_object
begin_object
name: index_id
string_value: EUR_LIBOR_06M
name: observation_date
string_value: 2015-10-26
name: rate
double_value: 1.4e-05
end_object
begin_object
name: index_id
string_value: EUR_LIBOR_06M
name: observation_date
string_value: 2015-10-27
name: rate
double_value: 1e-07
end_object
end_array
```

You can use a [staj_array_iterator](../staj_array_iterator.md) to group the CSV parse events into [basic_json](../basic_json.md) records:
```c++
int main()
{
    csv::csv_options options;
    options.assume_header(true);
    csv::csv_cursor cursor(data, options);

    staj_array_iterator<ojson> it(cursor);
    staj_array_iterator<ojson> end; // default-constructed iterator is end iterator

    while (it != end)
    {
        std::cout << pretty_print(*it) << "\n";
        ++it;
    }
}
```
Output:
```
{
    "index_id": "EUR_LIBOR_06M",
    "observation_date": "2015-10-23",
    "rate": 2.14e-05
}
{
    "index_id": "EUR_LIBOR_06M",
    "observation_date": "2015-10-26",
    "rate": 1.43e-05
}
{
    "index_id": "EUR_LIBOR_06M",
    "observation_date": "2015-10-27",
    "rate": 1e-07
}
```

Or into strongly typed records:
```c++
int main()
{
    typedef std::tuple<std::string,std::string,double> record_type;

    csv::csv_options options;
    options.assume_header(true);
    csv::csv_cursor cursor(data, options);

    staj_array_iterator<ojson,record_type> it(cursor);

    std::cout << std::fixed << std::setprecision(7);
    for (const auto& record : it) // range-based for loop 
    {
        std::cout << std::get<0>(record) << ", " << std::get<1>(record) << ", " << std::get<2>(record) << "\n";
    }
}
```
Output
```
EUR_LIBOR_06M, 2015-10-23, 0.0000214
EUR_LIBOR_06M, 2015-10-26, 0.0000143
EUR_LIBOR_06M, 2015-10-27, 0.0000001
```
