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

    iterator begin() noexcept;
    const_iterator begin() const noexcept;    (since 0.173.3)
    const_iterator cbegin();                  (until 0.173.3)
    const_iterator cbegin() const noexcept;   (since 0.173.3)
Returns an iterator to the beginning

    iterator end() noexcept; 
    const_iterator end() const noexcept;    (since 0.173.3)
    const_iterator cend();                  (until 0.173.3)
    const_iterator cend() const noexcept;   (since 0.173.3)
Returns an iterator to the end

    reverse_iterator rbegin() noexcept;
    const_reverse_iterator rbegin() const noexcept;  (since 0.173.3)
    const_reverse_iterator crbegin();                (until 0.173.3)
    const_reverse_iterator crbegin() const noexcept; (since 0.173.3)
Returns a reverse iterator to the beginning

    reverse_iterator rend() noexcept;
    const_reverse_iterator rend() const noexcept;    (since 0.173.3)
    const_reverse_iterator crend();                  (until 0.173.3)
    const_reverse_iterator crend() const noexcept;   (since 0.173.3)
Returns a reverse iterator to the end
