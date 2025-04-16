// Copyright 2013-2024 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_UTILITY_FILEIO_HPP
#define JSONCONS_UTILITY_FILEIO_HPP

#include <jsoncons/views/jsoncons_config.hpp>
#include <cstdio>
#include <cstdlib>

#define YYJSON_FOPEN_EXT
#if !defined(_MSC_VER) && defined(__GLIBC__) && defined(__GLIBC_PREREQ)
#   if __GLIBC_PREREQ(2, 7)
#       undef YYJSON_FOPEN_EXT
#       define YYJSON_FOPEN_EXT "e" /* glibc extension to enable O_CLOEXEC */
#   endif
#endif

namespace jsoncons { namespace utility {

FILE *fopen_safe(const char *path, const char *mode);

FILE *fopen_readonly(const char *path); 

FILE *fopen_writeonly(const char *path);
 
std::size_t fread_safe(void *buf, std::size_t size, FILE *file);
 
} // namespace utility
} // namespace jsoncons

#endif
