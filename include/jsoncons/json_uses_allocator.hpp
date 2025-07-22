#ifndef JSONCONS_JSON_USES_ALLOCATOR_HPP
#define JSONCONS_JSON_USES_ALLOCATOR_HPP

#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/config/jsoncons_config.hpp>

#include <memory> 
#include <utility>

namespace jsoncons {

template <typename Json, typename Alloc, typename... Args>
typename std::enable_if<std::uses_allocator<Json,Alloc>::value,Json>::type
make_json_using_allocator(const Alloc& alloc, Args&&... args)
{
    return Json(std::forward<Args>(args)..., alloc);
}

template <typename Json, typename Alloc, typename... Args>
typename std::enable_if<!std::uses_allocator<Json,Alloc>::value,Json>::type
make_json_using_allocator(const Alloc&, Args&&... args)
{
    return Json(std::forward<Args>(args)...);
}

} // namespace jsoncons

#endif // JSONCONS_JSON_USES_ALLOCATOR_HPP

