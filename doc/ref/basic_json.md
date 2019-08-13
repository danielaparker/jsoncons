### jsoncons::basic_json

```c++
#include <jsoncons/json.hpp>

template< 
    class CharT,
    class ImplementationPolicy = sorted_policy,
    class Allocator = std::allocator<char>
> class basic_json;
```

The class `basic_json` resembles a union. A `basic_json` holds a data item of one of its alternative types:
null, `bool`, `int64_t`, `uint64_t`, `double`, text string, byte string, array, and object. The data item may be tagged with a 
[semantic_tag](semantic_tag.md) that provides additional information about the data item.

When assigned a new `basic_json` value, the old value is overwritten. The type of the new value may be different from the old value. 

The definition of the character type of text strings is supplied via the `CharT` template parameter.
Implementation policies for arrays and objects are provided via the `ImplementationPolicy` template parameter.
A custom allocator may be supplied with the `Allocator` template parameter, which a `basic_json` will
rebind to internal data structures. 

Several typedefs for common character types and policies for ordering an object's name/value pairs are provided:

Type                |Definition
--------------------|------------------------------
[json](json.md)     |`basic_json<char,sorted_policy,std::allocator<char>>`
[ojson](ojson.md)   |`basic_json<char, preserve_order_policy, std::allocator<char>>`
[wjson](wjson.md)   |`basic_json<wchar_t,sorted_policy,std::allocator<char>>`
[wojson](wojson.md) |`basic_json<wchar_t, preserve_order_policy, std::allocator<char>>`

