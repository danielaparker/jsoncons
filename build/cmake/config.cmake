
# set compiler on some platforms
if (UNIX)
  if (NOT CMAKE_C_COMPILER)
	set (CMAKE_C_COMPILER clang)
  endif()
  if (NOT CMAKE_CXX_COMPILER)
	set (CMAKE_CXX_COMPILER clang++)
  endif()
endif()
