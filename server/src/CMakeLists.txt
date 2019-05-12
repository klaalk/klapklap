cmake_minimum_required(VERSION 3.13)
project(server)

set(CMAKE_CXX_STANDARD 14)

add_executable(server main.cpp classes/db_connector/db_connector.cpp classes/db_connector/db_connector.h classes/db_interface/db_interface.cpp classes/db_interface/db_interface.h classes/classes_include.h classes/db_interface/HTTP_listener/HTTP_listener.cpp classes/db_interface/HTTP_listener/HTTP_listener.h classes/db_interface/HTTP_session/HTTP_session.cpp classes/db_interface/HTTP_session/HTTP_session.h classes/db_interface/HTTP_request_handler/HTTP_request_handler.h)
find_package(Boost)
IF (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    set (Boost_USE_STATIC_LIBS OFF) # enable dynamic linking
    set (Boost_USE_MULTITHREAD ON)  # enable multithreading
    find_package (Boost COMPONENTS REQUIRED chrono filesystem)
    target_link_libraries(server  ${BOOST_LIBRARYDIR})
    if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
            target_link_libraries(server ws2_32)
        endif()
endif()


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

target_link_libraries( server mysqlcppconn)