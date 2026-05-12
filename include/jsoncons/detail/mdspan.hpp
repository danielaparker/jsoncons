// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_DETAIL_MDSPAN_HPP
#define JSONCONS_DETAIL_MDSPAN_HPP

#include <array>
#include <cstddef>
#include <iterator>
#include <limits>
#include <memory> // std::addressof
#include <type_traits> // std::enable_if, std::true_type, std::false_type

#include <jsoncons/utility/more_type_traits.hpp>

namespace jsoncons {
namespace detail {

template< class IndexType, std::size_t Rank >
class dextents
{
public:
    using index_type = IndexType;
    std::size_t rank = Rank;
};

struct layout_left
{
};

struct layout_right
{
};

template< typename T, typename Extents, typename LayoutPolicy = layout_right>
class mdspan
{
};

} // namespace detail
} // namespace jsoncons

#endif // JSONCONS_DETAIL_MDSPAN_HPP
