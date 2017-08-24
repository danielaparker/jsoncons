```c++
jsoncons::dump
```
Serialize C++ object to a JSON formatted stream, governed by `json_stream_traits`

### Header
```c++
#include <jsoncons/json_stream_traits.hpp>

template <class CharT, class T>
void dump(const T& val, basic_json_output_handler<CharT>& handler) (1)

template <class CharT, class T>
void dump_body(const T& val, basic_json_output_handler<CharT>& handler) (2)

template <class CharT, class T>
void dump(const T& val, std::basic_ostream<CharT>& os) (3)

template <class CharT, class T>
void dump(const T& val, const basic_serialization_options<CharT>& options,
          std::basic_ostream<CharT>& os) (4)

template <class CharT, class T>
void dump(const T& val, std::basic_ostream<CharT>& os, bool pprint) (5)

template <class CharT, class T>
void dump(const T& val, const basic_serialization_options<CharT>& options,
          std::basic_ostream<CharT>& os, bool pprint) (6)
```

(1) Calls `begin_json()` on `handler`, applies `json_stream_traits` to serialize `val` to JSON output stream, and calls `end_json()` on `handler`.

(2) Applies `json_stream_traits` to serialize `val` to JSON output stream, but does not call begin_json() and `end_json()`.

### Parameters

<table>
  <tr>
    <td>val</td>
    <td>C++ object</td> 
  </tr>
  <tr>
    <td>handler</td>
    <td>JSON output handler</td> 
  </tr>
  <tr>
    <td>options</td>
    <td>Serialization options</td> 
  </tr>
  <tr>
    <td>os</td>
    <td>Output stream</td> 
  </tr>
  <tr>
    <td>pprint</td>
    <td><code>true</code> to pretty print, otherwise <code>false</code></td> 
  </tr>
</table>

### Return value

None 

### See also

- [json_output_handler](json_output_handler.md)
- [serialization_options](serialization_options.md)
    
### Examples

#### Map with string-tuple pairs

```c++
#include <iostream>
#include <map>
#include <tuple>
#include <jsoncons/json_stream_traits.hpp>

using namespace jsoncons;

int main()
{
    std::map<std::string,std::tuple<std::string,std::string,double>> employees = 
    { 
        {"John Smith",{"Hourly","Software Engineer",10000}},
        {"Jane Doe",{"Commission","Sales",20000}}
    };

    std::cout << "(1)\n" << std::endl; 
    dump(employees,std::cout);
    std::cout << "\n\n";

    std::cout << "(2) Again, with pretty print\n" << std::endl; 
    dump(employees,std::cout,true);
}
```
Output:
```
(1)
{"Jane Doe":["Commission","Sales",20000.0],"John Smith":["Hourly","Software Engineer",10000.0]}

(2) Again, with pretty print
{
    "Jane Doe": ["Commission","Sales",20000.0],
    "John Smith": ["Hourly","Software Engineer",10000.0]
}
```
    
#### Contain JSON output in an object

```c++
#include <iostream>
#include <map>
#include <tuple>
#include <jsoncons/json_stream_traits.hpp>

using namespace jsoncons;

int main()
{
    std::map<std::string,std::tuple<std::string,std::string,double>> employees = 
    { 
        {"John Smith",{"Hourly","Software Engineer",10000}},
        {"Jane Doe",{"Commission","Sales",20000}}
    };

    // `true` means pretty print
    json_serializer serializer(std::cout, true); 

    serializer.begin_json();       
    serializer.begin_object();       
    serializer.name("Employees");       
    dump_body(employees, serializer);
    serializer.end_object();       
    serializer.end_json();       
}
```
Output:
```json
{
    "Employees": {
        "Jane Doe": ["Commission","Sales",20000.0],
        "John Smith": ["Hourly","Software Engineer",10000.0]
    }
}
```
    
#### Extending `json_stream_traits`

```c++
class Employee
{
    std::string name_;
public:
    Employee(const std::string& name)
        : name_(name)
    {
    }
    virtual ~Employee() = default;
    const std::string& name() const
    {
        return name_;
    }
    virtual double calculatePay() const = 0;
};

class HourlyEmployee : public Employee
{
public:
    HourlyEmployee(const std::string& name)
        : Employee(name)
    {
    }
    double calculatePay() const override
    {
        return 10000;
    }
};

class CommissionedEmployee : public Employee
{
public:
    CommissionedEmployee(const std::string& name)
        : Employee(name)
    {
    }
    double calculatePay() const override
    {
        return 20000;
    }
};

namespace jsoncons
{
    template <>
    struct json_stream_traits<std::shared_ptr<Employee>>
    {
        static void encode(const std::shared_ptr<Employee>& val, json_output_handler& handler)
        {
            handler.begin_object();
            handler.name("Name");
            handler.string_value(val->name());
            handler.name("Pay");
            handler.double_value(val->calculatePay());
            handler.end_object();
        }
    };
};

int main()
{
    std::vector<std::shared_ptr<Employee>> employees; 

    employees.push_back(std::make_shared<HourlyEmployee>("John Smith"));
    employees.push_back(std::make_shared<CommissionedEmployee>("Jane Doe"));

    dump(employees,std::cout,true);
}
```
Output:
```json
[
    {
        "Name": "John Smith",
        "Pay": 10000.0
    },
    {
        "Name": "Jane Doe",
        "Pay": 20000.0
    }
]
```

#### Serialize `boost::numeric::ublas::matrix<double>` to a JSON formatted stream

```c++
#include <boost/numeric/ublas/matrix.hpp>
#include <jsoncons/json_stream_traits.hpp>
#include <sstream>

using boost::numeric::ublas::matrix;

namespace jsoncons
{
    template <>
    struct json_stream_traits<matrix<double>>
    {
        static void encode(const matrix<double>& val, json_output_handler& handler)
        {
            handler.begin_array();
            for (size_t i = 0; i < val.size1(); ++i)
            {
                handler.begin_array();
                for (size_t j = 0; j < val.size2(); ++j)
                {
                    handler.double_value(val(i, j),0);
                }
                handler.end_array();
            }
            handler.end_array();
        }
    };
};

using namespace jsoncons;

int main()
{
    std::ostringstream oss;

    matrix<double> A(2, 2);
    A(0, 0) = 1;
    A(0, 1) = 2;
    A(1, 0) = 3;
    A(1, 1) = 4;

    dump(A,oss,true);

    std::cout << oss.str() << std::endl;
}
```
Output:
```json
[
    [1.0,2.0],
    [3.0,4.0]
]
```


