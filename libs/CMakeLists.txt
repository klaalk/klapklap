
cmake_minimum_required(VERSION 3.13)
project(kklib LANGUAGES CXX C VERSION 0.1.0)
add_library(kklib STATIC
        src/classes_include.h
        src/classes/crypto/crypto.cpp src/classes/crypto/crypto.h

        src/classes/payload/kk_payload.cpp
        src/classes/payload/kk_payload.h
        src/constants/kk_constants.h

        src/classes/crdt/char/kk_char.cpp
        src/classes/crdt/char/kk_char.h
        src/classes/crdt/identifier/kk_identifier.cpp
        src/classes/crdt/identifier/kk_identifier.h
        src/classes/crdt/kk_crdt.cpp
        src/classes/crdt/kk_crdt.h
        src/classes/crdt/pos/kk_pos.cpp
        src/classes/crdt/pos/kk_pos.h
        )

#SLL
find_package(OpenSSL REQUIRED)
target_link_libraries(kklib OpenSSL::SSL)
#END_SSL