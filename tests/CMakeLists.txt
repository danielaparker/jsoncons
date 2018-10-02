cmake_minimum_required(VERSION 3.4)

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    project(jsoncons-test)

    find_package(jsoncons REQUIRED CONFIG)
    set(JSONCONS_INCLUDE_DIR ${jsoncons_INCLUDE_DIRS})
endif ()

message(STATUS "Forcing tests build type to Release")
set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)

include(CheckCXXCompilerFlag)

string(TOUPPER "${CMAKE_BUILD_TYPE}" U_CMAKE_BUILD_TYPE)

################
# ARM SETTINGS #
################

OPTION(CROSS_COMPILE_ARM "cross compile for ARM targets" OFF)

# Note: to compile on ARM (or cross compile), you may need to add the following:
# -DTARGET_ARCH="armv8-a -mfpu=neon -mfloat-abi=softfp -target arm-linux-gnueabi"
set(TARGET_ARCH "native" CACHE STRING "Target architecture arguments")
set(ARM_ARCH_DIRECTORY "arm-linux-gnueabi" CACHE STRING "ARM arch header dir")
set(ARM_GCC_VER "4.7.3" CACHE STRING "ARM GCC header dir")

# load per-platform configuration
include (${JSONCONS_PROJECT_DIR}/build/cmake/${CMAKE_SYSTEM_NAME}.cmake)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    if (CROSS_COMPILE_ARM)
        # We're cross-compiling with clang++ on Travis, this is all pretty specific and just for testing
        set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
        set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)
        set(CMAKE_THREAD_LIBS_INIT)

        include_directories(/usr/${ARM_ARCH_DIRECTORY}/include/c++/${ARM_GCC_VER}/${ARM_ARCH_DIRECTORY}/)
        include_directories(/usr/${ARM_ARCH_DIRECTORY}/include/c++/${ARM_GCC_VER}/)
        include_directories(/usr/${ARM_ARCH_DIRECTORY}/include/)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=${TARGET_ARCH} -Wunused-parameter -Wextra -Wreorder -std=c++11")
    elseif(NOT WIN32)
        CHECK_CXX_COMPILER_FLAG("-std=c++11" HAS_CPP11_FLAG)
        if (HAS_CPP11_FLAG)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fPIC -march=${TARGET_ARCH} -Wunused-parameter -Wextra -Wreorder -std=c++11")
        else()
            message(FATAL_ERROR "Unsupported compiler -- jsoncons requires C++11 support!")
        endif()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer")
    endif()
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /MP /bigobj")
    set(CMAKE_EXE_LINKER_FLAGS /MANIFEST:NO)
endif()

set(CTEST_OUTPUT_ON_FAILURE ON)

set(JSONCONS_TESTS_DIR ${JSONCONS_PROJECT_DIR}/tests)
set(JSONCONS_TESTS_SOURCE_DIR ${JSONCONS_PROJECT_DIR}/tests/src)
set(JSONCONS_INCLUDE_DIR ${JSONCONS_PROJECT_DIR}/include)
set(JSONCONS_THIRD_PARTY_INCLUDE_DIR ${JSONCONS_PROJECT_DIR}/third_party)

set(CATCH_INCLUDE_DIR ${JSONCONS_THIRD_PARTY_INCLUDE_DIR}/catch)
add_library(Catch INTERFACE)
target_include_directories (Catch INTERFACE ${CATCH_INCLUDE_DIR})

file(GLOB_RECURSE JSONCONS_TESTS_SOURCES ${JSONCONS_TESTS_SOURCE_DIR}/*.cpp)

set(JSONCONS_TARGET test_jsoncons)
add_executable(${JSONCONS_TARGET} EXCLUDE_FROM_ALL ${JSONCONS_TESTS_SOURCES} ${JSONCONS_HEADERS})
target_include_directories (${JSONCONS_TARGET} PUBLIC ${JSONCONS_INCLUDE_DIR}
                                           PUBLIC ${JSONCONS_THIRD_PARTY_INCLUDE_DIR})

target_link_libraries(${JSONCONS_TARGET} Catch)

if (CROSS_COMPILE_ARM)
    add_custom_target(jtest COMMAND qemu-arm -L /usr/arm-linux-gnueabi/ test_jsoncons DEPENDS ${JSONCONS_TARGET})
else()
    add_custom_target(jtest COMMAND test_jsoncons DEPENDS ${JSONCONS_TARGET})
endif()

