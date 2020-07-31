### Convenience Macros

The `jsoncons` library provides a number of macros that can be used to generate the code to specialize `json_type_traits`
for a user-defined class.

Macro names follow naming conventions.

Component | Description
----------|--------------------
TPL       | Template class with a specified number of template parameters
ALL       | All data members are mandatory
N         | A specified number of data members are mandatory
MEMBER    | Accesses and modifies class data members
CTOR      | Requires constructor that takes all data members in the order they appear in the list
GETTER    | Accesses data members through getter functions
SETTER    | Modifies data members through setter functions
NAME      | Serialize with provided names (instead of C++ member names)

```c++
#include <jsoncons/json_type_traits.hpp>

JSONCONS_N_MEMBER_TRAITS(class_name,num_mandatory,
                         member_name0,member_name1,...) // (1)

JSONCONS_ALL_MEMBER_TRAITS(class_name,
                           member_name0,member_name1,...) // (2)

JSONCONS_TPL_N_MEMBER_TRAITS(num_template_params,
                             class_name,num_mandatory,
                             member_name0,member_name1,...) // (3)  

JSONCONS_TPL_ALL_MEMBER_TRAITS(num_template_params,
                               class_name,
                               member_name0,member_name1,...) // (4)

JSONCONS_N_MEMBER_NAME_TRAITS(class_name,num_mandatory,
                              (member_name0,serialized_name0),
                              (member_name1,serialized_name1)...) // (5)

JSONCONS_ALL_MEMBER_NAME_TRAITS(class_name,
                                (member_name0,serialized_name0),
                                (member_name1,serialized_name1)...) // (6)

JSONCONS_TPL_N_MEMBER_NAME_TRAITS(num_template_params,
                                  class_name,num_mandatory,
                                  (member_name0,serialized_name0),
                                  (member_name1,serialized_name1)...) // (7)

JSONCONS_TPL_ALL_MEMBER_NAME_TRAITS(num_template_params,
                                    class_name,
                                    (member_name0,serialized_name0),
                                    (member_name1,serialized_name1)...) // (8)

JSONCONS_ENUM_TRAITS(enum_name,enumerator0,enumerator1,...) // (9)

JSONCONS_ENUM_NAME_TRAITS(enum_name,
                           (enumerator0,serialized_name0),
                           (enumerator1,serialized_name1)...) // (10)

JSONCONS_N_CTOR_GETTER_TRAITS(class_name,num_mandatory,
                              getter_name0,
                              getter_name1,...) // (11)

JSONCONS_ALL_CTOR_GETTER_TRAITS(class_name,
                                getter_name0,getter_name1,...) // (12)

JSONCONS_TPL_N_CTOR_GETTER_TRAITS(num_template_params,
                                  class_name,num_mandatory,
                                  getter_name0,getter_name1,...) // (13)

JSONCONS_TPL_ALL_CTOR_GETTER_TRAITS(num_template_params,
                                    class_name,
                                    getter_name0,getter_name1,...) // (14)

JSONCONS_N_CTOR_GETTER_NAME_TRAITS(class_name,num_mandatory,
                                   (getter_name0,serialized_name0),
                                   (getter_name1,serialized_name1)...) // (15)

JSONCONS_ALL_CTOR_GETTER_NAME_TRAITS(class_name,
                                     (getter_name0,serialized_name0),
                                     (getter_name1,serialized_name1)...) // (16)

JSONCONS_TPL_N_CTOR_GETTER_NAME_TRAITS(num_template_params,
                                       class_name,num_mandatory,
                                       (getter_name0,serialized_name0),
                                       (getter_name1,serialized_name1)...) // (17)

JSONCONS_TPL_ALL_CTOR_GETTER_NAME_TRAITS(num_template_params,
                                         class_name,
                                         (getter_name0,serialized_name0),
                                         (getter_name1,serialized_name1)...) // (18)

JSONCONS_N_GETTER_SETTER_TRAITS(class_name,get_prefix,set_prefix,num_mandatory,
                                field_name0,field_name1,...) // (19)

JSONCONS_ALL_GETTER_SETTER_TRAITS(class_name,get_prefix,set_prefix,
                                  field_name0,field_name1,...) // (20)

JSONCONS_TPL_N_GETTER_SETTER_TRAITS(num_template_params,
                                    class_name,get_prefix,set_prefix,num_mandatory,
                                    field_name0,field_name1,...) // (21)  

JSONCONS_TPL_ALL_GETTER_SETTER_TRAITS(num_template_params,
                                      class_name,get_prefix,set_prefix,
                                      field_name0,field_name1,...) // (22)

JSONCONS_N_GETTER_SETTER_NAME_TRAITS(class_name,num_mandatory,
                                     (getter_name0,setter_name0,serialized_name0),
                                     (getter_name1,setter_name1,serialized_name1)...) // (23)

JSONCONS_ALL_GETTER_SETTER_NAME_TRAITS(class_name,
                                       (getter_name0,setter_name0,serialized_name0),
                                       (getter_name1,setter_name1,serialized_name1)...) // (24)

JSONCONS_TPL_N_GETTER_SETTER_NAME_TRAITS(num_template_params,
                                         class_name,num_mandatory,
                                         (getter_name0,setter_name0,serialized_name0),
                                         (getter_name1,setter_name1,serialized_name1)...) // (25)

JSONCONS_TPL_ALL_GETTER_SETTER_NAME_TRAITS(num_template_params,
                                           class_name,
                                           (getter_name0,setter_name0,serialized_name0),
                                           (getter_name1,setter_name1,serialized_name1)...) // (26)

JSONCONS_POLYMORPHIC_TRAITS(base_class_name,derived_class_name0,derived_class_name1,...) // (27)
```

