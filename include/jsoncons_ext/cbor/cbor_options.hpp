// Copyright 2019 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CBOR_CBOR_OPTIONS_HPP
#define JSONCONS_CBOR_CBOR_OPTIONS_HPP

#include <string>
#include <limits> // std::numeric_limits
#include <cwchar>
#include <jsoncons/json_exception.hpp>
#include <jsoncons_ext/cbor/cbor_detail.hpp>

namespace jsoncons { namespace cbor {

class cbor_encode_options
{
protected:
    bool use_stringref_;
    bool enable_typed_arrays_;
public:
    cbor_encode_options()
        : use_stringref_(false),
          enable_typed_arrays_(false)
    {
    }

    bool pack_strings() const 
    {
        return use_stringref_;
    }

    bool enable_typed_arrays() const 
    {
        return enable_typed_arrays_;
    }
};

class cbor_options final : public cbor_encode_options
{
public:
    using cbor_encode_options::pack_strings;
    using cbor_encode_options::enable_typed_arrays;

    cbor_options& pack_strings(bool value)
    {
        this->use_stringref_ = value;
        return *this;
    }

    cbor_options& enable_typed_arrays(bool value)
    {
        this->enable_typed_arrays_ = value;
        return *this;
    }
};

}}
#endif
