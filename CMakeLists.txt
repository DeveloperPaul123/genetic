cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(
    genetic
    VERSION 0.2.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(
        FATAL_ERROR
            "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
    )
endif()

find_program(CCACHE_EXE ccache)
if(EXISTS ${CCACHE_EXE})
    message(STATUS "Found ccache ${CCACHE_EXE}")
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_EXE})
endif()

# ---- Add dependencies via CPM ----
# see https://github.com/TheLartians/CPM.cmake for more info
include(cmake/CPM.cmake)

# PackageProject.cmake will be used to make our target installable
CPMAddPackage("gh:TheLartians/PackageProject.cmake@1.6.0")

CPMAddPackage(
    NAME thread-pool
    GITHUB_REPOSITORY DeveloperPaul123/thread-pool
    VERSION 0.6.2
    GIT_TAG 0.6.2
    OPTIONS "TP_BUILD_TESTS OFF" "TP_BUILD_BENCHMARKS OFF" "TP_BUILD_EXAMPLES OFF"
)

# ---- Add source files ----

# Note: globbing sources is considered bad practice as CMake's generators may not detect new files
# automatically. Keep that in mind when changing files, or explicitly mention them here.
file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

# ---- Create library ----
add_library(${PROJECT_NAME} INTERFACE)
add_library(dp::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

message(STATUS "Compiler version: ${CMAKE_CXX_COMPILER_VERSION}")

if((MSVC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.32)
   OR (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 12)
   OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 17)
)
    option(DP_GENETIC_EXPERIMENTAL "Turn on experimental C++23 feature support" ON)
endif()

if(DEFINED DP_GENETIC_EXPERIMENTAL AND DP_GENETIC_EXPERIMENTAL)
    message(STATUS "Using experimental C++23 features.")
    target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_23)
else()
    target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)
endif()

# being a cross-platform target, we enforce standards conformance on MSVC
target_compile_options(${PROJECT_NAME} INTERFACE $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->)

target_include_directories(
    ${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                              $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
)

target_link_libraries(${PROJECT_NAME} INTERFACE dp::thread-pool)

# ---- Create an installable target ----
# this allows users to install and find the library via `find_package()`.

# the location where the project's version header will be placed should match the project's regular
# header paths
string(TOLOWER ${PROJECT_NAME}/version.h VERSION_HEADER_LOCATION)

packageProject(
    NAME ${PROJECT_NAME}
    VERSION ${PROJECT_VERSION}
    NAMESPACE ${PROJECT_NAME}
    BINARY_DIR ${PROJECT_BINARY_DIR}
    INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include
    INCLUDE_DESTINATION include/${PROJECT_NAME}-${PROJECT_VERSION}
    VERSION_HEADER "${VERSION_HEADER_LOCATION}"
    COMPATIBILITY SameMajorVersion
    DEPENDENCIES ""
)

option(DP_GENETIC_BUILD_TESTS "Turn on to build unit tests." ON)
option(DP_GENETIC_BUILD_EXAMPLES "Turn on to build examples." ON)
option(DP_GENETIC_BUILD_BENCHMARKS "Turn on to build benchmarks." ON)

if(${DP_GENETIC_BUILD_TESTS})
    enable_testing()
    add_subdirectory(test)
endif()
if(${DP_GENETIC_BUILD_EXAMPLES})
    add_subdirectory(examples)
endif()
if(${DP_GENETIC_BUILD_BENCHMARKS})
    # add_subdirectory(benchmark)
endif()
