
# set compiler on some platforms
if (APPLE)
  if (NOT CMAKE_CXX_COMPILER)
	set (CMAKE_CXX_COMPILER clang++)
  endif()
endif()
