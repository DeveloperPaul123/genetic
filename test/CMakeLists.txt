cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(genetic_tests LANGUAGES CXX)

# ---- Options ----
option(TEST_INSTALLED_VERSION "Test the version found by find_package" OFF)

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies ----
include(../cmake/CPM.cmake)

CPMAddPackage("gh:onqtam/doctest#2.4.5")
CPMAddPackage("gh:TheLartians/Format.cmake@1.7.0")

if(TEST_INSTALLED_VERSION)
    find_package(dp::genetic REQUIRED)
endif()

# ---- Create binary ----

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
add_executable(${PROJECT_NAME} ${sources})
target_link_libraries(${PROJECT_NAME} doctest::doctest dp::genetic)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)

# enable compiler warnings
if(NOT TEST_INSTALLED_VERSION)
    target_compile_options(
        ${PROJECT_NAME} INTERFACE $<$<COMPILE_LANG_AND_ID:CXX,Clang,GNU>:-Wall -Wpedantic -Wextra
                                  -Werror>
    )
    target_compile_options(${PROJECT_NAME} INTERFACE $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W4 /WX>)
    target_compile_definitions(
        ${PROJECT_NAME} PUBLIC $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:DOCTEST_CONFIG_USE_STD_HEADERS>
    )
endif()

# Note: doctest and similar testing frameworks can automatically configure CMake tests. For other
include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(${PROJECT_NAME})
