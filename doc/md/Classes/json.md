```c++
jsoncons::json

typedef basic_json<char,
                   JsonTraits = json_traits<char>,
                   Allocator = std::allocator<char>> json
```
The `json` class is an instantiation of the `basic_json` class template that uses `char` as the character type. The order of an object's name/value pairs is not preserved, they are sorted alphabetically by name. If you want to preserve the original insertion order, use [ojson](ojson) instead.

The `jsoncons` library will always rebind the supplied allocator from the template parameter to internal data structures.

### Header
```c++
#include <jsoncons/json.hpp>
```

### Member types

Member type                         |Definition
------------------------------------|------------------------------
`json_type`|json type
`allocator_type`|Allocator type
`char_allocator`|String allocator type
`array_allocator`|Array allocator type
`object_allocator`|Object allocator 
`string_view_type`|A non-owning view of a string, holds a pointer to character data and length. Supports conversion to and from strings. Will be typedefed to the C++ 17 [string view](http://en.cppreference.com/w/cpp/string/basic_string_view) if `JSONCONS_HAS_STRING_VIEW` is defined in `jsoncons_config.hpp`, otherwise proxied.  
`key_value_pair_type`|[key_value_pair_type](key_value_pair_type) is a class that stores a name and a json value
`null_type`|An alias for `jsoncons::null_type`
`object`|json object type
`array`|json array type
`object_iterator`|A [RandomAccessIterator](http://en.cppreference.com/w/cpp/concept/RandomAccessIterator) to [key_value_pair_type](key_value_pair_type)
`const_object_iterator`|A const [RandomAccessIterator](http://en.cppreference.com/w/cpp/concept/RandomAccessIterator) to const [key_value_pair_type](key_value_pair_type)
`array_iterator`|A [RandomAccessIterator](http://en.cppreference.com/w/cpp/concept/RandomAccessIterator) to `json`
`const_array_iterator`|A const [RandomAccessIterator](http://en.cppreference.com/w/cpp/concept/RandomAccessIterator) to `const json`

### Static member functions

    static json parse(string_view_type s)
    static json parse(string_view_type s, 
                      parse_error_handler& err_handler)
Parses a string of JSON text and returns a json object or array value. 
Throws [parse_error](parse_error) if parsing fails.

    static json parse(std::istream& is)
    static json parse(std::istream& is, 
                      parse_error_handler& err_handler)
Parses an input stream of JSON text and returns a json object or array value. 
Throws [parse_error](parse_error) if parsing fails.

```c++
template <class T>
static json make_array(size_ n, const T& val)

template <class T>
static json make_array(size_ n, const T& val, 
                       const allocator_type& alloc = allocator_type())

template <size_t N>
static json make_array(size_t size1 ... size_t sizeN)

template <size_t N,typename T>
static json make_array(size_t size1 ... size_t sizeN, const T& val)

template <size_t N,typename T>
static json make_array(size_t size1 ... size_t sizeN, const T& val, 
                       const allocator_type& alloc)
```
Makes a multidimensional array with the number of dimensions specified as a template parameter. The size of each dimension is passed as a parameter, and optionally an inital value. If no initial value, the default is an empty json object. The elements may be accessed using familiar C++ native array syntax.

    static const json& null()
Returns a null value        

### Constructors

    json()
Constructs an empty json object. 

    json(const allocator_type& allocator)
Constructs a json object. 

    json(std::initializer_list<json> list, const allocator_type& allocator)
Constructs a json array with the elements of the initializer-list `init`. 

    json(const json& val)
Constructs a copy of val

    json(const json& val, const allocator_type& allocator)
Copy with allocator

    json(json&& val) noexcept
Acquires the contents of val, leaving val a `null` value

    json(json&& val, const allocator_type& allocator) 
Move with allocator

    template <class T>
    json(const T& val, const allocator_type& allocator=allocator_type())
Constructs a `json` value for types supported in [json_type_traits](json_type_traits).

    template <class T>
    json(const CharT* val, const allocator_type& allocator=allocator_type())
Constructs a `json` value for string literals.

    template <class InputIterator>
    json(InputIterator first, InputIterator last, 
         const allocator_type& allocator=allocator_type())

Constructs a json array with the elements in the range [first,last).

### Destructor

    ~json()
Destroys all values and deletes all memory allocated for strings, arrays, and objects.

### Assignment operator

    json& operator=(const json& rhs)
    json& operator=(json&& rhs) noexcept
Assigns a new `json` value to a `json` variable, replacing it's current contents.

    template <class T>
    json& operator=(const T& rhs)
Assigns the templated value to a `json` variable using [json_type_traits](json_type_traits).

    json& operator=(const char_type* rhs)

### Ranges and Iterators

    range<object_iterator> object_range();  
    range<const_object_iterator> object_range() const;  
Returns a "range" defined by `begin()` and `end()` over the members of a `json` object      
Throws `std::runtime_error` if not an object.

    range<array_iterator> array_range();
    range<const_array_iterator> array_range() const;
Returns a "range" defined by `begin()` and `end()` over the elements of a `json` array      
Throws `std::runtime_error` if not an array.

### Capacity

    size_t size() const noexcept
Returns the number of elements in a json array, or the number of members in a json object, or `zero`

    bool empty() const noexcept
Returns `true` if a json string, object or array has no elements, otherwise `false`.

    size_t capacity() const
Returns the size of the storage space currently allocated for a json object or array.

    void reserve(size_t n)
Increases the capacity of a json object or array to allow at least `n` members or elements. 

    void resize(size_t n)
Resizes a json array so that it contains `n` elements. 

    void resize(size_t n, const json& val)
Resizes a json array so that it contains `n` elements that are initialized to `val`. 

### Accessors

    bool has_key(string_view_type name) const
Returns `true` if an object has a member with the given `name`, otherwise `false`.    

    size_t count(string_view_type name) const
Returns the number of object members that match `name`.    

    template <class T>
    bool is() const noexcept
Returns `true` if the json value is the same as type `T` according to [json_type_traits](json_type_traits), `false` otherwise.  

    bool is<X> const noexcept 
Type `X` is integral: returns `true` if the json value is integral and within the range of the type `X`, `false` otherwise.  
Type `X` is floating point: returns `true` if the json value is floating point and within the range of the type `X`, `false` otherwise.  

    bool is<std::string> const noexcept 
Returns `true` if the json value is of string type, `false` otherwise.  

    bool is<bool> const noexcept 
Returns `true` if the json value is of boolean type, `false` otherwise.  

    bool is<json::null_type> const noexcept
Returns `true` if the json value is null, `false` otherwise.  

    bool is<json::object> const noexcept 
Returns `true` if the json value is an object, `false` otherwise.  

    bool is<json::array> const noexcept 
Returns `true` if the json value is an array, `false` otherwise.  

    bool is<X<T>> const noexcept
If the type `X` is not `std::basic_string` but otherwise satisfies [SequenceContainer](http://en.cppreference.com/w/cpp/concept/SequenceContainer), `is<X<T>>()` returns `true` if the json value is an array and each element is the "same as" type `T` according to [json_type_traits](json_type_traits), `false` otherwise.

    bool is<X<std::string,T>> const noexcept
If the type 'X' satisfies [AssociativeContainer](http://en.cppreference.com/w/cpp/concept/AssociativeContainer) or [UnorderedAssociativeContainer](http://en.cppreference.com/w/cpp/concept/UnorderedAssociativeContainer), `is<X<T>>()` returns `true` if the json value is an object and each mapped value is the "same as" `T` according to [json_type_traits](json_type_traits), `false` otherwise.

    bool is_null() const noexcept
    bool is_string() const noexcept
    bool is_number() const noexcept
    bool is_integer() const noexcept
    bool is_uinteger() const noexcept
    bool is_double() const noexcept
    bool is_bool() const noexcept
    bool is_object() const noexcept
    bool is_array() const noexcept
Non-generic versions of `is_` methods

    template <class T>
    T as() const
Attempts to convert the json value to the template value type using [json_type_traits](json_type_traits).

    X as<X>() const
Type X is integeral: returns integer value if value is integral, performs cast if value has double type, returns 1 or 0 if value has bool type, attempts conversion if value is string, otherwise throws.
Type X is floating point: returns value cast to X if value is integral, returns `NaN` if value is `null`, attempts conversion if value is string, otherwise throws.

    as<bool>()
Returns `true` if value is `bool` and `true`, or if value is integral and non-zero, or if value is floating point and non-zero, or if value is string and parsed value evaluates as `true`. 
Returns `false` if value is `bool` and `false`, or if value is integral and zero, or if value is floating point and zero, or if value is string and parsed value evaluates as `false`. 
Otherwise throws `std::runtime_exception`

    std::string as<std::string>() const noexcept
    std::string as<std::string>(const char_allocator& allocator) const noexcept
If value is string, returns value, otherwise returns result of `dump`.

    as<X<T>>()
If the type `X` is not `std::basic_string` but otherwise satisfies [SequenceContainer](http://en.cppreference.com/w/cpp/concept/SequenceContainer), `as<X<T>>()` returns the `json` value as an `X<T>` if the `json` value is an array and each element is convertible to type `T`, otherwise throws.

    as<X<std::string,T>>()
If the type 'X' satisfies [AssociativeContainer](http://en.cppreference.com/w/cpp/concept/AssociativeContainer) or [UnorderedAssociativeContainer](http://en.cppreference.com/w/cpp/concept/UnorderedAssociativeContainer), `as<X<std::string,T>>()` returns the `json` value as an `X<std::string,T>` if the `json` value is an object and if each member value is convertible to type `T`, otherwise throws.

    bool as_bool() const
    int64_t as_integer() const
    uint64_t as_uinteger() const
    double as_double() const
    string_view_type as_string_view() const
    unsigned int as<unsigned int> const 
Non-generic versions of `as` methods

    json& operator[](size_t i)
    const json& operator[](size_t i) const
Returns a reference to the value at position i in a json object or array.
Throws `std::runtime_error` if not an object or array.

    json& operator[](string_view_type name)
Returns a proxy to a keyed value. If written to, inserts or updates with the new value. If read, evaluates to a reference to the keyed value, if it exists, otherwise throws. 
Throws `std::runtime_error` if not an object.
If read, throws `std::out_of_range` if the object does not have a member with the specified name.  

    const json& operator[](string_view_type name) const
If `name` matches the name of a member in the json object, returns a reference to the json object, otherwise throws.
Throws `std::runtime_error` if not an object.
Throws `std::out_of_range` if the object does not have a member with the specified name.  

    object_iterator find(string_view_type name)
    const_object_iterator find(string_view_type name) const
Returns an object iterator to a member whose name compares equal to `name`. If there is no such member, returns `end_member()`.
Throws `std::runtime_error` if not an object.

    json& at(string_view_type name)
    const json& at(string_view_type name) const
Returns a reference to the value with the specifed name in a json object.
Throws `std::runtime_error` if not an object.
Throws `std::out_of_range` if the object does not have a member with the specified name.  

    json& at(size_t i)
    const json& at(size_t i) const
Returns a reference to the element at index `i` in a json array.  
Throws `std::runtime_error` if not an array.
Throws `std::out_of_range` if the index is outside the bounds of the array.  

    template <class T>
    T get_with_default(string_view_type name, 
                       const T& default_val) const
If `name` matches the name of a member in the json object, returns the member value converted to the default's data type, otherwise returns `default_val`.
Throws `std::runtime_error` if not an object.

    const char_type* get_with_default(string_view_type name, 
                                      const char_type* default_val) const
Make `get_with_default` do the right thing for string literals.

### Modifiers

    void clear()
Remove all elements from an array or object value, otherwise do nothing

    void erase(array_iterator first, array_iterator last)
Remove the elements from an array in the range '[first,last)'.
Throws `std::runtime_error` if not an object.

    void erase(object_iterator first, object_iterator last)
Remove the members from an object in the range '[first,last)'.
Throws `std::runtime_error` if not an object.

    void erase(string_view_type name)
Remove a member with the specified name from an object
Throws `std::runtime_error` if not an object.

    void shrink_to_fit()
Requests the removal of unused capacity.

    template <class T>
    void set(string_view_type name, T&& val)
Inserts a new member or replaces an existing member in a json object.
Throws `std::runtime_error` if not an object.

    template <class T>
    object_iterator set(object_iterator hint, string_view_type name, T&& val)
Inserts a new member or replaces an existing member in a json object.
Insertion time is optimized if `hint` points to the member that will precede the inserted member.
Returns a `member_iterator` pointing at the member that was inserted or updated
Throws `std::runtime_error` if not an object.

    template <class T>
    void add(T&& val)
Adds a new json element at the end of a json array. The argument `val` is forwarded to the `json` constructor as `std::forward<T>(val)`.
Throws `std::runtime_error` if not an array.

    template <class T>
    array_iterator add(const_array_iterator pos, T&& val)
Adds a new json element at the specified position of a json array, shifting all elements currently at or above that position to the right.
The argument `val` is forwarded to the `json` constructor as `std::forward<T>(val)`.
Returns an `array_iterator` that points to the new value
Throws `std::runtime_error` if not an array.

    void swap(json& val)
Exchanges the content of the `json` value with the content of `val`, which is another `json` value.

[emplace_back](json_emplace_back)
Constructs a value in place at the end of a json array

[emplace](json_emplace)
Constructs a value in place before a specified position in a json array

[try_emplace](json_try_emplace)
Constructs a key-value pair in place in a json object if the key does not exist, does nothing if the key exists

[insert_or_assign](json_insert_or_assign)
Inserts a key-value pair in a json object if the key does not exist, or assigns a new value if the key already exists 

### Relational operators

    bool operator==(const json& rhs) const
Returns `true` if two json objects compare equal, `false` otherwise.

    bool operator!=(const json& rhs) const
Returns `true` if two json objects do not compare equal, `false` otherwise.

### Serialization

    template <class SAllocator>
    void dump(std::basic_string<char_type,char_traits_type,SAllocator>& s) const
Inserts json value into string using default serialization_options.

    template <class SAllocator>
    void dump(std::basic_string<char_type,char_traits_type,SAllocator>& s, 
              const serialization_options& options) const
Inserts json value into string using specified [serialization_options](serialization_options).

    void dump(basic_json_output_handler<char_type>& output_handler) const
Calls `begin_json()` on `output_handler`, emits json value to `output_handler`, and calls `end_json()` on `output_handler`.

    void dump(std::ostream& os) const
Inserts json value into stream with default serialization options.

    void dump(std::ostream<CharT> os, const serialization_options& options) const
Inserts json value into stream using specified [serialization_options](serialization_options).

    void dump_body(json_output_handler& handler) const
Emits JSON events for JSON objects, arrays, object members and array elements to a [json_output_handler](json_output_handler), such as a [json_serializer](json_serializer).

### Non member functions

    std::istream& operator>> (std::istream& os, json& val)
Reads a `json` value from a stream.

    std::ostream& operator<< (std::ostream& os, const json& val)
Inserts json value into stream.

    std::ostream& print(const json& val)  
    std::ostream& print(const json& val, const serialization_options<CharT>& options)  
Inserts json value into stream using the specified [serialization_options](serialization_options) if supplied.

    std::ostream& pretty_print(const json& val)  
    std::ostream& pretty_print(const json& val, const serialization_options<CharT>& options)  
Inserts json value into stream using the specified [serialization_options](serialization_options) if supplied.

    void swap(json& a, json& b)
Exchanges the values of `a` and `b`

### Deprecated names

As the `jsoncons` library has evolved, names have sometimes changed. To ease transition, jsoncons deprecates the old names but continues to support many of them. See the [deprecated list](deprecated) for the status of old names. The deprecated names can be suppressed by defining macro JSONCONS_NO_DEPRECATED, which is recommended for new code.

### See also

- [ojson](ojson) constructs a json value that preserves the original name-value insertion order

- [wjson](wjson) constructs a wide character json value that sorts name-value members alphabetically

- [wojson](wojson) constructs a wide character json value that preserves the original name-value insertion order

## Examples

### is and as
```c++
    json j = json::parse(R"(
    {
        "k1" : 2147483647,
        "k2" : 2147483648,
        "k3" : -10,
        "k4" : 10.5,
        "k5" : true,
        "k6" : "10.5"
    }
    )");

    std::cout << std::boolalpha << "(1) " << j["k1"].is<int32_t>() << '\n';
    std::cout << std::boolalpha << "(2) " << j["k2"].is<int32_t>() << '\n';
    std::cout << std::boolalpha << "(3) " << j["k2"].is<long long>() << '\n';
    std::cout << std::boolalpha << "(4) " << j["k3"].is<signed char>() << '\n';
    std::cout << std::boolalpha << "(5) " << j["k3"].is<uint32_t>() << '\n';
    std::cout << std::boolalpha << "(6) " << j["k4"].is<int32_t>() << '\n';
    std::cout << std::boolalpha << "(7) " << j["k4"].is<double>() << '\n';
    std::cout << std::boolalpha << "(8) " << j["k5"].is<int>() << '\n';
    std::cout << std::boolalpha << "(9) " << j["k5"].is<bool>() << '\n';
    std::cout << std::boolalpha << "(10) " << j["k6"].is<double>() << '\n';
    std::cout << '\n';
    std::cout << "(1) " << j["k1"].as<int32_t>() << '\n';
    std::cout << "(2) " << j["k2"].as<int32_t>() << '\n';
    std::cout << "(3) " << j["k2"].as<long long>() << '\n';
    std::cout << "(4) " << j["k3"].as<signed char>() << '\n';
    std::cout << "(5) " << j["k3"].as<uint32_t>() << '\n';
    std::cout << "(6) " << j["k4"].as<int32_t>() << '\n';
    std::cout << "(7) " << j["k4"].as<double>() << '\n';
    std::cout << std::boolalpha << "(8) " << j["k5"].as<int>() << '\n';
    std::cout << std::boolalpha << "(9) " << j["k5"].as<bool>() << '\n';
    std::cout << "(10) " << j["k6"].as<double>() << '\n';
```
Output:
```
(1) true
(2) false
(3) true
(4) true
(5) false
(6) false
(7) true
(8) false
(9) true
(10) false

(1) 2147483647
(2) -2147483648
(3) 2147483648
(4) ÷
(5) 4294967286
(6) 10
(7) 10.5
(8) 1
(9) true
(10) 10.5
```

### Range-based for loop over members of an object
```c++
json book = json::parse(R"(
{
    "category" : "Fiction",
    "title" : "Pulp",
    "author" : "Charles Bukowski",
    "date" : "2004-07-08",
    "price" : 22.48,
    "isbn" : "1852272007"  
}
)");

for (const auto& kvp: book.object_range())
{
    std::cout << kvp.key() << ":" << kvp.value().as<string>() << std::endl;
} 
```
### Range-based for loop over elements of an array
```c++
json book1;    
book1["category"] = "Fiction";
book1["title"] = "A Wild Sheep Chase: A Novel";
book1["author"] = "Haruki Murakami";

json book2;    
book2["category"] = "History";
book2["title"] = "Charlie Wilson's War";
book2["author"] = "George Crile";

json book3;    
book3["category"] = "Fiction";
book3["title"] = "Kafka on the Shore";
book3["author"] = "Haruki Murakami";

// Constructing a json array with an initializer-list 
json booklist = json::array{book1, book2, book3};    

for (const auto& book: booklist.array_range())
{
    std::cout << book["title"].as<std::string>() << std::end;
} 
   
### Accessors and defaults
```c++
json val;

val["field1"] = 1;
val["field3"] = "Toronto";

double x1 = obj.has_key("field1") ? val["field1"].as<double>() : 10.0;
double x2 = obj.has_key("field2") ? val["field2"].as<double>() : 20.0;

std::string x3 = obj.get_with_default("field3","Montreal");
std::string x4 = obj.get_with_default("field4","San Francisco");

std::cout << "x1=" << x1 << '\n';
std::cout << "x2=" << x2 << '\n';
std::cout << "x3=" << x3 << '\n';
std::cout << "x4=" << x4 << '\n';
```
Output:
```c++
x1=1
x2=20
x3=Toronto
x4=San Francisco
```
### Nulls
```c++
json obj;
obj["field1"] = json::null();
std::cout << obj << std::endl;
```
Output: 
```json
{"field1":null}
```
### Array
```c++
json arr = json::array();
arr.add(10);
arr.add(20);
arr.add(30);

std::cout << arr << std::endl;
```
Output: 
```json
[10,20,30]
```
### Array from std::vector
```c++
std::vector<int> v;
v.push_back(10);
v.push_back(20);
v.push_back(30);

json arr(v.begin(),v.end());

std::cout << arr << std::endl;
```
Output: 
```c++
[10,20,30]
```
### Object iterator
```c++
json obj;
obj["city"] = "Toronto";
obj["province"] = "Ontario";
obj["country"] = "Canada";

for (auto it = obj.object_range().begin(); it != obj.object_range().end(); ++it)
{
    std::cout << it->key() << "=" << it->value().as<std::string>() << std::endl;
}
```
Output:
```c++
city=Toronto
country=Canada
province=Ontario
```
### Array iterator
```c++
json arr = json::array{"Toronto", "Vancouver", "Montreal"};

for (auto it = arr.array_range().begin(); it != arr.array_range().end(); ++it)
{
    std::cout << it->as<std::string>() << std::endl;
}
```
Output:
```json
Toronto
Vancouver 
Montreal
```
### Constructing json structures
```c++
json root;

root["persons"] = json::array();

json person;
person["first_name"] = "John";
person["last_name"] = "Smith";
person["birth_date"] = "1972-01-30";

json address;
address["city"] = "Toronto";
address["country"] = "Canada";

person["address"] = std::move(address);

root["persons"].add(std::move(person));

std::cout << pretty_print(root) << std::endl;
```
Output:
```c++
{
    "persons":
    [
        {
            "address":
            {
                "city":"Toronto",
                "country":"Canada"
            },
            "birth_date":"1972-01-30",
            "first_name":"John",
            "last_name":"Smith"
        }
    ]
}
```
