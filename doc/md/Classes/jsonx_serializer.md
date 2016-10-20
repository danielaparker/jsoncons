```c++
jsoncons::jsonx:jsonx_serializer

typedef basic_jsonx_serializer<char> jsonx_serializer
```
The `jsonx_serializer` class is an instantiation of the `basic_jsonx_serializer` class template that uses `char` as the character type. It implements [json_output_handler](json_output_handler) and supports formatting a JSON value as [JSONx](http://www.ibm.com/support/knowledgecenter/SS9H2Y_7.5.0/com.ibm.dp.doc/json_jsonx.html) (XML).

### Header

    #include "jsoncons/jsonx/jsonx_serializer.hpp"

### Implemented interfaces

[json_output_handler](json_output_handler)

### Constructors

    jsonx_serializer(std::ostream& os)
Constructs a new serializer that writes to the specified output stream.
You must ensure that the output stream exists as long as does `jsonx_serializer`, as `jsonx_serializer` holds a pointer to but does not own this object.

    jsonx_serializer(std::ostream& os, bool indenting)
Constructs a new serializer that writes to the specified output stream.
You must ensure that the output stream exists as long as does `jsonx_serializer`, as `jsonx_serializer` holds a pointer to but does not own this object.

    jsonx_serializer(std::ostream& os, const output_format& format)
Constructs a new serializer that writes to the specified output stream using the specified [output_format](output_format).
You must ensure that the output stream exists as long as does `jsonx_serializer`, as `jsonx_serializer` holds a pointer to but does not own this object.

    jsonx_serializer(std::ostream& os, const output_format& format, bool indenting)
Constructs a new serializer that writes to the specified output stream using the specified [output_format](output_format).
You must ensure that the output stream exists as long as does `jsonx_serializer`, as `jsonx_serializer` holds a pointer to but does not own this object.

### Destructor

    virtual ~jsonx_serializer()

### Examples
```
#include "jsoncons/json.hpp"
#include "jsoncons_ext/jsonx/jsonx_serializer.hpp"
#include <fstream>

using namespace jsoncons;
using namespace jsoncons::jsonx;
```
### JSONx conversion example
```c++
json person = json::parse(R"(
{
  "name":"John Smith",
  "address": {
    "streetAddress": "21 2nd Street",
    "city": "New York",
    "state": "NY",
    "postalCode": 10021
  },
  "phoneNumbers": [
    "212 555-1111",
    "212 555-2222"
  ],
  "additionalInfo": null,
  "remote": false,
  "height": 62.4,
  "ficoScore": " > 640"
}
)");

jsonx_serializer serializer(std::cout,true);

person.write(serializer);
```

Output:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<json:object xsi:schemaLocation="http://www.datapower.com/schemas/json jsonx.xsd"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xmlns:json="http://www.ibm.com/xmlns/prod/2009/jsonx">
    <json:null name="additionalInfo">null</json:null>
    <json:object name="address">
        <json:string name="city">New York</json:string>
        <json:number name="postalCode">10021</json:number>
        <json:string name="state">NY</json:string>
        <json:string name="streetAddress">21 2nd Street</json:string>
    </json:object>
    <json:string name="ficoScore"> > 640</json:string>
    <json:number name="height">62.4</json:number>
    <json:string name="name">John Smith</json:string>
    <json:array name="phoneNumbers">
        <json:string>212 555-1111</json:string>
        <json:string>212 555-2222</json:string>
    </json:array>
    <json:boolean name="remote">false</json:boolean>
</json:object>
```
### Special characters and escaped characters
```c++
ojson special_chars = ojson::parse(R"(
{
  "&":"&",
  "<":"<",
  ">":">",
  "\"":"\"",
  "'":"'",
  "\u00A9":""
}
)");

jsonx_serializer serializer(std::cout,true);

special_chars.write(serializer);
```

Output:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<json:object xsi:schemaLocation="http://www.datapower.com/schemas/json jsonx.xsd"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xmlns:json="http://www.ibm.com/xmlns/prod/2009/jsonx">
    <json:string name="&amp;">&amp;</json:string>
    <json:string name="&lt;">&lt;</json:string>
    <json:string name=">">></json:string>
    <json:string name="&#34;">"</json:string>
    <json:string name="'">'</json:string>
    <json:string name="&#x00A9;"></json:string>
</json:object>
```