(1)-(4) generate the code to specialize `json_type_traits` for a class from member data. 
The serialized names are the stringified member names. 
When decoding to a C++ data structure, 
(1) and (3) require that the first `num_mandatory` member names be present in the JSON,
the rest can have default values. (2) and (4)
require that all member names be present in the JSON. The class must have a default constructor.
If the member data or default constructor are private, the macro `JSONCONS_TYPE_TRAITS_FRIEND`
will make them accessible to `json_type_traits`, used so
 
```c++
class MyClass
{
    JSONCONS_TYPE_TRAITS_FRIEND
...
};
```

(3)-(4) generate the code to specialize `json_type_traits` for a class template from member data. 

(5)-(8) generate the code to specialize `json_type_traits` for a class from member data.
The serialized names are the provided names. The sequence of `(member_nameN,serialized_nameN)`
pairs declares the member name and provided name for each of the class members
that are part of the sequence.
When decoding to a C++ data structure, 
(5) and (7) require that the first `num_mandatory` member names be present in the JSON,
the rest can have default values. (6) and (8) 
require that all member names be present in the JSON. The class must have a default constructor.
If the member data or default constructor are private, the macro `JSONCONS_TYPE_TRAITS_FRIEND`
will make them accessible to `json_type_traits`.
(7)-(8) generate the code to specialize `json_type_traits` for a class template from member data. 

(9) generates the code to specialize `json_type_traits` for an enumerated type from its enumerators.
The serialized name is the stringified enumerator name. 

(10) generates the code to specialize `json_type_traits` for an enumerated type from its enumerators.
The serialized name is the provided name. The sequence of `(enumeratorN,serialized_nameN)`
pairs declares the named constant and provided name for each of the enumerators
that are part of the sequence.

(11)-(14) generate the code to specialize `json_type_traits` for a class from a constructor and get functions. 
The serialized names are the stringified field names. 
When decoding to a C++ data structure, 
(11) and (13) require that the first `num_mandatory` member names be present in the JSON,
the rest can have default values. (12) and (14) 
require that all member names be present in the JSON. The class must have a constructor such that the return types 
of the get functions are convertible to its parameters, taken in order. 
(13)-(14) generate the code to specialize `json_type_traits` for a class template from a constructor and get functions.  

(15)-(18) generate the code to specialize `json_type_traits` for a class from a constructor and get functions.
The serialized names are the provided names. The sequence of `(getter_nameN,serialized_nameN)`
pairs declares the get function and provided name for each of the class members
that are part of the sequence. 
When decoding to a C++ data structure, 
(15) and (17) require that the first `num_mandatory` member names be present in the JSON,
the rest can have default values. (16) and (18) 
require that all member names be present in the JSON. The class must have a constructor such that the return types 
of the get functions are convertible to its parameters, taken in order. 
(17)-(18) generate the code to specialize `json_type_traits` for a class template from a constructor and get functions.  

(19)-(22) generate the code to specialize `json_type_traits` for a class from get and set functions.
The serialized names are the stringified field names. The get and set function names are
formed from the concatenation of `get_prefix` and `set_prefix` with field name.
(19) and (21) require that the first `num_mandatory` member names be present in the JSON,
the rest can have default values. (20) and (22) 
require that all member names be present in the JSON. (21)-(22) generate the code to specialize `json_type_traits` 
for a class template from get and set functions.

