### jsoncons::filtered_staj_reader

```c++
typedef basic_filtered_staj_reader<char> filtered_staj_reader
```

#### Header
```c++
#include <jsoncons/staj_reader.hpp>
```

### Implemented interfaces

[staj_reader](staj_reader.md)

The `filtered_staj_reader` interface supports forward, read-only, access to JSON and JSON-like data formats.

The `filtered_staj_reader` is designed to iterate over stream events until `done()` returns `true`.
The `next()` function causes the reader to advance to the next stream event that satisfies the condition of a [staj_filter](staj_filter.md). The `current()` function
returns the current stream event. The data can be accessed using the [staj_event](staj_event.md) 
interface. When `next()` is called, copies of data previously accessed may be invalidated.

### Examples

The example JSON text, `book_catalog.json`, is used in the example below.

```json
[ 
  { 
      "author" : "Haruki Murakami",
      "title" : "Hard-Boiled Wonderland and the End of the World",
      "isbn" : "0679743464",
      "publisher" : "Vintage",
      "date" : "1993-03-02",
      "price": 18.90
  },
  { 
      "author" : "Graham Greene",
      "title" : "The Comedians",
      "isbn" : "0099478374",
      "publisher" : "Vintage Classics",
      "date" : "2005-09-21",
      "price": 15.74
  }
]
```

#### Filtering a JSON stream

```c++
#include <jsoncons/json_cursor.hpp>
#include <string>
#include <fstream>

using namespace jsoncons;

class author_filter : public staj_filter
{
    bool accept_next_ = false;
public:
    bool accept(const staj_event& event, const ser_context&) override
    {
        if (event.event_type()  == staj_event_type::name &&
            event.as<jsoncons::string_view>() == "author")
        {
            accept_next_ = true;
            return false;
        }
        else if (accept_next_)
        {
            accept_next_ = false;
            return true;
        }
        else
        {
            accept_next_ = false;
            return false;
        }
    }
};

int main()
{
    std::ifstream is("book_catalog.json");
    json_cursor cursor(is);

    author_filter filter;
    filtered_staj_reader reader(cursor, filter);

    for (; !reader.done(); reader.next())
    {
        const auto& event = reader.current();
        switch (event.event_type())
        {
            case staj_event_type::string_value:
                std::cout << event.as<jsoncons::string_view>() << "\n";
                break;
        }
    }
}
```
Output:
```
Haruki Murakami
Graham Greene
```

#### See also

- [staj_filter](staj_filter.md) 
- [staj_reader](staj_reader.md) 
- [json_cursor](json_cursor.md)

