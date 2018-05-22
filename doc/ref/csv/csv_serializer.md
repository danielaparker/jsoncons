### jsoncons::csv::csv_serializer

The `csv_serializer` class is an instantiation of the `basic_csv_serializer` class template that uses `char` as the character type.  It implements [json_content_handler](json_content_handler.md) and supports formatting a JSON value as a [CSV file](http://tools.ietf.org/html/rfc4180).

`csv_serializer` is noncopyable and nonmoveable.

#### Header
```c++
#include <jsoncons_ext/csv/csv_serializer.hpp>
```
#### Constructors

    csv_serializer(std::ostream& os)
Constructs a `csv_serializer` that is associated with an output stream
`os`. Uses default [csv_serializing_options](csv_serializing_options.md).
You must ensure that the output stream exists as long as does `csv_serializer`, as `json_serializer` holds a pointer to but does not own this object.

    csv_serializer(std::ostream& os,
                   const csv_serializing_options& options)
Constructs a `csv_serializer` that is associated with an output stream
`os` and [csv_serializing_options](csv_serializing_options.md).
You must ensure that the output stream exists as long as does `csv_serializer`, as `json_serializer` holds a pointer to but does not own this object.

#### Member functions


#### Destructor

    virtual ~json_serializer()

### Examples

### Serializing an array of json values to a comma delimted file

#### JSON input file 
```json
[
    ["country_code","name"],
    ["ABW","ARUBA"],
    ["ATF","FRENCH SOUTHERN TERRITORIES, D.R. OF"],
    ["VUT","VANUATU"],
    ["WLF","WALLIS & FUTUNA ISLANDS"]
]
```
Note 

- The third array element has a value that contains a comma, in the CSV file this value will be quoted.

#### Serializing the comma delimited file with csv_serializer
```c++
std::string in_file = "input/countries.json";
std::ifstream is(in_file);

json_decoder<json> decoder;
json_reader reader(is,decoder);
reader.read();
json countries = decoder.get_result();

csv_serializer serializer(std::cout);

countries.dump(serializer);
```
#### Output 
```
country_code,name
ABW,ARUBA
ATF,"FRENCH SOUTHERN TERRITORIES, D.R. OF"
VUT,VANUATU
WLF,WALLIS & FUTUNA ISLANDS
```
### Serializing an array of json objects to a tab delimted file

#### JSON input file
```json
[
    {
        "dept":"sales",
        "employee-name":"Smith, Matthew",
        "employee-no":"00000001",
        "note":"",
        "salary":"150,000.00"
    },
    {
        "dept":"sales",
        "employee-name":"Brown, Sarah",
        "employee-no":"00000002",
        "note":"",
        "salary":"89,000.00"
    },
    {
        "dept":"finance",
        "employee-name":"Oberc, Scott",
        "employee-no":"00000003",
        "salary":"110,000.00"
    },
    {
        "dept":"sales",
        "employee-name":"Scott, Colette",
        "employee-no":"00000004",
        "note":"\"Exemplary\" employee\nDependable, trustworthy",
        "comment":"Team player",
        "salary":"75,000.00"
    }
]
```
Note 

- The names in the first object in the array will be used for the header row of the CSV file
- The fourth object has a `note` member whose value contains escaped quotes and an escaped new line character, in the CSV file, this value will be quoted, and it will contain a new line character and escaped quotes.

#### Dump json value to a tab delimited file
```c++
std::string in_file = "input/employees.json";
std::ifstream is(in_file);

json_decoder<json> decoder;
csv_serializing_options options;
params.field_delimiter = '\t';

json_reader reader(is,decoder);
reader.read();
json employees = decoder.get_result();

csv_serializer serializer(std::cout,options);

employees.dump(serializer);
```
#### Tab delimited output file
```
dept    employee-name   employee-no     note    salary
sales   Smith, Matthew  00000001                150,000.00
sales   Brown, Sarah    00000002                89,000.00
finance Oberc, Scott    00000003                110,000.00
sales   Scott, Colette  00000004        ""Exemplary"" employee
Dependable, trustworthy 75,000.00
```

#### Dump json value to csv file

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/csv/csv_serializer.hpp>

using namespace jsoncons;

int main()
{
    const json books = json::parse(R"(
    [
        {
            "title" : "Kafka on the Shore",
            "author" : "Haruki Murakami",
            "price" : 25.17
        },
        {
            "title" : "Women: A Novel",
            "author" : "Charles Bukowski",
            "price" : 12.00
        },
        {
            "title" : "Cutter's Way",
            "author" : "Ivan Passer"
        }
    ]
    )");

    csv_serializing_options options;
    params.column_names("author,title,price");

    csv_serializer serializer(std::cout, options);

    books.dump(serializer);
}
```

Output:

```csv
author,title,price
Haruki Murakami,Kafka on the Shore,25.17
Charles Bukowski,Women: A Novel,12.0
Ivan Passer,Cutter's Way,
```

