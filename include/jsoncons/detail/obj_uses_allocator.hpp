#ifndef JSONCONS_DETAIL_OBJ_USES_ALLOCATOR
#define JSONCONS_DETAIL_OBJ_USES_ALLOCATOR 

#include <new>			// for placement operator new
#include <tuple>		// for tuple, make_tuple, make_from_tuple
#include <utility>
#include <jsoncons/utility/more_type_traits.hpp>

namespace jsoncons {
namespace detail {

template <typename T, typename Alloc, typename... Args>
typename std::enable_if<!ext_traits::is_std_pair<T>::value && std::uses_allocator<T, Alloc>::value, T>::type
make_obj_using_allocator(const Alloc& alloc, Args&&... args)
{
    return T(std::forward<Args>(args)..., alloc);
}

template <typename T, typename Alloc, typename... Args>
typename std::enable_if<!std::uses_allocator<T, Alloc>::value, T>::type
make_obj_using_allocator(const Alloc&, Args&&... args)
{
    return T(std::forward<Args>(args)...);
}

} // namespace detail
} // namespace jsoncons

#endif // JSONCONS_DETAIL_OBJ_USES_ALLOCATOR
