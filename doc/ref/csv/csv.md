### csv extension

The csv extension implements reading (writing) JSON values from (to) CSV files

[decode_csv](decode_csv.md)

[encode_csv](encode_csv.md)

[csv_serializing_options](csv_serializing_options.md)

[csv_reader](csv_reader.md)

[csv_serializer](csv_serializer.md)

### Examples

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


