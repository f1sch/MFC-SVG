cmake_minimum_required(VERSION 3.20)

project(MFC_SVG LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Shared MFC
set(CMAKE_MFC_FLAG 2)

add_compile_definitions(_WIN32_WINNT=0x0A00)

add_subdirectory(svglib)
add_subdirectory(TestWindow)