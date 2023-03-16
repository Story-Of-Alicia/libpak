cmake_minimum_required(VERSION 3.24)
project(libpak
        VERSION 1.0.0
        LANGUAGES CXX C
        DESCRIPTION "libpak is platform-agnostic implementation for reading and writing in the ntreev paks format")

add_library(libpak_project INTERFACE)

target_compile_features(libpak_project
        INTERFACE cxx_std_17)
if (MSVC)
    # MSVC compiler flags
    target_compile_options(libpak_project
            INTERFACE /W4)
else()
    # GCC compiler flags
    target_compile_options(libpak_project
            INTERFACE -Wall -Wextra -Wpedantic)
endif()

find_package(fmt REQUIRED)
find_package(ZLIB REQUIRED)

# libpak library
add_library(libpak
        include/libpak/libpak.hpp
        src/libpak/libpak.cpp
        include/libpak/definitions.hpp
        include/libpak/algorithms.hpp include/libpak/util.hpp include/test.hpp)
target_include_directories(libpak
        PUBLIC include)
target_link_libraries(libpak
        PRIVATE libpak_project)
target_link_libraries(libpak
        PUBLIC ZLIB::ZLIB fmt::fmt)

# libpak test
add_executable(libpak_test
        src/test/libpak_test.cpp)
target_link_libraries(libpak_test
        PRIVATE libpak_project)
target_link_libraries(libpak_test
        PUBLIC libpak)
