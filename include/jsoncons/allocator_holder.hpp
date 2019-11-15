// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_ALLOCATOR_HOLDER_HPP
#define JSONCONS_ALLOCATOR_HOLDER_HPP

namespace jsoncons {

template <class Allocator>
class allocator_holder
{
public:
    typedef Allocator allocator_type;
private:
    allocator_type allocator_;
public:
    allocator_holder()
        : allocator_()
    {
    }
    allocator_holder(const allocator_type& allocator)
        : allocator_(allocator)
    {
    }

    allocator_type get_allocator() const
    {
        return allocator_;
    }
};

}

#endif
