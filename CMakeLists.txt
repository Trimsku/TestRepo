cmake_minimum_required(VERSION 3.20)
project(archiver)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_CURRENT_SOURCE_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(archiver main.cpp ZlibApi.cpp)
find_package(ZLIB)
target_link_libraries(archiver ZLIB::ZLIB)
