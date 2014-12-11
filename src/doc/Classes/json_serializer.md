    jsoncons::json_serializer

    typedef basic_json_serializer<char> json_serializer

The `json_serializer` class is an instantiation of the `basic_json_serializer` class template that uses `char` as the character type. It implements [json_output_handler](json_output_handler) and supports pretty print serialization.

### Header

    #include "jsoncons/json_serializer.hpp"

### Implemented interfaces

[json_output_handler](json_output_handler)

### Constructors

    json_serializer(std::ostream& os)
Constructs a new serializer that writes to the specified output stream.

    json_serializer(std::ostream& os, bool indenting)
Constructs a new serializer that writes to the specified output stream.

    json_serializer(std::ostream& os, const output_format& format)
Constructs a new serializer that writes to the specified output stream using the specified [output_format](output_format).

    json_serializer(std::ostream& os, const output_format& format, bool indenting)
Constructs a new serializer that writes to the specified output stream using the specified [output_format](output_format).

### Destructor

    virtual ~json_serializer()

### Implementing methods

    virtual void begin_json()
Receive notification of the start of a JSON document

    virtual void end_json()
Receive notification of the end of a JSON document

    virtual void begin_object()
Receive notification of the start of a JSON object

    virtual void end_object()
Receive notification of the end of a JSON object

    virtual void begin_array()
Receive notification of the start of a JSON array

    virtual void end_array()
Receive notification of the end of a JSON object

    virtual void write_name(const char* p, size_t length)
Receive notification of the `name` of a name value pair

    virtual void write_null()
Receive notification of `null`

    virtual void write_string(const char* p, size_t length)
Receive notification of a `string` value

    virtual void write_double(double value)
Receive notification of a floating point number

    virtual void write_longlong(long long value)
Receive notification of a signed integer

    virtual void write_ulonglong(unsigned long long value)
Receive notification of a non-negative integer

    virtual void write_bool(bool value)
Receive notification of a boolean value

### Examples

### Feeding json events directly to a `json_serializer`

    #include <iostream>
    #include <boost/numeric/ublas/matrix.hpp>
    #include "jsoncons/json_serializer.hpp"

    using boost::numeric::ublas::matrix;
    using jsoncons::json_serializer;
    using jsoncons::output_format;

    int main()
    {
        matrix<double> A(2,2);
        A(0,0) = 1;
        A(0,1) = 2;
        A(1,0) = 3;
        A(1,1) = 4;

        json_serializer os(std::cout, true);  // pretty printing
        os.begin_array();
        for (size_t i = 0; i < A.size1(); ++i)
        {
            os.begin_array();
            for (size_t j = 0; j < A.size2(); ++j)
            {
                os.value(A(i, j));
            }
            os.end_array();
        }
        os.end_array();

        return 0;
    }

The output is

    [
        [1,2],
        [3,4]
    ]

