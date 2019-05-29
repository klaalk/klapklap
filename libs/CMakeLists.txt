
cmake_minimum_required(VERSION 3.13)
project(kklib LANGUAGES CXX C VERSION 0.1.0)

add_library(
        kklib STATIC
        src/classes_include.h
        src/classes/crypto/crypto.cpp src/classes/crypto/crypto.h
)

#SLL
find_package(OpenSSL REQUIRED)
target_link_libraries(kklib OpenSSL::SSL)
#END_SSL