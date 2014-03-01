    jsoncons::wjson

The `wjson` class is an instantiation of the `basic_json` class template that uses `wchar_t` as the character type.

### Header

    #include "jsoncons/json.hpp"

### Member types

    member_type

[member_type](wjson_member_type) stores a name and a wjson value

    object

    array

    custom_type
Type tags that can be used with `is<T>` and `as<T>`

    object_iterator
A random access iterator to `wjson::member_type`

    const_object_iterator
A random access iterator to `const wjson::member_type`

    array_iterator
A random access iterator to `wjson`

    const_array_iterator
A random access iterator to `const wjson`

### Member constants

    an_object
Empty constant wjson object value

    an_array
Empty constant wjson array value

    null
Constant wjson null value

### Static member functions

    static wjson parse(std::wistream& is)
Parses an input stream of JSON text and returns a wjson object or array value

    static wjson parse_string(const std::wstring& s)
Parses a string of JSON text and returns a wjson object or array value

    static wjson parse_file(const std::string& filename)
Parses JSON text from a file opened as a binary stream, and returns a wjson object or array value.

    template <typename N>
    static wjson make_multi_array(size1 ... sizeN)

    template <typename N>
    static wjson make_multi_array(size1 ... sizeN, const wjson& val)
Makes a multidimensional array with the number of dimensions specified as a template parameter. The size of each dimension is passed as a parameter, and optionally an inital value. If no initial value, the default is an empty wjson object. The elements may be accessed using familiar C++ native array syntax.

### Constructors

    explicit wjson()
Constructs an empty wjson object. 

    wjson(const wjson& val)
Constructs a copy of val

    wjson(wjson&& val)
Acquires the contents of val, leaving val a `null` value

    wjson(double val)
Constructs a `double` value

    wjson(int val)
Constructs a `integer` value

    wjson(unsigned int val)
Constructs a `unsigned integer` value

    wjson(long val)
Constructs a `integer` value

    wjson(unsigned long val)
Constructs a `unsigned integer` value

    wjson(long long val)
Constructs a `integer` value

    wjson(unsigned long long val)
Constructs a `unsigned integer` value

    wjson(const std::wstring& val)
Constructs a `string` value

    wjson(char val)
Constructs a `string` value that has one character

    wjson(bool val)
Constructs a `true` or `false` value

    wjson(jsoncons::null_type)
Constructs a `null` value

    template <class InputIterator>
    wjson(InputIterator first, InputIterator last)

Constructs a wjson array with the elements in the range [first,last).

### Destructor

    ~wjson()
Destroys all values and deletes all memory allocated for strings, arrays, and objects.

### Assignment operator

    template <class T>
    wjson& operator=(T rhs)

    wjson& operator=(wjson rhs)
Supports copy and move assignment

### Iterators

    object_iterator begin_members()
    const_object_iterator begin_members() const
Returns an iterator to the first member of the wjson object,
provided this value is a wjson object, otherwise throws a `json_exception`

    object_iterator end_members()
    const_object_iterator end_members() const
Returns an iterator to one-past the last member of the wjson object
provided this value is a wjson object, otherwise throws a `json_exception`

    array_iterator begin_elements()
    const_array_iterator begin_elements() const
Returns an iterator to the first element of the wjson array
provided this value is a wjson array, otherwise throws a `json_exception`

    array_iterator end_elements()
    const_array_iterator end_elements() const
Returns an iterator to one-past the last element of the wjson array
provided this value is a wjson array, otherwise throws a `json_exception`

### Capacity

    size_t size() const
Returns the number of elements in a wjson array, or the number of members in a wjson object, or `zero`

    bool is_empty() const
Returns `true` if a wjson string, object or array has no elements, otherwise `false`.

    size_t capacity() const
Returns the size of the storage space currently allocated for a wjson object or array.

    void reserve(size_t n)
Increases the capacity of a wjson object or array to allow at least `n` members or elements. 

    void resize_array(size_t n)
Resizes a wjson array so that it contains `n` elements. 

    void resize_array(size_t n, const wjson& val)
Resizes a wjson array so that it contains `n` elements that are initialized to `val`. 

### Accessors

    bool has_member(const std::wstring& name) const
Returns `true` if a wjson object has a member named `name`, otherwise `false`.    

    template <typename T>
    bool is() const
