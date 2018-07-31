### jsoncons::json_read_options

```c++
typedef basic_json_read_options<char> json_read_options
```

An abstract class that defines accessors for JSON deserialization options. The `json_read_options` class is an instantiation of the `basic_json_read_options` class template that uses `char` as the character type.

#### Header
```c++
#include <jsoncons/json_read_options.hpp>
```

#### Implementing classes

[json_serializing_options](json_serializing_options)

#### Destructor

    virtual ~json_read_options();

#### Accessors

    virtual bool can_read_nan_replacement() const = 0;

    virtual bool can_read_pos_inf_replacement() const = 0;

    virtual bool can_read_neg_inf_replacement() const = 0;

    virtual const std::string& nan_replacement() const = 0;
NaN replacement. Double NaN values are substituted for JSON string values, if specified

    virtual const std::string& pos_inf_replacement() const = 0; 
Positive infinity replacement. Double positive infinity values are substituted for JSON string values, if specified

    virtual const std::string& neg_inf_replacement() const = 0; 
Negative infinity replacement. Double negative infinity values are substituted for JSON string values, if specified

    virtual size_t max_nesting_depth() = 0;
 Maximum nesting depth when parsing JSON.

