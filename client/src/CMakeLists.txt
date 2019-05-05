cmake_minimum_required(VERSION 3.13)
project(client)

set(CMAKE_CXX_STANDARD 17)

add_executable(client main.cpp)
include_directories(${CMAKE_SOURCE_DIR}/kklib) # include files from kklib
target_link_libraries(client kklib)

install(TARGETS client RUNTIME DESTINATION bin)

find_package(Boost)
IF (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    set (Boost_USE_STATIC_LIBS OFF) # enable dynamic linking
    set (Boost_USE_MULTITHREAD ON)  # enable multithreading
    find_package (Boost COMPONENTS REQUIRED chrono filesystem)
    target_link_libraries(client  ${BOOST_LIBRARYDIR})
    if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        target_link_libraries(client ws2_32)
    endif()
endif()
