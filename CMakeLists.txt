cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0135 NEW)

project(IntegralEmbeddedTracing
    VERSION 0.1.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Detect if this project is the top-level one
set(INTEGRAL_IS_TOP_LEVEL OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(INTEGRAL_IS_TOP_LEVEL ON)
endif()

option(INTEGRAL_BUILD_TESTS "Build IntegralEmbeddedTracing tests" ${INTEGRAL_IS_TOP_LEVEL})

# Collect all source files from src/
file(GLOB_RECURSE INTEGRAL_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.[ch]
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.[ch]pp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.[ch]xx
)

# Main library target
add_library(IntegralEmbeddedTracing ${INTEGRAL_SOURCES})
add_library(IntegralEmbeddedTracing::IntegralEmbeddedTracing ALIAS IntegralEmbeddedTracing)

target_include_directories(IntegralEmbeddedTracing
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(IntegralEmbeddedTracing PUBLIC cxx_std_20)

# ----------------- Tests -----------------
if(INTEGRAL_BUILD_TESTS)
    include(FetchContent)

    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )

    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    file(GLOB_RECURSE INTEGRAL_TEST_SOURCES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp
    )

    add_executable(IntegralEmbeddedTracingTests ${INTEGRAL_TEST_SOURCES})

    target_link_libraries(IntegralEmbeddedTracingTests PRIVATE
        IntegralEmbeddedTracing::IntegralEmbeddedTracing
        GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(IntegralEmbeddedTracingTests)
endif()
