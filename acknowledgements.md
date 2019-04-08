A big thanks to the following individuals for contributing:

- Andrew Hutko (early code review)        

- Marc Chevrier (contributed clang port, build files, json is<T> and as<T> methods, 
and make_array template implementation.)

- Pedro Larroy and the developers of the clearskies_core project (contributed build 
system for posix systems, adding GCC to list of supported compilers, bug fixes, 
Android fix)

- [Cory Fields](https://github.com/theuni) for fixing warnings about unused variables

- Vitaliy Gusev (reported error in json object operator[size_t i])

- [Alex Merry](https://github.com/amerry) for reporting errors with "typename" keyword experienced with gcc and providing 
workaround for gcc 4.8 regex issues.

- Ignatov Serguei (reported issues experienced with gcc for 0.95 and 
0.96 candidate and helped fix them)

- [Milan Burda](https://github.com/miniak) for fix for clang build error

- [Peter Tissen](https://github.com/Bigpet), for reporting and suggesting a fix for get(name,default_val)

- Tom Bass for assistance with clang build errors

- Andrey Alifanov and Amit Naik for failing test cases for JSON Path

- [Yuri Plaksyuk](https://github.com/yplaksyuk) for contributing an extension to JsonPath to allow filter 
expressions over a single object. 

- [Nikolay Amiantov](https://github.com/abbradar) for fixing compilation errors and warnings by GCC and 
Clang, adding read support for std::array and, most appreciated,
adding Travis CI configuration.

- jakalx contributed fix for operator== throws when comparing a string 
against an empty object

- Alexander (rog13) for contributing fix to jsonpatch::diff

- Stefano Sinigardi for contributing workaround for vs2017 platform issue

- [xezon](https://github.com/danielaparker/jsoncons/pull/140) for proposing decode_csv and encode_csv functions, the
ignore_empty_lines option, and fixes to mismatched allocator types. Also for fixes and improvements in string_view code. 

- Vojtech Fried for contributing patches to JSONCONS_DEFINE_LITERAL 
and to json::as_string to remove warnings

- [Joshua Pritikin](https://github.com/jpritikin), for reporting gcc ubsan runtime warnings about 
load of misaligned addresses, and verifying fix

- [Tobias Hermann](https://github.com/Dobiasd), for reporting issue with `UINT_MAX` not declared 
in `bignum.hpp`, and proposing fix.

- [Cebtenzzre](https://github.com/Cebtenzzre), for finding and fixing an issue with conversions on 
a basic_json value leading to an infinite recursion when the 
value is a bignum, and for fixing undefined behavior in the bignum 
class. 

- Alexander B, for uncovering a bug in how json_parser validated
UTF-8 strings.

- [zhskyy](https://github.com/zhskyy), for contributing __FILE__ and __LINE__ macros removed 
from JSONCONS_ASSERT if not defined _DEBUG.

- [soberich](https://github.com/soberich), for contributing the jsonpath sum and prod functions,
and a proposal for aggregation functions that work outside a filter.

- [patternoia](https://github.com/patternoia) for fixing the installation script
to include copying the jsoncons_ext directory into the installation place