(23)-(26) generate the code to specialize `json_type_traits` for a class from get and set functions.
The serialized names are the provided names. The sequence of `(getter_nameN,setter_nameN,serialized_nameN)`
triples declares the get and set functions and provided name for each of the class members
that are part of the sequence. When decoding to a C++ data structure, 
(23) and (25) require that the first `num_mandatory` member names be present in the JSON,
the rest can have default values. (24) and (26) 
require that all member names be present in the JSON. The class must have a default constructor. 
(25)-(26) generate the code to specialize `json_type_traits` for a class template from get and set functions.

(27) generates the code to specialize `json_type_traits` for `std::shared_ptr<base_class>` and `std::unique_ptr<base_class>`.
Each derived class must have a `json_type_traits<Json,derived_class_name>` specialization.
The type selection strategy is based on `json_type_traits<Json,derived_class_name>::is(const Json& j)`.
In the case that `json_type_traits<Json,derived_class_name>` has been generated by one of the
conveniences macros (1)-(26), the type selection strategy is based on the presence of members
in the derived classes.

#### Parameters

`class_name` - the name of a class or struct  
`num_mandatory` - the number of mandatory class data members or accessors  
`enum_name` - the name of an enum type or enum class type  
`num_template_params` - for a class template, the number of template parameters  
`member_nameN` - the name of a class data member. Class data members are normally modifiable, but may be `const` or
`static const`. Data members that are `const` or `static const` are one-way serialized.  
`getter_nameN` - the getter for a class data member  
`(enumeratorN,serialized_nameN)` - an enumerator and corresponding serialized name  
`(getter_nameN,serialized_nameN)` - the getter for a class data member and corresponding serialized name  
`field_nameN` - the base name of a class getter or setter with prefix `get` or `set` stripped out.  
`(getter_nameN,setter_nameN,serialized_nameN)` - the getter and setter for a class data member, and corresponding serialized name  
`base_class_name` - the name of a base class  
`derived_class_nameN` - a class that is derived from the base class, and that has a `json_type_traits<Json,derived_class_nameN>` specialization.  

These macro declarations must be placed at global scope, outside any namespace blocks, and `class_name`, 
`base_class_name` and `derived_class_nameN` must be a fully namespace qualified names.

All of the `json_type_traits` specializations for type `T` generated by the convenience macros include a specialization of
`is_json_type_traits_declared<T>` with member constant `value` equal `true`.

### Examples

