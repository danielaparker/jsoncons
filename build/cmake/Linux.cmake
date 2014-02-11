#
# Global Configuration for linux platform
#

#
# GNU libstdc++ runtime is not supported because not yet C++11 compliant
#

# customize compiler flags
## Add new flags
add_definitions (-std=c++11 -pthread)
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  add_definitions (-stdlib=libc++)
endif()

set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
set (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
  set (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
endif()
