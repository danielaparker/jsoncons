#
# Global Configuration for linux platform
#

#
# GNU libstdc++ runtime is not supported because not yet C++11 compliant
#

# customize compiler flags
## Add new flags
add_definitions (-std=c++11 -stdlib=libc++ -pthread)

set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -pthread")
set (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -pthread")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -pthread")
