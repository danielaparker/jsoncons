    jsoncons::output_format

    typedef basic_output_format<char> output_format

The output_format class is an instantiation of the basic_output_format class template that uses `char` as the character type.

The default floating point formatting produces digits in decimal format if possible, if not, it produces digits in exponential format. Trailing zeros are removed, except the one immediately following the decimal point. The period character (‘.’) is always used as the decimal point, non English locales are ignored.  A `precision` gives the maximum number of significant digits, the default precision is `16`. On most modern machines, 17 digits is usually enough to capture a floating-point number's value exactly, however, if you change precision to 17, there will be less space saving from removing trailing zeros, e.g. 1.1 becomes 1.1000000000000001. 

The modifier method `floatfield` supports setting the floatfield format flag to `fixed` or `scientific` with `precision` now meaning a specified number of decimal places.

### Header

    #include "jsoncons/output_format.hpp"

### Member constants

    default_precision
The default precision is 16

    default_indent
The default indent is 4

### Constructors

    output_format()
Constructs an `output_format` with default values. 

### Accessors

    size_t indent() const
Returns the level of indentation, the default is 4

    size_t precision() const 
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

    std::ios_base::fmtflags floatfield() const
Returns the floatfield format flag, which by default is not set. The default floating point formatting produces digits in decimal notation if possible, otherwise it produces digits in exponential notation, and all trailing zeros are removed except the one immediately following the decimal point. 

### Modifiers

    void indent(size_t value)

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

    void floatfield(std::ios_base::fmtflags flags)
Allows the floatfield format flag to be set to a specific notation (either `std::ios::fixed` or `std::ios::scientific`.) In the fixed and scientific notations, the precision field specifies the number of digits to display after the decimal point.


