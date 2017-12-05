### jsoncons::csv::decode_csv

Decodes a csv string or file to a `json` value.

#### Header
```c++
#include <jsoncons_ext/csv/csv_reader.hpp>

template <class Json>
Json decode_csv(typename Json::string_view_type s); // (1)

template <class Json>
Json decode_csv(typename Json::string_view_type s, 
                const basic_csv_parameters<typename Json::char_type>& params); // (2)

template <class Json>
Json decode_csv(std::basic_istream<typename Json::char_type>& is); // (3)

template <class Json>
Json decode_csv(std::basic_istream<typename Json::char_type>& is, 
                const basic_csv_parameters<typename Json::char_type>& params); // (4)
```

(1) Decodes csv string to json value using default [parameters](csv_parameters)

(2) Decodes csv string to json value using specified [parameters](csv_parameters)

(3) Decodes csv input stream to json value using default [parameters](csv_parameters)

(4) Decodes csv input stream to json value using specified [parameters](csv_parameters)

#### Return value

Returns a `Json` value

#### Exceptions

Throws [parse_error](parse_error.md) if parsing fails.

### Examples

#### Decode a json value from a string

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/csv/csv_reader.hpp>

using namespace jsoncons;
using namespace jsoncons::csv;

int main()
{
    std::string s = R"(employee-no,employee-name,dept,salary
00000001,\"Smith,Matthew\",sales,150000.00
00000002,\"Brown,Sarah\",sales,89000.00
)";

    csv_parameters params;
    params.assume_header(true)
          .column_types("string,string,string,float");
    json j = decode_csv<json>(s,params);

    std::cout << pretty_print(j) << std::endl;
}
```
Output:
```json
[
    {
        "dept": "sales",
        "employee-name": "Smith,Matthew\\",
        "employee-no": "00000001",
        "salary": 150000.0
    },
    {
        "dept": "sales",
        "employee-name": "Brown,Sarah\\",
        "employee-no": "00000002",
        "salary": 89000.0
    }
]
```

#### Decode a json value from a stream

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/csv/csv_reader.hpp>

using namespace jsoncons;
using namespace jsoncons::csv;

int main()
{
    const std::string bond_yields = R"(Date,1Y,2Y,3Y,5Y
2017-01-09,0.0062,0.0075,0.0083,0.011
2017-01-08,0.0063,0.0076,0.0084,0.0112
2017-01-08,0.0063,0.0076,0.0084,0.0112
)";

    csv_parameters params;
    params.assume_header(true)
          .column_types("string,float,float,float,float");

    std::istringstream is(bond_yields);

    ojson j = decode_csv<ojson>(is,params);

    std::cout << pretty_print(j) << std::endl;
}
```
Output:
```json
[
    {
        "Date": "2017-01-09",
        "1Y": 0.0062,
        "2Y": 0.0075,
        "3Y": 0.0083,
        "5Y": 0.011
    },
    {
        "Date": "2017-01-08",
        "1Y": 0.0063,
        "2Y": 0.0076,
        "3Y": 0.0084,
        "5Y": 0.0112
    },
    {
        "Date": "2017-01-08",
        "1Y": 0.0063,
        "2Y": 0.0076,
        "3Y": 0.0084,
        "5Y": 0.0112
    }
]
```

