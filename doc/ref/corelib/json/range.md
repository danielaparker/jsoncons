### jsoncons::range

```cpp
#include <jsoncons/basic_json.hpp>

template <class IteratorT, class ConstIteratorT>
class range; 
```

Member type                         |Definition
------------------------------------|------------------------------
`iterator`|`IteratorT`
`const_iterator`|`ConstIteratorT`
`reverse_iterator`|`std::reverse_iterator<IteratorT>`
`const_reverse_iterator`|`std::reverse_iterator<ConstIteratorT>`

### Range access

    iterator begin();
    const_iterator begin() const noexcept;
    const_iterator cbegin() const noexcept;
Returns an iterator to the beginning

    iterator end();
    const_iterator end() const;
    const_iterator cend();
Returns an iterator to the end

    reverse_iterator rbegin();
    const_reverse_iterator rbegin() const noexcept;
    const_reverse_iterator crbegin() const noexcept;
Returns a reverse iterator to the beginning

    reverse_iterator rend();
    const_reverse_iterator rend() const noexcept;
    const_reverse_iterator crend() const noexcept;
Returns a reverse iterator to the end
