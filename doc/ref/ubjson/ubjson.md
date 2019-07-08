### ubjson extension

The ubjson extension implements encode to and decode from the [Universal Binary JSON Specification](http://ubjson.org/) data format.
You can either parse into or serialize from a variant-like structure, [basic_json](../json.md), or your own
data structures, using [json_type_traits](../json_type_traits.md).

[decode_ubjson](decode_ubjson.md)

[encode_ubjson](encode_ubjson.md)

[ubjson_encoder](ubjson_encoder.md)

#### jsoncons-ubjson mappings

jsoncons data item|jsoncons tag|UBJSON data item
--------------|------------------|---------------
null          |                  | null
bool          |                  | true or false
int64         |                  | uint8_t or integer
uint64        |                  | uint8_t or integer
double        |                  | float 32 or float 64
string        |                  | string
string        | bigint      | high precision number type
string        | bigdec      | high precision number type
byte_string   |                  | array of uint8_t
array         |                  | array 
object        |                  | object

### Examples

#### encode/decode UBJSON from/to basic_json

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/ubjson/ubjson.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using namespace jsoncons;

int main()
{
    ojson j1 = ojson::parse(R"(
    {
       "application": "hiking",
       "reputons": [
       {
           "rater": "HikingAsylum",
           "assertion": "advanced",
           "rated": "Marilyn C",
           "rating": 0.90
         }
       ]
    }
    )");

    // Encode a basic_json value to UBJSON
    std::vector<uint8_t> data;
    ubjson::encode_ubjson(j1, data);

    // Decode UBJSON to a basic_json value
    ojson j2 = ubjson::decode_ubjson<ojson>(data);
    std::cout << "(1)\n" << pretty_print(j2) << "\n\n";

    // Accessing the data items 

    const ojson& reputons = j2["reputons"];

    std::cout << "(2)\n";
    for (auto element : reputons.array_range())
    {
        std::cout << element.at("rated").as<std::string>() << ", ";
        std::cout << element.at("rating").as<double>() << "\n";
    }
    std::cout << std::endl;

    // Get a UBJSON value for a nested data item with jsonpointer
    std::error_code ec;
    const auto& rated = jsonpointer::get(j2, "/reputons/0/rated", ec);
    if (!ec)
    {
        std::cout << "(3) " << rated.as_string() << "\n";
    }

    std::cout << std::endl;
}
```
Output:
```
(1)
{
    "application": "hiking",
    "reputons": [
        {
            "rater": "HikingAsylum",
            "assertion": "advanced",
            "rated": "Marilyn C",
            "rating": 0.9
        }
    ]
}

(2)
Marilyn C, 0.9

(3) Marilyn C
```

#### encode/decode UBJSON from/to your own data structures

```c++
#include <cassert>
#include <iostream>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/ubjson/ubjson.hpp>

namespace ns {

    enum class hiking_experience {beginner,intermediate,advanced};

    class hiking_reputon
    {
        std::string rater_;
        hiking_experience assertion_;
        std::string rated_;
        double rating_;
    public:
        hiking_reputon(const std::string& rater,
                       hiking_experience assertion,
                       const std::string& rated,
                       double rating)
            : rater_(rater), assertion_(assertion), rated_(rated), rating_(rating)
        {
        }

        const std::string& rater() const {return rater_;}
        hiking_experience assertion() const {return assertion_;}
        const std::string& rated() const {return rated_;}
        double rating() const {return rating_;}

        friend bool operator==(const hiking_reputon& lhs, const hiking_reputon& rhs)
        {
            return lhs.rater_ == rhs.rater_ && lhs.assertion_ == rhs.assertion_ && 
                   lhs.rated_ == rhs.rated_ && lhs.rating_ == rhs.rating_;
        }

        friend bool operator!=(const hiking_reputon& lhs, const hiking_reputon& rhs)
        {
            return !(lhs == rhs);
        };
    };

    class hiking_reputation
    {
        std::string application_;
        std::vector<hiking_reputon> reputons_;
    public:
        hiking_reputation(const std::string& application, 
                          const std::vector<hiking_reputon>& reputons)
            : application_(application), 
              reputons_(reputons)
        {}

        const std::string& application() const { return application_;}
        const std::vector<hiking_reputon>& reputons() const { return reputons_;}

        friend bool operator==(const hiking_reputation& lhs, const hiking_reputation& rhs)
        {
            return (lhs.application_ == rhs.application_) && (lhs.reputons_ == rhs.reputons_);
        }

        friend bool operator!=(const hiking_reputation& lhs, const hiking_reputation& rhs)
        {
            return !(lhs == rhs);
        };
    };

} // namespace ns

// Declare the traits. Specify which data members need to be serialized.
JSONCONS_ENUM_TRAITS_DECL(ns::hiking_experience, beginner, intermediate, advanced)
JSONCONS_GETTER_CTOR_TRAITS_DECL(ns::hiking_reputon, rater, assertion, rated, rating)
JSONCONS_GETTER_CTOR_TRAITS_DECL(ns::hiking_reputation, application, reputons)

int main()
{
    ns::hiking_reputation val("hiking", { ns::hiking_reputon{"HikingAsylum",ns::hiking_experience::advanced,"Marilyn C",0.90} });

    // Encode a ns::hiking_reputation value to UBJSON
    std::vector<uint8_t> data;
    ubjson::encode_ubjson(val, data);

    // Decode UBJSON to a ns::hiking_reputation value
    ns::hiking_reputation val2 = ubjson::decode_ubjson<ns::hiking_reputation>(data);

    assert(val2 == val);
}
```

