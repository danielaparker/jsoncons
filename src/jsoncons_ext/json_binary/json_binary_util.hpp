// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_BINARY_JSON_BINARY_UTIL_HPP
#define JSONCONS_JSON_BINARY_JSON_BINARY_UTIL_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <memory>
#include <limits>

namespace jsoncons { namespace json_binary {

// to_big_endian

template<typename T, size_t Size>
struct to_big_endian
{
};

template<typename T>
struct to_big_endian<T, 1>
{
    inline void operator()(T val, std::vector<uint8_t>& v)
    {
        v.push_back(static_cast<uint8_t>((val) & 0xff));
    }
};

template<typename T>
struct to_big_endian<T, 2>
{
    inline void operator()(T val, std::vector<uint8_t>& v)
    {
        v.push_back(static_cast<uint8_t>((val >> 0x08) & 0xff));
        v.push_back(static_cast<uint8_t>((val) & 0xff));
    }
};

template<typename T>
struct to_big_endian<T, 4>
{
    inline void operator()(T val, std::vector<uint8_t>& v)
    {
        v.push_back(static_cast<uint8_t>((val >> 0x18) & 0xff));
        v.push_back(static_cast<uint8_t>((val >> 0x10) & 0xff));
        v.push_back(static_cast<uint8_t>((val >> 0x08) & 0xff));
        v.push_back(static_cast<uint8_t>((val) & 0xff));
    }
};

template<typename T>
struct to_big_endian<T, 8>
{
    inline void operator()(T val, std::vector<uint8_t>& v)
    {
        v.push_back(static_cast<uint8_t>((val >> 0x38) & 0xff));
        v.push_back(static_cast<uint8_t>((val >> 0x30) & 0xff));
        v.push_back(static_cast<uint8_t>((val >> 0x28) & 0xff));
        v.push_back(static_cast<uint8_t>((val >> 0x20) & 0xff));
        v.push_back(static_cast<uint8_t>((val >> 0x18) & 0xff));
        v.push_back(static_cast<uint8_t>((val >> 0x10) & 0xff));
        v.push_back(static_cast<uint8_t>((val >> 0x08) & 0xff));
        v.push_back(static_cast<uint8_t>((val) & 0xff));
    }
};

template<>
struct to_big_endian<float, 4>
{
    inline void operator()(float val, std::vector<uint8_t>& v)
    {
        to_big_endian<uint64_t, sizeof(uint32_t)>()(*(uint32_t*)&val, v);
    }
};

template<>
struct to_big_endian<double, 8>
{
    inline void operator()(double val, std::vector<uint8_t>& v)
    {
        to_big_endian<uint64_t, sizeof(uint64_t)>()(*(uint64_t*)&val, v);
    }
};

template<class T,class Iterator>
static T from_big_endian(Iterator& it, Iterator& end)
{
    if (it + sizeof(T) + 1 > end)
    {
        throw std::out_of_range("Failed attempting to read " + std::to_string(sizeof(T)) + " bytes from vector");
    }

    T result;
    uint8_t* ptr = reinterpret_cast<uint8_t*>(&result);
    for (size_t i = 0; i < sizeof(T); ++i)
    {
        *ptr++ = *(it+ sizeof(T) - i);
    }
    return result;
}

}}

#endif
