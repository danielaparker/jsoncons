#include <string>
#include <vector>
#include <assert.h>
#include <algorithm>
#include "jsoncons/json_value.hpp"
#include "jsoncons/json_parser.hpp"

namespace jsoncons {

const json_value json_value::object_prototype(new json_object());
const json_value json_value::array_prototype(new json_array());
const json_value json_value::null_prototype(new json_null());

json_value json_value::parse(std::istream& is)
{
    json_parser parser;
    json_variant* object = parser.parse(is);
    return json_value(object);
}

json_value json_value::parse(const std::string& s)
{
    std::istringstream is(s);
    json_parser parser;
    json_variant* object = parser.parse(is);
    return json_value(object);
}

}