Returns `true` if wjson value is of type `T`, `false` otherwise.  
`is<short>`, `is<unsigned short>`, `is<int>`, `is<unsigned int>`, `is<long>`, `is<unsigned long>`, `is<long long>`, `is<unsigned long long>` all return `true` if wjson value is of integral type and within the range of the template type, `false` otherwise.  
`is<float>` and `is<double>` returns true if the wjson value is of floating point type and within the range of the template type, `false` otherwise.  
`is<std::wstring>` returns `true` if the wjson value is of string type, `false` otherwise.  
`is<bool>` returns `true` if the wjson value is of boolean type, `false` otherwise.  
`is<jsoncons::null_type>` returns `true` if the wjson value is null, `false` otherwise.  
`is<wjson::object>` returns `true` if the wjson value is an object, `false` otherwise.  
`is<wjson::array>` returns `true` if the wjson value is an array, `false` otherwise.  
`is<wjson::custom_type>` returns `true` if the wjson value is a custom type, `false` otherwise.
`is<wjson::std::vector<T>>` returns `true` if the wjson value is an array and each element is of type `T`, `false` otherwise.

    bool is_null() const

    bool is_string() const

    bool is_numeric() const

    bool is_longlong() const

    bool is_ulonglong() const

    bool is_double() const

    bool is_bool() const

    bool is_object() const

    bool is_array() const

    bool is_custom() const

    wjson& operator[](size_t i)
    const wjson& operator[](size_t i) const
Returns a reference to the value at position i in a wjson object or array.

    wjson& operator[](const std::wstring& name)
Returns a proxy to a keyed value. If written to, inserts or updates with the new value. If read, evaluates to a reference to the keyed value, if it exists, otherwise throws. 

    const wjson& operator[](const std::wstring& name) const
If `name` matches the name of a member in the wjson object, returns a reference to the wjson object, otherwise throws.

    wjson& at(const std::wstring& name)
    const wjson& at(const std::wstring& name) const
