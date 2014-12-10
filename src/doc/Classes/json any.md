    jsoncons::json::any

`any` can contain any value that supports copy construction and assignment.

### Constructors

    any()

    any(const any& val)

    any(any&& val)

    template<typename T>
    explicit any(T val, typename std::enable_if<!std::is_same<any, typename std::decay<T>::type>::value,int>::type* = 0)

### Destructor

    ~any()

### Assignment operator

    any& operator=(any rhs)
Supports copy and move assignment

### Accessors
    
    template <typename T>
    const T& cast() const

    template <typename T>
    T& cast() 

### Example

A `json::any` object can contain a value of any type as long as 
that type supports copy construction and assignment. This allows you, for example, to insert a boost matrix into a `json` object, 
and to retrieve it back cast to the appropriate type. 

    #include "jsoncons/json.hpp"
    #include <boost/numeric/ublas/matrix.hpp>

    using jsoncons::json;
    using boost::numeric::ublas::matrix;

    json obj;

    matrix<double> A(2,2);
    A(0,0) = 1;
    A(0,1) = 2;
    A(1,0) = 3;
    A(1,1) = 4;

    obj.set("mydata",json::any(A));

    matrix<double>& B = obj["mydata"].any_cast<matrix<double>>();


