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
#   SOCK
    target_link_libraries(server ws2_32)
    target_link_libraries(server wsock32)
endif ()

add_executable(server main.cpp
        classes/db_connector/db_connector.cpp classes/db_connector/db_connector.h
        classes/QSMTP_service/src/SmtpMime
        classes/QSMTP_service/src/emailaddress.cpp classes/QSMTP_service/src/emailaddress.h
        classes/QSMTP_service/src/mimeattachment.cpp classes/QSMTP_service/src/mimeattachment.h
        classes/QSMTP_service/src/mimecontentformatter.cpp classes/QSMTP_service/src/mimecontentformatter.h
        classes/QSMTP_service/src/mimefile.cpp classes/QSMTP_service/src/mimefile.h
        classes/QSMTP_service/src/mimehtml.cpp classes/QSMTP_service/src/mimehtml.h
        classes/QSMTP_service/src/mimeinlinefile.cpp classes/QSMTP_service/src/mimeinlinefile.h
        classes/QSMTP_service/src/mimemessage.cpp classes/QSMTP_service/src/mimemessage.h
        classes/QSMTP_service/src/mimemultipart.cpp classes/QSMTP_service/src/mimemultipart.h
        classes/QSMTP_service/src/mimepart.cpp classes/QSMTP_service/src/mimepart.h
        classes/QSMTP_service/src/mimetext.cpp classes/QSMTP_service/src/mimetext.h
        classes/QSMTP_service/src/quotedprintable.cpp classes/QSMTP_service/src/quotedprintable.h
        classes/QSMTP_service/src/smtpclient.cpp classes/QSMTP_service/src/smtpclient.h
        classes/QSMTP_service/src/smtpexports.h
        classes/QSMTP_service/QSMTP_service.cpp classes/QSMTP_service/QSMTP_service.h
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
find_package(Qt5Gui REQUIRED )
find_package(Qt5Network REQUIRED)
find_package(Qt5Sql REQUIRED)
qt5_use_modules(server Core Gui Widgets Network Sql)
#END_QT

#KKLIB
add_subdirectory(../../libs kklib)
target_link_libraries(server kklib)
#END_KKLIB