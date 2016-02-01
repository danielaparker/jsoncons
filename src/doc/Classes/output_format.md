    jsoncons::output_format

    typedef basic_output_format<char> output_format

The output_format class is an instantiation of the basic_output_format class template that uses `char` as the character type.

The default floating point formatting produces digits in decimal format if possible, if not, it produces digits in exponential format. Trailing zeros are removed, except the one immediately following the decimal point. The period character (‘.’) is always used as the decimal point, non English locales are ignored.  A `precision` gives the maximum number of significant digits, the default precision is `15`, which allows round-trip IEEE 754. On most modern machines, 17 digits is usually enough to capture a floating-point number's value exactly, however, if you change precision to 17, you lose round-trip, e.g. 1.1 read may become 1.1000000000000001 written. 

### Header

    #include "jsoncons/output_format.hpp"

### Member constants

    default_precision
The default precision is 15

    default_indent
The default indent is 4

### Constructors

    output_format()
Constructs an `output_format` with default values. 

### Accessors

    int indent() const
Returns the level of indentation, the default is 4

    int precision() const 
Returns the maximum number of significant digits.

    bool escape_all_non_ascii() const
The default is false

    bool escape_solidus() const
The default is false

    bool replace_nan() const
The defult is `true`

    bool replace_pos_inf() const
The defult is `true`

    bool replace_neg_inf() const
The defult is `true`

    std::string nan_replacement() const 
The default is "null"

    std::string pos_inf_replacement() const 
The default is "null"

    std::string neg_inf_replacement() const 
The default is "null"

### Modifiers

    void indent(int value)

    void escape_all_non_ascii(bool value)

    void escape_solidus(bool value)

    void replace_nan(bool replace)

    void replace_inf(bool replace)

    void replace_pos_inf(bool replace)

    void replace_neg_inf(bool replace)

    void nan_replacement(const std::string& replacement)

    void pos_inf_replacement(const std::string& replacement)

    void neg_inf_replacement(const std::string& replacement)
Sets replacement text for negative infinity.

    void precision(size_t prec)

## Examples

### Default NaN, inf and -inf replacement
```c++
    json obj;
    obj["field1"] = std::sqrt(-1.0);
    obj["field2"] = 1.79e308*1000;
    obj["field3"] = -1.79e308*1000;
    std::cout << obj << std::endl;
```
The output is
```json
    {"field1":null,"field2":null,"field3":null}
```
### User specified `Nan`, `Inf` and `-Inf` replacement

```c++
    json obj;
    obj["field1"] = std::sqrt(-1.0);
    obj["field2"] = 1.79e308*1000;
    obj["field3"] = -1.79e308*1000;

    output_format format;
    format.nan_replacement("null");        // default is "null"
    format.pos_inf_replacement("1e9999");  // default is "null"
    format.neg_inf_replacement("-1e9999"); // default is "null"

    std::cout << pretty_print(obj,format) << std::endl;
```

The output is
```json
    {
        "field1":null,
        "field2":1e9999,
        "field3":-1e9999
    }
```

