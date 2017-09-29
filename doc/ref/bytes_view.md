### jsoncons::bytes_view

```c++
class bytes_view;
```

#### Header

    #include <jsoncons/jsoncons_utilities.hpp>

#### Member types

Member type     |Definition
----------------|------------------------------
value_type      | uint8_t
const_reference | const uint8_t&
const_iterator  | const uint8_t*
iterator        | const uint8_t*
size_type       | std::size_t

#### Constructors

    bytes_view(const uint8_t* data, size_t length);

#### Iterators

    const_iterator begin() const noexcept;

    const_iterator end() const noexcept;

#### Element access

    const_reference operator[](size_type pos) const;

    const uint8_t* data() const;

#### Capacity

    size_t length() const;  

    size_t size() const;

