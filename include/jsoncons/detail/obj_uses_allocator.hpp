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

// std::pair

template <typename T, typename Alloc>
typename std::enable_if<ext_traits::is_std_pair<T>::value, T>::type
make_obj_using_allocator(const Alloc& alloc)
{
    return std::make_pair(make_obj_using_allocator<typename T::first_type>(alloc), 
        make_obj_using_allocator<typename T::second_type>(alloc));
}

template <typename T, typename Alloc, typename U, typename V>
typename std::enable_if<ext_traits::is_std_pair<T>::value, T>::type
make_obj_using_allocator(const Alloc& alloc, U&& u, V&& v)
{
    return std::make_pair(make_obj_using_allocator<typename T::first_type>(alloc,std::forward<U>(u)), 
        make_obj_using_allocator<typename T::second_type>(alloc,std::forward<V>(v)));
}

} // namespace detail
} // namespace jsoncons

#endif // JSONCONS_DETAIL_OBJ_USES_ALLOCATOR
