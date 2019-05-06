cmake_minimum_required(VERSION 3.13)
project(client)

set(CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")

add_executable(client main.cpp
        classes/chat/chat_client.cpp classes/chat/chat_client.h
        classes/message/chat_message.cpp classes/message/chat_message.h)

#include_directories(${CMAKE_SOURCE_DIR}/kklib) # include files from kklib
#target_link_libraries(client kklib)
#
#install(TARGETS client RUNTIME DESTINATION bin)

find_package(Boost)
IF (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    set (Boost_USE_STATIC_LIBS OFF) # enable dynamic linking
    set (Boost_USE_MULTITHREAD ON)  # enable multithreading
    find_package (Boost COMPONENTS chrono filesystem thread system REQUIRED)
    target_link_libraries(client ${BOOST_LIBRARYDIR})
    if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        target_link_libraries(client ws2_32)
    endif()
endif()
