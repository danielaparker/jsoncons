### jsoncons::range

```c++
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
    const_iterator cbegin();
Returns an iterator to the beginning

    iterator end();
    const_iterator cend();
Returns an iterator to the end

    reverse_iterator rbegin();
    const_reverse_iterator crbegin();
Returns a reverse iterator to the beginning

    reverse_iterator rend();
    const_reverse_iterator crend();
Returns a reverse iterator to the end
