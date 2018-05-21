### jsoncons::csv::decode_csv

Reads a `json` value from a CSV string or input stream.

#### Header
```c++
#include <jsoncons_ext/csv/csv_reader.hpp>

template <class Json>
Json decode_csv(typename Json::string_view_type s); // (1)

template <class Json>
Json decode_csv(typename Json::string_view_type s, 
                const basic_csv_serializing_options<typename Json::char_type>& options); // (2)

template <class Json>
Json decode_csv(std::basic_istream<typename Json::char_type>& is); // (3)

template <class Json>
Json decode_csv(std::basic_istream<typename Json::char_type>& is, 
                const basic_csv_serializing_options<typename Json::char_type>& options); // (4)
```

(1) Reads json value from CSV string using default [parameters](csv_serializing_options.md)

(2) Reads json value from CSV string using specified [parameters](csv_serializing_options.md)

(3) Reads json value from CSV input stream using default [parameters](csv_serializing_options.md)

(4) Reads json value from CSV input stream using specified [parameters](csv_serializing_options.md)

#### Return value

Returns a `Json` value

#### Exceptions

Throws [parse_error](parse_error.md) if parsing fails.

### Examples

#### Decode a CSV file with type inference (default)

Example file (sales.csv)
```csv
customer_name,has_coupon,phone_number,zip_code,sales_tax_rate,total_amount
"John Roe",true,0272561313,01001,0.05,431.65
"Jane Doe",false,416-272-2561,55416,0.15,480.70
"Joe Bloggs",false,"4162722561","55416",0.15,300.70
"John Smith",FALSE,NULL,22313-1450,0.15,300.70
```

```c++
#include <fstream>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/csv/csv_reader.hpp>

using namespace jsoncons;
using namespace jsoncons::csv;

int main()
{
    csv_serializing_options options;
    params.assume_header(true);

    params.mapping(mapping_type::n_objects);
    std::ifstream is1("input/sales.csv");
    ojson j1 = decode_csv<ojson>(is1,options);
    std::cout << "\n(1)\n"<< pretty_print(j1) << "\n";

    params.mapping(mapping_type::n_rows);
    std::ifstream is2("input/sales.csv");
    ojson j2 = decode_csv<ojson>(is2,options);
    std::cout << "\n(2)\n"<< pretty_print(j2) << "\n";

    params.mapping(mapping_type::m_columns);
    std::ifstream is3("input/sales.csv");
    ojson j3 = decode_csv<ojson>(is3,options);
    std::cout << "\n(3)\n"<< pretty_print(j3) << "\n";
}
```
Output:
```json
(1)
[
    {
        "customer_name": "John Roe",
        "has_coupon": true,
        "phone_number": "0272561313",
        "zip_code": "01001",
        "sales_tax_rate": 0.05,
        "total_amount": 431.65
    },
    {
        "customer_name": "Jane Doe",
        "has_coupon": false,
        "phone_number": "416-272-2561",
        "zip_code": 55416,
        "sales_tax_rate": 0.15,
        "total_amount": 480.7
    },
    {
        "customer_name": "Joe Bloggs",
        "has_coupon": false,
        "phone_number": "4162722561",
        "zip_code": "55416",
        "sales_tax_rate": 0.15,
        "total_amount": 300.7
    },
    {
        "customer_name": "John Smith",
        "has_coupon": false,
        "phone_number": null,
        "zip_code": "22313-1450",
        "sales_tax_rate": 0.15,
        "total_amount": 300.7
    }
]

(2)
[
    ["customer_name","has_coupon","phone_number","zip_code","sales_tax_rate","total_amount"],
    ["John Roe",true,"0272561313","01001",0.05,431.65],
    ["Jane Doe",false,"416-272-2561",55416,0.15,480.7],
    ["Joe Bloggs",false,"4162722561","55416",0.15,300.7],
    ["John Smith",false,null,"22313-1450",0.15,300.7]
]

(3)
{
    "customer_name": ["John Roe","Jane Doe","Joe Bloggs","John Smith"],
    "has_coupon": [true,false,false,false],
    "phone_number": ["0272561313","416-272-2561",4162722561,null],
    "zip_code": ["01001",55416,55416,"22313-1450"],
    "sales_tax_rate": [0.05,0.15,0.15,0.15],
    "total_amount": [431.65,480.7,300.7,300.7]
}
```

#### Decode a CSV string without type inference

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/csv/csv_reader.hpp>

using namespace jsoncons;
using namespace jsoncons::csv;

