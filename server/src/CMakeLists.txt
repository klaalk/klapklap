cmake_minimum_required(VERSION 3.13)
project(server)

set(CMAKE_CXX_STANDARD 17)
add_executable(server main.cpp)
add_library(asio SHARED ../libs/asio-1.12.2/include/asio/impl/src.cpp)

find_package(Boost)
IF (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    set (Boost_USE_STATIC_LIBS OFF) # enable dynamic linking
    set (Boost_USE_MULTITHREAD ON)  # enable multithreading
    find_package (Boost COMPONENTS REQUIRED chrono filesystem)
    target_link_libraries (boost Boost)
endif()