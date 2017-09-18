#
# jsoncons tests suite CMake file
#

cmake_minimum_required (VERSION 2.8)

# load global config
include (../../../build/cmake/config.cmake)

project (Tests CXX)

# load per-platform configuration
include (../../../build/cmake/${CMAKE_SYSTEM_NAME}.cmake)

# set (Boost_NO_BOOST_CMAKE ON)

if (APPLE1 OR WIN32)
    set(Boost_USE_STATIC_LIBS TRUE)
endif()

find_package (Boost COMPONENTS filesystem chrono thread date_time unit_test_framework system REQUIRED)

file(GLOB_RECURSE UnitTests_sources ../../src/*.cpp)

if (APPLE1 OR WIN321)
elseif()
    ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK) 
endif()

add_executable(jsoncons_tests
    ${UnitTests_sources}
)

if (APPLE OR WIN32)
target_compile_definitions (jsoncons_tests PUBLIC)
elseif()
target_compile_definitions (jsoncons_tests PUBLIC BOOST_ALL_DYN_LINK)
endif()

if (NO_DEPRECATED)
add_definitions(-DJSONCONS_NO_DEPRECATED)
endif()

target_include_directories (jsoncons_tests PUBLIC ${Boost_INCLUDE_DIRS}
                                           PUBLIC ../../../include
                                           PRIVATE ../../include)

target_link_libraries (jsoncons_tests ${Boost_LIBRARIES})

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux" AND ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  # special link option on Linux because llvm stl rely on GNU stl
  target_link_libraries (jsoncons_tests -Wl,-lstdc++)
endif()