Member type                         |Definition
------------------------------------|------------------------------
`char_type`|CharT
`implementation_policy'|ImplementationPolicy
`allocator_type`|Allocator
`char_traits_type`|`std::char_traits<char_type>`
`char_allocator_type`|`allocator_type` rebound to `char_type`
`reference`|`basic_json&`
`const_reference`|`const basic_json&`
`pointer`|`basic_json*`
`const_pointer`|`const basic_json*`
`string_view_type`|`basic_string_view<char_type>`
`key_type`|std::basic_string<char_type,char_traits_type,char_allocator_type>
`key_value_type`|`key_value<key_type,basic_json>`
`object`|`json_object<key_type,basic_json>`
`array`|`json_array<basic_json>`
`object_iterator`|A [RandomAccessIterator](http://en.cppreference.com/w/cpp/concept/RandomAccessIterator) to [key_value_type](json/key_value.md)
`const_object_iterator`|A const [RandomAccessIterator](http://en.cppreference.com/w/cpp/concept/RandomAccessIterator) to const [key_value_type](json/key_value.md)
`array_iterator`|A [RandomAccessIterator](http://en.cppreference.com/w/cpp/concept/RandomAccessIterator) to `basic_json`
`const_array_iterator`|A const [RandomAccessIterator](http://en.cppreference.com/w/cpp/concept/RandomAccessIterator) to `const basic_json`
`proxy_type`|proxy<basic_json>. The `proxy_type` class supports conversion to `basic_json&`.

### Static member functions

<table border="0">
  <tr>
    <td><a href="json/parse.md">parse</a></td>
    <td>Parses JSON.</td> 
  </tr>
  <tr>
    <td><a href="json/make_array.md">make_array</a></td>
    <td>Makes a multidimensional basic_json array.</td> 
  </tr>
  <tr>
    <td><a>const basic_json& null()</a></td>
    <td>Returns a null value</td> 
  </tr>
</table>

### Member functions
<table border="0">
  <tr>
    <td><a href="json/constructor.md">(constructor)</a></td>
    <td>constructs the basic_json value</td> 
  </tr>
  <tr>
    <td><a href="json/destructor.md">(destructor)</a></td>
    <td>destructs the basic_json value</td> 
  </tr>
  <tr>
    <td><a href="json/operator=.md">operator=</a></td>
    <td>assigns values</td> 
  </tr>
</table>

    allocator_type get_allocator() const
Returns the allocator associated with the basic_json value.

#### Ranges and Iterators

<table border="0">
  <tr>
    <td><a href="json/array_range.md">array_range</a></td>
    <td>Returns a "range" that supports a range-based for loop over the elements of a `basic_json` array.</td> 
  </tr>
  <tr>
    <td><a href="json/object_range.md">obect_range</a></td>
    <td>Returns a "range" that supports a range-based for loop over the key-value pairs of a `basic_json` object.</td> 
  </tr>
</table>

#### Capacity

<table border="0">
  <tr>
    <td><a>size_t size() const noexcept</a></td>
    <td>Returns the number of elements in a basic_json array, or the number of members in a basic_json object, or `zero`</td> 
  </tr>
  <tr>
    <td><a>bool empty() const noexcept</a></td>
    <td>Returns <code>true</code> if a basic_json string, object or array has no elements, otherwise <code>false</code></td> 
  </tr>
  <tr>
    <td><a>size_t capacity() const</a></td>
    <td>Returns the size of the storage space currently allocated for a basic_json object or array</td> 
  </tr>
  <tr>
    <td><a>void reserve(size_t n)</a></td>
    <td>Increases the capacity of a basic_json object or array to allow at least `n` members or elements</td> 
  </tr>
  <tr>
    <td><a>void resize(size_t n)</a></td>
    <td>Resizes a basic_json array so that it contains `n` elements</td> 
  </tr>
  <tr>
    <td><a>void resize(size_t n, const basic_json& val)</a></td>
    <td>Resizes a basic_json array so that it contains `n` elements that are initialized to `val`</td> 
  </tr>
  <tr>
    <td><a>void shrink_to_fit()</a></td>
    <td>Requests the removal of unused capacity</td> 
  </tr>
</table>

#### Accessors

<table border="0">
  <tr>
    <td><code>bool contains(const string_view_type& key) const</code></td>
    <td>Returns <code>true</code> if an object has a member with the given `key`, otherwise <code>false</code></td> 
  </tr>
  <tr>
    <td><a href="json/is.md">is</a></td>
    <td>Checks if a basic_json value matches a type.</td> 
  </tr>
  <tr>
    <td><a href="json/as.md">as</a></td>
    <td>Attempts to convert a basic_json value to a value of a type.</td> 
  </tr>
  <tr>
    <td><a href="json/operator_at.md">operator[]</a></td>
    <td>Access or insert specified element.</td> 
  </tr>
  <tr>
    <td><a href="json/at.md">at</a></td>
    <td>Access specified element.</td> 
  </tr>
</table>

    semantic_tag tag() const
Returns the [semantic_tag](semantic_tag.md) associated with this value
 
    object_iterator find(const string_view_type& name)
    const_object_iterator find(const string_view_type& name) const
Returns an object iterator to a member whose name compares equal to `name`. If there is no such member, returns `object_range.end()`.
Throws `std::runtime_error` if not an object.  

    template <class T>
    T get_with_default(const string_view_type& name, 
                       const T& default_val) const
If `name` matches the name of a member in the basic_json object, returns the member value converted to the default's data type, otherwise returns `default_val`.
Throws `std::runtime_error` if not an object.

    template <class T = std::string>
    T get_with_default(const string_view_type& name, 
                       const char_type* default_val) const
Make `get_with_default` do the right thing for string literals. 
Throws `std::runtime_error` if not an object. 

#### Modifiers

<table border="0">
  <tr>
    <td><a>void clear()</a></td>
    <td>Remove all elements from an array or members from an object, otherwise do nothing</td> 
  </tr>
  <tr>
    <td><a href="json/erase.md">erase</a></td>
    <td>Erases array elements and object members</td> 
  </tr>
  <tr>
    <td><a href="json/push_back.md">push_back</a></td>
    <td>Adds a value to the end of a basic_json array</td> 
  </tr>
  <tr>
    <td><a href="json/insert.md">insert</a></td>
    <td>Inserts elements</td> 
  </tr>
  <tr>
    <td><a href="json/emplace_back.md">emplace_back</a></td>
    <td>Constructs a value in place at the end of a basic_json array</td> 
  </tr>
  <tr>
    <td><a href="json/emplace.md">emplace</a></td>
    <td>Constructs a value in place before a specified position in a basic_json array</td> 
  </tr>
  <tr>
    <td><a href="json/try_emplace.md">try_emplace</a></td>
    <td>Constructs a key-value pair in place in a basic_json object if the key does not exist, does nothing if the key exists</td> 
  </tr>
  <tr>
    <td><a href="json/insert_or_assign.md">insert_or_assign</a></td>
    <td>Inserts a key-value pair in a basic_json object if the key does not exist, or assigns a new value if the key already exists</td> 
  </tr>
  <tr>
    <td><a href="json/merge.md">merge</a></td>
    <td>Inserts another basic_json object's key-value pairs into a basic_json object, if they don't already exist.</td>
  </tr>
  <tr>
    <td><a href="json/merge_or_update.md">merge_or_update</a></td>
    <td>Inserts another basic_json object's key-value pairs into a basic_json object, or assigns them if they already exist.</td>
  </tr>
  <tr>
    <td><a>void swap(basic_json& val)</a></td>
    <td>Exchanges the content of the <code>basic_json</code> value with the content of <code>val</code>, which is another <code>basic_json</code> value</td>
  </tr>
</table>

#### Serialization

<table border="0">
  <tr>
    <td><a href="json/dump.md"</a>dump</td>
    <td>Serializes basic_json value to a string, stream, or output handler.</td> 
  </tr>
</table>

#### Non member functions

<table border="0">
  <tr>
    <td><code>bool operator==(const basic_json& lhs, const basic_json& rhs)</code></td>
    <td>Returns <code>true</true> if two basic_json objects compare equal, <code>false</true> otherwise.</td> 
  </tr>
  <tr>
    <td><code>bool operator!=(const basic_json& lhs, const basic_json& rhs)</code></td>
    <td>Returns <code>true</true> if two basic_json objects do not compare equal, <code>false</true> otherwise.</td> 
  </tr>
  <tr>
    <td><code>bool operator<(const basic_json& lhs, const basic_json& rhs)</code></td>
    <td>Compares the contents of lhs and rhs lexicographically.</td> 
  </tr>
  <tr>
    <td><code>bool operator<=(const basic_json& lhs, const basic_json& rhs)</code></td>
    <td>Compares the contents of lhs and rhs lexicographically.</td> 
  </tr>
  <tr>
    <td><code>bool operator>(const basic_json& lhs, const basic_json& rhs)</code></td>
    <td>Compares the contents of lhs and rhs lexicographically.</td> 
  </tr>
  <tr>
    <td><code>bool operator>=(const basic_json& lhs, const basic_json& rhs)</code></td>
    <td>Compares the contents of lhs and rhs lexicographically.</td> 
  </tr>
</table>

    std::basic_istream<char_type>& operator>>(std::basic_istream<char_type>& is, basic_json& o)
Reads a `basic_json` value from a stream.

    std::basic_ostream<char_type>& operator<<(std::basic_ostream<char_type>& os, const basic_json& o)
Inserts basic_json value into stream.

    std::basic_ostream<char_type>& print(const basic_json& val)  
    std::basic_ostream<char_type>& print(const basic_json& val, const basic_json_options<CharT>& options)  
Inserts basic_json value into stream using the specified [basic_json_options](basic_json_options.md) if supplied.

    std::basic_ostream<char_type>& pretty_print(const basic_json& val)  
    std::basic_ostream<char_type>& pretty_print(const basic_json& val, const basic_json_options<CharT>& options)  
Inserts basic_json value into stream using the specified [basic_json_options](basic_json_options.md) if supplied.

    void swap(basic_json& a, basic_json& b)
Exchanges the values of `a` and `b`

