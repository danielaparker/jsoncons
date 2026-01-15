// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_TOON_TOON_OPTIONS_HPP
#define JSONCONS_TOON_TOON_OPTIONS_HPP

#include <cstdint>
#include <cwchar>
#include <functional>
#include <string>
#include <system_error>

#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/json_error.hpp>
#include <jsoncons/ser_util.hpp>

namespace jsoncons {
namespace toon {

class toon_options;

class toon_options_common
{
    friend class toon_options;
public:
    using char_type = char;
    using string_type = std::string;
private:

    int indent_{2};
    char delimiter_{','};
    jsoncons::optional<char> length_marker_;
    int max_nesting_depth_{1024};

protected:
    toon_options_common() = default;

    virtual ~toon_options_common() = default;

    toon_options_common(const toon_options_common&) = default;
    toon_options_common& operator=(const toon_options_common&) = default;
    toon_options_common(toon_options_common&&) = default;
    //toon_options_common& operator=(toon_options_common&&) = default;

public:

    int indent() const
    {
        return indent_;
    }

    char delimiter() const
    {
        return delimiter_;
    }

    jsoncons::optional<char> length_marker() const
    {
        return length_marker_;
    }

    int max_nesting_depth() const 
    {
        return max_nesting_depth_;
    }
};

class toon_decode_options : public virtual toon_options_common
{
    friend class toon_options;
    using super_type = toon_options_common;
public:
    using typename super_type::char_type;
    using typename super_type::string_type;
private:
public:
    toon_decode_options() = default;

    toon_decode_options(const toon_decode_options&) = default;

    toon_decode_options(toon_decode_options&& other) noexcept
        : super_type(std::move(other))
    {
    }
protected:
    toon_decode_options& operator=(const toon_decode_options&) = default;
    toon_decode_options& operator=(toon_decode_options&&) = default;
public:
}; 

class toon_encode_options : public virtual toon_options_common
{
    friend class toon_options;
    using super_type = toon_options_common;
public:
    using typename super_type::char_type;
    using typename super_type::string_type;
private:
public:
    toon_encode_options() = default;

    toon_encode_options(const toon_encode_options&) = default;

    toon_encode_options(toon_encode_options&& other) noexcept
        : super_type(std::move(other))
    {
    }
    
    ~toon_encode_options() = default;
protected:
    toon_encode_options& operator=(const toon_encode_options&) = default;
    toon_encode_options& operator=(toon_encode_options&&) = default;
public:
};

class toon_options final: public toon_decode_options, 
                                public toon_encode_options
{
public:
    using char_type = char;
    using string_type = std::string;

    using toon_options_common::indent;
    using toon_options_common::delimiter;
    using toon_options_common::length_marker;
    using toon_options_common::max_nesting_depth;
public:

//  Constructors

    toon_options() = default;
    toon_options(const toon_options&) = default;
    toon_options(toon_options&&) = default;
    toon_options& operator=(const toon_options&) = default;
    toon_options& operator=(toon_options&&) = default;

    toon_options& indent(int value)
    {
        this->indent_ = value;
        return *this;
    }

    toon_options& delimiter(char value)
    {
        this->delimiter_ = value;
        return *this;
    }

    toon_options& length_marker(jsoncons::optional<char> value)
    {
        this->length_marker_ = value;
        return *this;
    }

    toon_options& max_nesting_depth(int value)
    {
        this->max_nesting_depth_ = value;
        return *this;
    }
};

} // namespace toon
} // namespace jsoncons

#endif // JSONCONS_TOON_TOON_OPTIONS_HPP
