// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONPOINTER_FLATTEN_HPP
#define JSONCONS_JSONPOINTER_FLATTEN_HPP

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer_error.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

namespace jsoncons { namespace jsonpointer {

template<class J>
void flatten(const std::basic_string<typename J::char_type>& reference_string,
             const J& value,
             J& result)
    {
        switch (value.type())
        {
            case json_type::array_value:
            {
                if (value.empty())
                {
                    // flatten empty array as null
                    result[reference_string] = J::null();
                }
                else
                {
                    // iterate array and use index as reference string
                    for (std::size_t i = 0; i < value.size(); ++i)
                    {
                        flatten(reference_string + "/" + std::to_string(i),
                                value.at(i), result);
                    }
                }
                break;
            }

            case json_value::object:
            {
                if (value.empty())
                {
                    // flatten empty object as null
                    result[reference_string] = J::null();
                }
                else
                {
                    // iterate object and use keys as reference string
                    for (const auto& element : value.object_range())
                    {
                        flatten(reference_string + "/" + escape(element.first), element.second, result);
                    }
                }
                break;
            }

            default:
            {
                // add primitive value with its reference string
                result[reference_string] = value;
                break;
            }
        }
    }
#if 0
    /*!
    @param[in] value  flattened JSON
    @return unflattened JSON
    @throw parse_error.109 if array index is not a number
    @throw type_error.314  if value is not an object
    @throw type_error.315  if object values are not primitive
    @throw type_error.313  if value cannot be unflattened
    */
    static J
    unflatten(const J& value)
    {
        if (JSON_HEDLEY_UNLIKELY(not value.is_object()))
        {
            JSON_THROW(detail::type_error::create(314, "only objects can be unflattened"));
        }

        J result;

        // iterate the JSON object values
        for (const auto& element : *value.m_value.object)
        {
            if (JSON_HEDLEY_UNLIKELY(not element.second.is_primitive()))
            {
                JSON_THROW(detail::type_error::create(315, "values in object must be primitive"));
            }

            // assign value to reference pointed to by JSON pointer; Note that if
            // the JSON pointer is "" (i.e., points to the whole value), function
            // get_and_create returns a reference to result itself. An assignment
            // will then create a primitive value.
            json_pointer(element.first).get_and_create(result) = element.second;
        }

        return result;
    }
#endif
}}

#endif