[Specialize json_type_traits to support a book class](#A1)  
[Using JSONCONS_ALL_CTOR_GETTER_TRAITS to generate the json_type_traits](#A2)  
[Example with std::shared_ptr, std::unique_ptr and std::optional](#A3)  
[Serialize a polymorphic type based on the presence of members](#A4)  
[Ensuring type selection is possible](#A5)  

<div id="A1"/> 

#### Specialize json_type_traits to support a book class.

```c++
#include <iostream>
#include <jsoncons/json.hpp>
#include <vector>
#include <string>

namespace ns {
    struct book
    {
        std::string author;
        std::string title;
        double price;
    };
} // namespace ns

```c++
JSONCONS_N_MEMBER_TRAITS(ns::book, author, title, price)
```

which expands to the code above.

```c++
using namespace jsoncons; // for convenience

int main()
{
    const std::string s = R"(
    [
        {
            "author" : "Haruki Murakami",
            "title" : "Kafka on the Shore",
            "price" : 25.17
        },
        {
            "author" : "Charles Bukowski",
            "title" : "Pulp",
            "price" : 22.48
        }
    ]
    )";

    std::vector<ns::book> book_list = decode_json<std::vector<ns::book>>(s);

    std::cout << "(1)\n";
    for (const auto& item : book_list)
    {
        std::cout << item.author << ", " 
                  << item.title << ", " 
                  << item.price << "\n";
    }

    std::cout << "\n(2)\n";
    encode_json_pretty(book_list, std::cout);
    std::cout << "\n\n";
}
```
Output:
```
(1)
Haruki Murakami, Kafka on the Shore, 25.17
Charles Bukowski, Pulp, 22.48

(2)
[
    {
        "author": "Haruki Murakami",
        "price": 25.17,
        "title": "Kafka on the Shore"
    },
    {
        "author": "Charles Bukowski",
        "price": 22.48,
        "title": "Pulp"
    }
]
```

<div id="A2"/> 

#### Using JSONCONS_ALL_CTOR_GETTER_TRAITS to generate the json_type_traits 

The macro `JSONCONS_ALL_CTOR_GETTER_TRAITS` will generate the `json_type_traits` boilerplate
for your own types from a constructor and getter functions.

```c++
#include <cassert>
#include <iostream>
#include <jsoncons/json.hpp>

namespace ns {
    enum class hiking_experience {beginner,intermediate,advanced};

    class hiking_reputon
    {
        std::string rater_;
        hiking_experience assertion_;
        std::string rated_;
        double rating_;
        std::optional<std::chrono::seconds> generated_; // use std::optional if C++17
        std::optional<std::chrono::seconds> expires_;
    public:
        hiking_reputon(const std::string& rater,
                       hiking_experience assertion,
                       const std::string& rated,
                       double rating,
                       const std::optional<std::chrono::seconds>& generated = std::optional<std::chrono::seconds>(),
                       const std::optional<std::chrono::seconds>& expires = std::optional<std::chrono::seconds>())
            : rater_(rater), assertion_(assertion), rated_(rated), rating_(rating),
              generated_(generated), expires_(expires)
        {
        }

        const std::string& rater() const {return rater_;}
        hiking_experience assertion() const {return assertion_;}
        const std::string& rated() const {return rated_;}
        double rating() const {return rating_;}
        std::optional<std::chrono::seconds> generated() const {return generated_;}
        std::optional<std::chrono::seconds> expires() const {return expires_;}

        friend bool operator==(const hiking_reputon& lhs, const hiking_reputon& rhs)
        {
            return lhs.rater_ == rhs.rater_ && lhs.assertion_ == rhs.assertion_ && 
                   lhs.rated_ == rhs.rated_ && lhs.rating_ == rhs.rating_ &&
                   lhs.confidence_ == rhs.confidence_ && lhs.expires_ == rhs.expires_;
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
    };

} // namespace ns

// Declare the traits. Specify which data members need to be serialized.

JSONCONS_ENUM_TRAITS(ns::hiking_experience, beginner, intermediate, advanced)
// First four members listed are mandatory, generated and expires are optional
JSONCONS_N_CTOR_GETTER_TRAITS(ns::hiking_reputon, 4, rater, assertion, rated, rating, 
                              generated, expires)

// All members are mandatory
JSONCONS_ALL_CTOR_GETTER_TRAITS(ns::hiking_reputation, application, reputons)

int main()
{
    // Decode the string of data into a c++ structure
    ns::hiking_reputation v = decode_json<ns::hiking_reputation>(data);

    // Iterate over reputons array value
    std::cout << "(1)\n";
    for (const auto& item : v.reputons())
    {
        std::cout << item.rated() << ", " << item.rating();
        if (item.generated())
        {
            std::cout << ", " << (*item.generated()).count();
        }
        std::cout << "\n";
    }

    // Encode the c++ structure into a string
    std::string s;
    encode_json_pretty(v, s);
    std::cout << "(2)\n";
    std::cout << s << "\n";
}
```
Output:
```
(1)
Marilyn C, 0.9, 1514862245
(2)
{
    "application": "hiking",
    "reputons": [
        {
            "assertion": "advanced",
            "generated": 1514862245,
            "rated": "Marilyn C",
            "rater": "HikingAsylum",
            "rating": 0.9
        }
    ]
}
```

<div id="A3"/> 

#### Example with std::shared_ptr, std::unique_ptr and std::optional

This example assumes C++17 language support for `std::optional`.
Lacking that, you can use `jsoncons::optional`.

```c++
#include <cassert>
#include <iostream>
#include <jsoncons/json.hpp>

namespace ns {
    struct smart_pointer_and_optional_test
    {
        std::shared_ptr<std::string> field1;
        std::unique_ptr<std::string> field2;
        std::optional<std::string> field3;
        std::shared_ptr<std::string> field4;
        std::unique_ptr<std::string> field5;
        std::optional<std::string> field6;
        std::shared_ptr<std::string> field7;
        std::unique_ptr<std::string> field8;
        std::optional<std::string> field9;
        std::shared_ptr<std::string> field10;
        std::unique_ptr<std::string> field11;
        std::optional<std::string> field12;
    };

} // namespace ns

// Declare the traits, first 6 members mandatory, last 6 non-mandatory
JSONCONS_N_MEMBER_TRAITS(ns::smart_pointer_and_optional_test,6,
                         field1,field2,field3,field4,field5,field6,
                         field7,field8,field9,field10,field11,field12)

using namespace jsoncons; // for convenience

int main()
{
    ns::smart_pointer_and_optional_test val;
    val.field1 = std::make_shared<std::string>("Field 1"); 
    val.field2 = jsoncons::make_unique<std::string>("Field 2"); 
    val.field3 = "Field 3";
    val.field4 = std::shared_ptr<std::string>(nullptr);
    val.field5 = std::unique_ptr<std::string>(nullptr);
    val.field6 = std::optional<std::string>();
    val.field7 = std::make_shared<std::string>("Field 7"); 
    val.field8 = jsoncons::make_unique<std::string>("Field 8"); 
    val.field9 = "Field 9";
    val.field10 = std::shared_ptr<std::string>(nullptr);
    val.field11 = std::unique_ptr<std::string>(nullptr);
    val.field12 = std::optional<std::string>();

    std::string buf;
    encode_json_pretty(val, buf);
    std::cout << buf << "\n";

    auto other = decode_json<ns::smart_pointer_and_optional_test>(buf);

    assert(*other.field1 == *val.field1);
    assert(*other.field2 == *val.field2);
    assert(*other.field3 == *val.field3);
    assert(!other.field4);
    assert(!other.field5);
    assert(!other.field6);
    assert(*other.field7 == *val.field7);
    assert(*other.field8 == *val.field8);
    assert(*other.field9 == *val.field9);
    assert(!other.field10);
    assert(!other.field11);
    assert(!other.field12);
}
```
Output:
```
{
    "field1": "Field 1",
    "field2": "Field 2",
    "field3": "Field 3",
    "field4": null,
    "field5": null,
    "field6": null,
    "field7": "Field 7",
    "field8": "Field 8",
    "field9": "Field 9"
}
```

<div id="A4"/> 

#### Serialize a polymorphic type based on the presence of members

This example uses the convenience macro `JSONCONS_N_CTOR_GETTER_TRAITS`
to generate the `json_type_traits` boilerplate for the `HourlyEmployee` and `CommissionedEmployee` 
derived classes, and `JSONCONS_POLYMORPHIC_TRAITS` to generate the `json_type_traits` boilerplate
for `std::shared_ptr<Employee>` and `std::unique_ptr<Employee>`. The type selection strategy is based
on the presence of mandatory members, in particular, to the `firstName`, `lastName`, and `wage` members of an
`HourlyEmployee`, and to the `firstName`, `lastName`, `baseSalary`, and `commission` members of a `CommissionedEmployee`.
Non-mandatory members are not considered for the purpose of type selection.

```c++
#include <cassert>
#include <iostream>
#include <vector>
#include <jsoncons/json.hpp>

using namespace jsoncons;

namespace ns {

class Employee
{
    std::string firstName_;
    std::string lastName_;
public:
    Employee(const std::string& firstName, const std::string& lastName)
        : firstName_(firstName), lastName_(lastName)
    {
    }
    virtual ~Employee() noexcept = default;

    virtual double calculatePay() const = 0;

    const std::string& firstName() const {return firstName_;}
    const std::string& lastName() const {return lastName_;}
};

class HourlyEmployee : public Employee
{
    double wage_;
    unsigned hours_;
public:
    HourlyEmployee(const std::string& firstName, const std::string& lastName, 
                   double wage, unsigned hours)
        : Employee(firstName, lastName), 
          wage_(wage), hours_(hours)
    {
    }

    double wage() const {return wage_;}

    unsigned hours() const {return hours_;}

    double calculatePay() const override
    {
        return wage_*hours_;
    }
};

class CommissionedEmployee : public Employee
{
    double baseSalary_;
    double commission_;
    unsigned sales_;
public:
    CommissionedEmployee(const std::string& firstName, const std::string& lastName, 
                         double baseSalary, double commission, unsigned sales)
        : Employee(firstName, lastName), 
          baseSalary_(baseSalary), commission_(commission), sales_(sales)
    {
    }

    double baseSalary() const
    {
        return baseSalary_;
    }

    double commission() const
    {
        return commission_;
    }

    unsigned sales() const
    {
        return sales_;
    }

    double calculatePay() const override
    {
        return baseSalary_ + commission_*sales_;
    }
};

} // ns

JSONCONS_N_CTOR_GETTER_TRAITS(ns::HourlyEmployee, 3, firstName, lastName, wage, hours)
JSONCONS_N_CTOR_GETTER_TRAITS(ns::CommissionedEmployee, 4, firstName, lastName, baseSalary, commission, sales)
JSONCONS_POLYMORPHIC_TRAITS(ns::Employee, ns::HourlyEmployee, ns::CommissionedEmployee)

int main()
{
    std::string input = R"(
[
    {
        "firstName": "John",
        "hours": 1000,
        "lastName": "Smith",
        "wage": 40.0
    },
    {
        "baseSalary": 30000.0,
        "commission": 0.25,
        "firstName": "Jane",
        "lastName": "Doe",
        "sales": 1000
    }
]
    )"; 

    auto v = decode_json<std::vector<std::unique_ptr<ns::Employee>>>(input);

    std::cout << "(1)\n";
    for (const auto& p : v)
    {
        std::cout << p->firstName() << " " << p->lastName() << ", " << p->calculatePay() << "\n";
    }

    std::cout << "\n(2)\n";
    encode_json_pretty(v, std::cout);

    std::cout << "\n\n(3)\n";
    json j(v);
    std::cout << pretty_print(j) << "\n\n";
}
```
Output:
```
(1)
John Smith, 40000
Jane Doe, 30250

(2)
[
    {
        "firstName": "John",
        "hours": 1000,
        "lastName": "Smith",
        "wage": 40.0
    },
    {
        "baseSalary": 30000.0,
        "commission": 0.25,
        "firstName": "Jane",
        "lastName": "Doe",
        "sales": 1000
    }
]

(3)
[
    {
        "firstName": "John",
        "hours": 1000,
        "lastName": "Smith",
        "wage": 40.0
    },
    {
        "baseSalary": 30000.0,
        "commission": 0.25,
        "firstName": "Jane",
        "lastName": "Doe",
        "sales": 1000
    }
]
```

<div id="A5"/>

#### Ensuring type selection is possible

When deserializing a polymorphic type, jsoncons needs to know how
to convert a json value to the proper derived class. In the Employee
example above, the type selection strategy is based
on the presence of members in the derived classes. If
derived classes cannot be distinguished in this way, 
you can introduce extra members. The convenience
macros `JSONCONS_N_MEMBER_TRAITS`, `JSONCONS_ALL_MEMBER_TRAITS`,
`JSONCONS_TPL_N_MEMBER_TRAITS`, `JSONCONS_TPL_ALL_MEMBER_TRAITS`,
`JSONCONS_N_MEMBER_NAME_TRAITS`, `JSONCONS_ALL_MEMBER_NAME_TRAITS`,
`JSONCONS_TPL_N_MEMBER_NAME_TRAITS`, and `JSONCONS_TPL_ALL_MEMBER_NAME_TRAITS`
allow you to have `const` or `static const` data members that are serialized and that 
particpate in the type selection strategy during deserialization. 

```c++
namespace ns {

class Foo
{
public:
    virtual ~Foo() noexcept = default;
};

class Bar : public Foo
{
    static const bool bar = true;
    JSONCONS_TYPE_TRAITS_FRIEND
};

class Baz : public Foo 
{
    static const bool baz = true;
    JSONCONS_TYPE_TRAITS_FRIEND
};

} // ns

JSONCONS_N_MEMBER_TRAITS(ns::Bar,1,bar)
JSONCONS_N_MEMBER_TRAITS(ns::Baz,1,baz)
JSONCONS_POLYMORPHIC_TRAITS(ns::Foo, ns::Bar, ns::Baz)

int main()
{
    std::vector<std::unique_ptr<ns::Foo>> u;
    u.emplace_back(new ns::Bar());
    u.emplace_back(new ns::Baz());

    std::string buffer;
    encode_json(u, buffer);
    std::cout << "(1)\n" << buffer << "\n\n";

    auto v = decode_json<std::vector<std::unique_ptr<ns::Foo>>>(buffer);

    std::cout << "(2)\n";
    for (const auto& ptr : v)
    {
        if (dynamic_cast<ns::Bar*>(ptr.get()))
        {
            std::cout << "A bar\n";
        }
        else if (dynamic_cast<ns::Baz*>(ptr.get()))
        {
            std::cout << "A baz\n";
        } 
    }
}
```

Output:
```
(1)
[{"bar":true},{"baz":true}]

(2)
A bar
A baz
```

