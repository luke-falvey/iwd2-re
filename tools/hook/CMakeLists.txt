# Test-only tools for running several game instances at once (see
# iwd2hook.cpp). Built separately from the game:
#   cmake -S tools/hook -B build/hook -G "Visual Studio 17 2022" -A Win32
#   cmake --build build/hook --config Release
cmake_minimum_required(VERSION 3.20)
project(iwd2hook CXX)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

add_library(iwd2hook SHARED iwd2hook.cpp)
add_executable(iwd2launch iwd2launch.cpp)
