cmake_minimum_required(VERSION 3.13)
project(client LANGUAGES CXX C VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 14)
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    #    Non supportato in MacOS (da controllare su linux)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")
endif ()


add_executable(client main.cpp
        classes/chat/chat_client.cpp classes/chat/chat_client.h
        ../src/classes/auth_service/auth.cpp ../src/classes/auth_service/auth.h
        )

add_subdirectory(../../libs kklib)
target_link_libraries(client kklib)


find_package(Boost)
IF (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    set (Boost_USE_STATIC_LIBS OFF) # enable dynamic linking
    set (Boost_USE_MULTITHREAD ON)  # enable multithreading
    find_package (Boost COMPONENTS chrono filesystem thread system REQUIRED)
    target_link_libraries(client ${BOOST_LIBRARYDIR})
endif()

target_link_libraries(client mysqlcppconn)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")


if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_link_libraries(client ws2_32)
    target_link_libraries(client wsock32)
endif()

#Serve per impostare conan come sorgente librerie
if (EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
else ()
    message(WARNING "The file conanbuildinfo.cmake doesn't exist, you have to run conan install first")
endif ()

find_package(Poco REQUIRED COMPONENTS Net NetSSL)
include_directories(${Poco_INCLUDE_DIRS})
target_link_libraries(client ${Poco_LIBRARIES})

