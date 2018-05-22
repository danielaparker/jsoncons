/// Copyright 2013-2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_SERIALIZING_CONTEXT_HPP
#define JSONCONS_SERIALIZING_CONTEXT_HPP

namespace jsoncons {

class serializing_context
{
public:
    virtual ~serializing_context() = default;

    size_t line_number() const
    {
        return do_line_number();
    }
    size_t column_number() const 
    {
        return do_column_number();
    }

private:
    virtual size_t do_line_number() const = 0;
    virtual size_t do_column_number() const = 0;
};

#if !defined(JSONCONS_NO_DEPRECATED)
typedef serializing_context parsing_context;
#endif

}
#endif
