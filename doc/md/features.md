Features:

- Uses the standard C++ input/output streams library
- Supports conversion from and to the standard library sequence containers, associative containers, and std::tuple
- Supports conversion from and to user defined types
- Passes all tests from [JSON_checker](http://www.json.org/JSON_checker/) except `fail1.json`, which is allowed in [RFC7159](http://www.ietf.org/rfc/rfc7159.txt)
- Returns the expected results for all tests from [JSONTestSuite](https://github.com/nst/JSONTestSuite)
- Supports object members sorted alphabetically by name or in original order
- Implements parsing and serializing JSON text in UTF-8 for narrow character strings and streams
- Supports UTF16 (UTF32) encodings with size 2 (size 4) wide characters
- Correctly handles surrogate pairs in \uXXXX escape sequences
- Supports event based JSON parsing and serializing with user defined input and output handlers
- Accepts and ignores single line comments that start with //, and multi line comments that start with /* and end with */
- Parses files with duplicate names but uses only the last entry
- Supports optional escaping of the solidus (/) character
- Supports Nan, Inf and -Inf replacement
- Supports reading a sequence of JSON texts from a stream
- Supports optional escaping of non-ascii UTF-8 octets
- Supports conversion from and to user defined types
- Handles JSON texts of arbitrarily large depth of nesting, a limit can be set if desired

