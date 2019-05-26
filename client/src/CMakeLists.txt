cmake_minimum_required(VERSION 3.13)
project(client LANGUAGES CXX C VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(Qt5Core REQUIRED )
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui REQUIRED )

set(RESOURCES
        assets/textedit.qrc
        )

set(SOURCES
        main.cpp
#        classes/textedit/textedit.cpp
        classes/client/kk_client.cpp
        )

set(HEADERS
#        classes/textedit/textedit.h
        classes/client/kk_client.h
        )

# This will create you executable
add_executable(client ${SOURCES} ${HEADERS} ${RESOURCES})
# This will link necessary Qt5 libraries to your project
qt5_use_modules(client Core Gui Widgets)


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

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_link_libraries(client ws2_32)
    target_link_libraries(client wsock32)
endif()