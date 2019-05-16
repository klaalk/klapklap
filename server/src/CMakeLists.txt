cmake_minimum_required(VERSION 3.13)
project(server LANGUAGES CXX C VERSION 0.1.0)

# first build conan
# SEACRH LIB conan search <libname> -r=conan-center
# DEBUG conan install . -s build_type=Debug --install-folder=cmake-build-debug
# RELASE conan install . -s build_type=Release --install-folder=cmake-build-release

set(CMAKE_CXX_STANDARD 14)
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    #    Non supportato in MacOS (da controllare su linux)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")
endif ()

add_executable(server main.cpp
        classes/chat/chat_server.cpp classes/chat/chat_server.h
        classes/chat/session/chat_session.cpp classes/chat/session/chat_session.h
        classes/chat/room/chat_room.cpp classes/chat/room/chat_room.h
        classes/chat/partecipant/chat_partecipant.cpp classes/chat/partecipant/chat_partecipant.h

        classes/crdt/room/crdt_room.cpp classes/crdt/room/crdt_room.h
        classes/crdt/crdt_server.cpp classes/crdt/crdt_server.h
        classes/crdt/session/crdt_session.cpp classes/crdt/session/crdt_session.h
        classes/crdt/file/crdt_file.cpp classes/crdt/file/crdt_file.h
        classes/crdt/partecipant/crdt_partecipant.cpp classes/crdt/partecipant/crdt_partecipant.h
        classes/classes_include.h
        classes/db_connector/db_connector.cpp classes/db_connector/db_connector.h
        classes/db_interface/db_interface.cpp classes/db_interface/db_interface.h
        classes/classes_include.h classes/db_interface/HTTP_listener/HTTP_listener.cpp
        classes/db_interface/HTTP_listener/HTTP_listener.h classes/db_interface/HTTP_session/HTTP_session.cpp classes/SMTP_client/SMTP_client.cpp classes/SMTP_client/SMTP_client.h)

add_subdirectory(../../libs kklib)
target_link_libraries(server kklib)

find_package(Boost)
IF (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    set(Boost_USE_STATIC_LIBS OFF) # enable dynamic linking
    set(Boost_USE_MULTITHREAD ON)  # enable multithreading
    find_package(Boost COMPONENTS REQUIRED chrono filesystem)
    target_link_libraries(server ${BOOST_LIBRARYDIR})
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
target_link_libraries(server mysqlcppconn)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_link_libraries(server ws2_32)
    target_link_libraries(server wsock32)
endif ()


#Serve per impostare conan come sorgente librerie
if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
else()
    message(WARNING "The file conanbuildinfo.cmake doesn't exist, you have to run conan install first")
endif()

find_package(Poco REQUIRED COMPONENTS Net NetSSL)
include_directories( ${Poco_INCLUDE_DIRS})
target_link_libraries(server ${Poco_LIBRARIES})
