cmake_minimum_required(VERSION 3.13)
project(server  LANGUAGES CXX C VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")

add_executable(server main.cpp
        classes/server/room/kk_room.cpp classes/server/room/kk_room.h
        classes/server/kk_server.cpp classes/server/kk_server.h
        classes/server/session/kk_session.cpp classes/server/session/kk_session.h
        classes/server/file/kk_file.cpp classes/server/file/kk_file.h
        classes/server/partecipant/kk_partecipant.cpp classes/server/partecipant/kk_partecipant.h
        )


add_subdirectory(../../libs kklib)
target_link_libraries(server kklib)

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
