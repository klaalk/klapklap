project(kklib LANGUAGES CXX C VERSION 0.1.0)
add_library(kklib STATIC
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
