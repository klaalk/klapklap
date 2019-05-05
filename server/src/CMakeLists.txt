cmake_minimum_required(VERSION 3.13)
project(server)

set(CMAKE_CXX_STANDARD 17)

add_executable(server main.cpp classes/server/server.cpp classes/server/server.h)
include_directories(${CMAKE_SOURCE_DIR}/kklib) # include files from kklib
target_link_libraries(server kklib)

install(TARGETS server RUNTIME DESTINATION bin)


find_package(Boost)
IF (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    set (Boost_USE_STATIC_LIBS OFF) # enable dynamic linking
    set (Boost_USE_MULTITHREAD ON)  # enable multithreading
    find_package (Boost COMPONENTS REQUIRED chrono filesystem)
    target_link_libraries(server  ${BOOST_LIBRARYDIR})
    if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        target_link_libraries(server ws2_32)
        target_link_libraries(server wsock32)
    endif()
endif()
