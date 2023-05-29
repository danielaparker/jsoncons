// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_ALLOCATOR_STRATEGY_HPP
#define JSONCONS_ALLOCATOR_STRATEGY_HPP

#include <memory>

namespace jsoncons {


struct work_allocator_arg_t
{
    explicit work_allocator_arg_t() = default; 
};

constexpr work_allocator_arg_t work_allocator_arg{};

template <class ResultAllocator,class WorkAllocator>
class wrapped_allocators
{
    ResultAllocator result_alloc_;
    WorkAllocator work_alloc_;
public:
    using result_allocator_type = ResultAllocator;
    using work_allocator_type = WorkAllocator;

    wrapped_allocators(const ResultAllocator& result_alloc, const WorkAllocator& work_alloc)
        : result_alloc_(result_alloc), work_alloc_(work_alloc)
    {
    }

    wrapped_allocators(const wrapped_allocators&)  = default;
    wrapped_allocators(wrapped_allocators&&)  = default;
    wrapped_allocators& operator=(const wrapped_allocators&)  = delete;
    wrapped_allocators& operator=(wrapped_allocators&&)  = delete;
    ~wrapped_allocators() = default;

    ResultAllocator get_result_allocator() const {return result_alloc_;}
    WorkAllocator get_work_allocator() const {return work_alloc_;}
};

template <class ResultAllocator,class WorkAllocator>
wrapped_allocators<ResultAllocator,WorkAllocator> wrap_allocators(const ResultAllocator& result_alloc = std::allocator<char>(), 
    const WorkAllocator& work_alloc = std::allocator<char>())
{
    return wrapped_allocators<ResultAllocator,WorkAllocator>(result_alloc, work_alloc);
}

template <class WorkAllocator>
wrapped_allocators<std::allocator<char>,WorkAllocator> wrap_allocators(work_allocator_arg_t, 
    const WorkAllocator& work_alloc)
{
    return wrapped_allocators<std::allocator<char>,WorkAllocator>(std::allocator<char>(), work_alloc);
}

} // namespace jsoncons

#endif