If `name` matches the name of a member in the wjson object, returns a reference to the wjson object, otherwise throws.  These have the same behavior as the corresponding `operator[]` functions, but the non-const `at` is more efficient (doesn't have to return a proxy.)

    wjson& at(size_t i)
    const wjson& at(size_t i) const
Returns a reference to the element at position `i` in a wjson array.  These have the same behavior as the corresponding `operator[]` functions.

    const wjson& get(const std::wstring& name) const
If `name` matches the name of a member in the wjson object, returns a const reference to the wjson object, otherwise returns a const reference to `wjson::null`.

    const wjson get(const std::wstring& name, const wjson& default_val) const
If `name` matches the name of a member in the wjson object, returns a copy of the wjson object, otherwise returns a copy of `default_val`.

    template <typename T>
    T as() const
Attempts to coerce the wjson value to the template type

    bool as_bool() const

    double as_double() const
If value is double, returns value, if value is signed or unsigned integer, casts to double, if value is `null`, returns `NaN`, otherwise throws.

    long long as_longlong() const
If value is long long, returns value, if value is unsigned long long or double, casts to long long, if value is bool, returns 1 if true and 0 if false, otherwise throws.

    unsigned long long as_ulonglong() const

    std::wstring as_string() const
If value is string, returns value, otherwise returns result of `to_string`.

    template <class T>
    const T& custom_data() const
If the value is custom, returns a reference to the custom value, otherwise throws  

    template <class T>
    T& custom_data() 
If the value is custom, returns a reference to the custom value, otherwise throws  

### Modifiers

    void clear()
Remove all elements from an array or object value, otherwise do nothing

    void remove_range(size_t from_index, size_t to_index)
Removes all elements from an array value whose index is between `from_index`, the first element to be removed, and `to_index`, one after the last element to be removed.

    void remove_member(const std::wstring& name)
Remove a member from a `wjson` object

    template <typename T>
    void set(const std::wstring& name, T val)

    void set(const std::wstring& name, const wjson& val)
    void set(std::wstring&& name, wjson&& val)
Inserts a new member or replaces an existing member in a wjson object.

    template <class T>
    void set_custom_data(const std::wstring& name, const T& value)
Inserts custom data of type `T` into a wjson object.

    template <typename T>
    void add(T val)

    void add(const wjson& val)
    void add(wjson&& val)
Adds a new element at the end of a wjson array. The content of `val` is copied (or moved) to the new element.

    void add(size_t index, const wjson& val)
    void add(size_t index, wjson&& val)
Adds a new element at the specified index of a wjson array, shifting all elements currently at or above that index to the right.
The content of `val` is copied (or moved) to the new element.

    template <class T>
    void add_custom_data(const T& val)
Adds a new custom data element of type `T at the end of a wjson array. The content of `val` is copied to the new element.

    template <class T>
    void add_custom_data(size_t index, const T& val)
Adds a new custom data element of type `T` at the specified index of a wjson array, shifting all elements currently at or above that index to the right.
The content of `val` is copied to the new element.

    void swap(wjson& val)
Exchanges the content of the `wjson` value with the content of `val`, which is another `wjson` value.

### Relational operators

    bool operator==(const wjson& rhs) const
Returns `true` if two wjson objects compare equal, `false` otherwise.

    bool operator!=(const wjson& rhs) const
Returns `true` if two wjson objects do not compare equal, `false` otherwise.

### Serialization

    std::wstring to_string() const
Inserts wjson value into string.

    std::wstring to_string(const woutput_format& format) const
Inserts wjson value into string using specified [woutput_format](woutput_format).

    std::wostream& to_stream(std::wostream& os) const
Inserts wjson value into stream with default output format.

    std::wostream& to_stream(std::wostream<Char> os, const woutput_format& format) const
Inserts wjson value into stream using specified [woutput_format](woutput_format).

    void to_stream(json_output_handler& handler) const
Reports JSON related events for JSON objects, arrays, object members and array elements to a [wjson_output_handler](basic_json_output_handler), such as a [wjson_serializer](json_serializer).

### Non-member overloads

    std::wostream& operator<< (std::wostream& os, const wjson& val)
Inserts wjson value into stream.

    void swap(wjson& a, wjson& b)
Exchanges the values of `a` and `b`

### Examples

### Accessors and defaults

    wjson obj;

    obj[L"field1"] = 1;
    obj[L"field3"] = L"Toronto";

    double x1 = obj.has_member(L"field1") ? obj[L"field1"].as<double>() : 10.0;
    double x2 = obj.has_member(L"field2") ? obj[L"field2"].as<double>() : 20.0;

    std::wstring x3 = obj.get(L"field3",L"Montreal").as<std::wstring>();
    std::wstring x4 = obj.get(L"field4",L"San Francisco").as<std::wstring>();

    std::wcout << "x1=" << x1 << L"\n";
    std::wcout << "x2=" << x2 << L"\n";
    std::wcout << "x3=" << x3 << L"\n";
    std::wcout << "x4=" << x4 << L"\n";

The output is

    x1=1
    x2=20
    x3=Toronto
    x4=San Francisco

### Nulls

    wjson obj;
    obj[L"field1"] = json::null;
    std::wcout << obj << L"\n";

The output is 

    {L"field1"L:null}

### Array

    wjson arr(wjson::an_array);
    arr.add(10);
    arr.add(20);
    arr.add(30);

    std::wcout << arr << L"\n";

The output is 

    [10,20,30]

### Array from std::vector

    std::vector<int> v;
    v.push_back(10);
    v.push_back(20);
    v.push_back(30);

    wjson arr(v.begin(),v.end());

    std::wcout << arr << L"\n";

The output is 

    [10,20,30]

### Object iterator

    wjson obj;
    obj[L"city"] = L"Toronto";
    obj[L"province"] = L"Ontario";
    obj[L"country"] = L"Canada";

    for (auto it = obj.begin_members(); it != obj.end_members(); ++it)
    {
        std::wcout << it->name() << "=" << it->value().as<std::wstring>() << L"\n";
    }

The output is

    city=Toronto
    country=Canada
    province=Ontario

### Array iterator

    wjson arr(wjson::an_array);
    arr.add(L"Toronto");
    arr.add(L"Vancouver");
    arr.add(L"Montreal");

    for (auto it = arr.begin_elements(); it != arr.end_elements(); ++it)
    {
        std::wcout << it->as<std::wstring>() << L"\n";
    }

The output is

    Toronto
    Vancouver 
    Montreal

### Constructing wjson structures

	wjson root;

    root["persons"] = wjson::an_array;

    wjson person;
    person[L"first_name"] = L"John";
    person[L"last_name"] = L"Smith";
    person[L"birth_date"] = L"1972-01-30";
    
    wjson address;
    address[L"city"] = L"Toronto";
    address[L"country"] = L"Canada";
    
    person["address"] = std::move(address);

    root["persons"].add(std::move(person));

    std::wcout << pretty_print(root) << L"\n";

The output is

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

### Default NaN, inf and -inf replacement

    wjson obj;
    obj["field1"] = std::sqrt(-1.0);
    obj["field2"] = 1.79e308*1000;
    obj["field3"] = -1.79e308*1000;
    std::wcout << obj << L"\n";

The output is

    {"field1":null,"field2":null,"field3":null}

### Custom NaN, inf and -inf replacement

    wjson obj;
    obj["field1"] = std::sqrt(-1.0);
    obj["field2"] = 1.79e308*1000;
    obj["field3"] = -1.79e308*1000;

    woutput_format format;
    format.nan_replacement(L"null");
    format.pos_inf_replacement(L"1e9999");
    format.neg_inf_replacement(L"-1e9999");

    obj.to_stream(std::wcout,format);
    std::wcout << L"\n";

The output is

    {"field1":null,"field2":1e9999,"field3":-1e9999}

### Suppressing NaN, inf and -inf replacement

    wjson obj;
    obj["field1"] = std::sqrt(-1.0);
    obj["field2"] = 1.79e308*1000;
    obj["field3"] = -1.79e308*1000;

    woutput_format format;
    format.replace_nan(false);
    format.replace_inf(false);

    obj.to_stream(std::wcout,format);
    std::wcout << L"\n";

The (illegal) wjson output produced by Visual Studio 2010 is

    {"field1":-1.#IND,"field2":1.#INF,"field3":-1.#INF}

