    jsoncons_ext::csv::csv_serializer

The `csv_serializer` class is an instantiation of the `basic_csv_serializer` class template that uses `char` as the character type.  It implements [json_output_handler](json_output_handler) and supports formatting a JSON value as a [CSV file](http://tools.ietf.org/html/rfc4180).

### Header

    #include "jsoncons_ext/csv/csv_serializer.hpp"

### Constructors

    csv_serializer(std::ostream& os)
Constructs a `csv_serializer` that is associated with an output stream
`os`. Uses default [csv formatting parameters](csv formatting parameters).
You must ensure that the output stream exists as long as does `csv_serializer`, as `json_serializer` holds a pointer to but does not own this object.

    csv_serializer(std::ostream& os,
                   const json& params)
Constructs a `csv_serializer` that is associated with an output stream
`os` and [csv formatting parameters](csv formatting parameters).
You must ensure that the output stream exists as long as does `csv_serializer`, as `json_serializer` holds a pointer to but does not own this object.

### Member functions


### Destructor

    virtual ~json_serializer()

### Examples

### Serializing an array of json values to a comma delimted file

#### JSON input file 

    [
        ["country_code","name"],
        ["ABW","ARUBA"],
        ["ATF","FRENCH SOUTHERN TERRITORIES, D.R. OF"],
        ["VUT","VANUATU"],
        ["WLF","WALLIS & FUTUNA ISLANDS"]
    ]

Note 

- The third array element has a value that contains a comma, in the CSV file this value will be quoted.

#### Serializing the comma delimited file with csv_serializer

    std::string in_file = "input/countries.json";
    std::ifstream is(in_file);

    json_deserializer handler;
    json_reader reader(is,handler);
    reader.read();
    json countries = std::move(handler.root());

    csv_serializer serializer(std::cout);

    countries.to_stream(serializer);

#### Output 

    country_code,name
    ABW,ARUBA
    ATF,"FRENCH SOUTHERN TERRITORIES, D.R. OF"
    VUT,VANUATU
    WLF,WALLIS & FUTUNA ISLANDS

### Serializing an array of json objects to a tab delimted file

#### JSON input file

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

Note 

- The names in the first object in the array will be used for the header row of the CSV file
- The fourth object has a `note` member whose value contains escaped quotes and an escaped new line character, in the CSV file, this value will be quoted, and it will contain a new line character and escaped quotes.

#### Reading the tab delimited file with csv_serializer

    std::string in_file = "input/employees.json";
    std::ifstream is(in_file);

    json_deserializer handler;
    json params;
    params["field_delimiter"] = "\t";

    json_reader reader(is,handler);
    reader.read();
    json employees = std::move(handler.root());

    csv_serializer serializer(std::cout,params);

    employees.to_stream(serializer);

#### Tab delimited output file

    dept    employee-name   employee-no     note    salary
    sales   Smith, Matthew  00000001                150,000.00
    sales   Brown, Sarah    00000002                89,000.00
    finance Oberc, Scott    00000003                110,000.00
    sales   Scott, Colette  00000004        ""Exemplary"" employee
    Dependable, trustworthy 75,000.00
