### jsoncons::csv::encode_csv

Encodes a C++ data structure into the CSV data format.

```c++
#include <jsoncons_ext/csv/csv.hpp>

template <class T,class CharT>
void encode_csv(const T& val, 
                std::basic_string<CharT>& s, 
                const basic_csv_encode_options<CharT>& options = basic_csv_encode_options<CharT>()); // (1)

template <class T, class CharT>
void encode_csv(const T& val, 
                std::basic_ostream<CharT>& os, 
                const basic_csv_encode_options<CharT>& options = basic_csv_encode_options<CharT>()); // (2)
```

(1) Writes a value of type T into a string in the CSV data format, using the specified (or defaulted) [options](basic_csv_options.md). 
Type 'T' must be an instantiation of [basic_json](../basic_json.md) 
or support [json_type_traits](../json_type_traits.md). 

(2) Writes a value of type T into an output stream in the CSV data format, using the specified (or defaulted) [options](basic_csv_options.md). 
Type 'T' must be an instantiation of [basic_json](../basic_json.md) 
or support [json_type_traits](../json_type_traits.md). 

### Examples

#### Encode a json array of objects (n_objects format)

```c++
#include <cassert>
#include <iostream>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/csv/csv.hpp>

using namespace jsoncons;

int main()
{
    const std::string input = R"(
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
        "zip_code": "55416",
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
    )";

    ojson j = ojson::parse(input);

    std::string output;
    csv::csv_options ioptions;
    ioptions.quote_style(csv::quote_style_kind::nonnumeric);
    csv::encode_csv(j, output, ioptions);
    std::cout << output << "\n\n";

    csv::csv_options ooptions;
    ooptions.assume_header(true);
    ojson other = csv::decode_csv<ojson>(output, ooptions);
    assert(other == j);
}
```
Output:
```
customer_name,has_coupon,phone_number,zip_code,sales_tax_rate,total_amount
"John Roe",true,"0272561313","01001",0.05,431.65
"Jane Doe",false,"416-272-2561","55416",0.15,480.7
"Joe Bloggs",false,"4162722561","55416",0.15,300.7
"John Smith",false,null,"22313-1450",0.15,300.7
```

#### Encode a json array of arrays (n_rows format)

```c++
#include <cassert>
#include <iostream>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/csv/csv.hpp>

using namespace jsoncons;

int main()
{
    const std::string input = R"(
[
    ["customer_name","has_coupon","phone_number","zip_code","sales_tax_rate","total_amount"],
    ["John Roe",true,"0272561313","01001",0.05,431.65],
    ["Jane Doe",false,"416-272-2561","55416",0.15,480.7],
    ["Joe Bloggs",false,"4162722561","55416",0.15,300.7],
    ["John Smith",false,null,"22313-1450",0.15,300.7]
]
    )";


    json j = json::parse(input);

    std::string output;
    csv::csv_options ioptions;
    ioptions.quote_style(csv::quote_style_kind::nonnumeric);
    csv::encode_csv(j, output, ioptions);
    std::cout << output << "\n\n";

    csv::csv_options ooptions;
    ooptions.assume_header(true)
            .mapping(csv::mapping_kind::n_rows);
    json other = csv::decode_csv<json>(output, ooptions);
    assert(other == j);
}
```
Output:
```
"customer_name","has_coupon","phone_number","zip_code","sales_tax_rate","total_amount"
"John Roe",true,"0272561313","01001",0.05,431.65
"Jane Doe",false,"416-272-2561","55416",0.15,480.7
"Joe Bloggs",false,"4162722561","55416",0.15,300.7
"John Smith",false,null,"22313-1450",0.15,300.7
```

#### Encode a json object of name-array members (m_columns format)

```c++
#include <cassert>
#include <iostream>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/csv/csv.hpp>

using namespace jsoncons;

int main()
{
    const std::string input = R"(
{
    "customer_name": ["John Roe","Jane Doe","Joe Bloggs","John Smith"],
    "has_coupon": [true,false,false,false],
    "phone_number": ["0272561313","416-272-2561","4162722561",null],
    "zip_code": ["01001","55416","55416","22313-1450"],
    "sales_tax_rate": [0.05,0.15,0.15,0.15],
    "total_amount": [431.65,480.7,300.7,300.7]
}
    )";

    ojson j = ojson::parse(input);

    std::string output;
    csv::csv_options ioptions;
    ioptions.quote_style(csv::quote_style_kind::nonnumeric);
    csv::encode_csv(j, output, ioptions);
    std::cout << output << "\n\n";

    csv::csv_options ooptions;
    ooptions.assume_header(true)
            .mapping(csv::mapping_kind::m_columns);
    ojson other = csv::decode_csv<ojson>(output, ooptions);
    assert(other == j);
}
```
Output:
```
customer_name,has_coupon,phone_number,zip_code,sales_tax_rate,total_amount
"John Roe",true,"0272561313","01001",0.05,431.65
"Jane Doe",false,"416-272-2561","55416",0.15,480.7
"Joe Bloggs",false,"4162722561","55416",0.15,300.7
"John Smith",false,null,"22313-1450",0.15,300.7
```


