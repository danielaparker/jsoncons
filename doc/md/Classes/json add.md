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

