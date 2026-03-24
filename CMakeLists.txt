cmake_minimum_required(VERSION 3.24)
project(libpak
        VERSION 1.0.0
        LANGUAGES CXX C
        DESCRIPTION "libpak is library for manipulating the Ntreev PAK format")

add_subdirectory(3rd-party)

add_library(libpak-properties INTERFACE)

target_compile_features(libpak-properties
        INTERFACE cxx_std_23)

if (MSVC)
    # MSVC compiler flags
    target_compile_options(libpak-properties
            INTERFACE /W4)
else()
    # GCC compiler flags
    target_compile_options(libpak-properties
            INTERFACE -Wall -Wextra -Wpedantic)
endif()

# libpak library
add_library(libpak src/libpak/algorithms.cpp src/libpak/libpak.cpp)
target_include_directories(libpak
        PUBLIC include)
target_link_libraries(libpak
        PRIVATE libpak-properties)
target_link_libraries(libpak
        PUBLIC zlibstatic)
