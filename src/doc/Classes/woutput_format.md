    jsoncons::woutput_format

    typedef basic_output_format<wchar_t> woutput_format

The woutput_format class is an instantiation of the basic_output_format class template that uses `wchar_t` as the character type.

### Header

    #include "jsoncons/output_format.hpp"

### Interface

The interface is the same as [output_format](output_format), substituting wide character instantiations of classes - `std::wstring`, etc. - for narrow character ones.
