### jsoncons::uri

```cpp
#include <jsoncons/utility/uri.hpp>

```
The class `uri` represents a Uniform Resource Identifier (URI) reference.

#### Constructor

    uri();

    uri(const std::string& s);
Constructs a URI by parsing the given string.

    uri(const std::string& uri);
Constructs a URI by parsing the given string.

    uri(jsoncons::string_view scheme,
        jsoncons::string_view userinfo,
        jsoncons::string_view host,
        jsoncons::string_view port,
        jsoncons::string_view path,
        jsoncons::string_view query,
        jsoncons::string_view fragment)
Constructs a URI from the given non-encoded parts.

    uri(const uri& other);

    uri(uri&& other) noexcept;

#### Assignment
    
    uri& operator=(const uri& other);
    
    uri& operator=(uri&& other) noexcept;

#### Member functions

    jsoncons::string_view scheme() const noexcept;
Returns the scheme part of this URI. The scheme is the first part of the URI, before the : character.

    std::string userinfo() const;
Returns the decoded userinfo part of this URI.

    jsoncons::string_view encoded_userinfo() const noexcept;
Returns the encoded userinfo part of this URI.

    jsoncons::string_view host() const noexcept;
Returns the host part of this URI.

    jsoncons::string_view port() const noexcept;
Returns the port number of this URI.

    std::string authority() const;
Returns the decoded authority part of this URI.

    jsoncons::string_view encoded_authority() const noexcept;
Returns the encoded authority part of this URI.

    std::string path() const;
Returns the decoded path part of this URI.

    jsoncons::string_view encoded_path() const noexcept;
Returns the encoded path part of this URI.

    std::string query() const;
Returns the decoded query part of this URI.

    jsoncons::string_view encoded_query() const noexcept;
Returns the encoded query part of this URI.

    std::string fragment() const;
Returns the decoded fragment part of this URI.

    jsoncons::string_view encoded_fragment() const noexcept;
Returns the encoded fragment part of this URI.

    bool is_absolute() const noexcept;
Returns true if this URI is absolute, false otherwise. An absolute URI is a URI that has
a scheme part (the part after the colon), e.g. 'https://john.doe@www.example.com'.

    bool is_opaque() const noexcept; 
Returns true if this URI is opaque, false otherwise. An opaque URI is an absolute URI whose 
scheme-specific part does not begin with a slash character ('/'), e.g. 'mailto:john.doe@example.com'.

    uri base() const noexcept; 

    uri resolve(const uri& base) const;

    const std::string& string() const;

### Examples
  

