### jsoncons::uri

```cpp
#include <jsoncons/utility/uri.hpp>

```
The class `uri` represents a Uniform Resource Identifier (URI) reference.

#### Constructor

    uri();

    explicit uri(const std::string& str);
Constructs a `uri` by parsing the given string.

    uri(jsoncons::string_view scheme,
        jsoncons::string_view userinfo,
        jsoncons::string_view host,
        jsoncons::string_view port,
        jsoncons::string_view path,
        jsoncons::string_view query = "",
        jsoncons::string_view fragment = "")
Constructs a `uri` from the given unescaped parts.
 
    uri(const uri& other, uri_fragment_part_t, jsoncons::string_view fragment);
Constructs a `uri` from `other`, replacing it's fragment part with (unescaped) `fragment`.

    uri(const uri& other);
Copy constructor.

    uri(uri&& other) noexcept;
Move constructor.

#### Assignment
    
    uri& operator=(const uri& other);
Copy assignment.
    
    uri& operator=(uri&& other) noexcept;
Move assignment.

#### Member functions

    jsoncons::string_view scheme() const noexcept;
Returns the scheme part of this URI. The scheme is the first part of the URI, before the `:` character.

    std::string userinfo() const;
Returns the decoded userinfo part of this URI.

    jsoncons::string_view raw_userinfo() const noexcept;
Returns the encoded userinfo part of this URI.

    jsoncons::string_view host() const noexcept;
Returns the host part of this URI.

    jsoncons::string_view port() const noexcept;
Returns the port number of this URI.

    std::string authority() const;
Returns the decoded authority part of this URI.

    jsoncons::string_view raw_authority() const noexcept;
Returns the encoded authority part of this URI.

    std::string path() const;
Returns the decoded path part of this URI.

    jsoncons::string_view raw_path() const noexcept;
Returns the encoded path part of this URI.

    std::string query() const;
Returns the decoded query part of this URI.

    jsoncons::string_view raw_query() const noexcept;
Returns the encoded query part of this URI.

    std::string fragment() const;
Returns the decoded fragment part of this URI.

    jsoncons::string_view raw_fragment() const noexcept;
Returns the encoded fragment part of this URI.

    bool is_absolute() const noexcept;
Returns true if this URI is absolute, false if it is relative.
An absolute URI has a scheme part.

    bool is_opaque() const noexcept;
Returns true if this URI is opaque, otherwise false.
An opaque URI is an absolute URI whose scheme-specific part does not begin with a slash character ('/').

    uri base() const noexcept;
Returns the base uri. The base uri includes the scheme, userinfo, host, port, and path parts,
but not the query or fragment.     

    bool has_scheme() const noexcept;
Returns true if this URI has a scheme part, otherwise false.

    bool has_userinfo() const noexcept;
Returns true if this URI has a userinfo part, otherwise false.

    bool has_authority() const noexcept;
Returns true if this URI has an authority part, otherwise false.

    bool has_host() const noexcept;
Returns true if this URI has a host part, otherwise false.

    bool has_port() const noexcept;
Returns true if this URI has a port number, otherwise false.

    bool has_path() const noexcept;
Returns true if this URI has a path part, otherwise false.

    bool has_query() const noexcept;
Returns true if this URI has a query part, otherwise false.

    bool has_fragment() const noexcept;
Returns true if this URI has a fragment part, otherwise false.

    uri resolve(const uri& reference) const;
Resolve `reference` as a URI relative to this URI.

    const std::string& string() const noexcept;
Returns a URI string.

    static uri parse(const std::string& str, std::error_code& ec);
Creates a `uri` by parsing the given string. If a parse error is
encountered, returns a default constructed `uri` and sets `ec`.

### Examples
  

