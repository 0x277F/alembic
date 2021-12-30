cmake_minimum_required(VERSION 3.20)
project(alembic_tests)

set(CMAKE_CXX_STANDARD 20)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options (-fdiagnostics-color=always)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options (-fcolor-diagnostics)
endif ()

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/9ca071b6e55568dff6960bebe1a5cfaa180fb3ce.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
enable_testing()

add_executable(alembic_tests src/attractor_traits.cpp src/flux_test.cpp src/builtins.cpp)
target_include_directories(alembic_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(alembic_tests gtest_main)
include(GoogleTest)
gtest_discover_tests(alembic_tests)
