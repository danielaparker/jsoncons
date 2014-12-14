jsoncons provides a class json::any that can contain a value of 
any type as long as that type supports copy construction and 
assignment.

    #include "jsoncons/json.hpp"
    #include <boost/numeric/ublas/matrix.hpp>

    using jsoncons::json;
    using jsoncons::pretty_print;
    using boost::numeric::ublas::matrix;

### Inserting a boost matrix into a json object

This allows you, for example, to insert a boost matrix into a `json` object, 
and to retrieve it back cast to the appropriate type. You do so by wrapping it in
a json::any value, like this:

    json obj;

    matrix<double> A(2,2);
    A(0,0) = 1;
    A(0,1) = 2;
    A(1,0) = 3;
    A(1,1) = 4;

    obj.set("mydata",json::any(A));

    matrix<double>& B = obj["mydata"].any_cast<matrix<double>>();

### Serializing a json `any` value
 
By default, if you write the json object containing your boost matrix to a stream

    std::cout << obj << std::endl;

the output is

    {"mydata":null}

You can, however, implement a function template specialization of `serialize` in 
the `jsoncons` namespace, in a header file, like this:

    #ifndef MY_ANY_SPECIALIZATIONS_HPP
    #define MY_ANY_SPECIALIZATIONS_HPP

    #include "jsoncons/json.hpp"
    #include <boost/numeric/ublas/matrix.hpp>

    namespace jsoncons {

    template<> inline 
    void serialize(json_output_handler& os, const boost::numeric::ublas::matrix<double>& A)
    {
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
    }

    }

    #endif

You'll need to include this header file in every file that inserts a boost matrix into a `json` value.

Now when you write the json object to the stream, the output is

    {"mydata":[[1,2],[3,4]]}

or with pretty printing

    std::cout << pretty_print(obj) << std::endl;

the output becomes    

    {
        "mydata":
        [
            [1,2],
            [3,4]
        ]
    }

### Adding a boost matrix to a json array

You can also add a json `any` value to a `json` array, like this. 

    json arr(json::an_array);
    matrix<double> A(2,2);
    A(0,0) = 1;
    A(0,1) = 2;
    A(1,0) = 3;
    A(1,1) = 4;
    matrix<double> B(2,2);
    B(0,0) = 5;
    B(0,1) = 6;
    B(1,0) = 7;
    B(1,1) = 8;

    arr.add(json::any(A));
    arr.add(json::any(B));

    std::cout << pretty_print(arr) << std::endl;

The output is

    [
        [
            [1,2],
            [3,4]
        ],
        [
            [5,6],
            [7,8]
        ]
    ]
