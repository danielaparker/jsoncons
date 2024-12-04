### jsoncons::uri

```cpp
#include <jsoncons/utility/uri.hpp>

```
The class `uri` represents a Uniform Resource Identifier (URI) reference.

#### Constructor

    uri();

    uri(const std::string& uri);
Constructs a URI by parsing the given string.

    uri(jsoncons::string_view scheme,
        jsoncons::string_view userinfo,
        jsoncons::string_view host,
        jsoncons::string_view port,
        jsoncons::string_view path,
        jsoncons::string_view query,
        jsoncons::string_view fragment)
Constructs a URI from the given components.

    uri(const uri& other);

    uri(uri&& other) noexcept;

#### Assignment
    
    uri& operator=(const uri& other);
    
    uri& operator=(uri&& other) noexcept;

#### Member functions

    bool is_absolute() const noexcept;
Returns true if this URI is absolute, false otherwise. An absolute URI is a URI that has
a scheme part (the part after the colon), e.g. 'https://john.doe@www.example.com'.

    bool is_opaque() const noexcept; 
Returns true if this URI is opaque, false otherwise. An opaque URI is an absolute URI whose 
scheme-specific part does not begin with a slash character ('/'), e.g. 'mailto:john.doe@example.com'.

    uri base() const noexcept; 

    const std::string& string() const;

    jsoncons::string_view scheme() const noexcept;

    jsoncons::string_view scheme() const noexcept;

    jsoncons::string_view encoded_scheme() const noexcept;

    std::string userinfo() const;

    jsoncons::string_view encoded_userinfo() const noexcept;

    jsoncons::string_view host() const noexcept;

    jsoncons::string_view encoded_host() const noexcept;

    jsoncons::string_view port() const noexcept;

    jsoncons::string_view encoded_port() const noexcept;

    std::string authority() const;

    jsoncons::string_view encoded_authority() const noexcept;

    std::string path() const;

    jsoncons::string_view encoded_path() const noexcept;

    std::string query() const;

    jsoncons::string_view encoded_query() const noexcept;

    std::string query() const;

    jsoncons::string_view encoded_query() const noexcept;

    std::string fragment() const;

    jsoncons::string_view encoded_fragment() const noexcept;

    uri resolve(const uri& base) const;

### Examples
  