int main()
{
    std::string s = R"(employee-no,employee-name,dept,salary
00000001,"Smith,Matthew",sales,150000.00
00000002,"Brown,Sarah",sales,89000.00
)";

    csv_serializing_options options;
    params.assume_header(true)
          .infer_types(false);
    ojson j = decode_csv<ojson>(s,options);

    std::cout << pretty_print(j) << std::endl;
}
```
Output:
```json
[
    {
        "employee-no": "00000001",
        "employee-name": "Smith,Matthew",
        "dept": "sales",
        "salary": "150000.00"
    },
    {
        "employee-no": "00000002",
        "employee-name": "Brown,Sarah",
        "dept": "sales",
        "salary": "89000.00"
    }
]
```

#### Decode a CSV string with specified types

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/csv/csv_reader.hpp>

using namespace jsoncons;
using namespace jsoncons::csv;

int main()
{
    const std::string s = R"(Date,1Y,2Y,3Y,5Y
2017-01-09,0.0062,0.0075,0.0083,0.011
2017-01-08,0.0063,0.0076,0.0084,0.0112
2017-01-08,0.0063,0.0076,0.0084,0.0112
)";

    csv_serializing_options options;
    params.assume_header(true)
          .column_types("string,float,float,float,float");

    // mapping_type::n_objects
    params.mapping(mapping_type::n_objects);
    ojson j1 = decode_csv<ojson>(s,options);
    std::cout << "\n(1)\n"<< pretty_print(j1) << "\n";

    // mapping_type::n_rows
    params.mapping(mapping_type::n_rows);
    ojson j2 = decode_csv<ojson>(s,options);
    std::cout << "\n(2)\n"<< pretty_print(j2) << "\n";

    // mapping_type::m_columns
    params.mapping(mapping_type::m_columns);
    ojson j3 = decode_csv<ojson>(s,options);
    std::cout << "\n(3)\n" << pretty_print(j3) << "\n";
}
```
Output:
```json
(1)
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

(2)
[
    ["Date","1Y","2Y","3Y","5Y"],
    ["2017-01-09",0.0062,0.0075,0.0083,0.011],
    ["2017-01-08",0.0063,0.0076,0.0084,0.0112],
    ["2017-01-08",0.0063,0.0076,0.0084,0.0112]
]

(3)
{
    "Date": ["2017-01-09","2017-01-08","2017-01-08"],
    "1Y": [0.0062,0.0063,0.0063],
    "2Y": [0.0075,0.0076,0.0076],
    "3Y": [0.0083,0.0084,0.0084],
    "5Y": [0.011,0.0112,0.0112]
}
```
#### Decode a CSV string with multi-valued fields separated by subfield delimiters

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/csv/csv_reader.hpp>

using namespace jsoncons;
using namespace jsoncons::csv;

int main()
{
    const std::string s = R"(calculationPeriodCenters,paymentCenters,resetCenters
NY;LON,TOR,LON
NY,LON,TOR;LON
"NY";"LON","TOR","LON"
"NY","LON","TOR";"LON"
)";
    json_serializing_options print_options;
    print_options.array_array_split_lines(line_split_kind::same_line);

    csv_serializing_options options1;
    params1.assume_header(true)
           .subfield_delimiter(';');

    json j1 = decode_csv<json>(s,options1);
    std::cout << "(1)\n" << pretty_print(j1,print_options) << "\n\n";

    csv_serializing_options options2;
    params2.mapping(mapping_type::n_rows)
           .subfield_delimiter(';');

    json j2 = decode_csv<json>(s,options2);
    std::cout << "(2)\n" << pretty_print(j2,print_options) << "\n\n";

    csv_serializing_options options3;
    params3.assume_header(true)
           .mapping(mapping_type::m_columns)
           .subfield_delimiter(';');

    json j3 = decode_csv<json>(s,options3);
    std::cout << "(3)\n" << pretty_print(j3,print_options) << "\n\n";
}
```
Output:
```json
(1)
[

    {
        "calculationPeriodCenters": ["NY","LON"],
        "paymentCenters": "TOR",
        "resetCenters": "LON"
    },
    {
        "calculationPeriodCenters": "NY",
        "paymentCenters": "LON",
        "resetCenters": ["TOR","LON"]
    },
    {
        "calculationPeriodCenters": ["NY","LON"],
        "paymentCenters": "TOR",
        "resetCenters": "LON"
    },
    {
        "calculationPeriodCenters": "NY",
        "paymentCenters": "LON",
        "resetCenters": ["TOR","LON"]
    }
]

(2)
[

    ["calculationPeriodCenters","paymentCenters","resetCenters"],
    [["NY","LON"],"TOR","LON"],
    ["NY","LON",["TOR","LON"]],
    [["NY","LON"],"TOR","LON"],
    ["NY","LON",["TOR","LON"]]
]

(3)
{
    "calculationPeriodCenters": [["NY","LON"],"NY",["NY","LON"],"NY"],
    "paymentCenters": ["TOR","LON","TOR","LON"],
    "resetCenters": ["LON",["TOR","LON"],"LON",["TOR","LON"]]
}
```

