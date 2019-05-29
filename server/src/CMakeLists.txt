cmake_minimum_required(VERSION 3.13)
project(server LANGUAGES CXX C VERSION 0.1.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 14)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    #    Non supportato in MacOS
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")
endif ()

add_executable(server main.cpp
        classes/server/room/kk_room.cpp classes/server/room/kk_room.h
        classes/server/kk_server.cpp classes/server/kk_server.h
        classes/server/session/kk_session.cpp classes/server/session/kk_session.h
        classes/server/file/kk_file.cpp classes/server/file/kk_file.h
        classes/server/partecipant/kk_partecipant.cpp classes/server/partecipant/kk_partecipant.h

        classes/db/kk_db.cpp classes/db/kk_db.h
        classes/smtp/mime/SmtpMime
        classes/smtp/mime/emailaddress.cpp classes/smtp/mime/emailaddress.h
        classes/smtp/mime/mimecontentformatter.cpp classes/smtp/mime/mimecontentformatter.h
        classes/smtp/mime/mimehtml.cpp classes/smtp/mime/mimehtml.h
        classes/smtp/mime/mimemessage.cpp classes/smtp/mime/mimemessage.h
        classes/smtp/mime/mimemultipart.cpp classes/smtp/mime/mimemultipart.h
        classes/smtp/mime/mimepart.cpp classes/smtp/mime/mimepart.h
        classes/smtp/mime/mimetext.cpp classes/smtp/mime/mimetext.h
        classes/smtp/mime/quotedprintable.cpp classes/smtp/mime/quotedprintable.h
        classes/smtp/mime/smtpclient.cpp classes/smtp/mime/smtpclient.h
        classes/smtp/mime/smtpexports.h
        classes/smtp/kk_smtp.cpp classes/smtp/kk_smtp.h
        )


find_package(Boost)
IF (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    set(Boost_USE_STATIC_LIBS OFF) # enable dynamic linking
    set(Boost_USE_MULTITHREAD ON)  # enable multithreading
    find_package(Boost COMPONENTS REQUIRED chrono filesystem)
    target_link_libraries(server ${BOOST_LIBRARYDIR})
endif ()

#CPPCONN
target_link_libraries(server mysqlcppconn)

#QT
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Sql REQUIRED)
qt5_use_modules(server Core Gui Widgets Network Sql)
#END_QT

#KKLIB
add_subdirectory(../../libs kklib)
target_link_libraries(server kklib)
#END_KKLIB

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    #   SOCK
    target_link_libraries(server ws2_32)
    target_link_libraries(server wsock32)
endif ()