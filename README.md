jsoncons
========

The jsoncons library implements C++ classes for the construction of JavaScript Object Notation (JSON). 

The library is header-only: it consists solely of header files containing templates and inline functions, and requires no separately-compiled library binaries when linking. It has no dependence on other libraries.

It has been tested with MS Visual C++ 10, SP1

Release 0.12
------------

Implemented operator>> for json::proxy

Added to_stream methods to json::proxy

Release 0.11
------------

Added members to json_parser to access and modify the buffer capacity

Added checks when parsing integer values to determine overflow for 
long long and unsigned long long, and if overflow, parse them as
doubles.


